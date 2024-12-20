#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <unistd.h>
#include "gol.hpp"

using namespace std;
using namespace chrono;

void print_usage() {
    cout << "Usage: gameoflife [-i input_file] [-g generations] [-s] [-t]" << endl;
    cout << "Options:" << endl;
    cout << "  -i input_file    Input file (required)" << endl;
    cout << "  -g generations   Number of generations (default: 5)" << endl;
    cout << "  -s              Print statistics" << endl;
    cout << "  -t              Use torus topology" << endl;
}

int main(int argc, char* argv[]) {
    string input_file;
    int generations = 5;
    bool print_stats = false;
    bool use_torus = false;

    // 处理命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "i:g:sth")) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'g':
                generations = stoi(optarg);
                if (generations <= 0) {
                    cerr << "Generations must be positive" << endl;
                    return 1;
                }
                break;
            case 's':
                print_stats = true;
                break;
            case 't':
                use_torus = true;
                break;
            case 'h':
                print_usage();
                return 0;
            default:
                print_usage();
                return 1;
        }
    }

    // 验证必需参数
    if (input_file.empty()) {
        cerr << "Input file is required" << endl;
        print_usage();
        return 1;
    }

    // 打开输入文件
    ifstream infile(input_file);
    if (!infile.is_open()) {
        cerr << "Error opening input file: " << input_file << endl;
        return 1;
    }

    // 读取网格大小
    int rows, cols;
    if (!(infile >> rows >> cols) || rows <= 0 || cols <= 0) {
        cerr << "Invalid grid dimensions" << endl;
        return 1;
    }
    infile.ignore();

    // 初始化宇宙
    auto init_start = high_resolution_clock::now();
    Universe u(rows, cols);
    read_in_file(infile, u);
    infile.close();
    u.save_generation(1);
    auto init_end = high_resolution_clock::now();

    // 演化阶段
    auto evolve_start = high_resolution_clock::now();
    for (int gen = 2; gen <= generations; ++gen) {
        reproduce(u, use_torus);
        spread_infection(u, use_torus);
        u.save_generation(gen);

        if (print_stats) {
            cout << "\nGeneration " << gen << ":" << endl;
            print_statistics(u);
        }
    }
    auto evolve_end = high_resolution_clock::now();

    // 输出性能统计
    double init_time = duration_cast<duration<double>>(init_end - init_start).count();
    double evolve_time = duration_cast<duration<double>>(evolve_end - evolve_start).count();

    cout << "\nPerformance Statistics:" << endl;
    cout << "Initialization time: " << init_time << " seconds" << endl;
    cout << "Evolution time for " << generations << " generations: " << evolve_time << " seconds" << endl;
    cout << "Average time per generation: " << evolve_time / (generations - 1) << " seconds" << endl;

    return 0;
}
