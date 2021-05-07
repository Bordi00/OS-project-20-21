/// @file receiver_manager.c
/// @brief Contiene l'implementazione del receiver_manager.

#include "err_exit.h"
#include "defines.h"
#include "shared_memory.h"
#include "semaphore.h"
#include "fifo.h"
#include "pipe.h"
#include <sys/shm.h>
#include <sys/msg.h>

/*
invece di creare tutti e tre i processi tramite un ciclo for
potremmo creare un processo alla volta e fargli fare il suo dovere
intanto il padre aspetta che finisca il figlio, quando il figlio finisce
il padre riprende e crea un altro figlio.
*/


int main(int argc, char * argv[]) {
  struct mymsg{
    long mtype;
    struct msg m_message;
  }m;

  key_t msgKey = 01110001;
  int msqid = msgget(msgKey, IPC_CREAT | S_IRUSR | S_IWUSR);

  if(msqid == -1){
    ErrExit("msgget failed");
  }
  ssize_t internal_mSize = sizeof(struct mymsg) - sizeof(long);

while(1){
  if(msgrcv(msqid, &m, internal_mSize, 0, IPC_NOWAIT) == -1){

  }else{
    printf("receiver_manager message %s\n", m.m_message.message);
  }
}
}
