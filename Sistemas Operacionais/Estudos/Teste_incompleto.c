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

#define IDFILA 123
#define IDMEM1  1
#define IDMEM2  2
#define IDMEM3  3
#define SEM1    11
#define SEM2    22
#define SEM3    33


struct sembuf	g_sem_op1[1];
struct sembuf	g_sem_op2[1];


typedef struct {
int idMem;
int PID;
int idSem;
} dados

typedef struct {
	long mtype;
	char mtext[sizeof(dados)];
} msg;

	g_sem_op1[0].sem_num   =  0;
	g_sem_op1[0].sem_op    = -1;
	g_sem_op1[0].sem_flg   =  0;

	g_sem_op2[0].sem_num   =  0;
	g_sem_op2[0].sem_op    =  1;
	g_sem_op2[0].sem_flg   =  0;



int main()
{
    int count;
    int memoria_id;
    int semaforo_id;
    int fila_id;
    int *endereco_memoria;
    dados *conteudo;
    msg mensagem;
  
    
    
	for( count = 0; count < 3; count++) {
		if( 0 != rtn ) {
			rtn = fork();
		} else {
			break;
		}
	}

    if(rtn == 0)
    {
        if (count == 0)
        {
            printf("Iniciando filho %i\n", count+1);
            
            if( (memoria_id = shmget( IDMEM1, sizeof(int), 0666 | IPC_CREAT)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	    }
           if( (endereco_memoria = (int *)shmat(memoria_id, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	    }

        if( ( semaforo_id = semget( SEM1, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	    }
        
        
        // manda os dados pra fila
        conteudo = (dados *) mensagem.mtext;
        conteudo->idMem = IDMEM1;
        conteudo->PID = getpid();
        conteudo->idSem = SEM1;

        if((fila_id = msgget(IDFILA, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar ou entrar na fila de mensagens\n");
		exit(1);

        if( msgsnd(fila_id(struct msg *)&mensagem ,sizeof(dados),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
            
        // trava o semaforo (espera pela thread)
        if( semop( semaforo_id, g_sem_op1, 1 ) == -1 ) {      		
          fprintf(stderr,"chamada semop() falhou, impossivel travar o recurso!");
         	exit(1);
               	}
        
        printf("Filho %i PID %i, 

        
        
        










        }


        if(count == 1)
        {
            
        }


        if(count == 2)
        {

        }











    }


}




