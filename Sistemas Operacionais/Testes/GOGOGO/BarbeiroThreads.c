#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/time.h>       // for gettimeofday() 

#define QTD_BARBEIROS	3
#define QTD_CLIENTES	27
#define QTD_CADEIRAS	7
#define GETPID 11
#define MICRO_PER_SECOND 1000000
#define SEM_KEY_BAR 2333
#define TAMANHO_STRING 2048

#define BARBEIRO 0
#define CLIENTE  1 

void func_barbeiro(int id);
void func_cliente(int id);
struct timeval Apreciar_Corte();
void GeraString(int qtdelementos, unsigned char elementos[]);
void CortarCabelo(unsigned char cabelo[]);
void ApreciarCorte(unsigned char String_Des[]);

static pthread_mutex_t mutex_cadeiras;
static pthread_mutex_t mutex_print;

pthread_t thread_barbeiros_id[QTD_BARBEIROS];
pthread_t thread_clientes_id[QTD_CLIENTES];

int SemID;

struct sembuf Up[1];
struct sembuf Down[1];

typedef struct
{
	int Id_Cliente;
	int Id_Barbeiro;
	unsigned char String_Des[TAMANHO_STRING];
	unsigned char String_Ord[TAMANHO_STRING];

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
	//printf("Barbeiro %d: %d \n", id, cadeiras_ocupadas);

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
				CortarCabelo(dados[i].String_Ord);
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
	struct timeval horario_inicio;
	struct timeval horario_termino;
	float tempo_espera;
	int qtdelementos;
	int i;

	dados[id].Id_Cliente = id;
	pthread_mutex_lock(&mutex_print);
	srand((unsigned)time(NULL));
	qtdelementos =  rand()%1023;
	//printf("%d \n", qtdelementos);
	GeraString(qtdelementos, dados[id].String_Des);
	//printf("%c", dados[id].String_Ord[qtdelementos]);
	for(i=0; i<qtdelementos; i++)
	{
		dados[id].String_Ord[i] = dados[id].String_Des[i]; 
	}
	dados[id].String_Ord[i]='\n';
	pthread_mutex_unlock(&mutex_print);
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
	//	printf("Qtd cadeiras ocupadas %i\n", cadeiras_ocupadas);
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
			   

			//Calcula o tempo desde entrar até iniciar Apreciar_corte()--------------------------------------------------

				horario_termino = Apreciar_Corte();
				tempo_espera = (horario_termino.tv_usec - horario_inicio.tv_usec);
			//------------------------------------------------------------------------------------------------------------

			//Prints------------------------------------------------------------------------------------------------------

				pthread_mutex_lock(&mutex_print);
				printf("--------------------------------------------------------------------------------------------------------- \n");
				printf("Cliente atendido No. %d. \n", id);
				printf("Barbeiro que atendeu foi o No. %d. \n", dados[id].Id_Barbeiro);
				printf("O cliente %d demorou %.2f microsegundos na barbearia. \n", id, tempo_espera);
				printf("\n");
				printf("String a ser ordenda: \n");
				ApreciarCorte(dados[id].String_Des);
				printf("\n\n");
				printf("String a ja ordenda: \n");
				ApreciarCorte(dados[id].String_Ord);
				printf("\n\n");
				printf("--------------------------------------------------------------------------------------------------------- \n");
				printf("\n \n \n");

				pthread_mutex_unlock(&mutex_print);

			//------------------------------------------------------------------------------------------------------------
		}

		else
		{
			//Leitura e escrita eh liberada na memoria compartilhada--------------------------------------------------------
				pthread_mutex_unlock(&mutex_cadeiras);
			//------------------------------------------------------------------------------------------------------------

				//printf("Cliente %d foi embora!\n", id);
				//exit(0);  
		}
		pthread_exit(NULL);
}
	//----------------------------------------------------------------------------------------------------------------

struct timeval Apreciar_Corte()
{
	struct timeval horario_termino;
	gettimeofday( &horario_termino, NULL );
	return horario_termino;
}

void GeraString(int qtdelementos, unsigned char elementos[])
{
	unsigned int num, quebrado=0, i;
    unsigned char MSB=0, LSB=0;

    srand((unsigned)time(NULL));
    		
    for (i = 0; i<qtdelementos*2; i++)
    {
	    num =  rand()%1023;
        LSB = num & 0xFF;
        MSB = (num >> 8) & 0xFF;
        elementos[i] = MSB;
        i++;
        elementos[i] = LSB;
    }

    elementos[i] = '\n';
}

void CortarCabelo(unsigned char cabelo[])
{
    int i = 0, j = 0;
    unsigned char MSB, LSB;

    while(1)
    {
        if(cabelo[i] != '\n')
        {   
            j = 0;
            while(1)
            {
                if(cabelo[j] != '\n')
                {
                    if((cabelo[i] << 8 | cabelo[i+1]) < (cabelo[j] << 8 | cabelo[j+1]))
                    {
                        MSB = cabelo[j];
                        LSB = cabelo[j+1];
                        cabelo[j] = cabelo[i];
                        cabelo[j+1] = cabelo[i+1];
                        cabelo[i] = MSB;
                        cabelo[i+1] = LSB;
                    }     
                }
                else
                {
                    break;
                }
                j=j+2;
            }
        }
        else
        {
            break;          
        }
        i=i+2;          
    }
    cabelo[i] = '\n';
}

void ApreciarCorte(unsigned char String_Des[])
{	
	int i=0;

	while(1)
    {   
		if(String_Des[i] != '\n')
		{
			printf("%i ,", (String_Des[i] << 8 | String_Des[i+1]));
        	i=i+2;
		}
		else
		{
			break;
		}

	}
}