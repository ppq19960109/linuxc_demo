#-------------------------------------------------
#
# Project created by QtCreator 2021-05-27T09:01:43
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qt_ffmpeg
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    playthread.cpp \
    dec_video.c

HEADERS  += mainwindow.h \
    playthread.h \
    dec_video.h

FORMS    += mainwindow.ui

#INCLUDEPATH +=/home/ppq/git/linuxc_demo/ffmpeg/libs/libffmpeg/include
#LIBS += -L/home/ppq/git/linuxc_demo/ffmpeg/libs/libffmpeg/lib -lavcodec	-lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

#INCLUDEPATH +=/home/ppq/git/linuxc_demo/ffmpeg/libs/libx264/include
#LIBS += -L/home/ppq/git/linuxc_demo/ffmpeg/libs/libx264/lib -lx264

INCLUDEPATH +=/home/ppq/libs/ffmpeg-4.2.4/_install/include
LIBS += -L/home/ppq/libs/ffmpeg-4.2.4/_install/lib -lavcodec	-lavdevice -lavfilter -lavformat -lavutil -lpostproc -lswresample -lswscale

INCLUDEPATH +=/home/ppq/libs/x264-master/_install/include
LIBS += -L/home/ppq/libs/x264-master/_install/lib -lx264
