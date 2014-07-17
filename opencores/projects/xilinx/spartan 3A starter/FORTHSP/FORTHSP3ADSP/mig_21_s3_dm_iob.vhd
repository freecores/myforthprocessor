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
--  /   /        Filename           : mig_21_s3_dm_iob.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : This module instantiates DDR IOB output flip-flops, and an
--               output buffer for the data mask bits.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;

entity mig_21_s3_dm_iob is
  port (
    ddr_dm       : out std_logic;   --Data mask output
    mask_falling : in std_logic;    --Mask output on falling edge
    mask_rising  : in std_logic;    --Mask output on rising edge
    clk90        : in std_logic
    );
end mig_21_s3_dm_iob;

architecture arc of mig_21_s3_dm_iob is

--***********************************************************************\
-- Internal signal declaration
--***********************************************************************/

  signal mask_o : std_logic;
  signal gnd    : std_logic;
  signal vcc    : std_logic;
  signal clk270 : std_logic;
begin

  gnd    <= '0';
  vcc    <= '1';
  clk270 <= not clk90;

-- Data Mask Output during a write command

  DDR_DM0_OUT : FDDRRSE
    port map (
      Q  => mask_o,
      C0 => clk270,
      C1 => clk90,
      CE => vcc,
      D0 => mask_rising,
      D1 => mask_falling,
      R  => gnd,
      S  => gnd
      );

  DM1_OBUF : OBUF
    port map (
      I => mask_o,
      O => ddr_dm
      );
  
end arc;
