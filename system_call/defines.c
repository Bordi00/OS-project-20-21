/// @file defines.c
/// @brief Contiene l'implementazione delle funzioni
///specifiche del progetto.

#include "defines.h"

// funzione per riempire la struct con i relativi campi in F0
struct msg fill_structure(char buffer[]){

		struct msg message = {};

		int counter = 0; //conta quale campo del messaggio sto leggendo dal buffer
		int index = 0;
		int j = 0;

		//riempo la struct con il messaggio
		for(; buffer[j] != '\0'; j++){ //itero stringa
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

struct hackler fill_hackler_structure(char buffer[]){
	struct hackler message = { };

	int counter = 0;
	int index = 0;
	int j = 0;

		//riempo la struct con i messaggi
	for(; buffer[j] != '\0'; j++){ //itero stringa per stringa
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
				message.delay[index] = buffer[j];
				index++;
				break;
			case 2: // idSender
				if(buffer[j] != ';'){
					message.target[index] = buffer[j];
					index++;
				}else{
					strcpy(message.target, "-");
					counter++;
				}
				break;
			case 3: // idReceiver
				message.action[index] = buffer[j];
				index++;
				break;
				}
		}


		return message;
}

struct container get_time_arrival(){
  struct container msgFile = {};

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

struct ipc get_time(struct ipc historical, char flag){

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

	if(flag == 'c'){
	  strcat(historical.creation, hour_t);
		strcat(historical.creation, ":");
		strcat(historical.creation, min_t);
		strcat(historical.creation, ":");
		strcat(historical.creation, sec_t);
	}else{
		strcat(historical.destruction, hour_t);
	 	strcat(historical.destruction, ":");
	 	strcat(historical.destruction, min_t);
	 	strcat(historical.destruction, ":");
	 	strcat(historical.destruction, sec_t);
	}

  return historical;
}


void writeFile(struct container msgFile, struct msg message, int fd){

  ssize_t numWrite;
  char result[100] = "";

  strcpy(msgFile.id, message.id);
	strcpy(msgFile.message, message.message);

	//salviamo solo la lettera del sender/receiver
	msgFile.idSender[0] = message.idSender[1];
	msgFile.idReceiver[0] = message.idReceiver[1];

  strcat(result, msgFile.id);
  strcat(result, ";");
  strcat(result, msgFile.message);
  strcat(result, ";");
  strcat(result, msgFile.idSender);
  strcat(result, ";");
  strcat(result, msgFile.idReceiver);
  strcat(result, ";");
  strcat(result, msgFile.time_arrival);
  strcat(result, ";");
  strcat(result, msgFile.time_departure);
  strcat(result, "\n\0");

  //write atomica
  numWrite = write(fd, result, strlen(result));

  if (numWrite != strlen(result)) {
    ErrExit("write file");
  }

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

  char pid_S1[6];  //array che contiene il pid di S1 convertito in char
  char pid_S2[6];  //array che contiene il pid di S2 convertito in char
  char pid_S3[6];  //array che contiene il pid di S3 convertito in char

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

void writeF9(int pid_R[3]){
 const char headingF9[] = "ReceiverID;PID\n";
  //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
  getcwd(path, PATH_SZ);
  strcat(path, "/OutputFiles/F9.csv");

  int F9 = open(path, O_RDWR | O_CREAT, S_IRWXU);

  ssize_t numWrite = write(F9, headingF9, strlen(headingF9));
  if(numWrite != strlen(headingF9))
    ErrExit("write");


  char S1[] = "R1;"; //intestazione S1
  char S2[] = "R2;";
  char S3[] = "R3;";

  char pid_R1[6];  //array che contiene il pid di S1 convertito in char
  char pid_R2[6];  //array che contiene il pid di S2 convertito in char
  char pid_R3[6];  //array che contiene il pid di S3 convertito in char

  //Scrivo S1
  sprintf(pid_R1, "%d", pid_R[0]);  //converto pid di S1 in char e lo metto nell'array pid_R1
  strcat(pid_R1, "\n");             //aggiungo un \n alla fine del pid

  numWrite = write(F9, S1, strlen(S1));
  if(numWrite != strlen(S1))
  	ErrExit("write");

  numWrite = write(F9, pid_R1, strlen(pid_R1));
  if(numWrite != strlen(pid_R1))
    ErrExit("write");

    //Scrivo S2
  sprintf(pid_R2, "%d", pid_R[1]);  //converto pid di S2 in char e lo metto nell'array pid_R2
  strcat(pid_R2, "\n");             //aggiungo un \n alla fine del pid

  numWrite = write(F9, S2, strlen(S2));
  if(numWrite != strlen(S2))
    ErrExit("write");

  numWrite = write(F9, pid_R2, strlen(pid_R2));
  if(numWrite != strlen(pid_R2))
    ErrExit("write");

  //Scrivo S3
  sprintf(pid_R3, "%d", pid_R[2]);  //converto pid di S3 in char e lo metto nell'array pid_R3
  strcat(pid_R3, "\n");             //aggiungo un \n alla fine del pid

  numWrite = write(F9, S3, strlen(S3));
  if(numWrite != strlen(S3))
    ErrExit("write");

  numWrite = write(F9, pid_R3, strlen(pid_R3));
  if(numWrite != strlen(pid_R3))
    ErrExit("write");

	if(close(F9) == -1){
		ErrExit("Close F9 failed");
	}
}

void writeF10(struct ipc historical, int F10){
	ssize_t numWrite;
	char result[100] = "";

	strcat(result, historical.ipc);
  strcat(result, ";");
  strcat(result, historical.idKey);
  strcat(result, ";");
  strcat(result, historical.creator);
  strcat(result, ";");
  strcat(result, historical.creation);
  strcat(result, ";");
  strcat(result, historical.destruction);
  strcat(result, "\n\0");

  numWrite = write(F10, result, strlen(result));

  if (numWrite != strlen(result)) {
    ErrExit("write file F10");
  }

}

struct container init_container(struct container msgFile){
	strcpy(msgFile.time_arrival, "");  //inizializziamo i campi per la registrazione del tempo di arrivo e partenza dei messaggi
	strcpy(msgFile.time_departure, "");
	strcpy(msgFile.id, "");
	strcpy(msgFile.message, "");
	strcpy(msgFile.idSender, "");
	strcpy(msgFile.idReceiver, "");

	return msgFile;

}

struct msg init_msg(struct msg message){
		strcpy(message.id, "");
		strcpy(message.message, "");
		strcpy(message.idSender, "");
		strcpy(message.idReceiver, "");
		strcpy(message.delS1, "");
		strcpy(message.delS2, "");
		strcpy(message.delS3, "");
		strcpy(message.type, "");

		return message;
}

struct pid get_pidF8(struct pid pid){

	int F8;
	char bufferF8[100];
	char pidF8[5];
	ssize_t nBys;

	F8 = open("OutputFiles/F8.csv", O_RDONLY, S_IRUSR);
	nBys = read(F8, &bufferF8, sizeof(bufferF8));

  bufferF8[nBys] = '\0';

	int indexF8;
	int i = 0;
	int j = 0;

	for(indexF8 = 0; bufferF8[indexF8] != '\n'; indexF8++);
	indexF8++;

	while(bufferF8[indexF8] != '\0'){
		if(bufferF8[indexF8] == ';'){
			while(bufferF8[indexF8] != '\n'){
				indexF8++;
				pidF8[i] = bufferF8[indexF8];
				i++;
			}
			//printf("Pid %s")
			pid.pid_S[j] = atoi(pidF8);
			j++;
			i = 0;
		}
		indexF8++;
	}

	return pid;
}

struct pid get_pidF9(struct pid pid){

	int F9;
	char bufferF9[100];
	char pidF9[5];
  ssize_t nBys;

	F9 = open("OutputFiles/F9.csv", O_RDONLY, S_IRUSR);

	nBys = read(F9, &bufferF9, sizeof(bufferF9));
	bufferF9[nBys] = '\0';

	int indexF9;
	int i = 0;
	int j = 0;

	for(indexF9 = 0; bufferF9[indexF9] != '\n'; indexF9++);
	indexF9++;

	while(bufferF9[indexF9] != '\0'){
		if(bufferF9[indexF9] == ';'){
			while(bufferF9[indexF9] != '\n'){
				indexF9++;
				pidF9[i] = bufferF9[indexF9];
				i++;
			}
			//printf("Pid %s")
			pid.pid_R[j] = atoi(pidF9);
			j++;
			i = 0;
		}
		indexF9++;
	}

	return pid;
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

  printf("id: %d --> %d\n", 0, semVal[0]);
}


