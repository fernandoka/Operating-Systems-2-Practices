#include "cliente.h"

/* Authors */
/* Fernando Candelario Herrero */
/* Cristina Manso de la Viuda  */

/*Prototipes*/
static int start_cl();
static int runClient(int toClientfd,char * auxString2);
static int confirmConnection(int * toClientfd, char * auxString2);
static int connecToServer(int pid);
static int crearTuberias(int pid, char *auxString, char *auxString2);
static void leer(int toClientfd);
static void solicitar(int toServerfd);
static int removeFiles();
static char * iToa(int n, int *lenght);
static int countDigits(int n);

static volatile bool killClient = false; // Si el servidor decide apagarse el cliente tambien lo hace.

static const char * newClientName = "/tmp/newclient";
static const char * toClientName = "/tmp/toClient";
static const char * toServerName = "/tmp/toServer";
static const char * connectionAccepted = ":)";
static const char * connectionRefused = ":(";
static const char * sentinel = "times";

/* Dos tipos de mensajes, los que van precididos por el codigo de chCode son el de la hora actual */
/* Los que van precedidos por el codigo de dCode son la respuesta a la solicitud de tieme de los clientes */
static const int chCode = -1; // current hour Code
static const int dCode = -2; //date Code
static const int closeCode = -3; //to notify the shutdown of the server


#define MAX_STRING 50

/* AUX FUNCTIONS */
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


static int removeFiles(){
	pid_t pid = getpid();
	char * asciiPid;
	char auxString[MAX_STRING];
	int numDigits;


	asciiPid = iToa(pid,&numDigits);				


	strncpy(auxString,toServerName,MAX_STRING);
	strncat(auxString, asciiPid, (size_t)MAX_STRING);

	if( (unlink(auxString)) == -1){
		syslog(LOG_ERR,"Failed removing  toServerName  %s",strerror(errno));
		return -1;
	}

	strncpy(auxString,toClientName,MAX_STRING);
	strncat(auxString, asciiPid, (size_t)MAX_STRING);
	
	if( (unlink(auxString)) == -1){
		syslog(LOG_ERR,"Failed removing toClientName %s",strerror(errno));
		return -1;
	}

	free(asciiPid);
	return 0;
}

static void solicitar(int toServerfd)
{
	printf("Sending times\n");
	if (write(toServerfd, sentinel, sizeof(time_t)) == -1) {
		perror("Fail sending string times");
	}
}

static void leer(int toClientfd)
{
	int codeRecived;
	int msg[3];
	time_t cDate;

	read(toClientfd, &codeRecived, sizeof(int));

	if (codeRecived == dCode) {
		read(toClientfd, &cDate, sizeof(time_t));
		printf("Secs since 1/1/1970: %lis \n\n", cDate);
	}

	else if (codeRecived == chCode) {
		read(toClientfd, &msg, sizeof(int) * 3);
		printf("Recived time(hh:mm:ss)-> %i:%i:%i\n\n", msg[0], msg[1], msg[2]);
	}

	else if (codeRecived == closeCode) {
		printf("Server is closing, finishing client process. \n\n");
		killClient = true;
	}

	else
		printf("Not recognized message\n");

}

/* END AUX FUNCTIONS  */

/* SETUP */

static int crearTuberias(int pid, char *auxString, char *auxString2){

	char * asciiPid;
	int numDigits;

	asciiPid = iToa(pid,&numDigits);	
	
	/* Creo latuberia con nombre */
	strncpy(auxString,toServerName,MAX_STRING);
	strncat(auxString, asciiPid, (size_t)MAX_STRING);


	if( mkfifo(auxString,0666) < 0 ){
		syslog(LOG_ERR,"Error creating the fifo file toServer, %s",strerror(errno));
		free(asciiPid);
		return -1;
	}

	/* Creo latuberia con nombre */
	strncpy(auxString2,toClientName,MAX_STRING);
	strncat(auxString2, asciiPid, (size_t)MAX_STRING);


	if( mkfifo(auxString2,0666) < 0 ){
		syslog(LOG_ERR,"Error creating the fifo file toClient, %s",strerror(errno));
		free(asciiPid);
		return -1;
	}

	free(asciiPid);

	return 0;

}

static int connecToServer(int pid){
	
	int newClientPipe;
	//Descriptor de la tuberia para conectarse, mandamos el mensaje y luego close
	if ( (newClientPipe = open(newClientName,O_WRONLY )) == -1 ){
		syslog(LOG_ERR,"Failed during open fifo  file, %s",strerror(errno));
		return -1;
	}


	//Sending PID
	printf("Pid Sent!!\n");
	if( write(newClientPipe,&pid,sizeof(pid_t)) == -1 ){
		perror("Fail connect to server");
		return -1;
	}

	close(newClientPipe);

	return 0;
}


static int confirmConnection(int * toClientfd, char * auxString2){

		fd_set conjunto;
	    struct timeval timeout;
	    char confirmMsg[3];
	    int res = -1; //Fail by default
		int max, cambios;	    

		if ( (*toClientfd = open(auxString2,O_RDONLY )) == -1 ){
			syslog(LOG_ERR,"Failed during open fifo  file read, %s",strerror(errno));
			perror("Pete en toClientfd \n");
			return -1;
		}


		/* Compruebo si el servidor acepta mi conexion*/
		FD_ZERO(&conjunto);
		FD_SET(*toClientfd, &conjunto);
		max = *toClientfd;

		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		cambios = select(max+1, &conjunto, NULL, NULL, &timeout);

		if(cambios == 0) 
			printf("Timeout, no answer from server\n");
		else if(cambios == -1)
			perror("Select fails, closing...\n");
		else {
		
			if( FD_ISSET (*toClientfd, &conjunto) ){
				read(*toClientfd,confirmMsg,sizeof(char)*3);
				
				if( strncmp(confirmMsg,connectionAccepted,MAX_STRING) == 0 ){
					printf("Connection Accepted reciving data: %s \n",confirmMsg);
					res = 0;
				}
				else{
					printf("Connection refused \n");
					//Para comporbar si el servidor a contestado que no, o el mensaje de vuelta
					//se ha corrompido
					if(strncmp(confirmMsg,connectionRefused,MAX_STRING) == 0)
						printf("Server is full: %s\n",confirmMsg);

					close(*toClientfd);
				}

			}

		}

	return res;
}

/* END SETUP SECTION */

static int runClient(int toClientfd, char * auxString){

	int n;
	fd_set conjunto;
	struct timeval timeout;
	int toServerfd,max,cambios;
	int res = 0; // by default no error

	if ( (toServerfd = open(auxString,O_WRONLY )) == -1 ){
		syslog(LOG_ERR,"Failed during open fifo file write, %s",strerror(errno));
		perror("Pete en serverFD\n");
		return -1;
	}	

	srand(time(NULL));

	n = rand() % 11; 
	printf("Iteraciones a realizar: %i\n\n", n);

	while(!killClient && n > 0){

		FD_ZERO(&conjunto);
		FD_SET(toClientfd, &conjunto);
		max = toClientfd;

		timeout.tv_sec = 10;
		timeout.tv_usec = 0;

		cambios = select(max+1, &conjunto, NULL, NULL, &timeout);

		if (cambios == 0) 
			printf(" timeout of select expired... \n");
		else if(cambios == -1){
			perror("Failed during select, closing...");
			killClient = true;
			res = -1;
		}
		else{
			 
			if( n%2 == 0 ){
				printf("Vuelta par con n = %i\n", n);
				solicitar(toServerfd);
				if( FD_ISSET (toClientfd, &conjunto) ){
					leer(toClientfd);
				}
			}
			else{
				printf("Vuelta impar con n = %i\n", n);
				if( FD_ISSET (toClientfd, &conjunto) ){
					leer(toClientfd);
				}
			}
			n--;

		}

	}//While

	close(toServerfd);
	close(toClientfd);

	return res;
}


static int start_cl(){
		
		pid_t pid = getpid(); //Obtenemos el pid del proceso para generar las tuberias.
		char auxString[MAX_STRING];
		char auxString2[MAX_STRING];
		int toClientfd;

		printf("Bienvenido cliente nuevo, pid: %i\n",pid);
		
		/* SETUP */
		if ( crearTuberias(pid,auxString, auxString2) == -1 || 
			connecToServer(pid) == -1 || confirmConnection(&toClientfd,auxString2) == -1 )
		{
			removeFiles();
			perror("Setup fails");
			return -1;
		}

		if( runClient(toClientfd,auxString) == -1 ){
			perror("Fails while running...");
			return -1;
		}
	
		removeFiles();

		return 0;
}


int main(){

	if(access(newClientName,F_OK) != 0){
		printf("Fails to connect with server, dosen´t exist the file %s\n",toClientName);
		exit(0);
	}

	if(start_cl() == -1){
		perror("Client fails");
		exit(-1);
	}

	exit(0);
}
