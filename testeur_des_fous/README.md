# Le testeur des fous · Version Libft

> Créé et vibe-codé par **Splatch** (login 42 : `romasant`), pour rendre Libft plus marrant.
> Lore et idées : Splatch.

Un testeur de Libft qui se joue comme un jeu vidéo, directement dans le terminal. Il teste tes 43 fonctions. Ensuite, ton petit chevalier affronte un boss pour chaque partie de la lib. Les boss battus débloquent 21 niveaux d'entraînement, et tout ce que tu gagnes se dépense pour habiller ton chevalier.

## Vibe-coding

Ce projet a été entièrement **vibe-codé**, c'est-à-dire écrit en dialoguant avec une IA. C'est un outil à côté du projet, pas un rendu 42. Il ne contient aucune fonction de la Libft, ne donne aucune solution aux exercices et n'écrit rien à ta place. Il se contente de casser ton code.

## Installation

Place ce dépôt dans un dossier `testeur_des_fous` à la racine de ta libft, par exemple :

```bash
git clone <url-du-dépôt> testeur_des_fous
```

Puis lance :

```bash
bash testeur_des_fous/run.sh
```

Il te faut `cc`, `make`, un terminal d'au moins 80 x 24 et `norminette`. `valgrind` est utilisé s'il est installé.

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
| Paramètres | tout ce qui se règle (voir plus bas) |

## Ce qui est testé

- **Les 43 prototypes**, à la compilation, contre le sujet **v1.3** (`unsigned int ft_lstsize`). La structure `t_list` est vérifiée aussi.
- **Les 43 fonctions**, chacune dans son propre binaire, avec AddressSanitizer, UndefinedBehaviorSanitizer et LeakSanitizer. Les `malloc` échouent un par un pour vérifier que tu libères tout et renvoies `NULL`. Un test qui ne répond plus est arrêté.
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
- **tests** : temps maximum par test, AddressSanitizer, UBSan, valgrind, norminette dans l'audit, audit moulinette (auto, toujours, jamais) ;
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

## Crédits

- Créé et vibe-codé par Splatch (`romasant`).
- Lore et idées : Splatch.
- Exercices d'entraînement conçus pour le testeur des fous.
- Pixel art dessiné avec Python et Pillow (`outils/art.py` régénère `src/art.c`).
- Sujet de référence : Libft v1.3.
