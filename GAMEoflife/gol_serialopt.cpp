#include "gol.hpp"
#include <immintrin.h> // For AVX intrinsics
#include <iostream>
#include <cstring>

// 构造函数
Universe::Universe(int r, int c) : rows(r), columns(c), aliveCells(0), totalAliveCells(0) {
    grid = (char**)malloc(rows * sizeof(char*));
    infection_life = (int**)malloc(rows * sizeof(int*));
    immune_life = (int**)malloc(rows * sizeof(int*));

    for (int i = 0; i < rows; ++i) {
        grid[i] = (char*)malloc(columns * sizeof(char));
        infection_life[i] = (int*)malloc(columns * sizeof(int));
        immune_life[i] = (int*)malloc(columns * sizeof(int));
        std::memset(grid[i], DEAD, columns);
        std::memset(infection_life[i], 0, columns * sizeof(int));
        std::memset(immune_life[i], 0, columns * sizeof(int));
    }
}

// 析构函数
Universe::~Universe() {
    for (int i = 0; i < rows; ++i) {
        free(grid[i]);
        free(infection_life[i]);
        free(immune_life[i]);
    }
    free(grid);
    free(infection_life);
    free(immune_life);
}

// 保存每一代的状态到文件
void Universe::save_generation(int generation) const {
    std::ofstream outfile;
    if (generation == 1) {
        outfile.open("generations.txt", std::ios::out);
    } else {
        outfile.open("generations.txt", std::ios::app);
    }

    if (!outfile.is_open()) {
        std::cerr << "Error opening generations.txt" << std::endl;
        return;
    }

    outfile << "Generation " << generation << "\n";
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            outfile.put(grid[i][j]);
        }
        outfile.put('\n');
    }
    outfile << "---END---\n";
    outfile.close();
}

// 从文件读取初始状态
void read_in_file(std::ifstream& infile, Universe& u) {
    std::string line;
    for (int i = 0; i < u.rows && std::getline(infile, line); ++i) {
        for (int j = 0; j < u.columns && j < static_cast<int>(line.size()); ++j) {
            u.grid[i][j] = line[j];
            if (line[j] == HEALTHY) {
                u.aliveCells++;
                u.totalAliveCells++;
            } else if (line[j] == INFECTED) {
                u.infection_life[i][j] = INFECTION_LIFETIME;
            }
        }
    }
}

// 繁殖逻辑 with SIMD vectorization
void reproduce(Universe& u, bool use_torus) {
    char** new_grid = (char**)malloc(u.rows * sizeof(char*));
    for (int i = 0; i < u.rows; ++i) {
        new_grid[i] = (char*)malloc(u.columns * sizeof(char));
        std::memcpy(new_grid[i], u.grid[i], u.columns);
    }

    for (int i = 0; i < u.rows; ++i) {
        for (int j = 0; j < u.columns; j += 32) { // Process 32 cells at a time
            __m256i current_row = _mm256_loadu_si256((__m256i*)&u.grid[i][j]);
            __m256i neighbors = _mm256_setzero_si256();

            for (int di = -1; di <= 1; ++di) {
                for (int dj = -1; dj <= 1; ++dj) {
                    if (di == 0 && dj == 0) continue;

                    int ni = i + di;
                    int nj = j + dj;

                    if (use_torus) {
                        ni = (ni + u.rows) % u.rows;
                        nj = (nj + u.columns) % u.columns;
                    } else if (ni < 0 || ni >= u.rows || nj < 0 || nj >= u.columns) {
                        continue;
                    }

                    __m256i neighbor_row = _mm256_loadu_si256((__m256i*)&u.grid[ni][nj]);
                    neighbors = _mm256_add_epi8(neighbors, _mm256_cmpeq_epi8(neighbor_row, _mm256_set1_epi8(HEALTHY)));
                }
            }

            __m256i healthy_cells = _mm256_cmpeq_epi8(current_row, _mm256_set1_epi8(HEALTHY));
            __m256i dead_cells = _mm256_cmpeq_epi8(current_row, _mm256_set1_epi8(DEAD));

            __m256i healthy_neighbors_2_4 = _mm256_and_si256(
                _mm256_cmpgt_epi8(neighbors, _mm256_set1_epi8(1)),
                _mm256_cmplt_epi8(neighbors, _mm256_set1_epi8(5))
            );

            __m256i new_healthy = _mm256_and_si256(dead_cells, _mm256_cmpeq_epi8(neighbors, _mm256_set1_epi8(3)));
            __m256i remain_healthy = _mm256_and_si256(healthy_cells, healthy_neighbors_2_4);

            __m256i result = _mm256_or_si256(new_healthy, remain_healthy);

            _mm256_storeu_si256((__m256i*)&new_grid[i][j], result);
        }
    }

    for (int i = 0; i < u.rows; ++i) {
        std::memcpy(u.grid[i], new_grid[i], u.columns);
        free(new_grid[i]);
    }
    free(new_grid);
}

// 感染传播逻辑
void spread_infection(Universe& u, bool use_torus) {
    char** new_grid = (char**)malloc(u.rows * sizeof(char*));
    for (int i = 0; i < u.rows; ++i) {
        new_grid[i] = (char*)malloc(u.columns * sizeof(char));
        std::memcpy(new_grid[i], u.grid[i], u.columns);
    }

    for (int i = 0; i < u.rows; ++i) {
        for (int j = 0; j < u.columns; ++j) {
            if (u.grid[i][j] == INFECTED) {
                // 减少感染细胞生命周期
                u.infection_life[i][j]--;

                // 生命周期结束，变为免疫细胞
                if (u.infection_life[i][j] <= 0) {
                    new_grid[i][j] = IMMUNE;
                    u.immune_life[i][j] = IMMUNE_LIFETIME;
                    continue;
                }

                // 传播感染
                for (int di = -1; di <= 1; ++di) {
                    for (int dj = -1; dj <= 1; ++dj) {
                        if (di == 0 && dj == 0) continue;

                        int ni = i + di;
                        int nj = j + dj;

                        if (use_torus) {
                            ni = (ni + u.rows) % u.rows;
                            nj = (nj + u.columns) % u.columns;
                        } else if (ni < 0 || ni >= u.rows || nj < 0 || nj >= u.columns) {
                            continue;
                        }

                        // 健康细胞可以被感染，免疫细胞不可被感染
                        if (u.grid[ni][nj] == HEALTHY) {
                            new_grid[ni][nj] = INFECTED;
                            u.infection_life[ni][nj] = INFECTION_LIFETIME;
                            u.aliveCells--;
                        }
                    }
                }
            } else if (u.grid[i][j] == IMMUNE) {
                // 减少免疫细胞生命周期
                u.immune_life[i][j]--;
                if (u.immune_life[i][j] <= 0) {
                    new_grid[i][j] = HEALTHY; // 转变为健康细胞
                    u.aliveCells++;
                }
            }
        }
    }

    for (int i = 0; i < u.rows; ++i) {
        std::memcpy(u.grid[i], new_grid[i], u.columns);
        free(new_grid[i]);
    }
    free(new_grid);
}

// 打印统计信息
void print_statistics(const Universe& u) {
    std::cout << "Alive cells: " << u.aliveCells << std::endl;
    std::cout << "Total alive cells: " << u.totalAliveCells << std::endl;
}
