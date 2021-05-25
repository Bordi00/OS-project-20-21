/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"
#include <sys/msg.h>

int main(int argc, char * argv[]){

  //==================================================================================
  //creazione della message queue tra Sender e Hackler

  struct signal sig;

  key_t mqsig_Key = ftok("receiver_manager.c", 'H');
  int mqsig_id = msgget(mqsig_Key, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(mqsig_id == -1){
    ErrExit("internal msgget failed");
  }

  //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
  getcwd(path, PATH_SZ);
  strcat(path, argv[1]); //path contiene il percorso dove risiede F7

  //apro il file da leggere
  int F7 = open(path, O_RDWR, S_IRWXU);

  if(F7 == -1){
    ErrExit("open F7 failed");
  }

  struct pid pids = {};

  do{
    pids = get_pidF8(pids);
    pids = get_pidF9(pids);
  }while(pids.pid_S[2] == 0 || pids.pid_R[2] == 0);

  ssize_t numRead;
  int i = 0;
  int start_line = 0;
  int pid;
  struct hackler actions = {};
  struct list_t *list = new_list();

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
          while(msgrcv(mqsig_id, &sig, sizeof(sig) - sizeof(long), 1 , IPC_NOWAIT) != -1){

            insert_into_list(list, sig.pid);
            
            if(strcmp(actions.action, "IncreaseDelay") == 0){
              kill(sig.pid, SIGUSR1);
            }
            if(strcmp(actions.action, "RemoveMsg") == 0){
              printf("RMVMSG\n");
              kill(sig.pid, SIGKILL);
            }
            if(strcmp(actions.action, "SendMsg") == 0){
              kill(pids.pid_S[0], SIGALRM);
            }

          }

        }else if(strcmp(actions.target, "S2") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            kill(pids.pid_S[1], SIGUSR1);
          }else if(strcmp(actions.action, "RemoveMsg") == 0){
            kill(pids.pid_S[1], SIGUSR2);
          }else if(strcmp(actions.action, "SendMsg") == 0){
            kill(pids.pid_S[1], SIGALRM);
          }

        }else if(strcmp(actions.target, "S3") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            kill(pids.pid_S[2], SIGUSR1);
          }else if(strcmp(actions.action, "RemoveMsg") == 0){
            kill(pids.pid_S[2], SIGUSR2);
          }else if(strcmp(actions.action, "SendMsg") == 0){
            kill(pids.pid_S[2], SIGALRM);
          }

        }else if(strcmp(actions.target, "R1") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            kill(pids.pid_R[0], SIGUSR1);
          }else if(strcmp(actions.action, "RemoveMsg") == 0){
            kill(pids.pid_R[0], SIGUSR2);
          }else if(strcmp(actions.action, "SendMsg") == 0){
            kill(pids.pid_R[0], SIGALRM);
          }

        }else if(strcmp(actions.target, "R2") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            kill(pids.pid_R[1], SIGUSR1);
          }else if(strcmp(actions.action, "RemoveMsg") == 0){
            kill(pids.pid_R[1], SIGUSR2);
          }else if(strcmp(actions.action, "SendMsg") == 0){
            kill(pids.pid_R[1], SIGALRM);
          }

        }else if(strcmp(actions.target, "R3") == 0){

          if(strcmp(actions.action, "IncreaseDelay") == 0){
            kill(pids.pid_R[2], SIGUSR1);
          }else if(strcmp(actions.action, "RemoveMsg") == 0){
            kill(pids.pid_R[2], SIGUSR2);
          }else if(strcmp(actions.action, "SendMsg") == 0){
            kill(pids.pid_R[2], SIGALRM);
          }

        }else{  //SHUTDOWN

          kill(0,SIGTERM);

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
  return 0;

}
