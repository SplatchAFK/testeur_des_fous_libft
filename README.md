# Le testeur des fous · Version Libft

> **Version 2.0.0** · [historique des mises à jour](CHANGELOG.md) · `bash testeur_des_fous/run.sh --version`

> Créé et vibe-codé par **Splatch** (login 42 : `romasant`), pour rendre Libft plus marrant.
> Lore et idées : Splatch.

Un testeur de Libft qui se joue comme un jeu vidéo, directement dans le terminal. Il teste tes 43 fonctions. Ensuite, ton petit chevalier affronte un boss pour chaque partie de la lib. Les boss battus débloquent 21 niveaux d'entraînement, et tout ce que tu gagnes se dépense pour habiller ton chevalier.

## Vibe-coding

Ce projet a été entièrement **vibe-codé**, c'est-à-dire écrit en dialoguant avec une IA. C'est un outil à côté du projet, pas un rendu 42. Il ne contient aucune fonction de la Libft, ne donne aucune solution aux exercices et n'écrit rien à ta place. Il se contente de casser ton code.

## Installation

Place ce dépôt dans un dossier `testeur_des_fous` à la racine de ta libft, par exemple :

```bash
git clone https://github.com/SplatchAFK/testeur_des_fous_libft.git testeur_des_fous
```

Puis lance :

```bash
bash testeur_des_fous/run.sh
```

Il te faut `cc` (gcc ou clang), `make`, `bash`, un terminal d'au moins 80 x 24 et `norminette` (le testeur vérifie que c'est la dernière version, 3.3.60 au 5 novembre 2026 ; sinon : `python3 -m pip install --user -U norminette`). `valgrind` est utilisé s'il est installé.

Ça tourne sur Linux, WSL et les postes 42. Sur macOS, `run.sh` reste compatible avec le bash 3.2 d'Apple et n'a besoin ni de `timeout` ni de `--wrap` ; cette partie n'a pas pu être essayée sur un vrai Mac.

**Ne rends jamais ce dossier.** Le sujet interdit les fichiers inutiles, et la Norme porte sur tous tes fichiers. Ajoute-le à ton `.gitignore` :

```bash
echo "testeur_des_fous/" >> .gitignore
```

## Le menu

| Bouton | Ce qu'il fait |
|---|---|
| Tester tout (recompiler) | recompile ta libft et relance tous les tests, puis les combats de boss |
| Entraînement | les 21 niveaux, débloqués par les boss |
| Boss | la galerie des 3 boss, et la possibilité de rejouer un combat |
| Boutique | objets pour l'avatar, thèmes de couleur, 1000 titres |
| Avatar | ta garde-robe : tu équipes ce que tu possèdes et tu choisis les couleurs |
| Profil et succès | ton niveau, ta progression, tes 18 succès |
| Résultats détaillés | chaque fonction et son audit, avec les détails des erreurs |

Dans l'écran des résultats : `f` ouvre la **fiche d'échec** de la fonction choisie (ce qui a été vu,
l'attendu, le cas exact, une piste et la commande pour le rejouer seul ; `◀ ▶` passe à l'autre
fonction en échec), `m` ouvre l'audit. Pendant l'analyse, une barre de progression montre le
pourcentage, la fonction en cours, le nombre de contrôles et le temps restant.
| Paramètres | tout ce qui se règle (voir plus bas) |

## Ce qui est testé

- **Les 43 prototypes**, à la compilation, contre le sujet (`ft_lstsize` accepté en `int` comme en `unsigned int`). La structure `t_list` est vérifiée aussi.
- **Les 43 fonctions**, chacune dans son propre binaire, liée comme la moulinette (via une archive), avec AddressSanitizer et UndefinedBehaviorSanitizer.
- **Un traqueur mémoire** : chacun de tes `malloc` et `free` passe par le testeur (`-include tdf_hook.h`, seulement pendant les tests). Il connaît la taille de chaque bloc et le cas de test en cours, donc il voit :
  - les fuites, avec le cas exact où le bloc a été alloué ;
  - les double free, les `free` d'un argument, les chaînes littérales renvoyées ;
  - les maillons que `ft_lstdelone` ou `ft_lstclear` oublient de libérer ;
  - la **taille exacte** demandée à `malloc` : trop petit ou trop grand = FAUX, comme `mcheck` de tripouille (`malloc(len(s) - start + 1)` au lieu de `malloc(len + 1)` dans `ft_substr`, par exemple).
- **Les échecs de `malloc`**, un par un : tu dois tout libérer et renvoyer `NULL`.
- **Le cas fautif** est donné pour chaque crash, UB ou boucle infinie (« pendant : split("", ' ') »).
- **Les octets >= 0x80** partout où une boucle s'arrête sur `'\0'` (le classique `> '\0'` au lieu de `!= '\0'`).
- **La norminette**, à chaque lancement, sur tes `ft_*.c` et `libft.h`.
- **L'audit moulinette** :
  - règles du Makefile, relink, dépendance à `libft.h`, flags, `-std=c99`, wildcards, `ar` ;
  - fonctions de la libc appelées, variables globales ;
  - `fclean`, `re`, fichiers en trop ;
  - première ligne du README, norminette.
- Une **note moulinette estimée** : 100 ou 0, avec la raison.

## Les 3 boss

| Boss | Partie | Ce qu'il porte |
|---|---|---|
| Le Gardien de la Mémoire | libc | un bonnet de douche, un tutu et un canard en plastique |
| L'Alchimiste des Chaînes | additionnelles | un chapeau à hélice, une bouée canard et une potion verte |
| L'Hydre t_list | liste chaînée | 3 têtes à chapeaux absurdes, dans une baignoire à pattes |

Le combat se joue tout seul :
- chaque fonction qui passe le testeur est un coup d'épée ;
- chaque fonction ratée est une attaque du boss ;
- le boss tombe quand toute sa partie est propre.

Le combat utilise **tes propres fonctions**, les 43. Les points de vie passent par ton `ft_itoa`, les dialogues par ton `ft_split`, les étincelles sont une `t_list`, l'affichage passe par ton `ft_memcmp` et ton `ft_putstr_fd`, et ainsi de suite. Une fonction cassée fait buguer la scène pour de vrai.

Si une de tes fonctions fait planter la scène, elle tourne dans un processus séparé : un écran **CRASH** te donne le nom de la fonction, et tu peux relancer avec elle en quarantaine (une version de secours prend sa place). Une boucle infinie donne un écran **FREEZE**.

## Entraînement : 21 niveaux

Battre un boss débloque les 7 niveaux de sa partie. Dans chaque partie, les niveaux se font dans l'ordre. Chaque niveau commence par une courte histoire absurde (un pigeon en peignoir, une baguette qui fait du yoga…) et donne un sujet : une fonction **nouvelle**, qui n'est pas dans Libft, pour entraîner ce que Libft entraîne mal.

| Partie | Niveaux |
|---|---|
| 1 | `ft_strrev`, `ft_strspn`, `ft_memrchr`, `ft_strcasecmp`, `ft_memswap`, `ft_bitcount`, `ft_atoi_base` |
| 2 | `ft_strndup`, `ft_itoa_base`, `ft_tabdup`, `ft_split_set`, `ft_str_replace`, `ft_putnbr_base_fd`, `ft_sort_strs` |
| 3 | `ft_lstget`, `ft_lstreverse`, `ft_lstfind`, `ft_lstremove_if`, `ft_lstsort`, `ft_lst_to_array`, `ft_lstrotate` |

Tu écris ta fonction dans `testeur_des_fous/entrainement/niveau_XX/ft_nom.c` (le sujet complet est dans `sujet.txt`), puis tu appuies sur `t`. Le testeur vérifie :
- la **Norme**, obligatoire ;
- la compilation ;
- les fonctions autorisées : `malloc`, `free`, `write` et ta libft ;
- tes résultats, avec les sanitizers et des échecs de `malloc`.

Tu peux aussi tester un niveau en ligne de commande :

```bash
bash testeur_des_fous/run.sh --exo 07
```

## Avatar, boutique et progression

- **Ton chevalier casqué** a 11 emplacements : casque, déco de casque, yeux, armure, jambes, cape, arme, main gauche, familier, aura et fond.
- **114 objets** absurdes, par exemple :
  - un bocal à cornichons, une fleur ou une étiquette « -50 % » sur le casque ;
  - une chaussette géante en cape, une baguette de pain comme épée ;
  - un couvercle de poubelle comme bouclier ;
  - un bug en familier.
- **10 couleurs** pour chaque objet, soit des milliards de combinaisons.
- **1000 titres** à collectionner, et **7 thèmes de couleur** (payants, sauf le premier).
- Les **Octets ◈** se gagnent avec :
  - les fonctions validées pour la première fois ;
  - les runs (une fois toutes les 30 min) ;
  - les boss battus et les niveaux d'entraînement ;
  - les succès.
- **XP et niveau**, plus **18 succès**.
- Sauvegarde dans `~/.testeur_des_fous/`. Une sauvegarde modifiée à la main est détectée et remise à zéro.

## Paramètres

Tout se règle depuis le menu :
- **affichage** : animations (normales, rapides, aucune), couleurs (24 bits ou 256), langue (français ou anglais), touches (ZQSD, WASD ou HJKL), son, thème ;
- **tests** : temps maximum par test, AddressSanitizer, UBSan, valgrind, norminette, audit moulinette (auto, toujours, jamais) ;
- **compte** : pseudo, remise à zéro de la sauvegarde.

L'anglais couvre l'interface et les sujets. Le lore reste en français.

## Options de la ligne de commande

| Commande | Effet |
|---|---|
| `bash testeur_des_fous/run.sh` | tout tester, combats de boss, menu |
| `bash testeur_des_fous/run.sh split itoa` | tester seulement ces fonctions |
| `bash testeur_des_fous/run.sh --ci` | sortie texte simple, sans interface |
| `bash testeur_des_fous/run.sh --exo 07` | tester le niveau d'entraînement 07 |
| `bash testeur_des_fous/run.sh --audit` | forcer l'audit moulinette |
| `bash testeur_des_fous/run.sh --valgrind` | repasser les fonctions OK sous valgrind |
| `bash testeur_des_fous/run.sh --ci -v` | rapports complets des sanitizers |
| `bash testeur_des_fous/run.sh -k` | garder le dossier de build |
| `bash testeur_des_fous/run.sh --version` | afficher le numéro de version |


## Les statuts

| Statut | Sens |
|---|---|
| OK | tout passe |
| NOTE | tout passe, avec une remarque (par exemple `calloc(0, x)` renvoie NULL, autorisé par POSIX) |
| FAUX | un résultat ou un comportement mémoire est faux |
| CRASH | erreur mémoire (ASan) ou signal, avec le cas qui tournait |
| FUITE | des blocs alloués par ta fonction ne sont jamais libérés |
| UB | comportement indéfini (UBSan) |
| BOUCLE | aucune réponse dans le temps imparti |
| COMPIL | ne compile pas, ou appelle une fonction qui ne compile pas |
| ABSENT | fichier pas encore écrit |

## Crédits

- Créé et vibe-codé par Splatch (`romasant`).
- Lore et idées : Splatch.
- Exercices d'entraînement conçus pour le testeur des fous.
- Pixel art dessiné avec Python et Pillow (`outils/art.py` régénère `src/art.c`).
- Sujet de référence : Libft v1.3.
- Cas de test inspirés de [libftTester de Tripouille](https://github.com/Tripouille/libftTester) (vérification de la taille des `malloc`, cas de `ft_split`, `ft_strtrim`, `ft_substr`, `ft_calloc`) ; le code du testeur des fous est écrit à part.