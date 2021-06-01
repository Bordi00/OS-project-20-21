/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.

#include <errno.h>
#include "err_exit.h"
#include "semaphore.h"


void semOp (int semid, unsigned short sem_num, short sem_op) {
    struct sembuf sop = {sop.sem_num = sem_num, sop.sem_op = sem_op};

    if (semop(semid, &sop, 1) == -1) {
      if(errno != EINVAL)
        ErrExit("semop failed");
    }
}
