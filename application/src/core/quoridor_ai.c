/*
	Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
	Licensed under the MIT License.
	See LICENSE.md in the project root for license information.
*/

#include "core/quoridor_core.h"
#include "core/utils.h"
#include "core/quoridor_ai.h"
#include "limits.h"

#define MAX_BEST_WALLS 3

ListData* AIData_create()
{
	QuoridorData clear;
	memset(&clear, 0, sizeof(QuoridorData)); 

	ListData* data = ListData_create();

	for (int i = 0; i < MAX_BACK_ANALYS; i++)
		ListData_insertFirst(data, clear);

	return data;
}

void AIData_destroy(void *data)
{
	ListData* database = (ListData*)data;
	ListData_destroy(database);
	return;
}


void AIData_reset(void* data)
{
	ListData* database = (ListData*)data;
	QuoridorData clear;
	memset(&clear, 0, sizeof(QuoridorData)); // on remet a zero le coup 

	for (int i = 0; i < MAX_BACK_ANALYS; i++)
		ListData_insertFirstPopLast(database, clear); 

	return;
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
static float QuoridorCore_computeScore(QuoridorCore* self, int playerID, void * aiData)
{
	int playerA = self->playerID;
	int playerB = self->playerID ^ 1;

	int distA = 0;
	int distB = 0;

	QuoridorPos playerApath[MAX_GRID_SIZE * MAX_GRID_SIZE];
	QuoridorPos playerBpath[MAX_GRID_SIZE * MAX_GRID_SIZE];
	
	distA = BFS_search2(self, playerA, playerApath);
	distB = BFS_search2(self, playerB, playerBpath);

	int wallsA = self->wallCounts[playerA];
	int wallsB = self->wallCounts[playerB];
	float score = (float)(distB - distA);
	


	// 3. Ajouter un bruit aléatoire pour casser les égalités (pas obligatoire mais pratique)
    score += Float_randAB(-0.1f, +0.1f); 
	return score;
}


float isTheMoveWorth(int i, int j, void* aiData)
{
	ListData* data = (ListData*)aiData;
	NodeData* current = data->head;

	int count = 0;
	for (int k = 0; k < MAX_BACK_ANALYS; k++)
	{
		if (current->data.action == QUORIDOR_MOVE_TO)
		{
			if (current->data.pos.i == i && current->data.pos.j == j)
			{
				count++;
			}
		}
	}
	
	if (count >= 2)
	{
		switch (count)
		{
		case 2 : 
			return -1;
		case 3 :
			return -2;
		case 4 :
			return -4;
		default:
			return -5;
		}
	}

	return 0;
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
static float QuoridorCore_minMax(QuoridorCore* self, int playerID, int currDepth, int maxDepth, float alpha, float beta, QuoridorTurn* turn, void* aiData)
{


    if (currDepth >= maxDepth) //si on atteint le profondzeur max
    {
		int score = QuoridorCore_computeScore(self, playerID,aiData);  
		

		/*if (turn->action == QUORIDOR_MOVE_TO)
			score += isTheMoveWorth(turn->i, turn->j, aiData);*/
		
		return score;
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
	QuoridorPos moves[8] = { 0 };
    moveCount = QuoridorCore_getMoves(self, moves, self->positions[playerID], 1); 
	//printf("%d %d\n", playerID, self->playerID);
    for (int k = 0; k < moveCount; k++) //mouvements
    {
        QuoridorCore copy = *self;
        copy.positions[playerID].i = moves[k].i;
        copy.positions[playerID].j = moves[k].j;
		turn->action = QUORIDOR_MOVE_TO;
		turn->i = copy.positions[playerID].i;
		turn->j = copy.positions[playerID].j;



        float tmp = QuoridorCore_minMax(&copy, playerID ^ 1, currDepth + 1, maxDepth, alpha, beta, turn, aiData);

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
			if (alpha > beta)
				return value;
			if (value > alpha)
				alpha = value;

        }
        if (!maximizing) //élagage béta
        {
            if (tmp <= value)
                value = tmp;
			if (value < alpha)
				return value;
			if (value < beta)
				beta = value;
        }
    }
	QuoridorWall walls[MAX_BEST_WALLS] = { 0 };  //on initialise le tableau de 5 murs 
	
	int wallCount = 0;
    //getBestWall(self, playerID, 999, walls,&wallCount,collectAllWallsNearPath);  
	 

	getBestWall(self, playerID, 999, walls,&wallCount); //on recupere les meilleurs murs a jouer collectFewWallsInFrontOfPath collectAllWallsNearPath

		
    for (int m = 0; m < wallCount; m++) //murs 
    {
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
        float tmp = QuoridorCore_minMax(&gamecopy, playerID ^ 1, currDepth + 1, maxDepth, alpha, beta, turn, aiData);

		if (maximizing) //élégage aplha
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
			if (alpha > beta)
				return value;
			if (value > alpha)
				alpha = value;

		}
		if (!maximizing) //élagage béta
		{
			if (tmp <= value)
				value = tmp;
			if (value < alpha)
				return value;
			if (value < beta)
				beta = value;
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


    return value;
}

QuoridorTurn QuoridorCore_computeTurn(QuoridorCore* self, int depth, void* aiData)
{
	QuoridorTurn childTurn = { 0 };

	if (self->state != QUORIDOR_STATE_IN_PROGRESS) return childTurn;


  const float alpha = -INFINITY;
  const float beta = INFINITY;



  ListData* database;

  float childValue = QuoridorCore_minMax(self, self->playerID, 0, 4, alpha, beta, &childTurn,aiData);



  QuoridorData turn;
  
  turn.action = childTurn.action;
  turn.pos.i = childTurn.i;
  turn.pos.j = childTurn.j;
  turn.score = 0; //0 pour l'instant 
  
  AIData_add((ListData*)aiData, turn);
	

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
	short int nbrOfcaseToAnalysis = 4; // nombre de case a analyser
	int maxSteps = (pathSize - 1) < nbrOfcaseToAnalysis ? (pathSize - 1) : nbrOfcaseToAnalysis; // examine les 4 premiere case 
	int maxWalls = 32; // nombre max de murs candidats 

	
	*candidatCount = 0;


	bool seen[MAX_GRID_SIZE][MAX_GRID_SIZE][2] = { false }; // 2 types : horizontal / vertical 


	for (int k = 0; k < maxSteps && *candidatCount < maxWalls; k++) 
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

		 
		//int i = path[k].i; 
		//int j = path[k].j; 
		//int di = path[k + 1].i - i; 
		//int dj = path[k + 1].j - j; 

		//// Mouvement vertical doncc candidat mur horizontal
		//if (di != 0 && i > 0 && !seen[i - 1][j][WALL_TYPE_HORIZONTAL]) 
		//{
		//	candidat[*candidatCount].type = WALL_TYPE_HORIZONTAL;
		//	candidat[*candidatCount].pos.i = i - 1;
		//	candidat[*candidatCount].pos.j = j;
		//	(*candidatCount)++;
		//	seen[i - 1][j][WALL_TYPE_HORIZONTAL] = true;
		//}

		//// Mouvement horizontal donc candidat mur vertical
		//if (dj != 0 && j > 0 && !seen[i][j - 1][WALL_TYPE_VERTICAL]) 
		//{
		//	candidat[*candidatCount].type = WALL_TYPE_VERTICAL;
		//	candidat[*candidatCount].pos.i = i;
		//	candidat[*candidatCount].pos.j = j - 1;
		//	(*candidatCount)++;
		//	seen[i][j - 1][WALL_TYPE_VERTICAL] = true;
		//}
	}

	return;
}

void getBestWall(QuoridorCore* self, int player, int tolerance, QuoridorWall* bestWalls,int *wallCount)
{

	if(self->wallCounts[player] == 0) // si le joueur n'a plus de mur
		return;
	int nbrList = 0; 


	const int gridSize = self->gridSize;
	const int otherPlayer = player ^ 1;


	int longestEnemyWaySize = 0, longesPlayerWaySize = -1;
	int playerRatio, enemyRatio = 0;
	int gain = 0;
	*wallCount = 0;
	int actualPlayerSize, actualEnemySize = 0;
	//pour collecter les murs a traiter
	int attemptingCount = 0;

	//QuoridorWall* bestWalls = (QuoridorWall*)calloc(sizeof(QuoridorWall), MAX_BEST_WALLS); // on initialise le tableau de 5 murs 

	int enemySize = 0; // on initialise la taille a 0

	int playerSize = 0; // on initialise la taille a 0


	QuoridorPos playerPath[MAX_GRID_SIZE * MAX_GRID_SIZE];
	QuoridorPos enemyPath[MAX_GRID_SIZE * MAX_GRID_SIZE];


	actualPlayerSize = BFS_search2(self, player, playerPath);
	actualEnemySize = BFS_search2(self, otherPlayer, enemyPath);

	// ==> reupere les murs autour du chemin ennemi

	QuoridorWall attemptingWalls[100]; // ca sera les mur a tester

	collectFewWallsInFrontOfPath(self, enemyPath, actualEnemySize, attemptingWalls, &attemptingCount);  

	QuoridorCore copy = *self;

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
			copy.hWalls[i][j] = WALL_STATE_START;
			copy.hWalls[i][j+1] = WALL_STATE_END;
		}
		else if (type == WALL_TYPE_VERTICAL) 
		{
			copy.vWalls[i][j] = WALL_STATE_START;
			copy.vWalls[i + 1][j] = WALL_STATE_END;
		}
		playerSize = BFS_search2(&copy, player, playerPath);
		enemySize = BFS_search2(&copy, otherPlayer, enemyPath);

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
		copy = *self;
	}

	if (*wallCount > 1)
		qsort(bestWalls, *wallCount, sizeof(QuoridorWall), compareWalls);

	return;
}

int BFS_search2(QuoridorCore* self, int playerID, QuoridorPos* tab)
{
	    int gridSize = self->gridSize;
    int front = 0, back = 1;
    int visited[MAX_GRID_SIZE][MAX_GRID_SIZE] = { 0 };
    int distance = 0;

    QuoridorPos predecessor[MAX_GRID_SIZE][MAX_GRID_SIZE] = { -1 };
	for(int i = 0;i<MAX_GRID_SIZE;i++)
		for (int j = 0; j < MAX_GRID_SIZE; j++)
		{
			predecessor[i][j].i = -1;
			predecessor[i][j].j = -1;
		}

    QuoridorPos queue[MAX_PATH_LEN * 2];
    QuoridorPos initalPos = self->positions[playerID];
    queue[0] = self->positions[playerID];
    visited[self->positions[playerID].i][self->positions[playerID].j] = true;
     
   while (front < back) {
       QuoridorPos current = queue[front];
       front++;

       int i = current.i;
       int j = current.j;
       
       if (!QuoridorCore_hasWallAbove(self, i, j) && i>0 && !visited[i-1][j]) {
           queue[back].i = i-1;
           queue[back].j = j;
           predecessor[i - 1][j] = current;
           visited[i - 1][j] = 1;
           back++;
       }
       if (!QuoridorCore_hasWallBelow(self, i, j) && i < MAX_GRID_SIZE - 1 && !visited[i + 1][j]) {
           queue[back].i = i + 1;
           queue[back].j = j;
           predecessor[i + 1][j] = current;
           visited[i + 1][j] = 1;
           back++;
       }
       if (!QuoridorCore_hasWallLeft(self, i, j) && j > 0 && !visited[i][j - 1]) {
           queue[back].i = i;
           queue[back].j = j - 1;
           predecessor[i][j - 1] = current;
           visited[i][j - 1] = 1;
           back++;
       }
       if (!QuoridorCore_hasWallRight(self, i, j) && j < MAX_GRID_SIZE - 1 && !visited[i][j + 1]) {
           queue[back].i = i;
           queue[back].j = j + 1;
           predecessor[i][j + 1] = current;
           visited[i][j + 1] = 1;
           back++;
       }

       if ((current.j == 0 && playerID == 1) || (playerID == 0 && current.j == gridSize - 1)) {
           QuoridorPos temp = current;
           for (int i = 0; i < MAX_PATH_LEN; i++) {

               temp = predecessor[temp.i][temp.j];
               distance++;
               if (temp.i == -1 && temp.j == -1)
                   break;
           }
		   temp = current;
		   for (int i = distance - 1; i >= 0; i--) {
			   tab[i] = temp;
			   temp = predecessor[temp.i][temp.j];


		   }
		   return distance;
       }
   }
	return -1; // Retourne -1 si aucun chemin n'est trouvé
}
