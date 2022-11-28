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
#include <math.h>
#include <sys/sem.h>

/* LA SEGUENTE MACRO E' STATA PRESA DA test-pipe-round.c */
#define TEST_ERROR    if (errno) {fprintf(stderr,           \
                      "%s:%d: PID=%5d: Error %d (%s)\n", \
                      __FILE__,         \
                      __LINE__,         \
                      getpid(),         \
                      errno,            \
                      strerror(errno));}

typedef struct {
    pid_t pid;
    double x;
    double y;
} sinfo;

int capacita, velocita;
double ord,asc;
double xdest,ydest;
int id;
int sem_id;
sinfo* shmnavi, *shmporti;


void sem_accesso(int semid,int num_risorsa);

void sem_uscita(int semid,int num_risorsa);

void navigazione(double x, double y);

/*HANDLER PER GESTIRE IL SEGNALE DI TERMINAZIONE DEL PADRE*/
void handle_signal(int signum){
    printf("\033[0;31m");
    printf("uccisa nave[%d]\n", getpid());
    printf("\033[0m");

    exit(EXIT_SUCCESS);
}

int main(int argc, char** argv){
    /*DICHIARAZIONE DELLE VARIABILI*/
    struct sigaction sa;
    int temp;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT,&sa,NULL);
    srand(time(NULL));
    capacita = atoi(argv[4]);
    velocita = atoi(argv[5]);
    shmporti = shmat(atoi(argv[2]), NULL, 0);
    shmnavi = shmat(atoi(argv[3]), NULL, 0);
    sem_id = atoi(argv[1]);
    id = atoi(argv[6]);
    TEST_ERROR;

    sem_accesso(sem_id,1);/*sem[0]=>shmporti, sem[1]=>shmnavi*/
    asc = shmnavi[id].x;
    ord = shmnavi[id].y;
    sem_uscita(sem_id,1);

    if(id==0){/*prova di navigazione verso porto[1]*/
        printf("\n\nnave[%d]:(%.2f,%.2f)\n\n", id, asc, ord);
        sem_accesso(sem_id,0);
        temp = rand()%5;
        xdest = shmporti[temp].x;
        ydest = shmporti[temp].y;
        sem_uscita(sem_id,0);
        printf("navigazione verso porto[%d]\n", temp);
        navigazione(xdest,ydest);
        printf("\n\nnave[%d]:(%.2f,%.2f)\n\n", id, asc, ord);
        
    }
    /*ENTRA IN UN CICLO INFINITO PER ATTENDERE LA TERMINAZIONE DEL PADRE.
    VA POI MODIFICATO PER ESEGUIRE LE OPERAZIONI NECESSARIE.*/
    for(;;){}
    exit(0);
}

/*LA SEGUENTE FUNZIONE SERVE PER FAR SPOSTARE LA NAVE IN UN ALTRO PORTO.
    È STATA FATTA IN MODO CHE, DOPO UNA SOLA NANOSLEEP, LA NAVE ARRIVI A DESTINAZIONE.
    BISOGNA POI AGGIUNGERE L'IDENTIFICATORE DI NAVE IN PORTO\NAVE IN MARE*/
void navigazione(double x, double y){
    double dist;
    double tempo;
    struct timespec my_time;
    dist = sqrt(pow((y-ord),2)+pow((x-asc),2));
    tempo = dist/velocita;
    my_time.tv_sec = (int)tempo;
    my_time.tv_nsec = (tempo-(int)tempo) * 10000;
    nanosleep(&my_time, NULL);
    ord = y;
    asc = x;
    sem_accesso(sem_id,1);
    shmnavi[id].x = ord;
    shmnavi[id].y = asc;
    sem_uscita(sem_id,1);
}

void sem_accesso(int semid, int num_risorsa){
    struct sembuf my_op;
    /*printf("\nil processo:%d tenta l'accesso al semaforo:%d\n",getpid(),semid);*/
    my_op.sem_num = num_risorsa;
    my_op.sem_flg = 0;
    my_op.sem_op = -1;
    semop(semid,&my_op,1);
    /*printf("\nil processo:%d ha avuto accesso al semaforo:%d\n",getpid(),semid);*/
    TEST_ERROR;
}

void sem_uscita(int semid, int num_risorsa){
    struct sembuf my_op;
    my_op.sem_num = num_risorsa;
    my_op.sem_flg = 0;
    my_op.sem_op = 1;
    semop(semid,&my_op,1);
    /*printf("\nil processo:%d è uscito dal semaforo:%d\n",getpid(),semid);*/
    TEST_ERROR;
}