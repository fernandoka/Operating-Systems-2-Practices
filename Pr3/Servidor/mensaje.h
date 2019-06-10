#ifndef _MENSAJE_H
#define _MENSAJE_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>  

#define MAX_MSG_STRING 10

struct _dato_t
{
	char s[MAX_MSG_STRING]; //Para enviar la cadena de confirmacion de conexion.
	time_t t; // Respuesta de times
	int currentHour[3]; //Respuesta de alarm.
};

typedef struct _dato_t dato_t;    

/* Estructura de datos que constituye el mensaje */
struct _msg_t
{
	long tipo; //Pid del cliente
	int op;
    dato_t dato;
};

typedef struct _msg_t msg_t;

#define LONG (sizeof(msg_t) - sizeof(long))

/* Defines para la creacion de llaves */
#define TOK_CS	'a'
#define TOK_SC	'b'
#define KEYFILE "./mensaje.h"

#endif