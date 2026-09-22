import matplotlib.pyplot as plt
import sys

def read_data(file_path):
    x, y, yaw = [], [], []
    with open(file_path, 'r') as file:
        for line in file:
            values = line.strip().split()
            if len(values) == 3:
                x_val, y_val, yaw_val = map(float, values)
                x.append(x_val)
                y.append(y_val)
                yaw.append(yaw_val)
    return x, y, yaw

def plot_xy_trajectory(trajectories, labels):
    plt.figure(figsize=(10, 5))
    for i, (x, y) in enumerate(trajectories):
        plt.plot(x, y, linestyle='-', linewidth=1, label=labels[i])
    plt.title('Trajectory')
    plt.xlabel('X(m)')
    plt.ylabel('Y(m)')
    plt.grid()
    plt.legend()

def plot_yaw(yaw_list, labels):
    plt.figure(figsize=(10, 5))
    for i, yaw in enumerate(yaw_list):
        plt.plot(yaw, linestyle='-', linewidth=1, label=labels[i])
    plt.title('Yaw Angle')
    plt.xlabel('Index')
    plt.ylabel('Yaw (radians)')
    plt.grid()
    plt.legend()

if __name__ == "__main__":
    file_path1 = sys.argv[1]  # 第一个 txt 文件路径
    file_path2 = sys.argv[2]  # 第二个 txt 文件路径
    file_path3 = sys.argv[3]  # 第三个 txt 文件路径
    
    x1, y1, yaw1 = read_data(file_path1)
    x2, y2, yaw2 = read_data(file_path2)
    x3, y3, yaw3 = read_data(file_path3)
    
    trajectories = [(x1, y1), (x2, y2), (x3, y3)]
    yaw_list = [yaw1, yaw2, yaw3]
    labels = ['GPS', 'LIO', 'Fusion']
    
    plot_xy_trajectory(trajectories, labels)
    # 如果需要绘制yaw图，可以取消注释以下行
    # plot_yaw(yaw_list, labels)
    
    plt.show()
