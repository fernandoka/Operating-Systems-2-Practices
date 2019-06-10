/* Authors */
/* Fernando Candelario Herrero */
/* Cristina Manso de la Viuda  */

#include "server.h"

/* Prototipes */
static int setupDaemon();
static void check_queue();
static int countDigits(int n);
static void sigterm_action();
static int create_witness();
static int removeFile();
static void iniListofClients();
static int searchFirstEmptyPosition();
static char* iToa(int n, int * lenght);
static int configure_alarm();
static int read_conf();
static void ini_handler(int signal, void *handler);
static void notifyShutdown();
static void deleteClient(int i);
static void alarm_action();
static void sighup_action();
static void sigusr1_action();
static void notifyHour();

/* Globals Variables */
static volatile bool killServer = false;
static volatile int numClients = 0;

//Colas de mensajes
static int msg_cs; 
static int msg_sc;

static volatile int segundos=-1;

static const char * fileConf =".server.conf"; // File where the freq of alarm is stored
static const char * fileWitness =".server.lock"; // File where the PID of the server process will be save.

static const char * connectionAccepted =":)";
static const char * connectionRefused =":(";

/* Dos tipos de mensajes, los que van precididos por el codigo de chCode son el de la hora actual */
/* Los que van precedidos por el codigo de dCode son la respuesta a la solicitud de tieme de los clientes */
static const int chCode = -1; // current hour Code
static const int dCode = -2; //date Code
static const int closeCode = -3; //to notify the shutdown of the server
static const int newClientCode = -4;
static const int closeClientCode = -5;
static const int timeRequestCode = -6;


static pid_t clientsPid[MAX_CLIENTS];


/****************Handlers*****************/
static void alarm_action(){
	
 	syslog(LOG_INFO,"Performing alarm action");
	notifyHour();	
}

static void sigterm_action(){
 	syslog(LOG_INFO,"Performing sigterm action");

	killServer = true;
}


static void sighup_action(){
  	  
	if(configure_alarm()!=0)
		syslog(LOG_ERR,"Error in alarm config, handler");
	
}

static void sigusr1_action(){

	syslog(LOG_INFO,"Signal SIGUSR1 recived,sending current hour to all clients");
	notifyHour();
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

static void check_queue(){
	int pos;
    msg_t msg;
    

	while(!killServer){

		errno = 0;

		if ( msgrcv(msg_cs,&msg,LONG,0,0) != -1 ){
			
			if( msg.op == newClientCode ){

				if( numClients < MAX_CLIENTS ){
					pos = searchFirstEmptyPosition();
					clientsPid[pos] = msg.tipo;
					numClients++;
					
					strncpy(msg.dato.s,connectionAccepted,MAX_MSG_STRING);
					
					if( msgsnd(msg_sc,&msg,LONG,0) == -1 )
						syslog(LOG_ERR,"Failed sending accepted conection: %d", errno);
					else
						syslog(LOG_INFO,"New client added, pidOfClient: %i ,numClients: %i",clientsPid[pos],numClients);

				}else{

					strncpy(msg.dato.s,connectionRefused,MAX_STRING);

					if( msgsnd(msg_sc,&msg,LONG,0) == -1 )
						syslog(LOG_ERR,"Failed refused accepted conection: %d", errno);

					syslog(LOG_INFO,"Server is full numClients: %i",numClients);
			
				}

			}
			else if( msg.op == timeRequestCode ){
				
				//Relleno el mensaje
				msg.op = dCode;
				msg.dato.t = time(NULL);

				if( msgsnd(msg_sc,&msg,LONG,0) == -1 )
					syslog(LOG_ERR,"Failed sending times answer,pid of Cliente: %li,  %d", msg.tipo,errno);

			}
			else if (msg.op == closeClientCode){
				deleteClient(msg.tipo);
				syslog(LOG_INFO,"Removed conection with client: %li, numClients: %i", msg.tipo,numClients);				

			}

		}
		else if(errno != EINTR){
			syslog(LOG_ERR,"Failed reciving msg: %d closing...", errno);
			killServer = true;
		}
		

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
		found = ( clientsPid[i] == -1 );
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
		clientsPid[i] = -1;
}

static void deleteClient(int pid){
	int i = 0;
	bool found = false;

	while(i < MAX_CLIENTS && !found)
		found = (clientsPid[i++] == pid);

	clientsPid[--i] = -1;
	numClients--;

}

static void notifyShutdown(){
    msg_t msg;

    msg.tipo = getpid();
    msg.op = closeCode;

    syslog(LOG_INFO,"Notifying close of server");
    
	for (int i = 0; i < MAX_CLIENTS; ++i){
		
		if (clientsPid[i] != -1){

			msg.tipo = clientsPid[i];

			if( msgsnd(msg_sc,&msg,LONG,0) == -1 )
				syslog(LOG_ERR,"Failed sending closingCode,pid of Cliente: %li,  %d", msg.tipo,errno);
		}
	}

}


static void notifyHour(){
	time_t t;
	struct tm *ct;
	msg_t msgQueue;

	for (int i = 0; i < MAX_CLIENTS; ++i){
	
		if(clientsPid[i] != -1){
			t = time(NULL);
			ct = localtime((const time_t *)&t);
			
			msgQueue.op = chCode;
			msgQueue.dato.currentHour[0] = ct->tm_hour; 
			msgQueue.dato.currentHour[1] = ct->tm_min; 
			msgQueue.dato.currentHour[2] = ct->tm_sec;
			msgQueue.tipo = clientsPid[i];

			if( msgsnd(msg_sc,&msgQueue,LONG,0) == -1 )
				syslog(LOG_ERR,"Failed sending times answer,pid of Cliente: %li,  %d", msgQueue.tipo,errno);
		}
	}
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

static int removeFile(){
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
	sigdelset(&grupo, SIGTERM);
	sigdelset(&grupo, SIGUSR1);
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
	
    key_t llave;

    // CREAR las 2 colas de mensajes    
    llave  = ftok(KEYFILE,TOK_SC);
    msg_sc = msgget(llave,IPC_CREAT | 0600); 
    
    llave  = ftok(KEYFILE,TOK_CS);
    msg_cs = msgget(llave,IPC_CREAT | 0600);

	if(setupDaemon() == -1){
		syslog(LOG_ERR,"Setup failed");
		exit(-1);
	}

	iniListofClients();

	if( create_witness()!=0 ){
		syslog(LOG_ERR,"Error creating the witness file");
		exit(-1);
	}

	/*SIGTERM*/  	  
	ini_handler(SIGTERM, sigterm_action);

 	/*Señal SIGHUP*/
  	ini_handler(SIGHUP, sighup_action);
  	 
	/*Señal SIGUSR1*/
  	ini_handler(SIGUSR1, sigusr1_action);

	/*Señal SIGALRM*/
	ini_handler(SIGALRM, alarm_action);
	if(configure_alarm()!=0){
		syslog(LOG_ERR,"Error in alarm config");
		exit(-1);
	}

	syslog(LOG_INFO,"Setup finished, running daemon...");
    syslog(LOG_INFO,"msg_cs: %i\n",msg_cs);
    syslog(LOG_INFO,"msg_sc: %i\n",msg_sc);

	check_queue();

	notifyShutdown();
	removeFile();

 	//Eliminar colas
	if(msgctl(msg_sc, IPC_RMID, NULL) == -1){
		perror("Fails removing sc queue");
	}

    if(msgctl(msg_cs, IPC_RMID, NULL) == -1){
    	perror("fails removing cs queue");
    }	

    
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