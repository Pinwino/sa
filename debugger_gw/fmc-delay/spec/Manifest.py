target = "xilinx"
action = "synthesis"

fetchto = "../fmc-delay-1ns-8cha/hdl/ip_cores"

syn_device = "xc6slx100t"
syn_grade = "-3"
syn_package = "fgg484"
syn_top = "spec_top_std"
syn_project = "spec_fine_delay.xise"

modules = { "local" : [ "../top", "../fmc-delay-1ns-8cha/hdl/platform", "../../etherbone-core"] }

files = "wrc-ethb.ram"
