/// @file client.c
/// @brief Contiene l'implementazione del client.

#include "defines.h"

int main(int argc, char * argv[]){

	/*
       //ottengo la directory corrente e concateno con la stringa mancante per compatibilità con altri OS
        getcwd(path, PATH_SZ);
        strcat(path, argv[1]); //path contiene il percorso dove risiede F7

        //apro il file da leggere
        int F7 = open(path, O_RDWR, S_IRWXU);

        if(F7 == -1){
            ErrExit("open F7 failed");
        }
      
        ssize_t numRead = read(F7, buffer, BUFFER_SZ); //leggo F7 e salvo il contenuto in buffer
        
        if (numRead == -1)
          ErrExit("read");
        
        if(close(F7) == -1)
          ErrExit("close");

        buffer[numRead] = '\0'; //numRead è l'indice dopo il quale non ci sono altri caratteri letti da F7
      
        int j = 0;
        int i = 0;
        int k = 0;

        //riempo la struct con i messaggi
        for(; i < 5; i++){
          for(k = 0; buffer[j] != '\n'; j++, k++){ //itero stringa per stringa
            msgF7[i].message[k] = buffer[j];       //riempo l'array della struct con la stringa
          }
          msgF7[i].message[k] = '\n'; //alla fine della stringa inserisco il carattere invio
          j++; //vado al prossimo carattere
        }
        msgF7[i].message[j] = '\0';

        //Metto in path il percorso di F7_out
        getcwd(path, PATH_SZ);
        strcat(path, "/OutputFiles/F7_out.csv");

        int F7_out = open(path, O_RDWR | O_CREAT, S_IRWXU);

        if(F7_out == -1){
            ErrExit("open F7_out failed");
        }
  
      ssize_t numWrite;
          
      //scrivo F7 (buffer) in F7_out 
      numWrite = write(F7_out, msgF7[0].message, strlen(msgF7[0].message));  
      if (numWrite != strlen(msgF7[0].message))
        ErrExit("write");

      numWrite = write(F7_out, msgF7[4].message, strlen(msgF7[4].message));  
      if (numWrite != strlen(msgF7[4].message))
        ErrExit("write");

      numWrite = write(F7_out, msgF7[3].message, strlen(msgF7[3].message));
      if (numWrite != strlen(msgF7[3].message))
        ErrExit("write");  

      numWrite = write(F7_out, msgF7[2].message, strlen(msgF7[2].message)); 
      if (numWrite != strlen(msgF7[2].message))
        ErrExit("write"); 

      numWrite = write(F7_out, msgF7[1].message, strlen(msgF7[1].message));  
      if (numWrite != strlen(msgF7[1].message))
        ErrExit("write");
   
      if(close(F7_out) == -1)
        ErrExit("close");

    sleep(2);
    return 0;

		*/

}