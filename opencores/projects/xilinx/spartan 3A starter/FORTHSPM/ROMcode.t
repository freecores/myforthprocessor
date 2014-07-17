----------------------------------------------------------------------------------
-- Company: RIIC
-- Engineer: Gerhard Hohner Mat.nr.: 7555111
-- 
-- Create Date:    01/07/2004 
-- Design Name:    Diplomarbeit
-- Module Name:    ROMcode - Rtl 
-- Project Name:   32 bit FORTH processor
-- Target Devices: Spartan 3
-- Tool versions:  ISE 8.2
-- Description: implements a ROM containing the BIOS
-- Dependencies: global.vhd
-- 
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------


library IEEE, work;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use work.global.all;

entity ROMcode is
  port (Clock: in std_ulogic;									-- system clock
        Address: in std_ulogic_vector(ROMrange);		-- address bus
		  Data: out DataVec);									-- outgoing data
end ROMcode;

architecture RTL of ROMcode is

#loop
  signal old#pends : std_ulogic;
  signal bdata#pends : DataVec;
  type blocks#pends is array (0 to #pblksize  / 4 - 1) of DataVec;
  constant ROM#pends : blocks#pends := 
(
#block
);
                          
#endloop
  
begin
  
  fetch: process(Clock) is
  begin
    if rising_edge(Clock) then
#loop
      old#pends <= Address(#pld(blksize) );
      bData#pends <= ROM#pends (to_integer(std_ulogic_vector(Address(#pld(blksize)  - 1 downto 2))) MOD ROM#pends 'length);
#endloop
    end if;
  end process fetch;

  Data <= 
#loop
          bdata#pends when old#pends = '0' else
#endloop
          (others => '0');
			 
end RTL;
