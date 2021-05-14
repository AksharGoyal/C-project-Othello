#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "othello.h"

/*
    initPiece will initialize the color, the position and the status of the cell
*/
void initPiece(cell *c, posn p)
{
    c->set = false;
    c->pos = p;
    c->color = '-';
}

/*
    validPosition checks if the coordinates provided are not outside the grid
*/
bool validPosition(int i, int j, int n)
{
    if (i < 0 || i >= n || j < 0 || j >= n)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/*
    setPiece will set the color, the position and the status of the cell.
    Key difference from initPiece is that a piece already having a color cannot be set.
    Also, only Black or White pieces can be set; no other pieces including a blank piece.
    But first the validity of the position is checked and then the piece is set.
    If not valid, nothing is done.
*/
void setPiece(grid *Game, posn p, char colour)
{
    if (validPosition(p.x, p.y, Game->size))
    {
        if ((!Game->board[p.x][p.y].set) && (colour == 'B' || colour == 'W'))
        {
            Game->board[p.x][p.y].set = true;
            Game->board[p.x][p.y].color = colour;
            Game->num_pieces++;
        }
    }
}

/*
    initBoard will be used to initialize the Board.
    Every board starts with blank pieces except the central square.
    The 4 squares in centre have {{B, W}, {W, B}} always set.
    initBoard takes integer n as paramter which is the size of the Board.
    Note, n has to be even and positive otherwise the game CANNOT be played.
*/
grid initBoard(grid * Game, int n)
{
    Game->size = n;
    Game->board = malloc(n * sizeof(cell *));
    Game->num_pieces = 0;
    int halfSize = n / 2;
    for (int i = 0; i < n; i++)
    {
        Game->board[i] = calloc(n, sizeof(cell));
        for (int j = 0; j < n; j++)
        {
            posn p = {i, j};
            initPiece(&Game->board[i][j], p);
        }
    }
    posn m1 = {halfSize - 1, halfSize - 1};
    setPiece(Game, m1, 'B');
    posn m2 = {halfSize - 1, halfSize};
    setPiece(Game, m2, 'W');
    posn m3 = {halfSize, halfSize - 1};
    setPiece(Game, m3, 'W');
    posn m4 = {halfSize, halfSize};
    setPiece(Game, m4, 'B');
    return *Game;
}

/*
    printBoard is called whenever we want to print the board.
    Uses POINTER ARITHMETIC
*/

void printBoard(grid *Game)
{
    int n = Game->size;

    for (int i = 0; i < n; i++)
    {
        printf(" |");
        for (int j = 0; j < n; j++)
        {
            printf(" %c", (*(Game->board + i) + j)->color);
        }
        printf(" |\n");
    }
}

/*
    gridDeepCopy creates and returns a deep copy of the game
*/
grid gridDeepCopy(grid *Game)
{
    grid gameCopy;
    gameCopy = initBoard(&gameCopy, Game->size);
    gameCopy.num_pieces = Game->num_pieces;

    for (unsigned int i = 0; i < gameCopy.size; i++)
    {
        for (unsigned int j = 0; j < gameCopy.size; j++)
        {
            gameCopy.board[i][j] = *(*(Game->board + i) + j); // No need to worry about shallow copy
        }
    }
    return gameCopy;
}

/*
    captureLine checks if a row, a column or a diagonal can have the pieces flipped or not
*/
bool captureLine(grid *Game, char player, int x, int y, int dx, int dy)
{
    // We found a piece of our own that we want to flip all pieces to
    if ((*(Game->board + x) + y)->color == player)
    {
        return true;
    }
    // We cannot flip the colors with a blank end
    if ((*(Game->board + x) + y)->color == '-')
    {
        return false;
    }
    // To ensure that our next step is a valid move
    if (!validPosition(x + dx, y + dy, Game->size))
    {
        return false;
    }
    // We continue searching along the line
    return captureLine(Game, player, x + dx, y + dy, dx, dy);
}

/*
    checkCapture decides if the player's planned piece will toggle other pieces around itself
*/
bool checkCapture(grid *Game, char player, int x, int y, int dx, int dy)
{
    char opponent = '-';
    if (player == 'W')
    {
        opponent = 'B';
    }
    else if (player == 'B')
    {
        opponent = 'W';
    }
    else // Either the piece is blank or the player has set up a different colour somehow.
    {
        return false;
    }

    if (!validPosition(x + dx, y + dy, Game->size)) // Can't proceed left or right
    {
        return false;
    }
    // if (!validPosition(x, y + dy, Game->size)) // Can't proceed up or down
    // {
    //     return false;
    // }
    if (!validPosition(x + dx + dx, y + dy + dy, Game->size)) // Can't proceed up or down
    {
        // There should be at least 2 valid moves for the sake of comparison
        return false;
    }
    // if (!validPosition(x, y + dy + dy, Game->size)) // Can't proceed up or down
    // {
    //     // There should be at least 2 valid moves for the sake of comparison
    //     return false;
    // }

    if ((*(Game->board + x + dx) + y + dy)->color != opponent)
    {
        // This is an important factor to check if we can start toggling the opponent's pieces or not.
        // If we can't, then we won't go further than this.
        return false;
    }

    return captureLine(Game, player, x + dx, y + dy, dx, dy);
}

void boardDestroy(grid *Game)
{
    for (unsigned int i = 0; i < Game->size; i++)
    {
        free(Game->board[i]);
        Game->board[i] = NULL;
    }
    free(Game->board);
    Game->board = NULL;
}

/*
    With the help of capture and checkCapture, we will send the player all the possibilities of
    valid moves they can make.
*/
void validMoves(grid *Game, char player)
{
    grid gameCopy = gridDeepCopy(Game);
    bool north = false, south = false, east = false, west = false;
    bool northeast = false, northwest = false, southeast = false, southwest = false;

    for (unsigned int i = 0; i < gameCopy.size; i++)
    {
        for (unsigned int j = 0; j < gameCopy.size; j++)
        {
            if ((*(Game->board + i) + j)->color == '-')
            {
                northwest = checkCapture(Game, player, i, j, -1, -1);
                north = checkCapture(Game, player, i, j, -1, 0);
                northeast = checkCapture(Game, player, i, j, -1, 1);
                east = checkCapture(Game, player, i, j, 0, 1);
                west = checkCapture(Game, player, i, j, 0, -1);
                southwest = checkCapture(Game, player, i, j, 1, -1);
                south = checkCapture(Game, player, i, j, 1, 0);
                southeast = checkCapture(Game, player, i, j, 1, 1);

                if (north || south || east || west || northeast || northwest || southeast || southwest)
                {
                    (*(gameCopy.board + i) + j)->color = player;
                }
            }
        }
    }
    printBoard(&gameCopy);
    boardDestroy(&gameCopy);
}

/*
    It's time to implement the function that will flip the whole game otherwise without
    flipped pieces the match will always endup in a draw

    flip flips the opponent's pieces in the provided direction
*/

void flip(grid *Game, char player, int x, int y, int dx, int dy)
{
    if (checkCapture(Game, player, x, y, dx, dy))
    { 
        for (int r = x + dx, c = y + dy; (*(Game->board + r) + c)->color != player; r += dx, c += dy)
        {   
            (*(Game->board + r) + c)->color = player;
        }
    }
}

/*
    flipLine flips the line in all possible directions around the set piece
*/
void flipLine(grid *Game, char player, int x, int y)
{
    flip(Game, player, x, y, -1, 1);        // Will flip up-rightwards
    flip(Game, player, x, y, -1, 0);        // Will flip upwards
    flip(Game, player, x, y, -1, -1);       // Will flip up-leftwards
    flip(Game, player, x, y, 0, -1);        // Will flip leftwards
    flip(Game, player, x, y, 0, 1);         // Will flip rightwards
    flip(Game, player, x, y, 1, 1);         // Will flip down-rightwards
    flip(Game, player, x, y, 1, 0);         // Will flip down
    flip(Game, player, x, y, 1, -1);         // Will flip down-leftwards

}

/*
    The game reversi is over when all the pieces are set.
    That is, the number of pieces both player 1 and player 2 have set
    are equal to number of cells on the board.
    Each row has size number of cells and the board has size number of rows.
   
*/
bool GameOver(grid *Game)
{
    return ((Game->num_pieces) == ((Game->size) * (Game->size)));   
}

/*
    After all the pieces are set, whoWon is called to determine who won the Game.
    Therefore, the function shouldn't be called if there are still any blank pieces left.
    There are 3 possibilities: Black wins, White wins, and Draw (nobody wins).
    We iterate through the whole grid and do a tally of every colour.
    A Draw is called when the number of black and white pieces are same.
*/
char whoWon(grid *Game)
{
    int b_count = 0, w_count = 0; // These variables keep tally of each colour
    int n = Game->size;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if ((*(Game->board + i) + j)->color == 'B')
            {
                b_count++; // If a black piece is found, we increase the count
            }
            else if ((*(Game->board + i) + j)->color == 'W')
            {
                w_count++; // If a white piece is found, we increase the count
            }
        }
    }

    if (b_count > w_count) // Black won!
    {
        return 'B';
    }
    else if (b_count < w_count) // White won!
    {
        return 'W';
    }
    else // It's a Draw!
    {
        return 'D';
    }
}

///////////////////////////////////////////////////////////////////////
///////////////////////         MAIN GAME        //////////////////////
///////////////////////////////////////////////////////////////////////

// positiveAndGreater4 checks if an integer is greater than or equal to 4 and even
bool positiveAndGE4(int n)
{
    return ((n >= 4) && ((n % 2) == 0));
}

// is used to clear screen
void clearScreen()
{
    const char *CLEAR_SCREEN_ANSI = "\e[1;1H\e[2J";
    write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
}

// Taken from Kenken
// is used to delay execution of certain parts of the program
void delay(double seconds)
{
    /*
    delays the program by a number of seconds given by double seconds
    We recommend:
        a delay of 1 for puzzles smaller than 4x4
        a delay of 0.5 for puzzles of size 4x4
        a delay of 0 or 0.1 for puzzles larger than 4x4
     */
    double milli_seconds = CLOCKS_PER_SEC * seconds;
    clock_t start_time = clock();
    while (clock() < start_time + milli_seconds)
        ;
}

int main(void)
{
    ///////////////////////////////////////////////////////////////////////
    ///////////////////////           SETUP          //////////////////////
    ///////////////////////////////////////////////////////////////////////

    /////           WELCOME         /////
    printf("Hello! Welcome to the game Reversi!\n");
    printf("We assume you know the rules already so let's get started!\n");
    printf("Please enter the size of board (even integer >= 4): ");
    // Size of the board is chosen: even and > 4
    int size;
    scanf("%d", &size);

    while (!positiveAndGE4(size))
    {
        printf("Sorry! Size of the board needs to be EVEN and GREATER THAN 4!\n");
        printf("Enter again: ");
        scanf("%d", &size);
    }

    grid Game;
    Game = initBoard(&Game, size);
    //  Players decide their colors
    char player1, player2 ; 
    printf("Player 1! Enter the color you would like to play with (B - Black / W - White): ");
    scanf(" %c", &player1);

    if (player1 != 'B' && player1 != 'W')
    {
        while (player1 != 'B' && player1 != 'W')
        {
            printf("Invalid colour choice! Please choose either 'B' for Black or 'W' for White!\n");
            printf("Enter again: ");
            scanf(" %c", &player1);
        }
    }

    player2 = player1 == 'B' ? 'W' : 'B';
    printf("Player 1 is %c and Player 2 is %c.\n", player1, player2);
    printf("\nThis is the board!\n\n");
    printBoard(&Game);

    // Variables/Objects for the game are created
    char pieces[3] = {player1, player2}; // array to hold the colour of pieces
    int count = player1 == 'B' ? 0 : 1;  // Game starts with Black piece
    int player = (count % 2) + 1;        // player index is based on turn or count
    int choice = 0;                      // Player's choice from menu
    int r = 0, c = 0;                    // Indices player's want to place the piece at
    posn p;

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////         GAME TIME        //////////////////////
    ///////////////////////////////////////////////////////////////////////

    printf("\nPlayer %d(%c), it's your turn. ", player, pieces[player - 1]);
    printf("\nChoices (1-5):\n1. Play\n2. Hint\n3. Pass\n4. Print\n5. Quit\n\nEnter your choice: ");

    // Game Loop begins
    while ((!GameOver(&Game)) && (choice != 5))
    {   
        scanf("%d", &choice);

        if (choice == 1)
        {
        printf("\nEnter the place you would like to set your piece at (r c):");
        scanf("%d %d", &r, &c);
        p.x = r;
        p.y = c;
            if (validPosition(r, c, size))
            {
                if ((*(Game.board + r) + c)->set == false)
                {
                    setPiece(&Game, p, pieces[count % 2]);
                    flipLine(&Game, pieces[player - 1], r, c);
                    printf("\n");
                    printBoard(&Game);
                    count++;
                }
            }
            else
            {
                printf("Sorry! Invalid Move! Try again!\n");
            }
        }
        else if (choice == 2)
        {
            printf("\n");
            validMoves(&Game, pieces[player - 1]);
        }
        else if (choice == 3)
        {
            count++;
        }
        else if (choice == 4)
        {
            printf("\n");
            printBoard(&Game);
        }
        else if (choice == 5)
        {
            break;
        }
        else
        {
            printf("Sorry! Wrong choice! Try again!\n");
            delay(2);
            clearScreen();
        }
        if (GameOver(&Game))
        {
            break;
        }
        player = (count % 2) + 1;
        printf("\nPlayer %d(%c), it's your turn. ", player, pieces[player - 1]);
        printf("\nChoices (1-5):\n1. Play\n2. Hint\n3. Pass\n4. Print\n5. Quit\n\nEnter your choice: ");   
    }

    ///////////////////////////////////////////////////////////////////////
    ///////////////////////         END GAME        ///////////////////////
    ///////////////////////////////////////////////////////////////////////

    // Player decided to quit the game.
    if (choice == 5)
    {
        count++;
        player = (count % 2) + 1;
        printf("\nPlayer %d(%c) won!\n", player, pieces[player - 1]);
        printf("Hope you enjoyed!\n");
        boardDestroy(&Game);
        delay(5);
        clearScreen();
        return 0;
    }

    // Board is full and it's time to call out the winner
    char gameStatus = whoWon(&Game);
    printf("\n#############   ");
    if (gameStatus == 'B')
    {
        printf("Black Won! Player ");
        if (player1 == 'B')
        {
            printf("1 won!");
        }
        else
        {
            printf("2 won!");
        }
    }
    else if (gameStatus == 'W')
    {
        printf("White Won! Player ");
        if (player1 == 'W')
        {
            printf("1 won!");
        }
        else
        {
            printf("2 won!");
        }
    }
    else
    {
        printf("It's Draw! Both lost!");
    }

    printf("   #############\n");
    boardDestroy(&Game);
}