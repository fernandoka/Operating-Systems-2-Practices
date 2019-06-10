#ifndef _CLIENTE_H
#define _CLIENTE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h> 

#include <stdbool.h>

//Biblioteca para ficheros (open)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Para manejo de cadenas
#include <string.h>

//Para syslog()
#include <syslog.h>

//Para mensajes IPC
#include "mensaje.h"

#define MAX_STRING 50

#endif