/// @file pipe.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle PIPE.

#pragma once
#include <unistd.h>
#include "defines.h"

void create_pipe(int filedes[2]);
void write_pipe(int fd, struct msg message, int pipe[2], int index);