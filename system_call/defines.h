/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "semaphore.h"
#include "err_exit.h"

#define BUFFER_SZ 100
#define PATH_SZ 100
char buffer[BUFFER_SZ + 1];
char path[PATH_SZ + 1];

//contiene il messaggio da scrivere sui file
struct container{
  char id[5];
  char message[50];
  char idSender[3];
  char idReceiver[3];
  char time_arrival[9];
  char time_departure[9];
};

//contiene il messaggio letto da F0
struct msg{
  char id[5];
  char message[50];
  char idSender[3];
  char idReceiver[3];
  char delS1[3];
  char delS2[3];
  char delS3[3];
	char type[5];
};

//contiene il messaggio da scrivere su F10
struct ipc{
  char ipc[10];
  char idKey[10];
  char creator[8];
  char creation[10];
  char destruction[10];
};

//contiene i pid dei processi Receiver e Senders
struct pid{
  int pid_S[3];
  int pid_R[3];
};

//contiene il messaggio letto da F7
struct hackler{
  char id[5];
  char delay[3];
  char target[3];
  char action[25];
};

//contiene le informazioni per inviare i segnali ai processi
struct signal{
  long mtype; //identifica a quale processo appartiene il pid (1:S1, 2:S2, 3:S3, 4:R3, 5:R2, 6:R1)
  int pid;
};


// dichiarazione funzione per riempire la struct con i relativi campi in F0
struct msg fill_structure(char buffer[]);
//dichiarazione funzione che riempie il campo [time_arrival] di msgF1 con l'orario locale
struct container get_time_arrival();
//dichiarazione funzione che riempie il campo [time_departure] di msgF1 con l'orario locale
struct container get_time_departure(struct container msgFile);
//prende tempo di distruzione e creazione delle IPC
struct ipc get_time(struct ipc historical, char flag);
//dichiarazione funzione per la scrittura sul file f1;
void writeFile(struct container msgF1, struct msg message, int fd);
//dichiarazione funzione per la scrittura di F8.csv
void writeF8(int pid_S[3]);
//dichiarazione funzione per la scrittura di F8.csv
void writeF9(int pid_R[3]);
//scrive F10
void writeF10(struct ipc historical, int F10);
//dichiarazione funzione gestione dei segnali
void sigHandler(int sig);
//dichiarazione funzione stampa valori dei semafori
void printSemaphoresValue (int semid);
//dichiarazione funzione inizializzazione strutture contenimento messaggi
struct container init_container(struct container msgFile);
//inizializza struct msg
struct msg init_msg(struct msg message);
//legge i pid da F8
struct pid get_pidF8(struct pid pid);
//legge i pid da F9
struct pid get_pidF9(struct pid pid);
//riempe la struttura hackler
struct hackler fill_hackler_structure(char buffer[]);
//stampa il valore dei semafori (debug)
void printSemaphoresValue (int semid);
