-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_infrastructure_top.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has instantiations clk_dcm and generate reset 
--							 signals.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.global.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_infrastructure_top is
  port(
    reset_in_n              : in  std_logic;
    SYS_CLK               : in  std_logic;
    delay_sel_val1_val    : out std_logic_vector(4 downto 0);
    sys_rst_val           : out std_logic;
    sys_rst90_val         : out std_logic;
    clk_int_val           : out std_logic;
    clk90_int_val         : out std_logic;
    sys_rst180_val        : out std_logic;
    wait_200us            : out std_logic
    );

end vhdl_syn_bl4_infrastructure_top;

architecture arc of vhdl_syn_bl4_infrastructure_top is

  component vhdl_syn_bl4_clk_dcm
    port(
      input_clk : in  std_logic;
      rst       : in  std_logic;
      clk       : out std_logic;
      clk90     : out std_logic;
      dcm_lock  : out std_logic
      );
  end component;

  component vhdl_syn_bl4_cal_top
    port (
      clk          : in  std_logic;
      clk0dcmlock  : in  std_logic;
      reset        : in  std_logic;
      tapForDqs    : out std_logic_vector(4 downto 0)
      );
  end component;

  signal user_rst          : std_logic := '1';
  signal user_cal_rst      : std_logic := '1';
  signal clk_int           : std_logic;
  signal clk90_int         : std_logic;
  signal dcm_lock          : std_logic := '1';
  signal sys_rst_o         : std_logic := '1';
  signal sys_rst_1         : std_logic := '1';
  signal sys_rst           : std_logic := '1';
  signal sys_rst90_o       : std_logic := '1';
  signal sys_rst90_1       : std_logic := '1';
  signal sys_rst90         : std_logic := '1';
  signal sys_rst180_o      : std_logic := '1';
  signal sys_rst180_1      : std_logic := '1';
  signal sys_rst180        : std_logic := '1';
  signal delay_sel_val1    : std_logic_vector(4 downto 0);
  signal clk_int_val1      : std_logic;
  signal clk_int_val2      : std_logic;
  signal clk90_int_val1    : std_logic;
  signal clk90_int_val2    : std_logic;
  signal wait_200us_i      : std_logic;
  signal  wait_200us_int             : std_logic;
  signal wait_clk90        : std_logic;
  signal  wait_clk270            : std_logic;
  signal Counter200        : std_logic_vector(15 downto 0);
  signal sys_clk_ibuf      : std_logic;

begin
  sys_clk_ibuf <= SYS_CLK;
  --lvds_clk_input : IBUFG port map(
  --  I  => SYS_CLK,
  --  O  => sys_clk_ibuf
  --  );
  clk_int_val           <= clk_int;
  clk90_int_val         <= clk90_int;
  sys_rst_val           <= sys_rst;
  sys_rst90_val         <= sys_rst90;
  sys_rst180_val        <= sys_rst180;
  delay_sel_val1_val    <= delay_sel_val1;


-- To remove delta delays in the clock signals observed during simulation 
-- ,Following signals are used

  clk_int_val1   <= clk_int;
  clk90_int_val1 <= clk90_int;
  clk_int_val2   <= clk_int_val1;
  clk90_int_val2 <= clk90_int_val1;
  user_rst       <= not reset_in_n when reset_active_low = '1'  else reset_in_n;
  user_cal_rst   <= reset_in_n     when reset_active_low = '1'  else not reset_in_n;


  process(clk_int_val2)
  begin
   if clk_int_val2'event and clk_int_val2 = '1' then
    if user_rst = '1' or dcm_lock = '0' then
       wait_200us_i     <= '1';
       Counter200       <= (others => '0');
    else
        if( Counter200 < integer(real(theClock) * 200.0e-6 + 0.5)) then --6001 -- 33200
           wait_200us_i <= '1';
           Counter200 <= Counter200 + 1;
        else    
           Counter200 <= Counter200;
           wait_200us_i <= '0';
       end if;
    end if;
   end if;
  end process;

  process(clk_int_val2)
  begin
    if clk_int_val2'event and clk_int_val2 = '1' then
      wait_200us <= wait_200us_i;
    end if;
  end process;
  
  process(clk_int_val2)
  begin
    if clk_int_val2'event and clk_int_val2 = '1' then
      wait_200us_int <= wait_200us_i;
    end if;
  end process;
  
  process(clk90_int_val2)
  begin
    if clk90_int_val2'event and clk90_int_val2 = '0' then
      if user_rst = '1' or dcm_lock = '0' then
        wait_clk270 <= '1';
      else
        wait_clk270 <= wait_200us_int;
      end if;
    end if;
  end process;
  
  process(clk90_int_val2)
  begin
    if clk90_int_val2'event and clk90_int_val2 = '1' then
      wait_clk90 <= wait_clk270;
    end if;
  end process;

  process(clk_int_val2)
  begin
    if clk_int_val2'event and clk_int_val2 = '1' then
      if user_rst = '1' or dcm_lock = '0' or wait_200us_int = '1' then
        sys_rst_o <= '1';
        sys_rst_1 <= '1';
        sys_rst   <= '1';
      else
        sys_rst_o <= '0';
        sys_rst_1 <= sys_rst_o;
        sys_rst   <= sys_rst_1;
      end if;
    end if;
  end process;

  process(clk90_int_val2)
  begin
    if clk90_int_val2'event and clk90_int_val2 = '1' then
      if user_rst = '1' or dcm_lock = '0' or wait_clk90 = '1' then
        sys_rst90_o <= '1';
        sys_rst90_1 <= '1';
        sys_rst90   <= '1';
      else
        sys_rst90_o <= '0';
        sys_rst90_1 <= sys_rst90_o;
        sys_rst90   <= sys_rst90_1;
      end if;
    end if;
  end process;

  process(clk_int_val2)
  begin
    if clk_int_val2'event and clk_int_val2 = '0' then
      if user_rst = '1' or dcm_lock = '0' or wait_clk270 = '1' then
        sys_rst180_o <= '1';
        sys_rst180_1 <= '1';
        sys_rst180   <= '1';
      else
        sys_rst180_o <= '0';
        sys_rst180_1 <= sys_rst180_o;
        sys_rst180   <= sys_rst180_1;
      end if;
    end if;
  end process;

  clk_dcm0 : vhdl_syn_bl4_clk_dcm port map (
    input_clk => sys_clk_ibuf,
    rst       => user_rst,
    clk       => clk_int,
    clk90     => clk90_int,
    dcm_lock  => dcm_lock
    );

  cal_top0 : vhdl_syn_bl4_cal_top port map (
    clk          => clk_int_val2,
    clk0dcmlock  => dcm_lock,
    reset        => user_cal_rst,
    tapForDqs    => delay_sel_val1
    );


end arc;



