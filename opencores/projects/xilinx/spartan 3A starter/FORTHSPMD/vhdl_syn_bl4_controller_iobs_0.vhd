-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_controller_iobs_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:36 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the IOB instantiations to address and control 
--               signals.
-------------------------------------------------------------------------------
library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_controller_iobs_0 is
  port(
    clk0              : in  std_logic;
    ddr_rasb_cntrl    : in  std_logic;
    ddr_casb_cntrl    : in  std_logic;
    ddr_web_cntrl     : in  std_logic;
    ddr_cke_cntrl     : in  std_logic;
    ddr_csb_cntrl     : in  std_logic;
    ddr_ODT_cntrl     : in  std_logic;
    ddr_address_cntrl : in  std_logic_vector((row_address -1) downto 0);
    ddr_ba_cntrl      : in  std_logic_vector((bank_address -1) downto 0);
    rst_dqs_div_int   : in  std_logic;
    ddr_ODT           : out std_logic;
    ddr_rasb          : out std_logic;
    ddr_casb          : out std_logic;
    ddr_web           : out std_logic;
    ddr_ba            : out std_logic_vector((bank_address -1) downto 0);
    ddr_address       : out std_logic_vector((row_address -1) downto 0);
    ddr_cke           : out std_logic;
    ddr_csb           : out std_logic;
    rst_dqs_div       : out std_logic;
    rst_dqs_div_in    : in  std_logic;
    rst_dqs_div_out   : out std_logic
    );
end vhdl_syn_bl4_controller_iobs_0;

architecture arc of vhdl_syn_bl4_controller_iobs_0 is
  attribute KEEP_HIERARCHY : string;
  attribute KEEP_HIERARCHY of arc : architecture is "YES";

  signal GND             : std_logic;
  signal ddr_web_q       : std_logic;
  signal ddr_rasb_q      : std_logic;
  signal ddr_casb_q      : std_logic;
  signal ddr_cke_q       : std_logic;
  signal ddr_cke_int     : std_logic;
  signal ddr_address_reg : std_logic_vector((row_address -1) downto 0);
  signal ddr_ba_reg      : std_logic_vector((bank_address -1) downto 0);
  signal ddr_ODT_reg     : std_logic;
  signal clk180          : std_logic;

begin

  GND    <= '0';
  clk180 <= not clk0;

---- *******************************************  ----
----  Includes the instantiation of FD for cntrl  ----
----            signals                           ----
---- *******************************************  ----

  iob_web : FD
    port map (
      Q => ddr_web_q,
      D => ddr_web_cntrl,
      C => clk180
      );

  iob_rasb : FD
    port map (
      Q => ddr_rasb_q,
      D => ddr_rasb_cntrl,
      C => clk180
      );

  iob_casb : FD
    port map (
      Q => ddr_casb_q,
      D => ddr_casb_cntrl,
      C => clk180
      );

---- *************************************  ----
----  Output buffers for control signals    ----
---- *************************************  ----

  r16 : OBUF
    port map (
      I => ddr_web_q,
      O => ddr_web
      );

  r17 : OBUF
    port map (
      I => ddr_rasb_q,
      O => ddr_rasb
      );

  r18 : OBUF
    port map (
      I => ddr_casb_q,
      O => ddr_casb
      );

  r19 : OBUF
    port map (
      I => ddr_csb_cntrl,
      O => ddr_csb
      );

  iob_cke1 : FD
    port map(
      Q => ddr_cke_int,
      D => ddr_cke_cntrl,
      C => clk0
      );

  iob_cke : FD
    port map(
      Q => ddr_cke_q,
      D => ddr_cke_int,
      C => clk180
      );

  r20 : OBUF
    port map (
      I => ddr_cke_q,
      O => ddr_cke
      );


---- *******************************************  ----
----  Includes the instantiation of FD for address  ----
----            signals                           ----
---- *******************************************  ----

  iob_addr_0 : FD 
	port map (
      Q    => ddr_address_reg(0),
      D    => ddr_address_cntrl(0),
      C    => clk180
	  );
  iob_addr_1 : FD 
	port map (
      Q    => ddr_address_reg(1),
      D    => ddr_address_cntrl(1),
      C    => clk180
	  );
  iob_addr_2 : FD 
	port map (
      Q    => ddr_address_reg(2),
      D    => ddr_address_cntrl(2),
      C    => clk180
	  );
  iob_addr_3 : FD 
	port map (
      Q    => ddr_address_reg(3),
      D    => ddr_address_cntrl(3),
      C    => clk180
	  );
  iob_addr_4 : FD 
	port map (
      Q    => ddr_address_reg(4),
      D    => ddr_address_cntrl(4),
      C    => clk180
	  );
  iob_addr_5 : FD 
	port map (
      Q    => ddr_address_reg(5),
      D    => ddr_address_cntrl(5),
      C    => clk180
	  );
  iob_addr_6 : FD 
	port map (
      Q    => ddr_address_reg(6),
      D    => ddr_address_cntrl(6),
      C    => clk180
	  );
  iob_addr_7 : FD 
	port map (
      Q    => ddr_address_reg(7),
      D    => ddr_address_cntrl(7),
      C    => clk180
	  );
  iob_addr_8 : FD 
	port map (
      Q    => ddr_address_reg(8),
      D    => ddr_address_cntrl(8),
      C    => clk180
	  );
  iob_addr_9 : FD 
	port map (
      Q    => ddr_address_reg(9),
      D    => ddr_address_cntrl(9),
      C    => clk180
	  );
  iob_addr_10 : FD 
	port map (
      Q    => ddr_address_reg(10),
      D    => ddr_address_cntrl(10),
      C    => clk180
	  );
  iob_addr_11 : FD 
	port map (
      Q    => ddr_address_reg(11),
      D    => ddr_address_cntrl(11),
      C    => clk180
	  );
  iob_addr_12 : FD 
	port map (
      Q    => ddr_address_reg(12),
      D    => ddr_address_cntrl(12),
      C    => clk180
	  );
  iob_ba_0 : FD 
    port map (
      Q    =>  ddr_ba_reg(0),
      D    =>  ddr_ba_cntrl(0),
      C    =>  clk180
	  );
  iob_ba_1 : FD 
    port map (
      Q    =>  ddr_ba_reg(1),
      D    =>  ddr_ba_cntrl(1),
      C    =>  clk180
	  );

  iob_odt: FD
    port map (
      Q => ddr_ODT_reg,
      D => ddr_ODT_cntrl,
      C => clk180
      );

---- *************************************  ----
----  Output buffers for address signals    ----
---- *************************************  ----

  G3 : for i in (row_address -1) downto 0 generate
    r : OBUF
      port map (
        I => ddr_address_reg(i),
        O => ddr_address(i)
        );
  end generate G3;

  G4 : for i in (bank_address -1) downto 0 generate

    r : OBUF
      port map (
        I => ddr_ba_reg(i),
        O => ddr_ba(i)
        );

  end generate G4;



-- ODT REGISTERED AT IOB
  ODT_iob_obuf : OBUF
    port map (
      I => ddr_ODT_reg,
      O => ddr_ODT
      );


  rst_iob_inbuf : IBUF port map
    (
      I => rst_dqs_div_in,
      O => rst_dqs_div
      );

  rst_iob_outbuf : OBUF port map (
    I => rst_dqs_div_int,
    O => rst_dqs_div_out
    );

end arc;







