/* Authors */
/* Fernando Candelario Herrero */
/* Cristina Manso de la Viuda  */

#include "mitiempo.h"

typedef struct {
	double ini;
	double fin;
}interval_t;

//Globals Variables
static const char *confFile="config.txt";
static interval_t intervalStartStop,intervalResumePause;
static int mode=-1;
static double totalTime;
static bool pauseEnable,startChrono;
static bool initDone=false;
static clockid_t clocks[] = {CLOCK_REALTIME,CLOCK_MONOTONIC,CLOCK_PROCESS_CPUTIME_ID,CLOCK_THREAD_CPUTIME_ID,CLOCK_MONOTONIC_RAW,CLOCK_REALTIME_COARSE};


int init()
{
	int fd;
	char c[2];

	if( (fd=open(confFile,O_RDONLY)) < 0 ){
		perror("Error executing open...\n");
		return -1;	
	}
	
	//Expect to read only one byte, if 2 bytes was readed, there is more than one byte in the conf file.
	if( read(fd,c,2) != 1 ){
		perror("Execution of read fails, maybe bad file format, only one byte... \n");
		return -1;
	}

	c[0] -= 48;

	if ( (c[0] < 0 || c[0] >= MAX_OPTIONS) ){
		perror("Number of file differs [0-5]  \n");
		return -1;
	}

	mode = c[0];
	initDone = true;
	startChrono = false;
	
	close(fd);

	return  1;
}

int start()
{
	
	if( !initDone ){
		perror("You have to use init before!!");
		return -1;
	}

	struct timespec tp;

	if( clock_gettime(clocks[mode], &tp ) < 0 ){
		perror("clock_gettime fails");
		return -1;
	}

	//Guardar el tiempo en microsegundos.
	intervalResumePause.ini = intervalStartStop.ini = (double)( ( tp.tv_sec * 1E6 ) + (tp.tv_nsec * 1E-3) );
	totalTime = 0;
	startChrono = true;
	pauseEnable = false;
	
	return 1;
}

int pause()
{

	if( !initDone ){
		perror("You have to use init before!!");
		return -1;
	}
	
	if( !startChrono ){
		perror("You have to use start before!!");
		return -1;	
	}

	
	if( !pauseEnable ){
		
		struct timespec tp;

		if( clock_gettime(clocks[mode], &tp ) < 0 ){
			perror("clock_gettime fails");
			return -1;
		}

		//Guarda el tiempo en microsegundos.
		intervalResumePause.fin = intervalStartStop.fin = (double)( ( tp.tv_sec * 1E6 ) + (tp.tv_nsec * 1E-3) );
		totalTime += intervalStartStop.fin - intervalStartStop.ini;
		pauseEnable = true;
		
		return (int) round(intervalResumePause.fin - intervalResumePause.ini );

	}
	
	return 1;
}

int resume()
{
	
	if( !initDone ){
		perror("You have to use init before!!");
		return -1;
	}
	
	if( !startChrono ){
		perror("You have to use start before!!");
		return -1;	
	}

	
	if( pauseEnable ){
		
		struct timespec tp;

		if( clock_gettime(clocks[mode], &tp ) < 0 ){
			perror("clock_gettime fails");
			return -1;
		}

		intervalResumePause.ini = intervalStartStop.ini = (double)( ( tp.tv_sec * 1E6 ) + (tp.tv_nsec * 1E-3) );
		pauseEnable = false;
	
	}
	
	return 1;

}

int stop()
{

	if( !initDone ){
		perror("You have to use init before!!");
		return -1;
	}
	
	if( !startChrono ){
		perror("You have to use start before!!");
		return -1;	
	}

	
	if( pauseEnable ){
		perror("use resume first to stop the clock");
		return -1;
	}
	
	struct timespec tp;

	if( clock_gettime(clocks[mode], &tp ) < 0){
		perror("clock_gettime fails");
		return -1;
	}

	//Guardar el tiempo en microsegundos.
	intervalStartStop.fin = (double)( (tp.tv_sec * 1E6) + (tp.tv_nsec * 1E-3) );
	startChrono = false;
	
	return (int) round(intervalStartStop.fin - intervalStartStop.ini + totalTime);

}

