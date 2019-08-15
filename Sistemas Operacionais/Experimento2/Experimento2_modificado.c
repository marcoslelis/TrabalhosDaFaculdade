//Adriano de Oliveira Munin      RA:17066960
//Fábio Seiji Irokawa            RA:17057720
//Lucas Rodrigues Coutinho       RA:17776501
//Marcos Lelis de F. Oliveira    RA:16248387
//Paulo M. Birocchi              RA:16148363


#include <sys/time.h>		/* for gettimeofday() */
#include <stdio.h>			/* for printf() */
#include <unistd.h>			/* for fork() */
#include <sys/types.h>	/* for wait(), msgget(), msgctl() */
#include <wait.h>				/* for wait() */
#include <sys/ipc.h>		/* for msgget(), msgctl() */
#include <sys/msg.h>		/* for msgget(), msgctl() */
#include <stdlib.h>
#include <string.h>

#define NO_OF_ITERATIONS	500
#define MICRO_PER_SECOND	1000000
#define MESSAGE_QUEUE_ID	3102
#define MESSAGE_QUEUE_ID2	3103
#define SENDER_DELAY_TIME	10
#define MESSAGE_MTYPE			1
#define NO_OF_CHILDREN   	3

void Receiver(int tamanho_msg);
void Sender(int tamanho_msg);
void Printer();

//------------------------------------------------------------------------------------------------------------------- Structs

typedef struct {
	unsigned int msg_no;
	struct timeval send_time;
	float t_min;
	float t_medio;
	float t_max;
	float t_total;
	char *tamanho;
} data_t;

typedef struct {
	long mtype;
	char mtext[sizeof(data_t)];
} msgbuf_t;

//------------------------------------------------------------------------------------------------------------------- Main

int main( int argc, char *argv[] )
{
        int rtn = 1;
        int count = 10;
        int queue_id;
		int queue_id2;
		int tamanho_msg;
		key_t key = MESSAGE_QUEUE_ID;
		key_t key2 = MESSAGE_QUEUE_ID2;
		
		printf("Digite o um numero entre 1 e 10\n");
		scanf("%i", &tamanho_msg);
		tamanho_msg = tamanho_msg * 512;
		printf("O tamanho da mesnsagem enviada sera: %i\n", tamanho_msg);

	for( count = 0; count < NO_OF_CHILDREN; count++) {
		if( 0 != rtn ) {
			rtn = fork();
		} else {
			break;
		}
	}

	if( rtn == 0 && count == 1 ) {
                printf("Receptor iniciado ...\n");
                Receiver(tamanho_msg);
                exit(0);

	} else if( rtn == 0 && count == 2 ) {
                printf("Emissor iniciado ...\n");
                Sender(tamanho_msg);
                exit(0);
	}	 else if( rtn == 0 && count == 3 ) {
	                printf("Printer iniciado ...\n");
	                Printer();
	                exit(0);
	}
	else {
                printf("Pai aguardando ...\n");
								wait(NULL);
								wait(NULL);
								wait(NULL);
		//scanf("%i", &tamanho_msg);

		if((queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) {
			fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

		if((queue_id2 = msgget(key2, IPC_CREAT | 0666)) == -1 ) {
			fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
			exit(1);
		}

  	if( msgctl(queue_id,IPC_RMID,NULL) != 0 ) {
			fprintf(stderr,"Impossivel remover a fila!\n");
			exit(1);
		}
	if( msgctl(queue_id2,IPC_RMID,NULL) != 0 ) {
			fprintf(stderr,"Impossivel remover a fila!\n");
			exit(1);
		}	
            	exit(0);
	}
}

//-------------------------------------------------------------------------------------------------------------------Printer

void Printer()
{
	int count;
	int queue_id;

	key_t key = MESSAGE_QUEUE_ID2;

	queue_id = msgget(key, IPC_CREAT | 0666);

	if(queue_id == -1)
	{
		fprintf(stderr, "Impossivel criar ou entrar na fila de mensagens\n");
		exit(1);
	}

	msgbuf_t message_buffer;

	data_t *printer_ptr = (data_t *)(message_buffer.mtext);

	//for( count = 0; count < NO_OF_ITERATIONS; count++ ) {
		if(msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MESSAGE_MTYPE,0) == -1) {
			fprintf(stderr, "Printer-Impossivel receber mensagem!\n");
			exit(1);
		//}
		}


		printf("O tempo minimo foi de: %f\n", printer_ptr->t_min);
		printf("O tempo medio foi de: %f\n", printer_ptr->t_medio);
		printf("O tempo maximo foi de: %f\n", printer_ptr->t_max);
		printf("O tempo total foi de: %f\n", printer_ptr->t_total);

	return;
}

//------------------------------------------------------------------------------------------------------------------- Reciver

void Receiver(int tamanho_msg)
{
	int count;
	struct timeval receive_time;
	float delta=0;
	float max=0;
	float total=0;
	float min=99999;
	int queue_id;
	int queue_id2;
	key_t key = MESSAGE_QUEUE_ID;
	key_t key2 = MESSAGE_QUEUE_ID2;

	if((queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar ou entrar na fila de mensagens\n");
		exit(1);
	}

	if((queue_id2 = msgget(key2, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar ou entrar na fila de mensagens\n");
		exit(1);
	}

	msgbuf_t message_buffer;
	data_t *data_ptr = (data_t *)(message_buffer.mtext);

	for( count = 0; count < NO_OF_ITERATIONS; count++ ) {
		if( msgrcv(queue_id,(struct msgbuf *)&message_buffer,tamanho_msg,MESSAGE_MTYPE,0) == -1 ) {
			fprintf(stderr, "Receiver-Impossivel receber mensagem!\n");
			exit(1);
		}


		gettimeofday(&receive_time,NULL);
            	delta = receive_time.tv_sec  - data_ptr->send_time.tv_sec;
            	delta += (receive_time.tv_usec - data_ptr->send_time.tv_usec)/(float)MICRO_PER_SECOND;
							total += delta;

		if( delta > max ) {
			max = delta;
		}
		if( delta < min ) {
			min = delta;
		}

	}



	message_buffer.mtype = MESSAGE_MTYPE;

	data_ptr->t_min = min;
	data_ptr->t_medio = total / NO_OF_ITERATIONS;
	data_ptr->t_max = max;
	data_ptr->t_total = total;


	if( msgsnd(queue_id2,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
		fprintf(stderr, "Impossivel enviar mensagem!\n");
		exit(1);
	}
	usleep(SENDER_DELAY_TIME);

return;
}

//------------------------------------------------------------------------------------------------------------------- Sender

void Sender(int tamanho_msg)
{
	int queue_id;
	int count;
	int tam_msg;
	struct timeval send_time;
	msgbuf_t message_buffer;
	data_t *data_ptr = (data_t *)(message_buffer.mtext);
	key_t key = MESSAGE_QUEUE_ID;

	if((queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

	for( count = 0; count < NO_OF_ITERATIONS; count++ ) {
		gettimeofday(&send_time,NULL);
		message_buffer.mtype = MESSAGE_MTYPE;
		data_ptr->msg_no = count;
		data_ptr->send_time = send_time;
		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,tamanho_msg,0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
		usleep(SENDER_DELAY_TIME);
	}

  return;
}
