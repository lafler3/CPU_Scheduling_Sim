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

int *** next_exp(double lambda, int simN, int threshED){
	int*** data = calloc(simN, sizeof(int**));
	for(int i = 0; i<simN; i++){
		int aT = threshED+1;
		while(aT > threshED){
			double r = drand48();
			aT=floor(-log(r)/lambda);
		}
		int burstN = ceil(drand48()*100);
		int** tArray = calloc(3, sizeof(int*));
		int* aBD = calloc(3, sizeof(int));
		*(aBD + 0) = aT;
		*(aBD + 1) = burstN;
		*(aBD + 2) = 0;
		int* tCPU = calloc(burstN, sizeof(int));
		int* tIO = calloc((burstN-1), sizeof(int));
		int cTr = 0;
		int iTr = 0;
		for(int j = 0; j<(burstN+burstN-1); j++){
				if(j%2 == 0){
					*(tCPU + cTr) = ceil(drand48()*100);
					cTr++;
				}else{
					*(tIO + iTr) = ceil(drand48()*100*10);
					iTr++;
				}
		}
		*(tArray+0) = aBD;
		*(tArray+1) = tCPU;
		*(tArray+2) = tIO;
		*(data+i) = tArray;
	}
	return data;
}
/*
next_exp index: data[][][]
data[] determines which process: data[0] = A, data[25] = Z
data[][] determines data type: data[][0] = arrival time and cpu busts IN THAT ORDER,
Third thing is to track which cpu processes are done
data[][1] = CPU burst times IN ORDER, data[][2] = I/O add times IN ORDER
data[][][] refers to each indivisual value
*/
int main(int argc, char * argv[]){
	int simN = argv[1];
	int seedR = argv[2];
	double lambda = argv[3];
	int threshED = argv[4];
	double conSwitch = argv[5];
	double alphC = argv[6];
	int timeSlice = argv[7];

	srand(seedR);
	//FCFS
	int*** FCFSD = next_exp(lambda, simN, threshED);

	srand(seedR);
	//SJF;
	int*** SJFD = next_exp(lambda, simN, threshED);

	srand(seedR);
	//SRT;
	int*** SRTD = next_exp(lambda, simN, threshED);

	srand(seedR);
	//RR;
	int*** RRD = next_exp(lambda, simN, threshED);

	return 0;
}