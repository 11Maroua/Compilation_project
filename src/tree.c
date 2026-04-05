#include <stdio.h>
#include <stdlib.h>
#include "tree.h"

/* ─── Définition du tableau global qui contient régles grammaires go + programmes GPL ─── */

PTR A[MAX_RULES];

//  FONCTIONS DE CONSTRUCTION DES NŒUDS

PTR GenConc(PTR p1, PTR p2)
{
    PTR p = malloc(sizeof(struct Node));
    if (!p)
    {
        fprintf(stderr, "Erreur malloc GenConc\n");
        exit(1);
    }
    p->classe = Conc;
    p->conc.left = p1;
    p->conc.right = p2;
    return p;
}

PTR GenUnion(PTR p1, PTR p2)
{
    PTR p = malloc(sizeof(struct Node));
    if (!p)
    {
        fprintf(stderr, "Erreur malloc GenUnion\n");
        exit(1);
    }
    p->classe = Union;
    p->uni.left = p1;
    p->uni.right = p2;
    return p;
}

PTR GenStar(PTR store)
{
    PTR p = malloc(sizeof(struct Node));
    if (!p)
    {
        fprintf(stderr, "Erreur malloc GenStar\n");
        exit(1);
    }
    p->classe = Star;
    p->star.store = store;
    return p;
}

PTR GenUn(PTR un)
{
    PTR p = malloc(sizeof(struct Node));
    if (!p)
    {
        fprintf(stderr, "Erreur malloc GenUn\n");
        exit(1);
    }
    p->classe = Un;
    p->un.un = un;
    return p;
}

PTR GenAtom(int cod, int act, int ind, AtomType atype)
{
    PTR p = malloc(sizeof(struct Node));
    if (!p)
    {
        fprintf(stderr, "Erreur malloc GenAtom\n");
        exit(1);
    }
    p->classe = Atom;
    p->atom.cod = cod;
    p->atom.act = act;
    p->atom.ind = ind;
    p->atom.atype = atype;
    return p;
}

/* GÉNÉRATION DE LA FORÊT G0
   Construit A[1..5] à la main selon les 5 règles de G0 */

/* Codes spéciaux pour les tokens de G0 */
#define TOK_ARROW 256  /* '→'  */
#define TOK_IDNTER 257 /* identifiant non-terminal */
#define TOK_ELTER 258  /* terminal littéral entre apostrophes */

void GenForetG0(void)
{

    /*
     * Règle 1 : S → [N. '→' .E. ';']. ','

     *   Conc
     *   ├── Star
     *   │    └── Conc
     *   │         ├── Conc
     *   │         │    ├── Conc
     *   │         │    │    ├── Atom(N, NonTerminal)
     *   │         │    │    └── Atom('→', Terminal)
     *   │         │    └── Atom(E, NonTerminal)
     *   │         └── Atom(';', Terminal)
     *   └── Atom(',', Terminal)
     */
    A[IDX_S] = GenConc(
        GenStar(
            GenConc(
                GenConc(
                    GenConc(
                        GenAtom(TOK_IDNTER, 2, IDX_N, NonTerminal),
                        GenAtom(TOK_ARROW, 0, 0, Terminal)),
                    GenAtom(0, 0, IDX_E, NonTerminal)),
                GenAtom(';', 1, 0, Terminal))),
        GenAtom(',', 0, 0, Terminal));

    /*
     * Règle 2 : N → 'IDNTER'

     *   Atom(IDNTER, Terminal)
     */
    A[IDX_N] = GenAtom(TOK_IDNTER, 0, 0, Terminal);

    /*
     * Règle 3 : E → T. ['+'.T]
     *
     *   Conc
     *   ├── Atom(T, NonTerminal)
     *   └── Star
     *        └── Conc
     *             ├── Atom('+', Terminal)
     *             └── Atom(T, NonTerminal)
     */
    A[IDX_E] = GenConc(
        GenAtom(0, 0, IDX_T, NonTerminal),
        GenStar(
            GenConc(
                GenAtom('+', 3, 0, Terminal),
                GenAtom(0, 0, IDX_T, NonTerminal))));

    /*
     * Règle 4 : T → F. ['.'.F]
     *
     *   Conc
     *   ├── Atom(F, NonTerminal)
     *   └── Star
     *        └── Conc
     *             ├── Atom('.', Terminal)
     *             └── Atom(F, NonTerminal)
     */
    A[IDX_T] = GenConc(
        GenAtom(0, 0, IDX_F, NonTerminal),
        GenStar(
            GenConc(
                GenAtom('.', 4, 0, Terminal),
                GenAtom(0, 0, IDX_F, NonTerminal))));

    /*
     * Règle 5 : F → 'IDNTER' | 'ELTER' | '('.E.')' | '['.E.']' | '/'.E.'/'
     *
     *   Union
     *   ├── Atom(IDNTER, Terminal)
     *   ├── Union
     *   │    ├── Atom(ELTER, Terminal)
     *   │    ├── Union
     *   │    │    ├── Conc('('.E.')')
     *   │    │    ├── Union
     *   │    │    │    ├── Conc('['.E.']')
     *   │    │    │    └── Conc('/'.E.'/')
     */
    A[IDX_F] = GenUnion(
        GenAtom(TOK_IDNTER, 5, 0, Terminal),
        GenUnion(
            GenAtom(TOK_ELTER, 5, 0, Terminal),
            GenUnion(
                GenConc(
                    GenConc(
                        GenAtom('(', 7, 0, Terminal),
                        GenAtom(0, 0, IDX_E, NonTerminal)),
                    GenAtom(')', 0, 0, Terminal)),
                GenUnion(
                    GenConc(
                        GenConc(
                            GenAtom('[', 6, 0, Terminal),
                            GenAtom(0, 0, IDX_E, NonTerminal)),
                        GenAtom(']', 0, 0, Terminal)),
                    GenConc(
                        GenConc(
                            GenAtom('/', 7, 0, Terminal),
                            GenAtom(0, 0, IDX_E, NonTerminal)),
                        GenAtom('/', 0, 0, Terminal))))));
}

/* IMPRIMARBRE:
         Affiche l'arbre avec indentation selon la profondeur */

void ImprimArbre(PTR p, int prof)
{
    if (p == NULL)
        return;

    /* Indentation */
    for (int i = 0; i < prof; i++)
        printf("  ");

    switch (p->classe)
    {
    case Conc:
        printf("Conc\n");
        ImprimArbre(p->conc.left, prof + 1);
        ImprimArbre(p->conc.right, prof + 1);
        break;

    case Union:
        printf("Union\n");
        ImprimArbre(p->uni.left, prof + 1);
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
        if (p->atom.atype == Terminal)
        {
            if (p->atom.cod == TOK_ARROW)
                printf("Atom [Terminal: '→'  act=%d]\n", p->atom.act);
            else if (p->atom.cod == TOK_IDNTER)
                printf("Atom [Terminal: IDNTER  act=%d]\n", p->atom.act);
            else if (p->atom.cod == TOK_ELTER)
                printf("Atom [Terminal: ELTER  act=%d]\n", p->atom.act);
            else
                printf("Atom [Terminal: '%c'  act=%d]\n", p->atom.cod, p->atom.act);
        }
        else
        {
            printf("Atom [NonTerminal: A[%d]  act=%d]\n",
                   p->atom.ind, p->atom.act);
        }
        break;
    }
}