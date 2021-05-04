/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "defines.h"


/*
invece di creare tutti e tre i processi tramite un ciclo for
potremmo creare un processo alla volta e fargli fare il suo dovere
intanto il padre aspetta che finisca il figlio, quando il figlio finisce
il padre riprende e crea un altro figlio.
*/


int main(int argc, char * argv[]) {

    int pid_S[3];
    const char heading[] = "Id;Message;IDSender;IDReceiver;TimeArrival;TimeDeparture\n";
    const char headingF8[] = "SenderID;PID\n";
		//========================================================================================
    //creazione di PIPE1 e PIPE2
    int pipe1[2];
    int pipe2[2];
    create_pipe(pipe1);
    create_pipe(pipe2);

		//=========================================================================================
		//creazione coda di messaggi
		
    

			
		//==========================================================================================
		//generazione S1




    pid_t pid = fork(); 

    if(pid == -1)
        ErrExit("Child S1 not created.\n");

    else if(pid == 0){

        //chiusura canale di lettura 
        if(close(pipe1[0]) == -1)
			 	  ErrExit("can't close read(pipe1)"); 
      
        //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
        getcwd(path, PATH_SZ);
        strcat(path, argv[1]); //path contiene il percorso dove risiede F0

        //apro il file da leggere
        int F0 = open(path, O_RDWR, S_IRWXU);

        if(F0 == -1){
            ErrExit("open F0 failed");
        }

        //salvo Time arrival
        get_time_arrival(msgF1);
        ssize_t numRead = read(F0, buffer, BUFFER_SZ); //leggo F0 e salvo il contenuto in buffer

        if (numRead == -1)
          ErrExit("read");

        if(close(F0) == -1)
          ErrExit("close");

        buffer[numRead] = '\0'; //numRead è l'indice dopo il quale non ci sono altri caratteri letti da F0
				
				//riempie la struttura dati message
        fill_structure(buffer);
				
        //Metto in path il percorso di F1
        getcwd(path, PATH_SZ);
        strcat(path, "/OutputFiles/F1.csv");

        int F1 = open(path, O_RDWR | O_CREAT, S_IRWXU); //apro F1

        if(F1 == -1){
            ErrExit("Open F1 failed.");
        }
      
      int i = 0;
      while(i < 3){
				//aspetta DelS1 secondi
        sleep(message[i].delS1);
				//scrive il messaggio su pipe1 
				write_pipe(F1, message[i], pipe1, i);
        i++;
      }

      if(close(pipe1[1]) == -1){
        ErrExit("close of write-end pipe1 failed");
      }
      //scrivo F0 (buffer) in F1
      writeF1(F1, msgF1, message);

      //il processo dorme per 1 secondo poi termina
      sleep(1);
      exit(0);

    } //qui sono finiti i compiti di S1

    // parent process must run here!
    int status = 0;
    // get termination status of each created subprocess.
    while((pid = wait(&status)) != -1){
      pid_S[0] = pid;
      printf("Child %d exited, status = %d\n", pid_S[0], WEXITSTATUS(status)); //qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
    }



	//======================================================================================================
	//generzione S2


    

    pid = fork();

    if(pid == -1)
        ErrExit("Child S2 not created.\n");
    else if(pid == 0){

        //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
        getcwd(path, PATH_SZ);
        strcat(path, "/OutputFiles/F2.csv"); //path contiene il percorso dove risiederà F2

        //creo e apro il file F2.csv
        int F2 = open(path, O_RDWR | O_CREAT, S_IRWXU);

        if(F2 == -1){
            ErrExit("Open F2 failed.");
        }

        ssize_t numWrite = write(F2, heading, strlen(heading));

        if (numWrite != strlen(heading))
            ErrExit("write");

        if(close(F2) == -1)
            ErrExit("close");
        
        //chiudo write-end pipe1
        if(close(pipe1[1]) == -1)
          ErrExit("can't close write (pipe1)");
        
        //chiudo read-end pipe2
        if(close(pipe2[0]) == -1)
          ErrExit("can't close read (pipe2)");

        sleep(2);
        exit(1);

    } //terminano i compiti di S2

       // parent process must run here!
    status = 0;
    // get termination status of each created subprocess.
    while((pid = wait(&status)) != -1){
      pid_S[1] = pid;
      printf("Child %d exited, status = %d\n", pid, WEXITSTATUS(status));//qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
    }



	//===========================================================================================================
	//generazione S3


 
    
    pid = fork();

    if(pid == -1)
        ErrExit("Child S3 not created.\n");
    else if(pid == 0){

        //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
        getcwd(path, PATH_SZ);
        strcat(path, "/OutputFiles/F3.csv"); //path contiene il percorso dove risiederà F3

        //creo e apro il file F2.csv
        int F3 = open(path, O_RDWR | O_CREAT, S_IRWXU);

        if(F3 == -1){
            ErrExit("Open F3 failed.");
        }

        ssize_t numWrite = write(F3, heading, strlen(heading));

        if (numWrite != strlen(heading))
            ErrExit("write");

        if(close(F3) == -1)
            ErrExit("close");

        //chiudo write-end pipe2
        if(close(pipe2[1]) == -1)
          ErrExit("can't close write (pipe2)");

        sleep(3);
        exit(1);

    }
       // parent process must run here!
    status = 0;
    // get termination status of each created subprocess.
    while((pid = wait(&status)) != -1){
      pid_S[2] = pid;
      printf("Child %d exited, status = %d\n", pid, WEXITSTATUS(status));//qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
    }



	//==========================================================================================================
	//compilazione del file F8.csv


 
    //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
    getcwd(path, PATH_SZ);
    strcat(path, "/OutputFiles/F8.csv");

    int F8 = open(path, O_RDWR | O_CREAT, S_IRWXU);

    ssize_t numWrite = write(F8, headingF8, strlen(headingF8));
    if(numWrite != strlen(headingF8))
      ErrExit("write");


    char S1[] = "S1;"; //intestazione S1
    char S2[] = "S2;";
    char S3[] = "S3;";

    char pid_S1[2];  //array che contiene il pid di S1 convertito in char
    char pid_S2[2];  //array che contiene il pid di S2 convertito in char
    char pid_S3[2];  //array che contiene il pid di S3 convertito in char

    //Scrivo S1
    sprintf(pid_S1, "%d", pid_S[0]);  //converto pid di S1 in char e lo metto nell'array pid_S1
    strcat(pid_S1, "\n");             //aggiungo un \n alla fine del pid

    numWrite = write(F8, S1, strlen(S1));
    if(numWrite != strlen(S1))
      ErrExit("write");

    numWrite = write(F8, pid_S1, strlen(pid_S1));
    if(numWrite != strlen(pid_S1))
      ErrExit("write");

    //Scrivo S2
    sprintf(pid_S2, "%d", pid_S[1]);  //converto pid di S2 in char e lo metto nell'array pid_S2
    strcat(pid_S2, "\n");             //aggiungo un \n alla fine del pid

    numWrite = write(F8, S2, strlen(S2));
    if(numWrite != strlen(S2))
      ErrExit("write");

    numWrite = write(F8, pid_S2, strlen(pid_S2));
    if(numWrite != strlen(pid_S2))
      ErrExit("write");

    //Scrivo S3
    sprintf(pid_S3, "%d", pid_S[2]);  //converto pid di S3 in char e lo metto nell'array pid_S3
    strcat(pid_S3, "\n");             //aggiungo un \n alla fine del pid

    numWrite = write(F8, S3, strlen(S3));
    if(numWrite != strlen(S3))
      ErrExit("write");

    numWrite = write(F8, pid_S3, strlen(pid_S3));
    if(numWrite != strlen(pid_S3))
      ErrExit("write");

    return 0;
}
