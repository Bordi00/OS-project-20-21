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

  //===============================================================
  //collegamento al segmento di memoria condivisa

  key_t shmKey = 01101101;
  int shmid;
  shmid = alloc_shared_memory(shmKey, sizeof(struct message));
  struct message *messageSH = (struct message *) get_shared_memory(shmid, SHM_RDONLY);

  //=================================================================================
  //creazione del semaforo che permetterà di scrivere i file F8 e F9 prima di essere letti

  key_t semKey2 = ftok("receiver_manager.c", 'B');
  int semid2 = semget(semKey2, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid2 == -1){
    ErrExit("semget failed SEMAPHORE 2 (SM)");
  }

  unsigned short semInitVal2[1];
  union semun arg2;
  arg2.array = semInitVal2;

  if(semctl(semid2, 0, GETALL, arg2) == -1){
    ErrExit("semctl failed");
  }

  //==================================================================================
  //creazione semaforo apertura della fifo

  key_t semKey3 = ftok("receiver_manager.c", 'C');
  int semid3 = semget(semKey3, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid3 == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal3[1];
  union semun arg3;
  arg3.array = semInitVal3;

  if(semctl(semid3, 0, GETALL, arg3) == -1){
    ErrExit("semctl failed (semid3)");
  }

  //===============================================================
  //collegamento alla fifo in lettura

  int check;
  int fifo;

  unlink("OutputFiles/my_fifo.txt");

  if((check = mkfifo("OutputFiles/my_fifo.txt", S_IRUSR | S_IWUSR)) == -1){
    ErrExit("create fifo failed");
  }
  fifo = open("OutputFiles/my_fifo.txt", O_RDONLY | O_NONBLOCK);

  if(fifo == -1){
    ErrExit("Open fifo in read only mode failed");
  }

  //===============================================================
  //collegamento alla message queue tra Senders e Receivers

  struct mymsg {
      long mtype;
      struct msg message;
  } m;


  key_t msgKey = 01110001;
  int msqid = msgget(msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);

  if (msqid == -1) {
    ErrExit("msgget failed");
  }

  ssize_t mSize = sizeof(struct mymsg) - sizeof(long);


  //=================================================================================
  //creazione pipe e allocazione delle variabili necessarie

  int pipe3[2];
  int pipe4[2];

  create_pipe(pipe3);
  create_pipe(pipe4);

  fcntl(pipe3[0], F_SETFL, O_NONBLOCK);
  fcntl(pipe4[0], F_SETFL, O_NONBLOCK);

  //=================================================================================
  //dichiarazione intestazioni per F4, F5, F6

  int F4 = open("OutputFiles/F4.csv", O_RDWR | O_CREAT, S_IRWXU);
  if (F4 == -1) {
    ErrExit("open F4.csv failed");
  }

  int F5 = open("OutputFiles/F5.csv", O_RDWR | O_CREAT, S_IRWXU);
  if (F5 == -1) {
    ErrExit("open F5.csv failed");
  }

  int F6 = open("OutputFiles/F6.csv", O_RDWR | O_CREAT, S_IRWXU);
  if (F6 == -1) {
    ErrExit("open F6.csv failed");
  }

  const char heading[] = "Id;Message;IDSender;IDReceiver;TimeArrival;TimeDeparture\n";

  pid_t pid_R[3];
  pid_R[0] = 0;
  pid_R[1] = 0;
  pid_R[2] = 0;

  int process = 0;

  pid_t pid = fork();

  if(pid != 0){ //sono nel padre
    pid_R[0] = pid; //salvo pid di R1
  }else if(pid == 0){ //sono in R1
    process = 1;
  }else{
    ErrExit("Error on child R1");
  }

  if(pid != 0){//sono nel padre
    pid = fork(); //creo R2
    if(pid != 0) {
      pid_R[1] = pid; //salvo pid di R2
    }else if(pid == 0){ //sono in R2
      process = 2;
    }else{
      ErrExit("Error on child R2");
    }
  }

  if(pid != 0){//sono nel padre
    pid = fork(); //creo R3
    if(pid != 0) {
      pid_R[2] = pid; //salvo pid di R3
    }else if(pid == 0){ //sono in R3
      process = 3;
    }else{
      ErrExit("Error on child R3");
    }
  }

 if(process == 3){
   ssize_t numWrite;
   ssize_t numRead;
   struct msg message = {};
   struct container msgFile = {};

   if (close(pipe3[0]) == -1) {
     ErrExit("Close of Read hand of pipe4 failed.\n");
   }

   numWrite = write(F4, heading, strlen(heading)); //scriviamo l'intestazione sul file F2.csv
   if (numWrite != strlen(heading)) {
     ErrExit("write F4 failed");
   }
   while(1){
     numRead = read(fifo, &message, sizeof(struct msg));
     if (numRead < sizeof(struct msg) && numRead != 0) {
       printf("%ld\n", numRead);
       ErrExit("Read from fifo failed");
     }
     if (numRead == sizeof(struct msg)) {
       printf("Ho letto qualcosa dalla fifo...\n");
       msgFile = get_time_arrival();

       pid = fork();
       if (pid == -1) {
         ErrExit("fork failed! Child of R3 not created");
       }
       if (pid == 0) {
         int sec;
         if (strcmp(message.delS3, "-") != 0) {
           if ((sec = sleep(atoi(message.delS3))) > 0) { //il figlio dorme per DelS3 secondi
               sleep(sec);
           }
         }

         msgFile = get_time_departure(msgFile);
         writeFile(msgFile, message, F4);

         numWrite = write(pipe3[1], &message, sizeof(struct msg));
         if (numWrite != sizeof(struct msg)) {
           ErrExit("Write on pipe 3 failed");
         }

         exit(0);
       }
     }

     msgFile = init_container(msgFile);
     message = init_msg(message);

     if(msgrcv(msqid, &m, mSize, 3, IPC_NOWAIT) != -1){

       printf("Ho letto qualcosa dalla mq...\n");
       message = m.message;

       msgFile = get_time_arrival();

       pid = fork();
       if(pid == -1){
         ErrExit("fork failed! Child of R3 not created");
       }
       if(pid == 0){
         int sec;
         if(strcmp(message.delS3, "-") != 0) {
           if ((sec = sleep(atoi(message.delS3))) > 0) { //il figlio dorme per DelS1 secondi
             sleep(sec);
           }
         }
         msgFile = get_time_departure(msgFile);
         writeFile(msgFile, message, F4);

         numWrite = write(pipe3[1], &message, sizeof(struct msg));
         if(numWrite != sizeof(struct msg)){
           ErrExit("Write on pipe 3 failed");
         }

         exit(0);
       }
     }

     msgFile = init_container(msgFile);
     message = init_msg(message);

     if(strcmp(messageSH->idReceiver, "R3") == 0) {

       //scrivo sulla shared memory il  messaggio
       strcpy(message.id, messageSH->id);
       strcpy(message.message, messageSH->message);
       strcpy(message.idSender, messageSH->idSender);
       strcpy(message.idReceiver, messageSH->idReceiver);
       strcpy(message.delS1, messageSH->delS1);
       strcpy(message.delS2, messageSH->delS2);
       strcpy(message.delS3, messageSH->delS3);
       strcpy(message.type, messageSH->type);

       messageSH++;
       printf("Ho letto qualcosa dalla sh...\n");
       msgFile = get_time_arrival();



       pid = fork();
       if (pid == -1) {
         ErrExit("fork failed! Child of R3 not created");
       }
       if (pid == 0) {
         int sec;
         if (strcmp(message.delS3, "-") != 0) {
           if ((sec = sleep(atoi(message.delS3))) > 0) { //il figlio dorme per DelS1 secondi
             sleep(sec);
           }
         }
         msgFile = get_time_departure(msgFile);
         writeFile(msgFile, message, F4);

         numWrite = write(pipe3[1], &message, sizeof(struct msg));
         if (numWrite != sizeof(struct msg)) {
           ErrExit("Write on pipe 3 failed");
         }

         exit(0);
       }
     }else if(strcmp(messageSH->idReceiver, "R1") == 0 || strcmp(messageSH->idReceiver, "R2") == 0){
       messageSH++;
     }
   }
 }else if(process == 2){


   ssize_t numWrite;
   ssize_t numRead;
   struct msg message = {};
   struct container msgFile = {};

   if (close(pipe3[1]) == -1) {
     ErrExit("Close of write-end of pipe3 failed.\n");
   }

   if (close(pipe4[0]) == -1) {
     ErrExit("Close of read-end of pipe3 failed.\n");
   }

   numWrite = write(F5, heading, strlen(heading)); //scriviamo l'intestazione sul file F2.csv
   if (numWrite != strlen(heading)) {
     ErrExit("write F5 failed");
   }

   while(1){

     numRead = read(pipe3[0], &message, sizeof(struct msg));
     if(numRead < sizeof(struct msg)){
       ErrExit("Read from pipe3 failed");
     }
     if(numRead == sizeof(struct msg)) {

       msgFile = get_time_arrival();

       pid = fork();
       if (pid == -1) {
         ErrExit("fork failed! Child of R2 not created");
       }
       if (pid == 0) {
         int sec;
         if (strcmp(message.delS2, "-") != 0) {
           if ((sec = sleep(atoi(message.delS2))) > 0) { //il figlio dorme per DelS1 secondi
             sleep(sec);
           }
         }

         msgFile = get_time_departure(msgFile);
         writeFile(msgFile, message, F5);

         numWrite = write(pipe4[1], &message, sizeof(struct msg));
         if (numWrite != sizeof(struct msg)) {
           ErrExit("Write on pipe 4 failed");
         }

         exit(0);
       }
     }

     msgFile = init_container(msgFile);
     message = init_msg(message);

     if(msgrcv(msqid, &m, mSize, 2, IPC_NOWAIT) != -1){
       message = m.message;

       msgFile = get_time_arrival();

       pid = fork();
       if(pid == -1){
         ErrExit("fork failed! Child of R2 not created");
       }
       if(pid == 0){
         int sec;
         if(strcmp(message.delS2, "-") != 0) {
           if ((sec = sleep(atoi(message.delS2))) > 0) { //il figlio dorme per DelS2 secondi
             sleep(sec);
           }
         }
         msgFile = get_time_departure(msgFile);
         writeFile(msgFile, message, F5);

         numWrite = write(pipe4[1], &message, sizeof(struct msg));
         if(numWrite != sizeof(struct msg)){
           ErrExit("Write on pipe 4 failed");
         }

         exit(0);
       }
     }
     msgFile = init_container(msgFile);
     message = init_msg(message);

     if(strcmp(messageSH->idReceiver, "R2") == 0) {

       //scrivo sulla shared memory il  messaggio
       strcpy(message.id, messageSH->id);
       strcpy(message.message, messageSH->message);
       strcpy(message.idSender, messageSH->idSender);
       strcpy(message.idReceiver, messageSH->idReceiver);
       strcpy(message.delS1, messageSH->delS1);
       strcpy(message.delS2, messageSH->delS2);
       strcpy(message.delS3, messageSH->delS3);
       strcpy(message.type, messageSH->type);

       messageSH++;

       msgFile = get_time_arrival();



       pid = fork();
       if (pid == -1) {
         ErrExit("fork failed! Child of R2 not created");
       }
       if (pid == 0) {
         int sec;
         if (strcmp(message.delS2, "-") != 0) {
           if ((sec = sleep(atoi(message.delS2))) > 0) { //il figlio dorme per DelS1 secondi
             sleep(sec);
           }
         }
         msgFile = get_time_departure(msgFile);
         writeFile(msgFile, message, F5);

         numWrite = write(pipe4[1], &message, sizeof(struct msg));
         if (numWrite != sizeof(struct msg)) {
           ErrExit("Write on pipe 4 failed");
         }

         exit(0);
       }
     }else if(strcmp(messageSH->idReceiver, "R1") == 0 || strcmp(messageSH->idReceiver, "R3") == 0){
       messageSH++;
     }
   }

 }else if(process == 1){
   ssize_t numWrite;
   ssize_t numRead;
   struct msg message = {};
   struct container msgFile = {};

   if (close(pipe4[1]) == -1) {
     ErrExit("Close of write-end of pipe4 failed.\n");
   }


   numWrite = write(F6, heading, strlen(heading)); //scriviamo l'intestazione sul file F6.csv
   if (numWrite != strlen(heading)) {
     ErrExit("write F6 failed");
   }

   while(1) {

     numRead = read(pipe4[0], &message, sizeof(struct msg));
     if (numRead < sizeof(struct msg)) {
       ErrExit("Read from pipe4 failed");
     }
     if (numRead == sizeof(struct msg)) {

       msgFile = get_time_arrival();

       pid = fork();
       if (pid == -1) {
         ErrExit("fork failed! Child of R1 not created");
       }
       if (pid == 0) {
         int sec;
         if (strcmp(message.delS1, "-") != 0) {
           if ((sec = sleep(atoi(message.delS1))) > 0) { //il figlio dorme per DelS1 secondi
             sleep(sec);
           }
         }

         msgFile = get_time_departure(msgFile);
         writeFile(msgFile, message, F6);

         exit(0);
       }
     }
     msgFile = init_container(msgFile);
     message = init_msg(message);

     if(msgrcv(msqid, &m, mSize, 1, IPC_NOWAIT) != -1){
       message = m.message;

       msgFile = get_time_arrival();

       pid = fork();
       if(pid == -1){
         ErrExit("fork failed! Child of R1 not created");
       }
       if(pid == 0){
         int sec;
         if(strcmp(message.delS1, "-") != 0) {
           if ((sec = sleep(atoi(message.delS1))) > 0) { //il figlio dorme per DelS1 secondi
             sleep(sec);
           }
         }
         msgFile = get_time_departure(msgFile);
         writeFile(msgFile, message, F6);

         exit(0);
       }
     }
     msgFile = init_container(msgFile);
     message = init_msg(message);

     if(strcmp(messageSH->idReceiver, "R1") == 0) {

       //scrivo sulla shared memory il  messaggio
       strcpy(message.id, messageSH->id);
       strcpy(message.message, messageSH->message);
       strcpy(message.idSender, messageSH->idSender);
       strcpy(message.idReceiver, messageSH->idReceiver);
       strcpy(message.delS1, messageSH->delS1);
       strcpy(message.delS2, messageSH->delS2);
       strcpy(message.delS3, messageSH->delS3);
       strcpy(message.type, messageSH->type);

       messageSH++;

       msgFile = get_time_arrival();

       pid = fork();
       if (pid == -1) {
         ErrExit("fork failed! Child of R1 not created");
       }
       if (pid == 0) {
         int sec;
         if (strcmp(message.delS1, "-") != 0) {
           if ((sec = sleep(atoi(message.delS1))) > 0) { //il figlio dorme per DelS1 secondi
             sleep(sec);
           }
         }
         msgFile = get_time_departure(msgFile);
         writeFile(msgFile, message, F6);

         exit(0);
       }
     }else if(strcmp(messageSH->idReceiver, "R2") == 0 || strcmp(messageSH->idReceiver, "R3") == 0){
       messageSH++;
     }
   }

 }else{

   writeF9(pid_R);
   semOp(semid2, 0, -1);

   int status;

   while((pid = wait(&status)) != -1) {
     if (pid == pid_R[0]) {
       printf("R1 %d exited, status = %d\n", pid, WEXITSTATUS(status)); //qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
     }else if(pid == pid_R[1]){
       printf("R2 %d exited, status = %d\n", pid, WEXITSTATUS(status));
     }else{
       printf("R3 %d exited, status = %d\n", pid, WEXITSTATUS(status));
     }
   }

   if(close(fifo) == -1){
     ErrExit("close fifo failed");
   }

   if(close(pipe3[0]) == -1){
     ErrExit("Close pipe3 write-end failed");
   }

   if(close(pipe3[1]) == -1){
     ErrExit("Close pipe3 write-end failed");
   }

   if(close(F4) == -1){
     ErrExit("Close F4 failed");
   }

   if(close(F5) == -1){
     ErrExit("Close F5 failed");
   }

   if(close(F6) == -1){
     ErrExit("Close F6 failed");
   }

 }
  return 0;
}
