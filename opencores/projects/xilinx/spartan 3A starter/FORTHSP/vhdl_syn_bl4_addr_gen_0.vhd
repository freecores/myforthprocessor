-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_addr_gen_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:59 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
--
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : It generates address and burst done signals to the test bench.
-------------------------------------------------------------------------------
library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_addr_gen_0 is
  port(
    clk          : in  std_logic;
    rst          : in  std_logic;
    addr_rst     : in  std_logic;
    addr_inc     : in  std_logic;
    r_w          : in  std_logic;
    addr_out     : out std_logic_vector(((row_address + col_ap_width 
                                          + bank_address)- 1) downto 0);
    test_cnt_ena : in  std_logic;
    burst_done   : out std_logic;
    cnt_roll     : out std_logic);
end vhdl_syn_bl4_addr_gen_0;

architecture arc of vhdl_syn_bl4_addr_gen_0 is
  signal column_counter   : std_logic_vector(7 downto 0);
  signal ba_count         : std_logic_vector((bank_address - 1) downto 0);
  signal cnt              : std_logic_vector(1 downto 0);
  signal burst_done_reg   : std_logic;
  signal burst_done_1_reg : std_logic;
  signal burst_done_2_reg : std_logic;
  signal row_address1	    : std_logic_vector(row_address-1 downto 0);

  signal cnt_roll_p       : std_logic;
  signal cnt_roll_p2      : std_logic;
  signal low              : std_logic_vector(13 downto 0);
  signal rst_r            : std_logic;

  attribute syn_keep : boolean;
  attribute syn_keep of low : signal is true;

begin
  low      <= "00000000000000";
  addr_out <= (row_address1 & (low(col_ap_width-9 downto 0)) 
               & column_counter & ba_count);

   burst_done <= burst_done_1_reg;


  process ( clk )  
  begin
    if falling_edge(clk) then
      rst_r <= rst;
    end if;
  end process;

  process ( clk )  
  begin
    if falling_edge(clk) then
      if(rst_r = '1' or row_address1(5) = '1') then
        row_address1 <=  low(row_address-3 downto 0) & "10";
       elsif( r_w = '1' and burst_done_1_reg = '0' and burst_done_2_reg = '1') then
        row_address1 <=  row_address1 + "10";
      else
        row_address1 <=  row_address1;
      end if;
    end if;
  end process;
  
  process ( clk )  
  begin
    if falling_edge(clk) then
      if(rst_r = '1') then
        ba_count <=  (others => '0');
       elsif( r_w = '1' and burst_done_1_reg = '0' and burst_done_2_reg = '1') then
        ba_count <=  ba_count + '1';
      else
        ba_count <=  ba_count;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if (rst_r = '1' or addr_rst = '1') then
        column_counter <= "00000000";
        cnt            <= "00";

      elsif addr_inc = '1' then
        if cnt = "01" then
          cnt <= "00";
        else
          cnt <= cnt + "01";
        end if;
          if test_cnt_ena = '1' and cnt = "01" then
            if column_counter = "00010000" then  --11110000
              column_counter <= "00000000";
            else
              column_counter <= column_counter + "00000100";
            end if;
      else
        column_counter <= column_counter;
      end if;

      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then

    burst_done_reg <= ( not(rst) and column_counter(4));  --BL=4

    end if;
  end process;

  process(clk)
  begin
    if(clk'event and clk = '0') then
      burst_done_1_reg <= (not(rst_r) and burst_done_reg);
    end if;
  end process;
  process(clk)
  begin
    if(clk'event and clk = '0') then
      burst_done_2_reg <= burst_done_1_reg;
    end if;
  end process;


  process(clk)
  begin
    if(clk'event and clk = '0') then

    cnt_roll_p <= not(rst) and column_counter(3) and column_counter(2) 
						and not(column_counter(1)) and not(column_counter(0)) ; 

    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst_r = '1' then
        cnt_roll_p2 <= '0';
        cnt_roll    <= '0';
      else
        cnt_roll_p2 <= cnt_roll_p;
        cnt_roll    <= cnt_roll_p2;
      end if;
    end if;
  end process;

end arc;
