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

struct hackler fill_hackler_structure(char buffer[],int j){
	struct hackler message = { };

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
				message.delay[index] = buffer[j];
				index++;
				break;
			case 2: // idSender
				if(buffer[j] != ';'){
					message.target[index] = buffer[j];
					index++;
				}else{
					strcpy(message.target, "");
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

	numWrite = write(F10, historical.ipc, strlen(historical.ipc));
	if (numWrite != strlen(historical.ipc)){
		ErrExit("write ipc");
	}

	numWrite = write(F10, ";", sizeof(char));
	if (numWrite != sizeof(char)){
		ErrExit("write");
	}

	numWrite = write(F10, historical.idKey, strlen(historical.idKey));
	if (numWrite != strlen(historical.idKey)){
		ErrExit("write key");
	}

	numWrite = write(F10, ";", sizeof(char));
	if (numWrite != sizeof(char)){
		ErrExit("write");
	}

	numWrite = write(F10, historical.creator, strlen(historical.creator));
	if (numWrite != strlen(historical.creator)){
		ErrExit("write creator");
	}

	numWrite = write(F10, ";", sizeof(char));
	if (numWrite != sizeof(char)){
		ErrExit("write");
	}

	numWrite = write(F10, historical.creation, strlen(historical.creation));
	if (numWrite != strlen(historical.creation)){
		ErrExit("write creation");
	}

	numWrite = write(F10, ";", sizeof(char));
	if (numWrite != sizeof(char)){
		ErrExit("write");
	}

	numWrite = write(F10, historical.destruction, strlen(historical.destruction));
	if (numWrite != strlen(historical.destruction)){
		ErrExit("write destruction");
	}

	numWrite = write(F10, "\n", sizeof(char));
	if (numWrite != sizeof(char)){
		ErrExit("write");
	}

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
    for (int i = 0; i < 1; i++)
        printf("id: %d --> %d\n", semid, semVal[i]);
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
	char bufferF8[50];
	char pidF8[5];

	do{
		F8 = open("OutputFiles/F8.csv", O_RDONLY, S_IRUSR);
	}while(F8 == -1);

	read(F8, &bufferF8, sizeof(bufferF8));

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
	char bufferF9[50];
	char pidF9[5];

	do{
		F9 = open("OutputFiles/F9.csv", O_RDONLY, S_IRUSR);
	}while(F9 == -1);

	read(F9, &bufferF9, sizeof(bufferF9));

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

struct list_t *new_list()
{

    struct list_t *list = (struct list_t *)malloc(sizeof(struct list_t));

    list->head = NULL;

    return list;
}

void insert_into_list(struct list_t *list, int value)
{

    struct node_t *node = (struct node_t *)malloc(sizeof(struct node_t));
    struct node_t *current = list->head;
    struct node_t *prec = current;

    if (list->head == NULL || value < (current->value))
    { //se sono al primo inserimento o se il nodo entrante è piu piccolo del primo nodo inserisco in testa
        node->value = value;
        node->next = list->head;
        list->head = node;
        return;
    }
    else
    {
        current = current->next; //altrimenti incremento current perchè il nodo è gia stato confrontato con la testa
        while (current != NULL)
        { //scorro la lista fino alla fine, se prima c'era un solo nodo esco direttamente
            if (value > (prec->value) && value <= (current->value))
            { //altrimenti guardo se il nodo entrante deve essere messo tra due nodi
                node->value = value;
                prec->next = node;
                node->next = current;
                return;
            }
            prec = current; //aggiorno i puntatori
            current = current->next;
        }
    }

    //se sono qui, o c'era un solo nodo in lista oppure è entrato un nodo maggiore degli altri in entrambi i casi va in coda
    node->value = value;
    prec->next = node;
    node->next = NULL;
}
