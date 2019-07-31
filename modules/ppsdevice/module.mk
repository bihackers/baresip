#
# module.mk
#
# Copyright (C) 2010 Creytiv.com
#

MOD		:= ppsdevice
$(MOD)_SRCS	+= ppsdevice.c cJSON.c ppcs.c md5.c
$(MOD)_LFLAGS	+= -lPPCS_API
include mk/mod.mk