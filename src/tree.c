#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "scan_go.h"

// Definition du tableau global
PTR A[MAX_RULES];

// FONCTIONS DE CONSTRUCTION DES NOEUDS

PTR GenConc(PTR p1, PTR p2) {
    PTR p = malloc(sizeof(struct Node));
    if (!p) { fprintf(stderr, "Erreur malloc GenConc\n"); exit(1); }
    p->classe     = Conc;
    p->conc.left  = p1;
    p->conc.right = p2;
    return p;
}

PTR GenUnion(PTR p1, PTR p2) {
    PTR p = malloc(sizeof(struct Node));
    if (!p) { fprintf(stderr, "Erreur malloc GenUnion\n"); exit(1); }
    p->classe    = Union;
    p->uni.left  = p1;
    p->uni.right = p2;
    return p;
}

PTR GenStar(PTR store) {
    PTR p = malloc(sizeof(struct Node));
    if (!p) { fprintf(stderr, "Erreur malloc GenStar\n"); exit(1); }
    p->classe      = Star;
    p->star.store  = store;
    return p;
}

PTR GenUn(PTR un) {
    PTR p = malloc(sizeof(struct Node));
    if (!p) { fprintf(stderr, "Erreur malloc GenUn\n"); exit(1); }
    p->classe = Un;
    p->un.un  = un;
    return p;
}

PTR GenAtom(int cod, int act, int ind, AtomType atype) {
    PTR p = malloc(sizeof(struct Node));
    if (!p) { fprintf(stderr, "Erreur malloc GenAtom\n"); exit(1); }
    p->classe      = Atom;
    p->atom.cod    = cod;
    p->atom.act    = act;
    p->atom.ind    = ind;
    p->atom.atype  = atype;
    p->atom.val[0] = '\0';  
    return p;
}

// GENERATION DE LA FORET G0
// Construit A[1..5] a la main selon les 5 regles de G0
//
// Logique des actions :
//   act=2 sur IDNTER de la regle  -> ajouter le nom a la table des symboles
//   act=1 sur ';'                 -> fin de regle, stocker dans A[]
//   act=5 sur IDNTER/ELTER dans F -> creer un Atom et empiler
//   act=6 sur '['                 -> empiler marqueur debut Star
//   act=8 sur ']'                 -> depiler contenu + marqueur, creer Star
//   act=7 sur '('                 -> empiler marqueur debut Un
//   act=9 sur ')'                 -> depiler contenu + marqueur, creer Un
//   act=3 sur F apres '+'         -> depiler deux elements, creer Union
//   act=4 sur F apres '.'         -> depiler deux elements, creer Conc

void GenForetGO(void) {

    // Regle 1 : S -> [N. '->' .E. ';']. ','
    A[IDX_S] = GenStar(
        GenConc(
            GenConc(
                GenConc(
                    GenConc(
                        GenAtom(TOK_IDNTER, 2, IDX_N, NonTerminal),
                        GenAtom(TOK_ARROW,  0, 0,     Terminal)
                    ),
                    GenAtom(0, 0, IDX_E, NonTerminal)
                ),
                GenAtom(';', 1, 0, Terminal)
            ),
            GenAtom(',', 0, 0, Terminal)
        )
    );

    // Regle 2 : N -> 'IDNTER'
    A[IDX_N] = GenAtom(TOK_IDNTER, 0, 0, Terminal);

    // Regle 3 : E -> T. ['+'.T]
    // action 3 sur le T qui suit '+' : a ce moment T est deja empile
    // et le T precedent aussi -> on peut creer l'Union
    A[IDX_E] = GenConc(
        GenAtom(0, 0, IDX_T, NonTerminal),
        GenStar(
            GenConc(
                GenAtom('+', 3, 0,     Terminal),    // action 3 sur '+'
                GenAtom(0,   0, IDX_T, NonTerminal)  // pas d'action sur T
            )
        )
    );

    // Regle 4 : T -> F. ['.'.F]
    // action 4 sur le F qui suit '.' : a ce moment F est deja empile
    // et le F precedent aussi -> on peut creer le Conc
    A[IDX_T] = GenConc(
        GenAtom(0, 0, IDX_F, NonTerminal),
        GenStar(
            GenConc(
                GenAtom('.', 4, 0,     Terminal),    // action 4 sur '.'
                GenAtom(0,   0, IDX_F, NonTerminal)  // pas d'action sur F
            )
        )
    );
    

    // Regle 5 : F -> IDNTER | ELTER | '('.E.')' | '['.E.']' | '/'.E.'/'
    // action 5 sur IDNTER/ELTER  -> creer Atom et empiler
    // action 6 sur '['           -> empiler marqueur
    // action 8 sur ']'           -> creer Star
    // action 7 sur '('           -> empiler marqueur
    // action 9 sur ')'           -> creer Un
    A[IDX_F] = GenUnion(
        GenAtom(TOK_IDNTER, 5, 0, Terminal),
        GenUnion(
            GenAtom(TOK_ELTER, 5, 0, Terminal),
            GenUnion(
                GenConc(
                    GenConc(
                        GenAtom('(', 7, 0,     Terminal),
                        GenAtom(0,   0, IDX_E, NonTerminal)
                    ),
                    GenAtom(')', 9, 0, Terminal)
                ),
                GenUnion(
                    GenConc(
                        GenConc(
                            GenAtom('[', 6, 0,     Terminal),
                            GenAtom(0,   0, IDX_E, NonTerminal)
                        ),
                        GenAtom(']', 8, 0, Terminal)
                    ),
                    GenConc(
                        GenConc(
                            GenAtom('/', 7, 0,     Terminal),
                            GenAtom(0,   0, IDX_E, NonTerminal)
                        ),
                        GenAtom('/', 9, 0, Terminal)
                    )
                )
            )
        )
    );
}

// IMPRIMARBRE — affiche l'arbre avec indentation
void ImprimArbre(PTR p, int prof) {
    if (p == NULL) return;

    for (int i = 0; i < prof; i++) printf("  ");

    switch (p->classe) {
        case Conc:
            printf("Conc\n");
            ImprimArbre(p->conc.left,  prof + 1);
            ImprimArbre(p->conc.right, prof + 1);
            break;
        case Union:
            printf("Union\n");
            ImprimArbre(p->uni.left,  prof + 1);
            ImprimArbre(p->uni.right, prof + 1);
            break;
        case Star:
            printf("Star\n");
            ImprimArbre(p->star.store, prof + 1);
            break;
        case Un:
            printf("Un\n");
            ImprimArbre(p->un.un, prof + 1);
            break;
            case Atom:
            if (p->atom.atype == Terminal) {
                if (p->atom.val[0] != '\0')
                    printf("Atom [Terminal: '%s'  act=%d]\n", p->atom.val, p->atom.act);
                else if (p->atom.cod == TOK_ARROW)
                    printf("Atom [Terminal: '->'  act=%d]\n", p->atom.act);
                else if (p->atom.cod == TOK_IDNTER)
                    printf("Atom [Terminal: IDNTER  act=%d]\n", p->atom.act);
                else if (p->atom.cod == TOK_ELTER)
                    printf("Atom [Terminal: ELTER  act=%d]\n", p->atom.act);
                else
                    printf("Atom [Terminal: '%c'  act=%d]\n", p->atom.cod, p->atom.act);
            } else {
                printf("Atom [NonTerminal: A[%d]  act=%d]\n", p->atom.ind, p->atom.act);
            }
            break;
    }
}