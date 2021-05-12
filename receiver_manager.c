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

  //==================================================================================
  //creazione dei semafori

  key_t semKey = 01110011;
  int semid = semget(semKey, 2, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid == -1){
    ErrExit("semget failed Reicever");
  }

  unsigned short semInitVal[1];
  union semun arg;
  arg.array = semInitVal;

  if(semctl(semid, 0, GETALL, arg) == -1){
    ErrExit("semctl failed Receiver");
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

  if(semctl(semid2, 0, GETALL, arg2) == -1){
    ErrExit("semctl failed");
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
  struct msg *messageSH = (struct msg *) get_shared_memory(shmid, SHM_RDONLY);

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


  //==================================================================================
  //creazione della message queue tra Receiver e i loro figli

  struct child {
    long mtype;
    struct msg m_message;
    struct container msgFile;
  } internal_msg;

  internal_msg.mtype = 1;

  key_t mqKey = ftok("receiver_manager.c", 'D');
  int mqid = msgget(mqKey, IPC_CREAT | S_IRUSR | S_IWUSR);

  if (mqid == -1) {
    ErrExit("internal msgget failed");
  }

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

    //apro il file descriptor relativo alla FIFO in sola lettura

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

    int fifo = open("OutputFiles/my_fifo.txt", O_RDONLY);
    ssize_t nBys;

    while ((nBys = read(fifo, &internal_msg.m_message, sizeof(struct msg))) > 0){

      strcpy(internal_msg.msgFile.time_arrival, "");
      strcpy(internal_msg.msgFile.time_departure, "");

      printf("%s %s\n", internal_msg.m_message.message, internal_msg.m_message.id);

    } //esce dal while

    remove_fifo("OutputFiles/my_fifo.txt", fifo);

    ssize_t mSize = sizeof(struct mymsg) - sizeof(long);

    while (1) {
      if(msgrcv(msqid, &m, mSize, 3, IPC_NOWAIT) == -1){
        if(errno == ENOMSG){
          break;
        }else{
          ErrExit("MsgReceive failed\n");
        }
      }else{
        printf("message %s arrived in RM via MQ\n", m.m_message.message);
      }
    }

    bool finish = false;

    while(finish == false){
      printf("ready for message\n");
      semOp(semid2, 0, -1);
      printSemaphoresValue(semid);
      if(strcmp(messageSH->id, "-1") == 0){
        finish = true;
      }else{
        printf("message %s with id %s arrived in RM via SH\n", messageSH->message,
        messageSH->id);
      }
      semOp(semid, 0, 1);
      printSemaphoresValue(semid);
    }

  }else if(pid != 0 && pid_R[0] > 0 && pid_R[1] > 0 && pid_R[2] > 0){
    int status;
    int i = 0;

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

    //rimozione del semaforo
    if(semctl(semid, 0, IPC_RMID, 0) == -1)
      ErrExit("semctl failed");

  }
}
