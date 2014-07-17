-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_fifo_0_wr_en_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module generate the write enable signal to the fifos, 
--		 which are driven by posedge of data strobe
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_fifo_1_wr_en_0 is
  port (
    clk             : in  std_logic;
    rst_dqs_delay_n : in  std_logic;
    reset           : in  std_logic;
    din             : in  std_logic;
    dout            : out std_logic
    );
end vhdl_syn_bl4_fifo_1_wr_en_0;

architecture arc of vhdl_syn_bl4_fifo_1_wr_en_0 is
  attribute KEEP_HIERARCHY : string;
  attribute KEEP_HIERARCHY of arc : architecture is "YES";

  signal din_delay     : std_ulogic;
  signal TIE_HIGH      : std_ulogic;
  signal dout0         : std_ulogic;
  signal rst_dqs_delay : std_logic;

begin

  rst_dqs_delay <= not rst_dqs_delay_n;
  dout0         <= din and rst_dqs_delay_n;
  dout          <= rst_dqs_delay or din_delay;
  TIE_HIGH      <= '1';


  delay_ff_1 : FDCE port map (
    Q   => din_delay,
    C   => clk,
    CE  => TIE_HIGH,
    CLR => reset,
    D   => dout0
    );

end arc;
