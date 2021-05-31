/// @file fifo.h
/// @brief Contiene la definizioni di variabili e
///         funzioni specifiche per la gestione delle FIFO.

#pragma once
#include <unistd.h>
#include <sys/stat.h>



void remove_fifo(const char *pathname, int fd);
