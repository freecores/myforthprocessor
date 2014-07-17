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
--  /   /        Filename           : mig_21_ram8d_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : This module instantiates RAM16X1 premitives. There will be 8 or 4 RAM16X1
--               instances depending on the number of data bits per strobe.
-------------------------------------------------------------------------------

library IEEE;
library UNISIM;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;
use UNISIM.VCOMPONENTS.all;
use work.mig_21_parameters_0.all;

entity mig_21_ram8d_0 is
  port (
    dout  : out std_logic_vector((DATABITSPERSTROBE -1) downto 0);
    waddr : in  std_logic_vector(3 downto 0);
    din   : in  std_logic_vector((DATABITSPERSTROBE -1) downto 0);
    raddr : in  std_logic_vector(3 downto 0);
    wclk0 : in  std_logic;
    wclk1 : in  std_logic;
    we    : in  std_logic
    );
end mig_21_ram8d_0;

architecture arc of mig_21_ram8d_0 is

begin

  fifo_bit0 : RAM16X1D
    port map (
	  DPO    => dout(0),
      A0     => waddr(0),
      A1     => waddr(1),
      A2     => waddr(2),
      A3     => waddr(3),
      D      => din(0),
      DPRA0  => raddr(0),
      DPRA1  => raddr(1),
      DPRA2  => raddr(2),
      DPRA3  => raddr(3),
      WCLK   => wclk1,
      SPO    => OPEN,
      WE     => we 
	  );

  fifo_bit1 : RAM16X1D
    port map (
	  DPO    => dout(1),
      A0     => waddr(0),
      A1     => waddr(1),
      A2     => waddr(2),
      A3     => waddr(3),
      D      => din(1),
      DPRA0  => raddr(0),
      DPRA1  => raddr(1),
      DPRA2  => raddr(2),
      DPRA3  => raddr(3),
      WCLK   => wclk0,
      SPO    => OPEN,
      WE     => we 
	  );

  fifo_bit2 : RAM16X1D
    port map (
	  DPO    => dout(2),
      A0     => waddr(0),
      A1     => waddr(1),
      A2     => waddr(2),
      A3     => waddr(3),
      D      => din(2),
      DPRA0  => raddr(0),
      DPRA1  => raddr(1),
      DPRA2  => raddr(2),
      DPRA3  => raddr(3),
      WCLK   => wclk1,
      SPO    => OPEN,
      WE     => we 
	  );

  fifo_bit3 : RAM16X1D
    port map (
	  DPO    => dout(3),
      A0     => waddr(0),
      A1     => waddr(1),
      A2     => waddr(2),
      A3     => waddr(3),
      D      => din(3),
      DPRA0  => raddr(0),
      DPRA1  => raddr(1),
      DPRA2  => raddr(2),
      DPRA3  => raddr(3),
      WCLK   => wclk0,
      SPO    => OPEN,
      WE     => we 
	  );

fifo_bit4 : RAM16X1D
    port map (
	  DPO    => dout(4),
      A0     => waddr(0),
      A1     => waddr(1),
      A2     => waddr(2),
      A3     => waddr(3),
      D      => din(4),
      DPRA0  => raddr(0),
      DPRA1  => raddr(1),
      DPRA2  => raddr(2),
      DPRA3  => raddr(3),
      WCLK   => wclk1,
      SPO    => OPEN,
      WE     => we 
	  );

  fifo_bit5 : RAM16X1D
    port map (
	  DPO    => dout(5),
      A0     => waddr(0),
      A1     => waddr(1),
      A2     => waddr(2),
      A3     => waddr(3),
      D      => din(5),
      DPRA0  => raddr(0),
      DPRA1  => raddr(1),
      DPRA2  => raddr(2),
      DPRA3  => raddr(3),
      WCLK   => wclk0,
      SPO    => OPEN,
      WE     => we 
	  );

  fifo_bit6 : RAM16X1D
    port map (
	  DPO    => dout(6),
      A0     => waddr(0),
      A1     => waddr(1),
      A2     => waddr(2),
      A3     => waddr(3),
      D      => din(6),
      DPRA0  => raddr(0),
      DPRA1  => raddr(1),
      DPRA2  => raddr(2),
      DPRA3  => raddr(3),
      WCLK   => wclk1,
      SPO    => OPEN,
      WE     => we 
	  );

  fifo_bit7 : RAM16X1D
    port map (
	  DPO    => dout(7),
      A0     => waddr(0),
      A1     => waddr(1),
      A2     => waddr(2),
      A3     => waddr(3),
      D      => din(7),
      DPRA0  => raddr(0),
      DPRA1  => raddr(1),
      DPRA2  => raddr(2),
      DPRA3  => raddr(3),
      WCLK   => wclk0,
      SPO    => OPEN,
      WE     => we 
	  );

end arc;
