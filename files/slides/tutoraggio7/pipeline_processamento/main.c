#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
    char nome[50];
    char cognome[50];
    int eta;
    float altezza;
} Persona;

void parse_line(char *line, Persona *p) {
    char *token;

    token = strtok(line, ",");
    strcpy(p->nome, token);

    token = strtok(NULL, ",");
    strcpy(p->cognome, token);

    token = strtok(NULL, ",");
    p->eta = atoi(token);

    token = strtok(NULL, ",");
    p->altezza = atof(token);
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Uso: %s file.csv\n", argv[0]);
        exit(1);
    }

    int pipe_p0_p1[2];
    int pipe_p0_p2[2];

    pipe(pipe_p0_p1);
    pipe(pipe_p0_p2);

    pid_t p1 = fork();

    if (p1 == 0) {

        // ===== P1 =====

        close(pipe_p0_p1[1]);

        close(pipe_p0_p2[0]);
        close(pipe_p0_p2[1]);


        Persona p;

        int somma_eta = 0;
        int count = 0;

        while (1) {

            read(pipe_p0_p1[0], &p, sizeof(Persona));

            if (p.eta == -1)
                break;

            somma_eta += p.eta;
            count++;
        }

        if (count > 0)
            printf("P1 -> Eta media: %.2f\n",
                   (float)somma_eta / count);

        close(pipe_p0_p1[0]);

        exit(0);
    }

    pid_t p2 = fork();

    if (p2 == 0) {

    
	    
    	int pipe_p2_p3[2];
	pipe(pipe_p2_p3);
        pid_t p3 = fork();

        if (p3 == 0) {

            // ===== P3 =====

            close(pipe_p2_p3[1]);

            close(pipe_p0_p1[0]);
            close(pipe_p0_p1[1]);

            close(pipe_p0_p2[0]);
            close(pipe_p0_p2[1]);

            Persona p;

            float somma_u20 = 0;
            float somma_mid = 0;
            float somma_over = 0;

            int c_u20 = 0;
            int c_mid = 0;
            int c_over = 0;

            while (1) {

                read(pipe_p2_p3[0], &p, sizeof(Persona));

                if (p.eta == -1)
                    break;

                if (p.eta < 20) {
                    somma_u20 += p.altezza;
                    c_u20++;
                }
                else if (p.eta <= 40) {
                    somma_mid += p.altezza;
                    c_mid++;
                }
                else {
                    somma_over += p.altezza;
                    c_over++;
                }
            }

            printf("\nP3 -> Medie altezza per fascia:\n");

            if (c_u20)
                printf("Under 20: %.2f\n",
                       somma_u20 / c_u20);

            if (c_mid)
                printf("20-40: %.2f\n",
                       somma_mid / c_mid);

            if (c_over)
                printf("Over 40: %.2f\n",
                       somma_over / c_over);

            close(pipe_p2_p3[0]);

            exit(0);
        }

        // ===== P2 =====

        close(pipe_p0_p2[1]);

        close(pipe_p2_p3[0]);

        close(pipe_p0_p1[0]);
        close(pipe_p0_p1[1]);

        Persona p;

        float somma_alt = 0;
        int count = 0;

        while (1) {

            read(pipe_p0_p2[0], &p, sizeof(Persona));

            if (p.eta == -1) {

                write(pipe_p2_p3[1], &p, sizeof(Persona));
                break;
            }

            somma_alt += p.altezza;
            count++;

            write(pipe_p2_p3[1], &p, sizeof(Persona));
        }

        if (count > 0)
            printf("P2 -> Altezza media globale: %.2f\n",
                   somma_alt / count);

        close(pipe_p0_p2[0]);
        close(pipe_p2_p3[1]);

        wait(NULL);

        exit(0);
    }

    // ===== P0 =====

    close(pipe_p0_p1[0]);
    close(pipe_p0_p2[0]);


    FILE *fp = fopen(argv[1], "r");

    if (!fp) {
        perror("Errore apertura file");
        exit(1);
    }

    char line[256];

    Persona p;

    while (fgets(line, sizeof(line), fp)) {

        line[strcspn(line, "\n")] = 0;

        parse_line(line, &p);

        write(pipe_p0_p1[1], &p, sizeof(Persona));

        write(pipe_p0_p2[1], &p, sizeof(Persona));
    }

    Persona end_msg;
    end_msg.eta = -1;

    write(pipe_p0_p1[1], &end_msg, sizeof(Persona));
    write(pipe_p0_p2[1], &end_msg, sizeof(Persona));

    close(pipe_p0_p1[1]);
    close(pipe_p0_p2[1]);

    fclose(fp);

    wait(NULL);
    wait(NULL);

    printf("\nP0 -> Tutti i processi terminati\n");

    return 0;
}
