
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

bool wait_time = true;

void sigHandler(int sig){
  if(sig == SIGUSR1){
    sleep(5);
  }

  if(sig == SIGCONT){
    wait_time = false;
  }

}

int main(int argc, char * argv[]) {

  bool *check_time = &wait_time; //modifica il wait_time

  struct ipc historical[5] = {};

  int F10 = open("OutputFiles/F10.csv", O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR);

  if(F10 == -1){
    ErrExit("Open F10 failed\n");
  }

  char headingF10[] = "IPC;IDKey;Creator;CreationTime;DestructionTime\n";

  ssize_t numWrite = write(F10, headingF10, strlen(headingF10));

  if(numWrite != strlen(headingF10)){
    ErrExit("Error open F10");
  }

  if(signal(SIGUSR1, sigHandler) == SIG_ERR || signal(SIGCONT, sigHandler) == SIG_ERR){
    ErrExit("changing signal handler failed");
  }
  //================================================================================
  //dichiarazione e inizializzazione dell'array che conterrà i pid dei processi S1, S2, S3

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

  char pipe_1[2];
  char pipe_2[2];

  create_pipe(pipe1);
  create_pipe(pipe2);

  strcpy(historical[0].ipc, "PIPE1");
  strcpy(historical[1].ipc, "PIPE2");
  strcpy(historical[0].creator, "SM");
  strcpy(historical[1].creator, "SM");

  sprintf(pipe_1, "%d", pipe1[0]);
  strcpy(historical[0].idKey, pipe_1);
  sprintf(pipe_1, "%d", pipe1[1]);
  strcat(historical[0].idKey, "/");
  strcat(historical[0].idKey, pipe_1);

  sprintf(pipe_2, "%d", pipe2[0]);
  strcpy(historical[1].idKey, pipe_2);
  sprintf(pipe_2, "%d", pipe2[1]);
  strcat(historical[1].idKey, "/");
  strcat(historical[1].idKey, pipe_2);

  historical[0] = get_time(historical[0], 'c');
  historical[1] = get_time(historical[1], 'c');

  //=================================================================================
  //creazione fifo
  int fifo;
  if((fifo = mkfifo("OutputFiles/my_fifo.txt", S_IRUSR | S_IWUSR | S_IWGRP)) == -1){
    ErrExit("create fifo failed");
  }

  //=================================================================================
  //creazione della shared memory 	sprintf(idKey, "%x", historical.idKey);

  key_t shmKey = 01101101;
  int shmid;
  shmid = alloc_shared_memory(shmKey, sizeof(struct message));
  struct message *messageSH = (struct message *)get_shared_memory(shmid, 0);

  //=================================================================================
  //creazione della shared memory per la conidivisione del puntantore alla shmKey

  key_t shmKey2 = ftok("receiver_manager.c", 'L');

  int shmid2;
  shmid2 = alloc_shared_memory(shmKey2, sizeof(struct address));
  struct address *address = (struct address *)get_shared_memory(shmid2, 0);

  address->ptr = messageSH;

  sprintf(historical[2].idKey, "%x", shmKey2);
  strcpy(historical[2].ipc, "SH2");
  strcpy(historical[2].creator, "SM");
  historical[2] = get_time(historical[2], 'c');

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

  sprintf(historical[3].idKey, "%x", mqKey);
  strcpy(historical[3].ipc, "MQS");
  strcpy(historical[3].creator, "SM");
  historical[3] = get_time(historical[3], 'c');

  //==================================================================================
  //creazione della message queue tra Sender e Hackler

  struct signal sigInc; //MQ per IncreaseDelay

  sigInc.mtype = 1;

  key_t mqInc_Key = ftok("receiver_manager.c", 'H');
  int mqInc_id = msgget(mqInc_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqInc_id == -1){
    ErrExit("internal msgget failed");
  }


  struct signal sigRmv; //MQ per RemoveMSG

  sigRmv.mtype = 1;

  key_t mqRmv_Key = ftok("fifo.c", 'A');
  int mqRmv_id = msgget(mqRmv_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqRmv_id == -1){
    ErrExit("internal msgget failed");
  }

  struct signal sigSnd; //MQ per SendMSG

  sigSnd.mtype = 1;

  key_t mqSnd_Key = ftok("fifo.c", 'B');
  int mqSnd_id = msgget(mqSnd_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqSnd_id == -1){
    ErrExit("internal msgget failed");
  }

  //==================================================================================
  //creazione dei semafori

  key_t semKey = 01110011;
  int semid = semget(semKey, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal[] = {1};
  union semun arg;
  arg.array = semInitVal;

  if(semctl(semid, 0, SETALL, arg) == -1){
    ErrExit("semctl failed");
  }

  sprintf(historical[4].idKey, "%x", semKey);
  strcpy(historical[4].ipc, "SEMAPHORE");
  strcpy(historical[4].creator, "SM");
  historical[4] = get_time(historical[4], 'c');

  //==================================================================================
  //creazione dei semafori per la scrittura di F10

  key_t semKey2 = ftok("defines.c", 'D');
  int semid2 = semget(semKey2, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid2 == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal1[] = {1};
  union semun arg1;
  arg1.array = semInitVal1;

  if(semctl(semid2, 0, SETALL, arg1) == -1){
    ErrExit("semctl failed");
  }

  //==================================================================================
  //creazione dei semafori per la scrittura di F8

  key_t semKey3 = ftok("defines.c", 'E');
  int semid3 = semget(semKey3, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid3 == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal2[] = {0};
  union semun arg2;
  arg2.array = semInitVal2;

  if(semctl(semid3, 0, SETALL, arg2) == -1){
    ErrExit("semctl failed (semid3)");
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
      //printf("GLOBAL %d\n", global);

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

        if(pid > 0){
          //mandare il pid sulla MQ e cambio mtype
          sigInc.pid = pid;

          if(msgsnd(mqInc_id, &sigInc, sizeof(struct signal) - sizeof(long), 0) == -1){
            ErrExit("Sending pid to Hackler failed (S1 - INC)");
          }

          sigRmv.pid = pid;

          if(msgsnd(mqRmv_id, &sigRmv, sizeof(struct signal) - sizeof(long), 0) == -1){
            ErrExit("Sending pid to Hackler failed (S1 - RMV)");
          }

          sigSnd.pid = pid;

          if(msgsnd(mqSnd_id, &sigSnd, sizeof(struct signal) - sizeof(long), 0) == -1){
            ErrExit("Sending pid to Hackler failed (S1 - SND)");
          }
        }

        if(pid == 0){ //sono nel figlio
          int sec;

          if((sec = sleep(atoi(message.delS1))) > 0 && wait_time == true){ //il figlio dorme per DelS1 secondi
            sleep(sec);
            *check_time = false;  //rimettiamo wait_time a false per la prossima SendMSG
          }

          internal_msg.m_message = message; //salva il messaggio all'interno del campo specifico della struttura della msgqueue
          //internal_msg.msgFile = get_time_departure(internal_msg.msgFile);

          if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //invia il mesaaggio
            ErrExit("message send failed (S1 child)");
          }

          exit(0);  //termina
        }

        if(msgrcv(mqid, &internal_msg, internal_mSize, 1, IPC_NOWAIT) == -1){ //se non c'è ancora nessun messaggio non si fa nulla

      }else{//altrimenti guardiamo chi lo deve inviare
        internal_msg.msgFile = get_time_departure(internal_msg.msgFile);

        if(strcmp(internal_msg.m_message.idSender, "S1") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S1
          //internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
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

            messageSH = address->ptr;

            //printf("MessageSH ADDRESS %p\n", address->ptr);

            //scrivo sulla shared memory il  messaggio
            strcpy(messageSH->id, internal_msg.m_message.id);
            strcpy(messageSH->message, internal_msg.m_message.message);
            strcpy(messageSH->idSender, internal_msg.m_message.idSender);
            strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
            strcpy(messageSH->delS1, internal_msg.m_message.delS1);
            strcpy(messageSH->delS2, internal_msg.m_message.delS2);
            strcpy(messageSH->delS3, internal_msg.m_message.delS3);
            strcpy(messageSH->type, internal_msg.m_message.type);


            messageSH++;
            address->ptr = messageSH;

            //printf("MessageSH ADDRESS %p\n", address->ptr);
            //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
            semOp(semid, 0, 1);
          }

        }

        if(strcmp(internal_msg.m_message.idSender, "S2") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S2
          //internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
          writeFile(internal_msg.msgFile, internal_msg.m_message, F1);	//scrittura messaggio su F1

          //mandiamo ad S2 tramite pipe1
          ssize_t nBys = write(pipe1[1], &internal_msg.m_message, sizeof(internal_msg.m_message));
          if(nBys != sizeof(internal_msg.m_message))
          ErrExit("write to pipe1 failed");
        }

        if(strcmp(internal_msg.m_message.idSender, "S3") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S3
          //internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
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
    internal_msg.msgFile = get_time_departure(internal_msg.msgFile);

    if(strcmp(internal_msg.m_message.idSender, "S1") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S1
      //internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
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

        messageSH = address->ptr;
        //printf("MessageSH ADDRESS %p\n", address->ptr);

        //scrivo sulla shared memory il  messaggio
        strcpy(messageSH->id, internal_msg.m_message.id);
        strcpy(messageSH->message, internal_msg.m_message.message);
        strcpy(messageSH->idSender, internal_msg.m_message.idSender);
        strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
        strcpy(messageSH->delS1, internal_msg.m_message.delS1);
        strcpy(messageSH->delS2, internal_msg.m_message.delS2);
        strcpy(messageSH->delS3, internal_msg.m_message.delS3);
        strcpy(messageSH->type, internal_msg.m_message.type);


        messageSH++;
        address->ptr = messageSH;
        //printf("MessageSH ADDRESS %p\n", address->ptr);
        //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
        semOp(semid, 0, 1);
      }

    }

    if(strcmp(internal_msg.m_message.idSender, "S2") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S2
      //internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
      writeFile(internal_msg.msgFile, internal_msg.m_message, F1);	//scrittura messaggio su F1

      //mandiamo ad S2 tramite pipe1
      ssize_t nBys = write(pipe1[1], &internal_msg.m_message, sizeof(internal_msg.m_message));
      if(nBys != sizeof(internal_msg.m_message))
      ErrExit("write to pipe1 failed");

    }

    if(strcmp(internal_msg.m_message.idSender, "S3") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S3
      //internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
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


    if(pid > 0){
      //mandare il pid sulla MQ e cambio mtype
      sigInc.mtype = 2;
      sigInc.pid = pid;

      if(msgsnd(mqInc_id, &sigInc, sizeof(struct signal) - sizeof(long), 0) == -1){
        ErrExit("Sending pid to Hackler failed (S2 - INC)");
      }

      sigRmv.mtype = 2;
      sigRmv.pid = pid;

      if(msgsnd(mqRmv_id, &sigRmv, sizeof(struct signal) - sizeof(long), 0) == -1){
        ErrExit("Sending pid to Hackler failed (S2 - RMV)");
      }

      sigSnd.mtype = 2;
      sigSnd.pid = pid;

      if(msgsnd(mqSnd_id, &sigSnd, sizeof(struct signal) - sizeof(long), 0) == -1){
        ErrExit("Sending pid to Hackler failed (S2 - SND)");
      }

    }


    if(pid == 0){ //sono nel figlio
      int sec;

      if((sec = sleep(atoi(message.delS2))) > 0 && wait_time == true){ //il figlio dorme per DelS1 secondi
        sleep(sec);
        *check_time = false;
      }

      internal_msg.m_message = message; //salva il messaggio all'interno del campo specifico della struttura della msgqueue
      internal_msg.mtype = 2; //cambiamo l'mtype per sapere quali messaggi S2 deve leggere dalla msg queue
      internal_msg.msgFile = get_time_departure(internal_msg.msgFile);

      if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
        ErrExit("message send failed (S2 child)");
      }

      exit(0);
    }

    if(msgrcv(mqid, &internal_msg, internal_mSize, 2, IPC_NOWAIT) == -1){ //se non ci sono messaggi con mtype 2 nella msgqueue non fa nulla

    }else{  //altrimenti guardiamo chi lo deve inviare
      if(strcmp(internal_msg.m_message.idSender, "S2") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S2
        //internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
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
            ErrExit("message send failed (S2)");
          }
        }

        if(strcmp(internal_msg.m_message.type, "SH") == 0){ //SHARED MEMORY
          semOp(semid, 0, -1);	//entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

          messageSH = address->ptr;
          //printf("MessageSH ADDRESS %p\n", address->ptr);

          //scrivo sulla shared memory il  messaggio
          strcpy(messageSH->id, internal_msg.m_message.id);
          strcpy(messageSH->message, internal_msg.m_message.message);
          strcpy(messageSH->idSender, internal_msg.m_message.idSender);
          strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
          strcpy(messageSH->delS1, internal_msg.m_message.delS1);
          strcpy(messageSH->delS2, internal_msg.m_message.delS2);
          strcpy(messageSH->delS3, internal_msg.m_message.delS3);
          strcpy(messageSH->type, internal_msg.m_message.type);


          messageSH++;
          address->ptr = messageSH;
          //printf("MessageSH ADDRESS %p\n", address->ptr);
          //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
          semOp(semid, 0, 1);
        }

      }

      if(strcmp(internal_msg.m_message.idSender, "S3") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S3
        //internal_msg.msgFile = get_time_departure(internal_msg.msgFile);  // registriamo il tempo di partenza del messaggio
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
    //printf("GLOBAL S2 %d\n", global);
    if(strcmp(internal_msg.m_message.idSender, "S2") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S2
      //internal_msg.msgFile = get_time_departure(internal_msg.msgFile); // registriamo il tempo di partenza del messaggio
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
          ErrExit("message send failed (S2)");
        }
      }

      if(strcmp(internal_msg.m_message.type, "SH") == 0){ //SHARED MEMORY
        semOp(semid, 0, -1);	//entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

        messageSH = address->ptr;
        //printf("MessageSH ADDRESS %p\n", address->ptr);

        //scrivo sulla shared memory il  messaggio
        strcpy(messageSH->id, internal_msg.m_message.id);
        strcpy(messageSH->message, internal_msg.m_message.message);
        strcpy(messageSH->idSender, internal_msg.m_message.idSender);
        strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
        strcpy(messageSH->delS1, internal_msg.m_message.delS1);
        strcpy(messageSH->delS2, internal_msg.m_message.delS2);
        strcpy(messageSH->delS3, internal_msg.m_message.delS3);
        strcpy(messageSH->type, internal_msg.m_message.type);


        messageSH++;
        address->ptr = messageSH;
        //printf("MessageSH ADDRESS %p\n", address->ptr);
        //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
        semOp(semid, 0, 1);
      }

    }

    if(strcmp(internal_msg.m_message.idSender, "S3") == 0){ //se il processo incaricato di trasferire il messaggio al receiver è S3
      //internal_msg.msgFile = get_time_departure(internal_msg.msgFile);  // registriamo il tempo di partenza del messaggio
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
  int fifo;
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

    if(pid > 0){
      //mandare il pid sulla MQ e cambio mtype
      sigInc.mtype = 3;
      sigInc.pid = pid;

      if(msgsnd(mqInc_id, &sigInc, sizeof(struct signal) - sizeof(long), 0) == -1){
        ErrExit("Sending pid to Hackler failed (S3 - INC)");
      }

      sigRmv.mtype = 3;
      sigRmv.pid = pid;

      if(msgsnd(mqRmv_id, &sigRmv, sizeof(struct signal) - sizeof(long), 0) == -1){
        ErrExit("Sending pid to Hackler failed (S3 - RMV)");
      }

      sigSnd.mtype = 3;
      sigSnd.pid = pid;

      if(msgsnd(mqSnd_id, &sigSnd, sizeof(struct signal) - sizeof(long), 0) == -1){
        ErrExit("Sending pid to Hackler failed (S3 - SND)");
      }

    }


    if(pid == 0){ //sono nel figlio
      int sec;

      if((sec = sleep(atoi(message.delS3))) > 0 && wait_time == true){ //il figlio dorme per DelS1 secondi
        sleep(sec);
        *check_time = false;
      }

      internal_msg.m_message = message; //salva il messaggio all'interno del campo specifico della struttura della msgqueue
      internal_msg.mtype = 3; //cambiamo l'mtype per sapere quali messaggi S3 deve leggere dalla msg queue
      internal_msg.msgFile = get_time_departure(internal_msg.msgFile);

      if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
        ErrExit("message send failed (S3 child)");
      }

      exit(0);
    }

    if(msgrcv(mqid, &internal_msg, internal_mSize, 3, IPC_NOWAIT) == -1){ //se non ci sono messaggi con mtype 2 nella msgqueue non fa nulla

    }else{ //altrimenti guardiamo in che modo dobbiamo inviarlo

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
          ErrExit("message send failed (S3)");
        }

      }

      if(strcmp(internal_msg.m_message.type, "SH") == 0){

        semOp(semid, 0, -1);	//entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

        messageSH = address->ptr;
        //printf("MessageSH ADDRESS %p\n", address->ptr);

        //scrivo sulla shared memory il  messaggio
        strcpy(messageSH->id, internal_msg.m_message.id);
        strcpy(messageSH->message, internal_msg.m_message.message);
        strcpy(messageSH->idSender, internal_msg.m_message.idSender);
        strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
        strcpy(messageSH->delS1, internal_msg.m_message.delS1);
        strcpy(messageSH->delS2, internal_msg.m_message.delS2);
        strcpy(messageSH->delS3, internal_msg.m_message.delS3);
        strcpy(messageSH->type, internal_msg.m_message.type);


        messageSH++;
        address->ptr = messageSH;
        //printf("MessageSH ADDRESS %p\n", address->ptr);
        //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
        semOp(semid, 0, 1);
      }

      if(strcmp(internal_msg.m_message.type, "FIFO") == 0 || strcmp(internal_msg.m_message.type, "PIPE") == 0){  //FIFO

        fifo = open("OutputFiles/my_fifo.txt", O_WRONLY | O_NONBLOCK); //apro il file descriptor relativo alla FIFO in sola scrittura

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

      messageSH = address->ptr;
      //printf("MessageSH ADDRESS %p\n", address->ptr);

      //scrivo sulla shared memory il  messaggio
      strcpy(messageSH->id, internal_msg.m_message.id);
      strcpy(messageSH->message, internal_msg.m_message.message);
      strcpy(messageSH->idSender, internal_msg.m_message.idSender);
      strcpy(messageSH->idReceiver, internal_msg.m_message.idReceiver);
      strcpy(messageSH->delS1, internal_msg.m_message.delS1);
      strcpy(messageSH->delS2, internal_msg.m_message.delS2);
      strcpy(messageSH->delS3, internal_msg.m_message.delS3);
      strcpy(messageSH->type, internal_msg.m_message.type);


      messageSH++;
      address->ptr = messageSH;
      //printf("MessageSH ADDRESS %p\n", address->ptr);

      //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
      semOp(semid, 0, 1);
    }

    if(strcmp(internal_msg.m_message.type, "FIFO") == 0 || strcmp(internal_msg.m_message.type, "PIPE") == 0){  //FIFO

      fifo = open("OutputFiles/my_fifo.txt", O_WRONLY | O_NONBLOCK); //apro il file descriptor relativo alla FIFO in sola scrittura

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

//chiusura lato scrittura fifo
close(fifo);

exit(0);  //terminazione S3

  }else if(pid != 0 && pid_S[0] > 0 && pid_S[1] > 0 && pid_S[2] > 0){

  //==============================ESECUZIONE PADRE========================================//

  writeF8(pid_S);  //scrittura del file F8.csv
  semOp(semid3, 0, 1);

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

  historical[0] = get_time(historical[0], 'd');
  historical[1] = get_time(historical[1], 'd');

  messageSH = address->ptr;

  strcpy(messageSH->id, "null");
  strcpy(messageSH->idReceiver, "end");

  ssize_t mSize = sizeof(struct mymsg) - sizeof(long);
  m.mtype = 3;
  strcpy(m.m_message.id, "null");

  if(msgsnd(msqid, &m, mSize ,0) == -1){
    ErrExit("message send failed (S1-R3)");
  }

  m.mtype = 2;
  strcpy(m.m_message.id, "null");

  if(msgsnd(msqid, &m, mSize ,0) == -1){
    ErrExit("message send failed (S1-R2)");
  }

  m.mtype = 1;
  strcpy(m.m_message.id, "null");

  if(msgsnd(msqid, &m, mSize ,0) == -1){
    ErrExit("message send failed (S1-R1)");
  }


  //detach della shared memory
  free_shared_memory(address);
  remove_shared_memory(shmid2);

  historical[2] = get_time(historical[2], 'd');

  //rimozione della msg queue
  if(msgctl(mqid, IPC_RMID, NULL) == -1){
    ErrExit("close of MSG QUEUE beetween Senders and children failed");
  }


  historical[3] = get_time(historical[3], 'd');

  //rimozione del semaforo
  if(semctl(semid, 0, IPC_RMID, 0) == -1)
  ErrExit("semctl failed");

  historical[4] = get_time(historical[4], 'd');

  for(int i = 0; i < 5; i++){
    semOp(semid2, 0, -1);
    writeF10(historical[i], F10);
    semOp(semid2, 0, 1);
  }

  return 0; //terminazione di sender_manager

  }
}
