/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include "settings.h"
#include "core/quoridor_core.h"
#include "core/utils.h"
#include "core/graph.h"
#include "core/shortest_path.h"
#include "core/listQuor.h"
/// @brief Crée les données utilisées par l'IA.
/// @param core Instance du jeu Quoridor.
/// @return Pointeur vers les données de l'IA créées.
void *AIData_create(QuoridorCore *core);

/// @brief Détruit les données de l'IA.
/// @param self Pointeur vers les données de l'IA à détruire.
void AIData_destroy(void *self);

/// @brief Réinitialise les données de l'IA.
/// À appeler au début de chaque nouvelle partie.
/// @param self Pointeur vers les données de l'IA.
void AIData_reset(void *self);




/// @brief Calcule le coup joué par l'IA selon un algorithme de type min-max.
/// @param self Instance du jeu Quoridor.
/// @param depth Profondeur de la recherche dans l'arbre de jeu.
/// @param aiData Pointeur vers les données de l'IA.
/// @return Le tour choisi par l'IA.
QuoridorTurn QuoridorCore_computeTurn(QuoridorCore *self, int depth, void *aiData);
Graph* QuoridorCore_initGraph(QuoridorCore* self, int playerID);

/// @brief prend au + 4 mur *taille du chemin
void collectAllWallsNearPath(QuoridorCore* self, QuoridorPos* path, int pathSize, QuoridorWall* candidat, int* candidatCount);

// @brief prend au + 8 mur sur les 3 premiers cases du chemin et dans la direction du joueur
void collectFewWallsInFrontOfPath(QuoridorCore* self, QuoridorPos* path, int pathSize, QuoridorWall* candidat, int* candidatCount);


/// @brief Calcule les meilleurs mur à jouer.

void getBestWall(QuoridorCore* self, int player, int tolerance, QuoridorWall* bestWalls,int *wallCount);



/// @brief Calcule le plus court chemin entre la position du joueur et sa zone d'arrivée.
/// @param self Instance du jeu Quoridor.
/// @param playerID Identifiant du joueur (0 ou 1).
/// @param path Tableau (préalloué, taille MAX_PATH_LEN) dans lequel sera écrit le plus court chemin.
///     La position finale (dans la zone de victoire) est stockée à l'indice 0,
///     et la position actuelle du joueur à l'indice (size - 1).
/// @param size Adresse d'un entier dans lequel sera écrite la taille du chemin.
///     La distance est alors égale à (size - 1).
void QuoridorCore_getShortestPath(QuoridorCore *self, int playerID, QuoridorPos *path, int *size,Graph* graph);

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


/// @brief fonction qui retourne l'huristique du'un position dist en j
/// @param self Instance du jeu Quoridor.
/// @param node Position du joueur
/// @param player Identifiant du joueur (0 ou 1).
/// @return la valeur de l'huristique
int heuristic(QuoridorCore* self, int node, int player);

///@brief fonction pour retrouver le chemin passe apres A*
/// @param self Instance du jeu Quoridor.
/// @param cameFrom tableau contenant les noeuds parents
/// @param current position actuelle
/// @param path tableau contenant le chemin
/// @param pathSize taille du tableau path
/// @return le chemin
void pathRefinder(QuoridorCore* self, int* cameFrom, int current, QuoridorPos* path, int* pathSize);

/// @brief fonction pour trouver le noeud ayant le fscore le plus bas
/// @bief fscore = gscore + hscore
/// @param inOpenSet tableau contenant les noeuds ouverts
/// @param fScore tableau contenant les fscore
/// @param size taille du tableau
/// @return le noeud ayant le fscore le plus bas
int extractLowestF(bool* inOpenSet, int* fScore, int size);


/// @brief fonction pour trouver le + court chemin jusqu'a la position de victoire base sur l'heuristique
/// @param self Instance du jeu Quoridor.
/// @param graph le graphe
/// @param player Identifiant du joueur (0 ou 1).
/// @param path tableau contenant le chemin
/// @param pathSize taille du tableau path
void AStarShortestPath(QuoridorCore* self, Graph* graph, int player, QuoridorPos* path, int* pathSize);

ListQuor *BFS_search(QuoridorCore* self, int playerID);
int BFS_search2(QuoridorCore* self, int playerID, QuoridorPos* tab);
