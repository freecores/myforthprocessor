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
  type blocks is array (0 to #pblocksize  / 4 - 1) of DataVec;
  type myarray is array (natural range <>) of blocks;
  constant ROM: myarray := (
#r
(
#block
)
#ep,
                           );
  type dvec is array (0 to #pi  - 1) of DataVec;
  signal bdata: dvec;
  signal old: std_ulogic_vector(ROMrange'high downto #pld(blocksize) );
  
begin
  
  fetch: process(Clock) is
  begin
    if rising_edge(Clock) then
         old <= Address(old'range);
	   for i in bdata'range loop
	     bData(i) <= ROM(i)(to_integer(std_ulogic_vector(Address(#pld(blocksize)  - 1 downto 2))) MOD blocks'length);
		end loop;
	 end if;
  end process fetch;

  Data <= 
#r
          bdata(#pi ) when unsigned(old) = #pi else
#e
          (others => '0');
			 
end RTL;
