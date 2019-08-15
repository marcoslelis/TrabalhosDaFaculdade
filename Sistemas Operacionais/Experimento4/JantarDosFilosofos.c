#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define qtd_filosofo 5
#define qtd_garfo 5
#define tempo_pensando 25
#define tempo_comendo  25
#define no_of_interations 365

pthread_mutex_t garfo[qtd_garfo];
pthread_t       filosofo[qtd_filosofo];

void func_filosofo(int id);

int main(int argc, char *argv[])
{
    int i;

    for(i=0; i< qtd_garfo; i++)
    {
        pthread_mutex_init(&garfo[i], NULL);
    }

    for(i=0; i< qtd_filosofo; i++)
    {
       pthread_create(&filosofo[i], NULL, func_filosofo, (void *)i);
    }

    for(i=0; i< qtd_filosofo; i++)
    {
        pthread_join(filosofo[i], NULL);
    }
    
    pthread_exit(NULL);

    return 0;
}

void func_filosofo(int id)
{
    for(int i=0; i<no_of_interations; i++)
    {
        printf("O filosofo %d esta pensando...\n", id);
        usleep(tempo_pensando);

        pthread_mutex_lock(&garfo[id]);
        pthread_mutex_lock(&garfo[((id +1) % qtd_filosofo)]);

        usleep(tempo_comendo);
        printf("O filosofo %d esta comendo a porcao %d e falta %d...\n", id, i+1, no_of_interations - i -1);

        pthread_mutex_unlock(&garfo[id]);
        pthread_mutex_unlock(&garfo[((id +1) % qtd_filosofo)]);
    }

    pthread_exit(NULL);
}
