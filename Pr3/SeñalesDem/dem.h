
/* Authors */
/* Fernando Candelario Herrero */
/* Cristina Manso de la Viuda  */

#ifndef _DEM_H
#define _DEM_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

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

int start_dem(); //Start deamon 

#endif