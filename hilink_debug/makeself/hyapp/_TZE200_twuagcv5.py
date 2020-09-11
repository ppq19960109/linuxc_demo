# -*- coding: UTF-8 -*-


def up(inValue, valueLen):
	out1 = "Unknow"
	len1 = 6
	out2 = "0"
	len2 = 1
	# 新风开关
	if inValue[4:6] == '66':
		out1 = 'Switch_2'
		len1 = 8
		if inValue[12:] == '00':
			out2 = '0'
			len2 = 1
		else:
			out2 = '1'
			len2 = 1
	# 新风风速
	elif inValue[4:6] == '67':
		out1 = 'WindSpeed_2'
		len1 = 11
		if inValue[12:] == '00':
			out2 = '2'
			len2 = 1
		elif inValue[12:] == '01':
			out2 = '3'
			len2 = 1
		else: 
			out2 = '4'
			len2 = 1
	# 空调开关
	elif inValue[4:6] == '68':
		out1 = 'Switch_1'
		len1 = 8
		if inValue[12:] == '00':
			out2 = '0'
			len2 = 1
		else:
			out2 = '1'
			len2 = 1
	# 当前温度
	elif inValue[4:6] == '69':
		out1 = 'CurrentTemperature_1'
		len1 = 20
		out2 = str(int(inValue[18:], 16))
		len2 = len(out2)
	# 设置空调温度
	elif inValue[4:6] == '6A':
		out1 = 'TargetTemperature_1'
		len1 = 19
		out2 = str(int(inValue[18:], 16))
		len2 = len(out2)
	# 空调工作模式
	elif inValue[4:6] == '6B':
		out1 = 'WorkMode_1'
		len1 = 11
		if inValue[12:] == '01':
			out2 = '2'
			len2 = 1
		elif inValue[12:] == '02':
			out2 = '4'
			len2 = 1
		elif inValue[12:] == '03':
			out2 = '3'
			len2 = 1
		else: 
			out2 = '1'
			len2 = 1
	# 地暖开关
	elif inValue[4:6] == '6C':
		out1 = 'Switch_3'
		len1 = 8
		if inValue[12:] == '00':
			out2 = '0'
			len2 = 1
		else:
			out2 = '1'
			len2 = 1
	# 当前地暖温度
	elif inValue[4:6] == '6D':
		out1 = 'TargetTemperature_3'
		len1 = 19
		out2 = str(int(inValue[18:], 16))
		len2 = len(out2)
	# 设置地暖温度
	elif inValue[4:6] == '6E':
		out1 = 'TargetTemperature_3'
		len1 = 19
		out2 = str(int(inValue[18:], 16))
		len2 = len(out2)
	# 空调风速
	elif inValue[4:6] == '6F':
		out1 = 'WindSpeed_1'
		len1 = 11
		if inValue[12:] == '00':
			out2 = '2'
			len2 = 1
		elif inValue[12:] == '01':
			out2 = '3'
			len2 = 1
		else: 
			out2 = '4'
			len2 = 1
	# 场景图片
	elif inValue[4:6] == '71':
		out1 = 'ScePhoto_'+str(inValue[7])
		len1 = len(out1)
		out2 = str(int(inValue[17:], 16))
		len2 = len(out2)
	# 空调使能
	elif inValue[4:6] == '72':
		out1 = 'Enable_1'
		len1 = 8
		if inValue[12:] == '00':
			out2 = '0'
			len2 = 1
		else:
			out2 = '1'
			len2 = 1
	# 新风使能
	elif inValue[4:6] == '73':
		out1 = 'Enable_2'
		len1 = 8
		if inValue[12:] == '00':
			out2 = '0'
			len2 = 1
		else:
			out2 = '1'
			len2 = 1
	# 地暖使能
	elif inValue[4:6] == '74':
		out1 = 'Enable_3'
		len1 = 8
		if inValue[12:] == '00':
			out2 = '0'
			len2 = 1
		else:
			out2 = '1'
			len2 = 1
	# 场景名称
	elif inValue[4:6] == '75':
		pass
	# 场景按键上报
	elif inValue[4:6] == '77':
		out1 = 'KeyFobValue'
		len1 = 11
		out2 = str(1+int(inValue[12:], 16))
		len2 = len(out2)
	else:
		pass
	return out1, len1, out2, len2


def down(inKey, keyLen, inValue, valueLen):
	out2 = b'\x00'
	len2 = 1
	print(11111)
	# 新风开关
	if inKey == 'Switch_2':
		if inValue=='0' or inValue=='00':
			out2 = '66'+'01'+'00'+'01'+'00'
		else:
			out2 = '66' + '01' + '00' + '01' + '01'
		len2 = 10
	# 新风风速
	elif inKey == 'WindSpeed_2':
		value='01'
		if inValue == '2':
			value = '00'
		elif inValue == '3':
			value = '01'
		else:
			value = '02'
		out2 = '67'+'01'+'00'+'01'+value
		len2 = 10
	# 空调开关
	elif inKey == 'Switch_1':
		if inValue == '0' or inValue == '00':
			out2 = '68'+'01'+'00'+'01'+'00'
		else:
			out2 = '68' + '01' + '00' + '01' + '01'
		len2 = 10
	# 当前温度
	elif inKey == 'CurrentTemperature_1':
		out2 = '69'+'02'+'00'+'04'+'00'+'00'+'00'+str(int(inValue))
		len2 = 16
	# 设置空调温度
	elif inKey == 'TargetTemperature_1':
		tmp = str(hex(int(inValue)))[2:]
		out2 = '6a'+'02'+'00'+'04'+'00'+'00'+'00'+tmp
		len2 = 16
	# 空调工作模式
	elif inKey == 'WorkMode_1':
		value = '00'
		if inValue == '1':
			value = '00'
		elif inValue == '2':
			value = '01'
		elif inValue == '3':
			value = '03'
		elif inValue == '4':
			value = '02'
		out2 = '6b'+'01'+'00'+'01'+value
		len2 = 10
	# 地暖开关
	elif inKey == 'Switch_3':
		if inValue == '0' or inValue == '00':
			out2 = '6c'+'01'+'00'+'01'+'00'
		else:
			out2 = '6c' + '01' + '00' + '01' + '01'
		len2 = 10
	# 当前地暖温度
	elif inKey == 'CurrentTemperature_3':
		pass
	# 设置地暖温度
	elif inKey == 'TargetTemperature_3':
		tmp = str(hex(int(inValue)))[2:]
		out2 = '6e'+'02'+'00'+'04'+'00'+'00'+'00'+tmp
		len2 = 16
	# 空调风速
	elif inKey == 'WindSpeed_1':
		if inValue == '2':
			value = '00'
		elif inValue == '3':
			value = '01'
		else:
			value = '02'
		out2 = '6f'+'01'+'00'+'01'+value
		len2 = 10
	# 空调使能
	elif inKey == 'Enable_1':
		out2 = '72'+'01'+'00'+'01'+str(hex(int(inValue)))[2:]
		len2 = 10
	# 新风使能
	elif inKey == 'Enable_2':
		out2 = '73'+'01'+'00'+'01'+str(hex(int(inValue)))[2:]
		len2 = 10
	# 地暖使能
	elif inKey == 'Enable_3':
		out2 = '74'+'01'+'00'+'01'+str(hex(int(inValue)))[2:]
		len2 = 10
	# 场景1图片
	elif inKey == 'ScePhoto_1':
		out2 = '71'+'00'+'00'+'04'+'00'+'00'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景2图片
	elif inKey == 'ScePhoto_2':
		out2 = '71'+'00'+'00'+'04'+'00'+'01'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景3图片
	elif inKey == 'ScePhoto_3':
		out2 = '71'+'00'+'00'+'04'+'00'+'02'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景4图片
	elif inKey == 'ScePhoto_4':
		out2 = '71'+'00'+'00'+'04'+'00'+'03'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景5图片
	elif inKey == 'ScePhoto_5':
		out2 = '71'+'00'+'00'+'04'+'00'+'04'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景6图片
	elif inKey == 'ScePhoto_6':
		out2 = '71'+'00'+'00'+'04'+'00'+'05'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景7图片
	elif inKey == 'ScePhoto_7':
		out2 = '71'+'00'+'00'+'04'+'00'+'06'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景8图片
	elif inKey == 'ScePhoto_8':
		out2 = '71'+'00'+'00'+'04'+'00'+'07'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景9图片
	elif inKey == 'ScePhoto_9':
		out2 = '71'+'00'+'00'+'04'+'00'+'08'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景10图片
	elif inKey == 'ScePhoto_10':
		out2 = '71'+'00'+'00'+'04'+'00'+'09'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景11图片
	elif inKey == 'ScePhoto_11':
		out2 = '71'+'00'+'00'+'04'+'00'+'0a'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景12图片
	elif inKey == 'ScePhoto_12':
		out2 = '71'+'00'+'00'+'04'+'00'+'0b'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景13图片
	elif inKey == 'ScePhoto_13':
		out2 = '71'+'00'+'00'+'04'+'00'+'0c'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景14图片
	elif inKey == 'ScePhoto_14':
		out2 = '71'+'00'+'00'+'04'+'00'+'0d'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景15图片
	elif inKey == 'ScePhoto_15':
		out2 = '71'+'00'+'00'+'04'+'00'+'0e'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景16图片
	elif inKey == 'ScePhoto_16':
		out2 = '71'+'00'+'00'+'04'+'00'+'0f'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景17图片
	elif inKey == 'ScePhoto_17':
		out2 = '71'+'00'+'00'+'04'+'00'+'10'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景18图片
	elif inKey == 'ScePhoto_18':
		out2 = '71'+'00'+'00'+'04'+'00'+'11'+'00'+str(hex(int(inValue)))[2:]
		len2 = 16
	# 场景名称
	elif inKey == 'SceneName':
		pass
	# 场景按键上报
	elif inKey == 'KeyFobValue':
		out2 = '77'+'04'+'00'+'01'+str(hex(int(inValue)))[2:]
		len2 = 10
	return out2, len2


if __name__ == '__main__':
	[out3, len3, out4, len4] = up('00037701000100', 7)
	print(out3)
	[out5, len5] = down('Switch_1', 8,'1',1)
	print(out5,len5)

	print(str(hex(int('16')))[2:])