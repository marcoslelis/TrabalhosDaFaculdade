
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
#include <string.h>

#define QTD_CADEIRAS        7
#define CLIENTES            20
#define BARBEIROS           2
#define MESSAGE_QUEUE_ID    4030
#define SENDER_DELAY_TIME   10
#define MSG_CLIENTE         1
#define MSG_BARBEIRO        2
#define MSG_MEMORIA_COMPART 3
#define SHM_KEY				0x1452
#define TAM_STRING			2048

void Barbeiro(int i);
void Cliente(int i);

void GeraString(int qtdelementos, unsigned char elementos[]);
void CortarCabelo(unsigned char cabelo[]);
void ApreciarCorte(unsigned char String_Des[]);

typedef struct {
	float tempo_corte;
	unsigned int IDCliente;
	unsigned int IDBarbeiro;
	unsigned int QtdCadeiras;
	unsigned char String_Ord[TAM_STRING];
	unsigned char String_Des[TAM_STRING];
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


	for( count = 0; count < CLIENTES + BARBEIROS; count++) 
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

	for( count = 0; count < CLIENTES; count++) 
    {
		wait(NULL);
	}


  	    if( msgctl(queue_id,IPC_RMID,NULL) != 0 ) 
        {
		    fprintf(stderr,"Impossivel remover a fila!\n");
		    exit(1);
	    }
    exit(0);
	}
}

void Barbeiro(int ID)
{
    key_t key = MESSAGE_QUEUE_ID;
    int queue_id;
    int cont=0;

	Data_ptr->IDBarbeiro = ID;
	
	if((queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

    while (1) 
    {

		//Barbeiro verifica se existe cliente para atender--------------------------------------------------------
		//Caso exista mensagem no buffer, ela eh consumida (trava leitura/escrita) e continua a execucao
		//Caso nao exista fica aguardano
		
		if(msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MSG_CLIENTE,0) == -1) {
			fprintf(stderr, "Printer-Impossivel receber mensagem!\n");
			exit(1);
    	} 

		//--------------------------------------------------------------------------------------------------------
		

		

		//Verifica se pode acessar status da fila de espera-------------------------------------------------------
		//Caso exista mensagem no buffer, ela eh consumida (trava leitura/escrita) e continua a execucao
		//Caso nao exista fica aguardano

		if(msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MSG_MEMORIA_COMPART,0) == -1) {
			fprintf(stderr, "Printer-Impossivel receber mensagem!\n");
			exit(1);
    	} 
		//--------------------------------------------------------------------------------------------------------	

		
		Data_ptr->QtdCadeiras--;//espera-- (memoria compartilhada eh editada)


		//Leitura e escrita eh librea na memoria compartilhada----------------------------------------------------
		message_buffer.mtype = MSG_MEMORIA_COMPART;//Tipo de mensagem para controle de mem. compartilhada
   		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
		usleep(SENDER_DELAY_TIME);
		//--------------------------------------------------------------------------------------------------------


		//CortarCabelo();		
		CortarCabelo(Data_ptr->String_Des);

		//Barbeiro termina corte e fica pronto para um novo corte-------------------------------------------------
		message_buffer.mtype = MSG_BARBEIRO;//Tipo de mensagem para controle de mem. compartilhada
   		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
		usleep(SENDER_DELAY_TIME);
		//--------------------------------------------------------------------------------------------------------
        			
    }
    exit(0);
}

void Cliente(int ID)
{
    key_t key = MESSAGE_QUEUE_ID;
    int queue_id;
	int tempo;

	//Usado para simular tempos de chegada diferentes entre clientes----------------------------------------------
	srand((unsigned)time(NULL)+ID);																		       //|
	tempo = rand()%5;																					       //|
	printf("Cliente %i Aguardando %i segundos\n", ID, tempo);											       //|
	sleep(tempo);																						       //|	
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

	//------------------------------------------------------------------------------------------------------------
	if(Data_ptr->QtdCadeiras<QTD_CADEIRAS)
	{
		Data_ptr->QtdCadeiras++;//espera++ (memoria compartilhada eh editada)	
		

		//Cliente eh liberado para iniciar corte------------------------------------------------------------------
		message_buffer.mtype = MSG_CLIENTE;//Tipo de mensagem para controle de cliente
   		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}

		usleep(SENDER_DELAY_TIME);
		//--------------------------------------------------------------------------------------------------------

		Data_ptr->IDCliente = ID;
		GeraString(50,Data_ptr->String_Des);

		//Leitura e escrita eh librea na memoria compartilhada----------------------------------------------------
		message_buffer.mtype = MSG_MEMORIA_COMPART;//Tipo de mensagem para controle de mem. compartilhada
   		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}

		usleep(SENDER_DELAY_TIME);
		//--------------------------------------------------------------------------------------------------------


		//Cliente aguarda termino do corte de cabelo--------------------------------------------------------------
		//Caso exista mensagem no buffer, ela eh consumida (trava leitura/escrita) e continua a execucao
		//Caso nao exista fica aguardano

		if(msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MSG_BARBEIRO,0) == -1) {
			fprintf(stderr, "Printer-Impossivel receber mensagem!\n");
			exit(1);
    	} 

		//--------------------------------------------------------------------------------------------------------
		printf("\n\n\nBarbeiro ID%i terminou o corte do cliente ID%i\n", Data_ptr->IDBarbeiro, Data_ptr->IDCliente);
		
		printf("Cliente %i\n", Data_ptr->IDCliente);
		printf("Barbeiro %i\n", Data_ptr->IDBarbeiro);
		ApreciarCorte(Data_ptr->String_Des);
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

		printf("Cliente%i foi embora!\n", ID);
    	exit(0);
	}
	exit(0);  
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