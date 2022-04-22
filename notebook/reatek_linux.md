11b模式
TX_11B
11b 发射模式 5.5M=11，Bw=20MHz

ifconfig wlan0 up 
rtwpriv wlan0 mp_start 
（1）频道 1 
rtwpriv wlan0 mp_channel 1 
rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0 
rtwpriv wlan0 mp_ant_tx a
rtwpriv wlan0 mp_rate 22
rtwpriv wlan0 mp_ctx count=%100,pkt
rtwpriv wlan0 mp_ctx stop
（2）频道 7
rtwpriv wlan0 mp_channel 7
rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0 
rtwpriv wlan0 mp_ant_tx a
rtwpriv wlan0 mp_rate 22
rtwpriv wlan0 mp_ctx count=%100,pkt
rtwpriv wlan0 mp_ctx stop
（3）频道 13
rtwpriv wlan0 mp_channel 13 
rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0 
rtwpriv wlan0 mp_ant_tx a
rtwpriv wlan0 mp_txpower patha=47
rtwpriv wlan0 mp_rate 22
rtwpriv wlan0 mp_ctx count=%100,pkt
rtwpriv wlan0 mp_ctx stop


11g 发射模式 54M=108，Bw=20MHz

ifconfig wlan0 up 
rtwpriv wlan0 mp_start 
(1)频道 1 
rtwpriv wlan0 mp_channel 1 
rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0 
rtwpriv wlan0 mp_ant_tx a
rtwpriv wlan0 mp_rate 108
rtwpriv wlan0 mp_ctx count=%100,pkt
rtwpriv wlan0 mp_ctx stop

(2)频道 7
rtwpriv wlan0 mp_channel 7
rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0 
rtwpriv wlan0 mp_ant_tx a
rtwpriv wlan0 mp_rate 108
rtwpriv wlan0 mp_ctx count=%100,pkt
rtwpriv wlan0 mp_ctx stop
(3)频道 13
rtwpriv wlan0 mp_channel 13 
rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0 
rtwpriv wlan0 mp_ant_tx a
rtwpriv wlan0 mp_rate 108
rtwpriv wlan0 mp_ctx count=%100,pkt
rtwpriv wlan0 mp_ctx stop

11n 发射模式 MCS7=135步骤
rtwpriv wlan0 mp_pwrctldm start  //温补
rtwpriv wlan0 mp_pwrctldm stop
rtwpriv wlan0 mp_get_txpower

ifconfig wlan0 up 
rtwpriv wlan0 mp_start
MCS7 11n模式 20M带宽 
(1)频道 1 
rtwpriv wlan0 mp_channel 1 
rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0 
rtwpriv wlan0 mp_ant_tx a
rtwpriv wlan0 mp_rate 135
rtwpriv wlan0 mp_ctx count=%100,pkt
rtwpriv wlan0 mp_ctx stop

(2)频道 7
rtwpriv wlan0 mp_channel 7
rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0 
rtwpriv wlan0 mp_ant_tx a
rtwpriv wlan0 mp_rate 135
rtwpriv wlan0 mp_ctx count=%100,pkt
rtwpriv wlan0 mp_ctx stop
(3)频道 13
rtwpriv wlan0 mp_channel 13 
rtwpriv wlan0 mp_bandwidth 40M=0,shortGI=0 
rtwpriv wlan0 mp_ant_tx a
rtwpriv wlan0 mp_rate 135
rtwpriv wlan0 mp_ctx count=%100,pkt
rtwpriv wlan0 mp_ctx stop
