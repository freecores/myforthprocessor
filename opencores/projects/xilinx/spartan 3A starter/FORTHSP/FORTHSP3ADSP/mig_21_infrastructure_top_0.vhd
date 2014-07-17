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
--  /   /        Filename           : mig_21_infrastructure_top.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has instantiations clk_dcm,cal_top and generate 
--               reset signals to the design
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.mig_21_parameters_0.all;

entity mig_21_infrastructure_top is
  port(
    reset_in_n             : in  std_logic;
    sys_clk                : in  std_logic;
    sys_clkb               : in  std_logic;
    delay_sel_val1_val     : out std_logic_vector(4 downto 0);
    sys_rst_val            : out std_logic;
    sys_rst90_val          : out std_logic;
    clk_int_val            : out std_logic;
    clk90_int_val          : out std_logic;
    sys_rst180_val         : out std_logic;
    wait_200us             : out std_logic;
    -- debug signals
    dbg_phase_cnt          : out std_logic_vector(4 downto 0);
    dbg_cnt                : out std_logic_vector(5 downto 0);
    dbg_trans_onedtct      : out std_logic;
    dbg_trans_twodtct      : out std_logic;
    dbg_enb_trans_two_dtct : out std_logic
    );

end mig_21_infrastructure_top;

architecture arc of mig_21_infrastructure_top is

  component mig_21_clk_dcm
    port(
      input_clk : in  std_logic;
      rst       : in  std_logic;
      clk       : out std_logic;
      clk90     : out std_logic;
      dcm_lock  : out std_logic
      );
  end component;

  component mig_21_cal_top
    port (
      clk                    : in  std_logic;
      clk0dcmlock            : in  std_logic;
      reset                  : in  std_logic;
      tapfordqs              : out std_logic_vector(4 downto 0);
      dbg_phase_cnt          : out std_logic_vector(4 downto 0);
      dbg_cnt                : out std_logic_vector(5 downto 0);
      dbg_trans_onedtct      : out std_logic;
      dbg_trans_twodtct      : out std_logic;
      dbg_enb_trans_two_dtct : out std_logic
      );
  end component;

  signal user_rst       : std_logic;
  signal user_cal_rst   : std_logic;
  signal clk_int        : std_logic;
  signal clk90_int      : std_logic;
  signal dcm_lock       : std_logic;
  signal sys_rst_o      : std_logic;
  signal sys_rst_1      : std_logic := '1';
  signal sys_rst        : std_logic;
  signal sys_rst90_o    : std_logic;
  signal sys_rst90_1    : std_logic := '1';
  signal sys_rst90      : std_logic;
  signal sys_rst180_o   : std_logic;
  signal sys_rst180_1   : std_logic := '1';
  signal sys_rst180     : std_logic;
  signal delay_sel_val1 : std_logic_vector(4 downto 0);
  signal clk_int_val1   : std_logic;
  signal clk_int_val2   : std_logic;
  signal clk90_int_val1 : std_logic;
  signal clk90_int_val2 : std_logic;
  signal wait_200us_i   : std_logic;
  signal wait_200us_int : std_logic;
  signal wait_clk90     : std_logic;
  signal wait_clk270    : std_logic;
  signal counter200     : std_logic_vector(15 downto 0);
  signal sys_clk_ibuf   : std_logic;

begin

  lvds_clk_input : IBUFGDS_LVDS_25
    port map(
      I  => sys_clk,
      IB => sys_clkb,
      O  => sys_clk_ibuf
      );
  
  clk_int_val        <= clk_int;
  clk90_int_val      <= clk90_int;
  sys_rst_val        <= sys_rst;
  sys_rst90_val      <= sys_rst90;
  sys_rst180_val     <= sys_rst180;
  delay_sel_val1_val <= delay_sel_val1;


-- To remove delta delays in the clock signals observed during simulation
-- ,Following signals are used

  clk_int_val1   <= clk_int;
  clk90_int_val1 <= clk90_int;
  clk_int_val2   <= clk_int_val1;
  clk90_int_val2 <= clk90_int_val1;
  user_rst       <= not reset_in_n when RESET_ACTIVE_LOW = '1' else reset_in_n;
  user_cal_rst   <= reset_in_n     when RESET_ACTIVE_LOW = '1' else not reset_in_n;

  process(clk_int_val2)
  begin
    if clk_int_val2'event and clk_int_val2 = '1' then
      if user_rst = '1' or dcm_lock = '0' then
        wait_200us_i   <= '1';
        counter200     <= (others => '0');
      else
        if( counter200 < 33400) then
          wait_200us_i <= '1';
          counter200   <= counter200 + 1;
        else
          counter200   <= counter200;
          wait_200us_i <= '0';
        end if;
      end if;
    end if;
  end process;

  process(clk_int_val2)
  begin
    if clk_int_val2'event and clk_int_val2 = '1' then
      wait_200us <= wait_200us_i;
    end if;
  end process;

  process(clk_int_val2)
  begin
    if clk_int_val2'event and clk_int_val2 = '1' then
      wait_200us_int <= wait_200us_i;
    end if;
  end process;

  process(clk90_int_val2)
  begin
    if clk90_int_val2'event and clk90_int_val2 = '0' then
      if user_rst = '1' or dcm_lock = '0' then
        wait_clk270 <= '1';
      else
        wait_clk270 <= wait_200us_int;
      end if;
    end if;
  end process;

  process(clk90_int_val2)
  begin
    if clk90_int_val2'event and clk90_int_val2 = '1' then
      wait_clk90 <= wait_clk270;
    end if;
  end process;

  process(clk_int_val2)
  begin
    if clk_int_val2'event and clk_int_val2 = '1' then
      if user_rst = '1' or dcm_lock = '0' or wait_200us_int = '1' then
        sys_rst_o <= '1';
        sys_rst_1 <= '1';
        sys_rst   <= '1';
      else
        sys_rst_o <= '0';
        sys_rst_1 <= sys_rst_o;
        sys_rst   <= sys_rst_1;
      end if;
    end if;
  end process;

  process(clk90_int_val2)
  begin
    if clk90_int_val2'event and clk90_int_val2 = '1' then
      if user_rst = '1' or dcm_lock = '0' or wait_clk90 = '1' then
        sys_rst90_o <= '1';
        sys_rst90_1 <= '1';
        sys_rst90   <= '1';
      else
        sys_rst90_o <= '0';
        sys_rst90_1 <= sys_rst90_o;
        sys_rst90   <= sys_rst90_1;
      end if;
    end if;
  end process;

  process(clk_int_val2)
  begin
    if clk_int_val2'event and clk_int_val2 = '0' then
      if user_rst = '1' or dcm_lock = '0' or wait_clk270 = '1' then
        sys_rst180_o <= '1';
        sys_rst180_1 <= '1';
        sys_rst180   <= '1';
      else
        sys_rst180_o <= '0';
        sys_rst180_1 <= sys_rst180_o;
        sys_rst180   <= sys_rst180_1;
      end if;
    end if;
  end process;

  clk_dcm0 : mig_21_clk_dcm
    port map (
      input_clk => sys_clk_ibuf,
      rst       => user_rst,
      clk       => clk_int,
      clk90     => clk90_int,
      dcm_lock  => dcm_lock
      );

  cal_top0 : mig_21_cal_top
    port map (
      clk                    => clk_int_val2,
      clk0dcmlock            => dcm_lock,
      reset                  => user_cal_rst,
      tapfordqs              => delay_sel_val1,
      dbg_phase_cnt          => dbg_phase_cnt,
      dbg_cnt                => dbg_cnt,
      dbg_trans_onedtct      => dbg_trans_onedtct,
      dbg_trans_twodtct      => dbg_trans_twodtct,
      dbg_enb_trans_two_dtct => dbg_enb_trans_two_dtct
      );

end arc;
