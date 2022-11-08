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



int main(int argc, char **argv){
   
    //Check the validity of command line arguments
	if(initializeArguments(argc,argv)==-1){
		return -1;
	}
    int K,N;
    char *X=argv[1];
    K=atoi(argv[2]);
    N=atoi(argv[3]);
    //Open the file
    FILE *fp=fopen(argv[1], "r");
    //File Not Found
    if(!fp){
        perror("File Not Found\n");
        exit(-1); 
    }
    char line[1024];
    int total_lines = 0;
    while(fgets(line,sizeof(line),fp )!= NULL){
        total_lines++;
    }
    if ( fseek(fp, 0L, SEEK_SET) != 0 ) {
        printf("fseek error\n");
    }
    pid_t pid;
    int shm_id,err,sem_id,value;
    int *sh;
    sem_t *read_sem, *sem, *write_sem;    

    //SHARED MEM
    // for view of shared mem check README size of int +semaphores + pinaka[101] * sizeof(char)
	shm_id = shmget(IPC_PRIVATE, sizeof(int) + 3*sizeof(sem_t) + 101*sizeof(char)  , IPC_CREAT | 0660);
	if(shm_id == -1) perror("Shared Memory Creation error");

    //Attach to shared memory
    sh = shmat(shm_id,NULL, 0);
    if (sh == NULL) perror("Shared memory attach error");
  

    //first time continues->val=1
    sem = (sem_t*) (sh + sizeof(int));
    value=sem_init(sem,1,1);
    if ( value != 0){
        perror ("Couldn 't initialize sem.") ; 
    }
    //first time waits->val=0
    read_sem = (sem_t * ) (sh + sizeof(int) +sizeof(sem_t));
    value=sem_init(read_sem,1,0);
    if ( value != 0){
        perror ("Couldn 't initialize read_sem.") ; 
    }
    
    //first time waits->val=0
    write_sem=(sem_t *) (sh + sizeof(int) + 2*sizeof(sem_t));
    value=sem_init(write_sem,1,0);
    if ( value != 0){
        perror ("Couldn 't initialize write_sem.") ; 
    }

    for(int i=0; i<K; i++){
        pid = fork();
		//Chlid process
		if(pid==0){
            char shbuf[1024];
            char numlines[12];
			char recycles[12];
            memset(shbuf,0,1024);
            sprintf(shbuf,"%d",shm_id);
			memset(numlines,0,12);
			sprintf(numlines,"%d",total_lines);
			memset(recycles,0,12);
			sprintf(recycles,"%d",N);

			char *args[]={"./child",numlines,recycles,shbuf,NULL};

            if(execv(args[0],args)==-1){
                perror("Execv error");
                exit(127);
            }

        }
        
    }
    
    for(int i=0;i<K*N;i++){
        err=sem_wait(read_sem);
        if(err==-1) printf("Semaphore error\n"); 
        OneLineGetter(fp,sh,*sh);
        if ( fseek(fp, 0L, SEEK_SET) != 0 ) {
            printf("fseek error\n");
        }
        sem_post(write_sem);
    }


    //wait for all children to terminate and print exit codes of children
    int status;
    for (int i = 0; i < K; i++) {
        pid_t wpid = waitpid(-1, &status, 0);
        if (WIFEXITED(status))
            printf("Child %d terminated with Exit Code %d\n", wpid, WEXITSTATUS(status));
        else
            printf("Child %d terminated abnormally\n", wpid);
    }

    //detach
    err=shmdt(sh);
    if(err == -1) perror("detach error");

    //delete shared mem segment
	err = shmctl(shm_id, IPC_RMID, 0);
	if(err == -1) perror("delete shared memory error");

    //destroy all semaphores
    sem_destroy(sem);
	sem_destroy(read_sem);
	sem_destroy(write_sem);
    
    //closing the text file
    fclose(fp);
    
    return 0;
}