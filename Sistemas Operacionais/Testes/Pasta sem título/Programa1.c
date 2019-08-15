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




int main()
{
	int queue_id, count;
	msgbuf_t mensagem;
	conteudo *conteudoMensagem;
	int *memoriaEnd;
	int memoriaID;
	int semaforoID;

	// criar fila de mensagem

	 if( (queue_id = msgget(CHAVEFILA, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}


	// estrutura do semaforo
	g_sem_op1[0].sem_num   =  0;
	g_sem_op1[0].sem_op    = -1; // trancar
	g_sem_op1[0].sem_flg   =  0;

	g_sem_op2[0].sem_num =  0;
	g_sem_op2[0].sem_op  =  1; // destrancar
	g_sem_op2[0].sem_flg =  0;






	// criar filhos

	 int rtn = 1;
       	for( count = 0; count < 3; count++ ) {
               if( rtn != 0 ) {
                      rtn = fork();
               } else {
                       break;  
               }
       	}

	if (rtn == 0)

	{	// filho 1
		if(count == 1)
		{
			printf("SOu o filho 1\n");
			fflush(stdout);
			
			// criando segmento de memoria
			if( (memoriaID = shmget( MEMFILHO1, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
			fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
			exit(1);
			}
			if( (memoriaEnd = (int *)shmat(memoriaID, NULL, 0)) == (int *)-1 ) {
			fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
			exit(1);
			}

			// criando semaforo
			if( ( semaforoID = semget( SEMFILHO1, 1, IPC_CREAT | 0666 ) ) == -1 ) {
			fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
			exit(1);
			}
			conteudoMensagem = (conteudo*) mensagem.mtext;
			conteudoMensagem->pid = getpid();
			conteudoMensagem->chaveMem = MEMFILHO1;
			conteudoMensagem->chaveSem = SEMFILHO1;
			printf("Filho 1 - PID: %i - Memoria Compartilhada: %i - Semaforo: %i\n" ,conteudoMensagem->pid,
			conteudoMensagem->chaveMem, conteudoMensagem->chaveSem);
			fflush(stdout);
			
			// colocar os dados na fila
			if(msgsnd(queue_id,(struct msgbuf *)&mensagem,sizeof(conteudo),0) == -1)    {
				perror("Envio de mensagem impossivel") ;
				exit(-1) ;
			}
			
			// travar semaforo (como foi inicilizado com 0 ficara travado aqui esperando o thread o destravar)
			   if( semop( semaforoID, g_sem_op1, 1 ) == -1 ) {
		                fprintf(stderr,"chamada semaap() falhou, impossivel fechar o recurso!");
		                exit(1);
              		  }
			printf("Filho 1 - PID: %i - Soma %i\n", conteudoMensagem->pid, *memoriaEnd);
			fflush(stdout);
			printf("SOu o filho 1 e finalizei\n");
			fflush(stdout);
			

			// destruir memoria compartilhada
			if( shmctl(memoriaID,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
		
               		 }
			// destruir semaforo
			if( semctl( semaforoID, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                	}

			exit(0);
		}
		/// filho 2
		else if(count == 2)
		{
			printf("SOu o filho 2\n");
			fflush(stdout);

			// criando segmento de memoria
			if( (memoriaID = shmget( MEMFILHO2, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
			fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
			exit(1);
			}
			if( (memoriaEnd = (int *)shmat(memoriaID, NULL, 0)) == (int *)-1 ) {
			fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
			exit(1);
			}

			// criando semaforo
			if( ( semaforoID = semget( SEMFILHO2, 1, IPC_CREAT | 0666 ) ) == -1 ) {
			fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
			exit(1);
			}
			conteudoMensagem = (conteudo*) mensagem.mtext;
			conteudoMensagem->pid = getpid();
			conteudoMensagem->chaveMem = MEMFILHO2;
			conteudoMensagem->chaveSem = SEMFILHO2;
			printf("Filho 2 - PID: %i - Memoria Compartilhada: %i - Semaforo: %i\n" ,conteudoMensagem->pid,
			conteudoMensagem->chaveMem, conteudoMensagem->chaveSem);
			fflush(stdout);
			// colocar os dados na fila
			if(msgsnd(queue_id,(struct msgbuf *)&mensagem,sizeof(conteudo),0) == -1)    {
				perror("Envio de mensagem impossivel") ;
				exit(-1) ;
			}

			// travar semaforo (como foi inicilizado com 0 ficara travado aqui esperando o thread o destravar)
			   if( semop( semaforoID, g_sem_op1, 1 ) == -1 ) {
		                fprintf(stderr,"chamada semaap() falhou, impossivel fechar o recurso!");
		                exit(1);
              		  }
			printf("Filho 2 - PID: %i - Soma %i\n", conteudoMensagem->pid, *memoriaEnd);
			fflush(stdout);
			printf("SOu o filho 2 e finalizei\n");
			fflush(stdout);


			// destruir memoria compartilhada
			if( shmctl(memoriaID,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
		
               		 }
			// destruir semaforo
			if( semctl( semaforoID, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                	}

			exit(0);
		}
		// filho 3
		else
		{
			printf("SOu o filho 3\n");
			fflush(stdout);

			// criando segmento de memoria
			if( (memoriaID = shmget( MEMFILHO3, sizeof(int), IPC_CREAT | 0666)) == -1 ) {
			fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
			exit(1);
			}
			if( (memoriaEnd = (int *)shmat(memoriaID, NULL, 0)) == (int *)-1 ) {
			fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
			exit(1);
			}

			// criando semaforo
			if( ( semaforoID = semget( SEMFILHO3, 1, IPC_CREAT | 0666 ) ) == -1 ) {
			fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
			exit(1);
			}
			conteudoMensagem = (conteudo*) mensagem.mtext;
			conteudoMensagem->pid = getpid();
			conteudoMensagem->chaveMem = MEMFILHO3;
			conteudoMensagem->chaveSem = SEMFILHO3;
			printf("Filho 3 - PID: %i - Memoria Compartilhada: %i - Semaforo: %i\n" ,conteudoMensagem->pid,
			conteudoMensagem->chaveMem, conteudoMensagem->chaveSem);
			fflush(stdout);
			// colocar os dados na fila
			if(msgsnd(queue_id,(struct msgbuf *)&mensagem,sizeof(conteudo),0) == -1)    {
				perror("Envio de mensagem impossivel") ;
				exit(-1) ;
			}

			// travar semaforo (como foi inicilizado com 0 ficara travado aqui esperando o thread o destravar)
			   if( semop( semaforoID, g_sem_op1, 1 ) == -1 ) {
		                fprintf(stderr,"chamada semaap() falhou, impossivel fechar o recurso!");
		                exit(1);
              		  }


			printf("Filho 3 - PID: %i - Soma %i\n", conteudoMensagem->pid, *memoriaEnd);
			fflush(stdout);
			printf("SOu o filho 3 e finalizei\n");
			fflush(stdout);



			// destruir memoria compartilhada
			if( shmctl(memoriaID,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada!\n");
                        exit(1);
		
               		 }
			// destruir semaforo
			if( semctl( semaforoID, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                	}


			exit(0);


		}

	}
	
	else
	{
		printf("SOu o pai\n");
		fflush(stdout);
		
		
	}
	wait(NULL);
  	wait(NULL);
	wait(NULL);




// remove fila
if( msgctl(queue_id,IPC_RMID,NULL) != 0 ) {
		fprintf(stderr,"Impossivel remover a fila!\n");
		exit(1);
		}





printf("Finalizando programa\n");
 
	exit(0);
}
