#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#include "helperfunctions.h"

int main(int argc, char**argv){
    int sumlines=atoi(argv[1]);
    int deals=atoi(argv[2]);
    int semoff,rsemoff,wsemoff;
    int *sh0,*shmem;
    int err=0;

    sem_t *sem1,*rsem,*wsem;

    srand((int)time(NULL) % getpid());
    //Attach
    shmem = (int *)shmat(atoi(argv[3]),(void *) 0 ,0);
	if (*shmem == -1) perror ("Attachment");
	sh0 = shmem;

    //setting offset for semaphores
    semoff = sizeof(int);
    sem1 = (sem_t *) (sh0 + semoff);

    rsemoff = sizeof(int) +sizeof(sem_t);
    rsem = (sem_t *) (sh0 + rsemoff);

    wsemoff = sizeof(int) +2*sizeof(sem_t);
    wsem = (sem_t *) (sh0 + wsemoff);

    clock_t begin,end;
    for(int i=0;i<deals;i++){
        //start counting time
        begin = clock() ;
        sem_wait(sem1);
        *shmem=rand()%sumlines;
        printf("Child with PID %d asks for line : %d\n",getpid(), *shmem);
        sem_post(rsem);
        sem_wait(wsem);
        printf("Child with PID %d returns : %s\n\n",getpid(), (char*) (shmem + sizeof(int) +3*sizeof(sem_t)));
        sem_post(sem1);
        //stop counting time
        end = clock();
    }
    //taking average time and print it
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    double average_time_spent = time_spent/deals; 
    printf("\n Average time for Process %d is %f seconds \n\n", getpid(),average_time_spent);


    //detach
    err = shmdt((void *) shmem);
    if(err == -1) perror("Detach child");
}