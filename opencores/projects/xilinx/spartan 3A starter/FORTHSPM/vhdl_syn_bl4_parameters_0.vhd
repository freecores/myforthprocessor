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
-- /___/   /\    Date Last Modified : $Date: 2006/12/27 06:38:55 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the parameters used in the design
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use UNISIM.VCOMPONENTS.all;
use work.global.all;

package  vhdl_syn_bl4_parameters_0  is

constant   data_width                                : INTEGER   :=  16;
constant   data_strobe_width                         : INTEGER   :=  2;
constant   data_mask_width                           : INTEGER   :=  2;
constant   clk_width                                 : INTEGER   :=  1;
constant   ReadEnable                                : INTEGER   :=  1;
constant   cke_width                                 : INTEGER   :=  1;
constant   deep_memory                               : INTEGER   :=  2;
constant   memory_width                              : INTEGER   :=  16;
constant   registered                                : INTEGER   :=  0;
constant   col_ap_width                              : INTEGER   :=  11;
constant   DatabitsPerStrobe                         : INTEGER   :=  8;
constant   DatabitsPerMask                           : INTEGER   :=  8;
constant   no_of_CS                                  : INTEGER   :=  1;
constant   RESET                                     : INTEGER   :=  0;
constant   data_mask                                 : INTEGER   :=  1;
constant   write_pipe_itr                            : INTEGER   :=  1;
constant   paramWidth                                : INTEGER   :=  1;
constant   ecc_enable                                : INTEGER   :=  0;
constant   ecc_width                                 : INTEGER   :=  0;
constant   dq_width                                  : INTEGER   :=  16;
constant   dm_width                                  : INTEGER   :=  2;
constant   dqs_width                                 : INTEGER   :=  2;
constant   write_pipeline                            : INTEGER   :=  4;
constant   top_bottom                                : INTEGER   :=   0;
constant   left_right                                : INTEGER   :=   1;
constant   row_address                               : INTEGER   :=  13;
constant   column_address                            : INTEGER   :=  10;
constant   bank_address                              : INTEGER   :=  2;
constant   spartan3a                                 : INTEGER   :=  1;
constant   burst_length                             		: std_logic_vector(2 downto 0) := "010";
constant   burst_type                               		: std_logic :=  '0';
constant   cas_latency_value                        		: std_logic_vector(2 downto 0) := "011";
constant   mode                                     		: std_logic :=  '0';
constant   dll_rst                                  		: std_logic :=  '1';
constant   write_recovery                           		: std_logic_vector(2 downto 0) := "010";
constant   pd_mode                                  		: std_logic :=  '0';
constant   load_mode_register                        : std_logic_vector(12 downto 0) := "0010100110010";

constant   outputs                                  		: std_logic :=  '0';
constant   rdqs_ena                                 		: std_logic :=  '0';
constant   dqs_n_ena                                		: std_logic :=  '0';
constant   ocd_operation                            		: std_logic_vector(2 downto 0) := "000";
constant   odt_enable                               		: std_logic_vector(1 downto 0) := "00";
constant   additive_latency_value                   		: std_logic_vector(2 downto 0) := "000";
constant   op_drive_strength                        		: std_logic :=  '0';
constant   dll_ena                                  		: std_logic :=  '0';
constant   ext_load_mode_register                    : std_logic_vector(12 downto 0) := "0000000000000";

constant   chip_address                              : INTEGER   :=  1;
constant   reset_active_low                         : std_logic := '1';
constant   rcd_count_value                           : std_logic_vector(2 downto 0) := "001";
constant   ras_count_value                           : std_logic_vector(3 downto 0) := "0101";
constant   mrd_count_value                           : std_logic := '1';
constant   rp_count_value                             : std_logic_vector(2 downto 0) := "001";
constant   rfc_count_value                            : std_logic_vector(5 downto 0) := std_logic_vector(to_unsigned(integer(60.0e-9 * real(theClock) + 0.49),6));--"000010";
constant   trtp_count_value                           : std_logic_vector(2 downto 0) := "000";
constant   twr_count_value                            : std_logic_vector(2 downto 0) := "010";
constant   twtr_count_value                      : std_logic_vector(2 downto 0) := "001";
constant   max_ref_width                                   : INTEGER   :=  11;
constant   max_ref_cnt                     : std_logic_vector(10 downto 0) := std_logic_vector(to_unsigned(integer(real(theClock) * 7.6e-6),max_ref_width));--"00011101100";



end vhdl_syn_bl4_parameters_0  ;
