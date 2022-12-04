#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <math.h>
#define MSG_LEN 120 /*ossia 120 meno size di variabile tipo long*/

typedef struct {
    int quantita;
    int dimensione;
    int tempo_scadenza;
} smerce;

typedef struct {
    pid_t pid;
    double x;
    double y;
    smerce* carico;
    int carico_tot;
} snave;

typedef struct {
    pid_t pid;
    double x;
    double y;
} sporto;

typedef struct {
    long rtype;
    int idporto;            /*REQUEST DATA*/
    int idmerce;
    int qmerce;
}argomento_coda;

typedef struct{
	long mtype;            
	argomento_coda mtext;    
}msg;




#define TEST_ERROR    if (errno) {fprintf(stderr,           \
                      "%s:%d: PID=%5d: Error %d (%s)\n", \
                      __FILE__,         \
                      __LINE__,         \
                      getpid(),         \
                      errno,            \
                      strerror(errno));}

void sem_accesso(int semid, int num_risorsa);

void sem_uscita(int semid, int num_risorsa);

void stampa_merci(smerce* temp_merci);

int msg_invio(int id, argomento_coda r);

int msg_lettura(int id, argomento_coda* r);

int msg_error();

void msg_print_stats(int fd, int q_id);