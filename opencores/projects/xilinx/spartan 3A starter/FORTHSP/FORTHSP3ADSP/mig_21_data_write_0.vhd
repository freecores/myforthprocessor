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
--  /   /        Filename           : mig_21_data_write0.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : Data write operation performed through the pipelines in this
--               module.
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
library UNISIM;
use UNISIM.VCOMPONENTS.all;
use work.mig_21_parameters_0.all;

entity mig_21_data_write_0 is
  port(
    user_input_data    : in  std_logic_vector((2*DATA_WIDTH-1) downto 0);
    user_data_mask     : in  std_logic_vector((2*DATA_MASK_WIDTH-1) downto 0);
    clk90              : in  std_logic;
    write_enable       : in  std_logic;
    write_en_val       : out std_logic;
    data_mask_f        : out std_logic_vector((DATA_MASK_WIDTH-1) downto 0);
    data_mask_r        : out std_logic_vector((DATA_MASK_WIDTH-1) downto 0);
    write_data_falling : out std_logic_vector((DATA_WIDTH-1) downto 0);
    write_data_rising  : out std_logic_vector((DATA_WIDTH-1) downto 0)
    );
end mig_21_data_write_0;

architecture arc of mig_21_data_write_0 is


  signal write_en_P1       : std_logic;  -- write enable Pipeline stage
  signal write_en_P2       : std_logic;
  signal write_en_int      : std_logic;
  signal write_data0       : std_logic_vector((2*DATA_WIDTH-1) downto 0);
  signal write_data1       : std_logic_vector((2*DATA_WIDTH-1) downto 0);
  signal write_data2       : std_logic_vector((2*DATA_WIDTH-1) downto 0);
  signal write_data3       : std_logic_vector((2*DATA_WIDTH-1) downto 0);
  signal write_data4       : std_logic_vector((2*DATA_WIDTH-1) downto 0);
  signal write_data_m0     : std_logic_vector ((2*DATA_MASK_WIDTH-1) downto 0);
  signal write_data_m1     : std_logic_vector ((2*DATA_MASK_WIDTH-1) downto 0);
  signal write_data_m2     : std_logic_vector ((2*DATA_MASK_WIDTH-1) downto 0);
  signal write_data_m3     : std_logic_vector ((2*DATA_MASK_WIDTH-1) downto 0);
  signal write_data_m4     : std_logic_vector ((2*DATA_MASK_WIDTH-1) downto 0);

  signal write_data90       : std_logic_vector((DATA_WIDTH-1) downto 0);
  signal write_data90_1     : std_logic_vector((DATA_WIDTH-1) downto 0);
  signal write_data90_2     : std_logic_vector((DATA_WIDTH-1) downto 0);
  signal write_data_m90     : std_logic_vector ((DATA_MASK_WIDTH-1) downto 0);
  signal write_data_m90_1   : std_logic_vector ((DATA_MASK_WIDTH-1) downto 0);
  signal write_data_m90_2   : std_logic_vector ((DATA_MASK_WIDTH-1) downto 0);

  signal write_data270     : std_logic_vector((DATA_WIDTH-1) downto 0);
  signal write_data270_1   : std_logic_vector((DATA_WIDTH-1) downto 0);
  signal write_data270_2   : std_logic_vector((DATA_WIDTH-1) downto 0);

  signal write_data_m270   : std_logic_vector ((DATA_MASK_WIDTH-1) downto 0);
  signal write_data_m270_1 : std_logic_vector ((DATA_MASK_WIDTH-1) downto 0);
  signal write_data_m270_2 : std_logic_vector ((DATA_MASK_WIDTH-1) downto 0);

  

  attribute syn_preserve : boolean;
  attribute syn_preserve of write_data0 : signal is true;
  attribute syn_preserve of write_data1 : signal is true;
  attribute syn_preserve of write_data2 : signal is true;
  attribute syn_preserve of write_data3 : signal is true;
  attribute syn_preserve of write_data4 : signal is true;

  attribute syn_preserve of write_data_m0 : signal is true;
  attribute syn_preserve of write_data_m1 : signal is true;
  attribute syn_preserve of write_data_m2 : signal is true;
  attribute syn_preserve of write_data_m3 : signal is true;
  attribute syn_preserve of write_data_m4 : signal is true;

  attribute syn_preserve of write_data90   : signal is true;
  attribute syn_preserve of write_data90_1 : signal is true;
  attribute syn_preserve of write_data90_2 : signal  is true;

  attribute syn_preserve of write_data270   : signal is true;
  attribute syn_preserve of write_data270_1 : signal is true;
  attribute syn_preserve of write_data270_2 : signal is true;

begin

  write_data0   <= user_input_data;
  write_data_m0 <= user_data_mask;

  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      write_data1   <= write_data0;
      write_data_m1 <= write_data_m0;
      write_data2   <= write_data1;
      write_data_m2 <= write_data_m1;
      write_data3   <= write_data2;
      write_data_m3 <= write_data_m2;
      write_data4   <= write_data3;
      write_data_m4 <= write_data_m3;
    end if;
  end process;

  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      write_data90        <= write_data4((DATA_WIDTH-1) downto 0); 
      write_data_m90      <= write_data_m4((DATA_MASK_WIDTH-1) downto 0);
      write_data90_1     <= write_data90;
      write_data_m90_1   <= write_data_m90;
      write_data90_2      <= write_data90_1;
      write_data_m90_2 <= write_data_m90_1;

    end if;
  end process;


  process(clk90)
  begin
    if clk90'event and clk90 = '0' then
      write_data270     <= write_data4((DATA_WIDTH*2-1) downto DATA_WIDTH);
      write_data_m270   <= write_data_m4((DATA_MASK_WIDTH*2-1) downto DATA_MASK_WIDTH);
      write_data270_1 <= write_data270;
      write_data270_2   <= write_data270_1;
      write_data_m270_1 <= write_data_m270;
      write_data_m270_2 <= write_data_m270_1;

    end if;
  end process;

  write_data_rising  <= write_data270_2;
  write_data_falling <= write_data90_2;
  data_mask_r        <= write_data_m270_2;
  data_mask_f        <= write_data_m90_2; 

-- write enable for data path
  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      write_en_P1 <= write_enable;
      write_en_P2 <= write_en_P1;
    end if;
  end process;

-- write enable for data path
  process(clk90)
  begin
    if clk90'event and clk90 = '0' then
      write_en_int <= write_en_P2;
      write_en_val <= write_en_P1;
    end if;
  end process;

end arc;
