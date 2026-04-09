#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 10
#define FLUSH_STDIN while(getchar() != '\n')

unsigned char vettore[SIZE]={0};
char array_types[SIZE]={0};
int start = 0;
int stop = 0;
int count = 0;

int spazio_libero() {
  return SIZE - count;
}
int get_size(char tipo) {
    if (tipo == 'c') return sizeof(char);
    if (tipo == 'i') return sizeof(int);
    if (tipo == 'l') return sizeof(long);
    if (tipo == 'd') return sizeof(double);
    return 1;
}
void rimuovi() {
    char tipo = array_types[start];
    int size = get_size(tipo);
    array_types[start] = 0;
    start = (start + size) % SIZE;
    count -= size;
}
void inserisci(void *buffer, int size, char type) {

    while (size > spazio_libero()) {
        rimuovi();
    }

    for (int i = 0; i < size; i++) {
        vettore[(stop + i) % SIZE] = ((unsigned char*)buffer)[i];
    }

    array_types[stop] = type;

    stop = (stop + size) % SIZE;
    count += size;
}

void stampa_Array() {
    int i = start;
    int letti = 0;

    while (letti < count) {
        char tipo = array_types[i];

        if (tipo == 0) {
            i = (i + 1) % SIZE;
            letti++;
            continue;
        }

        if (tipo == 'c') {
            printf("%c ", vettore[i]);
            i = (i + 1) % SIZE;
            letti += 1;
        } else {
            int size = get_size(tipo);
            unsigned char tmp[sizeof(double)];

            for (int j = 0; j < size; j++) {
                tmp[j] = vettore[(i + j) % SIZE];
            }

            if (tipo == 'i') printf("%d ", *(int*)tmp);
            else if (tipo == 'l') printf("%ld ", *(long*)tmp);
            else if (tipo == 'd') printf("%lf ", *(double*)tmp);

            i = (i + size) % SIZE;
            letti += size;
        }
    }

    printf("\n");
}
int main(){
	
	long curr=0;
	long size=0;
	unsigned long buf;
	char scelta;
	int r;
	int i;
	while(1){
			size = 0;
			stampa_Array(vettore,array_types,start,stop);			
			printf("inserire tipo di dato (c = char, i = int, l = long, d = double) oppure stampa array attuale con (s): \n");
			scanf("%c",&scelta);
			FLUSH_STDIN;
			switch(scelta){
				case 'c':
						printf("inserisci carattere: \n");
						r=scanf("%c",(char*)&buf);
						if( r == 1) size = sizeof(char);
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
				case 's':
						stampa_Array(vettore,array_types,start,stop);
						continue;		
				default:

					printf("Dato non valido\n");
					continue;
			}
			FLUSH_STDIN;
			if (size > 0) {
            			inserisci(&buf, size, scelta);
        		}
	}
	return 0;
}

