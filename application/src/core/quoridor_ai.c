/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "core/quoridor_core.h"
#include "core/utils.h"
#include "core/graph.h"
#include "core/shortest_path.h"

#define MAX_BEST_WALLS 5

void *AIData_create(QuoridorCore *core)
{
    // Cette fonction n'est utile que si vous avez besoin de mémoriser des informations sur les coups précédents de l'IA.

    return NULL;
}

void AIData_destroy(void *self)
{
    if (!self) return;
}

void AIData_reset(void *self)
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

void QuoridorCore_getShortestPath(QuoridorCore* self, int playerID, QuoridorPos* path, int* size, Graph* graph)
{

    int min = INT_MAX;
    Path* bestpath;
    if (playerID == 1)
        for (int j = 0; j < 9; j++)
        {
            Path* path = Graph_shortestPath(graph,  self->positions[playerID].i * 9 + self->positions[playerID].j,j * 9);
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
static float QuoridorCore_computeScore(QuoridorCore *self, int playerID)
{
    int playerA = playerID;
    int playerB = playerID ^ 1;

    // 1. Calculer la distance (chemin le plus court) vers la ligne de but
    QuoridorPos path[MAX_PATH_LEN];
    int distA = 0;
    int distB = 0;
    Graph *graph = QuoridorCore_initGraph(self, playerA);
    QuoridorCore_getShortestPath(self, playerA, path, &distA,graph);
    Graph* graph2 = QuoridorCore_initGraph(self, playerB);
    QuoridorCore_getShortestPath(self, playerB, path, &distB,graph2);
    printf("aa");
    Graph_destroy(graph);
    Graph_destroy(graph2);
    // 2. Ajouter pondération sur le nombre de murs restants (optionnel mais utile)
    int wallsA = self->wallCounts[playerA];
    int wallsB = self->wallCounts[playerB];
    
    // Pondérer les murs à petite échelle (ex : 0.5 par mur)
    float score = (float)(distB - distA) + 0.3f * (wallsA - wallsB);

    // 3. Ajouter un bruit aléatoire pour casser les égalités (pas obligatoire mais pratique)
    //score += Float_randAB(-0.1f, +0.1f);

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
static float QuoridorCore_minMax(
    QuoridorCore *self, int playerID, int currDepth, int maxDepth,
    float alpha, float beta, QuoridorTurn *turn)
{
    if (self->state != QUORIDOR_STATE_IN_PROGRESS)
    {
        // TODO
    }
    else if (currDepth >= maxDepth)
    {
        return QuoridorCore_computeScore(self, playerID);
    }


    const int gridSize = self->gridSize;
    const int currI = self->positions[self->playerID].i;
    const int currJ = self->positions[self->playerID].j;
    QuoridorTurn childTurn = { 0 };

    const bool maximizing = (currDepth % 2) == 0;
    float value = maximizing ? -INFINITY : INFINITY;

    // TODO
    QuoridorPos moves[8];

    int moveCount = 0;
    moveCount = QuoridorCore_getMoves(self, moves, self->positions[self->playerID], 1);
    int max = -9999;
    int Idmax = 0;
    for (int k = 0; k < moveCount; k++)
    {
        QuoridorCore copy = *self;
        copy.positions[playerID].i = moves[k].i;
        copy.positions[playerID].j = moves[k].j;
        float tmp = QuoridorCore_minMax(&copy, playerID ^ 1, currDepth + 1, maxDepth, 0, 0, 0);
        if (tmp > max)
        {
            max = tmp;
            Idmax = k;
        }
    }



    // Astuce :
    // vous devez effectuer toutes vos actions sur une copie du plateau courant.
    // Comme la structure QuoridorCore ne contient aucune allocation interne,
    // la copie s'éffectue simplement avec :
    // QuoridorCore gameCopy = *self;

    return value;
}

QuoridorTurn QuoridorCore_computeTurn(QuoridorCore *self, int depth, void *aiData)
{
    QuoridorTurn childTurn = { 0 };

    if (self->state != QUORIDOR_STATE_IN_PROGRESS) return childTurn;

    const float alpha = -INFINITY;
    const float beta = INFINITY;
    float childValue = QuoridorCore_minMax(self, self->playerID, 0, depth, alpha, beta, &childTurn);
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
        if (!(currI - 1 == otherI && currJ == otherJ))
        {
            moves[Id].i = currI - 1;
            moves[Id++].j = currJ;

        }
            
    }
    if (currI < gridSize - 1 && !QuoridorCore_hasWallBelow(self, currI, currJ)) //on ajoute en dessous
    {
        if (!(currI + 1 == otherI && currJ == otherJ))
        {
            moves[Id].i = currI + 1;
            moves[Id++].j = currJ;
        }
    }
    if (currJ > 0 && !QuoridorCore_hasWallLeft(self, currI, currJ)) // on ajoute a gauche
    {
        if (!(currI == otherI && currJ - 1 == otherJ))
        {
            moves[Id].i = currI;
            moves[Id++].j = currJ - 1;
        }

    }
    if (currJ < gridSize - 1 && !QuoridorCore_hasWallRight(self, currI, currJ)) // on ajoute a droite
    {
        if (!(currI == otherI && currJ + 1 == otherJ))
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



//fonction pour comparer des murs grace a leur score de gain

int compareWalls(const void* a, const void* b) {
    const QuoridorWall* wa = (const QuoridorWall*)a;
    const QuoridorWall* wb = (const QuoridorWall*)b;
    return wb->score - wa->score; // tri décroissant
}


QuoridorWall *getBestWall(QuoridorCore* self, int player, int tolerance)
{
	const int gridSize = self->gridSize;
	const int otherPlayer = player ^ 1;
	
    QuoridorWall bestWall; 
    bestWall.pos.i = -1; 
    bestWall.pos.j = -1; 
    bestWall.score = -1;
    
    int longestEnemyWaySize, longesPlayerWaySize = -1;  
    int playerRatio, enemyRatio = 0;
    int gain = 0 ; 
	int wallCount = 0;

	QuoridorWall* bestWalls = (QuoridorWall*)calloc(sizeof(QuoridorWall), MAX_BEST_WALLS); // on initialise le tableau de 5 murs 

	QuoridorPos* enemyPath = calloc(81, sizeof(QuoridorPos)); // on intialise un chemim maximum vide 
	int enemySize = 0; // on initialise la taille a 0
    
    QuoridorPos* playerPath = calloc(81, sizeof(QuoridorPos)); // on intialise un chemim maximum vide
	int playerSize = 0; // on initialise la taille a 0

    QuoridorPos* tempPath = calloc(81, sizeof(QuoridorPos)); // on intialise un chemim maximum vide 
	int actualPlayerSize, actualEnemySize = 0; 

	Graph* playerGraph = QuoridorCore_initGraph(self, player); // on initialise le graphe du joueur en cours 
	QuoridorCore_getShortestPath(self, player, tempPath, &actualPlayerSize, playerGraph); // on recupere la taille du plus court chemin
    Graph_destroy(playerGraph);

	Graph* enemyGraph = QuoridorCore_initGraph(self, otherPlayer); // on initialise le graphe de l'autre joueur 
	QuoridorCore_getShortestPath(self, otherPlayer, tempPath, &actualEnemySize, enemyGraph); // on recupere la taille du plus court chemin 
	Graph_destroy(enemyGraph); 

    free(tempPath); 
    

	for (int i = 0; i < gridSize - 1 ; i++)
    {
        for (int j = 0; j < gridSize - 1 ; j++)
        {
            for (int type = 0 ; type < 2 ; type++)
            {

                if (!QuoridorCore_canPlayWall(self, type, i, j))
                    continue;

                Graph* enenemyGraph = QuoridorCore_initGraph(self, otherPlayer); 
                Graph* playerGraph = QuoridorCore_initGraph(self, player); // on initialise le graphe du joueur ennemi et du joueur

                if (type == WALL_TYPE_HORIZONTAL) 
                {
					Graph_removeArc(enenemyGraph, i * 9 + j, (i + 1) * 9 + j); //on retire les arc aller retour des deux case 4 cases concernées pour le jouer enemis 
                    Graph_removeArc(enenemyGraph, (i + 1) * 9 + j, i * 9 + j);
                    Graph_removeArc(enenemyGraph, i * 9 + (j + 1), (i + 1) * 9 + (j + 1));
                    Graph_removeArc(enenemyGraph, (i + 1) * 9 + (j + 1), i * 9 + (j + 1));
                    
                    Graph_removeArc(playerGraph, i * 9 + j, (i + 1) * 9 + j); //on retire les arc aller retour des deux case 4 cases concernées pour le joueur en cours
                    Graph_removeArc(playerGraph, (i + 1) * 9 + j, i * 9 + j);
                    Graph_removeArc(playerGraph, i * 9 + (j + 1), (i + 1) * 9 + (j + 1));
                    Graph_removeArc(playerGraph, (i + 1) * 9 + (j + 1), i * 9 + (j + 1));


                }
                else 
                {
                    Graph_removeArc(enenemyGraph, i * 9 + j, i * 9 + (j + 1)); //on retire les arc aller retour des deux case 4 cases concernées ppur le joueur enemis
                    Graph_removeArc(enenemyGraph, i * 9 + (j + 1), i * 9 + j);
                    Graph_removeArc(enenemyGraph, (i + 1) * 9 + j, (i + 1) * 9 + (j + 1));
                    Graph_removeArc(enenemyGraph, (i + 1) * 9 + (j + 1), (i + 1) * 9 + j);
                    
                    Graph_removeArc(playerGraph, i * 9 + j, i * 9 + (j + 1)); //on retire les arc aller retour des deux case 4 cases concernées ppur le joueur en cours
                    Graph_removeArc(playerGraph, i * 9 + (j + 1), i * 9 + j);
                    Graph_removeArc(playerGraph, (i + 1) * 9 + j, (i + 1) * 9 + (j + 1));
                    Graph_removeArc(playerGraph, (i + 1) * 9 + (j + 1), (i + 1) * 9 + j);
                }

                QuoridorCore_getShortestPath(self, otherPlayer, enemyPath, &enemySize, enenemyGraph);
				QuoridorCore_getShortestPath(self, player, playerPath, &playerSize, playerGraph); 

                Graph_destroy(enenemyGraph); 
				Graph_destroy(playerGraph);  

				
                
				playerRatio = playerSize - actualPlayerSize  ; //le nombre de case en + pour le joeur
				enemyRatio = enemySize - actualEnemySize ;    // le nombre de cases  en + pour l enemy
               
				gain = playerRatio - enemyRatio;  // on fait la diff des deux ratio pour savoir si rentable

                if (playerRatio < tolerance && enemyRatio > 0)
                {
                    if (wallCount < MAX_BEST_WALLS) 
                    {
						bestWalls[wallCount].type = type; 
						bestWalls[wallCount].pos.i = i;
						bestWalls[wallCount].pos.j = j;
						bestWalls[wallCount].score = gain;
						wallCount++;
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
            }
			
		}
	}

    qsort(bestWalls, wallCount, sizeof(QuoridorWall), compareWalls); 

    free(enemyPath);
    free(playerPath);
    
    return bestWalls; // on retourne la tableau de mur
}
