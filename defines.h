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
#include "err_exit.h"

#define BUFFER_SZ 1000
#define PATH_SZ 100
char buffer[BUFFER_SZ + 1];
char path[PATH_SZ + 1];

struct container{
  char id[5];
  char message[50];
  char idSender[2];
  char idReceiver[2];
  char time_arrival[9];
  char time_departure[9];
};


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

struct msg_row{
	char message_row[100];
}row[3];

// dichiarazione funzione per riempire la struct con i relativi campi in F0
struct msg fill_structure(char buffer[],int j);
//dichiarazione funzione che riempie il campo [time_arrival] di msgF1 con l'orario locale
struct container get_time_arrival(struct container msgFile);
//dichiarazione funzione che riempie il campo [time_departure] di msgF1 con l'orario locale
struct container get_time_departure(struct container msgFile);
//dichiarazione funzione per la scrittura sul file f1;
void writeFile(struct container msgF1, struct msg message, int fd);
//dichiarazione funzione per la scrittura di F8.csv
void writeF8(int pid_S[3]);

void sigHandler(int sig);
