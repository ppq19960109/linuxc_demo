export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:libs/libffmpeg/lib:libs/libx264/lib:libs/liblame/lib:libs/libxvid/lib:libs/libaac/lib:libs/libx265/lib:/home/ppq/openssl-1.0.1u/_install/lib:libs/librtmp/lib
echo run:$1 $2 $3
./app $1 $2 $3