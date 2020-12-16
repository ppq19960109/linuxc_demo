LIBA_TARGET     := libiot_alink.a

HDR_REFS        += src/infra
HDR_REFS		+= src/mqtt
HDR_REFS		+= src/dev_sign
HDR_REFS		+= src/dev_reset
HDR_REFS		+= src/ota

DEPENDS         += wrappers
LDFLAGS         += -liot_sdk -liot_hal -liot_tls

LIB_SRCS_PATTERN     	:= *.c server/*.c client/*.c deprecated/*.c

LIB_SRCS_EXCLUDE     	  := examples/linkkit_example_solo.c examples/cJSON.c
SRCS_linkkit-example-solo := examples/linkkit_example_solo.c examples/cJSON.c

LIB_SRCS_EXCLUDE             += examples/linkkit_example_gateway.c examples/cJSON.c
SRCS_linkkit-example-gateway := examples/linkkit_example_gateway.c examples/cJSON.c

$(call Append_Conditional, LIB_SRCS_PATTERN, alcs/*.c, ALCS_ENABLED)

# $(call Append_Conditional, TARGET, linkkit-example-solo, DEVICE_MODEL_ENABLED, BUILD_AOS NO_EXECUTABLES)
# $(call Append_Conditional, TARGET, linkkit-example-gateway, DEVICE_MODEL_GATEWAY, BUILD_AOS NO_EXECUTABLES)


BASE_PATH=src/dev_model/alink

CFLAGS+=-I$(BASE_PATH)/hylink -I$(BASE_PATH)/cJSON -I$(BASE_PATH)/klib
CFLAGS+= -Wall -DUSE_LIBEVENT=0

LDFLAGS += -L$(BASE_PATH)/hylink/lib
LDFLAGS	+= -Wl,--start-group	\
		-Wl,-Bstatic -lhylink	\
		-Wl,-Bdynamic \
		-Wl,--end-group

SOURCE_FILES:= alink/src/*.c alink/src/cloudlink/*c

LIB_SRCS_EXCLUDE         += $(SOURCE_FILES) 
SRCS_alinkapp := $(SOURCE_FILES)
$(call Append_Conditional, TARGET, alinkapp, DEVICE_MODEL_GATEWAY, BUILD_AOS NO_EXECUTABLES)


