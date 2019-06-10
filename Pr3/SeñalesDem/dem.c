/* Authors */
/* Fernando Candelario Herrero */
/* Cristina Manso de la Viuda  */

#include "dem.h"

/* Prototipes */
static void alarm_action();
static void sighup_action();
static void sigterm_action();
static void ini_handler(int signal, void *handler);
static int countDigits(int n);
static int read_conf();
static int create_witness();
static int setupDaemon();
static int configure_alarm();
static int removeWitnessFile();
static char * iToa(int n, int *lenght);

/* Globals Variables */
static bool killDaemon=false;
static volatile int segundos=-1;
static const char * fileConf=".tiempo.conf";
static const char * fileWitness=".tiempo.lock";


/****************Handlers*****************/
static void alarm_action(){

	syslog(LOG_CRON|LOG_NOTICE," ");
}


static void sigusr1_action(){

	syslog(LOG_CRON|LOG_NOTICE,"Signal SIGUSR1 recived");
}


static void sighup_action(){
  	  
	if(configure_alarm()!=0)
		syslog(LOG_ERR,"Error in alarm config, handler");
	
}

static void sigterm_action(){
  	  
	killDaemon = true;
}


static void ini_handler(int signal, void *handler){
	struct sigaction act;
    struct sigaction old_act;

    act.sa_handler = handler;

    sigemptyset(&act.sa_mask);

    act.sa_flags = SA_RESTART;

    sigaction(signal, &act, &old_act);
}

/************AuxFunctions****************/
static int countDigits(int n){
	int c=1;

	while( (n/=10) != 0 )
		c++;
	
	return c;
}

static char * iToa(int n, int *lenght){
	int numDigits = countDigits(n);
	char * asciiPid = (char*) malloc(sizeof(char)*numDigits+1);

	//Passing PID to ASCII code
	for (int i = numDigits-1; i > -1; --i){
		asciiPid[i]=(char)( (n%10)+(char)48 );
		n/=10;
	}
	asciiPid[numDigits] = '\0';
	*(lenght) = numDigits+1;

	return asciiPid;
}

/*****************FunctionsToManageProgramsFiles*****************/
static int read_conf(){
	char dirOfFile[MAX_STRING];
	char *dir;
	char *sec;
	FILE * fd;
	int n;
	size_t maxString;

	if( (dir=getenv("HOME")) == NULL ){
		syslog(LOG_ERR,"Failed during getenv");
		return -1;
	}

	strncpy(dirOfFile, dir, (size_t)MAX_STRING);
	strncat(dirOfFile, "/", (size_t)MAX_STRING);
	strncat(dirOfFile, fileConf, (size_t)MAX_STRING);

	if( (fd=fopen(dirOfFile,"r")) < 0){
		syslog(LOG_ERR,"Failed during open");
		return -1;
	}

	maxString=MAX_STRING;
	sec = (char*) malloc(sizeof(char)*MAX_STRING);
	getline(&sec,&maxString,fd);
	n = atoi((const char*)sec);
	free(sec);

	return n;
}

static int create_witness(){
	char dirOfFile[MAX_STRING];
	char *dir;
	char *asciiPid;
	int fd;
	pid_t pid;
	int numDigits;

	if( (dir=getenv("HOME")) == NULL ){
		syslog(LOG_ERR,"Failed during getenv");
		return -1;
	}

	strncpy(dirOfFile, dir, (size_t)MAX_STRING);
	strncat(dirOfFile, "/", (size_t)MAX_STRING);
	strncat(dirOfFile, fileWitness, (size_t)MAX_STRING);

	if( (fd=open(dirOfFile,O_CREAT|O_WRONLY|O_TRUNC|O_EXCL)) < 0){
		syslog(LOG_ERR,"Failed during open");
		return -1;
	}

	pid=getpid();
	asciiPid = iToa((int)pid,&numDigits);

	write(fd,asciiPid,sizeof(char)*numDigits);
    write(fd,"\n",sizeof(char));
	close(fd);
	free(asciiPid);

	return 0;
}

static int removeWitnessFile(){
	char dirOfFile[MAX_STRING];
	char *dir;

	if( (dir=getenv("HOME")) == NULL ){
		syslog(LOG_ERR,"Failed during getenv");
		return -1;
	}

	strncpy(dirOfFile, dir, (size_t)MAX_STRING);
	strncat(dirOfFile, "/", (size_t)MAX_STRING);
	strncat(dirOfFile, fileWitness, (size_t)MAX_STRING);

	if( (remove(dirOfFile)) < 0){
		syslog(LOG_ERR,"Failed during remove %s",strerror(errno));
		return -1;
	}

	return 0;
}

/**************SignalsFunctions*************/
static int setupDaemon(){
	sigset_t grupo,old_grupo;

	close(0);
	close(1);
	close(2);

	sigfillset(&grupo);
	sigdelset(&grupo, SIGALRM);
	sigdelset(&grupo, SIGHUP);
	sigdelset(&grupo, SIGUSR1);
	sigdelset(&grupo, SIGTERM);
	sigprocmask(SIG_BLOCK,&grupo,&old_grupo);

	umask(0);
	if(setsid() == -1){
		syslog(LOG_ERR,"Setsid error");
		return -1;
	}

	if(chdir("/") < 0){
		syslog(LOG_ERR,"Chdir error");
		return -1;
	}

	return 0;
}



static int configure_alarm(){
	  struct itimerval it;
	  int segAux;

	  if( (segAux = read_conf()) < 1){
	  	syslog(LOG_ERR,"Error reading conf file, 0 and negative numbers its not supported");
	  	return -1;	
	  }
	 
	  if(segAux!=segundos){
	  	segundos=segAux;
  		it.it_value.tv_sec = segundos;
	  	it.it_value.tv_usec = 0;
	  	it.it_interval.tv_sec = segundos;
	  	it.it_interval.tv_usec = 0;
	 
	 	if(setitimer(ITIMER_REAL,&it,NULL)==-1){
	  		syslog(LOG_ERR,"setitimer fails");
	  		return -1;
	 	}
	 
	 	syslog(LOG_INFO,"The freq of Time Notifications is now updated, freq: %i secs",segundos);
	  	
	  }
	  
	  return 0;

}


int start_dem(){


  	  if(setupDaemon() == -1){
  	  	syslog(LOG_ERR,"Setup failed");
  	  	exit(-1);
  	  }

  	  
	  if(create_witness()!=0){
	  	syslog(LOG_ERR,"Error creating the witness file");
	  	 exit(-1);
	  }

	  /*Señal SIGALRM*/
  	  ini_handler(SIGALRM, alarm_action);
  	  if(configure_alarm()!=0){
	  	syslog(LOG_ERR,"Error in alarm config");
	  	exit(-1);
	  }

	  /*Señal SIGHUP*/
	  ini_handler(SIGHUP, sighup_action);
  	  
	  /*Señal SIGUSR1*/
  	  ini_handler(SIGUSR1, sigusr1_action);

	  /*SIGTERM*/  	  
	  ini_handler(SIGTERM, sigterm_action);

	  while(!killDaemon);

	  removeWitnessFile();
	  exit(0);

}


int main(){
	pid_t pid=fork();
	switch(pid){
		case -1:
			syslog(LOG_ERR,"fork failed %s",strerror(errno));
			exit(-1);
		case 0:
			start_dem();
			break;
		default:
			exit(0);
	}

}
