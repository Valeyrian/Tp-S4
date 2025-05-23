Fonctionnalit�s suppl�mentaires impl�ment�es :

- **Mode 4 joueurs** :
  Activable depuis les param�tres du jeu, puis en cliquant sur le bouton "Restart".

- **Mode 8 joueurs** :
  Activable �galement depuis les param�tres du jeu, puis en cliquant sur le bouton "Restart".

- **Barre de progression** :
  Une barre individuelle pour chaque joueur indique le niveau de progression vers la victoire.

- **Chronom�tre de tour** :
  Affiche le temps mis par chaque joueur pour effectuer son coup.

- **Annulation de coups** :
  Un bouton permet d�annuler un ou plusieurs coups jou�s, que ce soit par un joueur ou par l�IA.

- **Aper�u du chemin optimal** :
  Un bouton permet d�activer un aper�u du plus court chemin vers l�arriv�e depuis la position actuelle du joueur.

- **Instruction pr�processeur A_STAR** :
  Permet d�utiliser l�algorithme A* � la place de BFS pour toutes les recherches de chemin le plus court.

Remarques et limitations :

- Les modes 4 et 8 joueurs sont compatibles avec l�IA, mais la profondeur de recherche est limit�e � 2.

- La fonction de d�part al�atoire (random start) n�est pas compatible avec le mode 8 joueurs.

- L�algorithme A* n�est pas compatible avec le mode 8 joueurs.
