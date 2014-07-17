	 
library IEEE, work;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.global.all;

ENTITY intvectors_inttest_vhd_tb IS
END intvectors_inttest_vhd_tb;

ARCHITECTURE behavior OF intvectors_inttest_vhd_tb IS 

	COMPONENT intvectors
     generic (constant TableBitWidth : integer := 4);
	PORT(
		nReset : IN std_ulogic;
		Clock : IN std_ulogic;
		Quit : IN std_ulogic;
            Token : IN std_ulogic;
		IntSignal : IN std_ulogic_vector(15 downto 0);
	      Writes: in std_ulogic;
	      Reads: in std_ulogic;
		Address : IN std_ulogic_vector(1 downto 0);
	      DataIn: in std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
	      DataOut: out std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
	      Vector: out std_ulogic_vector(TableBitWidth - 1 downto 0);
		VectorValid : OUT std_ulogic
		);
	END COMPONENT;

	for one:intvectors
	  use entity work.intvectors(Rtl)
	PORT MAP(
		nReset => nReset,
		Clock => Clock,
		Quit => Quit,
		Token => Token,
		IntSignal => IntSignal,
		Reads => Reads,
		Writes => Writes
		Address => Address,
		DataIn => DataIn,
		DataOut => DataOut,
		Vector => Vector,
		VectorValid => VectorValid
	);

	SIGNAL nReset :  std_ulogic;
	SIGNAL Clock :  std_ulogic;
	SIGNAL Quit, Token, Reads, Writes :  std_ulogic;
	SIGNAL IntSignal :  std_ulogic_vector(0 to 15);
	SIGNAL Address :  std_ulogic_vector(1 downto 0);
	SIGNAL DataIn :  std_ulogic_vector(15 downto 0);
	SIGNAL DataOut :  std_ulogic_vector(15 downto 0);
	SIGNAL Vector :  std_ulogic_vector(3 downto 0);
	SIGNAL VectorValid :  std_ulogic;

BEGIN

	one:component intvectors
	generic map(TableBitWidth => 4)
	PORT MAP(
		nReset => nReset,
		Clock => Clock,
		Quit => Quit,
		Token => Token,
		IntSignal => IntSignal,
		Reads => Reads,
		Writes => Writes
		Address => Address,
		DataIn => DataIn,
		DataOut => DataOut,
		Vector => Vector,
		VectorValid => VectorValid
	);


-- *** Test Bench - User Defined Section ***
   tb : PROCESS
   BEGIN
      IOCode <= (others => '0');
	 Address <= (others => '0');
	 IntSignal <= (others => '0');
	 Quit <= '0';
      nReset <= '0' after 0 ns,
	           '1' after 200 ns;
      wait until nReset = '1';
	 wait for 100 ns;
	 IntSignal(4) <= '1';
	 wait for 200 ns;
	 IntSignal(5) <= '1';
	 wait for 200 ns;
	 IntSignal(4) <= '0';
	 IntSignal(5) <= '0';
	 Quit <= '1';
	 wait for 100 ns;
	 Quit <= '0';
	 IntSignal(3) <= '1';

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
