import matplotlib.pyplot as plt
import numpy as np
import sys



# Function to read data from a file
def read_data(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
    return [float(line.strip()) for line in lines]

# Read data from the two files
data1 = read_data(sys.argv[1])
# data2 = read_data(sys.argv[2])

# if len(data1) != len(data2):
    # raise ValueError("Data files must have the same number of lines")

# Calculate the difference between the two data sets
# data_diff = [d1 - d2 for d1, d2 in zip(data1, data2)]

x_values = list(range(1, len(data1) + 1))

# Plot the data
plt.plot(x_values, data1,label='Data 1')
# plt.plot(x_values, data2,label='Data 2')
# plt.plot(x_values, data_diff,label='Data 1 - Data 2')

# Adding labels, title, and legend
plt.xlabel('Data Index')
plt.ylabel('Value')
plt.title('Comparison of Data Values from Two Files')
plt.legend()

# Display the plot
plt.grid(True)
plt.show()


# # 读取点积文件数据
# def read_dot_product_file(file_path):
#     with open(file_path, 'r') as file:
#         dot_products = [float(line.strip()) for line in file]
#     return np.array(dot_products)

# # 将点积转换为角度
# def dot_product_to_angle(dot_product):
#     # 限制点积值在[-1, 1]之间，以避免反余弦函数的范围错误
#     dot_product = np.clip(dot_product, -1.0, 1.0)
#     angles = np.arccos(dot_product)  # 计算夹角（弧度）
#     return np.degrees(angles)  # 将弧度转换为角度

# # 读取点积数据
# dot_product_file_path = sys.argv[1]  # 替换为实际文件路径
# dot_products = read_dot_product_file(dot_product_file_path)

# # 将点积转换为角度
# angles = dot_product_to_angle(dot_products)

# # 绘制图表
# plt.plot(angles, label='Angle between vectors (degrees)')
# plt.xlabel('Data Index')
# plt.ylabel('Angle (degrees)')
# plt.title('Angle between Direction Vectors')
# plt.legend()
# plt.grid(True)
# plt.show()
