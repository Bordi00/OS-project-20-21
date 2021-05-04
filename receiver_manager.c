/// @file receiver_manager.c
/// @brief Contiene l'implementazione del receiver_manager.

#include "defines.h"

/*
invece di creare tutti e tre i processi tramite un ciclo for
potremmo creare un processo alla volta e fargli fare il suo dovere
intanto il padre aspetta che finisca il figlio, quando il figlio finisce
il padre riprende e crea un altro figlio.
*/



int main(int argc, char * argv[]) {

const char heading[] = "Id;Message;IdSender;IdReceiver;DelS1;DelS2;DelS3;Type\n";
const char headingF9[] = "Id;PID\n";
int pid_R[3];

    pid_t pid = fork(); //creazione R1

    if(pid == -1)
        ErrExit("Child R1 not created.\n");

    else if(pid == 0){

        //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
        getcwd(path, PATH_SZ);
        strcat(path, "/OutputFiles/F6.csv"); //concateno il percorso dove risiede F6

        //apro il file da leggere
        int F6 = open(path, O_RDWR | O_CREAT, S_IRWXU);

        if(F6 == -1){
            ErrExit("open F6 failed");
        }

        if(close(F6 == -1))
          ErrExit("close");
            ssize_t numWrite = write(F6, heading, strlen(heading));

        if (numWrite != strlen(heading))
            ErrExit("write");

        if(close(F6) == -1)
            ErrExit("close");



      //il processo dorme per 1 secondo poi termina
      sleep(3);
      exit(0);

    } //qui sono finiti i compiti di R1

    // parent process must run here!
    int status = 0;
    // get termination status of each created subprocess.
    while((pid = wait(&status)) != -1){
      pid_R[0] = pid;
      printf("Child %d exited, status = %d\n", pid_R[0], WEXITSTATUS(status)); //qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
    }

    //generazione R2
    pid = fork();

    if(pid == -1)
        ErrExit("Child R2 not created.\n");
    else if(pid == 0){

        //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
        getcwd(path, PATH_SZ);
        strcat(path, "/OutputFiles/F5.csv"); //path contiene il percorso dove risiederà F5

        //creo e apro il file F2.csv
        int F5 = open(path, O_RDWR | O_CREAT, S_IRWXU);

        if(F5 == -1){
            ErrExit("Open F5 failed.");
        }

        ssize_t numWrite = write(F5, heading, strlen(heading));

        if (numWrite != strlen(heading))
            ErrExit("write");

        if(close(F5) == -1)
            ErrExit("close");

        sleep(2);
        exit(1);

    } //terminano i compiti di R2

       // parent process must run here!
    status = 0;
    // get termination status of each created subprocess.
    while((pid = wait(&status)) != -1){
      pid_R[1] = pid;
      printf("Child %d exited, status = %d\n", pid, WEXITSTATUS(status));//qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
    }

    //genero R3
    pid = fork();

    if(pid == -1)
        ErrExit("Child R3 not created.\n");
    else if(pid == 0){

        //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
        getcwd(path, PATH_SZ);
        strcat(path, "/OutputFiles/F4.csv"); //path contiene il percorso dove risiederà F4

        //creo e apro il file F4.csv
        int F4 = open(path, O_RDWR | O_CREAT, S_IRWXU);

        if(F4 == -1){
            ErrExit("Open F4 failed.");
        }

        ssize_t numWrite = write(F4, heading, strlen(heading));

        if (numWrite != strlen(heading))
            ErrExit("write");

        if(close(F4) == -1)
            ErrExit("close");

        sleep(1);
        exit(1);

    }
       // parent process must run here!
    status = 0;
    // get termination status of each created subprocess.
    while((pid = wait(&status)) != -1){
      pid_R[2] = pid;
      printf("Child %d exited, status = %d\n", pid, WEXITSTATUS(status));//qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
    }

    //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
    getcwd(path, PATH_SZ);
    strcat(path, "/OutputFiles/F9.csv");

    int F9 = open(path, O_RDWR | O_CREAT, S_IRWXU);

    ssize_t numWrite = write(F9, headingF9, strlen(headingF9));
    if(numWrite != strlen(headingF9))
      ErrExit("write");


    char R1[] = "R1;"; //intestazione R1
    char R2[] = "R2;";
    char R3[] = "R3;";

    char pid_R1[2];  //array che contiene il pid di R1 convertito in char
    char pid_R2[2];  //array che contiene il pid di R2 convertito in char
    char pid_R3[2];  //array che contiene il pid di R3 convertito in char

    //Scrivo R1
    sprintf(pid_R1, "%d", pid_R[0]);  //converto pid di R1 in char e lo metto nell'array pid_R1
    strcat(pid_R1, "\n");             //aggiungo un \n alla fine del pid

    numWrite = write(F9, R1, strlen(R1));
    if(numWrite != strlen(R1))
      ErrExit("write");

    numWrite = write(F9, pid_R1, strlen(pid_R1));
    if(numWrite != strlen(pid_R1))
      ErrExit("write");

    //Scrivo R2
    sprintf(pid_R2, "%d", pid_R[1]);  //converto pid di R2 in char e lo metto nell'array pid_R2
    strcat(pid_R2, "\n");             //aggiungo un \n alla fine del pid

    numWrite = write(F9, R2, strlen(R2));
    if(numWrite != strlen(R2))
      ErrExit("write");

    numWrite = write(F9, pid_R2, strlen(pid_R2));
    if(numWrite != strlen(pid_R2))
      ErrExit("write");

    //Scrivo R3
    sprintf(pid_R3, "%d", pid_R[2]);  //converto pid di R3 in char e lo metto nell'array pid_R3
    strcat(pid_R3, "\n");             //aggiungo un \n alla fine del pid

    numWrite = write(F9, R3, strlen(R3));
    if(numWrite != strlen(R3))
      ErrExit("write");

    numWrite = write(F9, pid_R3, strlen(pid_R3));
    if(numWrite != strlen(pid_R3))
      ErrExit("write");

    return 0;

}
