----------------------------------------------------------------------------------
-- Company: RIIC
-- Engineer: Gerhard Hohner Mat.nr.: 7555111
-- 
-- Create Date:    01/07/2004 
-- Design Name:    Diplomarbeit
-- Module Name:    IntVectors - Rtl 
-- Project Name:   32 bit FORTH processor
-- Target Devices: Spartan 3
-- Tool versions:  ISE 8.2
-- Description: implements an interrupt controler
-- Dependencies: global.vhd
-- 
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------


library IEEE, work;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.global.all;

entity IntVectors is
  generic (constant TableBitWidth : integer := 4);
  port (nReset: in std_ulogic;															-- system reset
        Clock: in std_ulogic;																-- system clock
	     Quit: in std_ulogic;																-- interrupt quitted
		  Token: in std_ulogic;                                              -- update pending
	     IntSignal: in std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);	-- input
	     Writes: in std_ulogic;															-- write port
	     Reads: in std_ulogic;																-- read port
	     Address: in std_ulogic_vector(1 downto 0);									-- port
	     DataIn: in std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);		-- incomming data
	     DataOut: out std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);	-- outgoing data
	     Vector: out std_ulogic_vector(TableBitWidth - 1 downto 0);			-- vector number
	     VectorValid: out std_ulogic);													-- vector valid
end IntVectors;

architecture Rtl of IntVectors is
signal buffered, pending, oldSignal, Interrupts, masks, blocking: std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
signal Valid: std_ulogic;
signal pipe: std_ulogic_vector(3 downto 0);

begin

  VectorValid <= Valid;
  					
  DataOut <= blocking when Address(1) = '1' else
             (others => pipe(0)) when Address(0) = '1' else
				 masks;

  Operation: process(nReset, Clock) is
  variable newpending, newInterrupts: std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
  variable x: std_ulogic;
  variable newpipe: std_ulogic_vector(3 downto 0);
  begin
    if nReset /= '1' then
      masks <= (others => '1');
	   pending <= (others => '0');
	   interrupts <= (others => '0');
	   oldSignal <= (others => '1');
		pipe <= (others => '0');
    elsif rising_edge(Clock) then
	   newpipe := pipe(2 downto 0) & pipe(0);
      x := pipe(3);

	   -- evaluate command
	   if Address(1) = '1' then
		  null;
	   elsif Address(0) = '1' then
		  -- Interrupt lines unmasked, if desired
		  if Reads = '1' or (Writes = '1' and DataIn(0) = '0') then
		    newpipe := (others => '0');
			 x := '0';
		  elsif Writes = '1' then
          newpipe(0) := '1';
		  end if;
	   else
	     -- read distinct newInterrupts vector and mask
	     if Writes = '1' then
	       -- write distinct newInterrupts vector and mask
	       masks <= DataIn;
		  end if;
	   end if;
      
	   -- evaluate interrupts
	   newInterrupts := ((IntSignal and not oldSignal) or Interrupts) and (not masks);
		oldSignal <= IntSignal;
		
	   -- get unblocked interrupts
	   newpending := newInterrupts and std_ulogic_vector(unsigned(Blocking) - 1) and not Blocking;
	   if Token = '0' and unsigned(pending or buffered) /= 0 then
	     newpending := (others => '0');
	   elsif x = '0' then
	     newpending(newpending'high downto 2) := (others => '0');
	   end if;
		
	   -- select the one with highest priority
	   newpending := newpending and not std_ulogic_vector(unsigned(newpending) - 1);
		
	   -- this one is not longer pending
	   Interrupts <= newInterrupts and not newpending;
		pipe <= newpipe;
		pending <= newpending;
    end if;
  end process Operation;

  evaluate: process(nReset, Clock) is
  variable x, newBlocking: std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
  begin
    if nReset /= '1' then
	   buffered <= (others => '0');
	   blocking <= (others => '0');
		Valid <= '0';
	 elsif rising_edge(Clock) then
	   newBlocking := blocking;
		
	   -- evaluate opcode
	   if Address(1) = '1' then
		  if Writes = '1' then
	       -- remove blockade with highest priority
          newBlocking := std_ulogic_vector(unsigned(blocking) - 1) and blocking;
		  end if;
      end if;
		
	   -- set blockade
		if Token = '1' then
		  blocking <= newBlocking or buffered;
		  x := (others => '0');
		else
		  blocking <= newBlocking;
		  x := buffered;
		end if;
		
	   if unsigned(pending) /= 0 then
		  x := pending;
		  Valid <= '1';
		  Vector(0) <= pending(1) or pending(3) or pending(5) or pending(7) or pending(9) or pending(11) or pending(13) or pending(15);
		  Vector(1) <= pending(2) or pending(3) or pending(6) or pending(7) or pending(10) or pending(11) or pending(14) or pending(15);
		  Vector(2) <= pending(4) or pending(5) or pending(6) or pending(7) or pending(12) or pending(13) or pending(14) or pending(15);
		  Vector(3) <= pending(8) or pending(9) or pending(10) or pending(11) or pending(12) or pending(13) or pending(14) or pending(15);
      else
		  Valid <= Valid and not Quit;
	   end if;

      buffered <= x;
	 end if;
  end process evaluate;
  
end Rtl;
