#include <iostream>
#include <vector>
#include <omp.h>
using namespace std;

#define GRID_SIZE 100
#define ITERATIONS 1000

// Cell states
enum State { DEAD, HEALTHY, INFECTED, IMMUNE };

struct Cell {
    State state;
    int timer; // Timer for infected and immune states
};

void updateGrid(vector<vector<Cell>>& grid, int rows, int cols) {
    vector<vector<Cell>> nextGrid = grid;

    #pragma omp parallel for collapse(2)
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int healthyCount = 0, immuneCount = 0;
            // Count neighbors
            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {
                    if (x == 0 && y == 0) continue; // Skip self
                    int ni = (i + x + rows) % rows; // Wrap around borders
                    int nj = (j + y + cols) % cols;

                    if (grid[ni][nj].state == HEALTHY) healthyCount++;
                    if (grid[ni][nj].state == IMMUNE) immuneCount++;
                }
            }

            // Update state based on rules
            Cell& cell = nextGrid[i][j];
            switch (grid[i][j].state) {
                case DEAD:
                    if (healthyCount + immuneCount == 3) cell.state = HEALTHY;
                    break;
                case HEALTHY:
                    if (healthyCount + immuneCount < 2 || healthyCount + immuneCount > 4)
                        cell.state = DEAD;
                    break;
                case INFECTED:
                    if (--cell.timer <= 0) cell.state = IMMUNE;
                    break;
                case IMMUNE:
                    if (--cell.timer <= 0) cell.state = HEALTHY;
                    break;
            }
        }
    }
    grid = nextGrid;
}

int main() {
    // Initialize grid
    vector<vector<Cell>> grid(GRID_SIZE, vector<Cell>(GRID_SIZE, {DEAD, 0}));
    // Set initial states (example)
    grid[1][1] = {HEALTHY, 0};
    grid[1][2] = {HEALTHY, 0};
    grid[2][1] = {HEALTHY, 0};

    for (int iter = 0; iter < ITERATIONS; ++iter) {
        updateGrid(grid, GRID_SIZE, GRID_SIZE);
    }

    cout << "Simulation completed" << endl;
    return 0;
}
