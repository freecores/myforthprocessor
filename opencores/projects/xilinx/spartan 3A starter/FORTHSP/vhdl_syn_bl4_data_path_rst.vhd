-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_data_path_rst.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:36 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the FD instantiations to various reset signals.
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
library UNISIM;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_data_path_rst is
  port(
    clk        : in  std_logic;
    reset      : in  std_logic;
    reset_r    : out std_logic
    );
end vhdl_syn_bl4_data_path_rst;

architecture arc of vhdl_syn_bl4_data_path_rst is

begin

--***********************************************************************
-- Reset flip-flops
--***********************************************************************

  rst0_r : FD
    port map (
      Q => reset_r,
      C => clk,
      D => reset
      );

end arc;



