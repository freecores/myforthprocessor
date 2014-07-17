-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_rd_gray_cntr.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:36 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
--
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     :
-------------------------------------------------------------------------------

-- fifo_rd_addr gray counter with synchronous reset
library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_rd_gray_cntr is
  port (
    clk      : in  std_logic;
    reset    : in  std_logic;
    cnt_en   : in  std_logic;
    rgc_gcnt : out std_logic_vector(3 downto 0)
    );
end vhdl_syn_bl4_rd_gray_cntr;

architecture arc of vhdl_syn_bl4_rd_gray_cntr is

  signal gc_int  : std_logic_vector(3 downto 0);
  signal d_in    : std_logic_vector(3 downto 0);
  signal reset_r : std_logic;

begin

  rgc_gcnt <= gc_int(3 downto 0);
  process(clk)
  begin
    if(clk'event and clk = '1') then
      reset_r <= reset;
    end if;
  end process;

  process(gc_int)
  begin
    case gc_int is
      when "0000" => d_in <= "0001";    --0 > 1
      when "0001" => d_in <= "0011";    --1 > 3
      when "0010" => d_in <= "0110";    --2 > 6
      when "0011" => d_in <= "0010";    --3 > 2
      when "0100" => d_in <= "1100";    --4 > c
      when "0101" => d_in <= "0100";    --5 > 4
      when "0110" => d_in <= "0111";    --6 > 7
      when "0111" => d_in <= "0101";    --7 > 5
      when "1000" => d_in <= "0000";    --8 > 0
      when "1001" => d_in <= "1000";    --9 > 8
      when "1010" => d_in <= "1011";    --10 > b
      when "1011" => d_in <= "1001";    --11 > 9
      when "1100" => d_in <= "1101";    --12 > d
      when "1101" => d_in <= "1111";    --13 > f
      when "1110" => d_in <= "1010";    --14 > a
      when "1111" => d_in <= "1110";    --15 > e
      when others => d_in <= "0001";    --0 > 1
    end case;
  end process;

  bit0 : FDRE 
    port map (
      Q  => gc_int(0),
      C  => clk,
      CE => cnt_en,
      D  => d_in(0),
      R  => reset_r
      );

  bit1 : FDRE 
    port map (
      Q  => gc_int(1),
      C  => clk,
      CE => cnt_en,
      D  => d_in(1),
      R  => reset_r
      );

  bit2 : FDRE port map (
    Q  => gc_int(2),
    C  => clk,
    CE => cnt_en,
    D  => d_in(2),
    R  => reset_r
    );

  bit3 : FDRE port map (
    Q  => gc_int(3),
    C  => clk,
    CE => cnt_en,
    D  => d_in(3),
    R  => reset_r
    );

end arc;

