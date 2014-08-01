-------------------------------------------------------------------------------
-- Title      : Wishbone Debugger SDB descriptor
-- Project    : FMC DEL 1ns 4cha-stand-alone application (fmc-delay-1ns-4cha-sa)
-------------------------------------------------------------------------------
-- File       : synthesis_descriptor.vhd
-- Author     : Jose Jimenez <jjimenez.wr@gmail.com>
-- Company    : FREE INDEPENDENT ALLIANCE OF MAKERS (or looking for one)
-- Created    : 2014-07-31
-- Last update: 2014-07-36
-- Platform   : FPGA-generic
-- Standard   : VHDL'93
-------------------------------------------------------------------------------
-- Description: SDB descriptor for the WB Debugger and top level of the FMC used
--              on a SPEC carrier.
-- Contains synthesis & source repository information.
-- Warning: this file is modified whenever a synthesis is executed.
-------------------------------------------------------------------------------

library ieee;
use ieee.STD_LOGIC_1164.all;
use work.wishbone_pkg.all;

package synthesis_descriptor is
  
constant c_sdb_FMC_DEL_synthesis_info : t_sdb_synthesis :=
  (
    syn_module_name => "spec-fine-delay ",
    syn_commit_id => "00000000000000000000000000000000",
    syn_tool_name => "ISE     ",
    syn_tool_version => x"00000147",
    syn_date => x"20140731",
    syn_username => "jjimenez       ");

constant c_sdb_repo_url : t_sdb_repo_url :=
  (
    repo_url => "git://ohwr.org/fmc-projects/fmc-delay-1ns-8cha-sa.git          " 
  );

constant c_sdb_FMC_DEL_synthesis_info : t_sdb_synthesis :=
  (
    syn_module_name => "wb-debugger     ",
    syn_commit_id => "00000000000000000000000000000000",
    syn_tool_name => "ISE     ",
    syn_tool_version => x"00000147",
    syn_date => x"20140731",
    syn_username => "jjimenez       ");

end package synthesis_descriptor;
