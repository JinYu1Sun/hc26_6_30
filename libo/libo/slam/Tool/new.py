import pandas as pd
import matplotlib.pyplot as plt
import sys

# 读取txt文件
def read_txt_file(filename):
    data = pd.read_csv(filename, sep=" ", header=None, names=["Y1", "Y2"])
    data['X'] = data.index + 1  # 添加一列表示行号
    return data

# 读取两个文件的数据
data1 = read_txt_file(sys.argv[1])
data2 = read_txt_file(sys.argv[2])

# 绘制曲线图
plt.figure(figsize=(10, 6))

# 第一条曲线
plt.plot(data1['X'], data1['Y1'], label='File1 Y1', color='blue')
plt.plot(data1['X'], data1['Y2'], label='File1 Y2', linestyle='--', color='blue')

# 第二条曲线
plt.plot(data2['X'], data2['Y1'], label='File2 Y1', color='red')
plt.plot(data2['X'], data2['Y2'], label='File2 Y2', linestyle='--', color='red')

# 添加标题和标签
plt.title('Comparison of Two Files')
plt.xlabel('Row Number')
plt.ylabel('Values')
plt.legend()
plt.grid(True)

# 显示图形
plt.show()
