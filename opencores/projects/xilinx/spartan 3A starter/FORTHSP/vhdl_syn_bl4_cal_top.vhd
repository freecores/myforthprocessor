-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_cal_to.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $	
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the instantiations cal_ctl and tap_dly.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_cal_top is
  port(
    clk         : in  std_logic;
    clk0dcmlock : in  std_logic;
    reset       : in  std_logic;
    tapForDqs   : out std_logic_vector(4 downto 0)
    );
end vhdl_syn_bl4_cal_top;

architecture arc of vhdl_syn_bl4_cal_top is
  attribute KEEP_HIERARCHY : string;

  component vhdl_syn_bl4_cal_ctl
    port (
      clk        : in  std_logic;
      reset      : in  std_logic;
      flop2      : in  std_logic_vector(31 downto 0);
      tapForDqs  : out std_logic_vector(4 downto 0)
      );
  end component;

  component vhdl_syn_bl4_tap_dly
    port (
      clk   : in  std_logic;
      reset : in  std_logic;
      tapIn : in  std_logic;
      flop2 : out std_logic_vector(31 downto 0)
      );
  end component;

  signal fpga_rst  : std_logic;
  signal flop2_val : std_logic_vector(31 downto 0);
  attribute KEEP_HIERARCHY of vhdl_syn_bl4_cal_ctl : component is "YES";
  attribute KEEP_HIERARCHY of vhdl_syn_bl4_tap_dly : component is "YES";
  
begin

  fpga_rst <= (not reset) or (not clk0dcmlock);

  cal_ctl0 : vhdl_syn_bl4_cal_ctl
    port map(
      clk        => clk,
      reset      => fpga_rst,
      flop2      => flop2_val,
      tapForDqs  => tapForDqs
      );

  tap_dly0 : vhdl_syn_bl4_tap_dly
    port map (
      clk   => clk,
      reset => fpga_rst,
      tapIn => clk,
      flop2 => flop2_val
      );
  
end arc;
