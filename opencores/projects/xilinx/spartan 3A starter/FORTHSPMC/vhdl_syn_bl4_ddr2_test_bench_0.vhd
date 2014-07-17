-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_ddr2_test_bench_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module generate the commands, address and data associated
--              with a write and a read command.
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_ddr2_test_bench_0 is
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
end vhdl_syn_bl4_ddr2_test_bench_0;


architecture arc of vhdl_syn_bl4_ddr2_test_bench_0 is

  component vhdl_syn_bl4_addr_gen_0
    port(
      clk          : in  std_logic;
      rst          : in  std_logic;
      addr_rst     : in  std_logic;
      addr_inc     : in  std_logic;
      r_w          : in  std_logic;
      addr_out     : out std_logic_vector(((row_address + col_ap_width + 
					 bank_address)- 1) downto 0);
      test_cnt_ena : in  std_logic;
      burst_done   : out std_logic;
      cnt_roll     : out std_logic
                                     );
  end component;

  component vhdl_syn_bl4_cmd_fsm_0
    port (
      clk          : in  std_logic;
      clk90        : in  std_logic;
      cmd_ack      : in  std_logic;
      cnt_roll     : in  std_logic;
      r_w          : out std_logic;
      refresh_done : in  std_logic;
      rst90        : in  std_logic;
      rst180       : in  std_logic;
      init_val     : in  std_logic;
      addr_inc     : out std_logic;
      addr_rst     : out std_logic;
      u_cmd        : out std_logic_vector(3 downto 0);
      lfsr_rst     : out std_logic);
  end component;

  component vhdl_syn_bl4_cmp_data_0
    port(
      clk              : in  std_logic;
      data_valid       : in  std_logic;
      lfsr_data        : in  std_logic_vector(((2*data_width)-1) downto 0);
      read_data        : in  std_logic_vector(((2*data_width)-1) downto 0);
      rst              : in  std_logic;
      led_error_output : out std_logic;
      data_valid_out   : out std_logic
      );
  end component;



  component vhdl_syn_bl4_lfsr32_0
    port (
      clk         : in  std_logic;
      rst         : in  std_logic;
      lfsr_rst    : in  std_logic;
      lfsr_ena    : in  std_logic;
      lfsr_data_m : out std_logic_vector(((2*data_mask_width)-1) downto 0);
      lfsr_out    : out std_logic_vector(((2*data_width)-1) downto 0)
      );
  end component;


  signal clk           : std_logic;
  signal rst90_r       : std_logic;
  signal rst180_r      : std_logic;
  signal addr_inc      : std_logic;
  signal addr_rst      : std_logic;
  signal cmd_ack       : std_logic;
  signal cnt_roll      : std_logic;
  signal ctrl_ready    : std_logic;
  signal data_valid    : std_logic;
  signal lfsr_ena_r    : std_logic;
  signal lfsr_ena_w    : std_logic;
  signal lfsr_rst_r    : std_logic;
  signal lfsr_rst_r1   : std_logic;
  signal lfsr_rst_w    : std_logic;
  signal r_w           : std_logic;
  signal lfsr_data_r   : std_logic_vector(((2*data_width)-1) downto 0);
  signal lfsr_data_w   : std_logic_vector(((2*data_width)-1) downto 0);
  signal lfsr_data_m_r : std_logic_vector(((2*data_mask_width)-1) downto 0);
  signal lfsr_data_m_w : std_logic_vector(((2*data_mask_width)-1) downto 0);
  signal addr_out      : std_logic_vector(((row_address + col_ap_width +
																										bank_address)- 1) downto 0);
  signal u_dat_flag    : std_logic;
  signal u_dat_fl      : std_logic;

begin

-- Output : COMMAND REGISTER FORMAT
--          0000  - NOP
--          0001  - Precharge
--          0010  - Initialize memory
--          0100  - Write Request
--          0110  - Read request

-- Output : Address format
-- row address  = address((row_address + col_ap_width + bank_address) -1 downto 
--																								(col_ap_width + bank_address))
-- column addrs = address((row_address + bank_address)-1 downto bank_address)
-- bank addrs = address( bank_address-1 downto 0)


  clk        <= fpga_clk;
  cmd_ack    <= u_ack;
  data_valid <= u_data_val;
  ctrl_ready <= '1';
  u_addr(((row_address + col_ap_width + bank_address)- 1) downto 0) <= addr_out
																								(((row_address + col_ap_width +
                                                    bank_address)- 1) downto 0);
  u_data_i        <= lfsr_data_w;
  u_data_m        <= lfsr_data_m_w;




  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      rst90_r <= fpga_rst90;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      rst180_r <= fpga_rst180;
    end if;
  end process;

--*********************************************************
  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      if rst90_r = '1' then
        lfsr_ena_r <= '0';
      else
        if (u_data_val = '1') then
          lfsr_ena_r <= '1';
        else
          lfsr_ena_r <= '0';
        end if;

      end if;
    end if;
  end process;

  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      if rst90_r = '1' then
        lfsr_ena_w <= '0';
      else
        if ((r_w = '0') and (u_ack = '1')) then
          lfsr_ena_w <= '1';
        else
          lfsr_ena_w <= '0';
        end if;
      end if;
    end if;
  end process;

  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      if(rst90_r = '1') then
        u_dat_flag <= '0';
      else
        u_dat_flag <= cmd_ack;
      end if;
    end if;
  end process;

  u_dat_fl <= cmd_ack and (not u_dat_flag) and r_w;

  lfsr_rst_r1 <= u_dat_fl;


  process(clk90)
  begin
    if clk90'event and clk90 = '1' then
      if(rst90_r = '1') then
        lfsr_rst_r <= '0';
      else
        lfsr_rst_r <= lfsr_rst_r1;
      end if;
    end if;
  end process;




--*************************************************************************
  INST1 : vhdl_syn_bl4_addr_gen_0
    port map (
      clk          => clk,
      rst          => fpga_rst180,
      addr_rst     => addr_rst,
      addr_inc     => addr_inc,
      addr_out     => addr_out,
      test_cnt_ena => ctrl_ready,
      burst_done   => burst_done,
      r_w          => r_w,
      cnt_roll     => cnt_roll
      );

  INST_2 : vhdl_syn_bl4_cmd_fsm_0
    port map (
      clk          => clk,
      clk90        => clk90,
      cmd_ack      => cmd_ack,
      cnt_roll     => cnt_roll,
      r_w          => r_w,
      refresh_done => ar_done,
      rst90        => fpga_rst90,
      rst180       => fpga_rst180,
      init_val     => init_done,
      addr_inc     => addr_inc,
      addr_rst     => addr_rst,
      u_cmd        => u_cmd,
      lfsr_rst     => lfsr_rst_w
      );


  INST3 : vhdl_syn_bl4_cmp_data_0
    port map (
      clk              => clk90,
      data_valid       => data_valid,
      lfsr_data        => lfsr_data_r,
      read_data        => u_data_o,
      rst              => fpga_rst90,
      led_error_output => led_error_output,
      data_valid_out   => data_valid_out
      );

  INST5 : vhdl_syn_bl4_lfsr32_0
    port map (clk         => clk90,
      rst         => fpga_rst90,
      lfsr_rst    => lfsr_rst_r,
      lfsr_ena    => lfsr_ena_r,
      lfsr_data_m => lfsr_data_m_r,
      lfsr_out    => lfsr_data_r
      );

  INST7 : vhdl_syn_bl4_lfsr32_0
    port map (clk         => clk90,
      rst         => fpga_rst90,
      lfsr_rst    => lfsr_rst_w,
      lfsr_ena    => lfsr_ena_w,
      lfsr_data_m => lfsr_data_m_w,
      lfsr_out    => lfsr_data_w
      );

end arc;
