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
--  /   /        Filename           : mig_21_cal_ctl.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : This module generates the select lines for the LUT delay
--               circuit that generate the required delay for the DQS with
--               respect to the DQ. It calculates the dealy of a LUT dynalically
--               by finding the number of LUTs in a clock phase.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;

entity mig_21_cal_ctl is
  port (
    clk                    : in  std_logic;
    reset                  : in  std_logic;
    flop2                  : in  std_logic_vector(31 downto 0);
    tapfordqs              : out std_logic_vector(4 downto 0);
    -- debug signals
    dbg_phase_cnt          : out std_logic_vector(4 downto 0);
    dbg_cnt                : out std_logic_vector(5 downto 0);
    dbg_trans_onedtct      : out std_logic;
    dbg_trans_twodtct      : out std_logic;
    dbg_enb_trans_two_dtct : out std_logic
    );
end mig_21_cal_ctl;

architecture arc_cal_ctl of mig_21_cal_ctl is

  signal cnt                : std_logic_vector(5 downto 0);
  signal cnt1               : std_logic_vector(5 downto 0);
  signal trans_onedtct      : std_logic;
  signal trans_twodtct      : std_logic;
  signal phase_cnt          : std_logic_vector(4 downto 0);
  signal tap_dly_reg        : std_logic_vector(31 downto 0);
  signal enb_trans_two_dtct : std_logic;
  signal tapfordqs_val      : std_logic_vector(4 downto 0);
  signal cnt_val            : integer;
  signal reset_r            : std_logic;

  constant tap1        : std_logic_vector(4 downto 0) := "01111";
  constant tap2        : std_logic_vector(4 downto 0) := "10111";
  constant tap3        : std_logic_vector(4 downto 0) := "11011";
  constant tap4        : std_logic_vector(4 downto 0) := "11101";
  constant tap5        : std_logic_vector(4 downto 0) := "11110";
  constant tap6        : std_logic_vector(4 downto 0) := "11111";
  constant default_tap : std_logic_vector(4 downto 0) := "11101";

  attribute syn_keep : boolean;
  attribute syn_keep of cnt                : signal is true;
  attribute syn_keep of trans_onedtct      : signal is true;
  attribute syn_keep of trans_twodtct      : signal is true;
  attribute syn_keep of tap_dly_reg        : signal is true;
  attribute syn_keep of enb_trans_two_dtct : signal is true;
  attribute syn_keep of phase_cnt          : signal is true;
  attribute syn_keep of tapfordqs_val      : signal is true;

begin

  dbg_phase_cnt          <= phase_cnt;
  dbg_cnt                <= cnt;
  dbg_trans_onedtct      <= trans_onedtct;
  dbg_trans_twodtct      <= trans_twodtct;
  dbg_enb_trans_two_dtct <= enb_trans_two_dtct;

  process(clk)
  begin
    if(clk'event and clk = '1') then
      reset_r <= reset;
    end if;
  end process;

  process(clk)
  begin
    if(clk'event and clk = '1') then
      tapfordqs <= tapfordqs_val;
    end if;
  end process;

-----------For Successive Transition-------------------

  process(clk)
  begin
    if (clk'event and clk = '1') then
      if(reset_r = '1') then
        enb_trans_two_dtct <= '0';
      elsif(phase_cnt >= "00001") then
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

--------Free Running Counter For Counting 32 States ----------------------
------- Two parallel counters are used to fix the timing ------------------

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
      elsif (trans_onedtct = '1' and trans_twodtct = '0') then
        phase_cnt <= phase_cnt + "00001";
      else
        phase_cnt <= phase_cnt;
      end if;
    end if;
  end process;

----------- Checking For The First Transition ------------------

  process (clk)
  begin
    if clk'event and clk = '1' then
      if (reset_r = '1') then
        trans_onedtct <= '0';
        trans_twodtct <= '0';
      elsif(cnt(5) = '1') then
        trans_onedtct <= '0';
        trans_twodtct <= '0';
      elsif (cnt(4 downto 0) = "00000") then
        if (tap_dly_reg(0) = '1') then
          trans_onedtct <= '1';
          trans_twodtct <= '0';
        end if;
      elsif (tap_dly_reg(cnt_val) = '1' and trans_twodtct = '0') then
        if(trans_onedtct = '1' and enb_trans_two_dtct = '1') then
          trans_twodtct <= '1';
        else
          trans_onedtct <= '1';
        end if;
      end if;
    end if;
  end process;

  cnt_val <= conv_integer(cnt(4 downto 0));

  -- Tap values for Left/Right banks
  process (clk)
  begin
    if clk'event and clk = '1' then
      if(reset_r = '1') then
        tapfordqs_val <= default_tap;
      elsif(cnt1(4) = '1' and cnt1(3) = '1' and cnt1(2) = '1' and cnt1(1) = '1'
        and cnt1(0) = '1') then
        if ((trans_onedtct = '0') or (trans_twodtct = '0')
			or (phase_cnt > "01011")) then
          tapfordqs_val <= tap4;
        elsif (phase_cnt > "01000") then
          tapfordqs_val <= tap3;
        else
          tapfordqs_val <= tap2;
        end if;
      else
        tapfordqs_val <= tapfordqs_val;
      end if;
    end if;
  end process;

end arc_cal_ctl;
