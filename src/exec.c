#include <stdio.h>
#include <stdlib.h>
#include "pcode.h"
#include "exec.h"

// La pile d'execution et son sommet
static int Pilex[10000];
static int Spx = 0; // Spx pointe sur le sommet de la pile

// Interprete une instruction du P-code
static void Interpret(Case_Pcode x) {
    switch (x.inst) {

        // Charger une adresse dans la pile
        case LDA:
            Spx++;
            Pilex[Spx] = x.arg;
            CO++;
            break;

        // Charger une valeur depuis la pile d'execution
        case LDV:
            Spx++;
            Pilex[Spx] = Pilex[x.arg];
            CO++;
            break;

        // Charger une constante
        case LDC:
            Spx++;
            Pilex[Spx] = x.arg;
            CO += 2;
            break;

        // Affectation — dépiler valeur et adresse
        case AFF:
            // on empile adresse variable + valeur à affecter 
            Pilex[Pilex[Spx - 1]] = Pilex[Spx];
            //car on dépile les deux éléments ajoutés
            Spx -= 2;
            CO++;
            break;

        // Lire un entier depuis l'entrée
        case RD:
            scanf("%d", &Pilex[Spx]);
            CO++;
            break;

        // Afficher un entier
        case WRTLN:
            printf("%d\n", Pilex[Spx]);
            Spx--;
            CO++;
            break;

        // Saut inconditionnel
        case JMP:
            CO = x.arg;
            break;

        // Saut si faux
        case JIF:
            if (Pilex[Spx] == 0)
                CO = x.arg;
            else
                CO++;
            Spx--;
            break;

        // Addition
        case ADD:
            Pilex[Spx - 1] = Pilex[Spx - 1] + Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Soustraction
        case MIN:
            Pilex[Spx - 1] = Pilex[Spx - 1] - Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Multiplication
        case MULT:
            Pilex[Spx - 1] = Pilex[Spx - 1] * Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Division
        case DIV:
            if (Pilex[Spx] == 0) {
                fprintf(stderr, "Erreur : division par zero\n");
                exit(1);
            }
            Pilex[Spx - 1] = Pilex[Spx - 1] / Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Négation
        case NEG:
            Pilex[Spx] = -Pilex[Spx];
            CO++;
            break;

        // Incrémenter
        case INC:
            Pilex[Spx]++;
            CO++;
            break;

        // Décrémenter
        case DEC:
            Pilex[Spx]--;
            CO++;
            break;

        // Et logique
        case AND:
            Pilex[Spx - 1] = Pilex[Spx - 1] && Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Ou logique
        case OR:
            Pilex[Spx - 1] = Pilex[Spx - 1] || Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Non logique
        case NOT:
            Pilex[Spx] = !Pilex[Spx];
            CO++;
            break;

        // Supérieur >
        case SUP:
            Pilex[Spx - 1] = Pilex[Spx - 1] > Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Supérieur ou égal >=
        case SUPE:
            Pilex[Spx - 1] = Pilex[Spx - 1] >= Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Inférieur 
        case INF:
            Pilex[Spx - 1] = Pilex[Spx - 1] < Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Inférieur ou égal <=
        case INFE:
            Pilex[Spx - 1] = Pilex[Spx - 1] <= Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Égal =
        case EG:
            Pilex[Spx - 1] = Pilex[Spx - 1] == Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Différent <>
        case DIFF:
            Pilex[Spx - 1] = Pilex[Spx - 1] != Pilex[Spx];
            Spx--;
            CO++;
            break;

        // Arrêt
        case STOP:
            break;

        default:
            fprintf(stderr, "Erreur : instruction inconnue\n");
            exit(1);
    }
}

// Exec — boucle principale d'execution du P-code
void Exec(void) {
    CO = 1;
    if (Pcode[CO].inst == 0 && get_CO() == 1) {
        printf("Pas de P-code généré\n");
        return;
    }
    while (Pcode[CO].inst != STOP) {
        Interpret(Pcode[CO]);
    }
}
