#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define CNR_L 17 // card number length
#define PIN_L 5 // pin length
#define EXP_L 6 // expiry date length
#define CVV_L 4 // cvv length
#define ARR_L 80 // array length
#define SUM 10 // dimensiunea soldului

typedef int (*TFHash)(size_t M, char * card_number);

typedef struct history {
	char info[ARR_L];
	struct history * urm;
} THist, *THistory, **AHistory;

typedef struct LSC {
	char card_number[CNR_L];
	char pin[PIN_L];
	char expiry_date[EXP_L];
	char cvv[CVV_L];
	int balance;
	int status; //0->new 1->active 2->locked
    int attempts;
	struct history * hist;
	struct LSC * urm;
} UnCard, *TCard, **ALSC;

typedef struct LC {
	size_t M;
	struct LSC * info;
	struct LC * urm;
	TFHash fh; // functia hash
    int lista_goala;
} TCelula, *TLista, **ALista;

// Gherman Maria-Irina @ 314CB