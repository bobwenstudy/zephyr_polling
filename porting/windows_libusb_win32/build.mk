# define source directory
SRC		+= $(PORT_PATH)

# define include directory
INCLUDE	+= $(PORT_PATH)

# define lib directory
LIB		+=

PLATFORM_PATH := platform/windows

include $(PLATFORM_PATH)/build.mk
