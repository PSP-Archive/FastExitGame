TARGET = FastExitGame
OBJS = main.o syslibc/syslibc.o syslibc/strncasecmp.o

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = 
CFLAGS = -O2 -G0 -Wall -fno-builtin-printf
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = 

LIBS = -lpsppower -lpspsystemctrl_kernel -lpspkubridge libinip.a
# -lpspsystemctrl_user

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

