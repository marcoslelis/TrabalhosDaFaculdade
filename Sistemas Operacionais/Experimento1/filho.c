#include <sys/time.h> /* for gettimeofday() */
#include <unistd.h> /* for gettimeofday() and fork() */
#include <stdio.h> /* for printf() */
#include <sys/types.h> /* for wait() */
#include <sys/wait.h> /* for wait() */
#include <stdlib.h>

//Adriano de Oliveira Munin      RA:17066960
//Fábio Seiji Irokawa            RA:17057720
//Lucas Rodrigues Coutinho       RA:17776501
//Marcos Lelis de F. Oliveira    RA:16248387
//Paulo M. Birocchi              RA:16148363




#define NO_OF_ITERATIONS	1000

#define MICRO_PER_SECOND	1000000


        int main( int argc, char *argv[])
{
    
    int count;
    float sleep;    
   	struct timeval start_time;
	struct timeval stop_time;
     float drift;
    int c;

    count = atoi(argv[1]);  
    sleep = atoi(argv[2]);

		gettimeofday( &start_time, NULL );

		for( c= 0; c < NO_OF_ITERATIONS; c++ ) {
			usleep(sleep);
		}

		
		gettimeofday( &stop_time, NULL );

	
		drift = (float)(stop_time.tv_sec  - start_time.tv_sec);
		drift += (stop_time.tv_usec - start_time.tv_usec)/(float)MICRO_PER_SECOND;

		
		printf("Filho #%d -- desvio total: %.3f -- desvio medio: %.10f\n",
			count, drift - NO_OF_ITERATIONS*sleep/MICRO_PER_SECOND,
			(drift - NO_OF_ITERATIONS*sleep/MICRO_PER_SECOND)/NO_OF_ITERATIONS);
}


