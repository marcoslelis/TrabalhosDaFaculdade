/*
 * Includes Necessarios 
 */

#include <pthread.h> /* para poder manipular threads */
#include <stdio.h> /* para printf() */
#include <stdlib.h>
#include <unistd.h>
#include <sys/sem.h>            /* for semget(), semop(), semctl() */
#include <sys/time.h>   
#include <stdlib.h> // incluiu stdlib
#include <errno.h>              /* errno and error codes */
#include <sys/time.h>           /* for gettimeofday() */
#include <stdio.h>              /* for printf() */
#include <unistd.h>             /* for fork() */
#include <sys/types.h>          /* for wait() */
#include <sys/wait.h>           /* for wait() */
#include <signal.h>             /* for kill(), sigsuspend(), others */
#include <sys/ipc.h>            /* for all IPC function calls */
#include <sys/shm.h>            /* for shmget(), shmat(), shmctl() */
#include <sys/sem.h>            /* for semget(), semop(), semctl() */
#include <string.h>
#include <wait.h> /* for wait() */
#include <sys/msg.h> /* for msgget(), msgctl() */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define CHAVEFILA 123
#define MEMFILHO1 1
#define MEMFILHO2 2
#define MEMFILHO3 3
#define SEMFILHO1 11
#define SEMFILHO2 22
#define SEMFILHO3 33

pthread_t threadsID[3];



typedef struct {
int pid;
int chaveMem;
int chaveSem;
} conteudo;

typedef struct {
long mtype;
char mtext[sizeof(conteudo)]; 
} msgbuf_t;



struct sembuf g_sem_op1[1];
struct sembuf g_sem_op2[1];



void *thProcedimento(void *arg)
{
	int n = *((int*) arg);
	n = n+1;
	printf("sou a thread %i\n",n);
	fflush(stdout);
	int queue_id;
	int *memoriaEnd;
	int memoriaID;
	int semaforoID;
	


	g_sem_op1[0].sem_num   =  0;
	g_sem_op1[0].sem_op    = -1; // trancar
	g_sem_op1[0].sem_flg   =  0;

	g_sem_op2[0].sem_num =  0;
	g_sem_op2[0].sem_op  =  1; // destrancar
	g_sem_op2[0].sem_flg =  0;

	// obtendo o id da fila
	 if( (queue_id = msgget(CHAVEFILA, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}


	// declaraões para receber mensagem
	msgbuf_t mensagem;
	conteudo *conteudoMensagem;
	
	if( msgrcv(queue_id,(struct msgbuf *)&mensagem,sizeof(conteudo),0,0) == -1 ) {
	fprintf(stderr, "Impossivel receber mensagem!\n");
	exit(1);
	}
	conteudoMensagem = (conteudo*) mensagem.mtext;
	printf("Thread: %i - PID: %i - Memoria Compartilhada: %i - Semaforo: %i\n", n,conteudoMensagem->pid, 
	conteudoMensagem->chaveMem, conteudoMensagem->chaveSem);
	fflush(stdout);

///////////*****
		// criando segmento de memoria
			if( (memoriaID = shmget( conteudoMensagem->chaveMem, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
			fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
			exit(1);
			}
			if( (memoriaEnd = (int *)shmat(memoriaID, NULL, 0)) == (int *)-1 ) {
			fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
			exit(1);
			}

			// criando semaforo
			if( ( semaforoID = semget( conteudoMensagem->chaveSem, 1, IPC_CREAT | 0666 ) ) == -1 ) {
			fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
			exit(1);
			}

	// atualizando memoria compartilhada
	*memoriaEnd = n + conteudoMensagem->pid;
	
	// acordando processo
	if( semop( semaforoID, g_sem_op2, 1 ) == -1 ) {
	fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
	exit(1);
	}
	

	pthread_exit(NULL);
}


int main()
{
	int idT[3];
	int i, tc;
	
	for (i=0;i<3;i++)
	{
		idT[i] = i;
		tc = pthread_create(&threadsID[i], NULL, thProcedimento, (void *)&idT[i]);
		if (tc) {
		printf("ERRO: impossivel criar um thread consumidor\n");
		exit(-1);

		}
	}
 	pthread_join(threadsID[0], NULL);
	pthread_join(threadsID[1], NULL);
	pthread_join(threadsID[2], NULL);

	printf("Terminando a thread main()\n");
	fflush(stdout);
	exit(0);



}
