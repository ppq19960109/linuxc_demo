


export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/lib:/usr/lib:/app/run/appexe
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/app/run/appexe
echo $LD_LIBRARY_PATH
Install the new version.
Start the new version.
Successful installation!
opkg remove hy_manage
rm /etc/rc.d/S99hy_manage_init

iw wlan0 connect HUAWEI-WDNJ4L keys d:0:1234567890
iw wlan0 connect GUEST keys d:0:hongyana11
iwconfig wlan0 essid "GUEST" key hongyana11

set wireless.radio0=wifi-device
config wifi-device 'radio0'
        option type 'mac80211'
        option channel '11'
        option hwmode '11g'
        option path 'platform/10300000.wmac'
        option htmode 'HT20'
        option disabled '0'
        option country '00'
        option legacy_rates '1'

config wifi-iface 'default_radio0'
        option device 'radio0'
        option network 'lan'
        option mode 'ap'
        option ssid 'HONYAR-DA2A'
        option encryption 'psk2'
        option key '87654321'

config wifi-iface
        option network 'wwan'
        option ssid 'HUAWEI-WDNJ4L'
        option encryption 'psk2'
        option device 'radio0'
        option mode 'sta'
        option bssid '00:9A:CD:85:CB:8C'
        option key '1234567890'
uci revert wireless	

uci set wireless.@wifi-iface[0].hidden=0
uci set wireless.@wifi-iface[0].hidden=1
uci commit wireless && wifi
uci set wireless.@wifi-iface[1].ssid=GUEST
uci set wireless.@wifi-iface[1].key=hongyana11
uci set wireless.@wifi-iface[1].bssid=10:0E:0E:20:C3:F5
uci delete wireless.radio0.channel 
uci set wireless.radio0.channel=auto
 
uci delete wireless.@wifi-iface[1]
uci add wireless wifi-iface	
uci set wireless.@wifi-iface[1].network=wwan
uci set wireless.@wifi-iface[1].encryption=psk2
uci set wireless.@wifi-iface[1].device=radio0
uci set wireless.@wifi-iface[1].mode=sta
uci set wireless.@wifi-iface[1].ssid=HUAWEI-WDNJ4L
uci set wireless.@wifi-iface[1].key=1234567890
uci commit wireless && wifi

uci set wireless.@wifi-iface[1].ssid=瑷颐湾12-15
uci set wireless.@wifi-iface[1].key=17300925237

uci set wireless.@wifi-iface[1].ssid=2262
uci set wireless.@wifi-iface[1].key=12345678
uci set wireless.@wifi-iface[1].disabled=0
uci commit wireless && wifi

uci set wireless.@wifi-iface[1]=wifi-iface		
uci set wireless.cainiao.network=wwan
uci set wireless.cainiao.encryption=psk2
uci set wireless.cainiao.device=radio0
uci set wireless.cainiao.mode=sta
uci set wireless.cainiao.ssid=HUAWEI-WDNJ4L
uci set wireless.cainiao.key=1234567890
uci commit wireless && wifi
uci export wireless
uci delete wireless.@wifi-iface[1].bssid
uci set wireless.@wifi-iface[1].disabled=0
uci commit wireless && wifi
uci set wireless.@wifi-iface[1].bssid=10:0E:0E:20:C3:F5
uci set wireless.@wifi-iface[1].ssid=ZNSYS
uci set wireless.@wifi-iface[1].key=hongyana11
{
    "action":"wifi_config_request",
    "requestId": "",
    "apiVersion":"1.0",
    "params":{
        "bssid":"",
        "ssid" :"瑷颐湾12-15",
        "pwd": "17300925237" 
    }
}

{
    "action":"wifi_config_request",
    "requestId": "",
    "apiVersion":"1.0",
    "params":{
        "bssid":"",
        "ssid" :"Xiaomi_74DC",
        "pwd": "cnyz1234" 
    }
}
{
    "action":"wifi_config_request",
    "requestId": "",
    "apiVersion":"1.0",
    "params":{
        "bssid":"",
        "ssid" :"ZNSYS",
        "pwd": "hongyana11" 
    }
}
	
{
    "action":"wifi_config_request",
    "requestId": "",
    "apiVersion":"1.0",
    "params":{
        "bssid":"",
        "ssid" :"HUAWEI-WDNJ4L",
        "pwd": "1234567890" 
    }
}

{
    "action":"wifi_config_request",
    "requestId": "",
    "apiVersion":"1.0",
    "params":{
        "bssid":"",
        "ssid" :"CAINIAOYZ_2.4G",
        "pwd": "linshituoguan" 
    }
}


iw dev wlan0 link | grep -i SSID | wc -l
cat /sys/class/net/wlan0/operstate
ping -c 1 114.114.114.114 > /dev/null 2>&1 && echo $?
ping -c 1 223.5.5.5 > /dev/null 2>&1 && echo $?
mount -o nolock,tcp -t nfs 192.168.1.177:/home/ppq/nfs /mnt
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mnt/i2ctool/lib

l *(0x0043c80c)
l *(0x0043c7e8)
ulimit -c 2048
dmesg | grep i2c*
coap://127.0.0.1:5683
./i2cdump -f -y 0 0x2a 
./i2cdetect -y 0
./i2ctransfer -f -y 0 w3@0x2a 0x01 0x01 0x00
./i2ctransfer -f -y 0 w3@0x2a 0x01 0x01 0x00 r16
./i2ctransfer -f -y 0 w1@0x2a 0x11 r16
./i2ctransfer -f -y 0 w3@0x54 0x01 0x01 0x00 r18
./i2ctransfer -f -y 0 w1@0x55 0x11 r16
./i2ctransfer -f -y 0 r16@0x55
echo 44 > export
echo out > direction
echo 1 > value

uci delete network.wwan
uci set network.wwan=interface
uci set network.wwan.proto=dhcp
uci commit network && wifi
/etc/init.d/network reload