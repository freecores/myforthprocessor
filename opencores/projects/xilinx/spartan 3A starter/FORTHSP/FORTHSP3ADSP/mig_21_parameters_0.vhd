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
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : 2.1
--  \   \        Application	    : MIG
--  /   /        Filename	    : mig_21_parameters_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2008/01/09 14:38:22 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A/3A-DSP
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the parameters used in the design
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use UNISIM.VCOMPONENTS.all;

package  mig_21_parameters_0  is

-- The reset polarity is set to active low by default. 
-- You can change this by editing the parameter RESET_ACTIVE_LOW.
-- Please do not change any of the other parameters directly by editing the RTL. 
-- All other changes should be done through the GUI.

constant   DATA_WIDTH                                : INTEGER   :=  32;
constant   DATA_STROBE_WIDTH                         : INTEGER   :=  4;
constant   DATA_MASK_WIDTH                           : INTEGER   :=  4;
constant   CLK_WIDTH                                 : INTEGER   :=  2;
constant   CKE_WIDTH                                 : INTEGER   :=  1;
constant   DEEP_MEMORY                               : INTEGER   :=  1;
constant   ROW_ADDRESS                               : INTEGER   :=  13;
constant   MEMORY_WIDTH                              : INTEGER   :=  8;
constant   REGISTERED                                : INTEGER   :=  0;
constant   DATABITSPERSTROBE                         : INTEGER   :=  8;
constant   DATABITSPERMASK                           : INTEGER   :=  8;
constant   NO_OF_CS                                  : INTEGER   :=  1;
constant   RESET_PORT                                : INTEGER   :=  0;
constant   MASK_ENABLE                               : INTEGER   :=  1;
constant   COLUMN_ADDRESS                            : INTEGER   :=  10;
constant   BANK_ADDRESS                              : INTEGER   :=  2;
constant   LOAD_MODE_REGISTER                        : std_logic_vector(12 downto 0) := "0010100110010";

constant   EXT_LOAD_MODE_REGISTER                    : std_logic_vector(12 downto 0) := "0000000000000";

constant   RESET_ACTIVE_LOW                         : std_logic := '1';
constant   RFC_COUNT_VALUE                            : std_logic_vector(7 downto 0) := "00001101";
constant   TWR_COUNT_VALUE                            : std_logic_vector(2 downto 0) := "010";
constant   MAX_REF_WIDTH                                   : INTEGER   :=  11;
constant   MAX_REF_CNT                     : std_logic_vector(10 downto 0) := "10000000001";

end mig_21_parameters_0  ;
