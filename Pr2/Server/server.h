/* Authors */
/* Fernando Candelario Herrero */
/* Cristina Manso de la Viuda  */

#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

#include <stdbool.h>

//Biblioteca para ficheros (open)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Para manejo de cadenas
#include <string.h>

//Para syslog()
#include <syslog.h>

#define MAX_STRING 50
#define MAX_CLIENTS 2
#define SELECT_TIMEOUT_SEC 10
#define SELECT_TIMEOUT_USEC 0

void start_server(); //Start server deamon 

#endif