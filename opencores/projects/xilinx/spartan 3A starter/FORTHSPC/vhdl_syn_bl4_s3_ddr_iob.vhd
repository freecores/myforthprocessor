-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_s3_ddr_iob.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module instantiate DDR IOB output flip-flops, an 
--               output buffer with registered tri-state, and an input buffer  
--               for a single data/dq bit. The DDR IOB output flip-flops 
--               are used to forward data to memory during a write. 
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_s3_ddr_iob is
  port (
    ddr_dq_inout       : inout std_logic;--Bi-directional SDRAM data bus
    write_data_falling : in    std_logic;--Transmit data, output on falling edge
    write_data_rising  : in    std_logic;--Transmit data, output on rising edge
    read_data_in       : out   std_logic;-- Received data
    clk90              : in    std_logic;--Clock 90
    write_en_val       : in    std_logic
    );
end vhdl_syn_bl4_s3_ddr_iob;

architecture arc of vhdl_syn_bl4_s3_ddr_iob is

--***********************************************************************\
-- Internal signal declaration
--***********************************************************************/
  signal ddr_en   : std_logic;          -- Tri-state enable signal
  signal ddr_dq_q : std_logic;          -- Data output intermediate signal
  signal GND      : std_logic;
  signal clock_en : std_logic;
  signal enable_b : std_logic;
  signal clk270   : std_logic;

begin
  clk270 <= not clk90;
  GND      <= '0';
  enable_b <= not write_en_val;
  clock_en <= '1';                   

-- Transmission data path

  DDR_OUT : FDDRRSE 
    port map (  
      Q  => ddr_dq_q,
      C0 => clk270,
      C1 => clk90,
      CE => clock_en,
      D0 => write_data_rising,
      D1 => write_data_falling,
      R  => GND,
      S  => GND
      );

  DQ_T : FD 
    port map (  
      D  => enable_b,
      C => clk270,
      Q => ddr_en
      );

  DQ_OBUFT : OBUFT 
    port map ( 
      I  => ddr_dq_q,
      T => ddr_en,
      O => ddr_dq_inout
      );

-- Receive data path

  DQ_IBUF : IBUF 
    port map(
      I  => ddr_dq_inout,
      O => read_data_in
      );

end arc;

