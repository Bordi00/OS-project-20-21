/// @file pipe.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle PIPE.


#include "pipe.h"


void create_pipe(int filedes[2]){
    if(pipe(filedes) == -1)
        ErrExit("PIPE");
}

void write_pipe(int fd, struct msg message, int pipe[2], int i){
  	ssize_t numWrite;
    
    numWrite = write(pipe[1], &message, sizeof(struct msg));

    if(numWrite != sizeof(struct msg)){
      ErrExit("write pipe failed!");
    }
}