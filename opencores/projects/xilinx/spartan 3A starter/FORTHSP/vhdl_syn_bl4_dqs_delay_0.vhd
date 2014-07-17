-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_dqs_delay_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2006/12/27 06:38:55 $	
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module generate the delay in the dqs signal.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_dqs_delay is
  port (
    clk_in  : in  std_logic;
    sel_in  : in  std_logic_vector(4 downto 0);
    clk_out : out std_logic
    );
end vhdl_syn_bl4_dqs_delay;

architecture arc_dqs_delay of vhdl_syn_bl4_dqs_delay is

  signal delay1 : std_logic;
  signal delay2 : std_logic;
  signal delay3 : std_logic;
  signal delay4 : std_logic;
  signal delay5 : std_logic;
  signal high   : std_logic;
    attribute syn_noprune  : boolean;
  attribute syn_noprune of  one: label is true;
  attribute syn_noprune of  two: label is true;
  attribute syn_noprune of  three: label is true;
  attribute syn_noprune of  four: label is true;
  attribute syn_noprune of  five: label is true;
  attribute syn_noprune of  six: label is true;


begin

  high <= '1';

  one : LUT4 generic map (INIT => x"f3c0")
    port map (I0  => high,
               I1 => sel_in(4),
               I2 => delay5,
               I3 => clk_in,
               O  => clk_out
               );


  two : LUT4 generic map (INIT => x"ee22")
    port map (
      I0 => clk_in,
      I1 => sel_in(2),
      I2 => high,
      I3 => delay3,
      O  => delay4
      );

  three : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => clk_in,
      I1 => sel_in(0),
      I2 => delay1,
      I3 => high,
      O  => delay2
      );

  four : LUT4 generic map (INIT => x"ff00")
    port map (
      I0 => high,
      I1 => high,
      I2 => high,
      I3 => clk_in,
      O  => delay1
      );

  five : LUT4 generic map (INIT => x"f3c0")
    port map (
      I0 => high,
      I1 => sel_in(3),
      I2 => delay4,
      I3 => clk_in,
      O  => delay5
      );

  six : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => clk_in,
      I1 => sel_in(1),
      I2 => delay2,
      I3 => high,
      O  => delay3
      );

end arc_dqs_delay;
