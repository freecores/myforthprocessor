-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_RAM8D_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:36 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module instantiates RAM16X1 premitives. There will be 8 or 4 RAM16X1 
--		 instances depending on the number of data bits per strobe.
-------------------------------------------------------------------------------

library IEEE;
library UNISIM;
use IEEE.STD_LOGIC_1164.all;
use IEEE.STD_LOGIC_ARITH.all;
use IEEE.STD_LOGIC_UNSIGNED.all;
use UNISIM.VCOMPONENTS.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_RAM8D_0 is
  port (
    DOUT  : out std_logic_vector((DatabitsPerStrobe -1) downto 0);
    WADDR : in  std_logic_vector(3 downto 0);
    DIN   : in  std_logic_vector((DatabitsPerStrobe -1) downto 0);
    RADDR : in  std_logic_vector(3 downto 0);
    WCLK0 : in  std_logic;
    WCLK1 : in  std_logic;
    WE    : in  std_logic
    );
end vhdl_syn_bl4_RAM8D_0;
architecture arc of vhdl_syn_bl4_RAM8D_0 is

begin

  fifo_bit0 : RAM16X1D
    port map (
	  DPO    => DOUT(0),
      A0     => WADDR(0),
      A1     => WADDR(1),
      A2     => WADDR(2),
      A3     => WADDR(3),
      D      => DIN(0),
      DPRA0  => RADDR(0),
      DPRA1  => RADDR(1),
      DPRA2  => RADDR(2),
      DPRA3  => RADDR(3),
      WCLK   => WCLK0,
      SPO    => OPEN,
      WE     => WE 
	  );

  fifo_bit1 : RAM16X1D
    port map (
	  DPO    => DOUT(1),
      A0     => WADDR(0),
      A1     => WADDR(1),
      A2     => WADDR(2),
      A3     => WADDR(3),
      D      => DIN(1),
      DPRA0  => RADDR(0),
      DPRA1  => RADDR(1),
      DPRA2  => RADDR(2),
      DPRA3  => RADDR(3),
      WCLK   => WCLK1,
      SPO    => OPEN,
      WE     => WE 
	  );

fifo_bit2 : RAM16X1D
  port map (
	DPO    => DOUT(2),
    A0     => WADDR(0),
    A1     => WADDR(1),
    A2     => WADDR(2),
    A3     => WADDR(3),
    D      => DIN(2),
    DPRA0  => RADDR(0),
    DPRA1  => RADDR(1),
    DPRA2  => RADDR(2),
    DPRA3  => RADDR(3),
    SPO    => OPEN,
    WCLK   => WCLK1,
    WE     => WE 
	);


fifo_bit3 : RAM16X1D
  port map ( 
    DPO    => DOUT(3),
    A0     => WADDR(0),
    A1     => WADDR(1),
    A2     => WADDR(2),
    A3     => WADDR(3),
    D      => DIN(3),
    DPRA0  => RADDR(0),
    DPRA1  => RADDR(1),
    DPRA2  => RADDR(2),
    DPRA3  => RADDR(3),
    SPO    => OPEN,
    WCLK   => WCLK0,
    WE     => WE 
	);

fifo_bit4: RAM16X1D
  port map (
    DPO    => DOUT(4),
    A0     => WADDR(0),
    A1     => WADDR(1),
    A2     => WADDR(2),
    A3     => WADDR(3),
    D      => DIN(4),
    DPRA0  => RADDR(0),
    DPRA1  => RADDR(1),
    DPRA2  => RADDR(2),
    DPRA3  => RADDR(3),
    SPO    => OPEN,
    WCLK   => WCLK1,
    WE     => WE 
	);


fifo_bit5 : RAM16X1D
  port map (
    DPO    => DOUT(5),
    A0     => WADDR(0),
    A1     => WADDR(1),
    A2     => WADDR(2),
    A3     => WADDR(3),
    D      => DIN(5),
    DPRA0  => RADDR(0),
    DPRA1  => RADDR(1),
    DPRA2  => RADDR(2),
    DPRA3  => RADDR(3),
    SPO    => OPEN,
    WCLK   => WCLK0,
    WE     => WE 
	);

fifo_bit6 : RAM16X1D
  port map(
    DPO    => DOUT(6),
    A0     => WADDR(0),
    A1     => WADDR(1),
    A2     => WADDR(2),
    A3     => WADDR(3),
    D      => DIN(6),
    DPRA0  => RADDR(0),
    DPRA1  => RADDR(1),
    DPRA2  => RADDR(2),
    DPRA3  => RADDR(3),
    SPO    => OPEN,
    WCLK   => WCLK0,
    WE     => WE 
	);


fifo_bit7 : RAM16X1D
  port map (
    DPO    => DOUT(7),
    A0     => WADDR(0),
    A1     => WADDR(1),
    A2     => WADDR(2),
    A3     => WADDR(3),
    D      => DIN(7),
    DPRA0  => RADDR(0),
    DPRA1  => RADDR(1),
    DPRA2  => RADDR(2),
    DPRA3  => RADDR(3),
    SPO    => OPEN,
    WCLK   => WCLK1,
    WE     => WE 
	);

end arc;
