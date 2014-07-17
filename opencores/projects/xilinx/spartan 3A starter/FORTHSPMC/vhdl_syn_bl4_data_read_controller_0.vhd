-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_data_read_controller_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/08 16:03:08 $	
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Description : This module has instantiations fifo_0_wr_en, fifo_1_wr_en, 
--               dqs_delay and wr_gray_cntr.
-------------------------------------------------------------------------------
library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_data_read_controller_0 is
  port(
    clk90                 : in  std_logic;
    reset_r               : in  std_logic;
    reset90_r             : in  std_logic;
    rst_dqs_div_in        : in  std_logic;
    delay_sel             : in  std_logic_vector(4 downto 0);
    dqs_int_delay_in0     : in std_logic;
    dqs_int_delay_in1     : in std_logic;
    fifo_00_wr_en_val     : out std_logic;
    fifo_01_wr_en_val     : out std_logic;
    fifo_10_wr_en_val     : out std_logic;
    fifo_11_wr_en_val     : out std_logic;
    fifo_00_wr_addr_val   : out std_logic_vector(3 downto 0);
    fifo_01_wr_addr_val   : out std_logic_vector(3 downto 0);
    fifo_10_wr_addr_val   : out std_logic_vector(3 downto 0);
    fifo_11_wr_addr_val   : out std_logic_vector(3 downto 0);
    dqs0_delayed_col0_val : out std_logic;
    dqs0_delayed_col1_val : out std_logic;
    dqs1_delayed_col0_val : out std_logic;
    dqs1_delayed_col1_val : out std_logic;
    fifo0_rd_addr         : in  std_logic_vector(3 downto 0);
    fifo1_rd_addr         : in  std_logic_vector(3 downto 0);
    u_data_val            : out std_logic;
    read_valid_data_1_val : out std_logic
    );

end vhdl_syn_bl4_data_read_controller_0;

architecture arc of vhdl_syn_bl4_data_read_controller_0 is


  component vhdl_syn_bl4_dqs_delay
    port (
      clk_in  : in  std_logic;
      sel_in  : in  std_logic_vector(4 downto 0);
      clk_out : out std_logic
      );
  end component;

-- wr_gray_cntr is a gray counter with an ASYNC reset for fifo wr_addr
  component vhdl_syn_bl4_wr_gray_cntr
    port (
      clk      : in  std_logic;
      reset    : in  std_logic;
      cnt_en   : in  std_logic;
      wgc_gcnt : out std_logic_vector(3 downto 0)
      );
  end component;

-- fifo_wr_en module generates fifo write enable signal
-- enable is derived from rst_dqs_div signal

  component vhdl_syn_bl4_fifo_0_wr_en_0
    port (
      clk             : in  std_logic;
      reset           : in  std_logic;
      din             : in  std_logic;
      rst_dqs_delay_n : out std_logic;
      dout            : out std_logic
      );
  end component;


  component vhdl_syn_bl4_fifo_1_wr_en_0
    port (
      clk             : in  std_logic;
      rst_dqs_delay_n : in  std_logic;
      reset           : in  std_logic;
      din             : in  std_logic;
      dout            : out std_logic
      );
  end component;


  signal dqs_delayed_col0    : std_logic_vector((data_strobe_width-1) downto 0);
  signal dqs_delayed_col1    : std_logic_vector((data_strobe_width-1) downto 0);
  signal fifo_00_empty       : std_logic;
  signal fifo_01_empty       : std_logic;
  signal fifo_00_wr_addr     :  std_logic_vector(3 downto 0);
  signal fifo_01_wr_addr     :  std_logic_vector(3 downto 0); 
  signal fifo_10_wr_addr     :  std_logic_vector(3 downto 0);
  signal fifo_11_wr_addr     :  std_logic_vector(3 downto 0); 
  signal read_valid_data_0_1 : std_logic;
  signal read_valid_data_r   : std_logic;
  signal read_valid_data_r1  : std_logic;
  signal dqs0_delayed_col0   :  std_logic; 
  signal dqs1_delayed_col0   :  std_logic; 
  signal dqs0_delayed_col1   :  std_logic; 
  signal dqs1_delayed_col1   :  std_logic; 

-- dqsx_delayed_col0 negated signals
-- used for capturing negedge data into FIFO_*1
-- FIFO WRITE ENABLE SIGNALS
  signal fifo_00_wr_en        :  std_logic; 
  signal fifo_10_wr_en        :  std_logic; 
  signal fifo_01_wr_en        :  std_logic; 
  signal fifo_11_wr_en        :  std_logic; 

-- FIFO_WR_POINTER Delayed signals in clk90 domain
  signal fifo_00_wr_addr_d  : std_logic_vector(3 downto 0);
  signal fifo_00_wr_addr_2d : std_logic_vector(3 downto 0);
  signal fifo_00_wr_addr_3d : std_logic_vector(3 downto 0);
  signal fifo_01_wr_addr_d  : std_logic_vector(3 downto 0);
  signal fifo_01_wr_addr_2d : std_logic_vector(3 downto 0);
  signal fifo_01_wr_addr_3d : std_logic_vector(3 downto 0);

-- DDR_DQ_IN signals from DDR_DQ Input buffer
  signal rst_dqs_div : std_logic;
  signal reset90_reg : std_logic;

  signal rst_dqs_delay_0_n      : std_logic; 
  signal rst_dqs_delay_1_n      : std_logic; 
  signal dqs0_delayed_col0_n  : std_logic; 
  signal dqs1_delayed_col0_n  : std_logic; 
  signal dqs0_delayed_col1_n  : std_logic; 
  signal dqs1_delayed_col1_n  : std_logic; 


  attribute syn_noprune  : boolean;
  attribute syn_preserve : boolean;
  attribute syn_noprune of  dqs_delay0_col0: label is true;
  attribute syn_noprune of  dqs_delay0_col1: label is true;
  attribute syn_noprune of  dqs_delay1_col0: label is true;
  attribute syn_noprune of  dqs_delay1_col1: label is true;

begin

process(clk90)
begin
  if(clk90'event and clk90 = '1') then
    reset90_reg <= reset90_r;
  end if;
end process;

  fifo_00_wr_addr_val      <= fifo_00_wr_addr;
  fifo_01_wr_addr_val      <= fifo_01_wr_addr;
  fifo_10_wr_addr_val      <= fifo_10_wr_addr;
  fifo_11_wr_addr_val      <= fifo_11_wr_addr;
  fifo_00_wr_en_val    <= fifo_00_wr_en;
  fifo_01_wr_en_val    <= fifo_01_wr_en;
  fifo_10_wr_en_val    <= fifo_10_wr_en;
  fifo_11_wr_en_val    <= fifo_11_wr_en;
  dqs0_delayed_col0_val  <=  dqs0_delayed_col0 ;
  dqs0_delayed_col1_val  <=  dqs0_delayed_col1 ;
  dqs1_delayed_col0_val  <=  dqs1_delayed_col0 ;
  dqs1_delayed_col1_val  <=  dqs1_delayed_col1 ;
  dqs0_delayed_col0_n    <= not dqs0_delayed_col0 ;
  dqs0_delayed_col1_n    <= not dqs0_delayed_col1 ;
  dqs1_delayed_col0_n    <= not dqs1_delayed_col0 ;
  dqs1_delayed_col1_n    <= not dqs1_delayed_col1 ;

-- data_valid signal is derived from fifo_00 and fifo_01 empty signals only
-- FIFO WRITE POINTER DELAYED SIGNALS
-- To avoid meta-stability due to the domain crossing from ddr_dqs to clk90

  process (clk90)
  begin
    if (clk90'event and clk90 = '1') then
        fifo_00_wr_addr_d <= fifo_00_wr_addr;
        fifo_01_wr_addr_d <= fifo_01_wr_addr;
    end if;
  end process;

-- FIFO WRITE POINTER DOUBLE DELAYED SIGNALS

  process (clk90)
  begin
    if (clk90'event and clk90 = '1') then
        fifo_00_wr_addr_2d <= fifo_00_wr_addr_d;
        fifo_01_wr_addr_2d <= fifo_01_wr_addr_d;
    end if;
  end process;


  process (clk90)
  begin
    if (clk90'event and clk90 = '1') then
        fifo_00_wr_addr_3d <= fifo_00_wr_addr_2d;
        fifo_01_wr_addr_3d <= fifo_01_wr_addr_2d;
    end if;
  end process;
-- user data valid output signal from data path.

  fifo_00_empty         <= '1' when (fifo0_rd_addr(3 downto 0) =
				       fifo_00_wr_addr_3d(3 downto 0)) else '0';
  fifo_01_empty         <= '1' when (fifo1_rd_addr(3 downto 0) =
				       fifo_01_wr_addr_3d(3 downto 0)) else '0';
  read_valid_data_0_1   <= ((not fifo_00_empty) and (not fifo_01_empty));
  read_valid_data_1_val <= (read_valid_data_0_1);

  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      if reset90_reg = '1' then
        u_data_val         <= '0';
        read_valid_data_r  <= '0';
        read_valid_data_r1 <= '0';
      else
        read_valid_data_r  <= read_valid_data_0_1;
        read_valid_data_r1 <= read_valid_data_r;
        u_data_val         <= read_valid_data_r1;
      end if;
    end if;
  end process;

--  here vector component is assigned to a scalar signal
--  it was done because the delay_dqs signals were used

  dqs0_delayed_col0  <=  dqs_delayed_col0(0) ;
  dqs1_delayed_col0  <=  dqs_delayed_col0(1) ;
  dqs0_delayed_col1  <=  dqs_delayed_col1(0) ;
  dqs1_delayed_col1  <=  dqs_delayed_col1(1) ;

-- dqsx_delayed_col0 negated signals

  rst_dqs_div_delayed1 : vhdl_syn_bl4_dqs_delay port map (
    clk_in  => rst_dqs_div_in,
    sel_in  => delay_sel,
    clk_out => rst_dqs_div
    );

--******************************************************************************
-- DQS Internal Delay Circuit implemented in LUTs
--******************************************************************************

 -- Internal Clock Delay circuit placed in the first
   -- column (for falling edge data) adjacent to IOBs
  dqs_delay0_col0 : vhdl_syn_bl4_dqs_delay 
    port map (
       clk_in   => dqs_int_delay_in0,
       sel_in   => delay_sel,
       clk_out  => dqs_delayed_col0(0)
       );
  -- Internal Clock Delay circuit placed in the second 
  --column (for rising edge data) adjacent to IOBs
  dqs_delay0_col1 : vhdl_syn_bl4_dqs_delay 
    port map (
      clk_in   => dqs_int_delay_in0,
      sel_in   => delay_sel,
      clk_out  => dqs_delayed_col1(0)
      );
 -- Internal Clock Delay circuit placed in the first
   -- column (for falling edge data) adjacent to IOBs
  dqs_delay1_col0 : vhdl_syn_bl4_dqs_delay 
    port map (
       clk_in   => dqs_int_delay_in1,
       sel_in   => delay_sel,
       clk_out  => dqs_delayed_col0(1)
       );
  -- Internal Clock Delay circuit placed in the second 
  --column (for rising edge data) adjacent to IOBs
  dqs_delay1_col1 : vhdl_syn_bl4_dqs_delay 
    port map (
      clk_in   => dqs_int_delay_in1,
      sel_in   => delay_sel,
      clk_out  => dqs_delayed_col1(1)
      );

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------


  fifo_00_wr_en_inst: vhdl_syn_bl4_fifo_0_wr_en_0 
    port map (
      clk             => dqs0_delayed_col1_n,
      reset           => reset_r,
      din             => rst_dqs_div,
      rst_dqs_delay_n => rst_dqs_delay_0_n,
      dout            => fifo_00_wr_en
      );
  
  
  fifo_01_wr_en_inst: vhdl_syn_bl4_fifo_1_wr_en_0 
    port map (
      clk             => dqs0_delayed_col0,
      rst_dqs_delay_n => rst_dqs_delay_0_n,
      reset           => reset_r,
      din             => rst_dqs_div,
      dout            => fifo_01_wr_en
      );
  fifo_10_wr_en_inst: vhdl_syn_bl4_fifo_0_wr_en_0 
    port map (
      clk             => dqs1_delayed_col1_n,
      reset           => reset_r,
      din             => rst_dqs_div,
      rst_dqs_delay_n => rst_dqs_delay_1_n,
      dout            => fifo_10_wr_en
      );
  
  
  fifo_11_wr_en_inst: vhdl_syn_bl4_fifo_1_wr_en_0 
    port map (
      clk             => dqs1_delayed_col0,
      rst_dqs_delay_n => rst_dqs_delay_1_n,
      reset           => reset_r,
      din             => rst_dqs_div,
      dout            => fifo_11_wr_en
      );

-------------------------------------------------------------------------------
-- write pointer gray counter instances
-------------------------------------------------------------------------------


  fifo_00_wr_addr_inst : vhdl_syn_bl4_wr_gray_cntr
    port map (
      clk         =>   dqs0_delayed_col1,
      reset       =>   reset_r,
      cnt_en      =>   fifo_00_wr_en,
      wgc_gcnt    =>   fifo_00_wr_addr
      );
  
  fifo_01_wr_addr_inst : vhdl_syn_bl4_wr_gray_cntr
    port map (
      clk         =>   dqs0_delayed_col0_n,
      reset       =>   reset_r,
      cnt_en      =>   fifo_01_wr_en,
      wgc_gcnt    =>   fifo_01_wr_addr
      );
  fifo_10_wr_addr_inst : vhdl_syn_bl4_wr_gray_cntr
    port map (
      clk         =>   dqs1_delayed_col1,
      reset       =>   reset_r,
      cnt_en      =>   fifo_10_wr_en,
      wgc_gcnt    =>   fifo_10_wr_addr
      );
  
  fifo_11_wr_addr_inst : vhdl_syn_bl4_wr_gray_cntr
    port map (
      clk         =>   dqs1_delayed_col0_n,
      reset       =>   reset_r,
      cnt_en      =>   fifo_11_wr_en,
      wgc_gcnt    =>   fifo_11_wr_addr
      );

end arc;
