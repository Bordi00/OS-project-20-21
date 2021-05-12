/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche del progetto.

#include "defines.h"

// funzione per riempire la struct con i relativi campi in F0
struct msg fill_structure(char buffer[], int j){

		struct msg message = {"", "", "", "", "", "", "", ""};

		int counter = 0;
		int index = 0;

			//riempo la struct con i messaggi
		for(; buffer[j] != '\n'; j++){ //itero stringa per stringa
			if(buffer[j] == ';'){
				index = 0;
				counter++;
				j++;  //per passare dopo il ;
			}
			switch(counter){
				case 0: // id
					message.id[index] = buffer[j];
          index++;
					break;
				case 1: // msg
					message.message[index] = buffer[j];
					index++;
					break;
				case 2: // idSender
					message.idSender[index] = buffer[j];
					index++;
					break;
				case 3: // idReceiver
					message.idReceiver[index] = buffer[j];
					index++;
					break;
				case 4: // DelS1
					message.delS1[index] = buffer[j];
					index++;
					break;
				case 5: // DelS2
					message.delS2[index] = buffer[j];
					index++;
					break;
				case 6: // DelS3
					message.delS3[index] = buffer[j];
					index++;
					break;
				case 7: // type
					message.type[index] = buffer[j];
					index++;
					break;
					}
			}


      return message;
}

struct container get_time_arrival(struct container msgFile){

  time_t now = time(NULL);
  struct tm *tm_struct = localtime(&now);
  int hour = tm_struct->tm_hour;
  int min = tm_struct->tm_min;
  int sec = tm_struct->tm_sec;

	char hour_t[3];
	char min_t[3];
	char sec_t[3];
	sprintf(hour_t, "%d", hour);
  sprintf(min_t, "%d", min);
  sprintf(sec_t, "%d", sec);

  strcat(msgFile.time_arrival, hour_t);
	strcat(msgFile.time_arrival, ":");
	strcat(msgFile.time_arrival, min_t);
	strcat(msgFile.time_arrival, ":");
	strcat(msgFile.time_arrival, sec_t);

  return msgFile;
}

struct container get_time_departure(struct container msgFile){

  time_t now = time(NULL);
  struct tm *tm_struct = localtime(&now);
  int hour = tm_struct->tm_hour;
  int min = tm_struct->tm_min;
  int sec = tm_struct->tm_sec;

	char hour_t[3];
	char min_t[3];
	char sec_t[3];
	sprintf(hour_t, "%d", hour);
  sprintf(min_t, "%d", min);
  sprintf(sec_t, "%d", sec);

  strcat(msgFile.time_departure, hour_t);
	strcat(msgFile.time_departure, ":");
	strcat(msgFile.time_departure, min_t);
	strcat(msgFile.time_departure, ":");
	strcat(msgFile.time_departure, sec_t);

	msgFile.time_departure[8] = '\0';

  return msgFile;
}

void writeFile(struct container msgFile, struct msg message, int fd){

  ssize_t numWrite;

  strcpy(msgFile.id, message.id);
	strcpy(msgFile.message, message.message);

	msgFile.idSender[0] = message.idSender[1];
	msgFile.idReceiver[0] = message.idReceiver[1];

  numWrite = write(fd, msgFile.id, strlen(msgFile.id));
  	if (numWrite != strlen(msgFile.id))
  		ErrExit("write id");

      numWrite = write(fd, ";", sizeof(char));
      if (numWrite != sizeof(char))
        ErrExit("write");

      numWrite = write(fd, msgFile.message, strlen(msgFile.message));
      if (numWrite != strlen(msgFile.message))
        ErrExit("write message");

			numWrite = write(fd, ";", sizeof(char));
      if (numWrite != sizeof(char))
        ErrExit("write");

      numWrite = write(fd, msgFile.idSender, strlen(msgFile.idSender));
      if (numWrite != strlen(msgFile.idSender))
        ErrExit("write idSender");

			numWrite = write(fd, ";", sizeof(char));
      if (numWrite != sizeof(char))
        ErrExit("write");

      numWrite = write(fd, msgFile.idReceiver, strlen(msgFile.idReceiver));
      if (numWrite != strlen(msgFile.idReceiver))
        ErrExit("write idReceiver");

			numWrite = write(fd, ";", sizeof(char));
      if (numWrite != sizeof(char))
        ErrExit("write");

			numWrite = write(fd, msgFile.time_arrival, strlen(msgFile.time_arrival));
      if (numWrite != strlen(msgFile.time_arrival))
        ErrExit("write time_arrival");

			numWrite = write(fd, ";", sizeof(char));
      if (numWrite != sizeof(char))
        ErrExit("write");

			numWrite = write(fd, msgFile.time_departure, strlen(msgFile.time_departure));
      if (numWrite != strlen(msgFile.time_departure))
        ErrExit("write time_departure");

      numWrite = write(fd, "\n", sizeof(char));
        if (numWrite != sizeof(char))
          ErrExit("write");

				//printf("message arrived %s , td %s\n", message.message, msgFile.time_departure);
}

void writeF8(int pid_S[3]){
 const char headingF8[] = "SenderID;PID\n";
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

	if(close(F8) == -1){
		ErrExit("Close F8 failed");
	}
}

void sigHandler(int sig){
  	printf("Sigalarm catched\n");
}

void printSemaphoresValue (int semid) {
    unsigned short semVal[1];
    union semun arg;
    arg.array = semVal;

    // get the current state of the set
    if (semctl(semid, 0 /*ignored*/, GETALL, arg) == -1)
        ErrExit("semctl GETALL failed");

    // print the semaphore's value
    printf("semaphore set state:\n");
    for (int i = 0; i < 2; i++)
        printf("id: %d --> %d\n", i, semVal[i]);
}
