
/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "pipe.h"
#include <sys/shm.h>
#include <sys/msg.h>

int main(int argc, char * argv[]) {

  if(signal(SIGALRM, sigHandler) == SIG_ERR){
    ErrExit("changing signal handler failed");
  }
  //================================================================================
  //dichiarazione e inizializzazione dell'array che conterrà i pid dei processi  S1, S2, S3

  int pid_S[3];
  pid_S[0] = 0;
  pid_S[1] = 0;
  pid_S[2] = 0;

  //=================================================================================
  //dichiarazione intestazioni per F8, F1, F2, F3 e array PID


  const char heading[] = "Id;Message;IDSender;IDReceiver;TimeArrival;TimeDeparture\n";


  //=================================================================================
  //creazione pipe e allocazione delle variabili necessarie

  int pipe1[2];
  int pipe2[2];

  create_pipe(pipe1);
  create_pipe(pipe2);

  //=================================================================================
  //creazione fifo

  int fifo;
  if((fifo = mkfifo("OutputFiles/my_fifo.txt", S_IRWXO | S_IRWXU)) == -1){
    ErrExit("create fifo failed");
  }

  //=================================================================================
  //creazione della shared memory

  key_t shmKey = 01101101;
  int shmid;
  shmid = alloc_shared_memory(shmKey, sizeof(struct message));
  struct msg *messageSH = (struct msg *)get_shared_memory(shmid, 0);
  //==================================================================================
  //creazione della message queue tra Senders e Receivers

  struct mymsg{
    long mtype;
    struct msg m_message;
  }m;

  m.mtype = 1;

  key_t msgKey = 01110001;
  int msqid = msgget(msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(msqid == -1){
    ErrExit("msgget failed");
  }


  //==================================================================================
  //creazione della message queue tra Sender e i loro figli

  struct child{
    long mtype;
    struct msg m_message;
    struct container msgFile;
  }internal_msg;

  internal_msg.mtype = 1;

  key_t mqKey = ftok("receiver_manager.c", 'A');
  int mqid = msgget(mqKey, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqid == -1){
    ErrExit("internal msgget failed");
  }


  //==================================================================================
  //creazione dei semafori

  key_t semKey = 01110011;
  int semid = semget(semKey, 2, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal[] = {1};
  union semun arg;
  arg.array = semInitVal;

  if(semctl(semid, 0, SETALL, arg) == -1){
    ErrExit("semctl failed");
  }

  //2nd semaphore set
  key_t semKey2 = ftok("receiver_manager.c", 'H');
  int semid2 = semget(semKey2, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid2 == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal2[1];
  union semun arg2;
  arg2.array = semInitVal2;

  if(semctl(semid2, 0, GETVAL, arg2) == -1){
    ErrExit("semctl failed");
  }


  //===================================================================================
  //generazione dei processi figlio

  int pid;
  pid = fork(); //creo S1

  if(pid != 0){ //sono nel padre
    pid_S[0] = pid; //salvo pid di S1
  }else if(pid == 0){ //sono in S1
    pid_S[0] = getpid(); //inizializzo
    pid_S[1] = 0; //inizializzo
    pid_S[2] = 0; //inizializzo
  }else{
    ErrExit("Error on child S1");
  }

  if(pid != 0){ //sono nel padre
    pid = fork(); //creo S2

    if(pid == 0){ //sono in S2
      pid_S[0] = 0; //inizializzo
      pid_S[1] = getpid();
      pid_S[2] = 0;
    }

    if(pid != 0){ //sono nel padre
      pid_S[1] = pid; //salvo pid di S2
    }
  }

  if(pid != 0){
    pid = fork();   //creo S3

    if(pid == 0){ //sono in S3
      pid_S[0] = 0; //inizializzo
      pid_S[1] = 0;
      pid_S[2] = getpid();
    }

    if(pid != 0){ //sono nel padre
      pid_S[2] = pid; //salvo pid di S2
    }
  }

  //==============================ESECUZIONE S1========================================//

  if(pid == 0 && pid_S[0] > 0 && pid_S[1] == 0 && pid_S[2] == 0){
    if(close(pipe1[0]) == -1){
      ErrExit("Close of Read hand of pipe1 failed.\n");
    }

    getcwd(path, PATH_SZ);
    strcat(path, argv[1]);	//troviamo F0.csv

    int F0 = open(path, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH); //apriamo F0.csv in sola lettura

    if(F0 == -1){
      ErrExit("open F0 failed");
    }


    lseek(F0, strlen(heading) - 3, SEEK_CUR); //ci spostiamo dopo l'intestazione (heading)

    int F1 = open("OutputFiles/F1.csv", O_RDWR | O_CREAT, S_IRWXU); //creiamo F1.csv con permessi di lettura scrittura

    if(F1 == -1){
      ErrExit("open F1 failed");
    }

    ssize_t numWrite = write(F1, heading, strlen(heading)); //scriviamo l'intestazione sul file F1.csv
    if(numWrite != strlen(heading)){
      ErrExit("write F1 failed");
    }

    int i = 0;
    ssize_t numRead;
    int start_line = 0; //indica i-esimo carattere da qui ha inizio un riga
    ssize_t internal_mSize = sizeof(struct child) - sizeof(long); //per la msgqueue tra Senders e i propri figli
    ssize_t mSize = sizeof(struct mymsg) - sizeof(long);  //per la msgqueue tra Senders e Receivers

    //inizializiamo i campi della struttura in modo da essere sicuri che siano vuoti
    struct msg message = {"", "", "", "", "", "", "", ""};

    while((numRead = read(F0, &buffer[i], sizeof(char))) > 0 && buffer[i] != '\0'){ //while che legge carattere per carattere da F0.csv

      if(buffer[i] == '\n'){  //se i è \n allora siamo alla fine della prima riga ovvero abbiamo trovato il primo messaggio

        strcpy(internal_msg.msgFile.time_arrival, "");  //inizializziamo i campi per la registrazione del tempo di arrivo e partenza dei messaggi
        strcpy(internal_msg.msgFile.time_departure, "");

        internal_msg.msgFile = get_time_arrival(internal_msg.msgFile); //segniamo l'ora di arrivo di F0 e la scriviamo in msgF1
        message = fill_structure(buffer, start_line); //riempiamo la struttura message con il messaggio appena letto

        //creo figlio che gestisce il messaggio
        pid = fork();

        if(pid == -1){
          ErrExit("creation child of S1 failed");
        }

        if(pid == 0){ //sono nel figlio
          sleep(atoi(message.delS1)); //il figlio dorme per DelS1 secondi
          internal_msg.m_message = message; //salva il messaggio all'interno del campo specifico della struttura della msgqueue
          if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //invia il mesaaggio
            ErrExit("message send failed (S1 child)");
          }

          exit(0);  //termina
        }

        if(msgrcv(mqid, &internal_msg, internal_mSize, 1, IPC_NOWAIT) == -1){ //se non c'è ancora nessun messaggio non si fa nulla

        }else{//altrimenti guardiamo chi lo deve inviare
          if(strcmp(internal_msg.m_message.idSender, "S1") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S1
            internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
            writeFile(internal_msg.msgFile, internal_msg.m_message, F1);	//scrittura messaggio su F1

            //inviamo il messaggio alla corrispondente IPC
            if(strcmp(internal_msg.m_message.type, "Q") == 0){  //MESSAGE QUEUE
              m.m_message = internal_msg.m_message; //salviamo il messaggio nell'apposito campo della struttura di trasmissione

              //cambiamo l'mtype in base a chi deve ricevere il messaggio,
              // in modo che sia più facile al processo receiver capire quali messaggi sono destinati a se stesso
              if(strcmp(m.m_message.idReceiver, "R1") == 0){
                m.mtype = 1;
              }

              if(strcmp(m.m_message.idReceiver, "R2") == 0){
                m.mtype = 2;
              }

              if(strcmp(m.m_message.idReceiver, "R3") == 0){
                m.mtype = 3;
              }

              //infine inviamo il messaggio alla msg queue mymsg
              if(msgsnd(msqid, &m, mSize ,0) == -1){
                ErrExit("message send failed (S1)");
              }
            }

            if(strcmp(internal_msg.m_message.type, "SH") == 0){ //SHARED MEMORY

              semOp(semid, 0, -1);	//entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

              //scrivo sulla shared memory il  messaggio
              strcpy(messageSH->id, internal_msg.m_message.id);
              strcpy(messageSH->message, internal_msg.m_message.message);
              strcpy(messageSH->idSender, internal_msg.m_message.idSender);
              strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
              strcpy(messageSH->delS1, internal_msg.m_message.delS1);
              strcpy(messageSH->delS2, internal_msg.m_message.delS2);
              strcpy(messageSH->delS3, internal_msg.m_message.delS3);
              strcpy(messageSH->type, internal_msg.m_message.type);

              //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
              semOp(semid, 0, 1);
            }

          }

          if(strcmp(internal_msg.m_message.idSender, "S2") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S2
            internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
            writeFile(internal_msg.msgFile, internal_msg.m_message, F1);	//scrittura messaggio su F1

            //mandiamo ad S2 tramite pipe1
            ssize_t nBys = write(pipe1[1], &internal_msg.m_message, sizeof(internal_msg.m_message));
            if(nBys != sizeof(internal_msg.m_message))
            ErrExit("write to pipe1 failed");
          }

          if(strcmp(internal_msg.m_message.idSender, "S3") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S3
            internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
            writeFile(internal_msg.msgFile, internal_msg.m_message, F1);	//scrittura messaggio su F1

            //mandiamo ad S3 tramite pipe1
            ssize_t nBys = write(pipe1[1], &internal_msg.m_message, sizeof(internal_msg.m_message));
            if(nBys != sizeof(internal_msg.m_message))
            ErrExit("write to pipe1 failed");
          }
        }
        start_line = i + 1; //inizio di una nuova riga
      }
      i++; //carattere sucessivo
    }

    while(wait(NULL) != -1){  //aspettiamo che i figli di S1 terminino
      if(msgrcv(mqid, &internal_msg, internal_mSize, 1, IPC_NOWAIT) == -1){ //se non c'è nessun messaggio da leggere allora non facciamo nulla

      }else{  //altrimenti guardiamo chi lo deve inviare
        strcpy(internal_msg.msgFile.time_departure, "");  //inizializiamo il campo per la partenza del mex

        if(strcmp(internal_msg.m_message.idSender, "S1") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S1
          internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
          writeFile(internal_msg.msgFile, internal_msg.m_message, F1);	//scrittura messaggio su F1

          //inviamo il messaggio alla corrispondente IPC
          if(strcmp(internal_msg.m_message.type, "Q") == 0){  // MESSAGE QUEUE
            m.m_message = internal_msg.m_message; //salviamo il messaggio nell'apposito campo della struttura di trasmissione

            //cambiamo l'mtype in base a chi deve ricevere il messaggio,
            // in modo che sia più facile al processo receiver capire quali messaggi sono destinati a se stesso
            if(strcmp(m.m_message.idReceiver, "R1") == 0){
              m.mtype = 1;
            }

            if(strcmp(m.m_message.idReceiver, "R2") == 0){
              m.mtype = 2;
            }

            if(strcmp(m.m_message.idReceiver, "R3") == 0){
              m.mtype = 3;
            }

            //infine inviamo il messaggio alla msg queue
            if(msgsnd(msqid, &m, mSize ,0) == -1){
              ErrExit("message send failed (S1)");
            }
          }

          if(strcmp(internal_msg.m_message.type, "SH") == 0){ //SHARED MEMORY
            semOp(semid, 0, -1);	//entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

            //scrivo sulla shared memory il  messaggio
            strcpy(messageSH->id, internal_msg.m_message.id);
            strcpy(messageSH->message, internal_msg.m_message.message);
            strcpy(messageSH->idSender, internal_msg.m_message.idSender);
            strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
            strcpy(messageSH->delS1, internal_msg.m_message.delS1);
            strcpy(messageSH->delS2, internal_msg.m_message.delS2);
            strcpy(messageSH->delS3, internal_msg.m_message.delS3);
            strcpy(messageSH->type, internal_msg.m_message.type);

            //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
            semOp(semid, 0, 1);
          }

        }

        if(strcmp(internal_msg.m_message.idSender, "S2") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S2
          internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
          writeFile(internal_msg.msgFile, internal_msg.m_message, F1);	//scrittura messaggio su F1

          //mandiamo ad S2 tramite pipe1
          ssize_t nBys = write(pipe1[1], &internal_msg.m_message, sizeof(internal_msg.m_message));
          if(nBys != sizeof(internal_msg.m_message))
          ErrExit("write to pipe1 failed");

        }

        if(strcmp(internal_msg.m_message.idSender, "S3") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S3
          internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
          writeFile(internal_msg.msgFile, internal_msg.m_message, F1);	//scrittura messaggio su F1
          //mandiamo ad S3 tramite pipe1
          ssize_t nBys = write(pipe1[1], &internal_msg.m_message, sizeof(internal_msg.m_message));
          if(nBys != sizeof(internal_msg.m_message))
          ErrExit("write to pipe1 failed");
        }
      }
    }
    if(close(F1) == -1) //chiudiamo il file descriptor di F1
    ErrExit("close");

    //inizializzimo una struttura di tipo msg che servirà a comunicare che non ci sono più messaggi
    struct msg final_msg = {"-1", "", "", "", "", "", "", ""};

    //inviamo il messaggio finale tramite pipe
    numWrite = write(pipe1[1], &final_msg, sizeof(struct msg));

    if(close(pipe1[1]) == -1){  //chiusura del canale del canale di scrittura
      ErrExit("close pipe1 write end in S1 failed");
    }

    exit(0);  //terminazione S1

  }else if(pid == 0 && pid_S[0] == 0 && pid_S[1] > 0 && pid_S[2] == 0){

    //==============================ESECUZIONE S2========================================//

    if(close(pipe1[1]) == -1){  //chiusura del canale di scrittura di pipe1
      ErrExit("Close of Write end of pipe1 failed");
    }

    if(close(pipe2[0]) == -1){  //chiusura del canale di lettura di pipe2
      ErrExit("Close of Read end of pipe2 failed");
    }

    int F2 = open("OutputFiles/F2.csv", O_RDWR | O_CREAT, S_IRWXU); //creiamo F2.csv con permessi di lettura scrittura

    if(F2 == -1){
      ErrExit("open F2 failed");
    }

    ssize_t numWrite = write(F2, heading, strlen(heading)); //scriviamo l'intestazione sul file F2.csv
    if(numWrite != strlen(heading)){
      ErrExit("write F2 failed");
    }

    //inizializzimo una struttura di tipo msg che servirà a comunicare che non ci sono più messaggi
    struct msg message = {"", "", "", "", "", "", "", ""};
    ssize_t nBys;
    ssize_t internal_mSize = sizeof(struct child) - sizeof(long); //per la msgqueue tra Senders e i propri figli
    ssize_t mSize = sizeof(struct mymsg) - sizeof(long);  //per la msgqueue tra Senders e Receivers


    while((nBys = read(pipe1[0], &message, sizeof(internal_msg.m_message))) > 0){ //while che legge messaggio per messaggio da pipe1

      if(strcmp(message.id, "-1") == 0){  //se id == -1 allora non ci sono più messaggi da leggere
        break;
      }

      strcpy(internal_msg.msgFile.time_arrival, "");  //inizializziamo i campi per la registrazione del tempo di arrivo e partenza dei messaggi
      strcpy(internal_msg.msgFile.time_departure, "");

      internal_msg.msgFile = get_time_arrival(internal_msg.msgFile); //registriamo l'ora di arrivo del messaggio

      //creo figlio che gestisce il messaggio
      pid = fork();

      if(pid == -1){
        ErrExit("creation child of S2 failed");
      }

      if(pid == 0){ //sono nel figlio
        sleep(atoi(message.delS2)); //il figlio dorme per DelS2 secondi
        internal_msg.m_message = message; //salva il messaggio all'interno del campo specifico della struttura della msgqueue
        internal_msg.mtype = 2; //cambiamo l'mtype per sapere quali messaggi S2 deve leggere dalla msg queue

        if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
          ErrExit("message send failed (S2 child)");
        }

        exit(0);
      }

      if(msgrcv(mqid, &internal_msg, internal_mSize, 2, IPC_NOWAIT) == -1){ //se non ci sono messaggi con mtype 2 nella msgqueue non fa nulla

      }else{  //altrimenti guardiamo chi lo deve inviare
        if(strcmp(internal_msg.m_message.idSender, "S2") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S2
          internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
          writeFile(internal_msg.msgFile, internal_msg.m_message, F2);	//scrittura messaggio su F1

          //inviamo il messaggio alla corrispondente IPC
          if(strcmp(internal_msg.m_message.type, "Q") == 0){  // MESSAGE QUEUE
            m.m_message = internal_msg.m_message; //salviamo il messaggio nell'apposito campo della struttura di trasmissione

            //cambiamo l'mtype in base a chi deve ricevere il messaggio,
            // in modo che sia più facile al processo receiver capire quali messaggi sono destinati a se stesso
            if(strcmp(m.m_message.idReceiver, "R1") == 0){
              m.mtype = 1;
            }

            if(strcmp(m.m_message.idReceiver, "R2") == 0){
              m.mtype = 2;
            }

            if(strcmp(m.m_message.idReceiver, "R3") == 0){
              m.mtype = 3;
            }

            //infine inviamo il messaggio alla msg queue
            if(msgsnd(msqid, &m, mSize ,0) == -1){
              ErrExit("message send failed (S1)");
            }
          }

          if(strcmp(internal_msg.m_message.type, "SH") == 0){ //SHARED MEMORY
            semOp(semid, 0, -1);	//entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

            //scrivo sulla shared memory il  messaggio
            strcpy(messageSH->id, internal_msg.m_message.id);
            strcpy(messageSH->message, internal_msg.m_message.message);
            strcpy(messageSH->idSender, internal_msg.m_message.idSender);
            strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
            strcpy(messageSH->delS1, internal_msg.m_message.delS1);
            strcpy(messageSH->delS2, internal_msg.m_message.delS2);
            strcpy(messageSH->delS3, internal_msg.m_message.delS3);
            strcpy(messageSH->type, internal_msg.m_message.type);

            //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
            semOp(semid, 0, 1);
          }

        }

        if(strcmp(internal_msg.m_message.idSender, "S3") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S3
          internal_msg.msgFile = get_time_departure(internal_msg.msgFile);  // registriamo il tempo di partenza del messaggio
          writeFile(internal_msg.msgFile, internal_msg.m_message, F2);	//scrittura messaggio su F1

          //mandiamo ad S3 tramite pipe2
          ssize_t nBys = write(pipe2[1], &internal_msg.m_message, sizeof(internal_msg.m_message));
          if(nBys != sizeof(internal_msg.m_message))
          ErrExit("write to pipe2 failed");
        }
      }
    }


    while(wait(NULL) != -1){  //aspettiamo che i figli di S2 terminino
      if(msgrcv(mqid, &internal_msg, internal_mSize, 2, IPC_NOWAIT) == -1){ //se non c'è nessun messaggio da leggere allora non facciamo nulla

      }else{  //altrimenti guardiamo chi lo deve inviare
        if(strcmp(internal_msg.m_message.idSender, "S2") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S2
          internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
          writeFile(internal_msg.msgFile, internal_msg.m_message, F2);	//scrittura messaggio su F2

          //inviamo il messaggio alla corrispondente IPC
          if(strcmp(internal_msg.m_message.type, "Q") == 0){  // MESSAGE QUEUE
            m.m_message = internal_msg.m_message;//salviamo il messaggio nell'apposito campo della struttura di trasmissione

            //cambiamo l'mtype in base a chi deve ricevere il messaggio,
            // in modo che sia più facile al processo receiver capire quali messaggi sono destinati a se stesso
            if(strcmp(m.m_message.idReceiver, "R1") == 0){
              m.mtype = 1;
            }

            if(strcmp(m.m_message.idReceiver, "R2") == 0){
              m.mtype = 2;
            }

            if(strcmp(m.m_message.idReceiver, "R3") == 0){
              m.mtype = 3;
            }

            if(msgsnd(msqid, &m, mSize ,0) == -1){
              ErrExit("message send failed (S1)");
            }
          }

          if(strcmp(internal_msg.m_message.type, "SH") == 0){ //SHARED MEMORY
            semOp(semid, 0, -1);	//entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

            //scrivo sulla shared memory il  messaggio
            strcpy(messageSH->id, internal_msg.m_message.id);
            strcpy(messageSH->message, internal_msg.m_message.message);
            strcpy(messageSH->idSender, internal_msg.m_message.idSender);
            strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
            strcpy(messageSH->delS1, internal_msg.m_message.delS1);
            strcpy(messageSH->delS2, internal_msg.m_message.delS2);
            strcpy(messageSH->delS3, internal_msg.m_message.delS3);
            strcpy(messageSH->type, internal_msg.m_message.type);

            //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
            semOp(semid, 0, 1);
          }

        }

        if(strcmp(internal_msg.m_message.idSender, "S3") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S3
          internal_msg.msgFile = get_time_departure(internal_msg.msgFile);  // registriamo il tempo di partenza del messaggio
          writeFile(internal_msg.msgFile, internal_msg.m_message, F2);	//scrittura messaggio su F2

          //mandiamo ad S3 tramite pipe1
          ssize_t nBys = write(pipe2[1], &internal_msg.m_message, sizeof(internal_msg.m_message));
          if(nBys != sizeof(internal_msg.m_message))
          ErrExit("write to pipe2 failed");
        }
      }
    }


    if(close(pipe1[0]) == -1){
      ErrExit("Close of Read end of pipe1 failed (S2)");
    }

    struct msg final_msg = {"-1", "", "", "", "", "", "", ""};

    numWrite = write(pipe2[1], &final_msg, sizeof(struct msg));

    if(close(pipe2[1]) == -1){  //chiusura del canale del canale di scrittura
      ErrExit("Close of Write end of pipe2 failed (S2)");
    }

    exit(0);  //terminazione S2

  }else if(pid == 0 && pid_S[0] == 0 && pid_S[1] == 0 && pid_S[2] > 0){

    //==============================ESECUZIONE S3========================================//

    if(close(pipe2[1]) == -1){  //chiusura del canale di scrittura di pipe2
      ErrExit("Close write end of pipe2");
    }

    int F3 = open("OutputFiles/F3.csv", O_RDWR | O_CREAT, S_IRWXU); //creiamo F3.csv con permessi di lettura scrittura

    if(F3 == -1){
      ErrExit("open F3 failed");
    }

    ssize_t numWrite = write(F3, heading, strlen(heading)); //scriviamo l'intestazione sul file F3.csv
    if(numWrite != strlen(heading)){
      ErrExit("write F3 failed");
    }

    ssize_t nBys;

    //inizializzimo una struttura di tipo msg che servirà a comunicare che non ci sono più messaggi
    struct msg message = {"", "", "", "", "", "", "", ""};
    ssize_t internal_mSize = sizeof(struct child) - sizeof(long); //per la msgqueue tra Senders e i propri figli
    ssize_t mSize = sizeof(struct mymsg) - sizeof(long);  //per la msgqueue tra Senders e Receivers


    while((nBys = read(pipe2[0], &message, sizeof(internal_msg.m_message))) > 0){ //while che legge messaggio per messaggio da pipe2

      if(strcmp(message.id, "-1") == 0){  //se id == -1 allora non ci sono più messaggi da leggere
        break;
      }

      strcpy(internal_msg.msgFile.time_arrival, "");  //inizializziamo i campi per la registrazione del tempo di arrivo e partenza dei messaggi
      strcpy(internal_msg.msgFile.time_departure, "");

      internal_msg.msgFile = get_time_arrival(internal_msg.msgFile);  //registriamo l'ora di arrivo del messaggio

      //creo figlio che gestisce il messaggio
      pid = fork();

      if(pid == 0){ //sono nel figlio
        sleep(atoi(message.delS3)); //il figlio dorme per DelS3 secondi
        internal_msg.m_message = message; //salva il messaggio all'interno del campo specifico della struttura della msgqueue
        internal_msg.mtype = 3; //cambiamo l'mtype per sapere quali messaggi S3 deve leggere dalla msg queue

        if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
          ErrExit("message send failed (S3 child)");
        }

        exit(0);
      }

      if(msgrcv(mqid, &internal_msg, internal_mSize, 3, IPC_NOWAIT) == -1){ //se non ci sono messaggi con mtype 2 nella msgqueue non fa nulla

      }else{ //altrimenti guardiamo in che modo dobbiamo inviarlo
        internal_msg.msgFile = get_time_departure(internal_msg.msgFile);// registriamo il tempo di partenza del messaggio
        writeFile(internal_msg.msgFile, internal_msg.m_message, F3);	//scrittura messaggio su F3
        //inviamo il messaggio alla corrispondente IPC
        if(strcmp(internal_msg.m_message.type, "Q") == 0){  // MESSAGE QUEUE
          m.m_message = internal_msg.m_message; //salviamo il messaggio nell'apposito campo della struttura di trasmissione

          //cambiamo l'mtype in base a chi deve ricevere il messaggio,
          // in modo che sia più facile al processo receiver capire quali messaggi sono destinati a se stesso
          if(strcmp(m.m_message.idReceiver, "R1") == 0){
            m.mtype = 1;
          }

          if(strcmp(m.m_message.idReceiver, "R2") == 0){
            m.mtype = 2;
          }

          if(strcmp(m.m_message.idReceiver, "R3") == 0){
            m.mtype = 3;
          }

          //infine inviamo il messaggio alla msg queue
          if(msgsnd(msqid, &m, mSize ,0) == -1){
            ErrExit("message send failed (S1)");
          }

        }
        printf("semid: %d\n", semid);
        printf("semid2: %d\n", semid2);

        if(strcmp(internal_msg.m_message.type, "SH") == 0){
          semOp(semid, 0, -1);	//entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

          //scrivo sulla shared memory il  messaggio
          strcpy(messageSH->id, internal_msg.m_message.id);
          strcpy(messageSH->message, internal_msg.m_message.message);
          strcpy(messageSH->idSender, internal_msg.m_message.idSender);
          strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
          strcpy(messageSH->delS1, internal_msg.m_message.delS1);
          strcpy(messageSH->delS2, internal_msg.m_message.delS2);
          strcpy(messageSH->delS3, internal_msg.m_message.delS3);
          strcpy(messageSH->type, internal_msg.m_message.type);

          //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
          semOp(semid2, 0, 1);
        }

        if(strcmp(internal_msg.m_message.type, "FIFO") == 0){  //FIFO

          int fifo = open("OutputFiles/my_fifo.txt", O_WRONLY); //apro il file descriptor relativo alla FIFO in sola scrittura

          if(fifo == -1){
            ErrExit("open (fifo) failed");
          }

          numWrite = write(fifo, &internal_msg.m_message, sizeof(internal_msg.m_message));  // scrivo il messaggio sulla FIFO

          if(numWrite != sizeof(internal_msg.m_message)){
            ErrExit("write on fifo failed");
          }

        }
      }
    }
    while(wait(NULL) != -1){  //aspettiamo che i figli di S3 terminino
      if(msgrcv(mqid, &internal_msg, internal_mSize, 3, IPC_NOWAIT) == -1){ //se non c'è nessun messaggio da leggere allora non facciamo nulla

      }else{
        internal_msg.msgFile = get_time_departure(internal_msg.msgFile);  // registriamo il tempo di partenza del messaggio
        writeFile(internal_msg.msgFile, internal_msg.m_message, F3);	//scrittura messaggio su F3

        //inviamo il messaggio alla corrispondente IPC
        if(strcmp(internal_msg.m_message.type, "Q") == 0){  //MESSAGE QUEUE
          m.m_message = internal_msg.m_message; //salviamo il messaggio nell'apposito campo della struttura di trasmissione

          //cambiamo l'mtype in base a chi deve ricevere il messaggio,
          // in modo che sia più facile al processo receiver capire quali messaggi sono destinati a se stesso
          if(strcmp(m.m_message.idReceiver, "R1") == 0){
            m.mtype = 1;
          }

          if(strcmp(m.m_message.idReceiver, "R2") == 0){
            m.mtype = 2;
          }

          if(strcmp(m.m_message.idReceiver, "R3") == 0){
            m.mtype = 3;
          }

          //infine inviamo il messaggio alla msg queue
          if(msgsnd(msqid, &m, mSize ,0) == -1){
            ErrExit("message send failed (S1)");
          }

        }

        if(strcmp(internal_msg.m_message.type, "SH") == 0){ //SHARED MEMORY
          semOp(semid, 0, -1);	//entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

          printSemaphoresValue(semid);
          printSemaphoresValue(semid2);
          //scrivo sulla shared memory il  messaggio
          strcpy(messageSH->id, internal_msg.m_message.id);
          strcpy(messageSH->message, internal_msg.m_message.message);
          strcpy(messageSH->idSender, internal_msg.m_message.idSender);
          strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
          strcpy(messageSH->delS1, internal_msg.m_message.delS1);
          strcpy(messageSH->delS2, internal_msg.m_message.delS2);
          strcpy(messageSH->delS3, internal_msg.m_message.delS3);
          strcpy(messageSH->type, internal_msg.m_message.type);

          //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
          semOp(semid2, 0, 1);
        }

        if(strcmp(internal_msg.m_message.type, "FIFO") == 0){  //FIFO

          int fifo = open("OutputFiles/my_fifo.txt", O_WRONLY); //apro il file descriptor relativo alla FIFO in sola scrittura

          if(fifo == -1){
            ErrExit("open (fifo) failed");
          }

          numWrite = write(fifo, &internal_msg.m_message, sizeof(internal_msg.m_message));  // scrivo il messaggio sulla FIFO

          if(numWrite != sizeof(internal_msg.m_message)){
            ErrExit("write on fifo failed");
          }

        }
      }
    }


    if(close(F3) == -1){  //chiusura file descriptor F3
      ErrExit("Close F3 failed");
    }

    if(close(pipe2[0]) == -1){  //chiusura lato lettura della pipe2
      ErrExit("Close of read end of pipe2 failed (S3)");
    }

    exit(0);  //terminazione S3

  }else if(pid != 0 && pid_S[0] > 0 && pid_S[1] > 0 && pid_S[2] > 0){

    //==============================ESECUZIONE PADRE========================================//

    writeF8(pid_S);  //scrittura del file F8.csv


    int status = 0;
    int i = 0;

    // aspettiamo la terminazione di S1, S2, S3
    while((pid = wait(&status)) != -1){
      printf("S%i %d exited, status = %d\n", i + 1, pid_S[i], WEXITSTATUS(status)); //qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
      i++;
    }

    //chiusura pipe 1
    if(close(pipe1[1]) == -1){
      ErrExit("close pipe1 write end in father failed");
    }

    if(close(pipe1[0]) == -1){
      ErrExit("close pipe1 read end in father failed");
    }

    //chiusura pipe2
    if(close(pipe2[1]) == -1){
      ErrExit("close pipe2 write end in father failed");
    }
    if(close(pipe2[0]) == -1){
      ErrExit("close pipe2 read end in father failed");
    }

    semOp(semid, 0, -1);

    strcpy(messageSH->id, "-1");

    semOp(semid, 1, 1);

    //chiusura lato scrittura fifo
    close(fifo);

    //detach della shared memory
    free_shared_memory(messageSH);

    //rimozione della msg queue
    if(msgctl(mqid, IPC_RMID, NULL) == -1){
      ErrExit("close of MSG QUEUE beetween Senders and children failed");
    }

    return 0; //terminazione di sender_manager

  }
}
