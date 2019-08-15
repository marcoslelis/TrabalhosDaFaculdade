
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>       // for gettimeofday() 
#include <errno.h>          // Usado para codigos de errno e error
#include <unistd.h>         // Usado no fork()
#include <signal.h>         // Usado para kill(), sigsuspend(), outros
#include <sys/types.h>      // Usado para wait
#include <sys/wait.h>       // Usado para wait
#include <sys/ipc.h>		// for msgget(), msgctl() 
#include <sys/msg.h>		// for msgget(), msgctl()
//#include <sys/shm.h>        // for shmget(), shmat(), shmctl()
//#include <sys/sem.h>        // for semget(), semop(), semctl() 
#include <string.h>

#define QTD_CADEIRAS        3
#define CLIENTES            20
#define BARBEIROS           2 
#define MESSAGE_QUEUE_ID    1214
#define SENDER_DELAY_TIME   10
#define MSG_CLIENTE         1
#define MSG_BARBEIRO        2
#define MSG_MEMORIA_COMPART 3
#define SHM_KEY				0x1452

void Barbeiro(int i);
void Cliente(int i);
void CortarCabelo(int cabelo[]);
void ApreciarCorte(int cabelo[]);

typedef struct {
	unsigned int SenhaCliente;
	unsigned int QtdCadeiras;
    char nome[20];
} data_t; 

typedef struct {
	long mtype;
	char mtext[sizeof(data_t)];
} msgbuf_t;

msgbuf_t message_buffer;
data_t *Data_ptr = (data_t *)(message_buffer.mtext);

int main(int argc, char *argv[])
{
    int rtn = 1;
    int count = 10;
    int queue_id;
	key_t key = MESSAGE_QUEUE_ID;


	Data_ptr->QtdCadeiras=0;

	if((queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) 
        {
			fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

	//Cliente eh liberado para iniciar corte------------------------------------------------------------------
	message_buffer.mtype = MSG_MEMORIA_COMPART;//Tipo de mensagem para controle de cliente
   	if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
		fprintf(stderr, "Impossivel enviar mensagem!\n");
		exit(1);
	}
	printf("Inicia memoria compartilhada UP\n");
	usleep(SENDER_DELAY_TIME);
	//--------------------------------------------------------------------------------------------------------


	for( count = 0; count < 8; count++) 
    {
		if( 0 != rtn ) 
        {
			rtn = fork();
		} else 
        {
			break;
		}
	}

	if( rtn == 0 && count == 1 ) 
    {
        printf("Barbeiro 1 iniciado ...\n");
        Barbeiro(count);
        exit(0);
	} 
    else if( rtn == 0 && count == 2 )
    {
        printf("Barbeiro 2 iniciado ...\n");
        Barbeiro(count);
        exit(0);
	}	 
    else if( rtn == 0 && count >= 3 ) 
    {
        printf("Cliente %d iniciado ...\n", count);	        
        Cliente(count);               
        exit(0);
	}
	else 
    {
        printf("Aguardando ...\n");
		wait(NULL);
		wait(NULL);
		wait(NULL);
        wait(NULL);
		wait(NULL);
		wait(NULL);

  	    if( msgctl(queue_id,IPC_RMID,NULL) != 0 ) 
        {
		    fprintf(stderr,"Impossivel remover a fila!\n");
		    exit(1);
	    }
    exit(0);
	}
}

void Barbeiro(int i)
{
    key_t key = MESSAGE_QUEUE_ID;
    int queue_id;
    int cont=0;
	
	if((queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

    printf("QueueID Barbeiro: %i\n", queue_id);


    while (1) 
    {

		//Barbeiro verifica se existe cliente para atender--------------------------------------------------------
		//Caso exista mensagem no buffer, ela eh consumida (trava leitura/escrita) e continua a execucao
		//Caso nao exista fica aguardano
		
		if(msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MSG_CLIENTE,0) == -1) {
			fprintf(stderr, "Printer-Impossivel receber mensagem!\n");
			exit(1);
    	} 
		printf("Barbeiro%i- cliente  DOWN\n", i);
		//--------------------------------------------------------------------------------------------------------


		//Verifica se pode acessar status da fila de espera-------------------------------------------------------
		//Caso exista mensagem no buffer, ela eh consumida (trava leitura/escrita) e continua a execucao
		//Caso nao exista fica aguardano

		if(msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MSG_MEMORIA_COMPART,0) == -1) {
			fprintf(stderr, "Printer-Impossivel receber mensagem!\n");
			exit(1);
    	} 
		printf("Barbeiro%i- memoria compartilhada  DOWN\n", i);
		//--------------------------------------------------------------------------------------------------------	

		
		Data_ptr->QtdCadeiras--;//espera-- (memoria compartilhada eh editada)


		//Leitura e escrita eh librea na memoria compartilhada----------------------------------------------------
		message_buffer.mtype = MSG_MEMORIA_COMPART;//Tipo de mensagem para controle de mem. compartilhada
   		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
		printf("Barbeiro%i- memoria compartilhada  UP\n", i);
		usleep(SENDER_DELAY_TIME);
		//--------------------------------------------------------------------------------------------------------


		//CortarCabelo();


		//Barbeiro termina corte e fica pronto para um novo corte-------------------------------------------------
		message_buffer.mtype = MSG_BARBEIRO;//Tipo de mensagem para controle de mem. compartilhada
   		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
		printf("Barbeiro%i- barbeiro UP\n", i);
		usleep(SENDER_DELAY_TIME);
		//--------------------------------------------------------------------------------------------------------
        printf("Barbeiro%i- terminou cliente %i \n", i, Data_ptr->SenhaCliente);			
    }
    exit(0);
}

void Cliente(int i)
{
    key_t key = MESSAGE_QUEUE_ID;
    int queue_id;
	int tempo;

	//Usado para simular tempos de chegada diferentes entre clientes----------------------------------------------
	//srand((unsigned)time(NULL)+i);																		       //|
	//tempo = rand()%10;																					       //|
	//printf("Cliente %i Aguardando %i segundos\n", i, tempo);											       //|
	//sleep(tempo);																						       //|	
	//------------------------------------------------------------------------------------------------------------


	//Cliente se prepara para enviar/receber mensagens------------------------------------------------------------
	if((queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
		}	
	//------------------------------------------------------------------------------------------------------------


	//Verifica se pode acessar status da fila de espera-----------------------------------------------------------
		//Caso exista mensagem no buffer, ela eh consumida (trava leitura/escrita) e continua a execucao
		//Caso nao exista fica aguardano

	if(msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MSG_MEMORIA_COMPART,0) == -1) {
		fprintf(stderr, "Printer-Impossivel receber mensagem!\n");
		exit(1);
    }
	printf("Cliente%i- memoria compartilhada  DOWN\n", i);
	//------------------------------------------------------------------------------------------------------------
	printf("------------------------Cliente%i Quantidade de lugares ocupados %i\n",i , Data_ptr->QtdCadeiras);
	if(Data_ptr->QtdCadeiras<QTD_CADEIRAS)
	{
		Data_ptr->QtdCadeiras++;//espera++ (memoria compartilhada eh editada)
		

		//Cliente eh liberado para iniciar corte------------------------------------------------------------------
		message_buffer.mtype = MSG_CLIENTE;//Tipo de mensagem para controle de cliente
   		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
		printf("Cliente%i- cliente UP\n", i);
		usleep(SENDER_DELAY_TIME);
		//--------------------------------------------------------------------------------------------------------


		//Leitura e escrita eh librea na memoria compartilhada----------------------------------------------------
		message_buffer.mtype = MSG_MEMORIA_COMPART;//Tipo de mensagem para controle de mem. compartilhada
   		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
		printf("Cliente%i- memoria compartilhada UP\n", i);
		usleep(SENDER_DELAY_TIME);
		//--------------------------------------------------------------------------------------------------------


		//Cliente aguarda termino do corte de cabelo--------------------------------------------------------------
		//Caso exista mensagem no buffer, ela eh consumida (trava leitura/escrita) e continua a execucao
		//Caso nao exista fica aguardano

		if(msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MSG_BARBEIRO,0) == -1) {
			fprintf(stderr, "Printer-Impossivel receber mensagem!\n");
			exit(1);
    	} 
		printf("Cliente%i- barbeiro DOWN\n", i);
		//--------------------------------------------------------------------------------------------------------


		//ApreciarCorte();
	}
	else
	{
		//Leitura e escrita eh librea na memoria compartilhada----------------------------------------------------
		message_buffer.mtype = MSG_MEMORIA_COMPART;//Tipo de mensagem para controle de mem. compartilhada
   		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
		usleep(SENDER_DELAY_TIME);
		//--------------------------------------------------------------------------------------------------------


		printf("Cliente%i foi embora!\n", i);
    	exit(0);  
	}
	exit(0);  
}

void CortarCabelo(int cabelo[])
{

}
void ApreciarCorte(int cabelo[])
{

}