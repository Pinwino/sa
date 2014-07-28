DBG_SW = ./debugger_sw 
DBG_GW = ./debugger_gw 

all: software

software:
	$(MAKE) -C $(DBG_SW) fd_defconfig
	$(MAKE) -C $(DBG_SW)
	
gateware: software
	$(MAKE) -C $(DBG_GW)
