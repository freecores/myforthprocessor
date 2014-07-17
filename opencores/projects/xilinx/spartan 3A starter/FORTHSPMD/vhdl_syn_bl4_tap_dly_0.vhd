-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- All Rights Reserved
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_tap_dly.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:36 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     :This module generates a 32 bit tap delay register used by the 
--		cal_ctl module to find out the phase transitions.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;


entity vhdl_syn_bl4_tap_dly is
  port (
    clk   : in  std_logic;
    reset : in  std_logic;
    tapIn : in  std_logic;
    flop2 : out std_logic_vector(31 downto 0)
    );
end vhdl_syn_bl4_tap_dly;

architecture arc_tap_dly of vhdl_syn_bl4_tap_dly is
  attribute KEEP_HIERARCHY : string;
  attribute KEEP_HIERARCHY of arc_tap_dly : architecture is "YES";

  signal tap   : std_logic_vector(31 downto 0);
  signal flop1 : std_logic_vector(31 downto 0);
  signal high  : std_logic;
  signal LOW   : std_logic;

  attribute preserve_signal : boolean;
  
  attribute preserve_signal of         tap    : signal is true;
  attribute preserve_signal of         flop1  : signal is true;



  signal flop2i_0  : std_logic;
  signal flop2i_1  : std_logic;
  signal flop2i_2  : std_logic;
  signal flop2i_3  : std_logic;
  signal flop2i_4  : std_logic;
  signal flop2i_5  : std_logic;
  signal flop2i_6  : std_logic;
  signal flop2i_7  : std_logic;
  signal flop2i_8  : std_logic;
  signal flop2i_9  : std_logic;
  signal flop2i_10 : std_logic;
  signal flop2i_11 : std_logic;
  signal flop2i_12 : std_logic;
  signal flop2i_13 : std_logic;
  signal flop2i_14 : std_logic;
  signal flop2i_15 : std_logic;
  signal flop2i_16 : std_logic;
  signal flop2i_17 : std_logic;
  signal flop2i_18 : std_logic;
  signal flop2i_19 : std_logic;
  signal flop2i_20 : std_logic;
  signal flop2i_21 : std_logic;
  signal flop2i_22 : std_logic;
  signal flop2i_23 : std_logic;
  signal flop2i_24 : std_logic;
  signal flop2i_25 : std_logic;
  signal flop2i_26 : std_logic;
  signal flop2i_27 : std_logic;
  signal flop2i_28 : std_logic;
  signal flop2i_29 : std_logic;
  signal flop2i_30 : std_logic;
  signal reset_r   : std_logic;


begin

  process(clk)
  begin
    if(clk'event and clk='1') then
      reset_r <= reset;
    end if;
  end process;

  high <= '1';
  low  <= '0';

  l0 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tapIn,
      I2 => low,
      I3 => high,
      O  => tap(0)
      );

  l1 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(0),
      I2 => low,
      I3 => high,
      O  => tap(1)
      );
  l2 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(1),
      I2 => low,
      I3 => high,
      O  => tap(2)
      );
  l3 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(2),
      I2 => low,
      I3 => high,
      O  => tap(3)
      );
  l4 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(3),
      I2 => low,
      I3 => high,
      O  => tap(4)
      );
  l5 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(4),
      I2 => low,
      I3 => high,
      O  => tap(5)
      );
  l6 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(5),
      I2 => low,
      I3 => high,
      O  => tap(6)
      );
  l7 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(6),
      I2 => low,
      I3 => high,
      O  => tap(7)
      );
  l8 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(7),
      I2 => low,
      I3 => high,
      O  => tap(8)
      );
  l9 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(8),
      I2 => low,
      I3 => high,
      O  => tap(9)
      );
  l10 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(9),
      I2 => low,
      I3 => high,
      O  => tap(10)
      );
  l11 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(10),
      I2 => low,
      I3 => high,
      O  => tap(11)
      );
  l12 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(11),
      I2 => low,
      I3 => high,
      O  => tap(12)
      );
  l13 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(12),
      I2 => low,
      I3 => high,
      O  => tap(13)
      );
  l14 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(13),
      I2 => low,
      I3 => high,
      O  => tap(14)
      );
  l15 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(14),
      I2 => low,
      I3 => high,
      O  => tap(15)
      );
  l16 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(15),
      I2 => low,
      I3 => high,
      O  => tap(16)
      );
  l17 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(16),
      I2 => low,
      I3 => high,
      O  => tap(17)
      );
  l18 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(17),
      I2 => low,
      I3 => high,
      O  => tap(18)
      );
  l19 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(18),
      I2 => low,
      I3 => high,
      O  => tap(19)
      );
  l20 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(19),
      I2 => low,
      I3 => high,
      O  => tap(20)
      );
  l21 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(20),
      I2 => low,
      I3 => high,
      O  => tap(21)
      );
  l22 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(21),
      I2 => low,
      I3 => high,
      O  => tap(22)
      );
  l23 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(22),
      I2 => low,
      I3 => high,
      O  => tap(23)
      );
  l24 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(23),
      I2 => low,
      I3 => high,
      O  => tap(24)
      );
  l25 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(24),
      I2 => low,
      I3 => high,
      O  => tap(25)
      );
  l26 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(25),
      I2 => low,
      I3 => high,
      O  => tap(26)
      );
  l27 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(26),
      I2 => low,
      I3 => high,
      O  => tap(27)
      );
  l28 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(27),
      I2 => low,
      I3 => high,
      O  => tap(28)
      );
  l29 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(28),
      I2 => low,
      I3 => high,
      O  => tap(29)
      );
  l30 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(29),
      I2 => low,
      I3 => high,
      O  => tap(30)
      );
  l31 : LUT4 generic map (INIT => x"e2e2")
    port map (
      I0 => high,
      I1 => tap(30),
      I2 => low,
      I3 => high,
      O  => tap(31)
      );


  r0 : FDR port map (
    Q => flop1(0),
    C => clk,
    D => tap(0),
    R => reset_r
    );

  r1 : FDR port map (
    Q => flop1(1),
    C => clk,
    D => tap(1),
    R => reset_r
    );


  r2 : FDR port map (
    Q => flop1(2),
    C => clk,
    D => tap(2),
    R => reset_r
    );

  r3 : FDR port map (
    Q => flop1(3),
    C => clk,
    D => tap(3),
    R => reset_r
    );

  r4 : FDR port map (
    Q => flop1(4),
    C => clk,
    D => tap(4),
    R => reset_r
    );

  r5 : FDR port map (
    Q => flop1(5),
    C => clk,
    D => tap(5),
    R => reset_r
    );

  r6 : FDR port map (
    Q => flop1(6),
    C => clk,
    D => tap(6),
    R => reset_r
    );

  r7 : FDR port map (
    Q => flop1(7),
    C => clk,
    D => tap(7),
    R => reset_r
    );

  r8 : FDR port map (
    Q => flop1(8),
    C => clk,
    D => tap(8),
    R => reset_r
    );

  r9 : FDR port map (
    Q => flop1(9),
    C => clk,
    D => tap(9),
    R => reset_r
    );

  r10 : FDR port map (
    Q => flop1(10),
    C => clk,
    D => tap(10),
    R => reset_r
    );

  r11 : FDR port map (
    Q => flop1(11),
    C => clk,
    D => tap(11),
    R => reset_r
    );

  r12 : FDR port map (
    Q => flop1(12),
    C => clk,
    D => tap(12),
    R => reset_r
    );

  r13 : FDR port map (
    Q => flop1(13),
    C => clk,
    D => tap(13),
    R => reset_r
    );

  r14 : FDR port map (
    Q => flop1(14),
    C => clk,
    D => tap(14),
    R => reset_r
    );

  r15 : FDR port map (
    Q => flop1(15),
    C => clk,
    D => tap(15),
    R => reset_r
    );

  r16 : FDR port map (
    Q => flop1(16),
    C => clk,
    D => tap(16),
    R => reset_r
    );

  r17 : FDR port map (
    Q => flop1(17),
    C => clk,
    D => tap(17),
    R => reset_r
    );

  r18 : FDR port map (
    Q => flop1(18),
    C => clk,
    D => tap(18),
    R => reset_r
    );

  r19 : FDR port map (
    Q => flop1(19),
    C => clk,
    D => tap(19),
    R => reset_r
    );

  r20 : FDR port map (
    Q => flop1(20),
    C => clk,
    D => tap(20),
    R => reset_r
    );

  r21 : FDR port map (
    Q => flop1(21),
    C => clk,
    D => tap(21),
    R => reset_r
    );

  r22 : FDR port map (
    Q => flop1(22),
    C => clk,
    D => tap(22),
    R => reset_r
    );

  r23 : FDR port map (
    Q => flop1(23),
    C => clk,
    D => tap(23),
    R => reset_r
    );

  r24 : FDR port map (
    Q => flop1(24),
    C => clk,
    D => tap(24),
    R => reset_r
    );

  r25 : FDR port map (
    Q => flop1(25),
    C => clk,
    D => tap(25),
    R => reset_r
    );

  r26 : FDR port map (
    Q => flop1(26),
    C => clk,
    D => tap(26),
    R => reset_r
    );

  r27 : FDR port map (
    Q => flop1(27),
    C => clk,
    D => tap(27),
    R => reset_r
    );

  r28 : FDR port map (
    Q => flop1(28),
    C => clk,
    D => tap(28),
    R => reset_r
    );

  r29 : FDR port map (
    Q => flop1(29),
    C => clk,
    D => tap(29),
    R => reset_r
    );

  r30 : FDR port map (
    Q => flop1(30),
    C => clk,
    D => tap(30),
    R => reset_r
    );

  r31 : FDR port map (
    Q => flop1(31),
    C => clk,
    D => tap(31),
    R => reset_r
    );

  flop2i_0  <= flop1(0) xnor flop1(1);
  flop2i_1  <= flop1(1) xnor flop1(2);
  flop2i_2  <= flop1(2) xnor flop1(3);
  flop2i_3  <= flop1(3) xnor flop1(4);
  flop2i_4  <= flop1(4) xnor flop1(5);
  flop2i_5  <= flop1(5) xnor flop1(6);
  flop2i_6  <= flop1(6) xnor flop1(7);
  flop2i_7  <= flop1(7) xnor flop1(8);
  flop2i_8  <= flop1(8) xnor flop1(9);
  flop2i_9  <= flop1(9) xnor flop1(10);
  flop2i_10 <= flop1(10) xnor flop1(11);
  flop2i_11 <= flop1(11) xnor flop1(12);
  flop2i_12 <= flop1(12) xnor flop1(13);
  flop2i_13 <= flop1(13) xnor flop1(14);
  flop2i_14 <= flop1(14) xnor flop1(15);
  flop2i_15 <= flop1(15) xnor flop1(16);
  flop2i_16 <= flop1(16) xnor flop1(17);
  flop2i_17 <= flop1(17) xnor flop1(18);
  flop2i_18 <= flop1(18) xnor flop1(19);
  flop2i_19 <= flop1(19) xnor flop1(20);
  flop2i_20 <= flop1(20) xnor flop1(21);
  flop2i_21 <= flop1(21) xnor flop1(22);
  flop2i_22 <= flop1(22) xnor flop1(23);
  flop2i_23 <= flop1(23) xnor flop1(24);
  flop2i_24 <= flop1(24) xnor flop1(25);
  flop2i_25 <= flop1(25) xnor flop1(26);
  flop2i_26 <= flop1(26) xnor flop1(27);
  flop2i_27 <= flop1(27) xnor flop1(28);
  flop2i_28 <= flop1(28) xnor flop1(29);
  flop2i_29 <= flop1(29) xnor flop1(30);
  flop2i_30 <= flop1(30) xnor flop1(31);

  u0 : FDR port map (
    Q => flop2(0),
    C => clk,
    D => flop2i_0,
    R => reset_r
    );

  u1 : FDR port map (
    Q => flop2(1),
    C => clk,
    D => flop2i_1,
    R => reset_r
    );

  u2 : FDR port map (
    Q => flop2(2),
    C => clk,
    D => flop2i_2,
    R => reset_r
    );

  u3 : FDR port map (
    Q => flop2(3),
    C => clk,
    D => flop2i_3,
    R => reset_r
    );


  u4 : FDR port map (
    Q => flop2(4),
    C => clk,
    D => flop2i_4,
    R => reset_r
    );

  u5 : FDR port map (
    Q => flop2(5),
    C => clk,
    D => flop2i_5,
    R => reset_r
    );

  u6 : FDR port map (
    Q => flop2(6),
    C => clk,
    D => flop2i_6,
    R => reset_r
    );

  u7 : FDR port map (
    Q => flop2(7),
    C => clk,
    D => flop2i_7,
    R => reset_r
    );

  u8 : FDR port map (
    Q => flop2(8),
    C => clk,
    D => flop2i_8,
    R => reset_r
    );

  u9 : FDR port map (
    Q => flop2(9),
    C => clk,
    D => flop2i_9,
    R => reset_r
    );

  u10 : FDR port map (
    Q => flop2(10),
    C => clk,
    D => flop2i_10,
    R => reset_r
    );

  u11 : FDR port map (
    Q => flop2(11),
    C => clk,
    D => flop2i_11,
    R => reset_r
    );

  u12 : FDR port map (
    Q => flop2(12),
    C => clk,
    D => flop2i_12,
    R => reset_r
    );

  u13 : FDR port map (
    Q => flop2(13),
    C => clk,
    D => flop2i_13,
    R => reset_r
    );

  u14 : FDR port map (
    Q => flop2(14),
    C => clk,
    D => flop2i_14,
    R => reset_r
    );

  u15 : FDR port map (
    Q => flop2(15),
    C => clk,
    D => flop2i_15,
    R => reset_r
    );

  u16 : FDR port map (
    Q => flop2(16),
    C => clk,
    D => flop2i_16,
    R => reset_r
    );

  u17 : FDR port map (
    Q => flop2(17),
    C => clk,
    D => flop2i_17,
    R => reset_r
    );

  u18 : FDR port map (
    Q => flop2(18),
    C => clk,
    D => flop2i_18,
    R => reset_r
    );

  u19 : FDR port map (
    Q => flop2(19),
    C => clk,
    D => flop2i_19,
    R => reset_r
    );

  u20 : FDR port map (
    Q => flop2(20),
    C => clk,
    D => flop2i_20,
    R => reset_r
    );

  u21 : FDR port map (
    Q => flop2(21),
    C => clk,
    D => flop2i_21,
    R => reset_r
    );


  u22 : FDR port map (
    Q => flop2(22),
    C => clk,
    D => flop2i_22,
    R => reset_r
    );

  u23 : FDR port map (
    Q => flop2(23),
    C => clk,
    D => flop2i_23,
    R => reset_r
    );


  u24 : FDR port map (
    Q => flop2(24),
    C => clk,
    D => flop2i_24,
    R => reset_r
    );

  u25 : FDR port map (
    Q => flop2(25),
    C => clk,
    D => flop2i_25,
    R => reset_r
    );

  u26 : FDR port map (
    Q => flop2(26),
    C => clk,
    D => flop2i_26,
    R => reset_r
    );

  u27 : FDR port map (
    Q => flop2(27),
    C => clk,
    D => flop2i_27,
    R => reset_r
    );


  u28 : FDR port map (
    Q => flop2(28),
    C => clk,
    D => flop2i_28,
    R => reset_r
    );

  u29 : FDR port map (
    Q => flop2(29),
    C => clk,
    D => flop2i_29,
    R => reset_r
    );

  u30 : FDR port map (
    Q => flop2(30),
    C => clk,
    D => flop2i_30,
    R => reset_r
    );

  u31 : FDR port map (
    Q => flop2(31),
    C => clk,
    D => flop1(31),
    R => reset_r
    );

end arc_tap_dly;
