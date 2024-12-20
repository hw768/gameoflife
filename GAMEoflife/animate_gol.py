import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation, PillowWriter
from matplotlib.colors import ListedColormap

def read_generations(filename, grid_size=(100, 100)):
    """读取 generations.txt 文件并解析成数组"""
    generations = []
    current_gen = []
    grid_rows, grid_cols = grid_size
    
    with open(filename, 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith('Generation'):
                continue  # 跳过世代标记行
            elif line == '---END---':
                if current_gen:
                    # 如果当前世代的行数不足 grid_rows 行，补充空行
                    while len(current_gen) < grid_rows:
                        current_gen.append([0] * grid_cols)
                    # 确保只取前 grid_rows 行
                    current_gen = current_gen[:grid_rows]
                    generations.append(np.array(current_gen))
                    current_gen = []
            else:
                # 处理每一行，使用以下规则：
                # . -> 0, * -> 1, + -> 2, - -> 3
                if len(line) > 0:  # 只处理非空行
                    row = [
                        1 if c == '*' else 2 if c == '+' else 3 if c == '-' else 0 
                        for c in line[:grid_cols]
                    ]
                    # 如果行的长度小于 grid_cols，补充 0
                    while len(row) < grid_cols:
                        row.append(0)
                    current_gen.append(row)
    
    # 处理最后一个世代（如果没有 ---END--- 标记）
    if current_gen:
        while len(current_gen) < grid_rows:
            current_gen.append([0] * grid_cols)
        current_gen = current_gen[:grid_rows]
        generations.append(np.array(current_gen))
    
    return generations

def create_gif_animation(generations, output_filename='evolution_100x100.gif', interval=200, dpi=200):
    """创建 GIF 动画"""
    if not generations:
        raise ValueError("没有数据可以生成动画！")
        
    # 设置颜色映射
    cmap = ListedColormap(['#e6e6e6', '#85c17e', '#f46d43', '#fee08b'])  # 柔和渐变的灰色、绿色、橙红色、金色

    # 设置高分辨率图像
    fig, ax = plt.subplots(figsize=(6, 6), dpi=dpi)
    plt.close()

    img = ax.imshow(generations[0], cmap=cmap, interpolation='nearest', vmin=0, vmax=3)
    ax.set_xticks([])
    ax.set_yticks([])
    ax.set_title("Game of Life Evolution", fontsize=18)

    def update(frame):
        img.set_data(generations[frame])
        ax.set_title(f'Generation {frame + 1}', fontsize=18)
        return [img]

    # 创建动画，设置帧间隔为200ms
    anim = FuncAnimation(fig, update, frames=len(generations), interval=200, blit=True)
    
    # 保存为 GIF 动画
    writer = PillowWriter(fps=5)  # 设置每秒帧数为5（即200ms间隔）
    anim.save(output_filename, writer=writer)

    print(f"GIF 动画已保存到 {output_filename}")

def main():
    try:
        file_path = 'generations.txt'
        grid_size = (100, 100)  # 动态指定网格大小
        generations = read_generations(file_path, grid_size=grid_size)
        
        if not generations:
            print("警告：没有读取到任何世代数据")
            return
            
        # 打印每个世代的形状以进行调试
        print("读取的世代数据形状：")
        for i, gen in enumerate(generations):
            print(f"世代 {i+1} 形状: {gen.shape}")
        
        # 创建 GIF 动画
        create_gif_animation(generations, output_filename='evolution_100x100.gif', interval=200, dpi=200)
        
    except Exception as e:
        print(f"程序运行出错: {str(e)}")

if __name__ == "__main__":
    main()
