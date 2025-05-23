/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include "settings.h"
#include "core/quoridor_core.h"
#include "core/utils.h"
#include "core/listData.h" 



#define MAX_BEST_WALLS 500
#define RAND_VALUE 0.25
#define ALL_WALLS 
#define MAX_CANDIDATES 500


  
  



//// @brief Représente un mur sur le plateau de Quoridor.
typedef struct QuoridorWall
{
    /// @brief Type de mur (horizontal ou vertical).
    WallType type;
    QuoridorPos pos;
    int score;
} QuoridorWall;

typedef struct Node
{
	int hscore;
	int gscore;
	int fscore;
   
    bool inOpenList;
	bool inCloseList;

	QuoridorPos pos;
	QuoridorPos parent;

} node ;


/// @brief Crée les données utilisées par l'IA.
/// @param core Instance du jeu Quoridor.
/// @return Pointeur vers les données de l'IA créées.
ListData* AIData_create();

/// @brief Détruit les données de l'IA.
/// @param self Pointeur vers les données de l'IA à détruire.
void AIData_destroy(void* database); 


/// @brief Réinitialise les données de l'IA.
/// À appeler au début de chaque nouvelle partie.
/// @param self Pointeur vers les données de l'IA.
void AIData_reset(void* database);



/// @brief Calcule le coup joué par l'IA selon un algorithme de type min-max.
/// @param self Instance du jeu Quoridor.
/// @param depth Profondeur de la recherche dans l'arbre de jeu.
/// @param aiData Pointeur vers les données de l'IA.
/// @return Le tour choisi par l'IA.
QuoridorTurn QuoridorCore_computeTurn(QuoridorCore *self, int depth, void *aiData);

/// @brief prend au + 4 mur *taille du chemin
void collectAllWallsNearPath(QuoridorCore* self, QuoridorPos* path, int pathSize, QuoridorWall* candidat, int* candidatCount);

// @brief prend au + 8 mur sur les 3 premiers cases du chemin et dans la direction du joueur
//void collectFewWallsInFrontOfPath(QuoridorCore* self, QuoridorPos* path, int pathSize, QuoridorWall* candidat, int* candidatCount);


/// @brief Calcule les meilleurs mur à jouer.

void getBestWall(QuoridorCore* self, int player, int tolerance, QuoridorWall* bestWalls,int *wallCount);


/// @brief MInMax
static float QuoridorCore_minMax(QuoridorCore* self, int playerID, int currDepth, int maxDepth, float alpha, float beta, QuoridorTurn* turn, void* aiData);

/// computeScore
static float QuoridorCore_computeScore(QuoridorCore* self, int playerID);


/// @brief Calcule le plus court chemin entre la position du joueur et sa zone d'arrivée.
/// @param self Instance du jeu Quoridor.
/// @param playerID Identifiant du joueur (0 ou 1).
/// @param path Tableau (préalloué, taille MAX_PATH_LEN) dans lequel sera écrit le plus court chemin.
///     La position finale (dans la zone de victoire) est stockée à l'indice 0,
///     et la position actuelle du joueur à l'indice (size - 1).
/// @param size Adresse d'un entier dans lequel sera écrite la taille du chemin.
///     La distance est alors égale à (size - 1).
//void QuoridorCore_getShortestPath(QuoridorCore *self, int playerID, QuoridorPos *path, int *size,Graph* graph);

/// @brief Fonction spécifique à l'évaluation sur Moodle.
/// Calcule le coup joué par l'IA avec une profondeur de 2.
/// @param self Instance du jeu Quoridor.
/// @param aiData Pointeur vers les données de l'IA.
/// @return Le tour choisi par l'IA.
INLINE QuoridorTurn QuoridorCore_computeMoodleTurn(QuoridorCore* self, void* aiData)
{
    const int depth = 2;
    return QuoridorCore_computeTurn(self, depth, aiData);
}



int BFS_search2(QuoridorCore* self, int playerID, QuoridorPos* tab);


INLINE void* AIData_add(ListData* database, QuoridorData data)
{
    ListData_insertFirstPopLast(database, data);
}

void collectAllWall(QuoridorCore* self, QuoridorPos* path, int pathSize, QuoridorWall* candidat, int* candidatCount);

int AStar_search(QuoridorCore* self, int playerID, QuoridorPos* outPath);

bool isWallBetween(QuoridorCore* self, QuoridorPos depart, QuoridorPos arrive);
int StarHeuristique(QuoridorCore* self, node* node, int playerId);
float QuoridorCore_scoreNoRand(QuoridorCore* self, int playerID);






