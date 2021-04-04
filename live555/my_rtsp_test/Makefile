# Hisilicon Hi35xx sample Makefile

include ../Makefile.param

SRCS := $(wildcard *.c) 
TARGET := my_rtsp_test
COMM_OBJ:= $(COMM_OBJ:%sample_comm_venc.o=)

# compile linux or HuaweiLite
include $(PWD)/../Make.$(OSTYPE)
