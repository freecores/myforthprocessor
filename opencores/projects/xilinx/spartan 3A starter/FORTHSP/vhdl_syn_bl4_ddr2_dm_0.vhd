-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_ddr2_dm_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/06 08:09:02 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module instantiates DDR IOB output flip-flops, and an
--                    output buffer for the data mask bits.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity   vhdl_syn_bl4_ddr2_dm_0  is
  port (
    ddr_dm       : out std_logic_vector((data_mask_width-1) downto 0);   
												--Data mask output
    mask_falling : in std_logic_vector((data_mask_width-1) downto 0);    
												--Mask output on falling edge
    mask_rising  : in std_logic_vector((data_mask_width-1) downto 0);    
												--Mask output on rising edge
    clk90        : in std_logic
      );
end   vhdl_syn_bl4_ddr2_dm_0 ;


architecture arc of vhdl_syn_bl4_ddr2_dm_0 is

--***********************************************************************\
-- Internal signal declaration
--***********************************************************************/

  signal mask_o : std_logic_vector((data_mask_width-1) downto 0);  
  signal gnd    : std_logic;
  signal vcc    : std_logic;
  signal clk270 : std_logic;
begin

  gnd <= '0';
  vcc <= '1';
    clk270 <= not clk90;

-- Data Mask Output during a write command

    DDR_DM0_OUT : FDDRRSE 
    port map (
       Q  => mask_o(0),
       C0 => clk270,
       C1 => clk90,
       CE => vcc,
       D0 => mask_rising(0),
       D1 => mask_falling(0),
       R  => gnd,
       S  => gnd
      );
  DDR_DM1_OUT : FDDRRSE 
    port map (
       Q  => mask_o(1),
       C0 => clk270,
       C1 => clk90,
       CE => vcc,
       D0 => mask_rising(1),
       D1 => mask_falling(1),
       R  => gnd,
       S  => gnd
      );

    DM0_OBUF : OBUF 
    port map (
      I => mask_o(0),
      O => ddr_dm(0)
      );
  DM1_OBUF : OBUF 
    port map (
      I => mask_o(1),
      O => ddr_dm(1)
      );

end arc;

