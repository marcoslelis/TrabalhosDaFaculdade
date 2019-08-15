/***********************************************************************************
*
* Este programa não faz parte do curso sobre tempo real do Laboratorio Embry-Riddle
* embora tenha sido inspirado pelos demais experimentos lah existentes.
*
* Experimento # 4 na disciplina de Sistemas Operacionais da PUC-Campinas
* Originalmente programado por Florian Weizenegger
*							Data: 25/08/2003
* 
*       Proposito: O proposito deste programa e o de permitir ao aluno perceber
*       o que vem a ser um thread, de maneira tal que consiga distingui-lo de
*       um processo. Além disso, são usados os principais comandos para criação
*	e manipulação de threads.
*	O problema dos produtores e consumidores sobre um buffer circular é
*	usado como assunto, permitindo que o aluno experimente duas implementações
*	diferentes para sua solução. Desta maneira, além dos threads propriamente
*	ditos, tambem locks e semaforos sao usados para garantir sincronizacao
*	de threads.
*
*************************************************************************************/

/*
 * Includes Necessarios 
 */

#include <pthread.h>			/* para poder manipular threads */
#include <stdio.h>			/* para printf() */
#include <stdlib.h>

/*
 * Constantes Necessarias 
 */
#define NUM_THREADS       10
#define SIZEOFBUFFER      50
#define NO_OF_ITERATIONS  100

/*
 * O tipo pthread_t permite a declaração de uma variável que recebe
 * um id quando o thread é criado. Posteriormente, esse id pode ser
 * usado em comandos de controle para threads.
 * Seguem dois vetores para ids, para um numero de threads igual a
 * constante NUM_THREADS
 */
pthread_t consumers[NUM_THREADS];
pthread_t producers[NUM_THREADS];

/*
 * Variaveis Necessarias 
 */
int buffer[SIZEOFBUFFER];		/* Este e um buffer circular	*/
int *start;				          /* apontara para a primeira posicao do buffer */
int *rp;				            /* eh o apontador para o proximo item do buffer a ser consumido */
int *wp;				            /* eh o apontador para o proximo item do buffer a ser produzido */
int cont_p;             		/* eh um contador para controlar o numero de itens produzidos */
int cont_c = 0;         		/* eh um contador para controlar o numero de itens consumidos */
int elementos = 0;

/*
 * Rotina para produzir um item toAdd no buffer 
 */
int myadd(int toAdd) {

  if (elementos<SIZEOFBUFFER) {
    wp++;
    elementos++;
    if (wp == (start + SIZEOFBUFFER)) {
      wp = start;	
    }
    *wp = toAdd;
    return 1;
  } else return 0;
}

int myremove() {
int retValue;

  //verificacao se o buffer nao esta vazio
  if (elementos>0) {
    retValue = *rp;
    elementos--;
    rp++;
    //verificacao se rp chegou a ultima posicao do buffer
	if (rp == (start + SIZEOFBUFFER)) {
       rp = start;				/* realiza a circularidade no buffer */
    }
    return retValue;
  } else return 0;
}

/*
 * A rotina produce e responsavel por chamar myadd para que seja 
 * colocado o valor 10 em uma posicao do buffer NO_OF_ITERATIONS vezes
 */
void *produce(void *threadid)
{
  int sum;
  int ret = 0;
  int buffCheio=0;

  printf("Produtor #%d iniciou...\n", threadid);

  while (cont_p < NO_OF_ITERATIONS) {
    usleep(1);
    ret = myadd(10);
    if (ret) {
      cont_p++;
      sum += 10;
    }
    else {
    //printf("Buffer cheio!\n");	
    buffCheio++;
    //break;
    }

  }
  printf("Soma produzida pelo Produtor #%d : %d\n", threadid, sum);
  printf("A trhead #%d nao conseguiu produzir %d vezes\n", threadid, buffCheio);
  pthread_exit(NULL);
}

/*
 * A rotina consume e responsavel por chamar myremove para que seja
 * retorando um dos valores existentes no buffer NO_OF_ITERATIONS vezes 
 */
void *consume(void *threadid)
{
  int sum = 0;
  int ret;
  int buffVazio=0;

  printf("Consumidor #%d iniciou...\n", threadid);

  while (cont_c < NO_OF_ITERATIONS) {
    usleep(100);
    ret = myremove();
    if (ret != 0) {
      cont_c++;
      sum += ret;
    }
    else {
    //printf("Buffer vazio!\n");
    buffVazio++;	
    //break;
    }
  }
  printf("Soma do que foi consumido pelo Consumidor #%d : %d\n", threadid, sum);
  printf("A trhead #%d nao conseguiu consumir %d vezes\n", threadid, buffVazio);
  pthread_exit(NULL);
}

/*
 * Rotina Principal (que tambem e a thread principal, quando executada) 
 */
int main(int argc, char *argv[])
{
  int tp, tc;
  int i;
  void *ret;
  start = &buffer[0];
  wp = start + SIZEOFBUFFER - 1;
  rp = start-1;
  
  for (i=0;i<NUM_THREADS;i++) {

    // tenta criar um thread consumidor
    tc = pthread_create(&consumers[i], NULL, consume, (void *)i+1);

    if (tc) {
      printf("ERRO: impossivel criar um thread consumidor\n");
      exit(-1);
    }
    // tenta criar um thread produtor
    tp = pthread_create(&producers[i], NULL, produce, (void *)i+1);
    
    if (tp) {
      printf("ERRO: impossivel criar um thread rodutor\n");
      exit(-1);
    }
  }
  for (i=0;i<NUM_THREADS;i++) {
   pthread_join(producers[i], &ret);
   pthread_join(consumers[i], &ret);
  }

  printf("Terminando a thread main()\n");
  pthread_exit(NULL);
}