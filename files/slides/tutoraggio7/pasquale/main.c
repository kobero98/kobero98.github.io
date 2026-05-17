#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#define SIZE 256

int main() {

    int Tx[2];
    int Rx[2];

    pipe(Tx);
    pipe(Rx);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(1);
    }

   if (pid == 0) {

        close(Tx[1]); // B legge da Tx
        close(Rx[0]); // B scrive su Rx

        char buffer[SIZE];

        while (1) {

            int n = read(Tx[0], buffer, SIZE);

            if (n <= 0)
                break;

            buffer[strcspn(buffer, "\n")] = '\0';

            if (strcmp(buffer, "exit") == 0)
                break;

            for (int i = 0; buffer[i] != '\0'; i++) {
                buffer[i] = toupper(buffer[i]);
            }

            write(Rx[1], buffer, strlen(buffer) + 1);
        }

        close(Tx[0]);
        close(Rx[1]);

        exit(0);
    }

    close(Tx[0]); // A scrive su Tx
    close(Rx[1]); // A legge da Rx

    char buffer[SIZE];

    while (1) {

        printf("Inserisci stringa: ");
        fgets(buffer, SIZE, stdin);

        write(Tx[1], buffer, strlen(buffer) + 1);

        // uscita
        if (strncmp(buffer, "exit", 4) == 0)
            break;

        read(Rx[0], buffer, SIZE);

        printf("Risposta da B: %s\n", buffer);
    }

    close(Tx[1]);
    close(Rx[0]);

    wait(NULL);

    return 0;
}
