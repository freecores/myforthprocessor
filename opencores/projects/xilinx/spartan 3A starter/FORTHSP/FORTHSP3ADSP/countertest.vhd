--
-- Autor: Gerhard Hohner Mat.nr.: 7555111
--
-- Aufgabe:
-- implementiert ALU des PROL16
--				   


library IEEE, work;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.global.all;

ENTITY counter_countertest_vhd_tb IS
END counter_countertest_vhd_tb;

ARCHITECTURE behavior OF counter_countertest_vhd_tb IS 

	COMPONENT counter
	PORT(
		nReset : IN std_ulogic;
		Clock : IN std_ulogic;
		source : IN std_ulogic_vector(0 to 3);
		DataIn : IN std_ulogic_vector(26 downto 0);
		Address : IN std_ulogic_vector(3 downto 2);
		Writes : IN std_ulogic;          
		DataOut : OUT std_ulogic_vector(26 downto 0);
		Tint : OUT std_ulogic_vector(3 downto 0)
		);
	END COMPONENT;

	SIGNAL nReset :  std_ulogic;
	SIGNAL Clock :  std_ulogic;
	SIGNAL source :  std_ulogic_vector(0 to 3);
	SIGNAL DataIn :  std_ulogic_vector(26 downto 0);
	SIGNAL DataOut :  std_ulogic_vector(26 downto 0);
	SIGNAL Address :  std_ulogic_vector(3 downto 2);
	SIGNAL Writes :  std_ulogic;
	SIGNAL Tint :  std_ulogic_vector(3 downto 0);

  for one:counter
	use entity work.counter(Rtl)
	PORT MAP(
		nReset => nReset,
		Clock => Clock,
		source => source,
		DataIn => DataIn,
		DataOut => DataOut,
		Address => Address,
		Writes => Writes,
		Tint => Tint
	);

BEGIN

	one: component counter
	PORT MAP(
		nReset => nReset,
		Clock => Clock,
		source => source,
		DataIn => DataIn,
		DataOut => DataOut,
		Address => Address,
		Writes => Writes,
		Tint => Tint
	);


-- *** Test Bench - User Defined Section ***
   tb : PROCESS
   BEGIN
      nReset <= '0' after 0 ns,
	           '1' after 200 ns;
      Writes <= '0';
	 source <= (others => '0');
	 wait until nReset = '1';
	 -- set counter 0
	 Address <= (others => '0');
	 DataIn <= (1 | 2 | 3 => '1', others => '0');
	 Writes <= '1';
	 wait for 300 ns;
	 Writes <= '0';

	 wait until Tint(0) = '1';

	 wait for 200 ns;

      report "all done well"
      severity failure;

      wait; -- will wait forever
   END PROCESS;
-- *** End Test Bench - User Defined Section ***
   quartz: process(Clock) is
   begin
     if Clock'event and Clock = '1' then
	  Clock <= '0' after 50 ns;
     else
	  Clock <= '1' after 50 ns;
     end if;
   end process quartz;

END;
