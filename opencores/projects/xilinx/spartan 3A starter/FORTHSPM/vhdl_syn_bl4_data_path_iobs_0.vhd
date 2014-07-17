-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_parameters_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/19 12:03:12 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the instantiations s3_ddr_iob, s3_dqs_iob and ddr_dm modules.
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

library UNISIM;
use UNISIM.VCOMPONENTS.all;

use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_data_path_iobs_0 is
  port(
    clk                : in    std_logic;
    clk90              : in    std_logic;
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
    dqs_int_delay_in0  : out std_logic ;
    dqs_int_delay_in1  : out std_logic ;
    ddr_dm                : out std_logic_vector((data_mask_width-1) downto 0);
    ddr_dq_val         : out   std_logic_vector((data_width-1) downto 0)
    );
end vhdl_syn_bl4_data_path_iobs_0;


architecture arc of vhdl_syn_bl4_data_path_iobs_0 is


  component  vhdl_syn_bl4_s3_dqs_iob port(
     clk            : in std_logic;
     ddr_dqs_reset  : in std_logic;
     ddr_dqs_enable : in std_logic;
     ddr_dqs        : inout std_logic;
     ddr_dqs_n        : inout std_logic;
     dqs            : out std_logic
     );
end component;
 
  component vhdl_syn_bl4_s3_ddr_iob
    port (
      ddr_dq_inout       : inout std_logic;  --Bi-directional SDRAM data bus
      write_data_falling : in    std_logic;  --Transmit data, output on falling edge
      write_data_rising  : in    std_logic;  --Transmit data, output on rising edge
      read_data_in       : out   std_logic;  -- Received data
      clk90              : in    std_logic;  
      write_en_val       : in    std_logic
      ); 
  end component;

  component   vhdl_syn_bl4_ddr2_dm_0
port (
      ddr_dm       : out std_logic_vector((data_mask_width-1) downto 0);
      mask_falling : in std_logic_vector((data_mask_width-1) downto 0);
      mask_rising  : in std_logic_vector((data_mask_width-1) downto 0);
      clk90        : in std_logic      
	  );
end component;

  signal ddr_dq_in  : std_logic_vector((data_width-1) downto 0);
  signal reset90_r  : std_logic;

begin


  ddr_dq_val <= ddr_dq_in;


  ddr2_dm0 : vhdl_syn_bl4_ddr2_dm_0 port map   (
                             ddr_dm       => ddr_dm,
                             mask_falling => data_mask_f,
                             mask_rising  => data_mask_r,
                             clk90        => clk90
                             );

--***********************************************************************
--    Read Data Capture Module Instantiations
--***********************************************************************
-- DQS IOB instantiations
--***********************************************************************


  s3_dqs_iob0 : vhdl_syn_bl4_s3_dqs_iob port map (
                                clk             => clk,
                                ddr_dqs_reset   => dqs_reset,
                                ddr_dqs_enable  => dqs_enable,
                                ddr_dqs         => ddr_dqs(0),
                                ddr_dqs_n       => ddr_dqs_n(0),
                                dqs             => dqs_int_delay_in0
                                );
  s3_dqs_iob1 : vhdl_syn_bl4_s3_dqs_iob port map (
                                clk             => clk,
                                ddr_dqs_reset   => dqs_reset,
                                ddr_dqs_enable  => dqs_enable,
                                ddr_dqs         => ddr_dqs(1),
                                ddr_dqs_n       => ddr_dqs_n(1),
                                dqs             => dqs_int_delay_in1
                                );



--******************************************************************************
-- DDR Data bit instantiations 
--******************************************************************************           

  s3_ddr_iob0 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(0),
     write_data_falling => write_data_falling(0),
     write_data_rising  => write_data_rising(0),
     read_data_in       => ddr_dq_in(0),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob1 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(1),
     write_data_falling => write_data_falling(1),
     write_data_rising  => write_data_rising(1),
     read_data_in       => ddr_dq_in(1),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob2 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(2),
     write_data_falling => write_data_falling(2),
     write_data_rising  => write_data_rising(2),
     read_data_in       => ddr_dq_in(2),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob3 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(3),
     write_data_falling => write_data_falling(3),
     write_data_rising  => write_data_rising(3),
     read_data_in       => ddr_dq_in(3),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob4 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(4),
     write_data_falling => write_data_falling(4),
     write_data_rising  => write_data_rising(4),
     read_data_in       => ddr_dq_in(4),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob5 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(5),
     write_data_falling => write_data_falling(5),
     write_data_rising  => write_data_rising(5),
     read_data_in       => ddr_dq_in(5),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob6 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(6),
     write_data_falling => write_data_falling(6),
     write_data_rising  => write_data_rising(6),
     read_data_in       => ddr_dq_in(6),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob7 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(7),
     write_data_falling => write_data_falling(7),
     write_data_rising  => write_data_rising(7),
     read_data_in       => ddr_dq_in(7),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob8 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(8),
     write_data_falling => write_data_falling(8),
     write_data_rising  => write_data_rising(8),
     read_data_in       => ddr_dq_in(8),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob9 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(9),
     write_data_falling => write_data_falling(9),
     write_data_rising  => write_data_rising(9),
     read_data_in       => ddr_dq_in(9),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob10 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(10),
     write_data_falling => write_data_falling(10),
     write_data_rising  => write_data_rising(10),
     read_data_in       => ddr_dq_in(10),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob11 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(11),
     write_data_falling => write_data_falling(11),
     write_data_rising  => write_data_rising(11),
     read_data_in       => ddr_dq_in(11),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob12 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(12),
     write_data_falling => write_data_falling(12),
     write_data_rising  => write_data_rising(12),
     read_data_in       => ddr_dq_in(12),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob13 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(13),
     write_data_falling => write_data_falling(13),
     write_data_rising  => write_data_rising(13),
     read_data_in       => ddr_dq_in(13),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob14 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(14),
     write_data_falling => write_data_falling(14),
     write_data_rising  => write_data_rising(14),
     read_data_in       => ddr_dq_in(14),
     clk90              => clk90,
     write_en_val       => write_en_val
     );
  s3_ddr_iob15 : vhdl_syn_bl4_s3_ddr_iob 
   port map (
     ddr_dq_inout       => ddr_dq(15),
     write_data_falling => write_data_falling(15),
     write_data_rising  => write_data_rising(15),
     read_data_in       => ddr_dq_in(15),
     clk90              => clk90,
     write_en_val       => write_en_val
     );


end arc;

