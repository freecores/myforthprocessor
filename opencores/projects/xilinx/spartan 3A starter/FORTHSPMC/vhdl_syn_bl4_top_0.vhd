-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename           : vhdl_syn_bl4_top_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This is main controller block. This includes the following
--               features:
--                    - The main controller state machine that controlls the
--                    initialization process upon power up, as well as the
--                    read, write, and refresh commands.
--                    - handles the data path during READ and WRITEs.
--                    - Generates control signals for other modules, including the
--                      data strobe(DQS) signal
-------------------------------------------------------------------------------
library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use work.vhdl_syn_bl4_parameters_0.all;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl4_top_0 is
  port(
    wait_200us            : in    std_logic;
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
    burst_done            : in    std_logic;
    auto_ref_req          : out   std_logic;
    user_cmd_ack          : out   std_logic;
    init_val              : out   std_logic;
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
    DDR2_ODT              : out   std_logic;
    DDR2_CK   : out  std_logic;
    DDR2_CK_N : out  std_logic;

    clk_int               : in    std_logic;
    clk90_int             : in    std_logic;
    delay_sel_val         : in    std_logic_vector(4 downto 0);
    sys_rst               : in    std_logic;
    sys_rst90             : in    std_logic;
    sys_rst180            : in    std_logic
    );

end vhdl_syn_bl4_top_0;

architecture arc of vhdl_syn_bl4_top_0 is

  component vhdl_syn_bl4_controller_0
    port(
      auto_ref_req      : out std_logic;
      wait_200us        : in  std_logic;
      clk               : in  std_logic;
      rst0              : in  std_logic;
      rst180            : in  std_logic;
      address           : in  std_logic_vector(((row_address + col_ap_width)-1) 
                               downto 0);
      bank_addr         : in  std_logic_vector((bank_address-1) downto 0);
      command_register  : in  std_logic_vector(3 downto 0);
      burst_done        : in  std_logic;
      ddr_rasb_cntrl    : out std_logic;
      ddr_casb_cntrl    : out std_logic;
      ddr_web_cntrl     : out std_logic;
      ddr_ba_cntrl      : out std_logic_vector((bank_address-1) downto 0);
      ddr_address_cntrl : out std_logic_vector((row_address-1) downto 0);
      ddr_cke_cntrl     : out std_logic;
      ddr_csb_cntrl     : out std_logic;
      ddr_ODT_cntrl     : out std_logic;
      dqs_enable        : out std_logic;
      dqs_reset         : out std_logic;
      write_enable      : out std_logic;
      rst_calib         : out std_logic;
      rst_dqs_div_int   : out std_logic;
      cmd_ack           : out std_logic;
      init              : out std_logic;
      ar_done           : out std_logic
      );
  end component;


  component vhdl_syn_bl4_data_path_0
    port(
      user_input_data    : in  std_logic_vector(((2*data_width)-1) downto 0);
      user_data_mask     : in  std_logic_vector(((2*data_mask_width)-1) downto 0);
      clk                : in  std_logic;
      clk90              : in  std_logic;
      reset              : in  std_logic;
      reset90            : in  std_logic;
      write_enable       : in  std_logic;
      rst_dqs_div_in     : in  std_logic;
      delay_sel          : in  std_logic_vector(4 downto 0);
     dqs_int_delay_in0      : in std_logic;
     dqs_int_delay_in1      : in std_logic;
      dq                 : in  std_logic_vector((data_width-1) downto 0);
      u_data_val         : out std_logic;
      user_output_data   : out std_logic_vector(((2*data_width)-1) downto 0);
      write_en_val       : out std_logic;
    data_mask_f       : out std_logic_vector((data_mask_width-1) downto 0);
    data_mask_r       : out std_logic_vector((data_mask_width-1) downto 0);
      write_data_falling : out std_logic_vector((data_width-1) downto 0);
      write_data_rising  : out std_logic_vector((data_width-1) downto 0)
      );
  end component;


  component vhdl_syn_bl4_infrastructure
    port(
      clk_int            : in  std_logic;
      rst_calib1         : in  std_logic;
      delay_sel_val      : in  std_logic_vector(4 downto 0);
      delay_sel_val1_val : out std_logic_vector(4 downto 0)
      );
  end component;



  component vhdl_syn_bl4_iobs_0
    port(
      clk                : in    std_logic;
      clk90              : in    std_logic;
      ddr_rasb_cntrl     : in    std_logic;
      ddr_casb_cntrl     : in    std_logic;
      ddr_web_cntrl      : in    std_logic;
      ddr_cke_cntrl      : in    std_logic;
      ddr_csb_cntrl      : in    std_logic;
      ddr_ODT_cntrl      : in    std_logic;
      ddr_address_cntrl  : in    std_logic_vector((row_address-1) downto 0);
      ddr_ba_cntrl       : in    std_logic_vector((bank_address-1) downto 0);
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
      ddr_ba             : out   std_logic_vector((bank_address-1) downto 0);
      ddr_address        : out   std_logic_vector((row_address-1) downto 0);
      ddr_cke            : out   std_logic;
      ddr_csb            : out   std_logic;
      rst_dqs_div        : out   std_logic;
      rst_dqs_div_in     : in    std_logic;
      rst_dqs_div_out    : out   std_logic;

        dqs_int_delay_in0      : out std_logic;

        dqs_int_delay_in1      : out std_logic;
    ddr_dm                : out std_logic_vector((data_mask_width-1) downto 0);
      dq                 : out   std_logic_vector((data_width-1) downto 0)
      );
  end component;


  signal rst_calib          : std_logic;
  signal delay_sel          : std_logic_vector(4 downto 0);
  signal write_enable       : std_logic;
  signal dqs_div_rst        : std_logic;
  signal dqs_enable         : std_logic;
  signal dqs_reset          : std_logic;
    signal dqs_int_delay_in0       : std_logic; 
    signal dqs_int_delay_in1       : std_logic; 
  signal dq                 : std_logic_vector((data_width-1) downto 0);
  signal write_en_val       : std_logic;
    signal  data_mask_f  : std_logic_vector((data_mask_width-1) downto 0);
    signal  data_mask_r  : std_logic_vector((data_mask_width-1) downto 0);
  signal write_data_falling : std_logic_vector((data_width-1) downto 0);
  signal write_data_rising  : std_logic_vector((data_width-1) downto 0);
  signal ddr_rasb_cntrl     : std_logic;
  signal ddr_casb_cntrl     : std_logic;
  signal ddr_web_cntrl      : std_logic;
  signal ddr_ba_cntrl       : std_logic_vector((bank_address-1) downto 0);
  signal ddr_address_cntrl  : std_logic_vector((row_address-1) downto 0);
  signal ddr_cke_cntrl      : std_logic;
  signal ddr_csb_cntrl      : std_logic;
  signal ddr_ODT_cntrl      : std_logic;
  signal rst_dqs_div_int    : std_logic;
  signal user_mask	        : std_logic_vector(((data_mask_width*2)-1) downto 0);

begin



  user_mask        <=   user_data_mask;

  controller0 : vhdl_syn_bl4_controller_0
    port map (
      auto_ref_req      => auto_ref_req,
      wait_200us        => wait_200us,
      clk               => clk_int,
      rst0              => sys_rst,
      rst180            => sys_rst180,
      address           => user_input_address(((row_address + col_ap_width + 
                           bank_address)-1) downto bank_address),
      bank_addr         => user_input_address(bank_address-1 downto 0),
      command_register  => user_command_register,
      burst_done        => burst_done,
      ddr_rasb_cntrl    => ddr_rasb_cntrl,
      ddr_casb_cntrl    => ddr_casb_cntrl,
      ddr_web_cntrl     => ddr_web_cntrl,
      ddr_ba_cntrl      => ddr_ba_cntrl,
      ddr_address_cntrl => ddr_address_cntrl,
      ddr_cke_cntrl     => ddr_cke_cntrl,
      ddr_csb_cntrl     => ddr_csb_cntrl,
      ddr_ODT_cntrl     => ddr_ODT_cntrl,
      dqs_enable        => dqs_enable,
      dqs_reset         => dqs_reset,
      write_enable      => write_enable,
      rst_calib         => rst_calib,
      rst_dqs_div_int   => rst_dqs_div_int,
      cmd_ack           => user_cmd_ack,
      init              => init_val,
      ar_done           => ar_done
      );


  data_path0 : vhdl_syn_bl4_data_path_0
    port map (
      user_input_data    => user_input_data,
      user_data_mask     => user_mask,
      clk                => clk_int,
      clk90              => clk90_int,
      reset              => sys_rst,
      reset90            => sys_rst90,
      write_enable       => write_enable,
      rst_dqs_div_in     => dqs_div_rst,
      delay_sel          => delay_sel,
      dqs_int_delay_in0  => dqs_int_delay_in0,
      dqs_int_delay_in1  => dqs_int_delay_in1,
      dq                 => dq,
      u_data_val         => user_data_valid,
      user_output_data   => user_output_data,
      write_en_val       => write_en_val,
      data_mask_f        => data_mask_f,
      data_mask_r        => data_mask_r,
      write_data_falling => write_data_falling,
      write_data_rising  => write_data_rising
      );



  infrastructure0 : vhdl_syn_bl4_infrastructure
    port map (
      clk_int            => clk_int,
      rst_calib1         => rst_calib,
      delay_sel_val      => delay_sel_val,
      delay_sel_val1_val => delay_sel
      );


  iobs0 : vhdl_syn_bl4_iobs_0
    port map (
      clk                => clk_int,
      clk90              => clk90_int,
      ddr_rasb_cntrl     => ddr_rasb_cntrl,
      ddr_casb_cntrl     => ddr_casb_cntrl,
      ddr_ODT_cntrl      => ddr_ODT_cntrl,
      ddr_web_cntrl      => ddr_web_cntrl,
      ddr_cke_cntrl      => ddr_cke_cntrl,
      ddr_csb_cntrl      => ddr_csb_cntrl,
      ddr_address_cntrl  => ddr_address_cntrl,
      ddr_ba_cntrl       => ddr_ba_cntrl,
      rst_dqs_div_int    => rst_dqs_div_int,
      dqs_reset          => dqs_reset,
      dqs_enable         => dqs_enable,
      ddr_dqs            => DDR2_DQS,
    ddr_dqs_n            => DDR2_DQS_N,
      ddr_dq             => DDR2_DQ,
      write_data_falling => write_data_falling,
      write_data_rising  => write_data_rising,
      write_en_val       => write_en_val,
      data_mask_f        => data_mask_f,
      data_mask_r        => data_mask_r,
      ddr_ODT            => DDR2_ODT,
      DDR2_CK            => DDR2_CK,
      DDR2_CK_N          => DDR2_CK_N,
      ddr_rasb           => DDR2_RAS_N,
      ddr_casb           => DDR2_CAS_N,
      ddr_web            => DDR2_WE_N,
      ddr_ba             => DDR2_BA,
      ddr_address        => DDR2_A,
      ddr_cke            => DDR2_CKE,
      ddr_csb            => DDR2_CS_N,
      rst_dqs_div        => dqs_div_rst,
      rst_dqs_div_in     => rst_dqs_div_in,
      rst_dqs_div_out    => rst_dqs_div_out,
      dqs_int_delay_in0  => dqs_int_delay_in0,
      dqs_int_delay_in1  => dqs_int_delay_in1,
    ddr_dm             =>   DDR2_DM,
      dq                 => dq
      );


end arc;
