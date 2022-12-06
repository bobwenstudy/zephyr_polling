# define source directory
SRC		+= $(PLATFORM_PATH)

# define include directory
INCLUDE	+= $(PLATFORM_PATH)

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS += -lusb0

# Windows have diff cpu arch, ia64, amd64, x86
CPU_ARCH ?= x86
# define lib directory
#LIB		+= $(PLATFORM_PATH)/lib/amd64
#LIB		+= $(PLATFORM_PATH)/lib/ia64
LIB		+= $(PLATFORM_PATH)/lib/$(CPU_ARCH)
