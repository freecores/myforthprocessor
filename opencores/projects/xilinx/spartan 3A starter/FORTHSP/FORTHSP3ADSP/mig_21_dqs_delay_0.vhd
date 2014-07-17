-------------------------------------------------------------------------------
-- DISCLAIMER OF LIABILITY
-- 
-- This text/file contains proprietary, confidential
-- information of Xilinx, Inc., is distributed under license
-- from Xilinx, Inc., and may be used, copied and/or
-- disclosed only pursuant to the terms of a valid license
-- agreement with Xilinx, Inc. Xilinx hereby grants you a 
-- license to use this text/file solely for design, simulation, 
-- implementation and creation of design files limited 
-- to Xilinx devices or technologies. Use with non-Xilinx 
-- devices or technologies is expressly prohibited and 
-- immediately terminates your license unless covered by
-- a separate agreement.
--
-- Xilinx is providing this design, code, or information 
-- "as-is" solely for use in developing programs and 
-- solutions for Xilinx devices, with no obligation on the 
-- part of Xilinx to provide support. By providing this design, 
-- code, or information as one possible implementation of 
-- this feature, application or standard, Xilinx is making no 
-- representation that this implementation is free from any 
-- claims of infringement. You are responsible for 
-- obtaining any rights you may require for your implementation. 
-- Xilinx expressly disclaims any warranty whatsoever with 
-- respect to the adequacy of the implementation, including 
-- but not limited to any warranties or representations that this
-- implementation is free from claims of infringement, implied 
-- warranties of merchantability or fitness for a particular 
-- purpose.
--
-- Xilinx products are not intended for use in life support
-- appliances, devices, or systems. Use in such applications is
-- expressly prohibited.
--
-- Any modifications that are made to the Source Code are 
-- done at the user's sole risk and will be unsupported.
--
-- Copyright (c) 2006-2007 Xilinx, Inc. All rights reserved.
--
-- This copyright and support notice must be retained as part 
-- of this text at all times.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor             : Xilinx
-- \   \   \/    Version	    : 2.1
--  \   \        Application	    : MIG
--  /   /        Filename           : mig_21_dqs_delay_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : This module generate the delay in the dqs signal.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;

entity mig_21_dqs_delay is
  port (
    clk_in  : in  std_logic;
    sel_in  : in  std_logic_vector(4 downto 0);
    clk_out : out std_logic
    );
end mig_21_dqs_delay;

architecture arc_dqs_delay of mig_21_dqs_delay is

  signal delay1 : std_logic;
  signal delay2 : std_logic;
  signal delay3 : std_logic;
  signal delay4 : std_logic;
  signal delay5 : std_logic;
  signal high   : std_logic;

  attribute syn_noprune  : boolean;
  
  attribute syn_noprune of one   : label is true;
  attribute syn_noprune of two   : label is true;
  attribute syn_noprune of three : label is true;
  attribute syn_noprune of four  : label is true;
  attribute syn_noprune of five  : label is true;
  attribute syn_noprune of six   : label is true;

begin

  high <= '1';

  one : LUT4 generic map (INIT => x"f3c0")
    port map (
      I0  => high,
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
