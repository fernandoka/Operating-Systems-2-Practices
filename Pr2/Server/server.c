#include "server.h"

/* Authors */
/* Fernando Candelario Herrero */
/* Cristina Manso de la Viuda  */

typedef struct {
	pid_t pid;
	int pipe0;
	int pipe1;
}client_pipe_t;

/* Prototipes */
static int setupDaemon();
static void check_pipilines();
static int countDigits(int n);
static void sigterm_action();
static int create_witness();
static int removeFiles();
static void iniListofClients();
static int searchFirstEmptyPosition();
static char* iToa(int n, int * lenght);
static int configure_alarm();
static int read_conf();
static void ini_handler(int signal, void *handler);
static void closePipes();
static void sigpipe_action();
static void deleteClient(int i);
static void sighup_action();

/* Globals Variables */
static volatile bool killServer = false;
static volatile bool brokenPipe = false;
static volatile int numClients = 0;
static int newClientPipe;

static volatile int segundos=-1;

static const char * fileConf =".server.conf"; // File where the freq of alarm is stored
static const char * fileWitness =".server.lock"; // File where the PID of the server process will be save.
static const char * newClientName ="/tmp/newclient";
static const char * toClientName ="/tmp/toClient";
static const char * toServerName ="/tmp/toServer";
static const char * connectionAccepted =":)";
static const char * connectionRefused =":(";
static const char * sentinel = "times";

/* Dos tipos de mensajes, los que van precididos por el codigo de chCode son el de la hora actual */
/* Los que van precedidos por el codigo de dCode son la respuesta a la solicitud de tieme de los clientes */
static const int chCode = -1; // current hour Code
static const int dCode = -2; //date Code
static const int closeCode = -3; //to notify the shutdown of the server

static client_pipe_t clients[MAX_CLIENTS];


/****************Handlers*****************/
static void alarm_action(){
	time_t t;
	struct tm *ct;
	int msg[3];
 	syslog(LOG_INFO,"Performing alarm action");

	for (int i = 0; i < MAX_CLIENTS; ++i){
		
		if(clients[i].pid != -1){
			t = time(NULL);
			ct = localtime((const time_t *)&t);
			
			msg[0] = ct->tm_hour; msg[1] = ct->tm_min; msg[2] = ct->tm_sec;

			if( (write(clients[i].pipe1,&chCode,sizeof(int)) == -1 || write(clients[i].pipe1,(const void *)msg,sizeof(int)*3) == -1) && brokenPipe )
				deleteClient(i);
			
		}

	}
}

static void sigterm_action(){
 	syslog(LOG_INFO,"Performing sigterm action");

	killServer = true;
}


static void sigpipe_action(){
	syslog(LOG_INFO,"Performing sigpipe action");

	brokenPipe = true;	
}

static void sighup_action(){
  	  
	if(configure_alarm()!=0)
		syslog(LOG_ERR,"Error in alarm config, handler");
	
}

static void ini_handler(int signal, void *handler){
	struct sigaction act;
    struct sigaction old_act;

    act.sa_handler = handler;

    sigemptyset(&act.sa_mask);

    act.sa_flags = SA_RESTART;

    sigaction(signal, &act, &old_act);
}



/****************ServerMethod*************/

static void check_pipilines(){
	fd_set conjunto;
    struct timeval timeout;
    int cambios;
	int max=0;
	int i;
	int pos;
	int pidOfNewClient;
	char auxString[MAX_STRING];
	char *asciiPid;
	int numDigits;
	time_t t;
	int auxPipe;

	if ( (newClientPipe = open(newClientName,O_RDONLY/*|O_NONBLOCK*/)) == -1 ){
		syslog(LOG_ERR,"Failed during open fifo  file, %s",strerror(errno));
		exit(-1);
	}

	syslog(LOG_INFO,"numClients %i ",numClients);

	while(!killServer){
	
		FD_ZERO(&conjunto);
		FD_SET(newClientPipe, &conjunto);
		max = newClientPipe;

		// incluir resto de descriptores en el conjunto
		for (i=0;i < MAX_CLIENTS;i++) {
			if(clients[i].pid !=-1){
				FD_SET(clients[i].pipe0, &conjunto);

				if(clients[i].pipe0>max)
					max = clients[i].pipe0;
			}
		}

		timeout.tv_sec = SELECT_TIMEOUT_SEC;
		timeout.tv_usec = SELECT_TIMEOUT_USEC;

		cambios = select(max+1, &conjunto, NULL, NULL, &timeout);

		if (cambios == 0) 
			syslog(LOG_CRON | LOG_NOTICE," timeout of select expired... \n");
		else if(cambios == -1){
			syslog(LOG_ERR,"Failed during select, closing...");
			killServer = true;
		}
		else{
			
			if( FD_ISSET (newClientPipe, &conjunto) &&  read( newClientPipe,&pidOfNewClient,sizeof(int) ) > 0 ){
				
				asciiPid = iToa(pidOfNewClient,&numDigits);
				strncpy(auxString,toClientName,MAX_STRING);
				strncat(auxString, asciiPid, (size_t)MAX_STRING);
				auxPipe = open(auxString,O_WRONLY);

				if(numClients < MAX_CLIENTS ){
					
					pos = searchFirstEmptyPosition();
					clients[pos].pipe1 = auxPipe;

					if( !(write(auxPipe,connectionAccepted,sizeof(char)*3) == -1) ){
										
						strncpy(auxString,toServerName,MAX_STRING);
						strncat(auxString, asciiPid, (size_t)MAX_STRING);
						
						clients[pos].pipe0 = open(auxString,O_RDONLY);					
	
						clients[pos].pid = (pid_t)pidOfNewClient;
						numClients++;
		
						syslog(LOG_INFO,"New client added, pidOfClient: %i ,numClients: %i",pidOfNewClient,numClients);	
					}
					else{
						syslog(LOG_INFO,"Fail sending confirmation to client,connecion aborted");	
						close(clients[pos].pipe1);
					}
				
				}
				else{
					syslog(LOG_INFO,"Server full, not more clients are admitted, MAX_CLIENTS: %i",MAX_CLIENTS);	
					write(auxPipe,connectionRefused,sizeof(char)*3);// No te importa si al ciente le llega o no ya que tiene un timeout
					close(auxPipe);
				}

				free(asciiPid);
			}
			
				
			for (i = 0; i < MAX_CLIENTS; ++i){
				
				if ( FD_ISSET (clients[i].pipe0, &conjunto) ){
					
					read(clients[i].pipe0,auxString,MAX_STRING);
					
					if( strncmp(auxString,sentinel,MAX_STRING) == 0 ){
						t = time(NULL);
						
						if( (write(clients[i].pipe1,&dCode,sizeof(int)) == -1 || write(clients[i].pipe1,&t,sizeof(time_t)) == -1)  && brokenPipe)
							deleteClient(i);
					}
				}

			}//For
			
		}//Else

	}//While

}


/************AuxFunctions****************/
static int countDigits(int n){
	int c=1;

	while( (n/=10) != 0 )
		c++;
	
	return c;
}

static int searchFirstEmptyPosition(){
	int i=0;
	bool found = false;

	while (!found && i < MAX_CLIENTS){
		found = ( clients[i].pid == -1 );
		if(!found)i++;	
	}
	
	return i;
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

static void iniListofClients(){
	for (int i = 0; i < MAX_CLIENTS; ++i)
		clients[i].pid = -1;
}

static void closePipes(){
	for (int i = 0; i < MAX_STRING; i++){
		
		if(clients[i].pid != -1){
			write(clients[i].pipe1,&closeCode,sizeof(int)); //No te preocupa como acabe el write, el cliente tiene timeout
			close(clients[i].pipe0);
			close(clients[i].pipe1);
		}
	}
	close(newClientPipe);
}


static void deleteClient(int i){
	syslog(LOG_INFO,"Broken pipe closing connection, pid of client: %i",(int)clients[i].pid);
	close(clients[i].pipe1);
	close(clients[i].pipe0);
	clients[i].pid = -1;
	brokenPipe = false;
	numClients--;
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
		syslog(LOG_ERR,"Failed during open, %s",strerror(errno));
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
		syslog(LOG_ERR,"Failed during open, %s",strerror(errno));
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

static int removeFiles(){
	char dirOfFile[MAX_STRING];
	char *dir;

	if( (dir=getenv("HOME")) == NULL ){
		syslog(LOG_ERR,"Failed during getenv");
		return -1;
	}

	strncpy(dirOfFile, dir, (size_t)MAX_STRING);
	strncat(dirOfFile, "/", (size_t)MAX_STRING);
	strncat(dirOfFile, fileWitness, (size_t)MAX_STRING);


	if( (unlink(dirOfFile)) == -1){
		syslog(LOG_ERR,"Failed removing witnessFile %s",strerror(errno));
		return -1;
	}

	
	if( (unlink(newClientName)) == -1){
		syslog(LOG_ERR,"Failed removing newClientFifoFile %s",strerror(errno));
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
	sigdelset(&grupo, SIGPIPE);
	sigdelset(&grupo, SIGTERM);
	sigdelset(&grupo, SIGHUP);
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
	 
	 	syslog(LOG_INFO,"The freq of ALARM is now updated, freq: %i secs",segundos);
	  	
	  }
	  
	  return 0;

}


void start_server(){
	
	
	if(setupDaemon() == -1){
		syslog(LOG_ERR,"Setup failed");
		exit(-1);
	}

	iniListofClients();

	if( create_witness()!=0 ){
		syslog(LOG_ERR,"Error creating the witness file");
		exit(-1);
	}

	if( mkfifo(newClientName,0666) == -1 ){
		syslog(LOG_ERR,"Error creating the fifo file, %s",strerror(errno));
		exit(-1);
	}

	/*SIGTERM*/  	  
	ini_handler(SIGTERM, sigterm_action);

	/*Señal SIGPIPE*/
	ini_handler(SIGPIPE, sigpipe_action);

 	/*Señal SIGHUP*/
  	ini_handler(SIGHUP, sighup_action);
  	  
	/*Señal SIGALRM*/
	ini_handler(SIGALRM, alarm_action);
	if(configure_alarm()!=0){
		syslog(LOG_ERR,"Error in alarm config");
		exit(-1);
	}


	check_pipilines();

	removeFiles();
	closePipes();
 	
 	syslog(LOG_INFO,"closing daemon ");

	exit(0);
}


int main(){
	pid_t pid=fork();
	switch(pid){
		case -1:
			syslog(LOG_ERR,"fork failed %s",strerror(errno));
			exit(-1);
		case 0:
			start_server();
			break;
		default:
			exit(0);
	}

}
