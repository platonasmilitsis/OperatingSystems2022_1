#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#include "helperfunctions.h"

//Function to initialize command line arguments
int initializeArguments(int argc,char *argv[]){
	if (argc != 4){								
		printf("Invalid number of args, exiting\n");
		return -1;
	}
	
    return 1;
}

//Function to find one line from file and print it
void OneLineGetter(FILE *fp,int *sh,int ThisLine){
	char lines[1024];
	int total=1;
	while(fgets(lines,sizeof(lines),fp)!=NULL){
		if(ThisLine==total){
			lines[strlen(lines)-1]='\0';
			strcpy((char*) (sh+sizeof(int)+ 3*sizeof(sem_t)),lines);
			printf("Parent reads line %d ---> ( %s  )\n",ThisLine, lines);
			return;
		}
		total++;
	}
	
}