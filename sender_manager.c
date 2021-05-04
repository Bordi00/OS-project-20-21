
/// @file sender_manager.c
/// @brief Contiene l'implementazione del sender_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "pipe.h"
#include <sys/shm.h>
#include <sys/msg.h>


int main(int argc, char * argv[]) {

  int pid_S[3];
  pid_S[0] = 0;
  pid_S[1] = 0;
  pid_S[2] = 0;

	//=================================================================================
	//dichiarazione intestazioni per F8, F1, F2, F3 e array PID


    const char heading[] = "Id;Message;IDSender;IDReceiver;TimeArrival;TimeDeparture\n";


	//=================================================================================
  //creazione pipe e allocazione delle variabili necessarie

	int pipe1[2];
  int pipe2[2];

  create_pipe(pipe1);
  create_pipe(pipe2);

	//=================================================================================
	//creazione fifo
  int fifo;
  if((fifo = mkfifo("OutputFiles/my_fifo.txt", S_IRUSR | S_IWUSR | S_IRWXO)) == -1){
    ErrExit("create fifo failed");
  }

	//=================================================================================
	//creazione della shared memory

	key_t shmKey = 01101101;
  int shmid;
  shmid = alloc_shared_memory(shmKey, sizeof(struct message));
  struct msg *messageSH = (struct msg *)get_shared_memory(shmid, 0);

	//==================================================================================
	//creazione della message queue

	struct mymsg{
		long mtype;
		struct msg m_message;
	}m;

  m.mtype = 1;

	key_t msgKey = 01110001;
	int msqid = msgget(msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(msqid == -1){
    ErrExit("msgget failed");
  }


  //==================================================================================
	//creazione dei semafori

	key_t semKey = 01110011;
  int semid = semget(semKey, 1, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(semid == -1){
    ErrExit("semget failed");
  }

  unsigned short semInitVal[] = {1};
  union semun arg;
  arg.array = semInitVal;

  if(semctl(semid, 0, SETALL, arg) == -1){
    ErrExit("semctl failed");
  }


	//===================================================================================
	//generazione dei processi figlio

  int pid;
  pid = fork(); //creo S1

  if(pid != 0){ //sono nel padre
    pid_S[0] = pid; //salvo pid di S1
  }else if(pid == 0){ //sono in S1
    pid_S[0] = getpid(); //inizializzo
    pid_S[1] = 0; //inizializzo
    pid_S[2] = 0; //inizializzo
  }else{
    ErrExit("Error on child S1");
  }

  if(pid != 0){ //sono nel padre
    pid = fork(); //creo S2

    if(pid == 0){ //sono in S2
      pid_S[0] = 0; //inizializzo
      pid_S[1] = getpid();
      pid_S[2] = 0;
    }

    if(pid != 0){ //sono nel padre
      pid_S[1] = pid; //salvo pid di S2
    }
  }

  if(pid != 0){
    pid = fork();   //creo S3

    if(pid == 0){ //sono in S3
      pid_S[0] = 0; //inizializzo
      pid_S[1] = 0;
      pid_S[2] = getpid();
    }

    if(pid != 0){ //sono nel padre
      pid_S[2] = pid; //salvo pid di S2
    }
  }

//==============================ESECUZIONE S1========================================//

  if(pid == 0 && pid_S[0] > 0 && pid_S[1] == 0 && pid_S[2] == 0){
		if(close(pipe1[0]) == -1){
			ErrExit("Close of Read hand of pipe1 failed.\n");
		}

    getcwd(path, PATH_SZ);
    strcat(path, argv[1]);	//troviamo F0.csv

		int F0 = open(path, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);

    if(F0 == -1){
      ErrExit("open F0 failed");
    }


		lseek(F0, strlen(heading) - 3, SEEK_CUR); //ci spostiamo dopo l'intestazione (heading)

    int F1 = open("OutputFiles/F1.csv", O_RDWR | O_CREAT, S_IRWXU);

    if(F1 == -1){
      ErrExit("open F1 failed");
    }

		ssize_t numWrite = write(F1, heading, strlen(heading));
		if(numWrite != strlen(heading)){
			ErrExit("write F1 failed");
		}

    int i = 0;
    ssize_t numRead;
		int start_line = 0;

    while((numRead = read(F0, &buffer[i], sizeof(char))) > 0 && buffer[i] != '\0'){

      if(buffer[i] == '\n'){

        struct container msgFile = {"","","","","",""};
        msgFile = get_time_arrival(msgFile); //segniamo l'ora di arrivo di F0 e la scriviamo in msgF1

        struct msg message = {"", "", "", "", "", "", "", ""};
				message = fill_structure(buffer, message, start_line);

        if(strcmp(message.idSender, "S1") == 0){
					sleep(atoi(message.delS1));   //il messaggio attende delS1 secondi prima di essere inviato
					msgFile = get_time_departure(msgFile);
					writeFile(msgFile, message, F1);	//scrittura messaggio su F1

					//inviamo il messaggio alla corrispondente IPC
					if(strcmp(message.type, "Q") == 0){

						m.m_message = message;
						ssize_t mSize = sizeof(struct mymsg) - sizeof(long);
						if(msgsnd(msqid, &m, mSize ,0) == -1){
							ErrExit("message send failed (S1)");
						}
					}

					if(strcmp(message.type, "SH") == 0){
						semOp(semid, 0, -1);	//-1 sul semaforo di S1

            strcpy(messageSH->id, message.id);
            strcpy(messageSH->message, message.message);
            strcpy(messageSH->idSender, message.idSender);
            strcpy(messageSH->idReceiver, message.idReceiver);
            strcpy(messageSH->delS1, message.delS1);
            strcpy(messageSH->delS2, message.delS2);
            strcpy(messageSH->delS3, message.delS3);
            strcpy(messageSH->type, message.type);

            semOp(semid, 0, 1);
					}

        }

				if(strcmp(message.idSender, "S2") == 0){
        	sleep(atoi(message.delS1));   //il messaggio attende delS1 secondi prima di essere inviato
          msgFile = get_time_departure(msgFile);
          writeFile(msgFile, message, F1);	//scrittura messaggio su F1
          //mandiamo ad S2 tramite pipe1
          ssize_t nBys = write(pipe1[1], &message, sizeof(struct msg));
          if(nBys != sizeof(struct msg))
            ErrExit("write to pipe1 failed");

        }

				if(strcmp(message.idSender, "S3") == 0){
					sleep(atoi(message.delS1));   //il messaggio attende delS1 secondi prima di essere inviato
					msgFile = get_time_departure(msgFile);
					writeFile(msgFile, message, F1);	//scrittura messaggio su F1

          //mandiamo ad S2 tramite pipe1
          ssize_t nBys = write(pipe1[1], &message, sizeof(struct msg));
          if(nBys != sizeof(struct msg))
            ErrExit("write to pipe1 failed");
        }

        start_line = i + 1;
      }
      i++;
    }

		if(close(F1) == -1)
			ErrExit("close");

    struct msg final_msg = {"-1", "", "", "", "", "", "", ""};

    numWrite = write(pipe1[1], &final_msg, sizeof(struct msg));

    if(close(pipe1[1]) == -1){
      ErrExit("close pipe1 write end in S1 failed");
    }

    exit(0);

  }else if(pid == 0 && pid_S[0] == 0 && pid_S[1] > 0 && pid_S[2] == 0){

  //==============================ESECUZIONE S2========================================//

  if(close(pipe1[1]) == -1){
    ErrExit("Close of Write end of pipe1 failed");
  }

  if(close(pipe2[0]) == -1){
    ErrExit("Close of Read end of pipe2 failed");
  }

  int F2 = open("OutputFiles/F2.csv", O_RDWR | O_CREAT, S_IRWXU);

  if(F2 == -1){
    ErrExit("open F2 failed");
  }

  ssize_t numWrite = write(F2, heading, strlen(heading));
  if(numWrite != strlen(heading)){
    ErrExit("write F2 failed");
  }

  struct msg message;
  ssize_t nBys;

  while((nBys = read(pipe1[0], &message, sizeof(struct msg))) > 0){

    struct container msgFile = {"","","","","",""};
    msgFile = get_time_arrival(msgFile); //segniamo l'ora di arrivo di F0 e la scriviamo in msgF1

    if(strcmp(message.idSender, "S2") == 0){
      sleep(atoi(message.delS2));   //il messaggio attende delS1 secondi prima di essere inviato
      msgFile = get_time_departure(msgFile);
      writeFile(msgFile, message, F2);	//scrittura messaggio su F1

      //inviamo il messaggio alla corrispondente IPC
      if(strcmp(message.type, "Q") == 0){
        m.m_message = message;
        ssize_t mSize = sizeof(struct mymsg) - sizeof(long);
        if(msgsnd(msqid, &m, mSize ,0) == -1){
          ErrExit("message send failed (S2)");
        }
      }

      if(strcmp(message.type, "SH") == 0){

        semOp(semid, 0, -1);

        strcpy(messageSH->id, message.id);
        strcpy(messageSH->message, message.message);
        strcpy(messageSH->idSender, message.idSender);
        strcpy(messageSH->idReceiver, message.idReceiver);
        strcpy(messageSH->delS1, message.delS1);
        strcpy(messageSH->delS2, message.delS2);
        strcpy(messageSH->delS3, message.delS3);
        strcpy(messageSH->type, message.type);

        semOp(semid, 0, 1);
      }

    }

    if(strcmp(message.idSender, "S3") == 0){
      sleep(atoi(message.delS2));   //il messaggio attende delS1 secondi prima di essere inviato
      msgFile = get_time_departure(msgFile);
      writeFile(msgFile, message, F2);	//scrittura messaggio su F1

      //mandiamo ad S3 tramite pipe2
      ssize_t nBys = write(pipe2[1], &message, sizeof(struct msg));
      if(nBys != sizeof(struct msg))
        ErrExit("write to pipe2 failed");

    }

    if(strcmp(message.id, "-1") == 0){
      break;
    }

  }

  if(close(pipe1[0]) == -1){
    ErrExit("Close of Read end of pipe1 failed (S2)");
  }

  struct msg final_msg = {"-1", "", "", "", "", "", "", ""};

  numWrite = write(pipe2[1], &final_msg, sizeof(struct msg));

  if(close(pipe2[1]) == -1){
    ErrExit("Close of Write end of pipe2 failed (S2)");
  }

  exit(0);

  }else if(pid == 0 && pid_S[0] == 0 && pid_S[1] == 0 && pid_S[2] > 0){ //S3

    if(close(pipe2[1]) == -1){
      ErrExit("Close write end of pipe2");
    }

    int F3 = open("OutputFiles/F3.csv", O_RDWR | O_CREAT, S_IRWXU);

    if(F3 == -1){
      ErrExit("open F3 failed");
    }

    ssize_t numWrite = write(F3, heading, strlen(heading));
    if(numWrite != strlen(heading)){
      ErrExit("write F3 failed");
    }

    ssize_t nBys;
    struct msg message;

    while((nBys = read(pipe2[0], &message, sizeof(struct msg))) > 0){
      struct container msgFile = {"","","","","",""};

      msgFile = get_time_arrival(msgFile);
      sleep(atoi(message.delS3));   //il messaggio attende delS1 secondi prima di essere inviato

      msgFile = get_time_departure(msgFile);
      writeFile(msgFile, message, F3);	//scrittura messaggio su F1

      //inviamo il messaggio alla corrispondente IPC
      if(strcmp(message.type, "Q") == 0){
        m.m_message = message;
        ssize_t mSize = sizeof(struct mymsg) - sizeof(long);
        if(msgsnd(msqid, &m, mSize ,0) == -1){
          ErrExit("message send failed (S3)");
        }

      }

      if(strcmp(message.type, "SH") == 0){

        semOp(semid, 0, -1);	//-1 sul semaforo di S1

        strcpy(messageSH->id, message.id);
        strcpy(messageSH->message, message.message);
        strcpy(messageSH->idSender, message.idSender);
        strcpy(messageSH->idReceiver, message.idReceiver);
        strcpy(messageSH->delS1, message.delS1);
        strcpy(messageSH->delS2, message.delS2);
        strcpy(messageSH->delS3, message.delS3);
        strcpy(messageSH->type, message.type);

        semOp(semid, 0, 1);
      }

      if(strcmp(message.type, "FIFO") == 0){

        int fifo = open("OutputFiles/my_fifo.txt", O_WRONLY);

        if(fifo == -1){
          ErrExit("open (fifo) failed");
        }

        numWrite = write(fifo, &message, sizeof(struct msg));

        if(numWrite != sizeof(struct msg)){
          ErrExit("write on fifo failed");
        }
        
      }

      if(strcmp(message.id, "-1") == 0){
        break;
      }

    }

  if(close(F3) == -1){
    ErrExit("Close F3 failed");
  }

  if(close(pipe2[0]) == -1){
    ErrExit("Close of read end of pipe2 failed (S3)");
  }

  exit(0);

  }else if(pid != 0 && pid_S[0] > 0 && pid_S[1] > 0 && pid_S[2] > 0){  //Padre
    writeF8(pid_S);

    // parent process must run here!
    int status = 0;
    int i = 0;

    // get termination status of each created subprocess.
    while((pid = wait(&status)) != -1){
      printf("Child %d exited, status = %d\n", pid_S[i], WEXITSTATUS(status)); //qui sta eseguendo sicuramente il padre che ha nella variabile pid il pid reale del figlio che ha creato
      i++;
    }

    if(close(pipe1[1]) == -1){
      ErrExit("close pipe1 write end in father failed");
    }

    if(close(pipe1[0]) == -1){
      ErrExit("close pipe1 read end in father failed");
    }

    if(close(pipe2[1]) == -1){
      ErrExit("close pipe2 write end in father failed");
    }
    if(close(pipe2[0]) == -1){
      ErrExit("close pipe2 read end in father failed");
    }

    close(fifo);
    free_shared_memory(messageSH);

    if(semctl(semid, 0, IPC_RMID, 0) == -1)
      ErrExit("semctl failed");

    return 0;

  }
}
