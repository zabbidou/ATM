#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

// functia hash pentru a determina pozitia cardului
int FunctieHash (size_t M, char * card_number) {
    int s = 0, i;
    for (i = 0; i < strlen(card_number); i++) {
        if (isdigit(card_number[i])) {
            s = s + (card_number[i] - '0');
        }
    }
    return s % M;
}

// functia de baza pt alocare si initializare
TCard AlocareCard(char card_number[CNR_L], char pin[PIN_L], 
				 char expiry_date[EXP_L], char cvv[CVV_L]) {
	TCard card = (TCard)calloc(1, sizeof(UnCard));
	// daca nu s-a alocat, returnam null
	if (!card) {
		return NULL;
	}

	// initializam campurile cardului
	strcpy(card->card_number, card_number);
	strcpy(card->pin, pin);
	strcpy(card->expiry_date, expiry_date);
	strcpy(card->cvv, cvv);
	card->status = 0;
    card->balance = 0;
	card->hist = NULL;
	card->urm = NULL;
    card->attempts = 0;
	return card;
}

// destruction of the enemy
void distruge(ALista AL) {
	// parcurgem lista LC
    while ((*AL) != NULL) {
        TLista p = (*AL);
        TCard card = p->info;
        // parcurgem fiecare lista de carduri
        while (card != NULL) {
            TCard aux = card;
            THistory hist = card->hist;
            // parcurgem fiecare istoric al fiecarui card
            while (hist != NULL) {
                THistory haux = hist;
                hist = hist->urm;
                free(haux);
            }
            card = card->urm;
            free(aux);
        }
        (*AL) = (*AL)->urm;
        free(p);
    }
    free(*AL);
}

TCard find_card(TLista LC, char nr[CNR_L]) {
	// calculam pozitia
    int poz = LC->fh(LC->M, nr);
    int i = 0;
    TCard card;

    // daca lista e vida, nu avem ce face
    if (LC == NULL) {
        return NULL;
    }
    // parcurgem pana ajungem la pozitia dorita sau ultima pozitie nevida
    for (i = 0; LC != NULL && i < poz; i++) {
        LC = LC->urm;
    }
    // daca am ajuns la finalul listei atunci iarasi nu mai avem ce face
    if (LC == NULL) {
        return NULL;
    }
    
    card = LC->info;
    // cautam in lista de carduri
    while (card != NULL) {
        if (strncmp(card->card_number, nr, strlen(card->card_number)) == 0) {
            return card;
        }
        card = card->urm;
    }
    return NULL;
}

// functia de alocare + adaugare istoric
void add_history(TLista L, TCard card, char hist[ARR_L]) {
    THistory history = (THistory)calloc(1, sizeof(THist));
    // test de alocare
    if (!history) {
        return;
    }
    // copiem string-ul formatat in main
    strcpy(history->info, hist);
    // si facem legaturile
    history->urm = card->hist;  
    card->hist = history;
    return;
}

int add_card(ALista LC, TCard card) {
    int poz, i = 0;
    TLista p;
    // daca nu am primit un card valid, intoarcem cod de eroare
	if (card == NULL) {
		return 0;
	}
	// daca lista e nevida, cautam cardul
    if ((*LC) != NULL) {
    	// daca l-am gasit, intoarcem cod de eroare (cardul exista deja)
        if (find_card(*LC, card->card_number) != NULL) {
            return -1;
        }
    }
    // calculam pozitia de inserare
	poz = (*LC)->fh((*LC)->M, card->card_number);
	p = *LC;
    // parcurgem toata lista
    while (p->urm != NULL) {
    	// daca ajungem la pozitia dorita, continuam
        if (i == poz) break;
		p = p->urm;
        i++;
    }
    // daca totusi nu am ajuns la pozitie (lista e prea mica)
    if (i < poz) {
    	// alocam restul de celule ramase
        for (; i < poz; i++) {
            p->urm = malloc(sizeof(TCelula));
            p->urm->M = (*LC)->M;
            p->urm->info = NULL;
            p->urm->fh = (*LC)->fh;
            p->urm->urm = NULL;
            p = p->urm;
        }
    }
    // facem legaturile in liste
	card->urm = p->info;
	p->info = card;
    (*LC)->lista_goala = 0;
	return 1; // succes
}
// destruction of the smaller enemy
void delete_card(ALista AL, char nr[CNR_L]) {
    int i, poz = (*AL)->fh((*AL)->M, nr);
    TLista p = (*AL);
    TCard card, ant;
    THistory haux;
    // parcurgem iarasi lista pana ajungem la poz sau la final
    for (i = 0; i < poz && p!= NULL; i++) {
        p = p->urm;
    }
    // daca am ajuns la final, return
    if (p == NULL) {
        return;
    }
    // nu am folosit aici functia de find_card pentru ca avem nevoie de ant
    card = p->info;
    ant = NULL;
    // cautam cardul
    while (card != NULL) {
        if (strncmp(card->card_number, nr, strlen(card->card_number)) == 0) {
            break;
        }
        // actualizam anteriorul
        ant = card;
        card = card->urm;
    }
    // daca nu gasim cardul, return
    if (card == NULL) {
        return;
    }
    // eliminare element de inceput de lista
    if (ant == NULL) {
        p->info = card->urm;
    } else {
    	// eliminare la mijloc
        ant->urm = card->urm;
    }
    // eliberam memoria pentru history
    while (card->hist != NULL) {
        haux = card->hist;
        card->hist = card->hist->urm;
        free(haux);
    }
    // eliberam cardul
    free(card->hist);
    free(card);
    return;
}
// functie pentru afisarea unui singur card
void show_card(TCard card, FILE* out) {
    THistory haux;
    fprintf(out, "(card number: %s, PIN: %s, expiry date: %s, ",
            card->card_number, card->pin, card->expiry_date);
    fprintf(out, "CVV: %s, balance: %d", card->cvv, card->balance);
    // "decodificam" statusul
    if (card->status == 0) {
        fprintf(out, ", status: NEW, ");
    }
    if (card->status == 1) {
        fprintf(out, ", status: ACTIVE, ");
    }
    if (card->status == 2) {
        fprintf(out, ", status: LOCKED, ");
    }
    // afisam istoricul cu formatarile dubioase aferente
    fprintf(out, "history: [");

    if (card->hist == NULL) {
        fprintf(out, "])\n");
    } else {
        haux = card->hist;
        while (haux->urm != NULL) {
            fprintf(out, "(%s), ", haux->info);
            haux = haux->urm;
        }
        fprintf(out, "(%s)])\n", haux->info);
    }
    return;
}
// functia de afisare generala
void show(TLista LC, char card_number[CNR_L], FILE* out) {
    TLista p = LC;
    TCard card;
    int poz = 0, i = 0;
    // daca lista este goala, nu facem nimic
    // conditia asta este pentru a trata mai usor cazul in care am avut
    // carduri si le-am sters (desi mergea si fara)
    if (LC->lista_goala == 1) {
        return;
    }
    // daca nu am primit ca parametru niciun card specific de afisat,
    // afisam tot
    if (card_number == NULL) {
    	// parcurgem LC
    	while (p != NULL) {
    		card = p->info;
    		// daca nu avem card aici
    	    if (card == NULL) {
    	        fprintf(out, "pos%d: []\n", i);
    	    } else {
    	    	// daca exista totusi carduri
                fprintf(out, "pos%d: [\n", i);
                // afisam pe fiecare in parte cu functia de mai sus
                while (card->urm != NULL) {
                    show_card(card, out);
                    card = card->urm;
                }

                show_card(card, out);
        	    fprintf(out, "]\n");
        	}
        	// actualizam pozitia
            i++;
        	p = p->urm;
    	}
	} else {
		// daca am primit totusi un card specific de afisat, calculam pozitia
		poz = p->fh(p->M, card_number);
		// ne ducem direct pe pozitia care ne trebuie.
		// pentru ca nu avem vectori, folosim while
		for (i = 0; i < poz; i++) {
			p = p->urm;
		}
        card = p->info;
        // parcurgem toata lista de carduri
	   	while (card != NULL) {
	   		// cautam acel card special dat ca parametru
			if (strncmp(card->card_number, card_number,
				strlen(card->card_number)) == 0) {
				// si il afisam
                show_card(card, out);
			}
			card = card->urm;
		}
	}
    return;
}
// functie ajutatoare pentru resize
// duplica history-ul de la un card la altul
// adica un add_history dar cu inserare la final
void duplicate_history(TLista L, TCard card, char hist[ARR_L]) {
    THistory history = (THistory)calloc(1, sizeof(THist));
    THistory haux;
    /// verificam alocarea
    if (!history) {
        return;
    }
    // bagam informatia
    strcpy(history->info, hist);
    haux = card->hist;
    // daca nu avem nimic in history, inserare la inceput
    if (haux == NULL) {
        card->hist = history;
        history->urm = NULL;
        return;
    }
    // daca avem totusi, parcurgem pana la ultimul element
    while (haux->urm != NULL) {
        haux = haux->urm;
    }
    // si inseram dupa el
    haux->urm = history;
    history->urm = NULL;
    return;
}
// o functie a naibii de usoara conceptual, dar...
// face redimensionarea bazei de date
TLista resize(ALista LC, FILE* out) {
    TCard card, aux;
    // "duplicam" capul de lista cu informatiile aferente
    TLista LC_new = (TLista)malloc(sizeof(TCelula));
    LC_new->M = (*LC)->M * 2;
    LC_new->fh = (*LC)->fh;
    LC_new->info = NULL;
    LC_new->urm = NULL;

    TLista p = (*LC);
    THistory hist;
    char history[ARR_L];

    while (p != NULL) {
        card = p->info;
        // parcurgem lista veche
        while (card != NULL) {
        	// duplicam cardul de la 0
            aux = AlocareCard(card->card_number, card->pin, 
                              card->expiry_date, card->cvv);
            aux->status = card->status;
            aux->balance = card->balance;
            aux->attempts = card->attempts;
            hist = card->hist;
            card = card->urm;
            aux->urm = NULL;
            // si il adaugam
            add_card(&LC_new, aux);
            // nu mutam direct cardurile alocate deja pentru ca lui valgrind
            // nu i-a convenit

            // de asemenea duplicam si istoricul
            while (hist != NULL) {
                strcpy(history, hist->info);
                duplicate_history(LC_new, aux, history);
                hist = hist->urm;                
            }
        }
        p = p->urm;
    }
    // si sa nu uitam de eliberarea memoriei
    distruge(LC);
    return LC_new;
}

int insert_card(TLista L, char nr[CNR_L], char pin[PIN_L], FILE* out) {
    TCard card = find_card(L, nr);

    if (card == NULL) {
        return -2; // nu exista cardul
    }

    if (card->status == 2) { // verificam daca avem card blocat
        fprintf(out, "The card is blocked. "); 
        fprintf(out, "Please contact the administrator.\n");
        return -1;
    }
    // daca e pinul bun
    if (strcmp(card->pin, pin) == 0) {
        // verificam daca are status NEW
        if (card->status == 0) {
            fprintf(out, "You must change your PIN.\n");
        }
        // resetam incercarile de introducere a pinului
        card->attempts = 0;
        return card->status;
    }
    // daca nu e pinul bun, mesaj de eroare
    fprintf(out, "Invalid PIN\n");
    card->attempts++;
    // verificam daca trebuie sa devina locked
    if (card->attempts == 3) {
        card->status = 2;
        fprintf(out, "The card is blocked. "); 
        fprintf(out, "Please contact the administrator.\n");
    }
    return -1;
}

void unblock_card(TLista L, char card_number[CNR_L]) {
    TCard card = find_card(L, card_number);
    // daca nu se gaseste cardul, nu facem nimic
    if (card == NULL) {
        return;
    }
    // daca se gaseste, actualizam tot ce e necesar
    card->status = 1;
    card->attempts = 0;
    return;
}

int pin_change(TLista L, char card_number[CNR_L], char new_pin[PIN_L]) {
    char* check;
    TCard card;
    // transformam pinul in numar pentru a verifica daca nu contine cifre
    strtol(new_pin, &check, 10);
    // verificam daca nu contine cifre
    if (strlen(check) != 0) {
        return -1;
    }
    // verificam daca e de lungime corecta
    if (strlen(new_pin) != 4) {
        return -1;
    }
    // actualizam pinul
    card = find_card(L, card_number);
    strcpy(card->pin, new_pin);
    card->status = 1;
    return 0;
}

int balance_inquiry(TLista L, char nr[CNR_L]) {
    TCard card = find_card(L, nr);
    return card->balance;
}

int recharge(TLista L, char nr[CNR_L], char amount[SUM], FILE* out) {
    int aux = atoi(amount);
    TCard card;

    if (aux % 10 != 0) {
        return -1; // suma trebuie sa fie multiplu de 10
    }
    // actualizam balance-ul
    card = find_card(L, nr);
    card->balance = card->balance + aux;
    fprintf(out, "%d\n", card->balance);
    return 0;
}

int cash_withdrawal(TLista L, char nr[CNR_L], char amount[SUM], FILE* out) {
    int aux = atoi(amount); // transformam in numar
    TCard card;

    if (aux % 10 != 0) {
        return -1; // trebuie sa fie multiplu de 10
    }

    card = find_card(L, nr);

    if (card->balance < aux) {
        return -2; // fonduri insuficiente
    }

    card->balance = card->balance - aux;
    fprintf(out, "%d\n", card->balance);
    return 0;
}
// transfer_funds
int tr_funds(TLista L, char nr1[CNR_L], char nr2[CNR_L], 
			 char sum[SUM], FILE* out) {
    int amount = atoi(sum);
    TCard card_1, card_2;

    if (amount % 10 != 0) {
        return -1; // suma trebuie sa fie multiplu de 1-
    }

    card_1 = find_card(L, nr1);
    card_2 = find_card(L, nr2);
    
    if (card_1->balance < amount) {
        return -2; // fonduri insuficiente
    }

    card_1->balance = card_1->balance - amount;
    card_2->balance = card_2->balance + amount;
    fprintf(out, "%d\n", card_1->balance);
    return 0;
}

int reverse_transaction(TLista L, char transaction[ARR_L]) {
    char* params;
    char aux[ARR_L];
    char nr1[CNR_L], nr2[CNR_L], sum[SUM];
    TCard card1, card2;
    int amount;
    THistory ant, haux, hist;
    // delimitam parametrii tranzactiei
    strcpy(aux, transaction);
    aux[strlen(aux) - 1] = 0;
    params = strtok(transaction, " ");
    strcpy(nr1, params);
    params = strtok(NULL, " ");
    strcpy(nr2, params);
    params = strtok(NULL, "\n");
    strcpy(sum, params);
    // initializam tot ce trebuie
    amount = atoi(sum);
    card2 = find_card(L, nr2);
    card1 = find_card(L, nr1);
    ant = NULL;
    hist = card2->hist;
    // cautam history-ul corespunzator
    while (hist != NULL) {
        if (strstr(hist->info, aux) != NULL) {
            haux = hist;
            break;
        }

        ant = hist;
        hist = hist->urm;
    }

    if (hist == NULL) {
        return -1; // nu exista tranzactia in history
    }

    if (card2->balance < amount) {
        return -2; // transaction cannot be reversed
    }

    card2->balance = card2->balance - amount;
    card1->balance = card1->balance + amount;
    // eliminam history-ul
    if (ant == NULL) {
        card2->hist = card2->hist->urm;
        haux->urm = NULL;
    } else {
        ant->urm = haux->urm;
        haux->urm = NULL;
    }
    // si facem curat dupa noi
    free(haux);
    return 0;
}

// Gherman Maria-Irina @ 314CB