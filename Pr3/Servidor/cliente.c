#include "cliente.h"

/*Prototipes*/
static int start_cl();
static int runClient(int pid);
static int confirmConnection(int pid);
static int connecToServer(int pid);
static void leer(int pid);
static void solicitar(int pid);
static void notifyClose(int pid);

static volatile bool killClient = false; // Si el servidor decide apagarse el cliente tambien lo hace.

//Colas de mensajes
static int msg_cs; 
static int msg_sc;

static const char * connectionAccepted = ":)";

/* Dos tipos de mensajes, los que van precididos por el codigo de chCode son el de la hora actual */
/* Los que van precedidos por el codigo de dCode son la respuesta a la solicitud de tieme de los clientes */
static const int chCode = -1; // current hour Code
static const int dCode = -2; //date Code
static const int closeCode = -3; //to notify the shutdown of the server
static const int newClientCode = -4; 
static const int closeClientCode = -5;
static const int timeRequestCode = -6;

/* AUX FUNCTIONS */

static void solicitar(int pid)
{
	msg_t msg;

	msg.tipo = pid;
	msg.op = timeRequestCode;
	
	printf("Sending timeRequestCode\n");
	if( msgsnd(msg_cs,&msg,LONG,0) == -1 )
		perror("Fails sending times");

}

static void leer(int pid)
{
	msg_t msg;

	if ( msgrcv(msg_sc,&msg,LONG,pid,0) == -1 ){
		perror("Rcv fails");
	}

	if (msg.op == dCode) {
		printf("Secs since 1/1/1970: %lis \n\n", msg.dato.t);
	}

	else if (msg.op == chCode) {
		printf("Recived time(hh:mm:ss)-> %i:%i:%i\n\n", msg.dato.currentHour[0], msg.dato.currentHour[1], msg.dato.currentHour[2]);
	}

	else if (msg.op == closeCode) {
		printf("Server is closing, finishing client process. \n\n");
		killClient = true;
	}

	else
		printf("Not recognized message\n");

}


static void notifyClose(int pid){
	msg_t msg;

	msg.tipo = pid;
	msg.op = closeClientCode;
	
	printf("Sending closeClientCode\n");
	if( msgsnd(msg_cs,&msg,LONG,0) == -1 )
		perror("Fails sending closeClientCode");

}


/* END AUX FUNCTIONS  */

/* SETUP */


static int connecToServer(int pid){
    msg_t msg;
	
    msg.op = newClientCode;
    msg.tipo = pid;
    
    if( msgsnd(msg_cs,&msg,LONG,0) == -1 ){
   		perror("Failed sending connection resquest");
   		return -1; 
    }

	return 0;
}


static int confirmConnection(int pid){
    msg_t msg;
    int res = -1;
  
	if ( msgrcv(msg_sc,&msg,LONG,pid,0) == -1 ){
		perror("Rcv fails");
		return res;
	}

	if(msg.op == newClientCode){
		
		if(strncmp(msg.dato.s,connectionAccepted,MAX_MSG_STRING) == 0){
			res = 0;
			printf("Connection accepted: %s\n",msg.dato.s);
		}
		else
			printf("Connection refused: %s\n",msg.dato.s);
	}
	else{
		printf("Unexepected code, code recived: %i \n", msg.op);
		perror("Errno");
	}

	return res;
}

/* END SETUP SECTION */

static int runClient(int pid){

	int n;

	srand(time(NULL));

	n = rand() % 11; 
	printf("Iteraciones a realizar: %i\n\n", n);

	while(!killClient && n > 0){

		if( n%2 == 0 ){
			printf("Vuelta par con n = %i\n", n);
			solicitar(pid);
			leer(pid);
		}
		else{
			printf("Vuelta impar con n = %i\n", n);
			leer(pid);
		}

   		n--;
	}//While

	return killClient ? 0 : 1;
}


static int start_cl(){
		
		pid_t pid = getpid(); //Obtenemos el pid del proceso.
	

		printf("Bienvenido cliente nuevo, pid: %i\n",pid);
		
		/* SETUP */
		if ( connecToServer(pid) == -1 || confirmConnection(pid) == -1 ){
			perror("Setup fails");
			return -1;
		}

		if( runClient(pid) == 1 )
			notifyClose(pid);
		

		return 0;
}


int main(){

	key_t llave;

    // CREAR las 2 colas de mensajes 
    llave  = ftok(KEYFILE,TOK_SC);
    msg_sc = msgget(llave,IPC_CREAT | 0600); 
    
    llave  = ftok(KEYFILE,TOK_CS);
    msg_cs = msgget(llave,IPC_CREAT | 0600);

    printf("msg_cs: %i\n",msg_cs);
    printf("msg_sc: %i\n",msg_sc);	

	if(start_cl() == -1){
		perror("Client fails");
		exit(-1);
	}

	exit(0);
}
