-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_data_read_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/06 08:09:02 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : RAM8D modules are instantiated for Read data FIFOs. RAM8D is 
--               each 8 bits or 4 bits depending on number data bits per strobe. 
--               Each strobe  will have two instances, one for rising edge data
--				       and one for falling edge data. 
-------------------------------------------------------------------------------
library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_data_read_0 is
  port(
    clk90             : in  std_logic;
    reset90_r         : in  std_logic;
    ddr_dq_in         : in  std_logic_vector((data_width-1) downto 0);
    read_valid_data_1 : in  std_logic;
    fifo_00_wr_en    : in std_logic;
    fifo_01_wr_en    : in std_logic;
    fifo_10_wr_en    : in std_logic;
    fifo_11_wr_en    : in std_logic;
    fifo_00_wr_addr : in std_logic_vector(3 downto 0);
    fifo_01_wr_addr : in std_logic_vector(3 downto 0);
    fifo_10_wr_addr : in std_logic_vector(3 downto 0);
    fifo_11_wr_addr : in std_logic_vector(3 downto 0);
    dqs0_delayed_col0  : in std_logic;
    dqs0_delayed_col1  : in std_logic;
    dqs1_delayed_col0  : in std_logic;
    dqs1_delayed_col1  : in std_logic;
    user_output_data  : out std_logic_vector((2*data_width-1) downto 0);
    fifo0_rd_addr_val : out std_logic_vector(3 downto 0);
    fifo1_rd_addr_val : out std_logic_vector(3 downto 0)
    );
end vhdl_syn_bl4_data_read_0;

architecture arc of vhdl_syn_bl4_data_read_0 is
  attribute syn_noprune  : boolean;
  attribute syn_preserve : boolean;
  attribute KEEP_HIERARCHY : string;
  attribute KEEP_HIERARCHY of arc : architecture is "YES";

  component vhdl_syn_bl4_rd_gray_cntr 
    port (
      clk      : in  std_logic;
      reset    : in  std_logic;
      cnt_en   : in  std_logic;
      rgc_gcnt : out std_logic_vector(3 downto 0)
      );
  end component;

  component vhdl_syn_bl4_RAM8D_0 is 
    port (
      DOUT  : out std_logic_vector((DatabitsPerStrobe -1) downto 0);
      WADDR : in  std_logic_vector(3 downto 0);
      DIN   : in  std_logic_vector((DatabitsPerStrobe -1) downto 0);
      RADDR : in  std_logic_vector(3 downto 0);
      WCLK0 : in  std_logic;
      WCLK1 : in  std_logic;
      WE    : in  std_logic
      );
  end component;

  component vhdl_syn_bl4_RAM8D_1 is 
    port (
      DOUT  : out std_logic_vector((DatabitsPerStrobe -1) downto 0);
      WADDR : in  std_logic_vector(3 downto 0);
      DIN   : in  std_logic_vector((DatabitsPerStrobe -1) downto 0);
      RADDR : in  std_logic_vector(3 downto 0);
      WCLK0 : in  std_logic;
      WCLK1 : in  std_logic;
      WE    : in  std_logic
      );
  end component;


  signal read_valid_data_1_r  : std_logic;
  signal read_valid_data_1_r1 : std_logic;
  signal read_valid_data_1_r2 : std_logic;
  signal fifo00_rd_addr       : std_logic_vector(3 downto 0);
  signal fifo01_rd_addr       : std_logic_vector(3 downto 0);
  signal fifo00_rd_addr_r  : std_logic_vector(3 downto 0);
  signal fifo01_rd_addr_r  : std_logic_vector(3 downto 0);
  signal fifo10_rd_addr_r  : std_logic_vector(3 downto 0);
  signal fifo11_rd_addr_r  : std_logic_vector(3 downto 0);
  attribute syn_noprune of fifo00_rd_addr_r  : signal is true;
  attribute syn_noprune of fifo01_rd_addr_r  : signal is true;
  attribute syn_noprune of fifo10_rd_addr_r  : signal is true;
  attribute syn_noprune of fifo11_rd_addr_r  : signal is true;
  attribute syn_preserve of fifo00_rd_addr_r  : signal is true;
  attribute syn_preserve of fifo01_rd_addr_r  : signal is true;
  attribute syn_preserve of fifo10_rd_addr_r  : signal is true;
  attribute syn_preserve of fifo11_rd_addr_r  : signal is true;
  signal fifo_00_data_out  : std_logic_vector(DatabitsPerStrobe-1 downto 0);
  signal fifo_01_data_out  : std_logic_vector(DatabitsPerStrobe-1 downto 0);
  signal fifo_10_data_out  : std_logic_vector(DatabitsPerStrobe-1 downto 0);
  signal fifo_11_data_out  : std_logic_vector(DatabitsPerStrobe-1 downto 0);
  signal fifo_00_data_out_r  : std_logic_vector(DatabitsPerStrobe-1 downto 0);
  signal fifo_01_data_out_r  : std_logic_vector(DatabitsPerStrobe-1 downto 0);
  signal fifo_10_data_out_r  : std_logic_vector(DatabitsPerStrobe-1 downto 0);
  signal fifo_11_data_out_r  : std_logic_vector(DatabitsPerStrobe-1 downto 0);
  signal first_sdr_data : std_logic_vector((2*data_width-1) downto 0);
  signal dqs0_delayed_col0_n  : std_logic; 
  signal dqs1_delayed_col0_n  : std_logic; 
  signal dqs0_delayed_col1_n  : std_logic; 
  signal dqs1_delayed_col1_n  : std_logic; 
  signal reset90_reg : std_logic;

begin

  process(clk90)
  begin
    if(clk90'event and clk90='1') then
      reset90_reg <= reset90_r;
    end if;
  end process;

  dqs0_delayed_col0_n    <= not dqs0_delayed_col0 ;
  dqs0_delayed_col1_n    <= not dqs0_delayed_col1 ;
  dqs1_delayed_col0_n    <= not dqs1_delayed_col0 ;
  dqs1_delayed_col1_n    <= not dqs1_delayed_col1 ;
  user_output_data  <= first_sdr_data;
  fifo0_rd_addr_val <= fifo01_rd_addr;
  fifo1_rd_addr_val <= fifo00_rd_addr;

  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
  fifo_00_data_out_r  <= fifo_00_data_out;
  fifo_01_data_out_r  <= fifo_01_data_out; 
  fifo_10_data_out_r  <= fifo_10_data_out;
  fifo_11_data_out_r  <= fifo_11_data_out; 
    end if;
  end process;


  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
  fifo00_rd_addr_r  <= fifo00_rd_addr;
  fifo01_rd_addr_r  <= fifo01_rd_addr; 
  fifo10_rd_addr_r  <= fifo00_rd_addr;
  fifo11_rd_addr_r  <= fifo01_rd_addr; 
    end if;
  end process;


  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      if reset90_reg = '1' then
        first_sdr_data       <= (others => '0');
        read_valid_data_1_r  <= '0';
        read_valid_data_1_r1 <= '0';
        read_valid_data_1_r2 <= '0';
      else
        read_valid_data_1_r  <= read_valid_data_1;
        read_valid_data_1_r1 <= read_valid_data_1_r;
        read_valid_data_1_r2 <= read_valid_data_1_r1;
        if (read_valid_data_1_r1 = '1') then
          first_sdr_data <=
            (
         fifo_10_data_out_r  &          fifo_00_data_out_r  &          fifo_11_data_out_r  &          fifo_01_data_out_r 
              );
        else
          first_sdr_data <= first_sdr_data;
        end if;
      end if;
    end if;
  end process;

-------------------------------------------------------------------------------
-- fifo_x0_rd_addr and fifo_x1_rd_addr counters ( gray counters )
-------------------------------------------------------------------------------

  fifo0_rd_addr_inst : vhdl_syn_bl4_rd_gray_cntr 
    port map (
      clk      => clk90,
      reset    => reset90_r,
      cnt_en   => read_valid_data_1,
      rgc_gcnt => fifo00_rd_addr
      );
  fifo1_rd_addr_inst : vhdl_syn_bl4_rd_gray_cntr 
    port map (
      clk      => clk90,
      reset    => reset90_r,
      cnt_en   => read_valid_data_1,
      rgc_gcnt => fifo01_rd_addr      
      );

  strobe0 : vhdl_syn_bl4_RAM8D_0
    Port Map (
      DOUT  => fifo_00_data_out((DatabitsPerStrobe-1) 
  																	downto 0),
      WADDR => fifo_00_wr_addr(3 downto 0),
      DIN   => ddr_dq_in(7 downto 0),
      RADDR => fifo00_rd_addr_r(3 downto 0),
      WCLK0 => dqs0_delayed_col0,
      WCLK1 => dqs0_delayed_col1,
      WE    => fifo_00_wr_en
      );
  strobe0_n : vhdl_syn_bl4_RAM8D_0
    Port Map (
      DOUT  => fifo_01_data_out((DatabitsPerStrobe-1) 
  																	downto 0),
      WADDR => fifo_01_wr_addr(3 downto 0),
      DIN   => ddr_dq_in(7 downto 0),
      RADDR => fifo01_rd_addr_r(3 downto 0),
      WCLK0 => dqs0_delayed_col0_n,
      WCLK1 => dqs0_delayed_col1_n,
      WE    => fifo_01_wr_en
      );
  strobe1 : vhdl_syn_bl4_RAM8D_1
    Port Map (
      DOUT  => fifo_10_data_out((DatabitsPerStrobe-1) 
  																	downto 0),
      WADDR => fifo_10_wr_addr(3 downto 0),
      DIN   => ddr_dq_in(15 downto 8),
      RADDR => fifo10_rd_addr_r(3 downto 0),
      WCLK0 => dqs1_delayed_col0,
      WCLK1 => dqs1_delayed_col1,
      WE    => fifo_10_wr_en
      );
  strobe1_n : vhdl_syn_bl4_RAM8D_1
    Port Map (
      DOUT  => fifo_11_data_out((DatabitsPerStrobe-1) 
  																	downto 0),
      WADDR => fifo_11_wr_addr(3 downto 0),
      DIN   => ddr_dq_in(15 downto 8),
      RADDR => fifo11_rd_addr_r(3 downto 0),
      WCLK0 => dqs1_delayed_col0_n,
      WCLK1 => dqs1_delayed_col1_n,
      WE    => fifo_11_wr_en
      );

end arc;
