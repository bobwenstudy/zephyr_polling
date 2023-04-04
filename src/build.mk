# define source directory
SRC		+= $(BLUETOOTH)
SRC		+= $(BLUETOOTH)/base
SRC		+= $(BLUETOOTH)/bluetooth
SRC		+= $(BLUETOOTH)/common
SRC		+= $(BLUETOOTH)/crypto
SRC		+= $(BLUETOOTH)/drivers
SRC		+= $(BLUETOOTH)/host
SRC		+= $(BLUETOOTH)/logging
SRC		+= $(BLUETOOTH)/utils
SRC		+= $(BLUETOOTH)/services
SRC		+= $(BLUETOOTH)/tinycrypt

# define include directory
INCLUDE	+= $(BLUETOOTH)

# define lib directory
LIB		+=