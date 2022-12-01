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
#include "utilities.h"

/*MACRO PER NON METTERE INPUT*/
#define NO_INPUT
/*MACRO PER LA VELOCITA DELLE NAVI E LA CAPACITA*/



/*DICHIARAZIONE DEGLI ARRAY DEI PID DEI PORTI E DELLE NAVI*/
pid_t* na;
pid_t* po;
int SO_NAVI, SO_PORTI, q_id, SO_BANCHINE, SO_SIZE;
int idshmnavi, idshmporti;
int sem_id; /*id del semaforo che permette l'accesso alla shm*/
int sem_porto;/*semaforo per far approdare le navi al porto*/
/*STRUCT PER DEFINIRE LE COORDINATE DEI PORTI E DELLE NAVI E I RELATIVI PID*/



void handle_alarm(int signal);
/*HANDLER PER IL SEGNALE MANUALE DI TERMINAZIONE*/
void close_all(int signum);



/*HANDLER PER IL SEGNALE DI FINE PROGRAMMA (ALARM)*/

int main() {
    /* DICHIARAZIONE DELLE VARIABILI */
    smerce* temp_merci;
    char stringsem_id[3 * sizeof(sem_id) + 1];
    char stringsem_porto[3 * sizeof(sem_porto) + 1];
    char stringporti[3 * sizeof(idshmporti) + 1];
    char stringnavi[3 * sizeof(idshmnavi) + 1];
    char stringid[13];
    char stringq[13];
    char stringvelocita[13], stringcapacity[13];
    char* nave[10] = {""};
    char* porto[10] = {""};
    snave* shmnavi; sporto* shmporti;
    short uguali;
    struct timespec now;
    sporto* arrayporti;
    snave* arraynavi;
    int i, j, c, banchine_effettive;
    double SO_LATO;
    int SO_MERCI;
    int SO_MIN_VITA, SO_MAX_VITA; 
    int SO_CAPACITY;
    int SO_VELOCITA;
    int status;
    struct sigaction ca;
    struct sigaction sa;
    bzero(&ca, sizeof(ca));
    ca.sa_handler = close_all;
    sigaction(SIGINT, &ca, NULL);
    TEST_ERROR;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = handle_alarm;
    sigaction(SIGALRM, &sa, NULL);
    TEST_ERROR;

    sem_id = semget(IPC_PRIVATE, 2, 0600); /*INIZIALIZZAZIONE DI 2 SEMAFORI,
                                            IL SEMAFORO 0 SI OCCUPA DELLA SHM DEL 
                                            PORTO ED IL SEMAFORO 1 DELLA SHM DELLE NAVI*/
    TEST_ERROR;
    /*CREO LA CODA DI MESSAGGI*/
    q_id = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0666);
    TEST_ERROR;
    printf("sem_id: %d\n\n",sem_id);
    srand(time(NULL));


    /*INIZIO INPUT*/
    printf("\033[033;34m");
    
#ifndef NO_INPUT/*TO DO: CONTROLLO CHE I PARAMETRI SIANO POSITIVI*/
    printf("inserisci la grandezza della mappa: ");
    scanf("%le", &SO_LATO);
    do {
        printf("inserisci il numero di navi: ");
        scanf("%d", &SO_NAVI);
    } while (SO_NAVI < 1);
    do {
        printf("inserisci il numero di porti: ");
        scanf("%d", &SO_PORTI);
    } while (SO_PORTI < 4);
    printf("inserisci il carico massimo trasportabile delle navi: ");
    scanf("%d", &SO_CAPACITY);
    printf("inserisci la velocita' delle navi: ");
    scanf("%d", &SO_VELOCITA);
    printf("inserisci la dimensione massima della merce: ");
    scanf("%d", &SO_SIZE);
    printf("inserisci il numero di merci utilzzabi: ");
    scanf("%d", &SO_MERCI);
    printf("inserisci il minimo di giorni di vita delle merci: ");
    scanf("%d", &SO_MIN_VITA);
    printf("inserisci il massimo di giorni di vita delle merci: ");
    scanf("%d", &SO_MAX_VITA);
    
#endif

#ifdef NO_INPUT
    SO_LATO = 10;   /*(n > 0) !di tipo double!*/
    SO_NAVI = 20;    /*(n >= 1)*/
    SO_PORTI = 5;   /*(n >= 4)*/
    SO_BANCHINE = 10;
    SO_MERCI = 3;
    SO_SIZE = 10;
    SO_CAPACITY = 100;
    SO_VELOCITA = 20;
    SO_MAX_VITA = 5;
    SO_MIN_VITA = 2;
#endif
    /*FINE INPUT*/

    /*INIZIO MENU*/
    printf("\nSO_LATO = %.2f", SO_LATO);
    printf("\nSO_NAVI = %d", SO_NAVI);
    printf("\nSO_PORTI = %d", SO_PORTI);
    
    sem_porto = semget(IPC_PRIVATE, SO_PORTI, 0600);
    idshmporti = shmget(IPC_PRIVATE, sizeof(arrayporti), 0600);
    idshmnavi = shmget(IPC_PRIVATE, sizeof(arraynavi), 0600);
    shmporti = shmat(idshmporti, NULL, 0);
    shmnavi = shmat(idshmnavi, NULL, 0);
    /*ALLOCAZIONE DELLA MEMORIA PER GLI ARRAY DEI PID DEI FIGLI*/
    na = calloc(SO_NAVI, sizeof(*na));
    po = calloc(SO_PORTI, sizeof(*po));
    printf("\nidshmporti: %d\n\n", idshmporti);
    printf("\033[0m");
    /*FINE MENU*/
    sprintf(stringsem_porto, "%d", sem_porto);
    sprintf(stringsem_id, "%d", sem_id);
    sprintf(stringporti, "%d", idshmporti);
    sprintf(stringq, "%d", q_id);
    sprintf(stringcapacity, "%d", SO_CAPACITY);
    sprintf(stringvelocita, "%d", SO_VELOCITA);
    TEST_ERROR;
    porto[1] = stringsem_id;
    porto[2] = stringporti;
    porto[3] = stringnavi;
    porto[5] = stringsem_porto;
    porto[6] = stringq;
    porto[7] = NULL;
    sprintf(stringnavi, "%d", idshmnavi);
    TEST_ERROR;
    nave[1] = stringsem_id;
    nave[2] = stringporti;
    nave[3] = stringnavi;
    nave[4] = stringcapacity;
    nave[5] = stringvelocita;
    nave[7] = stringsem_porto;
    nave[8] = stringq;
    nave[9] = NULL;

    /*DICHIARAZOINE SEMAFORO FIRST*/
    semctl(sem_id, 0 , SETVAL, 1);
    semctl(sem_id, 1 , SETVAL, 1);
    TEST_ERROR;


    TEST_ERROR;

    alarm(4);
    arrayporti = calloc(SO_PORTI, sizeof(*arrayporti));
    /*CREAZIONE DEI PORTI*/
    for (i = 0; i < SO_PORTI; i++) {
        po[i] = fork();
        if (po[i] == -1) {
            TEST_ERROR;
            exit(1);
        }
        if (po[i] == 0) {
            /* CHILD */
            arrayporti[i].pid = getpid();
            switch (i) {
            case 0:
                arrayporti[i].x = 0;
                arrayporti[i].y = 0;
                break;
            case 1:
                arrayporti[i].x = SO_LATO;
                arrayporti[i].y = 0;
                break;
            case 2:
                arrayporti[i].x = 0;
                arrayporti[i].y = SO_LATO;
                break;
            case 3:
                arrayporti[i].x = SO_LATO;
                arrayporti[i].y = SO_LATO;
                break;
            default:
                j = 0;
                do {
                    int RANDMAX = (int)SO_LATO;
                    clock_gettime(CLOCK_REALTIME , &now);
                    arrayporti[i].x = (double)(now.tv_nsec % (RANDMAX * 100)) / 100;
                    clock_gettime(CLOCK_REALTIME , &now);
                    arrayporti[i].y = (double)(now.tv_nsec % (RANDMAX * 100)) / 100;
                    uguali = 0;
                    for (j = 0; j < i; j++) {
                        if (arrayporti[i].x == arrayporti[j].x && arrayporti[i].y == arrayporti[j].y) {
                            uguali = 1;
                        }
                    }
                } while (uguali);
            }
            clock_gettime(CLOCK_REALTIME, &now);
            banchine_effettive = ((now.tv_nsec % SO_BANCHINE * 1000) / 1000) + 1;
            semctl(sem_porto, i , SETVAL, banchine_effettive);
            sem_accesso(sem_id,0);
            shmporti[i] = arrayporti[i];
            sem_uscita(sem_id,0);
            sprintf(stringid,"%d",i);
            porto[4] = stringid;
            printf("creazione porto[%d], di pid:%d con coordinate x=%.2f, y=%.2f, con %d banchine\n\n", i, arrayporti[i].pid, arrayporti[i].x, arrayporti[i].y, banchine_effettive);
            execvp("./porto", porto);
            TEST_ERROR;
            exit(EXIT_FAILURE);
        }
        else {

        }
    }
    arraynavi = calloc(SO_NAVI, sizeof(*arraynavi));
    temp_merci = calloc(SO_MERCI, sizeof(*temp_merci));
    for(i=0; i<SO_MERCI;i++){/*dichiarazione array merci*/
        temp_merci[i].quantita = 0;
        clock_gettime(CLOCK_REALTIME , &now);
        temp_merci[i].dimensione = now.tv_nsec % SO_SIZE + 1;
        clock_gettime(CLOCK_REALTIME , &now);
        temp_merci[i].tempo_scadenza = (now.tv_nsec % (SO_MAX_VITA-SO_MIN_VITA)) + SO_MIN_VITA +1;
    }
    stampa_merci(temp_merci);

    /* CREAZIONE DELLE NAVI */
    for (i = 0; i < SO_NAVI; i++) {
        na[i] = fork();
        if (na[i] == -1) {
            TEST_ERROR;
            exit(1);
        }
        if (na[i] == 0) {
            /* CHILD */
            arraynavi[i].pid = getpid();
            arraynavi[i].carico = temp_merci;
            do {
                int RANDMAX = (int)SO_LATO;
                clock_gettime(CLOCK_REALTIME , &now);
                arraynavi[i].x = (double)(now.tv_nsec % (RANDMAX * 100)) / 100;
                clock_gettime(CLOCK_REALTIME , &now);
                arraynavi[i].y = (double)(now.tv_nsec % (RANDMAX * 100)) / 100;
                uguali = 0;
                for (j = 0; j < i; j++) {
                    if (arraynavi[i].x == arraynavi[j].x && arraynavi[i].y == arraynavi[j].y) {
                        uguali = 1;
                    }
                }
            } while (uguali);
            sem_accesso(sem_id,1);
            shmnavi[i] = arraynavi[i];
            sem_uscita(sem_id,1);
            sprintf(stringid,"%d",i);
            nave[6] = stringid;
            printf("creazione nave[%d], di pid:%d con coordinate x=%.2f, y=%.2f\n\n",i, arraynavi[i].pid, arraynavi[i].x, arraynavi[i].y);
            execvp("./nave", nave);
            TEST_ERROR;
            exit(EXIT_FAILURE);
        }
        else {
            /* PARENT */
        }
    }
    free(temp_merci);
    /*IL PROCESSO PADRE RIMANE IN PAUSA FINO ALL'ARRIVO DI UN SEGNALE (ALARM)*/
    pause();
    msgctl(q_id,IPC_RMID,NULL);
    shmctl(idshmporti,IPC_RMID,NULL);
    shmctl(idshmnavi,IPC_RMID,NULL);
    semctl(sem_id,1,IPC_RMID);
    semctl(sem_porto,1,IPC_RMID);
    printf("\n\nFine del programma\n");

    exit(EXIT_SUCCESS);
}

void handle_alarm(int signum) {
    int i, status;
    printf("\n\n\n\n");
    for (i = 0; i < SO_NAVI; i++) {
        kill(na[i], SIGINT);
        TEST_ERROR;
        wait(&status);
        TEST_ERROR;
    }
    for (i = 0; i < SO_PORTI; i++) {
        kill(po[i], SIGINT);
        TEST_ERROR;
        wait(&status);
        TEST_ERROR;
    }
}
void close_all(int signum) {
    msgctl(q_id,IPC_RMID,NULL);
    shmctl(idshmporti, IPC_RMID, NULL);
    shmctl(idshmnavi,IPC_RMID,NULL);
    semctl(sem_id,1,IPC_RMID);
    semctl(sem_porto,1,IPC_RMID);

    printf("\n\nFine del programma\n");
    exit(0);
    
}

