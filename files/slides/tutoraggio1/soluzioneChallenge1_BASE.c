#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define FLUSH_STDIN while(getchar() != '\n')

int main(){

	unsigned char vettore[SIZE];
	long curr,size=0;
	unsigned long buf;
	char scelta;
	int r;
	while(1){
			size = 0;
			printf("inserire tipo di dato (c = char, i = int, l = long, d = double): \n");
			scanf("%c",&scelta);
			FLUSH_STDIN;
			switch(scelta){
				case 'c':
						printf("inserisci carattere: \n");
						r=scanf("%c",(char*)&buf);
						if( r == 1)	size = sizeof(char);
						break;
				case 'i':
						printf("inserisci intero: \n");
						r=scanf("%d",(int*)&buf);
						if( r == 1) size = sizeof(int);
						break;
				case 'l':
						printf("inserisci long: \n");
						r=scanf("%ld",(long*)&buf);
						if( r == 1)size = sizeof(long);
						break;
				case 'd':
						printf("inserisci double: \n");
						r=scanf("%lf",(double*)&buf);
						if( r == 1) size = sizeof(double);
						break;
				default:

					printf("Dato non valido\n");
					continue;
			}
			FLUSH_STDIN;
			if(curr + size <=SIZE){
				memcpy(&vettore[curr],&buf,size);
				curr += size;
			}else{
				break;
			}
			printf("Dato inserito\n");
		if(curr == SIZE) break;
	}
	for(int i=0;i<curr;i++){
		printf("%x ",vettore[i]);
	}
	printf("\n");
	return 0;
}

