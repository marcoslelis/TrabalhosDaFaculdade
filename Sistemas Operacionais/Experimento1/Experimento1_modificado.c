/*******************************************************************************
* Este programa esta baseado no segundo experimento do curso sobre tempo real 
* do Laboratorio Embry-Riddle
* 
* Seguem os comentarios originais:
*
* Experiment #2: Multi-Tasking, Measuring Drift
*
*    Programmer: Eric Sorton
*          Date: 1/27/97
*           For: MSE599, Special Topics Class
*
*       Purpose: When a basic sleep call is used to determine the precise time
*                when an action will occur the problem of drift occurs. 
*                The measurement of time is imprecise.  Similarly, the time in 
*                which the sleep call returns is imprecise.  Over time, this 
*                will cause the ocurrence of time to drift.  Just as if a clock 
*                loses 1 second every day, over one day, it is significant, but 
*                over a year, it loses 365 seconds, which is over 6 minutes.  
*                This is an example of drift.
*
*       Proposito: Quando uma chamada básica sleep e usada para determinar o
*                instante exato em que alguma acao vai ocorrer, ocorre o problema
*                do desvio. A medicao de tempo e imprecisa. Similarmente, o tempo
*                que demora o retorno da chamada sleep tambem e impreciso. Ao
*                longo do tempo, isto ocasionara um desvio de tempo. Algo como se
*                um relogio perdesse um segundo a cada dia. Ao longo de um dia, 
*                essa diferenca e insignificante, mas, ao longo de um ano, sao 
*                perdidos 365 segundos, o que e superior a 6 minutos. Este e um
*                exemplo de desvio.
*
*******************************************************************************/

/*
 * Includes Necessarios, verifique se as bibliotecas no diretorio sys/ estao
 * lah. Caso nao estejam, verifique onde estao e altere o include
 */

#include <sys/time.h>		/* for gettimeofday() */
#include <unistd.h>		/* for gettimeofday() and fork() */
#include <stdio.h>		/* for printf() */
#include <sys/types.h>		/* for wait() */
#include <sys/wait.h>		/* for wait() */
#include <stdlib.h>
#include <string.h>

//Adriano de Oliveira Munin      RA:17066960
//Fábio Seiji Irokawa            RA:17057720
//Lucas Rodrigues Coutinho       RA:17776501
//Marcos Lelis de F. Oliveira    RA:16248387
//Paulo M. Birocchi              RA:16148363

#define NO_OF_CHILDREN	5

/*
 * SLEEP_TIME corresponde a quantidade de tempo para ficar bloqueado.
 */

#define SLEEP_TIME 1000

int main( int argc, char *argv[] )
{
	/*
	 * start_time e stop_time conterao o valor de tempo antes e depois
	 * que as trocas de contexto comecem
         */

	struct timeval start_time;
	struct timeval stop_time;

      /*
       * Outras variaveis importantes
       */

      float drift;
      int count;
      int child_no;
      int rtn;
      char contador [100];
      char tempo_sleep[100];

	/*
	 * Criacao dos processos filhos
	 */

	rtn = 1;
	for( count = 0; count < NO_OF_CHILDREN; count++ ) {
		if( rtn != 0 ) {
			rtn = fork();
		} else {
			break;
		}
	}


	/*
	 * Verifica-se rtn para determinar se o processo eh pai ou filho
	 */

	if( rtn == 0 ) {


        snprintf(contador, sizeof(contador), "%d", count);
        snprintf(tempo_sleep, sizeof(tempo_sleep), "%d", 400 + (200 * (count - 1)));
        execl("filho", "filho", contador, tempo_sleep, (char *) NULL);

	} else {
		/*
		 * Sou pai, aguardo o termino dos filhos
		 */
		for( count = 0; count < NO_OF_CHILDREN; count++ ) {
			wait(NULL);
		}
	}

	exit(0);
}
