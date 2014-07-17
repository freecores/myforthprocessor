-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_iobs_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the instantiations infrastructure_iobs, 
--							 data_path_iobs and controller_iobs modules.
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.vhdl_syn_bl4_parameters_0.all;


library UNISIM;
use UNISIM.VCOMPONENTS.all;


entity vhdl_syn_bl4_iobs_0 is
  port(
    clk                : in    std_logic;
    clk90              : in    std_logic;
    ddr_rasb_cntrl     : in    std_logic;
    ddr_casb_cntrl     : in    std_logic;
    ddr_web_cntrl      : in    std_logic;
    ddr_cke_cntrl      : in    std_logic;
    ddr_csb_cntrl      : in    std_logic;
    ddr_address_cntrl  : in    std_logic_vector((row_address -1) downto 0);
    ddr_ba_cntrl       : in    std_logic_vector((bank_address -1) downto 0);
    ddr_ODT_cntrl      : in    std_logic;
    rst_dqs_div_int    : in    std_logic;
    dqs_reset          : in    std_logic;
    dqs_enable         : in    std_logic;
    ddr_dqs           : inout STD_LOGIC_VECTOR((data_strobe_width -1) downto 0);
    ddr_dqs_n          : inout std_logic_vector((data_strobe_width-1) downto 0);
    ddr_dq             : inout std_logic_vector((data_width-1) downto 0);
    write_data_falling : in    std_logic_vector((data_width-1) downto 0);
    write_data_rising  : in    std_logic_vector((data_width-1) downto 0);
    write_en_val       : in    std_logic;
    data_mask_f       : in std_logic_vector((data_mask_width-1) downto 0);
    data_mask_r       : in std_logic_vector((data_mask_width-1) downto 0);
    ddr_ODT            : out   std_logic;
    DDR2_CK   : out  std_logic;
    DDR2_CK_N : out  std_logic;
    ddr_rasb           : out   std_logic;
    ddr_casb           : out   std_logic;
    ddr_web            : out   std_logic;
    ddr_ba             : out   std_logic_vector((bank_address -1) downto 0);
    ddr_address        : out   std_logic_vector((row_address -1) downto 0);
    ddr_cke            : out   std_logic;
    ddr_csb            : out   std_logic;
    rst_dqs_div        : out   std_logic;
    rst_dqs_div_in     : in    std_logic;
    rst_dqs_div_out    : out   std_logic;
    dqs_int_delay_in0  : out std_logic ;
    dqs_int_delay_in1  : out std_logic ;
    ddr_dm                : out std_logic_vector((data_mask_width-1) downto 0);
    dq                 : out   std_logic_vector((data_width-1) downto 0)
    );
end vhdl_syn_bl4_iobs_0;


architecture arc of vhdl_syn_bl4_iobs_0 is

  component vhdl_syn_bl4_infrastructure_iobs_0
    port(
    DDR2_CK   : out  std_logic;
    DDR2_CK_N : out  std_logic;
      clk0  : in std_logic
      );
  end component;


  component vhdl_syn_bl4_controller_iobs_0
    port(
      clk0              : in  std_logic;
      ddr_rasb_cntrl    : in  std_logic;
      ddr_casb_cntrl    : in  std_logic;
      ddr_web_cntrl     : in  std_logic;
      ddr_cke_cntrl     : in  std_logic;
      ddr_csb_cntrl     : in  std_logic;
      ddr_address_cntrl : in  std_logic_vector((row_address -1) downto 0);
      ddr_ba_cntrl      : in  std_logic_vector((bank_address -1) downto 0);
      ddr_ODT_cntrl     : in  std_logic;
      rst_dqs_div_int   : in  std_logic;
      ddr_rasb          : out std_logic;
      ddr_casb          : out std_logic;
      ddr_web           : out std_logic;
      ddr_ba            : out std_logic_vector((bank_address -1) downto 0);
      ddr_address       : out std_logic_vector((row_address -1) downto 0);
      ddr_cke           : out std_logic;
      ddr_csb           : out std_logic;
      ddr_ODT           : out std_logic;
      rst_dqs_div       : out std_logic;
      rst_dqs_div_in    : in  std_logic;
      rst_dqs_div_out   : out std_logic
      );
  end component;


  component vhdl_syn_bl4_data_path_iobs_0
    port(
      clk                : in    std_logic;
      clk90              : in    std_logic;
      dqs_reset          : in    std_logic;
      dqs_enable         : in    std_logic;
      ddr_dq             : inout std_logic_vector((data_width-1) downto 0);
    ddr_dqs           : inout STD_LOGIC_VECTOR((data_strobe_width -1) downto 0);
    ddr_dqs_n          : inout std_logic_vector((data_strobe_width-1) downto 0);
      write_data_falling : in    std_logic_vector((data_width-1) downto 0);
      write_data_rising  : in    std_logic_vector((data_width-1) downto 0);
      write_en_val       : in    std_logic;
    data_mask_f       : in std_logic_vector((data_mask_width-1) downto 0);
    data_mask_r       : in std_logic_vector((data_mask_width-1) downto 0);
    dqs_int_delay_in0  : out std_logic ;
    dqs_int_delay_in1  : out std_logic ;
    ddr_dm                : out std_logic_vector((data_mask_width-1) downto 0);
      ddr_dq_val         : out   std_logic_vector((data_width-1) downto 0)
      );
  end component;

begin


  infrastructure_iobs0 : vhdl_syn_bl4_infrastructure_iobs_0 
    port map (
      clk0      => clk,
      DDR2_CK   => DDR2_CK,
      DDR2_CK_N => DDR2_CK_N
      );



  controller_iobs0 : vhdl_syn_bl4_controller_iobs_0 
    port map (
      clk0              => clk,
      ddr_rasb_cntrl    => ddr_rasb_cntrl,
      ddr_casb_cntrl    => ddr_casb_cntrl,
      ddr_web_cntrl     => ddr_web_cntrl,
      ddr_cke_cntrl     => ddr_cke_cntrl,
      ddr_csb_cntrl     => ddr_csb_cntrl,
      ddr_ODT_cntrl     => ddr_ODT_cntrl,
      ddr_address_cntrl => ddr_address_cntrl((row_address -1) downto 0),
      ddr_ba_cntrl      => ddr_ba_cntrl((bank_address -1) downto 0), 
      rst_dqs_div_int   => rst_dqs_div_int,
      ddr_rasb          => ddr_rasb,
      ddr_casb          => ddr_casb,
      ddr_web           => ddr_web,
      ddr_ba            => ddr_ba((bank_address -1) downto 0),
      ddr_address       => ddr_address((row_address -1) downto 0), 
      ddr_cke		=> ddr_cke,
      ddr_csb           => ddr_csb,
      ddr_ODT           => ddr_ODT,
      rst_dqs_div       => rst_dqs_div,
      rst_dqs_div_in    => rst_dqs_div_in,
      rst_dqs_div_out   => rst_dqs_div_out
      );



  datapath_iobs0 : vhdl_syn_bl4_data_path_iobs_0 port map (
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
      dqs_int_delay_in0  => dqs_int_delay_in0,
      dqs_int_delay_in1  => dqs_int_delay_in1,
    ddr_dm             =>   ddr_dm,
    ddr_dq_val         => dq
    );



end arc;







