#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <fcntl.h>
#define MAX_WORD 256
#define MAX_COUNT 255

// Conta quante volte compare una parola (max 255)
int count_occurrences(const char *filename, const char *target) {
    int fd = open(filename,O_RDONLY);
    if (fd<0) {
        perror("Errore apertura file");
        exit(1);
    }
    char c;
    int count = 0;
    int r= read(fd,&c,1);
    while (r > 0){
            int i=0;
	    while(c==target[i]){
                int r= read(fd,&c, 1);
                if(r<0) {
                    close(fd);
                    return count;
                }
                i++;
                if(i>=strlen(target)){
                    count++;
                    break;
                }
            }
	    if(c != target[0]){
	    	r=read(fd,&c,1);
	    }
        }
    close(fd);
    return count;
}


int main(int argc, char *argv[]) {
    
	
    int occ;	
    if (argc != 4) {
        fprintf(stderr, "Uso: %s parola file1 file2\n", argv[0]);
        exit(1);
    }

    char *parola = argv[1];
    char *file1 = argv[2];
    char *file2 = argv[3];

    pid_t pid = fork();

    if (pid < 0) {
        perror("Errore fork");
        exit(1);
    }

    if (pid == 0) {
        occ = count_occurrences(file1, parola);
        printf("Figlio: %d parol in %s\n", occ, file1);

        exit(occ);
    } else {
        occ = count_occurrences(file2, parola);

        printf("Padre: %d parole in %s\n", occ, file2);

        int status;
        wait(&status);
	int occ_figlio = WEXITSTATUS(status);
	printf("Padre: figlio = %d\n", occ_figlio);
	int somma = occ + occ_figlio;
        printf("Somma totale: %d\n", somma);
        
    }
    return 0;
}

