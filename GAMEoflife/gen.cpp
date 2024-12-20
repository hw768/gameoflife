#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

void generate_matrix(const std::string& filename, int rows, int columns) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    // 输出行数和列数作为第一行
    outfile << rows << " " << columns << "\n";

    // 设置随机种子
    std::srand(std::time(0));

    // 定义中心区域范围
    int center_row_start = rows * 0.4;
    int center_row_end = rows * 0.6;
    int center_col_start = columns * 0.4;
    int center_col_end = columns * 0.6;

    // 生成矩阵
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            if (i >= center_row_start && i < center_row_end &&
                j >= center_col_start && j < center_col_end) {
                // 中心区域的点，有一定概率成为感染细胞
                int infection_probability = std::rand() % 100; // 生成 0-99 的随机数
                if (infection_probability < 30) { // 假设 30% 的概率是感染细胞
                    outfile << '+';
                } else {
                    outfile << '*'; // 健康细胞
                }
            } else {
                // 非中心区域，随机分布健康细胞和死亡细胞
                int state = std::rand() % 2; // 生成 0 或 1
                if (state == 0) {
                    outfile << '*'; // 健康细胞
                } else {
                    outfile << '.'; // 死亡细胞
                }
            }
        }
        outfile << "\n";
    }

    outfile.close();
    std::cout << "Matrix written to " << filename << std::endl;
}

int main() {
    generate_matrix("matrix_100x100.txt", 100, 100);
    return 0;
}
