//Adriano de Oliveira Munin      RA:17066960
//Fábio Seiji Irokawa            RA:17057720
//Lucas Rodrigues Coutinho       RA:17776501
//Marcos Lelis de Freitas Oliveira    RA:16248387
//Paulo M. Birocchi              RA:16148363

#include <sys/time.h>		/* for gettimeofday() */
#include <stdio.h>			/* for printf() */
#include <unistd.h>			/* for fork() */
#include <sys/types.h>		/* for wait(), msgget(), msgctl() */
#include <wait.h>			/* for wait() */
#include <sys/ipc.h>		/* for msgget(), msgctl() */
#include <sys/msg.h>		/* for msgget(), msgctl() */
#include <stdlib.h>


#define NO_OF_ITERATIONS	500
#define MICRO_PER_SECOND	1000000
#define MESSAGE_QUEUE_ID	3102
#define SENDER_DELAY_TIME	10
#define MESSAGE_MTYPE		1
#define NO_OF_CHILDREN   	2


void Receiver(int queue_id);
void Sender(int queue_id);

typedef struct {
	unsigned int msg_no;
	struct timeval send_time;
} data_t; 

typedef struct {
	long mtype;
	char mtext[sizeof(data_t)];
} msgbuf_t;

int main( int argc, char *argv[] )
{
        int rtn = 1;
        int count = 0;
        int queue_id;
        key_t key = MESSAGE_QUEUE_ID;

        if((queue_id = msgget(key, IPC_CREAT | 0666)) == -1 ) {
		fprintf(stderr,"Impossivel criar a fila de mensagens!\n");
		exit(1);
	}

	for( count = 0; count < NO_OF_CHILDREN; count++) {
		if( 0 != rtn ) {
			rtn = fork();
		} else {
			break;
		}
	}
	
	if( rtn == 0 && count == 1 ) {
                printf("Receptor iniciado ...\n");
                Receiver(queue_id);
                exit(0);

	} else if( rtn == 0 && count == 2 ) {
                printf("Emissor iniciado ...\n");
                Sender(queue_id);
                exit(0);
	} else {
                printf("Pai aguardando ...\n");
		wait(NULL);
		wait(NULL);
		
            if( msgctl(queue_id,IPC_RMID,NULL) != 0 ) {
			fprintf(stderr,"Impossivel remover a fila!\n");
			exit(1);
		}
            	exit(0);
	}
}

void Receiver(int queue_id)
{
	int count;
	struct timeval receive_time;
	float delta;
	float max;
	float total;

	msgbuf_t message_buffer;
	data_t *data_ptr = (data_t *)(message_buffer.mtext);

	for( count = 0; count < NO_OF_ITERATIONS; count++ ) {
		if( msgrcv(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),MESSAGE_MTYPE,0) == -1 ) {
			fprintf(stderr, "Impossivel receber mensagem!\n");
			exit(1);
		}

		gettimeofday(&receive_time,NULL);
            	delta = receive_time.tv_sec  - data_ptr->send_time.tv_sec;
            	delta += (receive_time.tv_usec - data_ptr->send_time.tv_usec)/(float)MICRO_PER_SECOND;
		total += delta;

		if( delta > max ) {
			max = delta;
		}
	}

	printf( "O tempo medio de transferencia: %.6f\n", total / NO_OF_ITERATIONS );
	fprintf(stdout, "O tempo maximo de transferencia: %.6f\n", max );

    	return;
}

void Sender(int queue_id)
{

	int count;
	struct timeval send_time;
	msgbuf_t message_buffer;
	data_t *data_ptr = (data_t *)(message_buffer.mtext);

	for( count = 0; count < NO_OF_ITERATIONS; count++ ) {
		gettimeofday(&send_time,NULL);
		message_buffer.mtype = MESSAGE_MTYPE;
		data_ptr->msg_no = count;
		data_ptr->send_time = send_time;
		if( msgsnd(queue_id,(struct msgbuf *)&message_buffer,sizeof(data_t),0) == -1 ) {
			fprintf(stderr, "Impossivel enviar mensagem!\n");
			exit(1);
		}
		usleep(SENDER_DELAY_TIME);
	}
	return;
}
