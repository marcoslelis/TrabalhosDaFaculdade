#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define QTD_BARBEIROS	3
#define QTD_CLIENTES	27
#define QTD_CADEIRAS	7
#define GETPID 11
#define MICRO_PER_SECOND	1000000
#define SEM_KEY_BAR 2003

#define BARBEIRO 0
#define CLIENTE  1 

void func_barbeiro(int id);
void func_cliente(int id);
void Apreciar_Corte(int id_cliente, float tempo_corte);

static pthread_mutex_t mutex_cadeiras; //= PTHREAD_MUTEX_INITIALIZER;

pthread_t thread_barbeiros_id[QTD_BARBEIROS];
pthread_t thread_clientes_id[QTD_CLIENTES];

int SemID;

struct timeval horario_inicio;
struct timeval horario_termino;

struct sembuf Up[1];
struct sembuf Down[1];

typedef struct
{
	int Id_Cliente;
	int Id_Barbeiro;
	int vetor[1023];

}struct_dados;


int teste[20];
int cadeiras_ocupadas = 0;

struct_dados dados[QTD_CLIENTES];


int main(int argc, char *argv[])
{
	//pthread_mutex_lock(&mutex_cadeiras);
	//pthread_mutex_unlock(&mutex_cadeiras);

	Up[0].sem_num = 0;
	Up[0].sem_op  = 1;
	Up[0].sem_flg = 0;

	Down[0].sem_num =  0;
	Down[0].sem_op  = -1;
	Down[0].sem_flg =  0;

	pthread_mutex_unlock(&cadeiras_ocupadas);

	SemID = semget( SEM_KEY_BAR, 2, IPC_CREAT | 0666 );
	if(SemID == -1)
	{
		printf("Semaforao nao criado! \n");
	}   

	for(int i=0; i<QTD_CLIENTES; i++)
	{
		dados[i].Id_Barbeiro = -1;
		dados[i].Id_Cliente = -1;
	}


	//Inicializacao e associacao das Threads -----------------------------------------------------------------
	int i;

	for(i = 0; i < QTD_BARBEIROS; i++)
	{
		pthread_create(&thread_barbeiros_id[i], NULL, func_barbeiro, i);
	}

	for(i = 0; i < QTD_CLIENTES; i++)
	{
		pthread_create(&thread_clientes_id[i], NULL, func_cliente, i);
	}

	//for(i = 0; i < QTD_BARBEIROS; i++)
	//{
	//	pthread_join(thread_barbeiros_id[i], NULL);
	//}

	for(i = 0; i < QTD_CLIENTES; i++)
	{
		pthread_join(thread_clientes_id[i], NULL);
	}
	//Fim das Threads ----------------------------------------------------------------------------------------------
  
	if( semctl( SemID, 0, IPC_RMID, 0) != 0 ) 
	{
    fprintf(stderr,"Impossivel remover o conjunto de semaforos!\n");
    exit(1);
  }

	for(i = 0; i<QTD_CLIENTES; i++)
	{
		//printf("---------------------------------------------------------------\n");
		//printf("Id_Barbeiro: %d \n", dados[i].Id_Barbeiro);
		//printf("Id_Cliente: %d \n", dados[i].Id_Cliente);
	}

	return 0;
}

void func_barbeiro(int id)
{
	printf("Barbeiro %d: %d \n", id, cadeiras_ocupadas);

	while (1) 
    {		
		//Barbeiro verifica se existe cliente para atender--------------------------------------------------------
		Down[0].sem_num = CLIENTE;
		semop(SemID, Down, 1);
		//--------------------------------------------------------------------------------------------------------
		
		//Verifica se pode acessar status da fila de espera-------------------------------------------------------
		pthread_mutex_lock(&mutex_cadeiras);
		//--------------------------------------------------------------------------------------------------------

		for(int i=0; i < QTD_CLIENTES; i++)
		{
			if(dados[i].Id_Barbeiro == -1 && dados[i].Id_Cliente != -1)
			{
				dados[i].Id_Barbeiro = id;
			}

		}

		cadeiras_ocupadas--;//(memoria compartilhada eh editada)

		//Leitura e escrita eh librea na memoria compartilhada----------------------------------------------------				
		pthread_mutex_unlock(&mutex_cadeiras);
		//--------------------------------------------------------------------------------------------------------

		//CortarCabelo();

		for(int i=0; i < 15000; i++)
		{
			i++;
			i--;
		}
		//Barbeiro termina corte e fica pronto para um novo corte-------------------------------------------------
		Up[0].sem_num = BARBEIRO;
			semop(SemID, Up, 1);
		//--------------------------------------------------------------------------------------------------------        		
    }
    exit(0);
}

void func_cliente(int id)
{
	float tempo_corte;
	dados[id].Id_Cliente = id;
	//printf("Cliente %d iniciado...\n", id);
	
	//Usado para simular tempos de chegada diferentes entre clientes----------------------------------------------
	//srand((unsigned)time(NULL)+i);																		   																		 //|
	//tempo = rand()%10;																					  																					 //|
	//printf("Cliente %i Aguardando %i segundos\n", i, tempo);											      										 //|
	//sleep(tempo);																						       																					 //|
	//------------------------------------------------------------------------------------------------------------

	//Cliente se prepara para enviar/receber mensagens------------------------------------------------------------
		pthread_mutex_lock(&mutex_cadeiras);
	//------------------------------------------------------------------------------------------------------------
	
	//Verifica se pode acessar status da fila de espera-----------------------------------------------------------
		printf("Qtd cadeiras ocupadas %i\n", cadeiras_ocupadas);
		if(cadeiras_ocupadas < QTD_CADEIRAS)
		{
			//espera++ (memoria compartilhada eh editada)-----------------------------------------------------------------
				cadeiras_ocupadas++;
				 gettimeofday( &horario_inicio, NULL );
			//------------------------------------------------------------------------------------------------------------

			//Cliente eh liberado para iniciar corte----------------------------------------------------------------------
				Up[0].sem_num = CLIENTE;
				semop(SemID, Up, 1);
			//------------------------------------------------------------------------------------------------------------
			
			//Leitura e escrita eh librea na memoria compartilhada--------------------------------------------------------
				pthread_mutex_unlock(&mutex_cadeiras);
			//------------------------------------------------------------------------------------------------------------

			//Cliente aguarda termino do corte de cabelo------------------------------------------------------------------
				Down[0].sem_num = BARBEIRO;
				semop(SemID, Down, 1);
			//------------------------------------------------------------------------------------------------------------
			   gettimeofday( &horario_termino, NULL );

				//Calcula o tempo desde entrar até iniciar Apreciar_corte()
           tempo_corte = (horario_termino.tv_usec - horario_inicio.tv_usec);///(float)MICRO_PER_SECOND;
        
			//ApreciarCorte();--------------------------------------------------------------------------------------------
				Apreciar_Corte(id, tempo_corte);
			//------------------------------------------------------------------------------------------------------------
		}

		else
		{
			//Leitura e escrita eh liberada na memoria compartilhada--------------------------------------------------------
				pthread_mutex_unlock(&mutex_cadeiras);
			//------------------------------------------------------------------------------------------------------------

				printf("Cliente %d foi embora!\n", id);
				//exit(0);  
		}
		pthread_exit(NULL);
}
	//----------------------------------------------------------------------------------------------------------------

void Apreciar_Corte(int id_cliente, float tempo_corte)
{
	tempo_corte = (horario_termino.tv_usec - horario_inicio.tv_usec);
	printf("Tempo do cliente %i em usec: %.2f\n", id_cliente, tempo_corte);

	printf("Corte Apreciado pelo cliente %d!!! \n", id_cliente);
	return;
}
