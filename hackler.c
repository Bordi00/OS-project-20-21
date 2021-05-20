/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"

int main(int argc, char * argv[]){

  //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
  getcwd(path, PATH_SZ);
  strcat(path, argv[1]); //path contiene il percorso dove risiede F7

  //apro il file da leggere
  int F7 = open(path, O_RDWR, S_IRWXU);

  if(F7 == -1){
    ErrExit("open F7 failed");
  }

  struct pid pids;

  pids = get_pidF8(pids);
  pids = get_pidF9(pids);

  ssize_t numRead;
  int i = 0;
  int start_line = 0;
  int pid;
  struct hackler actions = { };

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
          if(strcmp(actions.action, "IncreaseDelay") == 0){
            kill(pids.pid_S[0], SIGUSR1);
          }else if(strcmp(actions.action, "RemoveMsg") == 0){
            kill(pids.pid_S[0], SIGUSR2);
          }else if(strcmp(actions.action, "SendMsg") == 0){
            kill(pids.pid_S[0], SIGALRM);
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
