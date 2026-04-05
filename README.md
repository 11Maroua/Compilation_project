# Projet Compilo

Implémentation en C d'un méta-compilateur et d'un compilateur basés sur la grammaire G0.

---

## Architecture globale

```
Grammaire G0 (écrite à la main)
        │
        ▼
┌─────────────────────┐        ┌─────────────────────┐
│   MÉTA-COMPILATEUR  │        │     COMPILATEUR     │
│                     │        │                     │
│  ScanG0             │        │  ScanGPL            │
│  AnalyseG0   ───────┼──────► │  AnalyseGPL         │
│  ActionG0           │        │  ActionGPL          │
│                     │        │        │            │
│  → construit        │        │        ▼            │
│    l'arbre A[6..n]  │        │   P-code généré     │
└─────────────────────┘        │        │            │
                               │        ▼            │
                               │   Exec + Interpret  │
                               └─────────────────────┘
```

**Le méta-compilateur** lit une grammaire GPL et vérifie qu'elle respecte G0.
S'il la valide, il construit sa représentation interne dans le tableau A[6..n].

**Le compilateur** lit un programme écrit en GPL, l'analyse grâce à l'arbre
construit par le méta-compilateur, génère du P-code et l'exécute.

---

## La Grammaire G0 — le cœur du projet

G0 est une **méta-grammaire** : c'est la grammaire qui décrit ce qu'est une
grammaire valide. Elle se compose de 5 règles fixes, écrites à la main dans
le code. Toute grammaire GPL doit respecter ces 5 règles pour être acceptée.

### Les 5 règles de G0

```
(1) S → [N. '→' .E. ';']. ','
(2) N → 'IDNTER'
(3) E → T. ['+'.T]
(4) T → F. ['.'.F]
(5) F → 'IDNTER' | 'ELTER' | '('.E.')' | '['.E.']' | '/'.E.'/'
```

### Notation utilisée dans G0

| Notation | Signification                              | Exemple                          |
|----------|--------------------------------------------|----------------------------------|
| `[X]`    | X répété 0 ou plusieurs fois (étoile `*`)  | `['+'.T]` = zéro ou plusieurs +T |
| `(X)`    | X optionnel (0 ou 1 fois)                  | `('Else'.Inst)` = Else optionnel |
| `.`      | Concaténation — "suivi de"                 | `N.'→'` = N puis →              |
| `+`      | Union — "ou"                               | `'a'+'b'` = a ou b              |
| `'x'`    | Terminal littéral — le caractère x lui-même| `';'` = point-virgule           |
| `IDNTER` | Identifiant non-terminal (S, N, E, T, F…)  | N, E, Inst…                     |
| `ELTER`  | Terminal littéral entre apostrophes        | `'if'`, `'then'`                |

### Ce que chaque règle signifie en français

**Règle 1 — S** : une grammaire valide est une liste de règles séparées par des
virgules. Chaque règle a la forme `NomNonTerminal → Expression ;`

**Règle 2 — N** : un non-terminal est un identifiant (un nom comme S, Inst, Expr…)

**Règle 3 — E** : une expression est un ou plusieurs Termes reliés par `+` (union = "ou")

**Règle 4 — T** : un terme est un ou plusieurs Facteurs reliés par `.` (concaténation = "suivi de")

**Règle 5 — F** : un facteur est soit :
- un non-terminal (IDNTER)
- un terminal littéral (ELTER)
- une expression entre parenthèses `(E)` → optionnel
- une expression entre crochets `[E]` → répétition
- une expression entre slashs `/E/` → optionnel aussi

---

## Structure de données — l'Arbre A

### Le tableau A[]

```c
PTR A[MAX_RULES];
```

C'est le tableau central de tout le projet.

| Cases    | Contenu                      | Rempli par                  |
|----------|------------------------------|-----------------------------|
| A[1]     | Arbre de la règle S de G0    | GenForetG0 (à la main)      |
| A[2]     | Arbre de la règle N de G0    | GenForetG0 (à la main)      |
| A[3]     | Arbre de la règle E de G0    | GenForetG0 (à la main)      |
| A[4]     | Arbre de la règle T de G0    | GenForetG0 (à la main)      |
| A[5]     | Arbre de la règle F de G0    | GenForetG0 (à la main)      |
| A[6..n]  | Arbres des règles de la GPL  | ActionG0 automatiquement    |

### Les types de nœuds

Chaque nœud de l'arbre a un type parmi 5 :

| Type    | Symbole G0 | Fils           | Rôle                    |
|---------|-----------|----------------|-------------------------|
| `Conc`  | `.`       | gauche + droit | "A suivi de B"          |
| `Union` | `+`       | gauche + droit | "A ou B"                |
| `Star`  | `[X]`     | store          | "répéter 0 ou n fois"   |
| `Un`    | `(X)`     | un             | "optionnel, 0 ou 1 fois"|
| `Atom`  | feuille   | aucun          | terminal ou non-terminal|

### Le nœud Atom en détail

Un nœud `Atom` est une feuille de l'arbre. Il peut être de deux sortes :

**Terminal** : représente un symbole littéral à lire dans l'entrée.
```
cod   = code ASCII du caractère (ex: ';' = 59)
        ou code spécial (TOK_ARROW=256, TOK_IDNTER=257, TOK_ELTER=258)
act   = numéro d'action sémantique à déclencher (0 = aucune)
atype = Terminal
```

**NonTerminal** : représente une référence vers une autre règle.
```
ind   = index dans A[] vers la règle référencée
        (ex: ind=3 → A[IDX_E] → règle E)
act   = numéro d'action sémantique à déclencher (0 = aucune)
atype = NonTerminal
```

---

## Les Actions Sémantiques

Quand `AnalyseG0` reconnaît un token qui a un numéro d'action non nul
(`act != 0`), il appelle `ActionG0(numéro)`. Ces actions construisent
l'arbre de la GPL dans A[6..n].

Il y a 7 actions au total :

| #  | Déclenché quand                     | Ce que ça fait                                              |
|----|-------------------------------------|-------------------------------------------------------------|
| 1  | Fin d'une règle (token `;`)         | Raccorde le membre gauche au membre droit. Stocke dans A[index] |
| 2  | Reconnaissance d'un IDNTER          | Ajoute le nom à la table des symboles                       |
| 3  | Reconnaissance du `+` (union)       | Crée un nœud Union dans l'arbre en construction             |
| 4  | Reconnaissance du `.` (concat)      | Crée un nœud Conc dans l'arbre en construction              |
| 5  | IDNTER ou ELTER dans un facteur     | Crée une feuille Atom                                       |
| 6  | Crochets `[` et `]`                 | Crée un nœud Star (répétition)                              |
| 7  | Parenthèses `(` et `)`              | Crée un nœud Un (optionnel)                                 |

**Exemple concret** : quand le méta-compilateur lit la GPL `Sp → ['a'].'b',;`
il reconnaît dans l'ordre :

- `Sp`  → action 2 (ajoute Sp à la table des symboles)
- `[`   → action 6 (prépare un Star)
- `'a'` → action 5 (crée un Atom terminal)
- `]`   → action 6 (ferme le Star)
- `.`   → action 4 (crée un Conc)
- `'b'` → action 5 (crée un Atom terminal)
- `;`   → action 1 (finalise la règle, stocke dans A[6])

---

## Les fonctions de construction (Gen)

Ces fonctions créent les nœuds de l'arbre un par un.

```c
PTR GenConc(PTR p1, PTR p2)                          // nœud '.'
PTR GenUnion(PTR p1, PTR p2)                         // nœud '+'
PTR GenStar(PTR p)                                   // nœud '[X]'
PTR GenUn(PTR p)                                     // nœud '(X)'
PTR GenAtom(int cod, int act, int ind, AtomType t)   // feuille
```

Chacune alloue un nœud en mémoire avec `malloc`, remplit ses champs,
et retourne le pointeur.

---

## Le P-code

Le compilateur génère du P-code — un langage intermédiaire simple
que l'interpréteur exécute ensuite.

### Instructions disponibles

| Catégorie     | Instructions                                          |
|---------------|-------------------------------------------------------|
| Chargement    | `LDA @` (adresse), `LDV val` (valeur), `LDC c` (constante) |
| Affectation   | `AFF`                                                 |
| Entrée/Sortie | `RD` (read), `WRTLN` (writeln)                        |
| Saut          | `JMP @` (inconditionnel), `JIF @` (si faux), `JSR`, `RSR` |
| Arithmétique  | `ADD`, `MIN`, `MULT`, `DIV`, `NEG`, `INC`, `DEC`     |
| Logique       | `AND`, `OR`, `NOT`                                    |
| Comparaison   | `SUP`, `SUPE`, `INF`, `INFE`, `EG`, `DIFF`           |
| Arrêt         | `STOP`                                                |

### Exemple — programme Som compilé en P-code

Programme source GPL :
```
Program Som;
Var I, S, N : int;
Debut
  Read(N);
  S := 0;
  I := 1;
  While I <= N do
    Debut
      S := S + I;
      I := I + 1;
    Fin
  WriteLn(S);
Fin
```

P-code généré :
```
1  LDA 3     → adresse de N
3  RD        → lit N depuis l'entrée
4  AFF       → N := valeur lue
5  LDA 2     → adresse de S
7  LDC 0     → constante 0
9  AFF       → S := 0
10 LDA 1     → adresse de I
12 LDC 1     → constante 1
14 AFF       → I := 1
15 LDV 1     → charge I        ← début du while
17 LDV 3     → charge N
19 INFE      → I <= N ?
20 JIF 38    → si faux sauter à la fin
22 LDA 2     → S := S + I
24 LDV 2
26 LDV 1
28 ADD
29 AFF
30 LDA 1     → I := I + 1
32 LDV 1
34 INC
35 AFF
36 JMP 15    → retour au while
38 LDA 2     → WriteLn(S)
40 WRTLN
41 STOP
```

### Fonctionnement de l'interpréteur

L'interpréteur utilise deux structures :

- `Pcode[]` : le tableau contenant toutes les instructions générées
- `Pilex[]` : la pile d'exécution (joue le rôle des registres)
- `CO` : le compteur ordinal, pointe vers l'instruction courante

La procédure `Exec` boucle tant que `Pcode[CO] != STOP` et appelle
`Interpret(Pcode[CO])` sur chaque instruction. `Interpret` modifie
`Pilex` et `CO` selon l'instruction reçue.

---

## Structure du projet

```
projet-compilo/
├── README.md
├── rapport/
│   └── rapport.pdf
├── src/
│   ├── main.c           ← point d'entrée
│   ├── tree.h           ← types + prototypes (structures de données)
│   ├── tree.c           ← GenConc, GenUnion, GenStar, GenUn,
│   │                       GenAtom, GenForetG0, ImprimArbre
│   ├── scan_g0.h/.c     ← ScanG0 (analyse lexicale de la GPL)
│   ├── g0.h/.c          ← AnalyseG0 + ActionG0
│   ├── scan_gpl.h/.c    ← ScanGPL (analyse lexicale du programme)
│   ├── gpl.h/.c         ← AnalyseGPL + ActionGPL
│   ├── pcode.h/.c       ← génération du P-code
│   └── exec.c           ← Exec + Interpret
├── tests/
│   ├── gpl1.txt         ← Sp → ['a'].'b',;
│   └── prog_somme.txt   ← le programme Som
└── Makefile
```

---

## Compilation et exécution

```bash
# Compiler tout le projet
make

# Lancer le méta-compilateur sur une GPL
./compilo gpl1.txt programme.txt

# Nettoyer les fichiers compilés
make clean
```