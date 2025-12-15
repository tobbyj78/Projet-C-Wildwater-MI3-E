# Projet Distribution d'Eau

Analyse de données d'un système de distribution d'eau en France via un script Shell et un programme C.

## Structure

```
├── run.sh              # Script principal
├── src/                # Code source C (AVL, histogrammes, fuites)
├── data/               # Fichiers de données CSV
└── output/             # Résultats (CSV + PNG)
```

## Compilation

```bash
make -C src
```

## Utilisation

### Histogrammes des usines

```bash
./run.sh <fichier_données> histo <mode>
```

Modes disponibles :
- `max` : Capacité maximale de traitement
- `src` : Volume total capté depuis les sources
- `real` : Volume réellement traité (après fuites)

Exemple :
```bash
./run.sh data/c-wildwater_v3.dat histo max
```

Génère un fichier CSV trié et deux images PNG (50 plus petites / 10 plus grandes usines).

### Calcul des fuites

```bash
./run.sh <fichier_données> leaks "<identifiant_usine>"
```

Exemple :
```bash
./run.sh data/c-wildwater_v3.dat leaks "Plant #OU300273S"
```

Calcule le volume d'eau perdu en aval de l'usine et l'ajoute à l'historique.

## Sorties

Les résultats sont stockés dans `output/` :
- `histo_*.dat` : Données CSV des histogrammes
- `histo_*_small.png` / `histo_*_large.png` : Graphiques
- `leaks_history.dat` : Historique des calculs de fuites

## Dépendances

- GCC
- Make
- Gnuplot (pour les graphiques PNG)
