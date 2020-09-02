
def add(a,b):
    print("agr1 %d agr2 %s" %(a,b))
    return a,"hi",(1,23)

# print('add:',add(1,"good"))
# print('test',hex(12))

def hy_down(strVal,strLen):
    if strVal[0]==1:
        print("==")
    else:
        print("!=")
    print("%s,%d" %(strVal,strLen))
    bstr=bytes(strVal,'ascii')
    print(bstr)
    lstr=list(bstr)
    lstr[0]=lstr[0]+1
    print(lstr,bytes(lstr).decode())
    return bytes(lstr).decode(),strLen

print(hy_down('\x01\x02\x03\x04a',3))