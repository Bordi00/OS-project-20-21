/// @file fifo.c
/// @brief Contiene l'implementazione delle funzioni
///         specifiche per la gestione delle FIFO.

#include "err_exit.h"
#include "fifo.h"


void create_fifo(const char *pathname, int flags){
	if(mkfifo(pathname, flags) == -1)
		ErrExit("create fifo failed");
}

void remove_fifo(const char *pathname, int fd){
	close(fd);
	unlink(pathname);
}