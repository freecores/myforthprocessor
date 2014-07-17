-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_infrastructure.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     :
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;


entity vhdl_syn_bl4_infrastructure is
  port(
    delay_sel_val1_val : out std_logic_vector(4 downto 0);
    delay_sel_val      : in  std_logic_vector(4 downto 0);
    rst_calib1         : in  std_logic;
    clk_int            : in  std_logic
    );
end vhdl_syn_bl4_infrastructure;

architecture arc of vhdl_syn_bl4_infrastructure is
  signal delay_sel_val1   : std_logic_vector(4 downto 0);
  signal rst_calib1_r1    : std_logic;
  signal rst_calib1_r2    : std_logic;
begin

  delay_sel_val1_val <= delay_sel_val1;

  process(clk_int)
  begin
    if clk_int 'event and clk_int = '0' then
      rst_calib1_r1    <= rst_calib1;       
    end if;
  end process;

 process(clk_int)
 begin
   if clk_int 'event and clk_int = '1' then
     rst_calib1_r2    <= rst_calib1_r1;
   end if;
  end process;

  process(clk_int)
  begin
    if clk_int 'event and clk_int = '1' then
      if (rst_calib1_r2 = '0') then
        delay_sel_val1 <= delay_sel_val;
      else
        delay_sel_val1 <= delay_sel_val1;
      end if;
    end if;
  end process;


end arc;


