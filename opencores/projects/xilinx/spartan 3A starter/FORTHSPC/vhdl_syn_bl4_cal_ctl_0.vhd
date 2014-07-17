-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_cal_ctl.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module generates the select lines for the LUT delay 
--	             circuit that generate the required delay for the DQS with 
--               respect to the DQ. It calculates the dealy of a LUT dynalically
--               by finding the number of LUTs in a clock phase. 
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_cal_ctl is
  port (
    clk        : in  std_logic;
    reset      : in  std_logic;
    flop2      : in  std_logic_vector(31 downto 0);
    tapForDqs  : out std_logic_vector(4 downto 0)
    );
end vhdl_syn_bl4_cal_ctl;

architecture arc_cal_ctl of vhdl_syn_bl4_cal_ctl is
  attribute KEEP_HIERARCHY : string;
  attribute KEEP_HIERARCHY of arc_cal_ctl : architecture is "YES";

  signal cnt                : std_logic_vector(5 downto 0);
  signal cnt1               : std_logic_vector(5 downto 0);
  signal trans_oneDtct      : std_logic;
  signal trans_twoDtct      : std_logic;
  signal phase_cnt          : std_logic_vector(4 downto 0);
  signal tap_dly_reg        : std_logic_vector(31 downto 0);
  signal enb_trans_two_dtct : std_logic;
  signal tapForDqs_val      : std_logic_vector(4 downto 0);
  signal cnt_val            : integer;
  signal reset_r            : std_logic;

  constant tap1       : std_logic_vector(4 downto 0) := "01111";
  constant tap2       : std_logic_vector(4 downto 0) := "10111";
  constant tap3       : std_logic_vector(4 downto 0) := "11011";
  constant tap4       : std_logic_vector(4 downto 0) := "11101";
  constant tap5       : std_logic_vector(4 downto 0) := "11110";
  constant tap6       : std_logic_vector(4 downto 0) := "11111";
  constant defaultTap : std_logic_vector(4 downto 0) := "11101";

   attribute syn_keep : boolean;
  attribute syn_keep of cnt                : signal is true;
  attribute syn_keep of trans_oneDtct      : signal is true;
  attribute syn_keep of trans_twoDtct      : signal is true;
  attribute syn_keep of tap_dly_reg        : signal is true;
  attribute syn_keep of enb_trans_two_dtct : signal is true;
  attribute syn_keep of phase_cnt          : signal is true;
  attribute syn_keep of tapForDqs_val      : signal is true;

begin

  process(clk)
  begin
    if(clk'event and clk = '1') then
      reset_r <= reset;
    end if;
  end process;

  process(clk)
  begin
    if(clk'event and clk = '1') then
      tapForDqs <= tapForDqs_val;
    end if;
  end process;
  
-- /**************** For Successive Transition**************/
  process(clk)
  begin
    if (clk'event and clk = '1') then
      if(reset_r = '1') then
        enb_trans_two_dtct <= '0';
      elsif(phase_cnt >= "00011") then
        enb_trans_two_dtct <= '1';
      else
        enb_trans_two_dtct <= '0';
      end if;
    end if;
  end process;

  process (clk)
  begin
    if(clk'event and clk = '1') then
      if(reset_r = '1') then
        tap_dly_reg <= "00000000000000000000000000000000";
      elsif(cnt(5) = '1') then
        tap_dly_reg <= flop2;
      else
        tap_dly_reg <= tap_dly_reg;
      end if;
    end if;
  end process;

-- /*************** Free Running Counter For Counting 32 States ***************/

  process (clk)
  begin
    if(clk'event and clk = '1') then
      if(reset_r = '1' or cnt(5) = '1') then
        cnt(5 downto 0) <= "000000";
      else
        cnt(5 downto 0) <= cnt(5 downto 0) + "000001";
      end if;
    end if;
  end process;


  process(clk)
  begin
    if(clk'event and clk = '1') then
      if(reset_r = '1' or cnt1(5) = '1') then
        cnt1(5 downto 0) <= "000000";
      else
        cnt1(5 downto 0) <= cnt1(5 downto 0) + "000001";
      end if;
    end if;
  end process;


  process(clk)
  begin
    if clk'event and clk = '1' then
      if(reset_r = '1' or cnt(5) = '1') then
        phase_cnt <= "00000";
      elsif (trans_oneDtct = '1' and trans_twoDtct = '0') then
        phase_cnt <= phase_cnt + "00001";
      else
        phase_cnt <= phase_cnt;
      end if;
    end if;
  end process;


-- /************* Checking For The First Transition ***************************/

  process (clk)
  begin
    if clk'event and clk = '1' then
      if (reset_r = '1') then
        trans_oneDtct <= '0';
        trans_twoDtct <= '0';
      elsif(cnt(5) = '1') then
        trans_oneDtct <= '0';
        trans_twoDtct <= '0';
      elsif (cnt(4 downto 0) = "00000") then
        if (tap_dly_reg(0) = '1') then
          trans_oneDtct <= '1';
          trans_twoDtct <= '0';
        end if;
      elsif (tap_dly_reg(cnt_val) = '1' and trans_twoDtct = '0') then
        if(trans_oneDtct = '1' and enb_trans_two_dtct = '1') then
          trans_twoDtct <= '1';
        else
          trans_oneDtct <= '1';
        end if;
      end if;
    end if;
  end process;
  cnt_val <= conv_integer(cnt(4 downto 0));

  process (clk)
  begin
    if clk'event and clk = '1' then
      if(reset_r = '1') then
        tapForDqs_val <= defaultTap;
      elsif(cnt1(4) = '1' and cnt1(3) = '1' and cnt1(2) = '1' and cnt1(1) = '1'
        and cnt1(0) = '1') then
        if ((trans_oneDtct = '0') or (trans_twoDtct = '0')
			or (phase_cnt > "01011")) then
          tapForDqs_val <= tap4;
        elsif (phase_cnt > "01000") then
          tapForDqs_val <= tap3;
        else
          tapForDqs_val <= tap2;
        end if;
      else
        tapForDqs_val <= tapForDqs_val;
      end if;
    end if;
  end process;

end arc_cal_ctl;
