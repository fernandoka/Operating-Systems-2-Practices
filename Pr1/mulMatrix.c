/* Authors */
/* Fernando Candelario Herrero */
/* Cristina Manso de la Viuda  */

#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

//Biblioteca Chrono
#include "lib/mitiempo.h"

#define MAX_STRING_LENGTH 10
#define MAX_RANDOM 100


void mul(int *A, int *B, int *C,int hA,int wA, int wB){
	
	for (int i = 0; i < hA; ++i)
		for (int j = 0; j < wB; ++j)
			for (int k = 0; k < wA; ++k)
				C[i*wB+j]+=A[i*wA+k]*B[k*wB+j];				
}

void print_matrix(int *M, int hM, int wM, char name){
	int i,j;
	printf("\nMatrix: %c %ix%i\n\n",name,hM,wM );

	for (i=0; i<hM; i++){
		for (j=0; j<wM; j++)
			printf("%i ", M[i*wM+j]);
		printf("\n");
	}
	printf("\n");
}


void init_matrix(int *M, int hM, int wM){
	
	for (int i = 0; i < (hM*wM); ++i)
			M[i]=( (int)(rand()%MAX_RANDOM) );		
	
}

bool checkArguments(int argc,char const *argv[], bool *showMatrix, int *hB, int *wA ,int *wB, int *hA){
	int a;
	const char *v = "H:w:W:v";
	
	while( (a = getopt(argc,(char* *const)argv,v)) > 0){
	
		switch( (char)a ){
			case 'H':
				*(hA) = atoi(optarg);
				break;
			case 'w':
				*(hB) = *(wA) = atoi(optarg);
				break;
			case 'W':
				*(wB) = atoi(optarg);
				break;
			case'v': 
				*(showMatrix) = true;
				break;
			
			default:
				perror("USAGE: ./exec -H <val> -w <val> -W <val>");
				exit(EXIT_FAILURE);
		}
	}

	return *(hA) < 0 || *(wB) < 0 || *(hB) < 0 ;
}

int main(int argc, char const *argv[]){
		
	// Matrix variables
	int *A, *B, *C;
	int hA, wA, hB, wB,size_A,size_B,size_C;

	bool showMatrix = false;

	hB = wA = wB = hA = -1;
	
	if( checkArguments(argc,argv,&showMatrix, &hB, &wA ,&wB, &hA) ){
		perror("USAGE: ./exec -H <val> -w <val> -W <val>");
		exit(EXIT_FAILURE);
	}

	srand(time(NULL));
	
	/* Ini */
	init();
	
	// Mallocs A, B, C
	size_A = wA * hA;
	size_B = wB * hB;
	size_C = wB * hA;
	A = (int*)malloc(size_A*sizeof(int));
	B = (int*)malloc(size_B*sizeof(int));
	C = (int*)malloc(size_C*sizeof(int));
	
	/* The clock starts */
	start();
	//Init A
	init_matrix(A, hA, wA);
	printf("\nElapsed time to fill the matrix A with random numbers: %i us\n",pause());

	resume();
	//Init B
	init_matrix(B, hB, wB);
	printf("\nElapsed time to fill the matrix B with random numbers: %i us\n",pause());
	
	resume();
	//Init C
	for (int i= 0; i < size_C; i++) C[i] = 0;
	printf("\nElapsed time to fill the matrix C with 0: %i us\n",pause());
	
	//Mul
	resume();
	mul(A,B,C,hA,wA,wB);
	printf("\nElapsed time to process the multiplicaction: %i us\n",pause());
	
	resume();
	if(showMatrix){
		print_matrix(A,hA, wA,'A');
		print_matrix(B,hB, wB,'B');
		print_matrix(C,hA, wB,'C');
		printf("\nElapsed time to print the matrixs: %i us\n",pause());
		resume();
	}

	printf("\nTOTAL TIME: %i us\n",stop());

    free(A);
    free(B);
    free(C);
    
	return 0;
}
