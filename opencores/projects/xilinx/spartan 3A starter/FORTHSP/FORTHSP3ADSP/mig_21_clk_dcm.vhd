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
--  /   /        Filename           : mig_21_clk_dcm.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
--
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     :     This module generates the system clock for controller block
--                    This also generates the recapture clock, clock for the
--                    Refresh counter and also for the data path
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use UNISIM.VCOMPONENTS.all;
entity mig_21_clk_dcm is
  port(
    input_clk : in  std_logic;
    rst       : in  std_logic;
    clk       : out std_logic;
    clk90     : out std_logic;
    dcm_lock  : out std_logic
    );
end mig_21_clk_dcm;

architecture arc of mig_21_clk_dcm is

  signal clk0dcm   : std_logic;
  signal clk90dcm  : std_logic;
  signal clk0_buf  : std_logic;
  signal clk90_buf : std_logic;
  signal gnd       : std_logic;
  signal dcm1_lock : std_logic;

begin

  gnd   <= '0';
  clk   <= clk0_buf;
  clk90 <= clk90_buf;

  DCM_INST1 : DCM
    generic map(
      DLL_FREQUENCY_MODE    => "LOW",
      DUTY_CYCLE_CORRECTION => true
      )
    port map (
      CLKIN    => input_clk,
      CLKFB    => clk0_buf,
      DSSEN    => gnd,
      PSINCDEC => gnd,
      PSEN     => gnd,
      PSCLK    => gnd,
      RST      => rst,
      CLK0     => clk0dcm,
      CLK90    => clk90dcm,
      CLK180   => open,
      CLK270   => open,
      CLK2X    => open,
      CLK2X180 => open,
      CLKDV    => open,
      CLKFX    => open,
      CLKFX180 => open,
      LOCKED   => dcm1_lock,
      PSDONE   => open,
      STATUS   => open
      );

  BUFG_CLK0 : BUFGMUX
    port map (
      O  => clk0_buf,
      I0 => clk0dcm,
      I1 => clk0dcm,
      S  => '0'
      );

  BUFG_CLK90 : BUFGMUX
    port map (
      O  => clk90_buf,
      I0 => clk90dcm,
      I1 => clk90dcm,
      S  => '0'
      );

  dcm_lock <= dcm1_lock;

end arc;
