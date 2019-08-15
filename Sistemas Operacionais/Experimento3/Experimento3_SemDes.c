
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
#include <stdlib.h>

#define PROTECT
#define SEM_KEY		0x1451
#define SHM_KEY		0x1452
#define NO_OF_CHILDREN	5

int i;
int	g_sem_id;
int	g_shm_id;
int *g_shm_addr;
key_t chave = SHM_KEY;

struct sembuf	g_sem_op1[1];
struct sembuf	g_sem_op2[1];


char g_letters_and_numbers[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz 1234567890";

void PrintChars( void );


int main( int argc, char *argv[] )
{

      int rtn;
      int count;
      int pid[NO_OF_CHILDREN];
       

	g_sem_op1[0].sem_num   =  0;
	g_sem_op1[0].sem_op    = -1;
	g_sem_op1[0].sem_flg   =  0;

	g_sem_op2[0].sem_num   =  0;
	g_sem_op2[0].sem_op    =  1;
	g_sem_op2[0].sem_flg   =  0;


                    ////////////////////////////////// SEMAFORO ////////////////////////////////

	
	if( ( g_sem_id = semget( SEM_KEY, 1, IPC_CREAT | 0666 ) ) == -1 ) {
		fprintf(stderr,"chamada a semget() falhou, impossivel criar o conjunto de semaforos!");
		exit(1);
	}

    printf("id do semaforo:  %i\n", g_sem_id);
	
	if( semop( g_sem_id, g_sem_op2, 1 ) == -1 ) {
		fprintf(stderr,"chamada semop() falhou, impossivel inicializar o semaforo!");
		exit(1);
	}


                    ////////////////////////////////// MEMORIA COMPARTILHADA ////////////////////////////////


   

	if( (g_shm_id = shmget( SHM_KEY, sizeof(int), 0666 | IPC_CREAT)) == -1 ) {
		fprintf(stderr,"Impossivel criar o segmento de memoria compartilhada!\n");
		exit(1);
	}
	if( (g_shm_addr = (int *)shmat(g_shm_id, NULL, 0)) == (int *)-1 ) {
		fprintf(stderr,"Impossivel associar o segmento de memoria compartilhada!\n");
		exit(1);
	}

    *g_shm_addr = 0;

       rtn = 1;
       for( count = 0; count < NO_OF_CHILDREN; count++ ) {
               if( rtn != 0 ) {
                       pid[count] = rtn = fork();
                    //printf("Criou filho%i\n", count);
               } else {
                       break;
               }
	   }

       if( rtn == 0 ) {

                printf("Filho %i comecou ...\n", count);
		
				PrintChars();

        } else {
                usleep(15000);
        }
    
	for( i=0; i<count; i++){
		kill(pid[i], SIGKILL);
	} 

                if( shmctl(g_shm_id,IPC_RMID,NULL) != 0 ) {
                        fprintf(stderr,"Impossivel remover o segmento de memoria compartilhada\n");
                        exit(1);
                }

                if( semctl( g_sem_id, 0, IPC_RMID, 0) != 0 ) {
                        fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
                        exit(1);
                }

                exit(0);
        }


void PrintChars( void )
{
	struct timeval tv;
	int number;

	int tmp_index;
	int i;


	usleep(200);

	while(1) {

		if( gettimeofday( &tv, NULL ) == -1 ) {
			fprintf(stderr,"Impossivel conseguir o tempo atual, terminando.\n");
			exit(1);
		}
		number = ((tv.tv_usec / 47) % 3) + 1;


/*#ifdef PROTECT
                    
		if( semop( g_sem_id, g_sem_op1, 1 ) == -1 ) {
           fprintf(stderr,"chamada semop() falhou, impossivel fechar o recurso!");
            exit(1);
       	 }
#endif*/
     
		tmp_index = *g_shm_addr;

           for( i = 0; i < number; i++ ) {
			if(  (tmp_index + i < sizeof(g_letters_and_numbers)) ) {
				fprintf(stderr,"%c", g_letters_and_numbers[tmp_index + i]);
				usleep(1);
			}
		
     
            
		*g_shm_addr = tmp_index + i + 1;

		if( tmp_index + i > sizeof(g_letters_and_numbers) ) {
			fprintf(stderr,"\n");
			*g_shm_addr = 0;
		}
  
  }
/*#ifdef PROTECT
		if( semop( g_sem_id, g_sem_op2, 1 ) == -1 ) {      		
          fprintf(stderr,"chamada semop() falhou, impossivel liberar o recurso!");
         	exit(1);
               	}
#endif*/

	}
}
