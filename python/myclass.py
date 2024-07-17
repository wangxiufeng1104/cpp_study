#!/usr/bin/python3
class MyClass:
    """一个简单的类实例"""
    def __init__(self, realpart, imagpart):
        self.r = realpart
        self.i = imagpart
    def f(self):
        return 'hello world'




y = MyClass(3.0, -4.5)
print("MyClass的类属性 i 为:", y.i)
print("MyClass的类属性 i 为:", y.r)
print("MyClass的类方法 f 输出为:", y.f())
