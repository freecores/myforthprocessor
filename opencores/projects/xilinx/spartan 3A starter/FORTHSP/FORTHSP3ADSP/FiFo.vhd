----------------------------------------------------------------------------------
-- Company: RIIC
-- Engineer: Gerhard Hohner Mat.nr.: 7555111
-- 
-- Create Date:    01/07/2004 
-- Design Name:    Diplomarbeit
-- Module Name:    MYCPU - Rtl 
-- Project Name:   32 bit FORTH processor
-- Target Devices: Spartan 3
-- Tool versions:  ISE 8.2
-- Description: interfacing the busses
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

---- Uncomment the following library declaration if instantiating
---- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity FiFo is
  generic(constant WordLength: integer := 9;
          constant ldFiFoSize: integer := 3);
  port (nReset: in std_ulogic;
        Clock: in std_ulogic;
		  put: in std_ulogic;
		  get: in std_ulogic;
        DataIn: in std_ulogic_vector(WordLength - 1 downto 0);
        DataOut: out std_ulogic_vector(WordLength - 1 downto 0);
		  notFull: out std_ulogic;
		  AlmostFull: out std_ulogic;
		  Empty: out std_ulogic);
end FiFo;

architecture Rtl of FiFo is
type farray is array (0 to 2 ** ldFiFoSize - 1) of std_ulogic_vector(WordLength - 1 downto 0);
signal myFiFo: farray;
signal first, last: unsigned(ldFiFoSize - 1 downto 0);
signal free: unsigned(ldFiFoSize downto 0);

begin
  notFull <= '0' when nReset /= '1' or free = 0 else
             '1';
				 
  AlmostFull <= '0' when nReset /= '1' else
                free(free'high - 1) nor free(free'high);
				 
  Empty <= '0' when nReset /= '1' else
           free(free'high);
  
  DataOut <= (others => '0') when nReset /= '1' else
             myFiFo(to_integer(std_ulogic_vector(first)));
				 
  work: process(nReset, Clock) is
  variable incr: unsigned(ldFiFoSize downto 0);
  begin
    if nReset /= '1' then
		first <= (others => '0');
		last <= (others => '0');
		free(free'high) <= '1';
		free(free'high - 1 downto 0) <= (others => '0');
		for i in myFiFo'range loop
		  myFiFo(i) <= (others => '0');
		end loop;
	 elsif rising_edge(Clock) then
	   incr := free;
	   if free /= 0 and put = '1' then
		  myFiFo(to_integer(std_ulogic_vector(last))) <= DataIn;
		  incr := free - 1;
		  last <= last + 1;
		end if;
		if free(free'high) = '0' and get = '1' then
		  incr := incr + 1;
		  first <= first + 1;
		end if;
		free <= incr;
	 end if;
  end process work;

end Rtl;

