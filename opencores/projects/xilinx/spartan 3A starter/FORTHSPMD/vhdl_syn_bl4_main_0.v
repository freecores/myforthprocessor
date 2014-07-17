-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_main_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This modules has the instantiations top and test_bench.
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.vhdl_syn_bl4_parameters_0.all;
library UNISIM;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_main_0 is
  port(
    rst_dqs_div_in    : in  std_logic;
    rst_dqs_div_out   : out std_logic;
    delay_sel_val     : in  std_logic_vector(4 downto 0);
    clk_int           : in  std_logic;
    wait_200us        : in  std_logic;
    clk90_int         : in  std_logic;
    sys_rst           : in  std_logic;
    sys_rst90         : in  std_logic;
    sys_rst180        : in  std_logic;
    DDR2_CKE          : out std_logic;
    DDR2_CK   : out  std_logic;
    DDR2_CK_N : out  std_logic;

    DDR2_CS_N         : out   std_logic;
    DDR2_RAS_N        : out   std_logic;
    DDR2_CAS_N        : out   std_logic;
    DDR2_WE_N         : out   std_logic;
    DDR2_ODT          : out   std_logic;
    DDR2_A            : out   std_logic_vector((row_address - 1) downto 0);
    DDR2_BA           : out   std_logic_vector((bank_address - 1) downto 0);
    DDR2_DM                : out std_logic_vector((data_mask_width-1) downto 0);
    led_error_output1 : out   std_logic;
    data_valid_out    : out   std_logic;
    DDR2_DQS           : inout STD_LOGIC_VECTOR((data_strobe_width -1) downto 0);
    DDR2_DQS_N         : inout std_logic_vector((data_strobe_width-1) downto 0);
 
    DDR2_DQ           : inout std_logic_vector((data_width-1) downto 0)

    );
end vhdl_syn_bl4_main_0;

architecture arc of vhdl_syn_bl4_main_0 is

  component vhdl_syn_bl4_ddr2_test_bench_0
    port(
      fpga_clk         : in  std_logic;
      fpga_rst90       : in  std_logic;
      fpga_rst180      : in  std_logic;
      clk90            : in  std_logic;
      burst_done       : out std_logic;
      INIT_DONE        : in  std_logic;
      auto_ref_req     : in  std_logic;
      ar_done          : in  std_logic;
      u_ack            : in  std_logic;
      u_data_val       : in  std_logic;
      u_data_o         : in  std_logic_vector(((2*data_width)-1) downto 0);
      u_addr           : out std_logic_vector(((row_address + col_ap_width + 
                             bank_address)- 1) downto 0);
      u_cmd            : out std_logic_vector(3 downto 0);
      u_data_m         : out std_logic_vector(((2*data_mask_width)-1) downto 0);
      u_data_i         : out std_logic_vector(((2*data_width)-1) downto 0);
      led_error_output : out std_logic;
      data_valid_out   : out std_logic
      );
  end component;


  component vhdl_syn_bl4_top_0
    port(
      rst_dqs_div_in        : in    std_logic;
      rst_dqs_div_out       : out   std_logic;

      user_input_data       : in    std_logic_vector(((2*data_width)-1) downto 0);
    user_data_mask        : in std_logic_vector(((data_mask_width*2)-1) downto 0);
      user_output_data      : out   std_logic_vector(((2*data_width)-1) 
                                     downto 0) := (others => 'Z');
      user_data_valid       : out   std_logic;
      user_input_address    : in    std_logic_vector(((row_address + 
                                    col_ap_width + bank_address)-1) downto 0);
      user_command_register : in    std_logic_vector(3 downto 0);
      user_cmd_ack          : out   std_logic;
      burst_done            : in    std_logic;
      init_val              : out   std_logic;
      auto_ref_req          : out   std_logic;
      ar_done               : out   std_logic;
    DDR2_DQS           : inout STD_LOGIC_VECTOR((data_strobe_width -1) downto 0);
    DDR2_DQS_N         : inout std_logic_vector((data_strobe_width-1) downto 0);
 
      DDR2_DQ               : inout std_logic_vector((data_width-1) downto 0)
				:= (others => 'Z');
      DDR2_CKE              : out   std_logic;
      DDR2_CS_N             : out   std_logic;
      DDR2_RAS_N            : out   std_logic;
      DDR2_CAS_N            : out   std_logic;
      DDR2_WE_N             : out   std_logic;
    DDR2_DM                : out std_logic_vector((data_mask_width-1) downto 0);
      DDR2_BA               : out   std_logic_vector((bank_address-1) downto 0);
      DDR2_A                : out   std_logic_vector((row_address-1) downto 0);
      delay_sel_val         : in    std_logic_vector(4 downto 0);
      clk_int               : in    std_logic;
      wait_200us            : in    std_logic;
      clk90_int             : in    std_logic;
      sys_rst               : in    std_logic;
      sys_rst90             : in    std_logic;
      sys_rst180            : in    std_logic;
    DDR2_CK   : out  std_logic;
    DDR2_CK_N : out  std_logic;     
      DDR2_ODT              : out   std_logic
                                );
  end component;

  signal user_output_data : std_logic_vector(((2*data_width)-1) downto 0);
  signal u1_address : std_logic_vector(((row_address + col_ap_width +
                                               bank_address)-1) downto 0);
  signal user_data_val1  : std_logic;
  signal user_cmd1       : std_logic_vector(3 downto 0);
  signal auto_ref_req    : std_logic;
  signal user_ack1       : std_logic;
  signal u1_data_i       : std_logic_vector(((2*data_width)-1) downto 0);
  signal u1_data_m       : std_logic_vector(((2*data_mask_width)-1) downto 0);
  signal burst_done_val1 : std_logic;
  signal init_val1       : std_logic;
  signal ar_done_val1    : std_logic;
begin

  top0 : vhdl_syn_bl4_top_0
    port map(
      auto_ref_req          => auto_ref_req,
      wait_200us            => wait_200us,
      rst_dqs_div_in        => rst_dqs_div_in,
      rst_dqs_div_out       => rst_dqs_div_out,
      user_input_data       => u1_data_i,
      user_data_mask         =>   u1_data_m,
      user_output_data      => user_output_data,
      user_data_valid       => user_data_val1,
      user_input_address    => u1_address(((row_address + col_ap_width + 
						bank_address)-1) downto 0),						
      user_command_register => user_cmd1,
      user_cmd_ack          => user_ack1,
      burst_done            => burst_done_val1,
      init_val              => init_val1,
      ar_done               => ar_done_val1,
      DDR2_DQS              => DDR2_DQS,
      DDR2_DQ               => DDR2_DQ,
      DDR2_CKE              => DDR2_CKE,
      DDR2_CS_N             => DDR2_CS_N,
      DDR2_RAS_N            => DDR2_RAS_N,
      DDR2_CAS_N            => DDR2_CAS_N,
      DDR2_WE_N             => DDR2_WE_N,
       DDR2_DM             =>   DDR2_DM,

      DDR2_DQS_N          => DDR2_DQS_N,
      DDR2_BA               => DDR2_BA,
      DDR2_A                => DDR2_A,
      DDR2_ODT              => DDR2_ODT,
      DDR2_CK               => DDR2_CK,
      DDR2_CK_N             => DDR2_CK_N,
      clk90_int             => clk90_int,
      clk_int               => clk_int,
      delay_sel_val         => delay_sel_val,
      sys_rst               => sys_rst,
      sys_rst90             => sys_rst90,
      sys_rst180            => sys_rst180 
      );



  ddr2_test_bench0 : vhdl_syn_bl4_ddr2_test_bench_0
    port map (
      auto_ref_req     => auto_ref_req,
      fpga_clk         => clk_int,
      fpga_rst90       => sys_rst90,
      fpga_rst180      => sys_rst180,
      clk90            => clk90_int,
      burst_done       => burst_done_val1,
      INIT_DONE        => init_val1,
      ar_done          => ar_done_val1,
      u_ack            => user_ack1,
      u_data_val       => user_data_val1,
      u_data_o         => user_output_data,
      u_addr           => u1_address,
      u_cmd            => user_cmd1,
      u_data_i         => u1_data_i ,
      u_data_m         => u1_data_m,
      led_error_output => led_error_output1,
      data_valid_out   => data_valid_out
      );

end arc;
