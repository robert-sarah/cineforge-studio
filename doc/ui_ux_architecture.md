# Architecture UI/UX - Version Professionnelle

Pour transformer l'interface actuelle (qui ressemble à un simple formulaire) en un outil de montage vidéo professionnel (style CapCut, Premiere, DaVinci), nous allons utiliser les puissantes fonctionnalités de **Qt 6**.

## Thème Global
L'application utilisera un thème sombre (Dark Mode) natif. Qt permet de configurer la palette de couleurs via QPalette et QSS (Qt Style Sheets).
- **Fond principal :** Gris très foncé (`#1e1e24`)
- **Panneaux :** Gris foncé (`#2b2b36`)
- **Accents (Boutons, sélection) :** Bleu/Violet (`#5a5af0` ou `#007aff`)
- **Texte :** Blanc/Gris clair (`#e0e0e0`)

## Layout Principal
La fenêtre principale (`MainWindow`) sera divisée en trois zones majeures, typiques des logiciels de montage :

1. **Zone Supérieure Gauche : Panneau Média & Modèles (Media Pool & Templates)**
   - Un onglet "Médias" : Affiche sous forme de grille ou liste les fichiers importés (vidéos, images, audio).
   - Un onglet "Modèles/Styles" : Permet de choisir le style de montage (ex: "MrBeast", "Cinématique", "Tutoriel").
   - Un onglet "Agent" : Un champ de texte type "chat" pour interagir avec l'IA locale (ex: "Coupe les silences et ajoute des sous-titres jaunes").

2. **Zone Supérieure Droite : Lecteur & Inspecteur (Player & Inspector)**
   - **Lecteur (Player) :** Affiche la vidéo en cours de montage. Utilisera un `QLabel` (pour afficher des images/frames) ou `QMediaPlayer` si disponible, mais comme nous rendons avec FFmpeg, nous utiliserons probablement un lecteur personnalisé qui lit des images générées.
   - **Inspecteur :** Affiche les propriétés de l'élément sélectionné (Clip, Texte). Permet de modifier l'échelle, la position, la police des sous-titres, etc.

3. **Zone Inférieure : Timeline Multipiste**
   - C'est le cœur de l'application. Une vue défilable horizontalement (`QGraphicsView` ou un widget personnalisé avec `QPainter`).
   - Pistes vidéo/image.
   - Pistes audio/voix.
   - Piste de sous-titres.
   - Tête de lecture (Playhead) indiquant la position actuelle.

## Implémentation Technique (Qt)

- **QDockWidget :** Pour rendre les panneaux (Média, Inspecteur) détachables ou redimensionnables.
- **QGraphicsScene / QGraphicsView :** La meilleure approche pour créer une Timeline complexe. Chaque clip sera un `QGraphicsItem`.
- **QStyleSheet :** Pour appliquer le thème sombre.

## Intégration des Modèles et IA
- L'onglet "Agent" enverra ses commandes à la classe `LocalAgent` existante.
- L'interface affichera clairement si les modèles Whisper/Piper sont chargés (indicateurs de statut vert/rouge).
