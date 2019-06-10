
/* Authors */
/* Fernando Candelario Herrero */
/* Cristina Manso de la Viuda  */

#ifndef _MITIEMPO_H
#define _MITIEMPO_H

#include<math.h>
#include <stdio.h>
// Biblioteca para utilizar la variable bool
#include <stdbool.h>
// Bibliotecas de Temporizacion
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
// Bibliotecas de stat
#include <sys/stat.h>
//Biblioteca para ficheros (open)
#include <fcntl.h>

#define MAX_OPTIONS 6

//El tiempo esta medido en us.

// Funciones
int init(); //Lee del archivo de texto para configurar el reloj que lee.
int start(); //Toma el instante actual como referencia para el contador
int pause(); //Parará la cuentaasociada al contador. Si ya esta pausada, la llamada se ignora.
int resume(); //Cuntinua la cuenta tras pause(). Si no esta pausada, la llamada se ignora.
int stop(); //Para definitivamente el contador. Devuelve el tiempo transcurrido.


#endif