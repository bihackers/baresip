#
# module.mk
#
# Copyright (C) 2010 Creytiv.com
#

MOD		:= ppsdevice
$(MOD)_SRCS	+= ppsdevice.c
$(MOD)_LFLAGS	+= -lPPCS_API
include mk/mod.mk