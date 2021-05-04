/// @file semaphore.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione dei SEMAFORI.


#include <sys/sem.h>
#include "err_exit.h"
#include "semaphore.h"


void semOp (int semid, struct sembuf sops[3], short sem_op) {
    //struct sembuf sop[sem_num] = { .sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0};

    if (semop(semid, &sops, sem_op) == -1)
        ErrExit("semop failed");
}
