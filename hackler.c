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

  if(semctl(semid2, 0, GETALL, arg2) == -1){
    ErrExit("semctl failed");
  }

  //====================================================================================

  getcwd(path, PATH_SZ);
  strcat(path, argv[1]); //path contiene il percorso dove risiede F7

  //apro il file da leggere
  int F7 = open(path, O_RDWR, S_IRWXU);

  if(F7 == -1){
    ErrExit("open F7 failed");
  }

  struct pid pids = {};
  printf("before semaphore\n");
  semOp(semid2, 0, 0);
  printf("after semaphore\n");

  //controllare funzione per segmentation fault
  pids = get_pidF8(pids);
  pids = get_pidF9(pids);

  sleep(30);

  printf("kill pid: %d\n", pids.pid_S[0]);
  if(kill(pids.pid_S[0], SIGTERM) == -1){
    ErrExit("Kill failed S1");
  }

  printf("kill pid: %d\n", pids.pid_S[1]);
  if(kill(pids.pid_S[1], SIGTERM) == -1){
    ErrExit("Kill failed S2");
  }

  printf("kill pid: %d\n", pids.pid_S[2]);
  if(kill(pids.pid_S[2], SIGTERM) == -1){
    ErrExit("Kill failed S3");
  }

  printf("kill pid: %d\n", pids.pid_R[0]);
  if(kill(pids.pid_R[0], SIGTERM) == -1){
    ErrExit("Kill failed R1");
  }


  printf("kill pid: %d\n", pids.pid_R[1]);
  if(kill(pids.pid_R[1], SIGTERM) == -1){
    ErrExit("Kill failed R2");
  }

  printf("kill pid: %d\n", pids.pid_R[2]);
  if(kill(pids.pid_R[2], SIGTERM) == -1){
    ErrExit("Kill failed R3");
  }

  return 0;
}

