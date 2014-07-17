-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_cmp_data_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:36 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module compare the read data with written data and 
--               generates the error signal in case of data mismatch.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_cmp_data_0 is
  port(
    clk              : in  std_logic;
    data_valid       : in  std_logic;
    lfsr_data        : in  std_logic_vector((2*data_width-1) downto 0);
    read_data        : in  std_logic_vector((2*data_width-1) downto 0);
    rst              : in  std_logic;
    led_error_output : out std_logic;
    data_valid_out   : out std_logic
    );
end vhdl_syn_bl4_cmp_data_0;

architecture arc of vhdl_syn_bl4_cmp_data_0 is

  signal led_state     : std_logic;
  signal error         : std_logic;
  signal lfsr_0  :  std_logic_vector(15 downto 0);
  signal lfsr_1  :  std_logic_vector(15 downto 0);
  signal data_0  :  std_logic_vector(15 downto 0);
  signal data_1  :  std_logic_vector(15 downto 0);
  signal byte_err      : std_logic_vector((data_width/8)-1 downto 0);
  signal byte_err1     : std_logic_vector((data_width/8)-1 downto 0);
  signal valid         : std_logic;
  signal val_reg       : std_logic;
  signal read_data_reg : std_logic_vector((2*data_width-1) downto 0);
  signal rst_r         : std_logic;

begin

  data_valid_out <= valid;
  process(clk)
  begin
    if clk'event and clk = '1' then
      rst_r <= rst;
    end if;
  end process;


  process(clk)
  begin
    if clk'event and clk = '1' then
      read_data_reg  <= read_data;
    end if;
  end process;

  process (clk)
  begin
    if clk'event and clk = '1' then
      if rst_r = '1' then
        valid <= '0';
      else
        valid <= data_valid;
      end if;
    end if;
  end process;

  data_0  <= read_data_reg(15 downto 0);
  data_1  <= read_data_reg(31 downto 16);
  lfsr_0  <= lfsr_data(15 downto 0);
  lfsr_1  <= lfsr_data(31 downto 16);
  
  process(clk)
  begin
    if clk'event and clk = '1' then
      if (rst_r = '1') then
        val_reg <= '0';
      else
        val_reg <= valid;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '1' then
       if (data_0(7 downto 0) /= lfsr_0(7 downto 0)) then
               byte_err(0) <= '1';
       else
               byte_err(0) <= '0';
       end if;

       if (data_0(15 downto 8) /= lfsr_0(15 downto 8)) then
               byte_err1(0) <= '1';
       else
               byte_err1(0) <= '0';
       end if;
       if (data_1(7 downto 0) /= lfsr_1(7 downto 0)) then
               byte_err(1) <= '1';
       else
               byte_err(1) <= '0';
       end if;

       if (data_1(15 downto 8) /= lfsr_1(15 downto 8)) then
               byte_err1(1) <= '1';
       else
               byte_err1(1) <= '0';
       end if;
    end if;
  end process;

  error <= ((
            byte_err(0) or 
            byte_err(1)) or (
            byte_err1(0) or 
            byte_err1(1))) and val_reg;

-- LED error output
  process(clk)
  begin
    if clk'event and clk = '1' then
      led_state <= (not rst_r and (error or led_state));
    end if;
    assert (led_state = '0') report " DATA ERROR" severity warning;
  end process;

  led_error_output <= '1' when led_state = '1' else
                      '0';

end arc;
