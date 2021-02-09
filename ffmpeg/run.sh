export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/ppq/git/linuxc/ffmpeg/libs/libffmpeg/lib:/home/ppq/git/linuxc/ffmpeg/libs/libx264/lib:/home/ppq/lame-3.100/_install/lib:/home/ppq/xvidcore/build/generic/_install/lib:/home/ppq/fdk-aac-2.0.1/_install/lib:/home/ppq/x265_3.2.1/build/_install/lib
echo openfile:$1 $2 $3
./app $1 $2 $3