-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_infrastructure_iobs_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the FDDRRSE instantiations to the clocks.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_infrastructure_iobs_0 is
  port(
        DDR2_CK   : out  std_logic;
    DDR2_CK_N : out  std_logic;
    clk0  : in std_logic
    );
end vhdl_syn_bl4_infrastructure_iobs_0;

architecture arc of vhdl_syn_bl4_infrastructure_iobs_0 is

    signal ddr2_clk0_q     : std_logic;
  signal vcc : std_logic;
  signal gnd : std_logic;
  signal clk180 : std_logic;



---- **************************************************
---- iob attributes for instantiated FDDRRSE components
---- **************************************************
begin

  gnd <= '0';
  vcc <= '1';
  clk180 <= not clk0;


--- ***********************************
---- This includes instantiation of the output DDR flip flop
---- for ddr clk's and dimm clk's
---- ***********************************************************

    U0 : FDDRRSE 
  port map (
    Q => ddr2_clk0_q,
    C0 => clk0,
    C1 => clk180,
    CE => vcc,
    D0 => vcc,
    D1 => gnd,
    R => gnd,
    S => gnd
    );



---- ******************************************
---- Ouput BUffers for ddr clk's and dimm clk's
---- ******************************************

  
      r0 : OBUFDS 
	  port map (
        I  => ddr2_clk0_q,
        O  => DDR2_CK,
        OB => DDR2_CK_N
		);


end arc;
