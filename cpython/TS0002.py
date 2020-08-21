

def Switch_1_up(s, len):
    print(type(s), s)
    if s == 0:
        s = '0'
        len = 2
    else:
        s = '3'
        len = 3
    print(type(s), s)
    return s, len


def Switch_1_down(s, len):
    print(s, len)
    if s == '0':
        s = 0
        len = 2
    else:
        s = 1
        len = 3
    print(type(s), s)
    return s, len

# print('test:',Switch_1_down("0",1))
# print('test:',Switch_1_up(1,1))
