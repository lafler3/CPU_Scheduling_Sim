//Project File Opsys
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

int next_exp(){
	//had to restart
}

void FCFS(){

}

void SJF(){

}

void SRT(){

}

void RR(){

}

int main(int argc, char * argv[]){
	int simN = argv[1];
	int seedR = argv[2];
	double lambda = argv[3];
	int threshED = argv[4];
	double conSwitch = argv[5];
	double alphC = argv[6];
	int timeSlice = argv[7];

	srand(seedR);
	FCFS();
	srand(seedR);
	SJF();
	srand(seedR);
	SRT();
	srand(seedR);
	RR();

}