ETHERBONE_DIR  = ./etherbone-core
FMC_DEL_DIR    = ./fmc-delay



all: git_submodules
	$(MAKE) -C $(FMC_DEL_DIR) -f Makefile

git_submodules:
	@test -d $(ETHERBONE_DIR)/api  || git submodule update --init
	
clean:
	$(MAKE) -C $(FMC_DEL_DIR)/spec -f Makefile $@

mrproper:
	$(MAKE) -C $(FMC_DEL_DIR)/spec -f Makefile $@
