

RM := rm -rf
# All of the sources par
# All Target
all: gpio-extender

# Tool invocations
gpio-extender: $(OBJS) $(USER_OBJS)
	 arm-linux-gnueabi-gcc spidev_util.c -o gpio-extender -I/home/doron-linux/workspace/var_som_mx6_linux/VAR-MX6-ub/linux-imx/include

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) gpio-extender

.PHONY: all clean dependents
.SECONDARY:

