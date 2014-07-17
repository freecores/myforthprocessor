----------------------------------------------------------------------------------
-- Company: RIIC
-- Engineer: Gerhard Hohner Mat.nr.: 7555111
-- 
-- Create Date:    01/07/2004 
-- Design Name:    Diplomarbeit
-- Module Name:    UART - Rtl 
-- Project Name:   32 bit FORTH processor
-- Target Devices: Spartan 3
-- Tool versions:  ISE 8.2
-- Description: implements an UART
-- Dependencies: global.vhd
-- 
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
--
-- full duplex UART
--
-- Adress
--   0             Write         transmiter port
--                 Read          receiver port (bit 8 = 1 => erroneous receiving)
--   1             Write         bits 1 - 0 parity      		10 ... even, 01 ... odd, 00 ... none
--                                    3 - 2 Stopbits			00 ... 2 stopbits, 10 ... 1,5 stopbits, 01 ... 1 stopbit
--                                    4     Wordlength 		1 ... 8 bit, 0 ... 7 bit
--                 Read          read frame information 
--   2             Write         Scaling factor				must be so, that twice the baudrate will be generated
--                 Read
--	  3             Read          status register
--                               bit 0      currently receiving
--                                   1      receiver full
--                                   2      currently sending
--                                   3      transmiter ready for next word
--                                   4      baudrate set
--   4                           escape character (bit 8 marks disable of escape)
--   5                           xon character (bit 8 marks disable of xon)
--   6                           xoff character (bit 8 marks disable of xoff)
--
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
--use IEEE.numeric_std.ALL;
use IEEE.std_logic_arith.all;
use work.global.all;

entity UART is
      port (nReset: in std_ulogic;										 -- reset
            Clk: in std_ulogic;											 -- system clock
	         RxD: in std_ulogic;											 -- UART input
	         TxD: out std_ulogic;											 -- UART output
	         --DTR: out std_ulogic;											 -- data terminal ready
	         --DSR: in std_ulogic;											 -- data set ready
	         RTS: out std_ulogic;											 -- request to send
	         CToS: in std_ulogic;											 -- clear to send
		      ReadyR: out std_ulogic;										 -- receiver ready
		      ReadyT: out std_ulogic;										 -- transmiter ready
		      Address: in std_ulogic_vector(2 downto 0);			 -- port address
		      DataIn: in std_ulogic_vector(23 downto 0);			 -- parallel input
		      DataOut: out std_ulogic_vector(23 downto 0);			 -- parallel output
		      Writes: in std_ulogic;										 -- read from UART
				Reads: in std_ulogic);
end UART;

architecture Rtl of UART is
type TransmitState_t is (Idle, Sending, Stopping, SendingDone);
signal TransmitState: TransmitState_t;
type ReceiveState_t is (Idle, ReceiveData, ReceiveParity, ReceiveStopbit, ReceivedStopbits, move);
signal ReceiveState: ReceiveState_t;
signal Detect, shotT, halfbitT, shotR, halfbitR, Sample, baudrateset, unlock: std_ulogic;
signal Divisor: std_ulogic_vector(23 downto 0);
signal CountT, CountR: unsigned(Divisor'high+1 downto 0);
signal Count0: unsigned(Divisor'range);
signal ToTransmit: std_ulogic_vector(9 downto 0);
signal Received: std_ulogic_vector(8 downto 0);
signal ReadPort: std_ulogic_vector(8 downto 0);
signal WritePort: std_ulogic_vector(8 downto 0);
signal escapech: std_ulogic_vector(8 downto 0);
signal xonch: std_ulogic_vector(8 downto 0);
signal xoffch: std_ulogic_vector(8 downto 0);
signal Parity, Stopbits: std_ulogic_vector(1 downto 0);
signal DSR, WordLength: std_ulogic;
signal status: std_ulogic_Vector(4 downto 0);
signal escaped, xon, xoff, xonsent, xoffsent: std_ulogic;
signal sput, sget, snotFull, sAlmostFull, sEmpty, rget, rput, rnotFull, rAlmostFull, rEmpty: std_ulogic;

  component fifo is
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
  end component fifo;
  
  for rfifo:fifo
    use entity work.fifo(Rtl)
	 generic map(WordLength => WordLength,
	             ldFiFoSize => ldFiFoSize)
	 port map(nReset => nReset,
	          Clock => Clock,
				 put => put,
				 get => get,
				 DataIn => DataIn,
				 DataOut => DataOut,
				 notFull => notFull,
				 AlmostFull => AlmostFull,
				 Empty => Empty);
  
  for sfifo:fifo
    use entity work.fifo(Rtl)
	 generic map(WordLength => WordLength,
	             ldFiFoSize => ldFiFoSize)
	 port map(nReset => nReset,
	          Clock => Clock,
				 put => put,
				 get => get,
				 DataIn => DataIn,
				 DataOut => DataOut,
				 notFull => notFull,
				 AlmostFull => AlmostFull,
				 Empty => Empty);
				 
begin

  ReadyT <= sget and not sAlmostFull;
  ReadyR <= rput;
  
  sfifo: component fifo
	 generic map(WordLength => 9,
	             ldFiFoSize => 3)
	 port map(nReset => nReset,
	          Clock => Clk,
				 put => sput,
				 get => sget,
				 DataIn => DataIn(Writeport'range),
				 DataOut => Writeport,
				 notFull => snotFull,
				 AlmostFull => sAlmostFull,
				 Empty => sEmpty);

  sput <= '1' when Writes = '1' and Address = "000" else
          '0';
			 
  rfifo: component fifo
	 generic map(WordLength => 9,
	             ldFiFoSize => 3)
	 port map(nReset => nReset,
	          Clock => Clk,
				 put => rput,
				 get => rget,
				 DataIn => Received,
				 DataOut => Readport,
				 notFull => rnotFull,
				 AlmostFull => rAlmostFull,
				 Empty => rEmpty);
  
  rget <= '1' when Reads = '1' and Address = "000" else
          '0';
			 
  RTS <= baudrateset nand rnotFull;

  --DTR <= status(2);
  DSR <= not RxD;

  status(0) <= '1' when ReceiveState /= Idle else
               '0';
  status(1) <= not rEmpty;
  status(2) <= '1' when TransmitState /= Idle else
               '0';
  status(3) <= snotFull;
  status(4) <= baudrateset;

  DataOut <= (DataOut'high downto escapech'high + 1 => '0') & escapech when Address = "100" else
             (DataOut'high downto xonch'high + 1 => '0') & xonch when Address = "101" else
             (DataOut'high downto xoffch'high + 1 => '0') & xoffch when Address = "110" else
             (DataOut'high downto ReadPort'high + 1 => '0') & ReadPort when Address = "000" else
             (DataOut'high downto 5 => '0') & WordLength & Stopbits & Parity when Address = "001" else
				 Divisor when Address = "010" else
				 (DataOut'high downto status'high + 1 => '0') & status when Address = "011" else
				 (others => '0');

  --
  -- Transmiter
  --
  
  DivideT: process(nReset, Clk, Divisor, CountT, TransmitState, halfbitT, Writes)
  variable x: unsigned(CountT'range);
  begin
    if nReset /= '1' then
	   escapech <= (others => '1');
	   xonch <= (others => '1');
 	   xoffch <= (others => '1');
      CountT <= (others => '0');
	   shotT <= '0';
	   Divisor <= (0 => '1', others => '0');
	   Parity <= "00";
	   StopBits <= "01";
	   WordLength <= '1';
		baudrateset <= '0';
		unlock <= '0';
    elsif rising_edge(Clk) then
	   if Writes = '1' then
		  if Address = "001" then
	       -- set/get frame informations
	       Parity <= DataIn(1 downto 0);
		    Stopbits <= DataIn(3 downto 2);
		    WordLength <= DataIn(4);
			 unlock <= '1';
        elsif Address = "010" and unlock = '1' then
	       -- set/get divisor
	       Divisor <= DataIn;
			 baudrateset <= '1';
		  elsif Address = "100" then
		    escapech <= DataIn(escapech'range);
		  elsif Address = "101" then
		    xonch <= DataIn(xonch'range);
		  elsif Address = "110" then
		    xoffch <= DataIn(xoffch'range);
        end if;
	   end if;

		x := CountT - 1;

		if TransmitState = Idle then
		  CountT <= unsigned(Divisor & '0');
		  shotT <= '0';
		elsif x = 0 then
	     -- frame for next bit starts
		  shotT <= '1';
	     -- force loading of counter
	     if halfbitT = '1' then
	       -- a half bit only
	       CountT(Divisor'range) <= unsigned(Divisor);
	     else
	       -- bit with full length
	       CountT <= unsigned(Divisor & '0');
	     end if;
      else
	     CountT <= x;
	     shotT <= '0';
      end if;
    end if;
  end process DivideT;

  Transmiter: process (nReset, Clk)
  variable a, b, trigger, newon, newoff : std_ulogic;
  variable got: std_ulogic_vector(WritePort'range);
  begin
    if nReset /= '1' then
      TransmitState <= Idle;
      TxD <= '1';
	   halfbitT <= '0';
		xonsent <= '1';
		xoffsent <= '0';
		escaped <= '0';
		ToTransmit <= (others => '0');
    elsif rising_edge(Clk) then
	   got := WritePort;
		newon := xonsent;
		newoff := xoffsent;
		trigger := '0';
		b := '0';
		
      case TransmitState is
        when Idle =>
		    if CToS = '0' then
			   if escaped = '1' then
				  if sEmpty = '0' then
				    escaped <= '0';
		          trigger := '1';
					 b := '1';
				  end if;
            else
			     if xon = '1' and xonsent = '0' then
			       got := xonch;
			       b := '1';
			       newon := '1';
			       newoff := '0';
			     elsif xoff = '1' and xoffsent = '0' then
			       got := xoffch;
			       b := '1';
			       newon := '0';
			       newoff := '1';
				  elsif sEmpty = '0' then
				    if got(got'high) = '1' and escapech(escapech'high) = '0' then
				      got := escapech;
				      b := '1';
				      escaped <= '1';
					 else
		            trigger := '1';
				      b := '1';
					 end if;
			     end if;
			   end if;
            if b = '1' then
              -- prepare input for shifting
		        a := ((((got(7) and WordLength) xor got(6)) xor (got(5) xor got(4))) xor ((got(3) xor got(2)) xor
		                   (got(1) xor got(0))));
              if Parity(0) = '1' then
		          -- odd parity
		          a := not a;
              elsif Parity(1) = '0' then
		          -- no parity
		          a := '1';
              end if;
		        -- save inverted data to shift register
		        if WordLength = '1' then
                ToTransmit <= (Parity(0) or Parity(1)) & a &  got(7 downto 0);
              else
                ToTransmit <= '0' & (Parity(0) or Parity(1)) & a & got(6 downto 0);
		        end if;
              TransmitState <= Sending;
		        -- send startbit
              TxD <= '0';
            end if;
          end if;
        when Sending =>
          if ShotT = '1' then
		      -- shift least significant bit out
		      TxD <= ToTransmit(0);
		      ToTransmit <= '0' & ToTransmit(9 downto 1);
            if unsigned(ToTransmit(9 downto 1)) = 0 then
		        -- prepare for stopbits
		        if Stopbits(0) = '1' then
		          -- only one
                TransmitState <= SendingDone;
		        else
                TransmitState <= Stopping;
		          halfbitT <= stopbits(1);
		        end if;
		      end if;
          end if;
        when Stopping =>
          if ShotT = '1' then
		      -- initiate last stopbit
            TransmitState <= SendingDone;
		      halfbitT <= '0';
          end if;
        when SendingDone =>
          if ShotT = '1' then
		      -- stopbits still sent
            TransmitState <= Idle;
          end if;
      end case;
		
		sget <= trigger;
		xonsent <= newon;
		xoffsent <= newoff;
    end if;
  end process Transmiter;

  --
  -- Receiver
  --

  Sampler: process(nReset, Clk, Divisor)
  variable y: unsigned(Count0'range);
  variable x: unsigned(CountR'range);
  variable b: unsigned(0 to 0);
  begin
    if nReset /= '1' then
      shotR <= '0';
      Sample <= '1';
		Detect <= '0';
      CountR <= (others => '0');
      Count0 <= (others => '0');
    elsif rising_edge(Clk) then

	   if ReceiveState = Idle and Detect = '0' and shotR = '0' then
		  x := unsigned(Divisor & '0');
		  y := unsigned(Divisor);
		  Detect <= DSR;
		else
		  x := CountR;
		  y := Count0;
		end if;

		x := x - 1;
		if y /= 0 then
		  b(0) := not RxD;
		else
		  b(0) := '0';
      end if;
		y := y - b; 

		if x = 0 then
	     shotR <= '1';
		  Detect <= '0';
		  if y = 0 then
	       Sample <= '0';
		  else
	       Sample <= '1';
		  end if;
		  if halfbitR = '1' then
		    x := unsigned('0' & Divisor);
		    y := unsigned('0' & Divisor(Divisor'high downto 1));
		  else
		    x := unsigned(Divisor & '0');
		    y := unsigned(Divisor);
		  end if;
		else
	     shotR <= '0';
		end if;

		Count0 <= y;
		CountR <= x;
    end if;
  end process Sampler;

  Receiver: process (nReset, Clk)
  variable a: std_ulogic;
  begin
    if nReset /= '1' then
	   halfbitR <= '0';
	   ReceiveState <= Idle;
		xon <= '1';
		xoff <= '0';
		Received <= (others => '0');
		rput <= '0';
    elsif rising_edge(Clk) then
      case ReceiveState is
        when Idle =>
		    rput <= '0';
			 
	       -- fifo ready?
		    if rAlmostFull = '1' then
		      xoff <= not xoffch(xoffch'high);
		      xon <= xoffch(xoffch'high);
			 else
			   xon <= '1';
				xoff <= '0';
		    end if;

	       -- receiving startbit
	       if shotR = '1' and Sample = '0' then
		      ReceiveState <= ReceiveData;
		      if WordLength = '1' then
		        Received <= (6 downto 0 => '1', others => '0');
            else
		        Received <= (5 downto 0 => '1', others => '0');
            end if;
          end if;
        when ReceiveData =>
	       -- receive data and parity
          if shotR = '1' then
		      if WordLength = '1' then
		        Received(7 downto 0) <= Sample & Received(7 downto 1);
            else
		        Received(6 downto 0) <= Sample & Received(6 downto 1);
            end if;
		      if Received(0) = '0' then
		        if Parity = "00" then
                if stopbits(0) = '1' then
		            ReceiveState <= ReceivedStopbits;
		          else
		            ReceiveState <= ReceiveStopbit;
			       end if;
              else
		          ReceiveState <= ReceiveParity;
		        end if;
		      end if;
          end if;
        when ReceiveParity =>
	       -- receive parity
		    if shotR = '1' then
            if stopbits(0) = '1' then
		        ReceiveState <= ReceivedStopbits;
            else
		        halfbitR <= stopbits(1);
		        ReceiveState <= ReceiveStopbit;
            end if;
				a := Sample;
				for i in 0 to 7 loop
				  a := a xor Received(i);
				end loop;
		      Received(8) <= a xor Parity(0);
		    end if;
        when ReceiveStopbit =>
	       -- first stopbit received
          if shotR = '1' then
		      ReceiveState <= ReceivedStopbits;
		      halfbitR <= '0';
				if Sample = '0' then
				  Received(8) <= '1';
				end if;
          end if;
        when ReceivedStopbits =>
	       -- second stopbit
          if shotR = '1' then
		      -- datum ready to be read
		      ReceiveState <= move;
				if Sample = '0' then
				  Received(8) <= '1';
				end if;
          end if;
		  when move =>
		    rput <= '1';
		    ReceiveState <= Idle;
      end case;
    end if;
  end process Receiver;

end Rtl;
