# define source directory
SRC		+= $(PORT)

# define include directory
INCLUDE	+= $(PORT)

# define lib directory
LIB		+=

PLATFORM := platform/windows

include $(PLATFORM)/build.mk
