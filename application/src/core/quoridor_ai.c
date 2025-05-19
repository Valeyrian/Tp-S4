/*
	Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
	Licensed under the MIT License.
	See LICENSE.md in the project root for license information.
*/

#include "core/quoridor_core.h"
#include "core/utils.h"
#include "core/graph.h"
#include "core/shortest_path.h"
#include "core/quoridor_ai.h"

#define MAX_BEST_WALLS 1

void* AIData_create(QuoridorCore* core)
{
	// Cette fonction n'est utile que si vous avez besoin de mémoriser des informations sur les coups précédents de l'IA.

	return NULL;
}

void AIData_destroy(void* self)
{
	if (!self) return;
}

void AIData_reset(void* self)
{
}

Graph* QuoridorCore_initGraph(QuoridorCore* self, int playerID)
{

	Graph* graph = Graph_create(81);
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
		{

			QuoridorPos moves[8];
			QuoridorPos pos;
			pos.i = i;
			pos.j = j;
			int moveCount = 0;
			if (self->positions[playerID].i == pos.i && self->positions[playerID].j == pos.j)
				moveCount = QuoridorCore_getMoves(self, moves, pos, 1);
			else
				moveCount = QuoridorCore_getMoves(self, moves, pos, 0);


			for (int k = 0; k < moveCount; k++)
			{
				Graph_setArc(graph, pos.i * 9 + pos.j, moves[k].i * 9 + moves[k].j, 1);
				Graph_setArc(graph, moves[k].i * 9 + moves[k].j, pos.i * 9 + pos.j, 1);
			}
		}
	return graph;
}

///@brief calcul l'heuristique d'un point de la grille
int heuristic(QuoridorCore* self, int node, int player) 
{

	int goal = (player == 0) ? (self->gridSize - 1) : 0; // la ligne d'arrivée du joueur

	int col = node % self->gridSize; // on recupere la colonne

	// On calcule la distance de Manhattan entre le noeud et la ligne d'arrivée
	return abs(goal - col);
}

//@brief fonction pour remonter le chemin et le recup dans le bon sens
void pathRefinder(QuoridorCore *self, int* cameFrom, int current, QuoridorPos* path, int* pathSize) 
{
	
	int boardSize = self->gridSize * self->gridSize; // taille du tableau de chemin
	int* temp = calloc(boardSize, sizeof(int)); 

	int count = 0;


	// On remonte le chemin en partant du noeud courant jusqu'à la racine et dcp stocker a l'enver
	while (cameFrom[current] != -1) 
	{
		temp[count++] = current;
		current = cameFrom[current];
	}
	temp[count++] = current;

	*pathSize = count;

	//on retourne le chemin dans le bon sens et on separe i et j
	for (int i = 0; i < count; i++) {
		int node = temp[count - 1 - i];
		path[i].i = node / self->gridSize;
		path[i].j = node % self->gridSize;
	}

	free(temp); // on libere le tableau temporaire
}

//focntion pour prend le moin couteux dans ceux accesible
int extractLowestF(bool* inOpenSet, int* fScore, int size) 
{
	int minF = INT_MAX;
	int bestNode = -1;

	for (int i = 0; i < size; i++) {
		if (inOpenSet[i] && fScore[i] < minF) {
			minF = fScore[i];
			bestNode = i;
		}
	}
	return bestNode;
}

/// @Calcule le plus court chemin entre la position du joueur et sa zone d'arrivée. A*
/// chokbar c'est pas recursif
void AStarShortestPath(QuoridorCore *self ,Graph* graph, int player, QuoridorPos* path, int* pathSize) 
{


	int start = self->positions[player].i * self->gridSize + self->positions[player].j; // position de depart du joueur  


	
	int boardLength = self->gridSize * self->gridSize; 

	int* gScore = calloc(boardLength, sizeof(int));  //la distance entre le point de départ et mtn
	int* fScore = calloc(boardLength, sizeof(int));  //gscore + heuristique au point
	int* cameFrom = calloc(boardLength, sizeof(int)); //tableau du chemin parcourue
	
	bool* inOpenSet = calloc(boardLength, sizeof(bool));  //tableau de boolen pour savoir si le point est dans l'ensemble ouvert (deja vue)

	for (int i = 0; i < boardLength; i++) 
	{
		gScore[i] = INT_MAX;
		fScore[i] = INT_MAX; 
		cameFrom[i] = -1; 
	}

	gScore[start] = 0;
	fScore[start] = heuristic(self,start, player); 
	inOpenSet[start] = true; //faut bien commencer qlq part

	while (true) 
	{
		int current = extractLowestF(inOpenSet, fScore, boardLength); //on prend le plus petit fscore de l'ensemble ouvert si on trouve pas on prendrra le suivant
		if (current == -1)
			break; // Open set empty → pas de chemin


		
		int col = current % self->gridSize;  // extrait j (la colonne)  

		if (player == 0) // test de si on est arrive joueur 0 et 1  
		{
			if (col == self->gridSize - 1)
			{
				pathRefinder(self, cameFrom, current, path, pathSize); 
				return;
			}
		}
		else if (player == 1)
		{
			if (col == 0)
			{
				pathRefinder(self, cameFrom, current, path, pathSize); 
				return;
			}
		}
		
		inOpenSet[current] = false; // on marque le noeud comme visité

		ArcList* neighbors = Graph_getArcList(graph, current); //on recupere tt les voisins
		
		while (neighbors != NULL) 
		{
			int neighbor = neighbors->target; 
			int tentativeG = gScore[current] + 1; // poids 1 car case adjacente (a check a cause de mouton et cheval)

			if (tentativeG < gScore[neighbor]) //verifie si le chemin pour aller vers le voisin et plus cout en que le chemin deja connu
			{
				cameFrom[neighbor] = current; 
				gScore[neighbor] = tentativeG;
				fScore[neighbor] = tentativeG + heuristic(self,neighbor, player);   

				if (!inOpenSet[neighbor])  //si le voisin n'est pas dans l'ensemble a visiter on l'ajoute
				{
					inOpenSet[neighbor] = true;
				}
			}

			neighbors = neighbors->next; //on passe au voisin suivant
		}
	}

	*pathSize = -1; // Aucun chemin trouvé
	free(gScore); 
	free(fScore);   
	free(cameFrom); 
	free(inOpenSet); 
}

/// @brief Calcule le plus court chemin entre la position du joueur et sa zone d'arrivée. disjktra
void QuoridorCore_getShortestPath(QuoridorCore* self, int playerID, QuoridorPos* path, int* size, Graph* graph)
{

	int min = INT_MAX;
	Path* bestpath;
	if (playerID == 1)
		for (int j = 0; j < 9; j++)
		{
			Path* path = Graph_shortestPath(graph, self->positions[playerID].i * 9 + self->positions[playerID].j, j * 9);
			if (!path)
				continue;
			if (path->distance < min)
			{
				min = path->distance;
				bestpath = path;
			}
		}

	if (playerID == 0)
		for (int j = 0; j < 9; j++)
		{
			Path* path = Graph_shortestPath(graph, self->positions[playerID].i * 9 + self->positions[playerID].j, j * 9 + 8);
			if (!path)
				continue;
			if (path->distance < min)
			{
				min = path->distance;
				bestpath = path;
			}
		}

	*size = bestpath->distance + 1;
	int idx = 0;
	while (!ListInt_isEmpty(bestpath->list))
	{
		int tmp = ListInt_popLast(bestpath->list);


		path[idx].i = (tmp - tmp % 9) / 9;
		path[idx++].j = tmp % 9;



	}

}

/// @brief Calcule une heuristique d'évaluation de l'état du jeu pour un joueur donné.
/// Cette fonction est utilisée dans l'algorithme Min-Max pour estimer la qualité d'une position.
/// Elle retourne une valeur représentant l'avantage du joueur playerID.
/// Une valeur positive indique un avantage pour ce joueur, une valeur négative indique un avantage pour l'adversaire.
/// @param self Instance du jeu Quoridor.
/// @param playerID Indice du joueur à évaluer (0 ou 1).
/// @return Une estimation numérique de l'avantage du joueur playerID.
static float QuoridorCore_computeScore(QuoridorCore* self, int playerID)
{
	int playerA = self->playerID;
	int playerB = self->playerID ^ 1;

	// 1. Calculer la distance (chemin le plus court) vers la ligne de but
	QuoridorPos path[MAX_PATH_LEN];
	int distA = 0;
	int distB = 0;
	//Graph* graph = QuoridorCore_initGraph(self, playerA);
	//////QuoridorCore_getShortestPath(self, playerA, path, &distA, graph);
	//AStarShortestPath(self, graph, playerA, path, &distA); 

	//Graph* graph2 = QuoridorCore_initGraph(self, playerB);
	////QuoridorCore_getShortestPath(self, playerB, path, &distB, graph2);
	//AStarShortestPath(self, graph2, playerB, path, &distB); 
	//
	//Graph_destroy(graph);
	//Graph_destroy(graph2);

	QuoridorPos* tab = calloc(self->gridSize * self->gridSize, sizeof(QuoridorPos));
	distA = BFS_search2(self, playerA, tab);
	distB = BFS_search2(self, playerB, tab);

  /*
    ListQuor* list = BFS_search(self, playerA);
    distA = ListQuor_size(list);
    free(list);
    ListQuor* list2 = BFS_search(self, playerB);
    distB = ListQuor_size(list2);
    free(list2);
  */
	
  // 2. Ajouter pondération sur le nombre de murs restants (optionnel mais utile)
	int wallsA = self->wallCounts[playerA];
	int wallsB = self->wallCounts[playerB];

	// Pondérer les murs à petite échelle (ex : 0.5 par mur)
	float score = (float)(distB - distA);// +0.3f * (wallsA - wallsB);

	// 3. Ajouter un bruit aléatoire pour casser les égalités (pas obligatoire mais pratique)

	//score += Float_randAB(-0.2f, +0.2f); 


	return score;


}

/// @brief Applique l'algorithme Min-Max (avec élagage alpha-bêta) pour déterminer le coup joué par l'IA.
/// Cette fonction explore récursivement une partie de l'arbre des coups possibles jusqu'à une profondeur maximale donnée.
/// @param self Instance du jeu Quoridor.
/// @param playerID Identifiant du joueur pour lequel on cherche le meilleur coup.
/// @param currDepth Profondeur actuelle dans l'arbre de recherche.
/// @param maxDepth Profondeur maximale à atteindre dans l'arbre.
/// @param alpha Meilleure valeur actuellement garantie pour le joueur maximisant.
/// @param beta Meilleure valeur actuellement garantie pour le joueur minimisant.
/// @param turn Pointeur vers une variable où sera enregistré le meilleur coup trouvé (à la racine).
/// @return L'évaluation numérique de la position courante, selon la fonction heuristique.
static float QuoridorCore_minMax(QuoridorCore* self, int playerID, int currDepth, int maxDepth, float alpha, float beta, QuoridorTurn* turn)
{
	printf("depth : %d\n", currDepth); 

    if (self->state != QUORIDOR_STATE_IN_PROGRESS) //si un des joueurs a gagné
    {
        if (self->state == QUORIDOR_STATE_P0_WON && playerID == 0)
            return INFINITY;
        if (self->state == QUORIDOR_STATE_P1_WON && playerID == 0)
            return -INFINITY;
        if (self->state == QUORIDOR_STATE_P0_WON && playerID == 1)
            return -INFINITY;
        if (self->state == QUORIDOR_STATE_P1_WON && playerID == 1)
            return INFINITY;
    }
    else if (currDepth >= maxDepth) //si on atteint le profondzeur max
    {
        return QuoridorCore_computeScore(self, playerID);
    }
    int best = 0;
    int IsWall = 0;

    const int gridSize = self->gridSize;
    const int currI = self->positions[self->playerID].i;
    const int currJ = self->positions[self->playerID].j;
    QuoridorTurn childTurn = { 0 };

    const bool maximizing = (currDepth % 2) == 0;
    float value = maximizing ? -INFINITY : INFINITY;


    int moveCount = 0;
    QuoridorPos moves[8];
    moveCount = QuoridorCore_getMoves(self, moves, self->positions[playerID], 1); 

    for (int k = 0; k < moveCount; k++) //mouvements
    {
		printf("mouvement : %d\n", k);
        QuoridorCore copy = *self;
        copy.positions[playerID].i = moves[k].i;
        copy.positions[playerID].j = moves[k].j;

        float tmp = QuoridorCore_minMax(&copy, playerID ^ 1, currDepth + 1, maxDepth, alpha, beta, turn);

        if (maximizing) //élégage aplha
        {

            if (tmp >= value)
            {
                value = tmp;
                if (currDepth == 0)
                {
                    best = k;
                    IsWall = 0;

                }
            }
            alpha = value;
            if (alpha > beta)
                break;
        }
        if (!maximizing) //élagage béta
        {
            if (tmp <= value)
                value = tmp;
            beta = value;
            if (beta < alpha)
                break;
        }
    }
    QuoridorWall* walls = (QuoridorWall*)calloc(sizeof(QuoridorWall), MAX_BEST_WALLS);  //on initialise le tableau de 5 murs 
	
	int wallCount = 0;
    //getBestWall(self, playerID, 999, walls,&wallCount,collectAllWallsNearPath);  
	 
	getBestWall(self, playerID, 999, walls,&wallCount, collectFewWallsInFrontOfPath); //on recupere les meilleurs murs a jouer 

		
    for (int m = 0; m < wallCount; m++) //murs 
    {
		printf("murs : %d\n", m);
        QuoridorCore gamecopy = *self;
		
        if (walls[m].type == WALL_TYPE_HORIZONTAL)
        {
            gamecopy.hWalls[walls[m].pos.i][walls[m].pos.j] = WALL_STATE_START;
            gamecopy.hWalls[walls[m].pos.i][walls[m].pos.j+1] = WALL_STATE_END;

        }
        if (walls[m].type == WALL_TYPE_VERTICAL)
        {
            gamecopy.vWalls[walls[m].pos.i][walls[m].pos.j] = WALL_STATE_START;
            gamecopy.vWalls[walls[m].pos.i+1][walls[m].pos.j] = WALL_STATE_END;
        }
        gamecopy.wallCounts[playerID]--; 
        float tmp = QuoridorCore_minMax(&gamecopy, playerID ^ 1, currDepth + 1, maxDepth, alpha, beta, turn);

        if (maximizing)//élagage aplha
        {

            if (tmp >= value)
            {
                value = tmp;
                if (currDepth == 0)
                {
                    best = m;
                    IsWall = 1;

                }
            }
            alpha = value;
            if (alpha > beta)
                break;
        }
        if (!maximizing)//élagage béta
        {
            if (tmp <= value)
                value = tmp;
            beta = value;
            if (beta < alpha)
                break;
        }

    }
    if (IsWall == 0)
    {
        turn->action = QUORIDOR_MOVE_TO;
        turn->i = moves[best].i;
        turn->j = moves[best].j;


    }
    else
    {
        if (walls[best].type == WALL_TYPE_HORIZONTAL)
        {
            turn->action = QUORIDOR_PLAY_HORIZONTAL_WALL;
            turn->i = walls[best].pos.i;
            turn->j = walls[best].pos.j;
        }
        if (walls[best].type == WALL_TYPE_VERTICAL)
        {
            turn->action = QUORIDOR_PLAY_VERTICAL_WALL;
            turn->i = walls[best].pos.i;
            turn->j = walls[best].pos.j;
        }
    }
    ////// Astuce :
    ////// vous devez effectuer toutes vos actions sur une copie du plateau courant.
    ////// Comme la structure QuoridorCore ne contient aucune allocation interne,
    ////// la copie s'éffectue simplement avec :
    ////// QuoridorCore gameCopy = *self;
    free(walls);

    return value;
}

QuoridorTurn QuoridorCore_computeTurn(QuoridorCore* self, int depth, void* aiData)
{
	QuoridorTurn childTurn = { 0 };

	if (self->state != QUORIDOR_STATE_IN_PROGRESS) return childTurn;


  const float alpha = -INFINITY;
  const float beta = INFINITY;
  float childValue = QuoridorCore_minMax(self, self->playerID, 0, 6, alpha, beta, &childTurn);
  return childTurn;

}

int QuoridorCore_getMoves(QuoridorCore* self, QuoridorPos* moves, QuoridorPos pos, int player)
{

    int Id = 0;
    const int gridSize = self->gridSize;
    const int currI = pos.i;
    const int currJ = pos.j;
    const int otherI = self->positions[self->playerID ^ 1].i;
    const int otherJ = self->positions[self->playerID ^ 1].j;



    //ajoute ou non des 4 cases voisines

    if (currI > 0 && !QuoridorCore_hasWallAbove(self, currI, currJ)) // on ajoute au dessus
    {
        if (!(currI - 1 == otherI && currJ == otherJ) || !player)
        {
            moves[Id].i = currI - 1;
            moves[Id++].j = currJ;

        }
            
    }
    if (currI < gridSize - 1 && !QuoridorCore_hasWallBelow(self, currI, currJ)) //on ajoute en dessous
    {
        if (!(currI + 1 == otherI && currJ == otherJ) || !player)
        {
            moves[Id].i = currI + 1;
            moves[Id++].j = currJ;
        }
    }
    if (currJ > 0 && !QuoridorCore_hasWallLeft(self, currI, currJ)) // on ajoute a gauche
    {
        if (!(currI == otherI && currJ - 1 == otherJ) || !player)
        {
            moves[Id].i = currI;
            moves[Id++].j = currJ - 1;
        }

    }
    if (currJ < gridSize - 1 && !QuoridorCore_hasWallRight(self, currI, currJ)) // on ajoute a droite
    {
        if (!(currI == otherI && currJ + 1 == otherJ) || !player)
        {
            moves[Id].i = currI;
            moves[Id++].j = currJ + 1;
        }

    }
    if (!player)
        return Id;

    /// ------------------------------------------------------------------------
    //ajoust ou non des coups du mouton

    if (otherI > 0) // on saute par dessu l'autre joueurs vers le hauts
    {
        if (otherI == currI - 1 && otherJ == currJ)
        {
            if (!QuoridorCore_hasWallAbove(self, currI, currJ) && !QuoridorCore_hasWallAbove(self, otherI, currJ))
            {
                moves[Id].i = otherI - 1;
                moves[Id++].j = currJ;
            }
        }
    }

    if (otherI < gridSize - 1) // on saute par dessu l'autre joueurs vers le bas 
    {
        if (otherI == currI + 1 && otherJ == currJ)
        {
            if (!QuoridorCore_hasWallBelow(self, currI, currJ) && !QuoridorCore_hasWallBelow(self, otherI, currJ))
            {
                moves[Id].i = otherI + 1;
                moves[Id++].j = currJ;
            }
        }
    }

    if (otherJ > 0) // on saute par dessu l'autre joueurs vers la gauche
    {
        if (otherI == currI && otherJ == currJ - 1)
        {
            if (!QuoridorCore_hasWallLeft(self, currI, currJ) && !QuoridorCore_hasWallLeft(self, otherI, otherJ))
            {
                moves[Id].i = currI;
                moves[Id++].j = otherJ - 1;
            }
        }
    }

    if (otherJ < gridSize - 1) // on saute par dessu l'autre joueurs vers la droite 
    {
        if (otherI == currI && otherJ == currJ + 1)
        {
            if (!QuoridorCore_hasWallRight(self, currI, currJ) && !QuoridorCore_hasWallRight(self, otherI, otherJ))
            {
                moves[Id].i = currI;
                moves[Id++].j = otherJ + 1;
            }
        }
    }

    /// --------------------------------------------------------------------


    // ajoute les coups du cheval         

    if (otherI > 0) // on saute par dessu l'autre joueurs du bas vers la gauche ou la droite (asscendant)
    {
        if (otherI == currI - 1 && otherJ == currJ)
        {
            if (QuoridorCore_hasWallAbove(self, otherI, otherJ) && !QuoridorCore_hasWallAbove(self, currI, currJ))
            {
                if (!QuoridorCore_hasWallLeft(self, otherI, otherJ) && otherJ > 0)
                {
                    moves[Id].i = otherI;
                    moves[Id++].j = otherJ - 1;
                }
                if (!QuoridorCore_hasWallRight(self, otherI, otherJ) && otherJ < gridSize - 1)
                {
                    moves[Id].i = otherI;
                    moves[Id++].j = otherJ + 1;
                }
            }
        }
    }

    if (otherI < gridSize - 1) // on saute par dessu l'autre joueurs du haut vers la gauche ou la droite (descendant)
    {
        if (otherI == currI + 1 && otherJ == currJ)
        {
            if (QuoridorCore_hasWallBelow(self, otherI, otherJ) && !QuoridorCore_hasWallBelow(self, currI, currJ))
            {
                if (!QuoridorCore_hasWallLeft(self, otherI, otherJ) && otherJ > 0)
                {
                    moves[Id].i = otherI;
                    moves[Id++].j = otherJ - 1;
                }
                if (!QuoridorCore_hasWallRight(self, otherI, otherJ) && otherJ < gridSize - 1)
                {
                    moves[Id].i = otherI;
                    moves[Id++].j = otherJ + 1;
                }
            }
        }
    }

    if (otherJ > 0) // on saute par dessu l'autre joueurs vers la gauche
    {
        if (otherI == currI && otherJ == currJ - 1)
        {
            if (QuoridorCore_hasWallLeft(self, otherI, otherJ) && !QuoridorCore_hasWallLeft(self, currI, currJ))
            {
                if (!QuoridorCore_hasWallAbove(self, otherI, otherJ) && otherI > 0)
                {
                    moves[Id].i = otherI - 1;
                    moves[Id++].j = otherJ;
                }
                if (!QuoridorCore_hasWallBelow(self, otherI, otherJ) && otherI < gridSize - 1)
                {
                    moves[Id].i = otherI + 1;
                    moves[Id++].j = otherJ;
                }
            }

        }
    }


    if (otherJ < gridSize - 1) // on saute par dessu l'autre joueurs vers la droite
    {
        if (otherI == currI && otherJ == currJ + 1)
        {
            if (QuoridorCore_hasWallRight(self, otherI, otherJ) && !QuoridorCore_hasWallRight(self, currI, currJ))
            {
                if (!QuoridorCore_hasWallAbove(self, otherI, otherJ) && otherI > 0)
                {
                    moves[Id].i = otherI - 1;
                    moves[Id++].j = otherJ;
                }
                if (!QuoridorCore_hasWallBelow(self, otherI, otherJ) && otherI < gridSize - 1)
                {
                    moves[Id].i = otherI + 1;
                    moves[Id++].j = otherJ;
                }
            }
        }
    }
    return Id;
}

///@brief fonction pour comparer des murs grace a leur score de gain necesaire a qsort
int compareWalls(const void* a, const void* b) 
{
	const QuoridorWall* wa = (const QuoridorWall*)a;
	const QuoridorWall* wb = (const QuoridorWall*)b;
	return wb->score - wa->score; // tri décroissant
}

void collectAllWallsNearPath(QuoridorCore* self, QuoridorPos* path, int pathSize, QuoridorWall* candidat, int* candidatCount)
{
	//tableau de 2  boolen pour savoir si les mur ont deja ete vus 
	
	bool seen[MAX_GRID_SIZE][MAX_GRID_SIZE][2] = { false }; // 2 types : horizontal / vertical}  


	int maxCandidat = pathSize * 4; // 4 murs max par case	
	
	*candidatCount = 0; 
	
 
	for (int k = 0; k < pathSize; k++)
	{
		int i = path[k].i;
		int j = path[k].j;

		if (i > 0 && !seen[i - 1][j][WALL_TYPE_HORIZONTAL]) // mur au dessus
		{
			candidat[*candidatCount].pos.i = i - 1;
			candidat[*candidatCount].pos.j = j;
			candidat[*candidatCount].type = WALL_TYPE_HORIZONTAL;
			(*candidatCount)++;
			seen[i - 1][j][WALL_TYPE_HORIZONTAL] = true;
		}
		if (i < self->gridSize - 1 && !seen[i + 1][j][WALL_TYPE_HORIZONTAL]) // mur en dessous
		{
			candidat[*candidatCount].pos.i = i;
			candidat[*candidatCount].pos.j = j;
			candidat[*candidatCount].type = WALL_TYPE_HORIZONTAL;
			(*candidatCount)++;
			seen[i + 1][j][WALL_TYPE_HORIZONTAL] = true;
		}
		if (j > 0 && !seen[i][j - 1][WALL_TYPE_VERTICAL]) // mur a gauche
		{
			candidat[*candidatCount].pos.i = i;
			candidat[*candidatCount].pos.j = j - 1;
			candidat[*candidatCount].type = WALL_TYPE_VERTICAL;
			(*candidatCount)++;
			seen[i][j - 1][WALL_TYPE_VERTICAL] = true;
		}
		if (j < self->gridSize - 1 && !seen[i][j + 1][WALL_TYPE_VERTICAL]) // mur a droite
		{
			candidat[*candidatCount].pos.i = i;
			candidat[*candidatCount].pos.j = j;
			candidat[*candidatCount].type = WALL_TYPE_VERTICAL;
			(*candidatCount)++;
			seen[i][j + 1][WALL_TYPE_VERTICAL] = true; 
		}
	}

	return;
}

void collectFewWallsInFrontOfPath(QuoridorCore* self, QuoridorPos* path, int pathSize, QuoridorWall* candidat, int* candidatCount) //regarde les murs bien orient au debut du chemin
{
	int maxSteps = min(pathSize - 1, 4); // examine les 3 premiere case 
	int maxWalls = 32; // nombre max de murs candidats 

	
	*candidatCount = 0;


	bool seen[MAX_GRID_SIZE][MAX_GRID_SIZE][2] = { false }; // 2 types : horizontal / vertical 


	for (int k = 0; k < maxSteps && *candidatCount < maxWalls; k++) 
	{
		 
		int i = path[k].i; 
		int j = path[k].j; 
		int di = path[k + 1].i - i; 
		int dj = path[k + 1].j - j; 

		// Mouvement vertical doncc candidat mur horizontal
		if (di != 0 && i > 0 && !seen[i - 1][j][WALL_TYPE_HORIZONTAL]) 
		{
			candidat[*candidatCount].type = WALL_TYPE_HORIZONTAL;
			candidat[*candidatCount].pos.i = i - 1;
			candidat[*candidatCount].pos.j = j;
			(*candidatCount)++;
			seen[i - 1][j][WALL_TYPE_HORIZONTAL] = true;
		}

		// Mouvement horizontal donc candidat mur vertical
		if (dj != 0 && j > 0 && !seen[i][j - 1][WALL_TYPE_VERTICAL]) 
		{
			candidat[*candidatCount].type = WALL_TYPE_VERTICAL;
			candidat[*candidatCount].pos.i = i;
			candidat[*candidatCount].pos.j = j - 1;
			(*candidatCount)++;
			seen[i][j - 1][WALL_TYPE_VERTICAL] = true;
		}
	}

	return;
}


void getBestWall(QuoridorCore* self, int player, int tolerance, QuoridorWall* bestWalls,int *wallCount,void *(wichWall)(QuoridorCore*,QuoridorPos*,int,QuoridorWall*,int*))
{

	if(self->wallCounts[player] == 0) // si le joueur n'a plus de mur
		return;
	int nbrList = 0; 


	const int gridSize = self->gridSize;
	const int otherPlayer = player ^ 1;


	int longestEnemyWaySize, longesPlayerWaySize = -1;
	int playerRatio, enemyRatio = 0;
	int gain = 0;
	*wallCount = 0;
	int actualPlayerSize, actualEnemySize = 0;
	//pour collecter les murs a traiter
	int attemptingCount = 0;

	//QuoridorWall* bestWalls = (QuoridorWall*)calloc(sizeof(QuoridorWall), MAX_BEST_WALLS); // on initialise le tableau de 5 murs 

	QuoridorPos* enemyPath = calloc(81, sizeof(QuoridorPos)); // on intialise un chemim maximum vide 
	int enemySize = 0; // on initialise la taille a 0

	QuoridorPos* playerPath = calloc(81, sizeof(QuoridorPos)); // on intialise un chemim maximum vide
	int playerSize = 0; // on initialise la taille a 0


	Graph* playerGraph = QuoridorCore_initGraph(self, player); // on initialise le graphe du joueur en cours 

	Graph* enemyGraph = QuoridorCore_initGraph(self, otherPlayer); // on initialise le graphe de l'autre joueur 

	// QuoridorCore_getShortestPath(self, player, tempPath, &actualPlayerSize, playerGraph); // on recupere la taille du plus court chemin
	AStarShortestPath(self, playerGraph, player, playerPath, &actualPlayerSize);  // on recupere la taille du plus court chemin avec l'algorithme A*
	
	//QuoridorCore_getShortestPath(self, otherPlayer, tempPath, &actualEnemySize, enemyGraph); // on recupere la taille du plus court chemin 

	AStarShortestPath(self, enemyGraph, otherPlayer, enemyPath, &actualEnemySize);   // on recupere la taille du plus court chemin avec l'algorithme A*

	// ==> reupere les murs autour du chemin ennemi
	QuoridorWall attemptingWalls[100]; // ca sera les mur a tester
	wichWall(self, enemyPath, actualEnemySize, &attemptingWalls, &attemptingCount);   

	//update : tester que les murs qui sont pas sur le chemin
	for (int walls = 0; walls < attemptingCount; walls++)
	{
		int i = attemptingWalls[walls].pos.i;
		int j = attemptingWalls[walls].pos.j;
		int type = attemptingWalls[walls].type;

		if (!QuoridorCore_canPlayWall(self, type, i, j)) 
			continue; 

		if (type == WALL_TYPE_HORIZONTAL)
		{
			Graph_removeArc(enemyGraph, i * 9 + j, (i + 1) * 9 + j); //on retire les arc aller retour des deux case 4 cases concernées pour le jouer enemis 
			Graph_removeArc(enemyGraph, (i + 1) * 9 + j, i * 9 + j);
			Graph_removeArc(enemyGraph, i * 9 + (j + 1), (i + 1) * 9 + (j + 1));
			Graph_removeArc(enemyGraph, (i + 1) * 9 + (j + 1), i * 9 + (j + 1));

			Graph_removeArc(playerGraph, i * 9 + j, (i + 1) * 9 + j); //on retire les arc aller retour des deux case 4 cases concernées pour le joueur en cours
			Graph_removeArc(playerGraph, (i + 1) * 9 + j, i * 9 + j);
			Graph_removeArc(playerGraph, i * 9 + (j + 1), (i + 1) * 9 + (j + 1));
			Graph_removeArc(playerGraph, (i + 1) * 9 + (j + 1), i * 9 + (j + 1));

	
		}
		else if (type == WALL_TYPE_VERTICAL) 
		{
			Graph_removeArc(enemyGraph, i * 9 + j, i * 9 + (j + 1)); //on retire les arc aller retour des deux case 4 cases concernées ppur le joueur enemis
			Graph_removeArc(enemyGraph, i * 9 + (j + 1), i * 9 + j);
			Graph_removeArc(enemyGraph, (i + 1) * 9 + j, (i + 1) * 9 + (j + 1));
			Graph_removeArc(enemyGraph, (i + 1) * 9 + (j + 1), (i + 1) * 9 + j);

			Graph_removeArc(playerGraph, i * 9 + j, i * 9 + (j + 1)); //on retire les arc aller retour des deux case 4 cases concernées ppur le joueur en cours
			Graph_removeArc(playerGraph, i * 9 + (j + 1), i * 9 + j);
			Graph_removeArc(playerGraph, (i + 1) * 9 + j, (i + 1) * 9 + (j + 1));
			Graph_removeArc(playerGraph, (i + 1) * 9 + (j + 1), (i + 1) * 9 + j);
		}


		// QuoridorCore_getShortestPath(self, otherPlayer, enemyPath, &enemySize, enenemyGraph);
		AStarShortestPath(self, enemyGraph, otherPlayer, enemyPath, &enemySize);

		//QuoridorCore_getShortestPath(self, player, playerPath, &playerSize, playerGraph);
		AStarShortestPath(self, playerGraph, player, playerPath, &playerSize);



		playerRatio = playerSize - actualPlayerSize; //le nombre de case en + pour le joeur
		enemyRatio = enemySize - actualEnemySize;    // le nombre de cases en + pour l enemy

		gain = playerRatio - enemyRatio;  // on fait la diff des deux ratio pour savoir si rentable

		if (playerRatio < tolerance && enemyRatio > 0)
		{
			if (*wallCount < MAX_BEST_WALLS)
			{
				bestWalls[*wallCount].type = type;
				bestWalls[*wallCount].pos.i = i;
				bestWalls[*wallCount].pos.j = j;
				bestWalls[*wallCount].score = gain;
				(*wallCount)++;
			}
			else // remplace le pire mure si le courant est mieux
			{

				int minIndex = 0;
				for (int k = 1; k < MAX_BEST_WALLS; k++)  // on commence a 1 car on suppose case 0 pour commencer
				{
					if (bestWalls[k].score < bestWalls[minIndex].score)
					{
						minIndex = k;
					}
				}
				if (gain > bestWalls[minIndex].score)
				{
					bestWalls[minIndex].type = type;
					bestWalls[minIndex].pos.i = i;
					bestWalls[minIndex].pos.j = j;
					bestWalls[minIndex].score = gain;

				}

			}

		}
		//on remet les arretes
		if (type == WALL_TYPE_HORIZONTAL)
		{
			Graph_setArc(enemyGraph, i * 9 + j, (i + 1) * 9 + j,1); //on retire les arc aller retour des deux case 4 cases concernées pour le jouer enemis 
			Graph_setArc(enemyGraph, (i + 1) * 9 + j, i * 9 + j,1);
			Graph_setArc(enemyGraph, i * 9 + (j + 1), (i + 1) * 9 + (j + 1),1);
			Graph_setArc(enemyGraph, (i + 1) * 9 + (j + 1), i * 9 + (j + 1),1);

			Graph_setArc(playerGraph, i * 9 + j, (i + 1) * 9 + j,1); //on retire les arc aller retour des deux case 4 cases concernées pour le joueur en cours
			Graph_setArc(playerGraph, (i + 1) * 9 + j, i * 9 + j,1);
			Graph_setArc(playerGraph, i * 9 + (j + 1), (i + 1) * 9 + (j + 1),1);
			Graph_setArc(playerGraph, (i + 1) * 9 + (j + 1), i * 9 + (j + 1),1);

		}
		else if (type == WALL_TYPE_VERTICAL) 
		{
			Graph_setArc(enemyGraph, i * 9 + j, i * 9 + (j + 1),1); //on retire les arc aller retour des deux case 4 cases concernées ppur le joueur enemis
			Graph_setArc(enemyGraph, i * 9 + (j + 1), i * 9 + j,1);
			Graph_setArc(enemyGraph, (i + 1) * 9 + j, (i + 1) * 9 + (j + 1),1);
			Graph_setArc(enemyGraph, (i + 1) * 9 + (j + 1), (i + 1) * 9 + j,1);

			Graph_setArc(playerGraph, i * 9 + j, i * 9 + (j + 1),1); //on retire les arc aller retour des deux case 4 cases concernées ppur le joueur en cours
			Graph_setArc(playerGraph, i * 9 + (j + 1), i * 9 + j,1); 
			Graph_setArc(playerGraph, (i + 1) * 9 + j, (i + 1) * 9 + (j + 1),1); 
			Graph_setArc(playerGraph, (i + 1) * 9 + (j + 1), (i + 1) * 9 + j,1); 
		} 

	}

	if (*wallCount > 1)
		qsort(bestWalls, *wallCount, sizeof(QuoridorWall), compareWalls);


	Graph_destroy(playerGraph); 
	Graph_destroy(enemyGraph); 
	free(enemyPath);
	free(playerPath);
	free(attemptingWalls);
	return;
}

ListQuor* BFS_search(QuoridorCore* self, int playerID)
{
    int tab[MAX_GRID_SIZE][MAX_GRID_SIZE] = { 0 };
    ListQuor* list = ListQuor_create();
    ListQuor* list2 = ListQuor_create();
    ListQuor_insert_last(list, self->positions[playerID],NULL);
    int nbr = 1;
    while (list->head)
    {

        int moveCount = 0;
        QuoridorPos moves[8];
        NodeQuor* node = ListQuor_pop_first(list);
        moveCount = QuoridorCore_getMoves(self, moves, node->pos, nbr);
        ListQuor_insert_last(list2, node->pos, node->parent);

        nbr = 0;
        tab[node->pos.i][node->pos.j] = 1;

        if ((playerID == 0 && node->pos.j % MAX_GRID_SIZE == MAX_GRID_SIZE - 1) ||
            (playerID == 1 && node->pos.j % MAX_GRID_SIZE == 0)) {
            //printf("(%d,%d) %d\n", node->pos.i, node->pos.j, node->pos.j % MAX_GRID_SIZE);
            NodeQuor* node2 = ListQuor_pop_last(list2);
            //printf("2(%d,%d) %d\n", node2->pos.i, node2->pos.j, node2->pos.j % MAX_GRID_SIZE);

            ListQuor_free(list);  // libérer le reste de la file
            return list2;       // on retourne le chemin trouvé
        }

        for (int i = 0; i < moveCount; i++)
        {
            if(tab[moves[i].i][moves[i].j] == 0)
                ListQuor_insert_last(list,moves[i], node);
        }
    }
    return NULL;
}
int BFS_search2(QuoridorCore* self, int playerID, QuoridorPos* tab)
{
	int* parent = calloc(self->gridSize * self->gridSize, sizeof(int));
	int* explored = calloc(self->gridSize * self->gridSize, sizeof(int));
	tab[0] = self->positions[playerID];
	explored[self->positions[playerID].i * self->gridSize + self->positions[playerID].j] = 1;

	parent[0] = -1;
	int start = 0, end = 1;
	int nbr = 1;

	int size = 0;

	while (start < end)
	{
		int moveCount = 0;
		QuoridorPos moves[8];

		QuoridorPos position = tab[start++];

		moveCount = QuoridorCore_getMoves(self, moves, position, nbr);
		nbr = 0;

		int tmp = start - 1;
		if ((playerID == 0 && position.j == self->gridSize - 1) ||
			(playerID == 1 && position.j == 0))
		{
			while (tmp != -1)
			{
				size++;
				tmp = parent[tmp];
			}

			free(parent);
			free(explored);
			return size; // Retourne la taille du chemin trouvé
		}

		for (int i = 0; i < moveCount; i++)
		{
			if (explored[moves[i].i * self->gridSize + moves[i].j] == 0)
			{
				tab[end] = moves[i];
				parent[end++] = start - 1;
				explored[moves[i].i * self->gridSize + moves[i].j] = 1;
			}
		}
	}

	free(parent);
	free(explored);
	return -1; // Retourne -1 si aucun chemin n'est trouvé
}
