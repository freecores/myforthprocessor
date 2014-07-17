----------------------------------------------------------------------------------
-- Company: RIIC
-- Engineer: Gerhard Hohner Mat.nr.: 7555111
-- 
-- Create Date:    01/07/2004 
-- Design Name:    Diplomarbeit
-- Module Name:    counter - Rtl 
-- Project Name:   32 bit FORTH processor
-- Target Devices: Spartan 3
-- Tool versions:  ISE 8.2
-- Description: implements 4 downcounter
-- Dependencies: global.vhd
-- 
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
-- implements 4 software controlled down counters, each 24 bith wide
--
-- source		the number of prescaler, if source(2) is 0
--             00 and source(2) 1, the external source input pin triggers count
--             11 and source(2) 1, count locked
--             in all other combinations, system source(2) triggers count
-- source(2)       if 0, the terminal count of a prescaler identified through source triggers count
-- reload      the reload value for initialization after terminal count
-- count       the counter
-- tc          if count reached zero it holds 1, 0 otherwise
-- 
--				   


library IEEE, work;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use work.global.all;

entity counter is
  port (nReset: in std_ulogic;											-- system reset
        Clock: in std_ulogic;												-- system clock
		  Address: in std_ulogic_vector(1 downto 0);             -- address
	     source: in std_ulogic_vector(0 to 3);						-- counter input
	     DataIn: in std_ulogic_vector(26 downto 0);					-- incomming data
	     DataOut: out std_ulogic_vector(26 downto 0);				-- outgoing data
	     Writes: in std_ulogic;							    		   -- write operation (active high)
	     Tint: out std_ulogic_vector(3 downto 0));					-- interrupt request
end counter;

architecture Rtl of counter is
signal oldsource: std_ulogic_vector(0 to 3);
type acounter is record
  source: unsigned(2 downto 0);
  reload: unsigned(23 downto 0);
  count: unsigned(23 downto 0);
  tc: std_ulogic;
end record;
type carray is array (0 to 3) of acounter;
signal counters: carray;
type darray is array (0 to 3) of std_ulogic_vector(DataOut'range);
signal Data: darray;

begin

  DataOut <= Data(to_integer(std_ulogic_vector(Address)));
  
  count: process(nReset, Clock) is
  variable c: acounter;
  variable lock, shot: std_ulogic;
  begin
    if nReset /= '1' then
	   for i in 0 to 3 loop
        oldsource(i) <= '1';
		  Tint(i) <= '0';
		  counters(i).source <= "111";
		  counters(i).reload <= (others => '0');
		  counters(i).count <= (others => '0');
		  counters(i).tc <= '1';
		end loop;
    elsif rising_edge(Clock) then
      for i in 0 to 3 loop
		  c := counters(i);

	     -- save input
		  if Writes = '1' and to_integer(Address) = i then
		    counters(i).source <= unsigned(DataIn(2 downto 0));
		    counters(i).reload <= unsigned(DataIn(26 downto 3));
		  end if;

		  -- force reload, if locked
	     lock := c.source(1) and c.source(0) and c.source(2);
		  c.tc := c.tc or lock;

		  if c.tc = '1' then
		    c.count := c.reload;
		  end if;

	     if c.source(2) = '0' then
	       -- event is zero count of an other counter
		    shot := counters(to_integer(std_ulogic_vector(c.source(1 downto 0)))).tc;
	     elsif unsigned(c.source(1 downto 0)) = 0 then
	       -- event comes from external source
	       shot := source(i) and not oldsource(i);
	     else
	       -- count, if counter set
	       shot := not lock;
        end if;

		  -- count down
	     if shot = '1' then
	       c.count := c.count - 1;
	       if c.count = 0 then
	         c.tc := '1'; -- zero reached
          else
	         c.tc := '0';
	       end if;
	     end if;

	     -- update

	     oldsource(i) <= source(i);
	     TInt(i) <= c.tc and shot;

		  -- read counter always
		  Data(i) <= std_ulogic_vector(c.count) & std_ulogic_vector(c.source);

        counters(i).count <= c.count;
        counters(i).tc <= c.tc;
		end loop;
    end if;
  end process count;
  
end Rtl;