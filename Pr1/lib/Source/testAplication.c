#include "mitiempo.h"



void delay(int n){
	for (int i = 0; i < n; ++i)
		for (int i = 0; i < n; ++i)
			for (int i = 0; i < (n>>1); ++i);
}


int main(int argc, char const *argv[]){
	

	if(argc != 2){
		printf("./exec num \n");
		return 1;
	}

	init();
	
	start(); 
	start(); 

	delay( atoi(argv[1]) );
	 
	printf("Time nº1 pause: %ius\n", pause());//algo
	//printf("Time nº1 pause: %ius\n", pause());//algo

	printf("resume: %i\n",resume());
	//printf("resume: %i\n",resume());

	printf("Time nº2 pause: %ius\n", pause());//poco 
	
	//printf("Time: %ius\n", stop());//Peta!!
	
	printf("resume: %i\n",resume());
	
	delay( atoi(argv[1]) );
	
	printf("Time nº3 pause: %ius\n", pause());//Algo 
	printf("resume: %i\n",resume());

	printf("Time: %ius\n", stop());
	
	return 0;
}