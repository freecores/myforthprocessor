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
--  /   /        Filename           : mig_21_iobs_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the instantiations infrastructure_iobs,
--                data_path_iobs and controller_iobs modules.
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.mig_21_parameters_0.all;


library UNISIM;
use UNISIM.VCOMPONENTS.all;


entity mig_21_iobs_0 is
  port(
    clk                : in    std_logic;
    clk90              : in    std_logic;
    ddr_rasb_cntrl     : in    std_logic;
    ddr_casb_cntrl     : in    std_logic;
    ddr_web_cntrl      : in    std_logic;
    ddr_cke_cntrl      : in    std_logic;
    ddr_csb_cntrl      : in    std_logic;
    ddr_address_cntrl  : in    std_logic_vector((ROW_ADDRESS -1) downto 0);
    ddr_ba_cntrl       : in    std_logic_vector((BANK_ADDRESS -1) downto 0);
    ddr_odt_cntrl      : in std_logic;
    rst_dqs_div_int    : in std_logic;
    dqs_reset          : in std_logic;
    dqs_enable         : in std_logic;
    ddr_dqs            : inout std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    ddr_dqs_n         : inout std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
 
    ddr_dq             : inout std_logic_vector((DATA_WIDTH-1) downto 0);
    write_data_falling : in    std_logic_vector((DATA_WIDTH-1) downto 0);
    write_data_rising  : in    std_logic_vector((DATA_WIDTH-1) downto 0);
    write_en_val       : in    std_logic;
    data_mask_f        : in std_logic_vector((DATA_MASK_WIDTH-1) downto 0);
    data_mask_r        : in std_logic_vector((DATA_MASK_WIDTH-1) downto 0);
    ddr_odt            : out   std_logic;
    ddr2_ck            : out  std_logic_vector((CLK_WIDTH-1)  downto 0);
    ddr2_ck_n          : out  std_logic_vector((CLK_WIDTH-1)  downto 0);
    ddr_rasb           : out   std_logic;
    ddr_casb           : out   std_logic;
    ddr_web            : out   std_logic;
    ddr_ba             : out   std_logic_vector((BANK_ADDRESS -1) downto 0);
    ddr_address        : out   std_logic_vector((ROW_ADDRESS -1) downto 0);
    ddr_cke            : out   std_logic;
    ddr_csb            : out   std_logic;
    rst_dqs_div        : out   std_logic;
    rst_dqs_div_in     : in    std_logic;
    rst_dqs_div_out    : out   std_logic;
    dqs_int_delay_in   : out std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    ddr_dm             : out std_logic_vector((DATA_MASK_WIDTH-1) downto 0);
    dq                 : out   std_logic_vector((DATA_WIDTH-1) downto 0)
    );
  ATTRIBUTE X_CORE_INFO          : STRING;
  ATTRIBUTE CORE_GENERATION_INFO : STRING;

  ATTRIBUTE X_CORE_INFO of mig_21_iobs_0 : ENTITY IS "mig_v2_1_ddr2_sp3, Coregen 10.1i_ip0";
  ATTRIBUTE CORE_GENERATION_INFO of mig_21_iobs_0 : ENTITY IS "ddr2_sp3,mig_v2_1,{component_name=ddr2_sp3, data_width=32, memory_width=8, clk_width=2, bank_address=2, row_address=13, column_address=10, no_of_cs=1, cke_width=1, registered=0, data_mask=1, mask_enable=1, load_mode_register=0010100110010, ext_load_mode_register=0000000000000}";
end mig_21_iobs_0;

architecture arc of mig_21_iobs_0 is

  component mig_21_infrastructure_iobs_0
    port(
      ddr2_ck   : out  std_logic_vector((CLK_WIDTH-1)  downto 0);
      ddr2_ck_n : out  std_logic_vector((CLK_WIDTH-1)  downto 0);
      clk0  : in std_logic
      );
  end component;

  component mig_21_controller_iobs_0
    port(
      clk0              : in  std_logic;
      ddr_rasb_cntrl    : in  std_logic;
      ddr_casb_cntrl    : in  std_logic;
      ddr_web_cntrl     : in  std_logic;
      ddr_cke_cntrl     : in  std_logic;
      ddr_csb_cntrl     : in  std_logic;
      ddr_address_cntrl : in  std_logic_vector((ROW_ADDRESS -1) downto 0);
      ddr_ba_cntrl      : in  std_logic_vector((BANK_ADDRESS -1) downto 0);
      ddr_odt_cntrl     : in  std_logic;
      rst_dqs_div_int   : in  std_logic;
      ddr_rasb          : out std_logic;
      ddr_casb          : out std_logic;
      ddr_web           : out std_logic;
      ddr_ba            : out std_logic_vector((BANK_ADDRESS -1) downto 0);
      ddr_address       : out std_logic_vector((ROW_ADDRESS -1) downto 0);
      ddr_cke           : out std_logic;
      ddr_csb           : out std_logic;
      ddr_ODT           : out std_logic;
      rst_dqs_div       : out std_logic;
      rst_dqs_div_in    : in  std_logic;
      rst_dqs_div_out   : out std_logic
      );
  end component;

  component mig_21_data_path_iobs_0
    port(
      clk                : in    std_logic;
      clk90              : in    std_logic;
      dqs_reset          : in    std_logic;
      dqs_enable         : in    std_logic;
      ddr_dq             : inout std_logic_vector((DATA_WIDTH-1) downto 0);
      ddr_dqs            : inout std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    ddr_dqs_n         : inout std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
 
      write_data_falling : in    std_logic_vector((DATA_WIDTH-1) downto 0);
      write_data_rising  : in    std_logic_vector((DATA_WIDTH-1) downto 0);
      write_en_val       : in    std_logic;
      data_mask_f        : in std_logic_vector((DATA_MASK_WIDTH-1) downto 0);
      data_mask_r        : in std_logic_vector((DATA_MASK_WIDTH-1) downto 0);
      dqs_int_delay_in   : out std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
      ddr_dm             : out std_logic_vector((DATA_MASK_WIDTH-1) downto 0);
      ddr_dq_val         : out   std_logic_vector((DATA_WIDTH-1) downto 0)
      );
  end component;

begin

  infrastructure_iobs0 : mig_21_infrastructure_iobs_0
    port map (
      clk0      => clk,
      ddr2_ck   => ddr2_ck,
      ddr2_ck_n => ddr2_ck_n
      );

  controller_iobs0 : mig_21_controller_iobs_0
    port map (
      clk0              => clk,
      ddr_rasb_cntrl    => ddr_rasb_cntrl,
      ddr_casb_cntrl    => ddr_casb_cntrl,
      ddr_web_cntrl     => ddr_web_cntrl,
      ddr_cke_cntrl     => ddr_cke_cntrl,
      ddr_csb_cntrl     => ddr_csb_cntrl,
      ddr_odt_cntrl     => ddr_odt_cntrl,
      ddr_address_cntrl => ddr_address_cntrl((ROW_ADDRESS -1) downto 0),
      ddr_ba_cntrl      => ddr_ba_cntrl((BANK_ADDRESS -1) downto 0),
      rst_dqs_div_int   => rst_dqs_div_int,
      ddr_rasb          => ddr_rasb,
      ddr_casb          => ddr_casb,
      ddr_web           => ddr_web,
      ddr_ba            => ddr_ba((BANK_ADDRESS -1) downto 0),
      ddr_address       => ddr_address((ROW_ADDRESS -1) downto 0),
      ddr_cke           => ddr_cke,
      ddr_csb           => ddr_csb,
      ddr_odt           => ddr_odt,
      rst_dqs_div       => rst_dqs_div,
      rst_dqs_div_in    => rst_dqs_div_in,
      rst_dqs_div_out   => rst_dqs_div_out
      );

  datapath_iobs0 : mig_21_data_path_iobs_0
    port map (
      clk                => clk,
      clk90              => clk90,
      dqs_reset          => dqs_reset,
      dqs_enable         => dqs_enable,
      ddr_dqs            => ddr_dqs,
    ddr_dqs_n         => ddr_dqs_n,
      ddr_dq             => ddr_dq,
      write_data_falling => write_data_falling,
      write_data_rising  => write_data_rising,
      write_en_val       => write_en_val,
      data_mask_f        => data_mask_f,
      data_mask_r        => data_mask_r,
      dqs_int_delay_in   => dqs_int_delay_in,
      ddr_dm             => ddr_dm,
      ddr_dq_val         => dq
    );

end arc;
