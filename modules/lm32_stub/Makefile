TARGET     = myproject
MYPATH     = .
CHECKOUT   = <PATH-TO-MY-BEL_PROJECTS-CHECKOUT>
MYSRC      = $(MYPATH)/main.c
RAMSIZE    = 64K # must be less or equal to synthesized RAM size 
STACKSIZE  = 8K  # size of the stack, 8K recommended
SHAREDSIZE = 8K  # size of fixed memory range for data exchange with the host system

MYSYM      = -DPASS_MY_VAR=42
MYFLAGS    = $(MYSYM)

include $(CHECKOUT)/modules/lm32-include/build.mk
