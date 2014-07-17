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
--  /   /        Filename           : mig_21_data_read_controller_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Description : This module has instantiations fifo_0_wr_en, fifo_1_wr_en,
--               dqs_delay and wr_gray_cntr.
-------------------------------------------------------------------------------
library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.mig_21_parameters_0.all;

entity mig_21_data_read_controller_0 is
  port(
    clk                   : in  std_logic;
    clk90                 : in  std_logic;
    reset                 : in  std_logic;
    reset90               : in  std_logic;
    rst_dqs_div_in        : in  std_logic;
    delay_sel             : in  std_logic_vector(4 downto 0);
    dqs_int_delay_in      : in std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    fifo_0_wr_en_val      : out std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    fifo_1_wr_en_val      : out std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    fifo_0_wr_addr_val    : out std_logic_vector((4*DATA_STROBE_WIDTH)-1 downto 0);
    fifo_1_wr_addr_val    : out std_logic_vector((4*DATA_STROBE_WIDTH)-1 downto 0);
    dqs_delayed_col0_val  : out std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    dqs_delayed_col1_val  : out std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    fifo0_rd_addr         : in  std_logic_vector(3 downto 0);
    fifo1_rd_addr         : in  std_logic_vector(3 downto 0);
    u_data_val            : out std_logic;
    read_valid_data_1_val : out std_logic
    );

end mig_21_data_read_controller_0;

architecture arc of mig_21_data_read_controller_0 is

  component mig_21_dqs_delay
    port (
      clk_in  : in  std_logic;
      sel_in  : in  std_logic_vector(4 downto 0);
      clk_out : out std_logic
      );
  end component;

-- wr_gray_cntr is a gray counter with an ASYNC reset for fifo wr_addr
  
  component mig_21_wr_gray_cntr
    port (
      clk      : in  std_logic;
      reset    : in  std_logic;
      cnt_en   : in  std_logic;
      wgc_gcnt : out std_logic_vector(3 downto 0)
      );
  end component;

-- fifo_wr_en module generates fifo write enable signal
-- enable is derived from rst_dqs_div signal
  
  component mig_21_fifo_0_wr_en_0
    port (
      clk             : in  std_logic;
      reset           : in  std_logic;
      din             : in  std_logic;
      rst_dqs_delay_n : out std_logic;
      dout            : out std_logic
      );
  end component;
  
  component mig_21_fifo_1_wr_en_0
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
  signal fifo_0_wr_addr      : std_logic_vector((4*DATA_STROBE_WIDTH)-1 downto 0);
  signal fifo_1_wr_addr      : std_logic_vector((4*DATA_STROBE_WIDTH)-1 downto 0);
  signal fifo_0_empty        : std_logic;
  signal fifo_1_empty        : std_logic;
  signal read_valid_data_0_1 : std_logic;
  signal read_valid_data_r   : std_logic;
  signal read_valid_data_r1  : std_logic;

-- dqsx_delayed_col0 negated signals
-- used for capturing negedge data into FIFO_*1
-- FIFO WRITE ENABLE SIGNALS
  signal fifo_0_wr_en        : std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
  signal fifo_1_wr_en        : std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
  
-- FIFO_WR_POINTER Delayed signals in clk90 domain
  signal fifo_0_wr_addr_d    : std_logic_vector(3 downto 0);
  signal fifo_0_wr_addr_2d   : std_logic_vector(3 downto 0);
  signal fifo_0_wr_addr_3d   : std_logic_vector(3 downto 0);
  signal fifo_1_wr_addr_d    : std_logic_vector(3 downto 0);
  signal fifo_1_wr_addr_2d   : std_logic_vector(3 downto 0);
  signal fifo_1_wr_addr_3d   : std_logic_vector(3 downto 0);

  signal rst_dqs_div         : std_logic;
  signal reset90_r           : std_logic;
  signal reset_r             : std_logic;
  signal rst_dqs_delay_0_n   : std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
  signal dqs_delayed_col0_n  : std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
  signal dqs_delayed_col1_n  : std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);

  attribute syn_preserve  : boolean;
  attribute buffer_type   : string;
  attribute buffer_type  of  dqs_delayed_col0: signal is "none";
  attribute buffer_type  of  dqs_delayed_col1: signal is "none";

begin

  process(clk)
  begin
    if(clk'event and clk = '1') then
      reset_r <= reset;
    end if;
  end process;

  process(clk90)
  begin
    if(clk90'event and clk90 = '1') then
      reset90_r <= reset90;
    end if;
  end process;

  fifo_0_wr_addr_val   <= fifo_0_wr_addr;
  fifo_1_wr_addr_val   <= fifo_1_wr_addr;
  fifo_0_wr_en_val     <= fifo_0_wr_en;
  fifo_1_wr_en_val     <= fifo_1_wr_en;
  dqs_delayed_col0_val <= dqs_delayed_col0 ;
  dqs_delayed_col1_val <= dqs_delayed_col1 ;

  gen_asgn : for asgn_i in 0 to DATA_STROBE_WIDTH-1 generate
    dqs_delayed_col0_n(asgn_i) <= not dqs_delayed_col0(asgn_i);
    dqs_delayed_col1_n(asgn_i) <= not dqs_delayed_col1(asgn_i);
  end generate;
  
-- data_valid signal is derived from fifo_0 and fifo_1 empty signals only
-- FIFO WRITE POINTER DELAYED SIGNALS
-- To avoid meta-stability due to the domain crossing from ddr_dqs to clk90

  process (clk90)
  begin
    if (rising_edge(clk90)) then
      if reset90_r = '1' then
        fifo_0_wr_addr_d <= "0000";
        fifo_1_wr_addr_d <= "0000";
      else
        fifo_0_wr_addr_d <= fifo_0_wr_addr(3 downto 0);
        fifo_1_wr_addr_d <= fifo_1_wr_addr(3 downto 0);
      end if;
    end if;
  end process;

-- FIFO WRITE POINTER DOUBLE DELAYED SIGNALS

  process (clk90)
  begin
    if (rising_edge(clk90)) then
      if reset90_r = '1' then
        fifo_0_wr_addr_2d <= "0000";
        fifo_1_wr_addr_2d <= "0000";
      else
        fifo_0_wr_addr_2d <= fifo_0_wr_addr_d;
        fifo_1_wr_addr_2d <= fifo_1_wr_addr_d;
      end if;
    end if;
  end process;

  process (clk90)
  begin
    if (rising_edge(clk90)) then
      if reset90_r = '1' then
        fifo_0_wr_addr_3d <= "0000";
        fifo_1_wr_addr_3d <= "0000";
      else
        fifo_0_wr_addr_3d <= fifo_0_wr_addr_2d;
        fifo_1_wr_addr_3d <= fifo_1_wr_addr_2d;
      end if;
    end if;
  end process;
  
-- user data valid output signal from data path.

  fifo_0_empty          <= '1' when (fifo0_rd_addr(3 downto 0) =
                                     fifo_0_wr_addr_3d(3 downto 0)) else '0';
  fifo_1_empty          <= '1' when (fifo1_rd_addr(3 downto 0) =
                                     fifo_1_wr_addr_3d(3 downto 0)) else '0';
  read_valid_data_0_1   <= ((not fifo_0_empty) and (not fifo_1_empty));
  read_valid_data_1_val <= (read_valid_data_0_1);

  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      if reset90_r = '1' then
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

-- dqsx_delayed_col0 negated signals

  rst_dqs_div_delayed : mig_21_dqs_delay
    port map (
      clk_in  => rst_dqs_div_in,
      sel_in  => delay_sel,
      clk_out => rst_dqs_div
      );

--******************************************************************************
-- DQS Internal Delay Circuit implemented in LUTs
--******************************************************************************
  
  gen_delay: for dly_i in 0 to DATA_STROBE_WIDTH-1 generate
    attribute syn_preserve of  dqs_delay_col0: label is true;
    attribute syn_preserve of  dqs_delay_col1: label is true;
  begin
 -- Internal Clock Delay circuit placed in the first
   -- column (for falling edge data) adjacent to IOBs
    dqs_delay_col0 : mig_21_dqs_delay
      port map (
        clk_in  => dqs_int_delay_in(dly_i),
        sel_in  => delay_sel,
        clk_out => dqs_delayed_col0(dly_i)
       );
  -- Internal Clock Delay circuit placed in the second
  --column (for rising edge data) adjacent to IOBs
    dqs_delay_col1 : mig_21_dqs_delay
      port map (
        clk_in  => dqs_int_delay_in(dly_i),
        sel_in  => delay_sel,
        clk_out => dqs_delayed_col1(dly_i)
        );
  end generate;

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

  gen_wr_en: for wr_en_i in 0 to DATA_STROBE_WIDTH-1 generate
    fifo_0_wr_en_inst: mig_21_fifo_0_wr_en_0
      port map (
        clk             => dqs_delayed_col1_n (wr_en_i),
        reset           => reset_r,
        din             => rst_dqs_div,
        rst_dqs_delay_n => rst_dqs_delay_0_n(wr_en_i),
        dout            => fifo_0_wr_en(wr_en_i)
        );
    fifo_1_wr_en_inst: mig_21_fifo_1_wr_en_0
      port map (
        clk             => dqs_delayed_col0(wr_en_i),
        rst_dqs_delay_n => rst_dqs_delay_0_n(wr_en_i),
        reset           => reset_r,
        din             => rst_dqs_div,
        dout            => fifo_1_wr_en(wr_en_i)
        );
  end generate;

-------------------------------------------------------------------------------
-- write pointer gray counter instances
-------------------------------------------------------------------------------

  gen_wr_addr: for wr_addr_i in 0 to DATA_STROBE_WIDTH-1 generate
    fifo_0_wr_addr_inst : mig_21_wr_gray_cntr
      port map (
        clk      => dqs_delayed_col1(wr_addr_i),
        reset    => reset_r,
        cnt_en   => fifo_0_wr_en(wr_addr_i),
        wgc_gcnt => fifo_0_wr_addr((wr_addr_i*4-1)+4 downto wr_addr_i*4)
        );
    fifo_1_wr_addr_inst : mig_21_wr_gray_cntr
      port map (
        clk      => dqs_delayed_col0_n(wr_addr_i),
        reset    => reset_r,
        cnt_en   => fifo_1_wr_en(wr_addr_i),
        wgc_gcnt => fifo_1_wr_addr((wr_addr_i*4-1)+4 downto wr_addr_i*4)
        );        
  end generate;

end arc;
