#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct container{
    bool isFull;
    char msg[1024];
}container;

container Container;
//Container.isFull = false;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar_readable = PTHREAD_COND_INITIALIZER;
pthread_cond_t cvar_writeable = PTHREAD_COND_INITIALIZER;
int readerNum = 0;
int writerNum = 0;
int i = 0;

void* reader(void *arg)
{
    readerNum++;
    int readerID = readerNum;
    while(1){
    pthread_mutex_lock(&mutex);
    // üres a tároló, várjuk az írást -> olvasható lesz
    while(!Container.isFull) 
        pthread_cond_wait(&cvar_readable, &mutex);
    
    printf("%d. olvasó: %s\n", readerID, Container.msg);
    sleep(1);
    // kiürítettük a tárolót
    Container.isFull = false;
    pthread_cond_broadcast(&cvar_writeable);
    
    pthread_mutex_unlock(&mutex);
    }
}

void* writer(void *arg)
{
    writerNum++;
    int writerID = writerNum;
    while(1){
    pthread_mutex_lock(&mutex);
    // tároló teli, várjuk, hogy kiolvassák -> írható lesz
    while(Container.isFull)  
        pthread_cond_wait(&cvar_writeable, &mutex);  
    
    i++;
    sleep(1);
    sprintf(Container.msg, "Uzenet: %d\n", i);
    printf("%d. író írt\n", writerID);
    Container.isFull = true;
    // csak egy olvasó szálnak jelzünk
    pthread_cond_broadcast(&cvar_readable);
    
    pthread_mutex_unlock(&mutex); 
    }
}

int main()
{
  int th_writer_num = 1;
  int th_reader_num = 2;
  pthread_t th_write[th_writer_num];
  pthread_t th_read[th_reader_num];
  
  Container.isFull = false;
  pthread_cond_broadcast(&cvar_writeable);

  for(int i=0; i<th_writer_num; i++)
    if(pthread_create(&th_write[i], NULL, writer, NULL))
    {
        fprintf(stderr, "pthread_create (write)");
        exit(EXIT_FAILURE);
    }
  
 
  for(int i=0; i<th_reader_num; i++)
    if(pthread_create(&th_read[i], NULL, reader, NULL))
    {
        fprintf(stderr, "pthread_create (read2)");
        exit(EXIT_FAILURE);
    }
  pthread_join(th_read[0], NULL);
  exit(EXIT_SUCCESS);
}










