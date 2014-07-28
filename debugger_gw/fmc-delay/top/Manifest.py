files = ["synthesis_descriptor.vhd", "spec_top_std.vhd", "spec_top.ucf", "spec_reset_gen.vhd"]

fetchto = "../fmc-delay-1ns-8cha/hdl/ip_cores"

modules = {
    "local" : ["../fmc-delay-1ns-8cha/hdl/rtl", 
	                      "../fmc-delay-1ns-8cha/hdl/platform",
	                      "../.."],
    "git" : [ "git://ohwr.org/hdl-core-lib/wr-cores.git",
					    "git://ohwr.org/hdl-core-lib/gn4124-core.git" ]
    }
