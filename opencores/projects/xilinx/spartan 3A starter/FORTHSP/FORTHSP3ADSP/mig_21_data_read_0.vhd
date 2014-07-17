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
--  /   /        Filename           : mig_21_data_read_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created       : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : ram8d modules are instantiated for Read data FIFOs. ram8d is
--               each 8 bits or 4 bits depending on number data bits per strobe.
--               Each strobe  will have two instances, one for rising edge data
--               and one for falling edge data.
-------------------------------------------------------------------------------
library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.mig_21_parameters_0.all;

entity mig_21_data_read_0 is
  port(
    clk90             : in  std_logic;
    reset90         : in  std_logic;
    ddr_dq_in         : in  std_logic_vector((DATA_WIDTH-1) downto 0);
    read_valid_data_1 : in  std_logic;
    fifo_0_wr_en      : in std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    fifo_1_wr_en      : in std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    fifo_0_wr_addr    : in std_logic_vector((4*DATA_STROBE_WIDTH)-1 downto 0);
    fifo_1_wr_addr    : in std_logic_vector((4*DATA_STROBE_WIDTH)-1 downto 0);
    dqs_delayed_col0  : in std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    dqs_delayed_col1  : in std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
    user_output_data  : out std_logic_vector((2*DATA_WIDTH-1) downto 0);
    fifo0_rd_addr_val : out std_logic_vector(3 downto 0);
    fifo1_rd_addr_val : out std_logic_vector(3 downto 0)
    );
end mig_21_data_read_0;

architecture arc of mig_21_data_read_0 is

  component mig_21_rd_gray_cntr
    port (
      clk90    : in  std_logic;
      reset90  : in  std_logic;
      cnt_en   : in  std_logic;
      rgc_gcnt : out std_logic_vector(3 downto 0)
      );
  end component;

  component mig_21_ram8d_0 is
    port (
      DOUT  : out std_logic_vector((DATABITSPERSTROBE -1) downto 0);
      WADDR : in  std_logic_vector(3 downto 0);
      DIN   : in  std_logic_vector((DATABITSPERSTROBE -1) downto 0);
      RADDR : in  std_logic_vector(3 downto 0);
      WCLK0 : in  std_logic;
      WCLK1 : in  std_logic;
      WE    : in  std_logic
      );
  end component;

  signal read_valid_data_1_r  : std_logic;
  signal read_valid_data_1_r1 : std_logic;
  signal fifo0_rd_addr        : std_logic_vector(3 downto 0);
  signal fifo1_rd_addr        : std_logic_vector(3 downto 0);

  signal first_sdr_data       : std_logic_vector((2*DATA_WIDTH-1) downto 0);
  signal reset90_r          : std_logic;
  signal fifo0_rd_addr_r      : std_logic_vector((4*DATA_STROBE_WIDTH-1) downto 0);
  signal fifo1_rd_addr_r      : std_logic_vector((4*DATA_STROBE_WIDTH-1) downto 0);
  signal fifo_0_data_out      : std_logic_vector((DATA_WIDTH-1) downto 0);
  signal fifo_1_data_out      : std_logic_vector((DATA_WIDTH-1) downto 0);
  signal fifo_0_data_out_r    : std_logic_vector((DATA_WIDTH-1) downto 0);
  signal fifo_1_data_out_r    : std_logic_vector((DATA_WIDTH-1) downto 0);
  signal dqs_delayed_col0_n   : std_logic_vector((DATA_STROBE_WIDTH -1) downto 0);
  signal dqs_delayed_col1_n   : std_logic_vector((DATA_STROBE_WIDTH -1) downto 0);

  attribute syn_noprune  : boolean;
  attribute syn_preserve : boolean;

  attribute syn_noprune of fifo0_rd_addr_r   : signal is true;
  attribute syn_noprune of fifo1_rd_addr_r   : signal is true;
  attribute syn_preserve of fifo0_rd_addr_r  : signal is true;
  attribute syn_preserve of fifo1_rd_addr_r  : signal is true;

begin

  process(clk90)
  begin
    if(clk90'event and clk90='1') then
      reset90_r <= reset90;
    end if;
  end process;

  gen_asgn : for asgn_i in 0 to DATA_STROBE_WIDTH-1 generate
    dqs_delayed_col0_n(asgn_i) <= not dqs_delayed_col0(asgn_i);
    dqs_delayed_col1_n(asgn_i) <= not dqs_delayed_col1(asgn_i);
  end generate;
  
  user_output_data  <= first_sdr_data;
  fifo0_rd_addr_val <= fifo1_rd_addr;
  fifo1_rd_addr_val <= fifo0_rd_addr;

  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      fifo_0_data_out_r <= fifo_0_data_out;
      fifo_1_data_out_r <= fifo_1_data_out;
    end if;
  end process;

  gen_addr : for addr_i in 0 to DATA_STROBE_WIDTH-1 generate
    process(clk90)
    begin
      if clk90'event and clk90 = '1' then
        fifo0_rd_addr_r((addr_i*4-1)+ 4 downto addr_i*4) <= fifo0_rd_addr;
        fifo1_rd_addr_r((addr_i*4-1)+ 4 downto addr_i*4) <= fifo1_rd_addr;
      end if;
    end process;
  end generate;

  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      if reset90_r = '1' then
        first_sdr_data       <= (others => '0');
        read_valid_data_1_r  <= '0';
        read_valid_data_1_r1 <= '0';
      else
        read_valid_data_1_r  <= read_valid_data_1;
        read_valid_data_1_r1 <= read_valid_data_1_r;
        if (read_valid_data_1_r1 = '1') then
          first_sdr_data <=
            (fifo_0_data_out_r & fifo_1_data_out_r);
        else
          first_sdr_data <= first_sdr_data;
        end if;
      end if;
    end if;
  end process;

-------------------------------------------------------------------------------
-- fifo_x0_rd_addr and fifo_x1_rd_addr counters ( gray counters )
-------------------------------------------------------------------------------

  fifo0_rd_addr_inst : mig_21_rd_gray_cntr
    port map (
      clk90    => clk90,
      reset90  => reset90,
      cnt_en   => read_valid_data_1,
      rgc_gcnt => fifo0_rd_addr
      );
  fifo1_rd_addr_inst : mig_21_rd_gray_cntr
    port map (
      clk90    => clk90,
      reset90  => reset90,
      cnt_en   => read_valid_data_1,
      rgc_gcnt => fifo1_rd_addr
      );

  gen_strobe: for strobe_i in 0 to DATA_STROBE_WIDTH-1 generate
    strobe : mig_21_ram8d_0
      Port Map (
        DOUT  => fifo_0_data_out((strobe_i*DATABITSPERSTROBE-1)+ DATABITSPERSTROBE downto strobe_i*DATABITSPERSTROBE),
        WADDR => fifo_0_wr_addr((strobe_i*4-1)+4 downto strobe_i*4),
        DIN   => ddr_dq_in((strobe_i*DATABITSPERSTROBE-1)+ DATABITSPERSTROBE downto strobe_i*DATABITSPERSTROBE),
        RADDR => fifo0_rd_addr_r((strobe_i*4-1)+4 downto strobe_i*4),
        WCLK0 => dqs_delayed_col0(strobe_i),
        WCLK1 => dqs_delayed_col1(strobe_i),
        WE    => fifo_0_wr_en(strobe_i)
        );
    strobe_n : mig_21_ram8d_0
      Port Map (
        DOUT  => fifo_1_data_out((strobe_i*DATABITSPERSTROBE-1)+ DATABITSPERSTROBE downto strobe_i*DATABITSPERSTROBE),
        WADDR => fifo_1_wr_addr((strobe_i*4-1)+4 downto strobe_i*4),
        DIN   => ddr_dq_in((strobe_i*DATABITSPERSTROBE-1)+ DATABITSPERSTROBE downto strobe_i*DATABITSPERSTROBE),
        RADDR => fifo1_rd_addr_r((strobe_i*4-1)+4 downto strobe_i*4),
        WCLK0 => dqs_delayed_col0_n(strobe_i),
        WCLK1 => dqs_delayed_col1_n(strobe_i),
        WE    => fifo_1_wr_en(strobe_i)
        );
  end generate;

end arc;
