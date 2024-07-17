#!/usr/bin/env python

import os

# 当前文件名
print(__file__)

# 当前文件名的绝对路径
print( os.path.abspath(__file__) )

# 返回当前文件的路径
print(os.path.dirname( os.path.abspath(__file__) ))

"""
python3 ./os_path.py 
/home/arno/work/python_learn/./os_path.py
/home/arno/work/python_learn/os_path.py
/home/arno/work/python_learn
"""

 
print( os.path.basename('/root/runoob.txt') )   # 返回文件名
print( os.path.dirname('/root/runoob.txt') )    # 返回目录路径
print( os.path.split('/root/runoob.txt') )      # 分割文件名与路径
print( os.path.join('root','test','runoob.txt') )  # 将目录和文件名合成一个路径
"""
runoob.txt
/root
('/root', 'runoob.txt')
root/test/runoob.txt
"""