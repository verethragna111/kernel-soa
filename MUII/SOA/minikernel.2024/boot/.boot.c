/*
 *  boot/boot.c
 *
 *  Minikernel. Versi�n 2.0
 *
 *  Fernando P�rez Costoya
 *
 */

/* C�digo de uso interno */

#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

static char ejec_SO[128];

/* Se asegura de que en el nombre del ejecutable SO haya una /
   para evitar que lo busque en los PATH de bibliotecas */

static void procesar_nombre(char *ent, char *sal){
	if ((ent) && (ent[0]!='/'))
		sprintf(sal, "./%s", ent);
	else
		strcpy(sal, ent);
}

/* Equivalente a "dirname" */

static void *nombre_dir(char *ent){
	char *paux;
	paux=&(ent[strlen(ent)-1]);
	for ( ; *paux!='/'; paux--);
	*(paux+1)='\0';
	return ent;
}

int main(int argc, char **argv){
	void *desc;
	void (*fmain)();
	char **dir_base;
	sigset_t mascara;


	if (argc!=2){
		fprintf(stderr, "Uso: %s \"path del fichero del S.O.\"\n",
				argv[0]);
		exit(1);
	}
	/* Elimina buffering */
	setbuf(stdout, NULL);

	/* Proh�be "interrupciones de reloj */
	sigemptyset(&mascara);
	sigaddset(&mascara, SIGALRM);
	sigprocmask(SIG_BLOCK, &mascara, NULL);

	/* peque�o "retoque" en el nombre del ejecutable del SO */
	procesar_nombre(argv[1], ejec_SO);

	/* Carga del ejecutable del SO */
	if ((desc=dlopen(ejec_SO, RTLD_LAZY))==NULL){
		fprintf(stderr, "Error Cargando el ejecutable del S.O.: %s\n",
			dlerror());
		return(1);
	}

	/* Obtiene la direcci�n de su punto de arranque */
	if ((fmain=dlsym(desc, "main"))==NULL){
		fprintf(stderr,"Formato incorrecto del ejecutable del S.O.: %s\n",
			dlerror());
		return(1);
        }

	/* Busca la direcci�n de una variable del SO que se usar�
           para informarle de cu�l ser� el directorio base de la ejecuci�n */
	if ((dir_base=dlsym(desc, "dir_base"))==NULL){
		fprintf(stderr,"Formato incorrecto del ejecutable del S.O.: %s\n",
			dlerror());
		return(1);
        }

	/* La variable del SO queda apuntando al directorio base */
	*dir_base=nombre_dir(ejec_SO);

	/* Se invoca al SO */
	(*fmain)();

	/* No deber�a volver aqu� */
	dlclose(desc);
	fprintf(stderr,"El S.O. ha terminado inesperadamente\n");
	return(1);
}
