#/bin/sh
cd /userdata
killall hy_server_iot
rm -f hy_server_iot
wget -O hy_server_iot http://www.yyeting.xyz:9100/HYProjectManagement/download/?file=hy_server_iot
chmod 777 hy_server_iot
./hy_server_iot