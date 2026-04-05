#include <stdio.h>
#include "pcode.h"

// Le tableau P-code et le compteur ordinal
Case_Pcode Pcode[MAX_PCODE];
int        CO = 1; // commence à 1 comme dans le cours

// emit — ajoute une instruction dans le P-code
void emit(Instruction inst, int arg) {
    //on verrifie qu'on depasse pas taille max instruction
    if (CO >= MAX_PCODE) {
        fprintf(stderr, "Erreur : P-code plein\n");
        return;
    }
    //on remplit case courante avec l'instrcution et son argument
    Pcode[CO].inst = inst;
    Pcode[CO].arg  = arg;
    CO++; //on pointe vers la prochaine case libre
}

// get_CO — retourne l'adresse courante
int get_CO(void) {
    return CO;
}

// patch — modifie l'argument d'une instruction déjà émise
void patch(int addr, int val) {
    Pcode[addr].arg = val;
}

// afficher_pcode — affiche toutes les instructions
void afficher_pcode(void) {
    // Noms des instructions pour l'affichage
    const char *noms[] = {
        "LDA", "LDV", "LDC", "AFF", "RD", "WRTLN",
        "JMP", "JIF", "ADD", "MIN", "MULT", "DIV",
        "NEG", "INC", "DEC", "AND", "OR", "NOT",
        "SUP", "SUPE", "INF", "INFE", "EG", "DIFF", "STOP"
    };

    for (int i = 1; i < CO; i++)
        printf("%3d  %-6s  %d\n", i, noms[Pcode[i].inst], Pcode[i].arg);
}