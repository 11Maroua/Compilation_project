#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "scan_go.h"
#include "analyse_go.h"
#include "scan_gpl.h"
#include "analyse_gpl.h"
#include "pcode.h"
#include "exec.h"

int main(int argc, char *argv[]) {

    // Vérifier les arguments
    if (argc != 3) {
        fprintf(stderr, "Usage : ./compilo <grammaire.gpl> <programme.gpl>\n");
        exit(1);
    }

    // Etape 1 : construire l'arbre de G0 (les 5 règles)
    GenForetGO();
    printf("GO construite\n");

    // Etape 2 : afficher G0 pour debug
    printf("\n--- Arbre G0 ---\n");
    for (int i = 1; i <= 5; i++) {
        printf("\nRegle %d :\n", i);
        ImprimArbre(A[i], 0);
    }

    // Etape 3 : ouvrir et analyser la grammaire GPL
    printf("\n--- Analyse de la grammaire GPL ---\n");
    ScanGO_init(argv[1]);
    ScanGO();
    if (AnalyseGO(A[IDX_S])) {
        printf("Grammaire GPL valide\n");
    } else {
        fprintf(stderr, "Erreur : grammaire GPL invalide\n");
        ScanGO_close();
        exit(1);
    }
    ScanGO_close();

    // Etape 4 : afficher l'arbre GPL construit
    printf("\n--- Arbre GPL ---\n");
    for (int i = 6; i < 6 + 10; i++) {
        //printf("A[%d] = %p\n", i, (void*)A[i]); // debug
        if (A[i] == NULL) break;
        printf("\nRegle GPL %d :\n", i);
        ImprimArbre(A[i], 0);
    }

    // Etape 5 : ouvrir et compiler le programme GPL
    printf("\n--- Compilation du programme ---\n");
    ScanGPL_init(argv[2]);
    ScanGPL();
    if (AnalyseGPL(A[6])) {
        printf("Programme valide\n");
    } else {
        fprintf(stderr, "Erreur : programme GPL invalide\n");
        ScanGPL_close();
        exit(1);
    }
    ScanGPL_close();

    // Etape 6 : afficher le P-code généré
    afficher_pcode();

    // Etape 7 : executer le P-code
    printf("\n--- Execution ---\n");
    Exec();

    return 0;
}