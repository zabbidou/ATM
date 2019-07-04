#include "liste.h"
#include "functii.c"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

int main() {
    char buffer[ARR_L];
    char* param; // cu param parsam comenzile
    char operatie[ARR_L];
    char aux[ARR_L];
    char card_number[CNR_L];
    char card_number_2[CNR_L];
    char pin[PIN_L];
    char cvv[CVV_L];
    char expiry_date[EXP_L];
    char history[ARR_L];
    char amount[SUM];
    int balance = 0;
    int err = 0;
    int num_cards = 0;

    FILE* in = fopen("input.in","r");
    FILE* out = fopen("output.out", "a");
    TLista L = (TLista)malloc(sizeof(TCelula));
    TCard card, card2;
    // initializam L
    fscanf(in, "%ld", &L->M);
    L->info = NULL;
    L->urm = NULL;
    L->fh = FunctieHash;
    L->lista_goala = 1;
    // cat timp putem citi o linie
    while (fgets(buffer, ARR_L, in) != NULL) {
        // copiem comanda pentru a o avea intreaga pt history
    	strcpy(operatie, buffer);
        // delimitam numele comenzii
        param = strtok(buffer, " ");

        if (strncmp(param, "show", 4) == 0) {
            // citim numarul cardului (daca exista)
            param = strtok(NULL, "\n");

            show(L, param, out);
            // daca nu am avut numar de card, continuam
            if (param == NULL) {
                continue;
            }
        }

        if (strcmp(param, "add_card") == 0) {
            param = strtok(NULL, " ");
            strcpy(card_number, param);
            
            param = strtok(NULL, " ");
            strcpy(pin, param);

            param = strtok(NULL, " ");
            strcpy(expiry_date, param);

            param = strtok(NULL, "\n");
            strcpy(cvv, param);

            card = AlocareCard(card_number, pin, expiry_date, cvv);

            if (find_card(L, card_number) != NULL) {
                fprintf(out, "The card already exists\n");
                free(card);
            } else {
                // numaram cardurile
                num_cards++;
                // verificam conditia de resize
                if (num_cards > L->M) {
                    L = resize(&L, out);
                }

                add_card(&L, card);
            }
        }

        if (strcmp(param, "delete_card") == 0) {
            // actualizam nr de carduri
            num_cards--;
            param = strtok(NULL, " ");
            strcpy(card_number, param);
            delete_card(&L, card_number);
        }

        if (strcmp(param, "insert_card") == 0) {
        	param = strtok(NULL, " ");
        	strcpy(card_number, param);

        	param = strtok(NULL, "\n");
        	strcpy(pin, param);

        	err = insert_card(L, card_number, pin, out);

        	if (err == -2) {
        		continue; // nu exista cardul
        	}

        	if ((err == 0) || (err == 1)) {
                strcpy(history, "SUCCESS, "); // operatia a reusit
        	}

        	if (err == -1) {
                strcpy(history, "FAIL, "); // cardul e blocat    		
        	}

            strncat(history, operatie, strlen(operatie) - 1);
            card = find_card(L, card_number);
            add_history(L, card, history);
        }

        if (strcmp(param, "cancel") == 0) {
            param = strtok(NULL, "\n");
            // operatia are succes mereu
            strcpy(history, "SUCCESS, ");
            strncat(history, operatie, strlen(operatie) - 1);

            card = find_card(L, card_number);
            add_history(L, card, history);
        }

        if (strcmp(param, "unblock_card") == 0) {
            param = strtok(NULL, "\n");
            strcpy(card_number, param);

            unblock_card(L, card_number);
        }

        if (strcmp(param, "pin_change") == 0) {
            param = strtok(NULL, " ");
            strcpy(card_number, param);

            param = strtok(NULL, "\n");
            // daca pinul nu are lungimea corecta
            if (strlen(param) != 4) {
                fprintf(out, "Invalid PIN\n");
                strcpy(history, "FAIL, ");
                strncat(history, operatie, strlen(operatie) - 1);
                card = find_card(L, card_number);
                add_history(L, card, history);
            } else {
                // daca da, continuam
                strcpy(pin, param);
                // verificam daca pinul are litere
                if (pin_change(L, card_number, pin) == -1) {
                    strcpy(history, "FAIL, ");
                    fprintf(out, "Invalid PIN\n");
                } else {
                    strcpy(history, "SUCCESS, ");
                }
                strncat(history, operatie, strlen(operatie) - 1);
                card = find_card(L, card_number);
                add_history(L, card, history);
            }
        }

        if (!strcmp(param, "balance_inquiry")) {
            param = strtok(NULL, "\n");
            strcpy(history, "SUCCESS, ");

            strncat(history, operatie, strlen(operatie) - 1);
            card = find_card(L, card_number);
            add_history(L, card, history);

            balance = balance_inquiry(L, card_number);
            fprintf(out, "%d\n", balance);
        }

        if (!strcmp(param, "recharge")) {
            param = strtok(NULL, " ");
            strcpy(card_number, param);

            param = strtok(NULL, "\n");
            strcpy(amount, param);

            err = recharge(L, card_number, amount, out);
            // verificam codul de eroare
            if (err == -1) {
                fprintf(out, "The added amount must be multiple of 10\n");
                strcpy(history, "FAIL, ");
            } else {
                strcpy(history, "SUCCESS, ");
            }

            strncat(history, operatie, strlen(operatie) - 1);
            card = find_card(L, card_number);
            add_history(L, card, history);
        }

        if (!strcmp(param, "cash_withdrawal")) {
            param = strtok(NULL, " ");
            strcpy(card_number, param);

            param = strtok(NULL, "\n");
            strcpy(amount, param);

            err = cash_withdrawal(L, card_number, amount, out);
            // verificam codurile de eroare
            if (err == -1) {
                fprintf(out, "The requested amount must be multiple of 10\n");
                strcpy(history, "FAIL, ");
            } else if (err == -2) {
                fprintf(out, "Insufficient funds\n");
                strcpy(history, "FAIL, ");
            } else {
                strcpy(history, "SUCCESS, ");
            }

            strncat(history, operatie, strlen(operatie) - 1);
            card = find_card(L, card_number);
            add_history(L, card, history);
        }

        if (!strcmp(param, "transfer_funds")) {
            param = strtok(NULL, " ");
            strcpy(card_number, param);

            param = strtok(NULL, " ");
            strcpy(card_number_2, param);

            param = strtok(NULL, "\n");
            strcpy(amount, param);

            err = tr_funds(L, card_number, card_number_2, amount, out);
            // verificam codurile de eroare
            if (err == -1) {
                fprintf(out,
                        "The transferred amount must be multiple of 10\n");
                strcpy(history, "FAIL, ");
                strncat(history, operatie, strlen(operatie) - 1);
            } else if (err == -2) {
                fprintf(out, "Insufficient funds\n");
                strcpy(history, "FAIL, ");
                strncat(history, operatie, strlen(operatie) - 1);
            } else {
                strcpy(history, "SUCCESS, ");
                strncat(history, operatie, strlen(operatie) - 1);
                // actualizam istoricul in cele 2 cazuri
                card2 = find_card(L, card_number_2);
                add_history(L, card2, history);
            }
            card = find_card(L, card_number);
            add_history(L, card, history);
        }

        if (!strcmp(param, "reverse_transaction")) {
            param = strtok(NULL, " ");
            strcpy(card_number, param);

            strcpy(aux, operatie + strlen("reverse_transaction ")); // 20 -> strlen("reverse transaction")
            // verificam codurile de eroare
            err = reverse_transaction(L, aux);
            if (err == -1) {
                printf("Transaction could not be found\n");
            } else if (err == -2) {
                fprintf(out, "The transaction cannot be reversed\n");
                strcpy(history, "FAIL, ");
            } else {
                strcpy(history, "SUCCESS, ");
                strncat(history, operatie, strlen(operatie) - 1);

                card = find_card(L, card_number);
                add_history(L, card, history);
            }
        }
    }
    // sa nu uitam sa curatam dupa noi
    fclose(out);
    fclose(in);
    distruge(&L);
    return 0;
}

// Gherman Maria-Irina @ 314CB