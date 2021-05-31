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

  //==================================================================================
  //creazione della message queue tra Sender e Hackler

  struct signal sigInc; //MQ per IncreaseDelay

  key_t mqInc_Key = ftok("receiver_manager.c", 'D');
  int mqInc_id = msgget(mqInc_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqInc_id == -1){
    ErrExit("internal msgget failed");
  }


  struct signal sigRmv; //MQ per RemoveMSG


  key_t mqRmv_Key = ftok("receiver_manager.c", 'E');
  int mqRmv_id = msgget(mqRmv_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqRmv_id == -1){
    ErrExit("internal msgget failed");
  }

  struct signal sigSnd; //MQ per SendMSG


  key_t mqSnd_Key = ftok("receiver_manager.c", 'F');
  int mqSnd_id = msgget(mqSnd_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqSnd_id == -1){
    ErrExit("internal msgget failed");
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

  ssize_t numRead;
  int i = 0;
  char tmp;
  int pid;
  struct hackler actions = {};

  lseek(F7, 23, SEEK_CUR);

  while((numRead = read(F7, &tmp, sizeof(char))) > 0) { //while che legge carattere per carattere da F0.csv

    if (numRead > 0) {
      if (tmp != '\n') {
        buffer[i] = tmp;
        i++;
      } else {
        buffer[i] = '\0';
        i = 0;
        actions = fill_hackler_structure(buffer);
        printf("message: %s\n", actions.target);

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
            printf("Shutdown\n");

            kill(pids.pid_S[0],SIGTERM);
            kill(pids.pid_S[1],SIGTERM);
            kill(pids.pid_S[2],SIGTERM);

            kill(pids.pid_R[0],SIGTERM);
            kill(pids.pid_R[1],SIGTERM);
            kill(pids.pid_R[2],SIGTERM);

          }

          exit(0);  //termina
        }
      }
    }
  }

  while(wait(NULL) != -1);

  return 0;
}

