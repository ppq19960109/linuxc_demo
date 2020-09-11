import binascii
def up(inVal,inLen):
    hexVal=binascii.unhexlify('56')
    ret=bytes(inVal).decode('utf-16BE').encode('utf-8')
    return tuple(ret)

print('test:',up((0x56,0xde,0x5b,0xb6)))
