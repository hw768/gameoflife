#ifndef GOL_H
#define GOL_H

#include <string>
#include <fstream>
#include <cstdlib>

// 宇宙类
class Universe {
public:
    int rows;
    int columns;
    int aliveCells;
    int totalAliveCells;
    char** grid;
    int** infection_life;  // 记录感染细胞的生命周期
    int** immune_life;     // 新增：记录免疫细胞的生命周期

    Universe(int r, int c);
    ~Universe();
    void save_generation(int generation) const;
};

// 细胞状态定义
const char HEALTHY = '*';  // 健康细胞
const char INFECTED = '+'; // 感染细胞
const char IMMUNE = '-';   // 新增：免疫细胞
const char DEAD = '.';     // 死细胞

// 生命周期定义
const int INFECTION_LIFETIME = 10; // 感染细胞的生命周期长度
const int IMMUNE_LIFETIME = 5;     // 免疫细胞的生命周期长度

// 函数声明
void read_in_file(std::ifstream& infile, Universe& u);
void reproduce(Universe& u, bool use_torus);
void spread_infection(Universe& u, bool use_torus);
void print_statistics(const Universe& u);

#endif
