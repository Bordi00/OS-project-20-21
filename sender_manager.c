//
// Created by matteo on 29/05/21.
//

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "pipe.h"
#include <sys/shm.h>
#include <sys/msg.h>
#include <fcntl.h>



int main(int argc, char * argv[]) {

 //===================================================================
 //creazione msg queue

  struct mymsg{
      long mtype;
      struct msg message;
  }m;

  key_t msgKey = 01110001;
  int msqid = msgget(msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(msqid == -1){
    ErrExit("msgget failed");
  }

  ssize_t mSize = sizeof(struct mymsg) - sizeof(long);

 //==================================================================
 //creazione shared memory (SH) tra Senders & Receiver

  key_t shmKey = 01101101;
  int shmid;
  shmid = alloc_shared_memory(shmKey, sizeof(struct message));
  struct message *messageSH = (struct message *)get_shared_memory(shmid, 0);

  //==================================================================
  //creazione shared memory per la condivisione del puntatore

  key_t shmKey2 = ftok("receiver_manager.c", 'A');

  int shmid2;
  shmid2 = alloc_shared_memory(shmKey2, sizeof(struct address));
  struct address *address = (struct address *)get_shared_memory(shmid2, 0);

  address->ptr = messageSH;

  //==================================================================================
  //creazione del semaforo per la scrittura sulla SH

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

  //=================================================================================
  //creazione del semaforo che permetterà di scrivere i file F8 e F9 prima di essere letti

  key_t semKey2 = ftok("receiver_manager.c", 'B');
  int semid2 = semget(semKey2, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid2 == -1){
    ErrExit("semget failed SEMAPHORE 2 (SM)");
  }

  unsigned short semInitVal2[] = {2};
  union semun arg2;
  arg2.array = semInitVal2;

  if(semctl(semid2, 0, SETALL, arg2) == -1){
    ErrExit("semctl failed");
  }

  //==================================================================================
  //creazione semaforo apertura della fifo

  key_t semKey3 = ftok("receiver_manager.c", 'C');
  int semid3 = semget(semKey3, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid3 == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal3[] = {1};
  union semun arg3;
  arg3.array = semInitVal3;

  if(semctl(semid3, 0, SETALL, arg3) == -1){
    ErrExit("semctl failed (semid3)");
  }

  //=================================================================================
  //creazione fifo

  int fifo;

  do{
    fifo = open("OutputFiles/my_fifo.txt", O_WRONLY);
  }while(fifo == -1);

  //=================================================================================
  //creazione pipe e allocazione delle variabili necessarie

  int pipe1[2];
  int pipe2[2];

  create_pipe(pipe1);
  create_pipe(pipe2);

  fcntl(pipe1[0], F_SETFL, O_NONBLOCK);
  fcntl(pipe2[0], F_SETFL, O_NONBLOCK);


  pid_t pid_S[3]; //@param contiene i pid dei processi
  pid_S[0] = 0;
  pid_S[1] = 0;
  pid_S[2] = 0;
  int process = 0;


  int F1 = open("OutputFiles/F1.csv", O_RDWR | O_CREAT, S_IRWXU); //creiamo F1.csv con permessi di lettura scrittura

  if(F1 == -1){
    ErrExit("open F1 failed");
  }

  int F2 = open("OutputFiles/F2.csv", O_RDWR | O_CREAT, S_IRWXU); //creiamo F1.csv con permessi di lettura scrittura

  if(F2 == -1){
    ErrExit("open F2 failed");
  }

  int F3 = open("OutputFiles/F3.csv", O_RDWR | O_CREAT, S_IRWXU); //creiamo F3.csv con permessi di lettura scrittura

  if(F3 == -1){
    ErrExit("open F3 failed");
  }


  //intestazione dei file F1, F2, F3
  const char heading[] = "Id;Message;IDSender;IDReceiver;TimeArrival;TimeDeparture\n";

  pid_t pid;
  pid = fork(); //creo S1

  if(pid != 0){ //sono nel padre
    pid_S[0] = pid; //salvo pid di S1
  }else if(pid == 0){ //sono in S1
    process = 1;
  }else{
    ErrExit("Error on child S1");
  }

  if(pid != 0){//sono nel padre
    pid = fork(); //creo S2
    if(pid != 0) {
      pid_S[1] = pid; //salvo pid di S2
    }else if(pid == 0){ //sono in S2
      process = 2;
    }else{
      ErrExit("Error on child S2");
    }
  }

  if(pid != 0){//sono nel padre
    pid = fork(); //creo S3
    if(pid != 0) {
      pid_S[2] = pid; //salvo pid di S3
    }else if(pid == 0){ //sono in S3
      process = 3;
    }else{
      ErrExit("Error on child S3");
    }
  }


  if(process == 1){
    //dichiarazione variabili di S1;
    ssize_t numRead;
    ssize_t numWrite;
    ssize_t nBys;
    struct container msgFile = {};
    struct msg message = {};
    char tmp;
    int i = 0;

    if(close(pipe1[0]) == -1){
      ErrExit("Close pipe1 read-end failed");
    }

    getcwd(path, PATH_SZ);
    strcat(path, argv[1]);

    int F0 = open(path, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH); //apriamo F0.csv in sola lettura

    if(F0 == -1){
      ErrExit("open F0 failed");
    }

    lseek(F0, strlen(heading) - 3, SEEK_CUR); //ci spostiamo dopo l'intestazione (heading)

    numWrite = write(F1, heading, strlen(heading));

    if(numWrite != strlen(heading)){
      ErrExit("write F1 failed");
    }

    while(1){

      numRead = read(F0, &tmp, sizeof(char));

      if(numRead > 0) {
        if (tmp != '\n') {
          buffer[i] = tmp;
          i++;
        } else {
          buffer[i] = '\0';

          i = 0;
          msgFile = get_time_arrival();
          message = fill_structure(buffer);
          /*
            printf("id %s\n", message.id);
            printf("message %s\n", message.message);
            printf("idSender %s\n", message.idSender);
            printf("idReceiver %s\n", message.idReceiver);
            printf("delS1 %s\n", message.delS1);
            printf("delS2 %s\n", message.delS2);
            printf("delS3 %s\n", message.delS3);
          */

          pid = fork();

          if (pid == -1) {
            ErrExit("Fork failed! Child of S1 not created");
          }
          if (pid == 0) {
            int sec;

            if ((sec = sleep(atoi(message.delS1))) > 0) {
              sleep(sec);
            }

            msgFile = get_time_departure(msgFile);
            writeFile(msgFile, message, F1);

            if (strcmp(message.idSender, "S1") == 0) {

              if (strcmp(message.type, "Q") == 0) {
                m.message = message;

                if (strcmp(message.idReceiver, "R1") == 0) {
                  m.mtype = 1;
                } else if (strcmp(message.idReceiver, "R2") == 0) {
                  m.mtype = 2;
                } else if (strcmp(message.idReceiver, "R3") == 0) {
                  m.mtype = 3;
                }

                if (msgsnd(msqid, &m, mSize, 0) == -1) {
                  ErrExit("Message Send Failed[S1]");
                }
              }

              if (strcmp(message.type, "SH") == 0) {
                semOp(semid, 0, -1);  //entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

                messageSH = address->ptr;


                //scrivo sulla shared memory il  messaggio
                strcpy(messageSH->id, message.id);
                strcpy(messageSH->message, message.message);
                strcpy(messageSH->idSender, message.idSender);
                strcpy(messageSH->idReceiver, message.idReceiver);
                strcpy(messageSH->delS1, message.delS1);
                strcpy(messageSH->delS2, message.delS2);
                strcpy(messageSH->delS3, message.delS3);
                strcpy(messageSH->type, message.type);


                messageSH++;
                address->ptr = messageSH;

                //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
                semOp(semid, 0, 1);

              }
            } else{
              nBys = write(pipe1[1], &message, sizeof(message));
              if (nBys != sizeof(message)) {
                ErrExit("write to pipe1 failed");
              }
            }

            exit(0);
          }

        }
      }
    }
    while(wait(NULL) != -1);


    exit(0);
  }else if(process == 2){

    if(close(pipe1[1]) == -1){
      ErrExit("Close pipe1 write-end failed");
    }
    if(close(pipe2[0]) == -1){
      ErrExit("Close pipe2 read-end failed");
    }

    ssize_t numRead;
    ssize_t numWrite;
    ssize_t nBys;
    struct container msgFile = {};
    struct msg message;

    numWrite = write(F2, heading, strlen(heading));

    if(numWrite != strlen(heading)){
      ErrExit("write F2 failed");
    }

    while(1){
      numRead = read(pipe1[0], &message, sizeof(struct msg));
      if(numRead > 0) {
        if (strcmp(message.id, "-1") == 0) {
          break;
        }

        if (numRead == sizeof(struct msg)) {
          msgFile = get_time_arrival();
          pid = fork();

          if (pid == -1) {
            ErrExit("Fork failed! Child of S2 not created");
          }
          if (pid == 0) {
            int sec;

            if ((sec = sleep(atoi(message.delS2))) > 0) {
              sleep(sec);
            }

            msgFile = get_time_departure(msgFile);
            writeFile(msgFile, message, F2);

            if (strcmp(message.idSender, "S2") == 0) {

              if (strcmp(message.type, "Q") == 0) {
                m.message = message;

                if (strcmp(message.idReceiver, "R1") == 0) {
                  m.mtype = 1;
                } else if (strcmp(message.idReceiver, "R2") == 0) {
                  m.mtype = 2;
                } else if (strcmp(message.idReceiver, "R3") == 0) {
                  m.mtype = 3;
                }

                if (msgsnd(msqid, &m, mSize, 0) == -1) {
                  ErrExit("Message Send Failed[S2]");
                }
              }

              if (strcmp(message.type, "SH") == 0) {
                semOp(semid, 0, -1);  //entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

                messageSH = address->ptr;


                //scrivo sulla shared memory il  messaggio
                strcpy(messageSH->id, message.id);
                strcpy(messageSH->message, message.message);
                strcpy(messageSH->idSender, message.idSender);
                strcpy(messageSH->idReceiver, message.idReceiver);
                strcpy(messageSH->delS1, message.delS1);
                strcpy(messageSH->delS2, message.delS2);
                strcpy(messageSH->delS3, message.delS3);
                strcpy(messageSH->type, message.type);


                messageSH++;
                address->ptr = messageSH;

                //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
                semOp(semid, 0, 1);

              }
            } else if (strcmp(message.idSender, "S3") == 0) {
              nBys = write(pipe2[1], &message, sizeof(message));
              if (nBys != sizeof(message)) {
                ErrExit("write to pipe2 failed");
              }
            }
            exit(0);
          }
        } else {
          ErrExit("Read from pipe1 failed");
        }
      }
    }

    while(wait(NULL) != -1);


    exit(0);
  }else if(process == 3){

    if(close(pipe2[1]) == -1){
      ErrExit("Close pipe2 write-end failed");
    }

    ssize_t numRead;
    ssize_t numWrite;
    ssize_t nBys;
    struct container msgFile = {};
    struct msg message;



    numWrite = write(F3, heading, strlen(heading));

    if(numWrite != strlen(heading)){
      ErrExit("write F3 failed");
    }

    while(1) {
      numRead = read(pipe2[0], &message, sizeof(struct msg));

      if(numRead > 0) {
        if (strcmp(message.id, "-1") == 0) {
          break;
        }
        if (numRead == sizeof(struct msg)) {
          msgFile = get_time_arrival();
          pid = fork();

          if (pid == -1) {
            ErrExit("Fork failed! Child of S2 not created");
          }
          if (pid == 0) {
            int sec;

            if ((sec = sleep(atoi(message.delS3))) > 0) {
              sleep(sec);
            }

            msgFile = get_time_departure(msgFile);
            writeFile(msgFile, message, F3);

            if (strcmp(message.type, "Q") == 0) {
              m.message = message;

              if (strcmp(message.idReceiver, "R1") == 0) {
                m.mtype = 1;
              } else if (strcmp(message.idReceiver, "R2") == 0) {
                m.mtype = 2;
              } else if (strcmp(message.idReceiver, "R3") == 0) {
                m.mtype = 3;
              }

              if (msgsnd(msqid, &m, mSize, 0) == -1) {
                ErrExit("Message Send Failed[S3]");
              }
            } else if (strcmp(message.type, "SH") == 0) {
              semOp(semid, 0, -1);  //entro nella sezione critica se il semaforo è a 1 altrimenti aspetto

              messageSH = address->ptr;


              //scrivo sulla shared memory il  messaggio
              strcpy(messageSH->id, message.id);
              strcpy(messageSH->message, message.message);
              strcpy(messageSH->idSender, message.idSender);
              strcpy(messageSH->idReceiver, message.idReceiver);
              strcpy(messageSH->delS1, message.delS1);
              strcpy(messageSH->delS2, message.delS2);
              strcpy(messageSH->delS3, message.delS3);
              strcpy(messageSH->type, message.type);


              messageSH++;
              address->ptr = messageSH;

              //esco dalla sezione critica e rimetto il semaforo a 1 (libero)
              semOp(semid, 0, 1);
            } else {
              ssize_t numWrite = write(fifo, &message, sizeof(struct msg));
              if(numWrite != sizeof(struct msg)){
                ErrExit("Write on fifo failed");
              }
            }
            exit(0);
          }
        }
      }
    }
    while(wait(NULL) != -1);

    exit(0);

  }else{

    writeF8(pid_S);
    semOp(semid2, 0, -1);

    int status;

    while((pid = wait(&status)) != -1) {
      if (pid == pid_S[0]) {
        printf("S1 %d exited, status = %d\n", pid, WEXITSTATUS(status)); //qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
      }else if(pid == pid_S[1]){
        printf("S2 %d exited, status = %d\n", pid, WEXITSTATUS(status));
      }else{
        printf("S3 %d exited, status = %d\n", pid, WEXITSTATUS(status));
      }
    }

  }

  if(close(pipe1[0]) == -1){
    ErrExit("Close pipe1 read-end failed");
  }

  if(close(pipe1[1]) == -1){
    ErrExit("Close pipe1 write-end failed");
  }

  if(close(pipe2[0]) == -1){
    ErrExit("Close pipe2 read-end failed");
  }

  if(close(pipe2[1]) == -1){
    ErrExit("Close pipe2 write-end failed");
  }

  if(close(F1) == -1){
    ErrExit("Close F1 failed");
  }

  if(close(F2) == -1){
    ErrExit("Close F2 failed");
  }

  if(close(F3) == -1){
    ErrExit("Close F3 failed");
  }


  if(msgctl(msqid, 0, IPC_RMID) == -1){
    ErrExit("msgctl failed");
  }

  if(semctl(semid, 0, IPC_RMID) == -1){
    ErrExit("semctl failed");
  }

  if(semctl(semid2, 0, IPC_RMID) == -1){
    ErrExit("semctl failed");
  }

  free_shared_memory(messageSH);
  free_shared_memory(address);
  remove_shared_memory(shmid);
  remove_shared_memory(shmid2);

  if(close(fifo) == -1){
    ErrExit("Close fifo failed by sender");
  }

  remove_fifo("OutputFiles/my_fifo.txt", fifo);

  return 0;
}

