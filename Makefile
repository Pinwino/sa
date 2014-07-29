DBG_SW = ./debugger_sw
DBG_GW = ./debugger_gw
WRPC_DIR = $(DBG_SW)/wrpc-sw
BITSTREAM_DIR = $(DBG_GW)/fmc-delay/spec

all: wrpc dbg fd_std gateware
	@test -e $(BITSTREAM_PATH) || cp $(BITSTREAM_PATH) ./

fd_std:
	$(MAKE) -C $(DBG_SW) fd_defconfig
	$(MAKE) -C $(DBG_SW) $@.ram 
	@cp -f $(DBG_SW)/$@.ram ./

dbg:
	$(MAKE) -C $(DBG_SW) defconfig
	$(MAKE) -C $(DBG_SW) $@.ram 
	@cp -f $(DBG_SW)/$@.ram ./
	
wrpc:
	$(MAKE) -C $(DBG_SW) $@
	
install:
	sudo $(MAKE) -C $(DBG_SW) $@
	
gateware: wrpc
	@test -e $(DBG_SW)/dbg.ram 		&& cp -f $(DBG_SW)/dbg.ram    $(DBG_GW)/
	@test -e $(DBG_SW)/fd_std.ram 	&& cp -f $(DBG_SW)/fd_std.ram $(DBG_GW)/
	@test -e $(DBG_SW)/wrc-ethb.ram	&& \
	cp -f $(DBG_SW)/wrc-ethb.ram $(BITSTREAM_DIR)/
	$(MAKE) -C $(DBG_GW)
	@cp $(BITSTREAM_DIR)/spec_top_std.bit ./

clean:
	rm -f *.bit *.ram
	$(MAKE) -C $(DBG_SW) clean
	$(MAKE) -C $(DBG_GW) clean
