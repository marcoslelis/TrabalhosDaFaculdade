//Adriano de Oliveira Munin      RA:17066960
//Fábio Seiji Irokawa            RA:17057720
//Lucas Rodrigues Coutinho       RA:17776501
//Marcos Lelis de F. Oliveira    RA:16248387
//Paulo M. Birocchi              RA:16148363

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	

int main()
{
    int i;
    int qtdcarga;
    int rtn = 1;
    printf("Digite a quantidade de cargas\n");
    scanf("%i", &qtdcarga);
    
	for( int count = 0; count < qtdcarga; count++) {
		if( 0 != rtn ) {
			rtn = fork();
		} else {
			break;
		}
	}

    for(i = 0; i < 10; i++)
    {
        i = i + 1;
        i = i - 1;
        i = i * 1;
        i = 1 / 1;
    }  

return 0;
}
