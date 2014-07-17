-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_fifo_0_wr_en_0.v
-- /___/   /\    Date Last Modified : $Date: 2006/12/27 06:38:55 $	
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module generate the write enable signal to the fifos, 
--		 which are driven by negedge of data strobe
-------------------------------------------------------------------------------


library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_fifo_0_wr_en_0 is
  port (
    clk             : in  std_logic;
    reset           : in  std_logic;
    din             : in  std_logic;
    rst_dqs_delay_n : out std_logic;
    dout            : out std_logic
    );

end vhdl_syn_bl4_fifo_0_wr_en_0;

architecture arc of vhdl_syn_bl4_fifo_0_wr_en_0 is
  attribute KEEP_HIERARCHY : string;
  attribute KEEP_HIERARCHY of arc : architecture is "YES";

  signal din_delay : std_ulogic;
  signal TIE_HIGH  : std_ulogic;


begin

  rst_dqs_delay_n <= not din_delay;
  dout            <= din or din_delay;
  TIE_HIGH        <= '1';


  delay_ff : FDCE port map (
    Q   => din_delay,
    C   => clk,
    CE  => TIE_HIGH,
    CLR => reset,
    D   => din
    );

end arc;

