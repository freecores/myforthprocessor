-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_clk_dcm.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:36 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
--
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     :     This module generates the system clock for controller block
--                    This also generates the recapture clock, clock for the
--                    Refresh counter and also for the data path
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use UNISIM.VCOMPONENTS.all;
entity vhdl_syn_bl4_clk_dcm is
  port(
    input_clk : in  std_logic;
    rst       : in  std_logic;
    clk       : out std_logic;
    clk90     : out std_logic;
    dcm_lock  : out std_logic
    );
end vhdl_syn_bl4_clk_dcm;

architecture arc of vhdl_syn_bl4_clk_dcm is


begin

  clk   <= input_clk;
  clk90 <= input_clk after 8.5 ns;
  dcm_lock <= '1';
end arc;
