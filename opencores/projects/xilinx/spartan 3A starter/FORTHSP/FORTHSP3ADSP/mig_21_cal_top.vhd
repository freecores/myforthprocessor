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
--  /   /        Filename           : mig_21_cal_to.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the instantiations cal_ctl and tap_dly.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;

entity mig_21_cal_top is
  port(
    clk                    : in  std_logic;
    clk0dcmlock            : in  std_logic;
    reset                  : in  std_logic;
    tapfordqs              : out std_logic_vector(4 downto 0);
    -- debug signals
    dbg_phase_cnt          : out std_logic_vector(4 downto 0);
    dbg_cnt                : out std_logic_vector(5 downto 0);
    dbg_trans_onedtct      : out std_logic;
    dbg_trans_twodtct      : out std_logic;
    dbg_enb_trans_two_dtct : out std_logic
    );
  ATTRIBUTE X_CORE_INFO          : STRING;
  ATTRIBUTE CORE_GENERATION_INFO : STRING;

  ATTRIBUTE X_CORE_INFO of mig_21_cal_top : ENTITY IS "mig_v2_1_ddr2_sp3, Coregen 10.1i_ip0";
  ATTRIBUTE CORE_GENERATION_INFO of mig_21_cal_top  : ENTITY IS "ddr2_sp3,mig_v2_1,{component_name=ddr2_sp3, data_width=32, memory_width=8, clk_width=2, bank_address=2, row_address=13, column_address=10, no_of_cs=1, cke_width=1, registered=0, data_mask=1, mask_enable=1, load_mode_register=0010100110010, ext_load_mode_register=0000000000000}";
end mig_21_cal_top;

architecture arc of mig_21_cal_top is

  component mig_21_cal_ctl
    port (
      clk                    : in  std_logic;
      reset                  : in  std_logic;
      flop2                  : in  std_logic_vector(31 downto 0);
      tapfordqs              : out std_logic_vector(4 downto 0);
      dbg_phase_cnt          : out std_logic_vector(4 downto 0);
      dbg_cnt                : out std_logic_vector(5 downto 0);
      dbg_trans_onedtct      : out std_logic;
      dbg_trans_twodtct      : out std_logic;
      dbg_enb_trans_two_dtct : out std_logic
      );
  end component;

  component mig_21_tap_dly
    port (
      clk   : in  std_logic;
      reset : in  std_logic;
      tapin : in  std_logic;
      flop2 : out std_logic_vector(31 downto 0)
      );
  end component;

  signal fpga_rst  : std_logic;
  signal flop2_val : std_logic_vector(31 downto 0);

begin

  fpga_rst <= (not reset) or (not clk0dcmlock);

  cal_ctl0 : mig_21_cal_ctl
    port map(
      clk                    => clk,
      reset                  => fpga_rst,
      flop2                  => flop2_val,
      tapfordqs              => tapfordqs,
      dbg_phase_cnt          => dbg_phase_cnt,
      dbg_cnt                => dbg_cnt,
      dbg_trans_onedtct      => dbg_trans_onedtct,
      dbg_trans_twodtct      => dbg_trans_twodtct,
      dbg_enb_trans_two_dtct => dbg_enb_trans_two_dtct
      );

  tap_dly0 : mig_21_tap_dly
    port map (
      clk                    => clk,
      reset                  => fpga_rst,
      tapin		     => clk,
      flop2		     => flop2_val
      );

end arc;
