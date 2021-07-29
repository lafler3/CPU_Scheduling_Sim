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

int*** deepCopy(int*** arr, int simN){
	int burstN;
	int aT;
	int*** data = calloc(simN, sizeof(int**));
	for (int i = 0; i < simN; ++i)
	{
		int* aBDC = calloc(3, sizeof(int));
		int temp = arr[i][0][1];
		burstN = temp;
		aT = arr[i][0][0];
		
		*(aBDC + 0) = aT;
		*(aBDC + 1) = burstN;
		*(aBDC + 2) = 0;

		int* tCPUC = calloc(burstN, sizeof(int));
		for (int b = 0; b < burstN; ++b)
		{
			tCPUC[b] = arr[i][1][b];
		}

		int* tIOC = calloc((burstN-1), sizeof(int));
		for (int b = 0; b < burstN - 1; ++b)
		{
			tIOC[b] = arr[i][2][b];
		}

		int** tArrayC = calloc(3, sizeof(int*));

		*(tArrayC+0) = aBDC;
		*(tArrayC+1) = tCPUC;
		*(tArrayC+2) = tIOC;
		*(data+i) = tArrayC;
	}

	return data;

}

void printData(int*** data, int simN){
	for (int i = 0; i < simN; ++i)
	{
		printf("%d, %d, %d \n", data[i][0][0], data[i][0][1], data[i][0][2]);
		int num = data[i][0][1];

		for (int b = 0; b < num; ++b)
		{
			printf(" %d\n", data[i][1][b]);
		}
		for (int b = 0; b < num - 1; ++b)
		{
			printf(" %d\n", data[i][2][b]);
		}
	}
}

void freeData(int*** data, int simN){
	for (int i = 0; i < simN; ++i)
	{
		free(data[i][0]);
		free(data[i][1]);
		free(data[i][2]);
		free(data[i]);
	}
	free(data);
}

void printDataCompare(int*** data1, int*** data2, int simN){
	for (int i = 0; i < simN; ++i)
	{
		printf("%d, %d, %d \n", data1[i][0][0], data1[i][0][1], data1[i][0][2]);
		printf("%d, %d, %d \n", data2[i][0][0], data2[i][0][1], data2[i][0][2]);
		int num = data1[i][0][1];

		for (int b = 0; b < num; ++b)
		{
			printf(" %d\n", data1[i][1][b]);
			printf(" %d\n", data2[i][1][b]);
		}
		for (int b = 0; b < num - 1; ++b)
		{
			printf(" %d\n", data1[i][2][b]);
			printf(" %d\n", data2[i][2][b]);
		}
	}
}

char getProcessName(int process){
	char ALPHABET[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
	char ret = ALPHABET[process];
	return ret;
}

int avg(double total, double processes){
	int ret = ceil(total/processes);
	return ret;
}

void sjf(int*** data, double conSwitch, double alphC){
	double total = ceil(1/alphC);
	double processes = 1;
	int tau = avg(total, processes);

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
	int simN = atoi(argv[1]);
	int seedR = atoi(argv[2]);
	double lambda = strtod(argv[3], NULL);
	int threshED = atoi(argv[4]);
	int conSwitch = atoi(argv[5]);
	double alphC = strtod(argv[6], NULL);
	int timeSlice = atoi(argv[7]);


	srand(seedR);
	//FCFS
	int*** FCFSD = next_exp(lambda, simN, threshED);
	freeData(FCFSD, simN);
	srand(seedR);
	//SJF;
	int*** SJFD = next_exp(lambda, simN, threshED);
	sjf(SJFD, conSwitch, alphC);
	freeData(SJFD, simN);
	

	srand(seedR);
	//SRT;
	int*** SRTD = next_exp(lambda, simN, threshED);
	freeData(SRTD, simN);

	srand(seedR);
	//RR;
	int*** RRD = next_exp(lambda, simN, threshED);
	freeData(RRD, simN);

	return 0;
}