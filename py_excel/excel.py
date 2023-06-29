#!/usr/bin/python3
# from scipy.optimize import curve_fit
import matplotlib.pyplot as plt
import numpy as np
import openpyxl

wb = openpyxl.load_workbook('1.xlsx', data_only=True)
print(wb.sheetnames)
sheet1 = wb['Sheet1']

columns = sheet1['C']
column_list_x = []
for c in columns:
    # print(c.value, end='\t')
    column_list_x.append(c.value)
print(column_list_x)

columns = sheet1['B']
column_list_y = []
for c in columns:
    # print(c.value, end='\t')
    column_list_y.append(c.value)
print(column_list_y)

z1 = np.polyfit(column_list_x, column_list_y, 3)  # 用3次多项式拟合，输出系数从高到0
p1 = np.poly1d(z1)  # 使用次数合成多项式

print(p1)
y_pre = p1(column_list_x)
plt.title(p1)
plt.plot(column_list_x, column_list_y, ".")
plt.plot(column_list_x, y_pre)
plt.show()
