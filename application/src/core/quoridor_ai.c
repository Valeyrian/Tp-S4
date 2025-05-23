/*
	Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
	Licensed under the MIT License.
	See LICENSE.md in the project root for license information.
*/

#include "core/quoridor_core.h"
#include "core/utils.h"
#include "core/quoridor_ai.h"
#include "limits.h"




ListData* AIData_create()
{
	QuoridorData clear;
	memset(&clear, 0, sizeof(QuoridorData));

	ListData* data = ListData_create();

	for (int i = 0; i < MAX_BACK_ANALYS; i++)
		ListData_insertFirst(data, clear);

	return data;
}

void AIData_destroy(void* data)
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
void QuoridorCore_getShortestPath(QuoridorCore* self, int playerID, QuoridorPos* path, int* size)
{			
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
	int playerA = playerID;
	int playerB = playerID ^ 1;

#ifdef FOURPLAYERS
	playerB = (playerID + 1) % 4;
	int playerC = (playerID + 2) % 4;
	int playerD = (playerID + 3) % 4;
	QuoridorPos playerCpath[MAX_GRID_SIZE * MAX_GRID_SIZE];
	QuoridorPos playerDpath[MAX_GRID_SIZE * MAX_GRID_SIZE];
#endif
	int distA = 0;
	int distB = 0;
	int distC = 0;
	int distD = 0;


	QuoridorPos playerApath[MAX_GRID_SIZE * MAX_GRID_SIZE];
	QuoridorPos playerBpath[MAX_GRID_SIZE * MAX_GRID_SIZE];



#ifndef A_STAR 
	distA = BFS_search2(self, playerA, playerApath);
	distB = BFS_search2(self, playerB, playerBpath);
#else
	distA = AStar_search(self, playerA, playerApath);
	distB = AStar_search(self, playerB, playerBpath);
#endif
	float score = (float)(distB - distA);

#ifdef FOURPLAYERS
#ifndef A_STAR 
	distC = BFS_search2(self, playerC, playerCpath);
	distD = BFS_search2(self, playerD, playerDpath);
#else
	distC = AStar_search(self, playerC, playerCpath);
	distD = AStar_search(self, playerD, playerDpath);
#endif
	score = (float)(distB + distC + distD - distA);
#endif // FOURPLAYERS


	score += Float_randAB(-RAND_VALUE, +RAND_VALUE);


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
			if (current->data.destPos.i == i && current->data.destPos.j == j)
			{
				count++;
			}
		}
	}
	printf("%d , (%d,%d)\n", count, i, j);
	if (count >= 2)
	{
		switch (count)
		{
		case 2:
			return -1;
		case 3:
			return -2;
		case 4:
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
static float QuoridorCore_minMax(QuoridorCore* self, int playerID, int currDepth, int maxDepth, float alpha, float beta, QuoridorTurn* turn, void* aiData, int isAMoveTo)
{

#ifndef FOURPLAYERS
	if (self->state != QUORIDOR_STATE_IN_PROGRESS) //si un des joueurs a gagné
	{
		if (self->state == QUORIDOR_STATE_P0_WON && playerID == 0)
			return 1000.f - currDepth;
		if (self->state == QUORIDOR_STATE_P1_WON && playerID == 0)
			return -1000.f + currDepth * 10;
		if (self->state == QUORIDOR_STATE_P0_WON && playerID == 1)
			return -1000.f + currDepth * 10;
		if (self->state == QUORIDOR_STATE_P1_WON && playerID == 1)
			return 1000.f - currDepth;
	}
#endif // FOURPLAYERS


	if (currDepth >= maxDepth) //si on atteint la profondeur max
	{

		float score = QuoridorCore_computeScore(self, playerID);
		//if (isAMoveTo)
		//    score += isTheMoveWorth(self->positions[playerID].i, self->positions[playerID].j, aiData);
		return score;

	}

	const int gridSize = self->gridSize;
	const int currI = self->positions[self->playerID].i;
	const int currJ = self->positions[self->playerID].j;
	QuoridorTurn childTurn = { 0 };

	const bool maximizing = (currDepth % 2) == 0;
	float value = maximizing ? -INFINITY : INFINITY;

	// Try moving to valid positions
	for (int i = 0; i < MAX_GRID_SIZE; i++)
	{
		for (int j = 0; j < MAX_GRID_SIZE; j++)
		{
			if (QuoridorCore_canMoveTo(self, i, j))
			{
				QuoridorCore copy = *self;

				/*QuoridorCore* copy = calloc(1, sizeof(QuoridorCore));

				if (copy == NULL) {
					fprintf(stderr, "Erreur d'allocation mémoire\n");
					exit(EXIT_FAILURE);
				}

				memcpy(copy, self, sizeof(QuoridorCore));*/

				QuoridorCore_moveTo(&copy, i, j);
				float tmp = QuoridorCore_minMax(&copy, playerID, currDepth + 1, maxDepth, alpha, beta, &childTurn, aiData, 1);

				//free(copy);

				if (maximizing)
				{
					if (tmp > value)
					{
						value = tmp;
						turn->action = QUORIDOR_MOVE_TO;
						turn->i = i;
						turn->j = j;
					}

					if (value >= beta)
						return value;
					alpha = fmaxf(alpha, value);
				}
				else
				{

					if (tmp < value)
					{
						value = tmp;
						turn->action = QUORIDOR_MOVE_TO;
						turn->i = i;
						turn->j = j;
					}

					// Alpha-beta pruning
					if (value <= alpha)
						return value; // Prune - opponent won't allow this branch
					beta = fminf(beta, value);
				}
			}
		}
	}

	// Try placing walls
	QuoridorWall walls[MAX_BEST_WALLS] = { 0 };  // Initialize array of 5 walls
	int wallCount = 0;
	getBestWall(self, playerID, 999, walls, &wallCount); // Get best walls to play

	for (int m = 0; m < wallCount; m++)
	{
		QuoridorCore gamecopy = *self;

		//QuoridorCore* gamecopy = calloc(1, sizeof(QuoridorCore));
		//if (gamecopy == NULL) {
		//    fprintf(stderr, "Erreur d'allocation mémoire\n");
		//    exit(EXIT_FAILURE);
		//}
		//memcpy(gamecopy, self, sizeof(QuoridorCore)); 

		QuoridorCore_playWall(&gamecopy, walls[m].type, walls[m].pos.i, walls[m].pos.j);

		float tmp = QuoridorCore_minMax(&gamecopy, playerID, currDepth + 1, maxDepth, alpha, beta, &childTurn, aiData, 0);

		//free(gamecopy);

		if (maximizing)
		{
			if (tmp > value)
			{
				value = tmp;
				if (walls[m].type == WALL_TYPE_HORIZONTAL)
					turn->action = QUORIDOR_PLAY_HORIZONTAL_WALL;
				else if (walls[m].type == WALL_TYPE_VERTICAL)
					turn->action = QUORIDOR_PLAY_VERTICAL_WALL;
				turn->i = walls[m].pos.i;
				turn->j = walls[m].pos.j;
			}

			// Alpha-beta pruning
			if (value >= beta)
				return value; // Prune - opponent won't allow this branch
			alpha = fmaxf(alpha, value);
		}
		else
		{
			if (tmp < value)
			{
				value = tmp;
				if (walls[m].type == WALL_TYPE_HORIZONTAL)
					turn->action = QUORIDOR_PLAY_HORIZONTAL_WALL;
				else if (walls[m].type == WALL_TYPE_VERTICAL)
					turn->action = QUORIDOR_PLAY_VERTICAL_WALL;
				turn->i = walls[m].pos.i;
				turn->j = walls[m].pos.j;
			}

			// Alpha-beta pruning
			if (value <= alpha)
				return value; // Prune - opponent won't allow this branch
			beta = fminf(beta, value);
		}
	}

	return value;
}

QuoridorTurn QuoridorCore_computeTurn(QuoridorCore* self, int depth, void* aiData)
{
	QuoridorTurn childTurn = { 0 };


	const float alpha = -INFINITY;
	const float beta = INFINITY;



#ifdef FOURPLAYERS

	if (self->wallCounts[self->playerID] == 0) 
	{
		QuoridorPos path[MAX_PATH_LEN];


#ifndef A_STAR 
		BFS_search2(self, self->playerID, path);
#else 
		AStar_search(self, self->playerID, path);
#endif
		if (self->isValid[path[1].i][path[1].j])
		{
			childTurn.i = path[1].i;
			childTurn.j = path[1].j;
			childTurn.action = QUORIDOR_MOVE_TO;
			return childTurn;
		}
	}
#endif // FOURPLAYERS

	ListData* database;
	QuoridorData turn;  


	float childValue = QuoridorCore_minMax(self, self->playerID, 0, DEPTH_MAX, alpha, beta, &childTurn, aiData, 0);

	turn.action = childTurn.action;
	turn.destPos.i = childTurn.i;
	turn.destPos.j = childTurn.j;
	turn.score = 0; //0 pour l'instant 


	//	AIData_add((ListData*)aiData, turn);


	return childTurn;
}
	

	int QuoridorCore_getMoves(QuoridorCore * self, QuoridorPos * moves, QuoridorPos pos, int player)
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


	void collectAllWall(QuoridorCore * self, QuoridorPos * path, int pathSize, QuoridorWall * candidat, int* candidatCount)
	{
		// Vérifications des paramètres d'entrée
		assert(self != NULL && "self ne doit pas être NULL");
		assert(path != NULL && "path ne doit pas être NULL");
		assert(candidat != NULL && "candidat ne doit pas être NULL");
		assert(candidatCount != NULL && "candidatCount ne doit pas être NULL");
		assert(pathSize >= 0 && pathSize <= MAX_GRID_SIZE * MAX_GRID_SIZE && "pathSize doit être dans une plage valide");

		// Tableau pour suivre les murs déjà vus
		bool seen[MAX_GRID_SIZE][MAX_GRID_SIZE][2];
		memset(seen, 0, sizeof(seen)); // Initialiser le tableau à false

		*candidatCount = 0;

		// Stocker la taille de la grille localement
		int gridSize = self->gridSize;
		assert(gridSize > 0 && gridSize <= MAX_GRID_SIZE && "gridSize doit être valide");

		int caseCount = gridSize * gridSize;

		for (int i = 0; i < gridSize - 1; i++)
		{
			for (int j = 0; j < gridSize - 1; j++)
			{
				for (int k = 0; k < 2; k++)
				{
					if (k == WALL_TYPE_HORIZONTAL)
					{
						assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
						QuoridorWall* currentCandidat = &candidat[*candidatCount];
						currentCandidat->pos.i = i;
						currentCandidat->pos.j = j;
						currentCandidat->type = WALL_TYPE_HORIZONTAL;
						(*candidatCount)++;
					}
					else
					{
						assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
						QuoridorWall* currentCandidat = &candidat[*candidatCount];
						currentCandidat->pos.i = i;
						currentCandidat->pos.j = j;
						currentCandidat->type = WALL_TYPE_VERTICAL;
						(*candidatCount)++;
					}
				}
			}
		}
	}

	void collectAllWallsNearPath(QuoridorCore * self, QuoridorPos * path, int pathSize, QuoridorWall * candidat, int* candidatCount)
	{
		// Vérifications des paramètres d'entrée
		assert(self != NULL && "self ne doit pas être NULL");
		assert(path != NULL && "path ne doit pas être NULL");
		assert(candidat != NULL && "candidat ne doit pas être NULL");
		assert(candidatCount != NULL && "candidatCount ne doit pas être NULL");
		assert(pathSize >= 0 && pathSize <= MAX_GRID_SIZE * MAX_GRID_SIZE && "pathSize doit être dans une plage valide");

		// Tableau pour suivre les murs déjà vus
		bool seen[MAX_GRID_SIZE][MAX_GRID_SIZE][2];
		memset(seen, 0, sizeof(seen)); // Initialiser le tableau à false

		*candidatCount = 0;

		// Stocker la taille de la grille localement
		int gridSize = self->gridSize;
		assert(gridSize > 0 && gridSize <= MAX_GRID_SIZE && "gridSize doit être valide");




		for (int k = 0; k < pathSize && *candidatCount < MAX_CANDIDATES; k++)
		{
			// Vérifier que les coordonnées sont valides
			int i = path[k].i;
			int j = path[k].j;
			assert(i >= 0 && i < gridSize && "i doit être dans les limites de la grille");
			assert(j >= 0 && j < gridSize && "j doit être dans les limites de la grille");

			// Vérifier les murs horizontaux avec toutes les vérifications nécessaires
			if (i > 0 && j < gridSize && !seen[i - 1][j][WALL_TYPE_HORIZONTAL]) // mur au-dessus
			{


				assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
				QuoridorWall* currentCandidat = &candidat[*candidatCount];
				currentCandidat->pos.i = i - 1;
				currentCandidat->pos.j = j;
				currentCandidat->type = WALL_TYPE_HORIZONTAL;
				(*candidatCount)++;
				seen[i - 1][j][WALL_TYPE_HORIZONTAL] = true;
			}

			if (i > 0 && j > 0 && j - 1 < gridSize && !seen[i - 1][j - 1][WALL_TYPE_HORIZONTAL]) // mur au-dessus (gauche)
			{

				assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
				QuoridorWall* currentCandidat = &candidat[*candidatCount];
				currentCandidat->pos.i = i - 1;
				currentCandidat->pos.j = j - 1;
				currentCandidat->type = WALL_TYPE_HORIZONTAL;
				(*candidatCount)++;
				seen[i - 1][j - 1][WALL_TYPE_HORIZONTAL] = true;
			}

			if (i < gridSize - 1 && j < gridSize && !seen[i][j][WALL_TYPE_HORIZONTAL]) // mur en dessous
			{

				assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
				QuoridorWall* currentCandidat = &candidat[*candidatCount];
				currentCandidat->pos.i = i;
				currentCandidat->pos.j = j;
				currentCandidat->type = WALL_TYPE_HORIZONTAL;
				(*candidatCount)++;
				seen[i][j][WALL_TYPE_HORIZONTAL] = true;
			}

			if (i < gridSize - 1 && j > 0 && j - 1 < gridSize && !seen[i][j - 1][WALL_TYPE_HORIZONTAL]) // mur en dessous (gauche)
			{

				assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
				QuoridorWall* currentCandidat = &candidat[*candidatCount];
				currentCandidat->pos.i = i;
				currentCandidat->pos.j = j - 1;
				currentCandidat->type = WALL_TYPE_HORIZONTAL;
				(*candidatCount)++;
				seen[i][j - 1][WALL_TYPE_HORIZONTAL] = true;
			}

			// Vérifier les murs verticaux
			if (j > 0 && i < gridSize && !seen[i][j - 1][WALL_TYPE_VERTICAL]) // mur à gauche
			{

				assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
				QuoridorWall* currentCandidat = &candidat[*candidatCount];
				currentCandidat->pos.i = i;
				currentCandidat->pos.j = j - 1;
				currentCandidat->type = WALL_TYPE_VERTICAL;
				(*candidatCount)++;
				seen[i][j - 1][WALL_TYPE_VERTICAL] = true;
			}

			if (j > 0 && i > 0 && i - 1 < gridSize && !seen[i - 1][j - 1][WALL_TYPE_VERTICAL]) // mur à gauche (haut)
			{

				assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
				QuoridorWall* currentCandidat = &candidat[*candidatCount];
				currentCandidat->pos.i = i - 1;
				currentCandidat->pos.j = j - 1;
				currentCandidat->type = WALL_TYPE_VERTICAL;
				(*candidatCount)++;
				seen[i - 1][j - 1][WALL_TYPE_VERTICAL] = true;
			}

			if (j < gridSize - 1 && i < gridSize && !seen[i][j][WALL_TYPE_VERTICAL]) // mur à droite
			{

				assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
				QuoridorWall* currentCandidat = &candidat[*candidatCount];
				currentCandidat->pos.i = i;
				currentCandidat->pos.j = j;
				currentCandidat->type = WALL_TYPE_VERTICAL;
				(*candidatCount)++;
				seen[i][j][WALL_TYPE_VERTICAL] = true;
			}

			if (j < gridSize - 1 && i > 0 && i - 1 < gridSize && !seen[i - 1][j][WALL_TYPE_VERTICAL]) // mur à droite (haut)
			{

				assert(*candidatCount < MAX_CANDIDATES && "Dépassement du nombre maximal de candidats");
				QuoridorWall* currentCandidat = &candidat[*candidatCount];
				currentCandidat->pos.i = i - 1;
				currentCandidat->pos.j = j;
				currentCandidat->type = WALL_TYPE_VERTICAL;
				(*candidatCount)++;
				seen[i - 1][j][WALL_TYPE_VERTICAL] = true;
			}
		}

		// Vérification finale
		assert(*candidatCount <= MAX_CANDIDATES && "Trop de candidats détectés");
	}


	void getBestWall(QuoridorCore * self, int player, int tolerance, QuoridorWall * bestWalls, int* wallCount)
	{

		if (self->wallCounts[player] == 0) // si le joueur n'a plus de mur
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


		int enemySize = 0; // on initialise la taille a 0

		int playerSize = 0; // on initialise la taille a 0


		QuoridorPos playerPath[MAX_GRID_SIZE * MAX_GRID_SIZE];
		QuoridorPos enemyPath[MAX_GRID_SIZE * MAX_GRID_SIZE];




#ifndef A_STAR
		actualPlayerSize = BFS_search2(self, player, playerPath);
		actualEnemySize = BFS_search2(self, otherPlayer, enemyPath);
#else // !A_STAR
		actualPlayerSize = AStar_search(self, player, playerPath);
		actualEnemySize = AStar_search(self, otherPlayer, enemyPath);
#endif

		// ==> reupere les murs autour du chemin ennemi

		QuoridorWall attemptingWalls[MAX_CANDIDATES]; // ca sera les mur a tester

#ifdef ALL_WALLS

		//collectAllWallsNearPath(self, enemyPath, actualEnemySize, attemptingWalls, &attemptingCount);
		collectAllWallsNearPath(self, enemyPath, actualEnemySize, attemptingWalls, &attemptingCount);
#else
		collectFewWallsInFrontOfPath(self, enemyPath, actualEnemySize, attemptingWalls, &attemptingCount);
#endif 



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
				copy.hWalls[i][j + 1] = WALL_STATE_END;
			}
			else if (type == WALL_TYPE_VERTICAL)
			{
				copy.vWalls[i][j] = WALL_STATE_START;
				copy.vWalls[i + 1][j] = WALL_STATE_END;
			}



#ifndef A_STAR
			playerSize = BFS_search2(&copy, player, playerPath);
			enemySize = BFS_search2(&copy, otherPlayer, enemyPath);
#else
			playerSize = AStar_search(&copy, player, playerPath);
			enemySize = AStar_search(&copy, otherPlayer, enemyPath);
#endif



			playerRatio = playerSize - actualPlayerSize; //le nombre de case en + pour le joeur
			enemyRatio = enemySize - actualEnemySize;    // le nombre de cases en + pour l enemy

			gain = enemyRatio - playerRatio;  // on fait la diff des deux ratio pour savoir si rentable

			if (playerRatio < tolerance && enemyRatio > 0 && gain > 0)
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

	int BFS_search2(QuoridorCore * self, int playerID, QuoridorPos * tab)
	{
		int gridSize = self->gridSize;
		int front = 0, back = 1;
		int visited[MAX_GRID_SIZE][MAX_GRID_SIZE] = { 0 };
		int distance = 0;

		QuoridorPos predecessor[MAX_GRID_SIZE][MAX_GRID_SIZE] = { -1 };
		for (int i = 0; i < MAX_GRID_SIZE; i++)
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

			if (!QuoridorCore_hasWallAbove(self, i, j) && i > 0 && !visited[i - 1][j]) {
				queue[back].i = i - 1;
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


			if ((current.j == 0 && playerID == 1) || (playerID == 0 && current.j == gridSize - 1) || (playerID == 2 && current.i == gridSize - 1) || (playerID == 3 && current.i == 0)) {

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
		printf("bruhh %d %d\n", self->positions[playerID].i, self->positions[playerID].j);
		return -1; // Retourne -1 si aucun chemin n'est trouvé

	}


	int StarHeuristique(QuoridorCore * self, node * node, int playerId)
	{
		int gridsize = self->gridSize;
		// Distance brute jusqu'à la ligne d’arrivée
		int distance;
		if (playerId == 0)
		{
			distance = gridsize - 1 - node->pos.j;
		}
		else if (playerId == 1)
		{
			distance = node->pos.j;
		}
		else if (playerId == 2)
		{
			distance = gridsize - 1 - node->pos.i;
		}
		else if (playerId == 3)
		{
			distance = node->pos.i;
		}
		else
			return -1; // Erreur : ID de joueur invalide





		int centerPenalty = abs(node->pos.i - gridsize / 2); // idee malus si on est loin du centre

		distance = distance * 2 + centerPenalty; // ponderation 
		return  distance;
	}



	bool isWallBetween(QuoridorCore * self, QuoridorPos depart, QuoridorPos arrive)
	{
		if (depart.i == arrive.i && depart.j == arrive.j)
			return false; // même case


		if (depart.i > arrive.i) {

			return QuoridorCore_hasWallAbove(self, depart.i, depart.j);
		}
		else if (depart.i < arrive.i) {

			return QuoridorCore_hasWallBelow(self, depart.i, depart.j);
		}


		if (depart.j > arrive.j) {

			return QuoridorCore_hasWallLeft(self, depart.i, depart.j);
		}
		else if (depart.j < arrive.j) {

			return QuoridorCore_hasWallRight(self, depart.i, depart.j);
		}

		return false;
	}

	int AStar_search(QuoridorCore * self, int playerID, QuoridorPos * outPath)
	{
		QuoridorPos start = self->positions[playerID];
		int gridSize = self->gridSize;

		// Objectif : atteindre n'importe quelle ligne (haut ou bas selon le joueur)

		node nodes[MAX_GRID_SIZE][MAX_GRID_SIZE];
		memset(nodes, 0, sizeof(nodes));


		for (int i = 0; i < gridSize; i++)
			for (int j = 0; j < gridSize; j++)
			{
				nodes[i][j].pos.i = i;
				nodes[i][j].pos.j = j;
				nodes[i][j].gscore = 99999;
				nodes[i][j].hscore = 99999;
				nodes[i][j].fscore = 99999;
				nodes[i][j].inOpenList = false;
				nodes[i][j].inCloseList = false;
			}

		nodes[start.i][start.j].gscore = 0;
		nodes[start.i][start.j].hscore = 0;
		nodes[start.i][start.j].fscore = nodes[start.i][start.j].hscore;  //fscore h + g mais ici g = 0

		nodes[start.i][start.j].parent = start;
		nodes[start.i][start.j].inOpenList = true;
		nodes[start.i][start.j].inCloseList = false;

		while (true)
		{

			node* current = NULL;    // Trouver le noeud avec le plus petit fCost dans la open list
			for (int i = 0; i < gridSize; i++)
			{
				for (int j = 0; j < gridSize; j++)
				{
					if (nodes[i][j].inOpenList && !nodes[i][j].inCloseList)
					{
						if (current == NULL || nodes[i][j].fscore < current->fscore)
							current = &nodes[i][j];
					}
				}
			}

			if (!current) break; // aucun chemin trouvé

			current->inCloseList = true;
			current->inOpenList = false;

			// CORRECTION : Vérifier la vraie condition de victoire
			bool hasWon = false;
			if (playerID == 0) {
				// Joueur 0 : atteindre j = gridSize-1 (côté droit)
				hasWon = (current->pos.j == gridSize - 1);
			}
			else if (playerID == 1) {
				// Joueur 1 : atteindre j = 0 (côté gauche)
				hasWon = (current->pos.j == 0);
			}
			else if (playerID == 2) {
				// Joueur 2 : atteindre i = gridSize-1 (côté bas)
				hasWon = (current->pos.i == gridSize - 1);
			}
			else if (playerID == 3) {
				// Joueur 3 : atteindre i = 0 (côté haut)
				hasWon = (current->pos.i == 0);
			}


			if (hasWon) {
				// Reconstruire le chemin
				QuoridorPos tempPath[MAX_GRID_SIZE * MAX_GRID_SIZE];
				int pathLen = 0;

				QuoridorPos p = current->pos;

				while (!(p.i == start.i && p.j == start.j)) {
					assert(pathLen < MAX_GRID_SIZE * MAX_GRID_SIZE && "Chemin du A* trop grand");
					tempPath[pathLen++] = p;
					p = nodes[p.i][p.j].parent;
				}

				// Ajouter la position de départ
				if (pathLen < MAX_GRID_SIZE * MAX_GRID_SIZE) {
					tempPath[pathLen++] = start;
				}

				// Remettre dans l'ordre : start → goal
				for (int i = 0; i < pathLen; i++) {
					outPath[i] = tempPath[pathLen - 1 - i];
				}

				return pathLen; // Retourner la taille du chemin trouvé
			}

			// Explorer voisins (haut/bas/gauche/droite)
			int dI[] = { -1, +1, 0, 0 };
			int dJ[] = { 0, 0, -1, +1 };

			for (int d = 0; d < 4; d++)
			{
				int ni = current->pos.i + dI[d];
				int nj = current->pos.j + dJ[d];

				if (ni < 0 || ni >= gridSize || nj < 0 || nj >= gridSize)
					continue;


				QuoridorPos neighborPos = { ni, nj };

				if (isWallBetween(self, current->pos, neighborPos))
					continue;

				node* neighbor = &nodes[ni][nj];
				if (neighbor->inCloseList)
					continue;

				int tentativeG = current->gscore + 1;

				if (!neighbor->inOpenList || tentativeG < neighbor->gscore)
				{
					neighbor->gscore = tentativeG;
					neighbor->hscore = StarHeuristique(self, neighbor, playerID);
					neighbor->fscore = neighbor->gscore + neighbor->hscore;

					neighbor->parent = current->pos;
					neighbor->inOpenList = true;
					neighbor->inCloseList = false;
				}
			}
		}

		return 0; // pas de chemin trouvé
	}
	float QuoridorCore_scoreNoRand(QuoridorCore* self, int playerID)
	{
		int playerA = playerID;
		int playerB = playerID ^ 1;

#ifdef FOURPLAYERS
		playerB = (playerID + 1) % 4;
		int playerC = (playerID + 2) % 4;
		int playerD = (playerID + 3) % 4;
		QuoridorPos playerCpath[MAX_GRID_SIZE * MAX_GRID_SIZE];
		QuoridorPos playerDpath[MAX_GRID_SIZE * MAX_GRID_SIZE];
#endif
		int distA = 0;
		int distB = 0;
		int distC = 0;
		int distD = 0;


		QuoridorPos playerApath[MAX_GRID_SIZE * MAX_GRID_SIZE];
		QuoridorPos playerBpath[MAX_GRID_SIZE * MAX_GRID_SIZE];



#ifndef A_STAR 
		distA = BFS_search2(self, playerA, playerApath);
		distB = BFS_search2(self, playerB, playerBpath);
#else
		distA = AStar_search(self, playerA, playerApath);
		distB = AStar_search(self, playerB, playerBpath);
#endif
		float score = (float)(distB - distA);

#ifdef FOURPLAYERS
#ifndef A_STAR 
		distC = BFS_search2(self, playerC, playerCpath);
		distD = BFS_search2(self, playerD, playerDpath);
#else
		distC = AStar_search(self, playerC, playerCpath);
		distD = AStar_search(self, playerD, playerDpath);
#endif
		score = (float)((distB + distC + distD)/3 - distA);
#endif // FOURPLAYERS





		//printf("pos = %d,%d   %.2f\n", self->positions[playerA].i, self->positions[playerA].j,score);





		return score;
	}


