#include <stdio.h>
#include "pcode.h"

// Le tableau P-code et le compteur ordinal
Case_Pcode Pcode[MAX_PCODE];
int        CO = 1; // commence à 1 comme dans le cours

// emit — ajoute une instruction dans le P-code
void emit(Instruction inst, int arg) {
    if (CO >= MAX_PCODE) {
        fprintf(stderr, "Erreur : P-code plein\n");
        return;
    }
    Pcode[CO].inst = inst;
    Pcode[CO].arg  = arg;
    CO++;
}

// get_CO — retourne l'adresse courante
int get_CO(void) {
    return CO;
}

// patch — modifie l'argument d'une instruction déjà émise
void patch(int addr, int val) {
    Pcode[addr].arg = val;
}

// afficher_pcode — affiche toutes les instructions (debug)
void afficher_pcode(void) {
    // Noms des instructions pour l'affichage
    const char *noms[] = {
        "LDA", "LDV", "LDC", "AFF", "RD", "WRTLN",
        "JMP", "JIF", "ADD", "MIN", "MULT", "DIV",
        "NEG", "INC", "DEC", "AND", "OR", "NOT",
        "SUP", "SUPE", "INF", "INFE", "EG", "DIFF", "STOP"
    };

    printf("\n--- P-code generé ---\n");
    for (int i = 1; i < CO; i++)
        printf("%3d  %-6s  %d\n", i, noms[Pcode[i].inst], Pcode[i].arg);
    printf("---------------------\n");
}