-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_data_path_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $	
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the write and read data paths for the
--               DDR2 memory interface. The write data along with write enable
--               signals are forwarded to the DDR IOB FFs. The read data is
--               captured in CLB FFs and finally input to FIFOs.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_data_path_0 is
  port(
    user_input_data    : in  std_logic_vector(((2*data_width)-1) downto 0);
    user_data_mask     : in  std_logic_vector((2*data_mask_width-1) downto 0);
    clk                : in  std_logic;
    clk90              : in  std_logic;
    reset              : in  std_logic;
    reset90            : in  std_logic;
    write_enable       : in  std_logic;
    rst_dqs_div_in     : in  std_logic;
    delay_sel          : in  std_logic_vector(4 downto 0);
    dqs_int_delay_in0     : in std_logic;
    dqs_int_delay_in1     : in std_logic;
    dq                 : in  std_logic_vector((data_width-1) downto 0);
    u_data_val         : out std_logic;
    user_output_data   : out std_logic_vector(((2*data_width)-1) downto 0);
    write_en_val       : out std_logic;
    data_mask_f       : out std_logic_vector((data_mask_width-1) downto 0);
    data_mask_r       : out std_logic_vector((data_mask_width-1) downto 0);
    write_data_falling : out std_logic_vector((data_width-1) downto 0);
    write_data_rising  : out std_logic_vector((data_width-1) downto 0)
    );
end vhdl_syn_bl4_data_path_0;

architecture arc of vhdl_syn_bl4_data_path_0 is
  attribute KEEP_HIERARCHY : string;
  attribute KEEP_HIERARCHY of arc : architecture is "YES";

  component vhdl_syn_bl4_data_read_0 
    port(
      clk90             : in  std_logic;
      reset90_r         : in  std_logic;
      ddr_dq_in         : in  std_logic_vector((data_width-1) downto 0);
      read_valid_data_1 : in  std_logic;
      fifo_00_wr_en      : in std_logic;
      fifo_10_wr_en      : in std_logic;    fifo_01_wr_en      : in std_logic;
    fifo_11_wr_en      : in std_logic;
      fifo_00_wr_addr    : in std_logic_vector(3 downto 0);
      fifo_01_wr_addr    : in std_logic_vector(3 downto 0); 
      fifo_10_wr_addr    : in std_logic_vector(3 downto 0);
      fifo_11_wr_addr    : in std_logic_vector(3 downto 0); 
      dqs0_delayed_col1      : in std_logic;
      dqs1_delayed_col1      : in std_logic;      dqs0_delayed_col0      : in std_logic;
      dqs1_delayed_col0      : in std_logic;
      user_output_data  : out std_logic_vector(((2*data_width)-1) downto 0);
      fifo0_rd_addr_val : out std_logic_vector(3 downto 0);
      fifo1_rd_addr_val : out std_logic_vector(3 downto 0)
      );
  end component;

  component vhdl_syn_bl4_data_read_controller_0
    port(
      clk90                 : in  std_logic;
      reset_r               : in  std_logic;
      reset90_r             : in  std_logic;
      rst_dqs_div_in        : in  std_logic;
      delay_sel             : in  std_logic_vector(4 downto 0);
     dqs_int_delay_in0      : in std_logic;
     dqs_int_delay_in1      : in std_logic;
      fifo0_rd_addr         : in  std_logic_vector(3 downto 0);
      fifo1_rd_addr         : in  std_logic_vector(3 downto 0);
      fifo_00_wr_en_val      : out std_logic;
      fifo_10_wr_en_val      : out std_logic;
      fifo_01_wr_en_val      : out std_logic;
      fifo_11_wr_en_val      : out std_logic;
      fifo_00_wr_addr_val      : out std_logic_vector(3 downto 0);
    fifo_01_wr_addr_val      : out std_logic_vector(3 downto 0);
      fifo_10_wr_addr_val      : out std_logic_vector(3 downto 0);
    fifo_11_wr_addr_val      : out std_logic_vector(3 downto 0);
      dqs0_delayed_col0_val      : out std_logic;
      dqs1_delayed_col0_val      : out std_logic;
      dqs0_delayed_col1_val      : out std_logic;
      dqs1_delayed_col1_val      : out std_logic;
      u_data_val            : out std_logic;
      read_valid_data_1_val : out std_logic
      );
  end component;

  component vhdl_syn_bl4_data_write_0
    port(
      user_input_data    : in  std_logic_vector(((2*data_width)-1) downto 0);
      user_data_mask     : in  std_logic_vector((2*data_mask_width-1) downto 0);
      clk90              : in  std_logic;
      write_enable       : in  std_logic;
      write_en_val       : out std_logic;
    data_mask_f       : out std_logic_vector((data_mask_width-1) downto 0);
    data_mask_r       : out std_logic_vector((data_mask_width-1) downto 0);
      write_data_falling : out std_logic_vector((data_width-1) downto 0);
      write_data_rising  : out std_logic_vector((data_width-1) downto 0)
      );
  end component;

  component vhdl_syn_bl4_data_path_rst
    port(
      clk        : in  std_logic;
      reset      : in  std_logic;
      reset_r    : out std_logic
			);
  end component;

  signal reset_r           : std_logic;
  signal fifo0_rd_addr     : std_logic_vector(3 downto 0);
  signal fifo1_rd_addr     : std_logic_vector(3 downto 0);
  signal read_valid_data_1 : std_logic;
  signal fifo_00_wr_addr     :  std_logic_vector(3 downto 0);
  signal fifo_01_wr_addr     :  std_logic_vector(3 downto 0); 
  signal fifo_10_wr_addr     :  std_logic_vector(3 downto 0);
  signal fifo_11_wr_addr     :  std_logic_vector(3 downto 0); 
  signal fifo_00_wr_en        :  std_logic; 
  signal fifo_10_wr_en        :  std_logic; 
  signal fifo_01_wr_en        :  std_logic; 
  signal fifo_11_wr_en        :  std_logic; 
  signal dqs0_delayed_col1   :  std_logic; 
  signal dqs1_delayed_col1   :  std_logic; 
  signal dqs0_delayed_col0   :  std_logic; 
  signal dqs1_delayed_col0   :  std_logic; 

begin

  data_read0 : vhdl_syn_bl4_data_read_0
    port map (
      clk90             => clk90,
      reset90_r         => reset90,
      ddr_dq_in         => dq,
      read_valid_data_1 => read_valid_data_1,
      fifo_00_wr_en  => fifo_00_wr_en, 
      fifo_10_wr_en  => fifo_10_wr_en, 
      fifo_01_wr_en  => fifo_01_wr_en, 
      fifo_11_wr_en  => fifo_11_wr_en, 
      fifo_00_wr_addr    => fifo_00_wr_addr,
      fifo_01_wr_addr    => fifo_01_wr_addr,
      fifo_10_wr_addr    => fifo_10_wr_addr,
      fifo_11_wr_addr    => fifo_11_wr_addr,
      dqs0_delayed_col0  => dqs0_delayed_col0,
      dqs1_delayed_col0  => dqs1_delayed_col0,
      dqs0_delayed_col1  => dqs0_delayed_col1,
      dqs1_delayed_col1  => dqs1_delayed_col1,
      user_output_data  => user_output_data,
      fifo0_rd_addr_val => fifo0_rd_addr,
      fifo1_rd_addr_val => fifo1_rd_addr
      );

  data_read_controller0 : vhdl_syn_bl4_data_read_controller_0
    port map (
      clk90                 => clk90,
      reset_r               => reset_r,
      reset90_r             => reset90,
      rst_dqs_div_in        => rst_dqs_div_in,
      delay_sel             => delay_sel,
      dqs_int_delay_in0  => dqs_int_delay_in0,
      dqs_int_delay_in1  => dqs_int_delay_in1,
      fifo0_rd_addr         => fifo0_rd_addr,
      fifo1_rd_addr         => fifo1_rd_addr,
      u_data_val            => u_data_val,
      fifo_00_wr_en_val  => fifo_00_wr_en,
      fifo_10_wr_en_val  => fifo_10_wr_en,
      fifo_01_wr_en_val  => fifo_01_wr_en,
      fifo_11_wr_en_val  => fifo_11_wr_en,
      fifo_00_wr_addr_val  => fifo_00_wr_addr,
      fifo_01_wr_addr_val  => fifo_01_wr_addr,
      fifo_10_wr_addr_val  => fifo_10_wr_addr,
      fifo_11_wr_addr_val  => fifo_11_wr_addr,
      dqs0_delayed_col0_val  => dqs0_delayed_col0,
      dqs1_delayed_col0_val  => dqs1_delayed_col0,
      dqs0_delayed_col1_val  => dqs0_delayed_col1,
      dqs1_delayed_col1_val  => dqs1_delayed_col1,
      read_valid_data_1_val => read_valid_data_1
      );

  data_write0 : vhdl_syn_bl4_data_write_0
    port map (
      user_input_data    => user_input_data,
      user_data_mask     => user_data_mask,
      clk90              => clk90,
      write_enable       => write_enable,
      write_en_val       => write_en_val,
      write_data_falling => write_data_falling,
      write_data_rising  => write_data_rising,
      data_mask_f        => data_mask_f,
      data_mask_r        => data_mask_r
      );

  data_path_rst0 : vhdl_syn_bl4_data_path_rst
    port map (
      clk        => clk,
      reset      => reset,
      reset_r    => reset_r
      );


end arc;












