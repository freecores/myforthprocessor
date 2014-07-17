-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_lfsr32_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2006/12/29 09:37:22 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module generate the user input data for hardware test.
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_lfsr32_0 is
  port (
    clk         : in  std_logic;
    rst         : in  std_logic;
    lfsr_rst    : in  std_logic;
    lfsr_ena    : in  std_logic;
    lfsr_data_m : out std_logic_vector((2*data_mask_width-1) downto 0);
    lfsr_out    : out std_logic_vector((2*data_width-1) downto 0));
end vhdl_syn_bl4_lfsr32_0;

architecture arc of vhdl_syn_bl4_lfsr32_0 is

  signal lfsr_r : std_logic_vector(7 downto 0);
  signal lfsr_f : std_logic_vector(7 downto 0);
  signal rst_r  : std_logic;



begin

  process(clk)
  begin
    if rising_edge(clk) then
      rst_r <= rst;
    end if;
  end process;

  process(clk)
  begin
    if rising_edge(clk) then
      if (rst_r = '1') then
        lfsr_r <= (others => '0');
        lfsr_f <= "00000001";
      else
        if lfsr_rst = '1' then
          lfsr_r <= (others => '0');
          lfsr_f <= "00000001";
        elsif lfsr_ena = '1' then
          lfsr_r <= lfsr_r + "00000010";
          lfsr_f <= lfsr_f + "00000010";
        else
          lfsr_r <= lfsr_r;
          lfsr_f <= lfsr_f;
        end if;
      end if;
    end if;
  end process;

  lfsr_out <=     lfsr_r  &     lfsr_r  &     lfsr_f  &     lfsr_f ;

  lfsr_data_m <= (others => '0');

end arc;
