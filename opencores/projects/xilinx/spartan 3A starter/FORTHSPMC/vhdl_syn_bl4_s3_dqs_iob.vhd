 -------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_s3_dqs_iob.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:36 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module instantiates DDR IOB output flip-flops, an 
--               output buffer with registered tri-state, and an input buffer  
--               for a single strobe/dqs bit. The DDR IOB output flip-flops 
--               are used to forward strobe to memory during a write. During
--               a read, the output of the IBUF is routed to the internal 
--               delay module, dqs_delay. 
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
library UNISIM;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_s3_dqs_iob is
  port(
    clk            : in    std_logic;
    ddr_dqs_reset  : in    std_logic;
    ddr_dqs_enable : in    std_logic;
    ddr_dqs        : inout std_logic;
    ddr_dqs_n          : inout std_logic;
    dqs            : out   std_logic);
end vhdl_syn_bl4_s3_dqs_iob;

architecture arc of vhdl_syn_bl4_s3_dqs_iob is
  

  component OBUFTDS
    port(
      I : in  std_logic;
      T : in  std_logic;
      O : out std_logic;
      OB : out std_logic
      );
  end component;

  component IBUFDS
    port(
      I : in  std_logic;
      IB : in  std_logic;
      O : out std_logic
      );
  end component;


  

  signal dqs_q            : std_logic;
  signal ddr_dqs_enable1  : std_logic;
  signal vcc              : std_logic;
  signal gnd              : std_logic;
  signal ddr_dqs_enable_b : std_logic;
  signal data1            : std_logic;
  signal clk180           : std_logic;

begin

--******************************************************************************
-- Output DDR generation. This includes instantiation of the output DDR flip flop.
-- Additionally, to keep synthesis tools from register sharing, manually
-- instantiate the output tri-state flip-flop.
--****************************************************************************** 
  vcc              <= '1';
  gnd              <= '0';
  clk180           <= not clk;
  ddr_dqs_enable_b <= not ddr_dqs_enable;
  data1            <= '0' when ddr_dqs_reset = '1' else
                      '1';

  U1 : FD
    port map (
      D => ddr_dqs_enable_b,
      Q => ddr_dqs_enable1,
      C => clk
      );

    U2 : FDDRRSE
    port map (
      Q    => dqs_q,
      C0 => clk,
      C1 => clk180,
      CE => vcc,
      D0 => data1,
      D1 => gnd,
      R  => gnd,
      S  => gnd
      );
  

--***********************************************************************
--    IO buffer for dqs signal. Allows for distribution of dqs
--     to the data (DQ) loads.
--***********************************************************************


    U3 : OBUFTDS port map (
            I  => dqs_q,
            T  => ddr_dqs_enable1,
            O  => ddr_dqs,
            OB => ddr_dqs_n
            );

     U4 : IBUFDS  port map(
                   I  => ddr_dqs,
                   IB => ddr_dqs_n,
                   O  => dqs
                   );




end arc;
