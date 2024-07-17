#!/usr/bin/python
import json
test_dict = {'one':1, 'two':{2.1:['a', 'b']}}

print(test_dict)
print(type(test_dict))

#dumps 将数据转换成字符串
json_str = json.dumps(test_dict)
print(json_str)
print(type(json_str))
