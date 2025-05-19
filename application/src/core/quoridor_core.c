/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "core/quoridor_core.h"
#include "core/quoridor_ai.h"

void QuoridorCore_updateValidMoves(QuoridorCore *self);
bool QuoridorCore_isFeasible(QuoridorCore *self);

QuoridorCore *QuoridorCore_create()
{
    QuoridorCore *self = (QuoridorCore *)calloc(1, sizeof(QuoridorCore));
    AssertNew(self);

    QuoridorCore_reset(self, 9, 10, 0);

    return self;
}

void QuoridorCore_destroy(QuoridorCore *self)
{
    if (!self) return;
    free(self);
}

void QuoridorCore_reset(QuoridorCore *self, int gridSize, int wallCount, int firstPlayer)
{
    memset(self, 0, sizeof(QuoridorCore));

    self->gridSize = gridSize;
    self->playerID = firstPlayer;

    self->positions[0].i = gridSize / 2;
    self->positions[0].j = 0;
    self->positions[1].i = gridSize / 2;
    self->positions[1].j = gridSize - 1;

    self->wallCounts[0] = wallCount;
    self->wallCounts[1] = wallCount;

    QuoridorCore_updateValidMoves(self);
}

void QuoridorCore_randomStart(QuoridorCore *self)
{
    self->wallCounts[0] += 2;
    self->wallCounts[1] += 2;

    const int gridSize = self->gridSize;

    for (int cpt = 0; cpt < 4; cpt++)
    {
        while (true)
        {
            int i = rand() % (gridSize - 1);
            int j = rand() % (gridSize - 1);
            int type = rand() & 1 ? WALL_TYPE_HORIZONTAL : WALL_TYPE_VERTICAL;
            if (QuoridorCore_canPlayWall(self, type, i, j))
            {
                QuoridorCore_playWall(self, type, i, j);
                break;
            }
        }
    }
}

bool QuoridorCore_canPlayWall(QuoridorCore *self, WallType type, int i, int j)
{
    if (i < 0 || i >= self->gridSize - 1) return false;
    if (j < 0 || j >= self->gridSize - 1) return false;

	//check si il reste des murs
	if (self->wallCounts[self->playerID] <= 0) return false;

	//check si le mur est déjà placé

	if (type == WALL_TYPE_HORIZONTAL && self->hWalls[i][j] != WALL_STATE_NONE) return false;
	if (type == WALL_TYPE_VERTICAL && self->vWalls[i][j] != WALL_STATE_NONE) return false;

	// check si le mur n'en chevauche pas un autre mur
    if (type == WALL_TYPE_HORIZONTAL) 
    {
        if (j > 0 && self->hWalls[i][j] != WALL_STATE_NONE) return false; 
        if (j < self->gridSize - 2 && self->hWalls[i][j + 1] != WALL_STATE_NONE) return false; 
		if (self->vWalls[i][j] == WALL_STATE_START) return false;
    }
    else if (type == WALL_TYPE_VERTICAL) 
    {
        if (i > 0 && self->vWalls[i][j] != WALL_STATE_NONE) return false; 
        if (i < self->gridSize - 2 && self->vWalls[i + 1][j] != WALL_STATE_NONE) return false; 
		if (self->hWalls[i][j] == WALL_STATE_START) return false;
    }
    
    // on les pause potentielement temporairement
    if (type == WALL_TYPE_HORIZONTAL) 
    {
        self->hWalls[i][j] = WALL_STATE_START; 
		self->hWalls[i][j + 1] = WALL_STATE_END; //pas sur de la necesserite
    }
    else
    {
        self->vWalls[i][j] = WALL_STATE_START; 
		self->vWalls[i + 1][j] = WALL_STATE_END;//pas sur de la necesserite
    }
    

    // Vérifie si les deux joueurs ont toujours un chemin vers leur ligne d'arrivée
    bool feasible = QuoridorCore_isFeasible(self);  
	
	
    // Annule le placement temporaire
    if (type == WALL_TYPE_HORIZONTAL) 
    {
        self->hWalls[i][j] = WALL_STATE_NONE; 
        self->hWalls[i][j + 1] = WALL_STATE_NONE; 
    }
	else if (type == WALL_TYPE_VERTICAL)
    {
        self->vWalls[i][j] = WALL_STATE_NONE; 
        self->vWalls[i + 1][j] = WALL_STATE_NONE; 
    }
	
    return feasible;
   
}

void QuoridorCore_updateValidMoves(QuoridorCore *self)
{

    const int gridSize = self->gridSize;
    const int currI = self->positions[self->playerID].i;
    const int currJ = self->positions[self->playerID].j;
    const int otherI = self->positions[self->playerID ^ 1].i;
    const int otherJ = self->positions[self->playerID ^ 1].j;

    for (int i = 0; i < MAX_GRID_SIZE; i++)
    {
		for (int j = 0; j < MAX_GRID_SIZE; j++)
        {
			self->isValid[i][j] = false;
		}
    }
    
	//ajoute ou non des 4 cases voisines

    if (currI > 0 && !QuoridorCore_hasWallAbove(self, currI, currJ)) // on ajoute au dessus
    {
        if (!(currI - 1 == otherI && currJ == otherJ))
	        self->isValid[currI - 1][currJ] = true;
    }
	if (currI < gridSize - 1 && !QuoridorCore_hasWallBelow(self, currI, currJ)) //on ajoute en dessous
	{
        if (!(currI + 1 == otherI && currJ == otherJ))
            self->isValid[currI + 1][currJ] = true;
	}
	if (currJ > 0 && !QuoridorCore_hasWallLeft(self, currI, currJ)) // on ajoute a gauche
	{
        if (!(currI == otherI && currJ - 1 == otherJ))
		    self->isValid[currI][currJ - 1] = true;
	}
	if (currJ < gridSize - 1 && !QuoridorCore_hasWallRight(self, currI, currJ)) // on ajoute a droite
    { 
        if (!(currI == otherI && currJ + 1 == otherJ))
		    self->isValid[currI][currJ + 1] = true;
    }


	/// ------------------------------------------------------------------------
    //ajoust ou non des coups du mouton

	if (otherI > 0) // on saute par dessu l'autre joueurs vers le hauts
	{ 
		if (otherI == currI - 1 && otherJ == currJ) 
        { 
            if (!QuoridorCore_hasWallAbove(self, currI, currJ) && !QuoridorCore_hasWallAbove(self,otherI,currJ)) 
            {
				self->isValid[otherI -1 ][currJ] = true; 
			}
		}
	}

	if (otherI < gridSize - 1) // on saute par dessu l'autre joueurs vers le bas 
	{
		if (otherI == currI + 1 && otherJ == currJ) 
		{
			if (!QuoridorCore_hasWallBelow(self, currI, currJ) && !QuoridorCore_hasWallBelow(self,otherI,currJ) )
			{
				self->isValid[otherI + 1][currJ] = true; 
			} 
		}
	}

	if (otherJ > 0) // on saute par dessu l'autre joueurs vers la gauche
	{
		if (otherI == currI && otherJ == currJ - 1)
		{
			if (!QuoridorCore_hasWallLeft(self, currI, currJ) && !QuoridorCore_hasWallLeft(self, otherI, otherJ))
			{
				self->isValid[currI][otherJ - 1] = true;
			}
		}
	}

	if (otherJ < gridSize - 1) // on saute par dessu l'autre joueurs vers la droite 
	{
		if (otherI == currI && otherJ == currJ + 1) 
		{ 
			if (!QuoridorCore_hasWallRight(self, currI, currJ) && !QuoridorCore_hasWallRight(self, otherI, otherJ))
			{ 
				self->isValid[currI][otherJ + 1] = true;
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
				if (!QuoridorCore_hasWallLeft(self,otherI,otherJ) && otherJ > 0)
				{
					self->isValid[otherI][otherJ - 1] = true;
				}
				if (!QuoridorCore_hasWallRight(self, otherI, otherJ) && otherJ < gridSize - 1)
				{
					self->isValid[otherI][otherJ + 1] = true;
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
					self->isValid[otherI][otherJ - 1] = true; 
				}
				if (!QuoridorCore_hasWallRight(self, otherI, otherJ) && otherJ < gridSize - 1) 
				{
					self->isValid[otherI][otherJ + 1] = true;
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
					self->isValid[otherI - 1][otherJ] = true;
				}
                if (!QuoridorCore_hasWallBelow(self, otherI, otherJ) && otherI < gridSize - 1)
                {
					self->isValid[otherI + 1][otherJ] = true;
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
					self->isValid[otherI - 1][otherJ] = true; 
				}
				if (!QuoridorCore_hasWallBelow(self, otherI, otherJ) && otherI < gridSize - 1) 
				{
					self->isValid[otherI + 1][otherJ] = true; 
				}
			}
		}
	}
    
}

static bool QuoridorCore_isFeasibleRec0(QuoridorCore *self, bool explored[MAX_GRID_SIZE][MAX_GRID_SIZE], int i, int j)
{

// Fonction récursive vérifiant qu'il existe un chemin permettant au joueur 0 de terminer la partie.
// Cette fonction réalise un parcours en profondeur.
// Ordonnez les cases voisines à explorer pour améliorer les performances.
    const int gridSize = self->gridSize;
    if (j >= gridSize - 1) return true;

	explored[i][j] = true;

    int nextI = 0;
	int nextJ = 0;

	//test une ligne au dessus
	nextI = i - 1;
	nextJ = j;

	if (nextI >= 0 && nextJ >= 0 && nextI < gridSize && nextJ < gridSize) //on reste dans la grille
	{
		if (!QuoridorCore_hasWallAbove(self,i,j) && !explored[nextI][nextJ]) //verifie si y a pas de mur au dessus ou pas deja visite
		{ 
			if (QuoridorCore_isFeasibleRec0(self, explored, nextI, nextJ)) 
                return true; 
		}
	} 

	//test une ligne en dessous
	nextI = i + 1; 
	nextJ = j; 

    if (nextI >= 0 && nextJ >= 0 && nextI < gridSize && nextJ < gridSize) //on reste dans la grille
    {
		if (!QuoridorCore_hasWallBelow(self, i, j) && !explored[nextI][nextJ]) //verifie si y a pas de mur en dessous ou pas deja visite
		{
			if (QuoridorCore_isFeasibleRec0(self, explored, nextI, nextJ))
				return true;
		}
    }

	//test une case a gauche
	nextI = i;
	nextJ = j - 1;

	if (nextI >= 0 && nextJ >= 0 && nextI < gridSize && nextJ < gridSize) //on reste dans la grille
	{
		if (!QuoridorCore_hasWallLeft(self, i, j) && !explored[nextI][nextJ]) //verifie si y a pas de mur a gauche ou pas deja visite 
		{
			if (QuoridorCore_isFeasibleRec0(self, explored, nextI, nextJ)) 
				return true;
		}
	}

	//test une case a droite
	nextI = i;
	nextJ = j + 1;

    if (nextI >= 0 && nextJ >= 0 && nextI < gridSize && nextJ < gridSize) //on reste dans la grille 
    {
		if (!QuoridorCore_hasWallRight(self, i, j) && !explored[nextI][nextJ]) //verifie si y a pas de mur a droite ou pas deja visite 
        {
			if (QuoridorCore_isFeasibleRec0(self, explored, nextI, nextJ)) 
				return true;
		}
    }

    return false;
}

static bool QuoridorCore_isFeasibleRec1(QuoridorCore *self, bool explored[MAX_GRID_SIZE][MAX_GRID_SIZE], int i, int j)
{
    const int gridSize = self->gridSize;
    if (j <= 0) return true;


    explored[i][j] = true;

    int nextI = 0;
    int nextJ = 0;

    //test une ligne au dessus
    nextI = i - 1;
    nextJ = j;

    if (nextI >= 0 && nextJ >= 0 && nextI < gridSize && nextJ < gridSize) //on reste dans la grille
    {
        if (!QuoridorCore_hasWallAbove(self, i, j) && !explored[nextI][nextJ]) //verifie si y a pas de mur au dessus ou pas deja visite
        {
            if (QuoridorCore_isFeasibleRec1(self, explored, nextI, nextJ))
                return true;
        }
    }

    //test une ligne en dessous
    nextI = i + 1;
    nextJ = j;

    if (nextI >= 0 && nextJ >= 0 && nextI < gridSize && nextJ < gridSize) //on reste dans la grille
    {
        if (!QuoridorCore_hasWallBelow(self, i, j) && !explored[nextI][nextJ]) //verifie si y a pas de mur en dessous ou pas deja visite
        {
            if (QuoridorCore_isFeasibleRec1(self, explored, nextI, nextJ))
                return true;
        }
    }

    //test une case a gauche
    nextI = i;
    nextJ = j - 1;

    if (nextI >= 0 && nextJ >= 0 && nextI < gridSize && nextJ < gridSize) //on reste dans la grille
    {
        if (!QuoridorCore_hasWallLeft(self, i, j) && !explored[nextI][nextJ]) //verifie si y a pas de mur a gauche ou pas deja visite 
        {
            if (QuoridorCore_isFeasibleRec1(self, explored, nextI, nextJ))
                return true;
        }
    }

    //test une case a droite
    nextI = i;
    nextJ = j + 1;

    if (nextI >= 0 && nextJ >= 0 && nextI < gridSize && nextJ < gridSize) //on reste dans la grille 
    {
        if (!QuoridorCore_hasWallRight(self, i, j) && !explored[nextI][nextJ]) //verifie si y a pas de mur a droite ou pas deja visite 
        {
            if (QuoridorCore_isFeasibleRec1(self, explored, nextI, nextJ))
                return true;
        }
    }

    return false;
}

bool QuoridorCore_isFeasible(QuoridorCore *self)
{
    bool explored0[MAX_GRID_SIZE][MAX_GRID_SIZE] = { 0 };
    bool explored1[MAX_GRID_SIZE][MAX_GRID_SIZE] = { 0 };

	if (QuoridorCore_isFeasibleRec0(self,explored0,self->positions[0].i, self->positions[0].j) 
        && QuoridorCore_isFeasibleRec1(self, explored1, self->positions[1].i,self->positions[1].j))
	{
		return true;
	}
    return false;
}

bool QuoridorCore_canMoveTo(QuoridorCore *self, int nextI, int nextJ)
{
    const int gridSize = self->gridSize;
    if (nextI < 0 || nextI >= gridSize) return false;
    if (nextJ < 0 || nextJ >= gridSize) return false;
    if (self->state != QUORIDOR_STATE_IN_PROGRESS) return false;
    return self->isValid[nextI][nextJ];
}

void QuoridorCore_playWall(QuoridorCore *self, WallType type, int i, int j)
{
    assert(0 <= i && i < self->gridSize - 1);
    assert(0 <= j && j < self->gridSize - 1);
    assert(self->wallCounts[self->playerID] > 0);
    
	if (!QuoridorCore_canPlayWall(self, type, i, j)) return;

	self->wallCounts[self->playerID]--; 

	if (type == WALL_TYPE_HORIZONTAL)
    {
		self->hWalls[i][j] = WALL_STATE_START;
		self->hWalls[i][j + 1] = WALL_STATE_END; 
    }

	if (type == WALL_TYPE_VERTICAL)
	{
		self->vWalls[i][j] = WALL_STATE_START;
		self->vWalls[i + 1][j] = WALL_STATE_END;
	}
	
    self->playerID ^= 1; // xor on change de joueur
    QuoridorCore_updateValidMoves(self);
}

void QuoridorCore_moveTo(QuoridorCore *self, int i, int j)
{
    assert(0 <= i && i < self->gridSize);
    assert(0 <= j && j < self->gridSize);

	if (QuoridorCore_canMoveTo(self, i, j) == false) return;

	self->positions[self->playerID].i = i;
	self->positions[self->playerID].j = j;

	if (self->playerID == 0 && j == self->gridSize - 1) // joueur 0 a gagné   
	{
		self->state = QUORIDOR_STATE_P0_WON; 
	}
	else if (self->playerID == 1 && j == 0) // joueur 1 a gagné
	{
		self->state = QUORIDOR_STATE_P1_WON; 
	}
    // TODO
    // Déplacez le pion.
    // Changez l'état de la partie si le joueur vient de gagner.

	self->playerID ^= 1; // xor on change de joueur 
    QuoridorCore_updateValidMoves(self); 
}

void QuoridorCore_playTurn(QuoridorCore *self, QuoridorTurn turn)
{
    if (turn.action == QUORIDOR_MOVE_TO)
    {
        //assert(QuoridorCore_canMoveTo(self, turn.i, turn.j));
        QuoridorCore_moveTo(self, turn.i, turn.j);
    }
	if (turn.action == QUORIDOR_PLAY_HORIZONTAL_WALL)
	{
		//assert(QuoridorCore_canPlayWall(self, WALL_TYPE_HORIZONTAL, turn.i, turn.j)); 
		QuoridorCore_playWall(self, WALL_TYPE_HORIZONTAL, turn.i, turn.j); 
	}
	if (turn.action == QUORIDOR_PLAY_VERTICAL_WALL)
	{
		//assert(QuoridorCore_canPlayWall(self, WALL_TYPE_VERTICAL, turn.i, turn.j));
		QuoridorCore_playWall(self, WALL_TYPE_VERTICAL, turn.i, turn.j);
    }

    // TODO
    // Complétez les autres actions possibles.
}

void QuoridorCore_print(QuoridorCore *self)
{
    const int gridSize = self->gridSize;
    for (int i = 0; i < gridSize; i++)
    {
        printf("+-");
    }
    printf("+\n");

    for (int i = 0; i < gridSize; i++)
    {
        printf("|");
        for (int j = 0; j < gridSize; j++)
        {
            if (self->positions[0].i == i && self->positions[0].j == j)
                printf("0");
            else if (self->positions[1].i == i && self->positions[1].j == j)
                printf("1");
            else
                printf(".");

            if (self->vWalls[i][j] == WALL_STATE_START)
                printf("A");
            else if (self->vWalls[i][j] == WALL_STATE_END)
                printf("B");
            else
                printf("|");
        }
        printf("\n+");

        for (int j = 0; j < gridSize; j++)
        {
            if (self->hWalls[i][j] == WALL_STATE_START)
                printf("A=");
            else if (self->hWalls[i][j] == WALL_STATE_END)
                printf("B|");
            else if (self->vWalls[i][j] == WALL_STATE_START)
                printf("-#");
            else
                printf("-+");
        }
        printf("\n");
    }
    printf("\n");
}
