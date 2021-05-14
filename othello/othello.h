
typedef struct
{
    int x; // Row index of cell
    int y; // Column index of cell
} posn;

/*
    Below is the definition of cell.
    A board is made up of size x size cells, with each cell having two attributes: position and colour
    The top-left cell has position (0,0) and the bottom-right has (size - 1, size - 1), 
    where size is the dimension of the board.
    A cell can have 3 colours in this game: Black(B), White(W), and Empty(-, the default color).
*/
typedef struct
{
    char color; // Every cell will be black, white or empty
    posn pos;   // Position of cell in the grid
    bool set;   // Checks if the piece has a color already or not
} cell;         // A gameboard is made of n x n cells

/*
    Below is the definition of grid.
    A grid is made of size x size cells, where size is the number of cells in a row.
    Because a grid is square, one field 'size' is enough to convey the information about the board.
    The field 'board' contains these rows of cells.
*/
typedef struct
{
    unsigned int size; // number of rows of length size as well
    cell **board;      // array of (array of cells)
    int num_pieces;    // keeps track of number of pieces set so far; will always start from 4
} grid;