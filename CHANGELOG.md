# Historique des mises à jour

Le numéro de version suit le schéma `MAJEUR.MINEUR.CORRECTIF` et s'écrit à un seul
endroit : `src/version.h`. Il s'affiche dans le jeu (intro, menu, crédits), dans
l'en-tête du mode texte et avec `bash testeur_des_fous/run.sh --version`.
Au premier lancement d'une nouvelle version, le jeu affiche un message
« MISE À JOUR DU TESTEUR v ancienne → v nouvelle ».

- **MAJEUR** : un changement qui modifie les résultats ou la façon de s'en servir.
- **MINEUR** : une nouvelle fonctionnalité.
- **CORRECTIF** : une correction de bug, sans changement visible.

## 2.0.0 · 2026-11-06

Mise à jour de fiabilité. Les résultats changent : certaines libft qui passaient avant
peuvent maintenant échouer (c'est voulu, voir « Tests »).

### Nouveau
- **Numéro de version** partout (jeu, mode texte, `--version`, README) et message de mise à jour.
- **Fiche d'échec** : dans l'écran des résultats, `f` ouvre une fiche par fonction : ce qui a été vu,
  l'attendu, le cas exact, une piste pour corriger (jamais de code à ta place) et la commande pour
  rejouer ce test seul. `◀ ▶` passe à l'autre fonction en échec.
- **Barre de progression** pendant l'analyse : pourcentage, fonction en cours, nombre de contrôles
  en direct, temps restant. Le mode texte (`--ci`) numérote chaque ligne (`12/43`).

### Tests
- **Taille exacte des `malloc`** : un `malloc` trop grand ou trop petit est FAUX, comme `mcheck` de
  tripouille (par exemple `ft_substr` qui alloue `strlen(s) - start + 1` au lieu de la longueur
  réelle de la sous-chaîne). Fonctionne sur toutes les machines, avec ou sans valgrind.
- **Traqueur mémoire** : fuites avec le cas exact, double `free`, `free` d'un argument, chaîne
  littérale renvoyée, maillons non libérés par `ft_lstdelone` et `ft_lstclear`.
- **Échecs de `malloc`** testés un par un sur toutes les fonctions qui allouent.
- **Cas qui plantent** : le testeur donne le cas qui tournait (« pendant : split("", ' ') »).
- **Octets >= 0x80** et caractères inférieurs au séparateur (`ft_split`, `ft_strtrim`,
  `ft_strnstr`, `ft_strlcat`, `ft_striteri`) : le piège classique des `char` signés.
- **norminette** lancée à chaque fois, avec contrôle de la version (3.3.60).
- Faux positifs corrigés : fonctions ctype qui renvoient autre chose que 1 (1024 par exemple),
  `ft_lstclear` récursif, `ft_lstsize` en `int`, dossier du testeur compté comme fichier en trop.

### Jeu
- Le jeu se lance même si une de tes fonctions ne compile pas (version de secours).
- Noms internes préfixés `tdf_` : plus de collision avec tes fonctions d'aide (`fill`, `reset`...).
- Énoncés d'entraînement coupés (12 niveaux sur 21), audit tronqué, bandeau qui laissait une
  traînée, débordements à 80 colonnes (profil, paramètres, boutique, écran CRASH), compteur
  d'entraînement faux.
- Touche d'audit : `m` (la touche `a` déplaçait le curseur en mode WASD mais ouvrait l'audit).
- Fonctionne dans un dossier dont le chemin contient des espaces, avec bash 3.2, sans `timeout`
  et sans sanitizers.

## 1.x · avant le suivi des versions

Première version publique : 43 fonctions testées avec les sanitizers, audit moulinette, boss,
boutique, 21 niveaux d'entraînement.
