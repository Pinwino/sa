MAKE_DIR       = ./spec
FMC_DEL_DIR    = ./fmc-delay-1ns-8cha
GRAL_CORES_DIR = $(FMC_DEL_DIR)/hdl/ip_cores/general-cores
WR_CORES_DIR   = $(FMC_DEL_DIR)/hdl/ip_cores/wr-cores
GN_4124_DIR    = $(FMC_DEL_DIR)/hdl/ip_cores/gn4124-core


all: git_submodules build_syn_makefile
	$(MAKE) -C $(MAKE_DIR) -f Makefile

git_submodules:
	@cd $(FMC_DEL_DIR); \
	@test -d $(GRAL_CORES_DIR)/syn || git submodule update --init ; \
	@test -d $(WR_CORES_DIR)/syn   || git submodule update --init ; \
	@test -d $(GN_4124_DIR)/syn    || git submodule update --init

build_syn_makefile:
	@cd $(MAKE_DIR); \
	hdlmake --make-ise --ise-proj
	
clean:
	$(MAKE) -C $(FMC_DEL_DIR)/spec -f Makefile $@

mrproper:
	$(MAKE) -C $(FMC_DEL_DIR)/spec -f Makefile $@
