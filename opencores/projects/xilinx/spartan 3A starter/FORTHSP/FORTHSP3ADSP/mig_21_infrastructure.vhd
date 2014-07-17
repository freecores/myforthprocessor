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
--  /   /        Filename           : mig_21_infrastructure.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     :
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;


entity mig_21_infrastructure is
  port(
    delay_sel_val1_val : out std_logic_vector(4 downto 0);
    delay_sel_val      : in  std_logic_vector(4 downto 0);
    rst_calib1         : in  std_logic;
    clk_int            : in  std_logic;
    -- debug signals
    dbg_delay_sel      : out std_logic_vector(4 downto 0);
    dbg_rst_calib      : out std_logic
    );
end mig_21_infrastructure;

architecture arc of mig_21_infrastructure is
  
  signal delay_sel_val1 : std_logic_vector(4 downto 0);
  signal rst_calib1_r1  : std_logic;
  signal rst_calib1_r2  : std_logic;
  
begin

  delay_sel_val1_val <= delay_sel_val1;
  dbg_delay_sel      <= delay_sel_val1;
  dbg_rst_calib      <= rst_calib1_r2;

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
