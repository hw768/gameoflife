#include <mpi.h>
#include <vector>
#include <iostream>

using namespace std;

#define GRID_SIZE 100
#define ITERATIONS 100

enum State { DEAD, HEALTHY, INFECTED, IMMUNE };

struct Cell {
    State state;
    int timer;
};

// Exchange borders between processes
void exchangeBorders(vector<vector<Cell>>& grid, int rank, int size, int rows, int cols) {
    MPI_Status status;
    // Send and receive top border
    if (rank > 0) {
        MPI_Send(&grid[0][0], cols * sizeof(Cell), MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&grid[0][0], cols * sizeof(Cell), MPI_BYTE, rank - 1, 0, MPI_COMM_WORLD, &status);
    }
    // Send and receive bottom border
    if (rank < size - 1) {
        MPI_Send(&grid[rows - 1][0], cols * sizeof(Cell), MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD);
        MPI_Recv(&grid[rows - 1][0], cols * sizeof(Cell), MPI_BYTE, rank + 1, 0, MPI_COMM_WORLD, &status);
    }
}

// Parallel computation
void updateGrid(vector<vector<Cell>>& grid, int rows, int cols) {
    vector<vector<Cell>> nextGrid = grid;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Apply rules similar to OpenMP version
        }
    }
    grid = nextGrid;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows = GRID_SIZE / size; // Divide grid among processes
    vector<vector<Cell>> grid(rows, vector<Cell>(GRID_SIZE, {DEAD, 0}));

    for (int iter = 0; iter < ITERATIONS; ++iter) {
        exchangeBorders(grid, rank, size, rows, GRID_SIZE);
        updateGrid(grid, rows, GRID_SIZE);
    }

    MPI_Finalize();
    return 0;
}
