/// @file receiver_manager.c
/// @brief Contiene l'implementazione del receiver_manager.

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
  //dichiarazione e inizializzazione dell'array che conterrà i pid dei processi  R1, R2, R3

  int pid_R[3];
  pid_R[0] = 0;
  pid_R[1] = 0;
  pid_R[2] = 0;

  //=================================================================================
  //dichiarazione intestazioni per F4, F5, F6

  const char heading[] = "Id;Message;IDSender;IDReceiver;TimeArrival;TimeDeparture\n";

  //=================================================================================
  //creazione pipe e allocazione delle variabili necessarie

  int pipe3[2];
  int pipe4[2];

  create_pipe(pipe3);
  create_pipe(pipe4);

  //=================================================================================
  // collegamento alla shared memory creata da sender_manager

  key_t shmKey = 01101101;
  int shmid;
  shmid = alloc_shared_memory(shmKey, sizeof(struct message));
  struct message *messageSH = (struct message *) get_shared_memory(shmid, SHM_RDONLY);

  //==================================================================================
  //collegamento alla message queue tra Senders e Receivers

  struct mymsg {
    long mtype;
    struct msg m_message;
  } m;


  key_t msgKey = 01110001;
  int msqid = msgget(msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);

  if (msqid == -1) {
    ErrExit("msgget failed");
  }

  ssize_t mSize = sizeof(struct mymsg) - sizeof(long);

  //==================================================================================
  //creazione della message queue tra Receiver e i loro figli

  struct child {
    long mtype;
    struct msg m_message;
    struct container msgFile;
  }internal_msg;

  internal_msg.mtype = 1;

  key_t mqKey = ftok("receiver_manager.c", 'D');
  int mqid = msgget(mqKey, IPC_CREAT | S_IRUSR | S_IWUSR);

  if (mqid == -1) {
    ErrExit("internal msgget failed");
  }

  ssize_t internal_mSize = sizeof(struct child) - sizeof(long);
  //===================================================================
  //generazione di R1, R2, R3

  int pid;
  pid = fork(); //creo R1

  if (pid != 0) { //sono nel padre
    pid_R[0] = pid; //salvo pid di R1
  } else if (pid == 0) { //sono in R1
    pid_R[0] = getpid(); //inizializzo
    pid_R[1] = 0; //inizializzo
    pid_R[2] = 0; //inizializzo
  } else {
    ErrExit("Error on child R1");
  }

  if (pid != 0) { //sono nel padre
    pid = fork(); //creo R2

    if (pid == 0) { //sono in R2
      pid_R[0] = 0; //inizializzo
      pid_R[1] = getpid();
      pid_R[2] = 0;
    }

    if (pid != 0) { //sono nel padre
      pid_R[1] = pid; //salvo pid di R2
    }
  }

  if (pid != 0) {
    pid = fork();   //creo R3

    if (pid == 0) { //sono in R3
      pid_R[0] = 0; //inizializzo
      pid_R[1] = 0;
      pid_R[2] = getpid();
    }

    if (pid != 0) { //sono nel padre
      pid_R[2] = pid; //salvo pid di R2
    }
  }


  //==============================ESECUZIONE R3 ========================================//

  if (pid == 0 && pid_R[0] == 0 && pid_R[1] == 0 && pid_R[2] > 0) {

    if (close(pipe3[0]) == -1) {
      ErrExit("Close of Read hand of pipe2 failed.\n");
    }

    int F4 = open("OutputFiles/F4.csv", O_RDWR | O_CREAT, S_IRWXU);
    if (F4 == -1) {
      ErrExit("open F4.csv failed");
    }

    ssize_t numWrite = write(F4, heading, strlen(heading)); //scriviamo l'intestazione sul file F2.csv
    if (numWrite != strlen(heading)) {
      ErrExit("write F2 failed");
    }

    int fifo = open("OutputFiles/my_fifo.txt", O_RDONLY | O_NONBLOCK);
    ssize_t nBys;
    bool conditions[3] = {false};


    while (conditions[0] == false || conditions[1] == false || conditions[2] == false){

      internal_msg.msgFile = init_container(internal_msg.msgFile);
      internal_msg.m_message = init_msg(internal_msg.m_message);

      //========================FIFO=============================//
      nBys = read(fifo, &internal_msg.m_message, sizeof(struct msg));
      if(nBys > 0){
        printf("message %s with id %s arrived in RM via FIFO\n", internal_msg.m_message.message, internal_msg.m_message.id);
        internal_msg.msgFile = get_time_arrival(internal_msg.msgFile);

        pid = fork();

        if(pid == 0){
          sleep(atoi(internal_msg.m_message.delS3));
          internal_msg.mtype = 3;

          internal_msg.msgFile = get_time_departure(internal_msg.msgFile);
          if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
            ErrExit("message send failed (R3 child)");
          }

          exit(0);
        }
      }else if(nBys == 0){
        conditions[0] = true;
      }

      internal_msg.msgFile = init_container(internal_msg.msgFile);
      internal_msg.m_message = init_msg(internal_msg.m_message);

      //========================MSQ=============================//
      if(msgrcv(msqid, &m, mSize, 3, IPC_NOWAIT) == -1){

      }else{
        if(strcmp(m.m_message.id, "null") == 0){
          conditions[1] = true;
        }else{
          printf("message %s arrived in RM via MQ\n", m.m_message.message);
          internal_msg.msgFile = get_time_arrival(internal_msg.msgFile);

          pid = fork();

          if(pid == 0){
            internal_msg.m_message = m.m_message;
            if(strcmp(internal_msg.m_message.delS3, "-") != 0)
              sleep(atoi(internal_msg.m_message.delS3));

            internal_msg.msgFile = get_time_departure(internal_msg.msgFile);
            internal_msg.mtype = 3;

            if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
              ErrExit("message send failed (R3 child)");
            }

            exit(0);
          }
        }
      }

      internal_msg.msgFile = init_container(internal_msg.msgFile);
      internal_msg.m_message = init_msg(internal_msg.m_message);

      //========================SHARED MEMORY=============================//
      if(strcmp(messageSH->idReceiver, "R1") != 0 && strcmp(messageSH->idReceiver, "R2") != 0 && strcmp(messageSH->idReceiver, "R3") != 0){
      }else{
        if(strcmp(messageSH->idReceiver, "R3") == 0){
        //scrivo sulla shared memory il  messaggio
        strcpy(internal_msg.m_message.id, messageSH->id);
        strcpy(internal_msg.m_message.message, messageSH->message);
        strcpy(internal_msg.m_message.idSender, messageSH->idSender);
        strcpy(internal_msg.m_message.idReceiver, messageSH->idReceiver);
        strcpy(internal_msg.m_message.delS1, messageSH->delS1);
        strcpy(internal_msg.m_message.delS2, messageSH->delS2);
        strcpy(internal_msg.m_message.delS3, messageSH->delS3);
        strcpy(internal_msg.m_message.type, messageSH->type);

        internal_msg.msgFile = get_time_arrival(internal_msg.msgFile);

        pid = fork();

        if(pid == 0){
          if(strcmp(internal_msg.m_message.delS3, "-") != 0)
            sleep(atoi(internal_msg.m_message.delS3));

          internal_msg.msgFile = get_time_departure(internal_msg.msgFile);
          internal_msg.mtype = 3;

          if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
            ErrExit("message send failed (R3 child)");
          }

          exit(0);
        }
      }
      messageSH++;

      printf("message %s read by Receiver in SH\n", internal_msg.m_message.message);
      }

      if(strcmp(messageSH->id, "null") == 0){
        conditions[2] = true;
      }

      if(msgrcv(mqid, &internal_msg, internal_mSize, 3, IPC_NOWAIT) == -1){ //se non c'è nessun messaggio da leggere allora non facciamo nulla

      }else{
        //internal_msg.msgFile = get_time_departure(internal_msg.msgFile);  // registriamo il tempo di partenza del messaggio
        writeFile(internal_msg.msgFile, internal_msg.m_message, F4);	//scrittura messaggio su F3

        numWrite = write(pipe3[1], &internal_msg.m_message, sizeof(internal_msg.m_message));

        if(numWrite != sizeof(internal_msg.m_message)){
          ErrExit("Write on pipe3 failed");
        }
      }

    } //esce dal while

    while(wait(NULL) != -1){
      if(msgrcv(mqid, &internal_msg, internal_mSize, 3, IPC_NOWAIT) == -1){ //se non c'è nessun messaggio da leggere allora non facciamo nulla

      }else{
        //internal_msg.msgFile = get_time_departure(internal_msg.msgFile);  // registriamo il tempo di partenza del messaggio
        writeFile(internal_msg.msgFile, internal_msg.m_message, F4);	//scrittura messaggio su F3

        numWrite = write(pipe3[1], &internal_msg.m_message, sizeof(internal_msg.m_message));

        if(numWrite != sizeof(internal_msg.m_message)){
          ErrExit("Write on pipe3 failed");
        }
      }
    }

    struct msg final_msg = {"-1", "", "", "", "", "", "", ""};

    numWrite = write(pipe3[1], &final_msg, sizeof(struct msg));

    if(close(pipe3[1]) == -1){  //chiusura del canale del canale di scrittura
      ErrExit("Close of Write end of pipe3 failed (R3)");
    }

    remove_fifo("OutputFiles/my_fifo.txt", fifo);

    exit(0);

    //========================== R2 ============================//
  }else if (pid == 0 && pid_R[0] == 0 && pid_R[1] > 0 && pid_R[2] == 0) {

    if (close(pipe3[1]) == -1) {
      ErrExit("Close of Write end of pipe3 failed (R2).\n");
    }

    if (close(pipe4[0]) == -1) {
      ErrExit("Close of Read end of pipe4 failed (R2).\n");
    }

    int F5 = open("OutputFiles/F5.csv", O_RDWR | O_CREAT, S_IRWXU);
    if (F5 == -1) {
      ErrExit("open F5.csv failed");
    }

    ssize_t numWrite = write(F5, heading, strlen(heading)); //scriviamo l'intestazione sul file F2.csv

    if (numWrite != strlen(heading)) {
      ErrExit("write F5 failed");
    }

    bool conditions[3] = {false};


    while(conditions[0] == false || conditions[1] == false || conditions[2] == false){

      internal_msg.msgFile = init_container(internal_msg.msgFile);
      internal_msg.m_message = init_msg(internal_msg.m_message);

      ssize_t nBys = read(pipe3[0], &internal_msg.m_message, sizeof(internal_msg.m_message));

      if(nBys > 0){
          if(strcmp(internal_msg.m_message.id, "-1") == 0){
            //printf("Pipe receive msg -1\n");
            conditions[0] = true;
          }else{
            internal_msg.msgFile = get_time_arrival(internal_msg.msgFile);

            pid = fork();

            if(pid == 0){
              if(strcmp(internal_msg.m_message.delS2, "-") != 0)
                sleep(atoi(internal_msg.m_message.delS2));

              internal_msg.msgFile = get_time_departure(internal_msg.msgFile);
              internal_msg.mtype = 2;

              if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
                ErrExit("message send failed (R2 child)");
              }

              exit(0);
            }

          }
      }

      internal_msg.msgFile = init_container(internal_msg.msgFile);
      internal_msg.m_message = init_msg(internal_msg.m_message);

      if(msgrcv(msqid, &m, mSize, 2, IPC_NOWAIT) == -1){

      }else{

        if(strcmp(m.m_message.id, "null") == 0){
          //printf("MSQ receive msg null\n");
          conditions[1] = true;
        }else{
          //printf("message %s arrived in RM via MQ\n", m.m_message.message);
          internal_msg.msgFile = get_time_arrival(internal_msg.msgFile);

          pid = fork();

          if(pid == 0){
            internal_msg.m_message = m.m_message;
            if(strcmp(internal_msg.m_message.delS2, "-") != 0)
              sleep(atoi(internal_msg.m_message.delS2));

            internal_msg.msgFile = get_time_departure(internal_msg.msgFile);
            internal_msg.mtype = 2;

            if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
              ErrExit("message send failed (R2 child)");
            }

            exit(0);
          }
        }
      }

      internal_msg.msgFile = init_container(internal_msg.msgFile);
      internal_msg.m_message = init_msg(internal_msg.m_message);

      if(strcmp(messageSH->idReceiver, "R1") != 0 && strcmp(messageSH->idReceiver, "R2") != 0 && strcmp(messageSH->idReceiver, "R3") != 0){

      }else{
        if(strcmp(messageSH->idReceiver, "R2") == 0){
        //scrivo sulla shared memory il  messaggio
        strcpy(internal_msg.m_message.id, messageSH->id);
        strcpy(internal_msg.m_message.message, messageSH->message);
        strcpy(internal_msg.m_message.idSender, messageSH->idSender);
        strcpy(internal_msg.m_message.idReceiver, messageSH->idReceiver);
        strcpy(internal_msg.m_message.delS1, messageSH->delS1);
        strcpy(internal_msg.m_message.delS2, messageSH->delS2);
        strcpy(internal_msg.m_message.delS3, messageSH->delS3);
        strcpy(internal_msg.m_message.type, messageSH->type);

        internal_msg.msgFile = get_time_arrival(internal_msg.msgFile);

        pid = fork();

        if(pid == 0){
          if(strcmp(internal_msg.m_message.delS2, "-") != 0)
            sleep(atoi(internal_msg.m_message.delS2));

          internal_msg.msgFile = get_time_departure(internal_msg.msgFile);
          internal_msg.mtype = 2;

          if(msgsnd(mqid, &internal_msg, internal_mSize ,0) == -1){ //manda il messaggio sulla msgqueue
            ErrExit("message send failed (R3 child)");
          }

          exit(0);
        }
      }
      messageSH++;
      }

      if(strcmp(messageSH->id, "null") == 0){
        conditions[2] = true;
      }

      if(msgrcv(mqid, &internal_msg, internal_mSize, 2, IPC_NOWAIT) == -1){ //se non c'è nessun messaggio da leggere allora non facciamo nulla

      }else{
        //internal_msg.msgFile = get_time_departure(internal_msg.msgFile);  // registriamo il tempo di partenza del messaggio
        writeFile(internal_msg.msgFile, internal_msg.m_message, F5);	//scrittura messaggio su F3

        numWrite = write(pipe4[1], &internal_msg.m_message, sizeof(internal_msg.m_message));

        if(numWrite != sizeof(internal_msg.m_message)){
          ErrExit("Write on pipe4 failed");
        }
      }
    }

    while(wait(NULL) != -1){
      if(msgrcv(mqid, &internal_msg, internal_mSize, 2, IPC_NOWAIT) == -1){ //se non c'è nessun messaggio da leggere allora non facciamo nulla

      }else{
        //internal_msg.msgFile = get_time_departure(internal_msg.msgFile);  // registriamo il tempo di partenza del messaggio
        writeFile(internal_msg.msgFile, internal_msg.m_message, F5);	//scrittura messaggio su F3

        numWrite = write(pipe4[1], &internal_msg.m_message, sizeof(internal_msg.m_message));

        if(numWrite != sizeof(internal_msg.m_message)){
          ErrExit("Write on pipe4 failed");
        }
      }
    }

    struct msg final_msg = {"-1", "", "", "", "", "", "", ""};

    numWrite = write(pipe4[1], &final_msg, sizeof(struct msg));

    if(close(pipe4[1]) == -1){  //chiusura del canale del canale di scrittura
      ErrExit("Close of Write end of pipe4 failed (R2)");
    }

    if(close(pipe3[0]) == -1){  //chiusura del canale del canale di scrittura
      ErrExit("Close of Read end of pipe3 failed (R2)");
    }

    exit(0);

  }else if(pid != 0 && pid_R[0] > 0 && pid_R[1] > 0 && pid_R[2] > 0){
    int status;
    int i = 0;

    writeF9(pid_R);

    while((pid = wait(&status)) != -1){
      printf("R%i %d exited, status = %d\n", i + 1, pid_R[i], WEXITSTATUS(status)); //qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
      i++;
    }

    free_shared_memory(messageSH);
    remove_shared_memory(shmid);

    //rimozione della msg queue
    if(msgctl(msqid, IPC_RMID, NULL) == -1){
      ErrExit("close of MSG QUEUE beetween Senders and Receivers failed");
    }

    //rimozione della msg queue
    if(msgctl(mqid, IPC_RMID, NULL) == -1){
      ErrExit("close of MSG QUEUE beetween Receivers and children failed");
    }

  }
}
