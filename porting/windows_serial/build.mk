# define source directory
SRC		+= $(PORT_PATH)

# define include directory
INCLUDE	+= $(PORT_PATH)

# define lib directory
LIB		+=

PLATFORM_ROOT_PATH := platform
INCLUDE	+= $(PLATFORM_ROOT_PATH)

PLATFORM_PATH := $(PLATFORM_ROOT_PATH)/windows
include $(PLATFORM_PATH)/build.mk
