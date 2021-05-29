/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"
#include "semaphore.h"
#include <sys/msg.h>

int main(int argc, char * argv[]){

  // set of signals (N.B. it is not initialized!)
  sigset_t mySet, prevSet;
  // initialize mySet to contain all signals
  sigfillset(&mySet);
  // remove SIGINT from mySet
  sigdelset(&mySet, SIGTERM);
  sigdelset(&mySet, SIGUSR1);
  sigdelset(&mySet, SIGCONT);
  // blocking all signals but SIGTERM, SIGUSR1, SIGCONT
  sigprocmask(SIG_SETMASK, &mySet, NULL);

  struct ipc historical[8] = {};

  //su F10 scriviamo lo storico delle IPC utilizzate
  int F10 = open("OutputFiles/F10.csv", O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR);

  if(F10 == -1){
    ErrExit("Open F10 failed\n");
  }

  //==================================================================================
  //creazione della message queue tra Sender e Hackler

  struct signal sigInc;

  key_t mqInc_Key = ftok("receiver_manager.c", 'H');
  int mqInc_id = msgget(mqInc_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqInc_id == -1){
    ErrExit("internal msgget failed");
  }


  sprintf(historical[0].idKey, "%x", mqInc_Key);
  strcpy(historical[0].ipc, "MQINC");
  strcpy(historical[0].creator, "SM");
  historical[0] = get_time(historical[0], 'c');

  struct signal sigRmv; //MQ per RemoveMSG

  sigRmv.mtype = 1;

  key_t mqRmv_Key = ftok("fifo.c", 'A');
  int mqRmv_id = msgget(mqRmv_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqRmv_id == -1){
    ErrExit("internal msgget failed");
  }

  sprintf(historical[1].idKey, "%x", mqRmv_Key);
  strcpy(historical[1].ipc, "MQRMV");
  strcpy(historical[1].creator, "SM");
  historical[1] = get_time(historical[1], 'c');

  struct signal sigSnd; //MQ per SendMSG

  sigSnd.mtype = 1;

  key_t mqSnd_Key = ftok("fifo.c", 'B');
  int mqSnd_id = msgget(mqSnd_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqSnd_id == -1){
    ErrExit("internal msgget failed");
  }

  sprintf(historical[2].idKey, "%x", mqSnd_Key);
  strcpy(historical[2].ipc, "MQSND");
  strcpy(historical[2].creator, "SM");
  historical[2] = get_time(historical[2], 'c');

  //==================================================================================
  //creazione dei semafori per la scrittura di F8

  key_t semKey3 = ftok("defines.c", 'E');
  int semid3;

  do{
    semid3 = semget(semKey3, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
  }while(semid3 == -1);

  if(semid3 == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal2[1];
  union semun arg2;
  arg2.array = semInitVal2;

  if(semctl(semid3, 0, GETALL, arg2) == -1){
    ErrExit("semctl failed (semid3)");
  }

  sprintf(historical[3].idKey, "%x", semKey3);
  strcpy(historical[3].ipc, "SEMAPHORE");
  strcpy(historical[3].creator, "SM");
  historical[3] = get_time(historical[3], 'c');

  //==================================================================================
  //creazione dei semafori per la scrittura di F9

  key_t semKey9 = ftok("defines.c", 'U');
  int semid9;

  do {
    semid9 = semget(semKey9, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
  }while(semid9 == -1);

  if(semid9 == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal9[1];
  union semun arg9;
  arg9.array = semInitVal9;

  if(semctl(semid9, 0, GETALL, arg9) == -1){
    ErrExit("semctl failed (semid9)");
  }

  sprintf(historical[4].idKey, "%x", semKey9);
  strcpy(historical[4].ipc, "SEMAPHORE");
  strcpy(historical[4].creator, "SM");
  historical[4] = get_time(historical[4], 'c');

  //==================================================================================
  //creazione dei semafori per sincronizzare chiusura MSQ tra S,R e H

  key_t semKeySRH = ftok("defines.c", 'G');
  int semidSRH;

  do{
    semidSRH = semget(semKeySRH, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
  }while(semidSRH == -1);

  if(semidSRH == -1){
    ErrExit("semget failed semidSRH (R)");
  }

  unsigned short semInitValSRH[1];
  union semun argSRH;
  argSRH.array = semInitValSRH;

  if(semctl(semidSRH, 0, GETALL, argSRH) == -1){
    ErrExit("semctl failed semidSRH(R)");
  }

  sprintf(historical[5].idKey, "%x", semKeySRH);
  strcpy(historical[5].ipc, "SEMAPHORE");
  strcpy(historical[5].creator, "SM");
  historical[5] = get_time(historical[5], 'c');

  //==================================================================================
  //creazione dei semafori per la scrittura di F10

  key_t semKey2 = ftok("defines.c", 'D');
  int semid2;

  do{
    semid2 = semget(semKey2, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
    printf("SEMID 2 %d\n", semid2);
  }while(semid2 == -1);

  if(semid2 == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal[1];
  union semun arg;
  arg.array = semInitVal;

  if(semctl(semid2, 0, GETALL, arg) == -1){
    ErrExit("semctl failed");
  }

  sprintf(historical[6].idKey, "%x", semKey2);
  strcpy(historical[6].ipc, "SEMAPHORE");
  strcpy(historical[6].creator, "SM");
  historical[6] = get_time(historical[6], 'c');

  //==================================================================================
  //creazione dei semafori per la sincronizzazione uscita processi

  key_t semKeyEND = ftok("shared_memory.c", 'C');
  int semidEND;

  do{
    semidEND = semget(semKeyEND, 1, IPC_CREAT | S_IRUSR | S_IWUSR);
  }while(semidEND == -1);

  if(semidEND == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitValE[1];
  union semun argE;
  argE.array = semInitValE;

  if(semctl(semidEND, 0, GETALL, argE) == -1){
    ErrExit("semctl failed semidEND");
  }

  sprintf(historical[7].idKey, "%x", semKeyEND);
  strcpy(historical[7].ipc, "SEMAPHORE");
  strcpy(historical[7].creator, "SM");
  historical[7] = get_time(historical[7], 'c');

//========================================================================//

  //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
  getcwd(path, PATH_SZ);
  strcat(path, argv[1]); //path contiene il percorso dove risiede F7

  //apro il file da leggere
  int F7 = open(path, O_RDWR, S_IRWXU);

  if(F7 == -1){
    ErrExit("open F7 failed");
  }

  struct pid pids = {};


  semOp(semid3, 0, -1);
  pids = get_pidF8(pids);

  semOp(semid9, 0, -1);
  pids = get_pidF9(pids);

  ssize_t numRead;
  int i = 0;
  int start_line = 0;
  int pid;
  struct hackler actions = {};

  lseek(F7, 23, SEEK_CUR);

  while((numRead = read(F7, &buffer[i], sizeof(char))) > 0){ //while che legge carattere per carattere da F0.csv

    if(buffer[i] == '\n'){
      actions = fill_hackler_structure(buffer, start_line);

      pid = fork();

      if(pid == -1){
        ErrExit("creation child of hackler failed");
      }

      if(pid == 0){ //sono nel figlio
        if(strcmp(actions.delay, "-") != 0)
        sleep(atoi(actions.delay)); //il figlio dorme per DelS1 secondi

        if(strcmp(actions.target, "S1") == 0){
          //scarico MQ coi pid

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            while(msgrcv(mqInc_id, &sigInc, sizeof(struct signal) - sizeof(long), 1 , IPC_NOWAIT) != -1){
              printf("SIGUSR1 %d\n", sigInc.pid);
              kill(sigInc.pid, SIGUSR1);
            }
          }

          if(strcmp(actions.action, "RemoveMSG") == 0){
            while(msgrcv(mqRmv_id, &sigRmv, sizeof(struct signal) - sizeof(long), 1 , IPC_NOWAIT) != -1){
              printf("RMVMSG %d\n", sigRmv.pid);
              kill(sigRmv.pid, SIGKILL);
            }
          }

          if(strcmp(actions.action, "SendMSG") == 0){
            printf("SNDMSG\n");
            while(msgrcv(mqSnd_id, &sigSnd, sizeof(struct signal) - sizeof(long), 1 , IPC_NOWAIT) != -1){
              kill(sigSnd.pid, SIGCONT);
            }
          }

        }else if(strcmp(actions.target, "S2") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            while(msgrcv(mqInc_id, &sigInc, sizeof(struct signal) - sizeof(long), 2 , IPC_NOWAIT) != -1){
              printf("SIGUSR1 %d\n", sigInc.pid);
              kill(sigInc.pid, SIGUSR1);
            }
          }

          if(strcmp(actions.action, "RemoveMSG") == 0){
            while(msgrcv(mqRmv_id, &sigRmv, sizeof(struct signal) - sizeof(long), 2 , IPC_NOWAIT) != -1){
              printf("RMVMSG %d\n", sigRmv.pid);
              kill(sigRmv.pid, SIGKILL);
            }
          }

          if(strcmp(actions.action, "SendMSG") == 0){
            printf("SNDMSG\n");
            while(msgrcv(mqSnd_id, &sigSnd, sizeof(struct signal) - sizeof(long), 2 , IPC_NOWAIT) != -1){
              kill(sigSnd.pid, SIGCONT);
            }
          }

        }else if(strcmp(actions.target, "S3") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            while(msgrcv(mqInc_id, &sigInc, sizeof(struct signal) - sizeof(long), 3 , IPC_NOWAIT) != -1){
              printf("SIGUSR1 %d\n", sigInc.pid);
              kill(sigInc.pid, SIGUSR1);
            }
          }

          if(strcmp(actions.action, "RemoveMSG") == 0){
            while(msgrcv(mqRmv_id, &sigRmv, sizeof(struct signal) - sizeof(long), 3 , IPC_NOWAIT) != -1){
              printf("RMVMSG %d\n", sigRmv.pid);
              kill(sigRmv.pid, SIGKILL);
            }
          }

          if(strcmp(actions.action, "SendMSG") == 0){
            printf("SNDMSG\n");
            while(msgrcv(mqSnd_id, &sigSnd, sizeof(struct signal) - sizeof(long), 3 , IPC_NOWAIT) != -1){
              kill(sigSnd.pid, SIGCONT);
            }
          }


        }else if(strcmp(actions.target, "R1") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            while(msgrcv(mqInc_id, &sigInc, sizeof(struct signal) - sizeof(long), 6 , IPC_NOWAIT) != -1){
              printf("SIGUSR1 %d\n", sigInc.pid);
              kill(sigInc.pid, SIGUSR1);
            }
          }

          if(strcmp(actions.action, "RemoveMSG") == 0){
            while(msgrcv(mqRmv_id, &sigRmv, sizeof(struct signal) - sizeof(long), 6 , IPC_NOWAIT) != -1){
              printf("RMVMSG %d\n", sigRmv.pid);
              kill(sigRmv.pid, SIGKILL);
            }
          }

          if(strcmp(actions.action, "SendMSG") == 0){
            printf("SNDMSG\n");
            while(msgrcv(mqSnd_id, &sigSnd, sizeof(struct signal) - sizeof(long), 6 , IPC_NOWAIT) != -1){
              kill(sigSnd.pid, SIGCONT);
            }
          }


        }else if(strcmp(actions.target, "R2") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            while(msgrcv(mqInc_id, &sigInc, sizeof(struct signal) - sizeof(long), 5 , IPC_NOWAIT) != -1){
              printf("SIGUSR1 %d\n", sigInc.pid);
              kill(sigInc.pid, SIGUSR1);
            }
          }

          if(strcmp(actions.action, "RemoveMSG") == 0){
            while(msgrcv(mqRmv_id, &sigRmv, sizeof(struct signal) - sizeof(long), 5 , IPC_NOWAIT) != -1){
              printf("RMVMSG %d\n", sigRmv.pid);
              kill(sigRmv.pid, SIGKILL);
            }
          }

          if(strcmp(actions.action, "SendMSG") == 0){
            printf("SNDMSG\n");
            while(msgrcv(mqSnd_id, &sigSnd, sizeof(struct signal) - sizeof(long), 5 , IPC_NOWAIT) != -1){
              kill(sigSnd.pid, SIGCONT);
            }
          }


        }else if(strcmp(actions.target, "R3") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            while(msgrcv(mqInc_id, &sigInc, sizeof(struct signal) - sizeof(long), 4 , IPC_NOWAIT) != -1){
              printf("SIGUSR1 %d\n", sigInc.pid);
              kill(sigInc.pid, SIGUSR1);
            }
          }

          if(strcmp(actions.action, "RemoveMSG") == 0){
            while(msgrcv(mqRmv_id, &sigRmv, sizeof(struct signal) - sizeof(long), 4 , IPC_NOWAIT) != -1){
              printf("RMVMSG %d\n", sigRmv.pid);
              kill(sigRmv.pid, SIGKILL);
            }
          }

          if(strcmp(actions.action, "SendMSG") == 0){
            printf("SNDMSG\n");
            while(msgrcv(mqSnd_id, &sigSnd, sizeof(struct signal) - sizeof(long), 4 , IPC_NOWAIT) != -1){
              kill(sigSnd.pid, SIGCONT);
            }
          }

        }else{  //SHUTDOWN

          kill(pids.pid_S[0],SIGTERM);
          kill(pids.pid_S[1],SIGTERM);
          kill(pids.pid_S[2],SIGTERM);

          kill(pids.pid_R[0],SIGTERM);
          kill(pids.pid_R[1],SIGTERM);
          kill(pids.pid_R[2],SIGTERM);

        }

        exit(0);  //termina
      }

      start_line = i + 1;
    }
    i++;
  }

  int status;
  while((pid = wait(&status)) != -1){
    printf("child with pid %d exited, status = %d\n", pid, WEXITSTATUS(status)); //qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
    i++;
  }


  printf("SRH Hackler before Semaphore\n");
  semOp(semidSRH, 0, 0);
  printf("SRH Hackler\n");

  if(msgctl(mqInc_id, IPC_RMID, NULL) == -1){
    ErrExit("close of MSG QUEUE mqInc");
  }

  historical[0] = get_time(historical[0], 'd');

  if(msgctl(mqRmv_id, IPC_RMID, NULL) == -1){
    ErrExit("close of MSG QUEUE mqRmv");
  }

  historical[1] = get_time(historical[1], 'd');

  if(msgctl(mqSnd_id, IPC_RMID, NULL) == -1){
    ErrExit("close of MSG QUEUE mqSnd");
  }

  historical[2] = get_time(historical[2], 'd');

  //rimozione del semaforo
  if(semctl(semid3, 0, IPC_RMID, 0) == -1){
    ErrExit("semctl(3) failed");
  }

  historical[3] = get_time(historical[3], 'd');

  //rimozione del semaforo
  if(semctl(semid9, 0, IPC_RMID, 0) == -1){
    ErrExit("semctl(9) failed");
  }

  historical[4] = get_time(historical[4], 'd');

  //rimozione del semaforo
  if(semctl(semidSRH, 0, IPC_RMID, 0) == -1){
    ErrExit("semctl(srh) failed");
  }

  historical[5] = get_time(historical[5], 'd');

  for(int i = 0; i < 6; i++){
    semOp(semid2, 0, -1);
    writeF10(historical[i], F10);
    semOp(semid2, 0, 1);
  }

  semOp(semidEND, 0, 0);

  //rimozione del semaforo
  if(semctl(semid2, 0, IPC_RMID, 0) == -1){
    ErrExit("semctl(2) failed");
  }

  historical[6] = get_time(historical[6], 'd');

  //rimozione del semaforo
  if(semctl(semidEND, 0, IPC_RMID, 0) == -1){
    ErrExit("semctl(2) failed");
  }

  historical[7] = get_time(historical[7], 'd');

  writeF10(historical[6], F10);
  writeF10(historical[7], F10);

  if(close(F10) == -1){
    ErrExit("Close F10 failed");
  }

  sigprocmask(SIG_SETMASK, &prevSet, NULL);

  return 0;

}
