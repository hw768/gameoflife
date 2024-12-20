# 使用 g++ 编译器
CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -std=c++17

# 目标
TARGET = gameoflife

# 源文件
SRCS = gameoflife.cpp gol.cpp
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# 编译规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理规则
clean:
	rm -f $(OBJS) $(TARGET)
