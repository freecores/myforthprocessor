----------------------------------------------------------------------------------
-- Company: RIIC
-- Engineer: Gerhard Hohner Mat.nr.: 7555111
-- 
-- Create Date:    01/07/2004 
-- Design Name:    Diplomarbeit
-- Module Name:    ALU - Rtl 
-- Project Name:   32 bit FORTH processor
-- Target Devices: Spartan 3
-- Tool versions:  ISE 8.2
-- Description: implements a 32 bit ALU
-- Dependencies: global.vhd
-- 
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------


library IEEE, work;
use IEEE.std_logic_1164.all;
--use IEEE.numeric_std.all;
--use IEEE.std_logic_unsigned.all;
use IEEE.std_logic_arith.all;
use work.global.all;

entity ALU is
	port (nReset: in std_ulogic;					    -- reset
	      Clock: in std_ulogic;                   -- clock
	      SideA: in DataVec;						    -- first Operand
		   SideB: in DataVec;						    -- second Operand
		   AluResult: out DataVec;					    -- result
		   CarryIn: in std_ulogic;					    -- Carry in
		   CarryOut: out std_ulogic;			       -- Carry out
		   AluFunc: in AluFuncType; 				    -- Opcode
			Fwait: out std_ulogic);
end entity ALU;

architecture Rtl of ALU is
type state_t is (init, ld4, ld3, ld2, ld1, prepare, divi, divf, finish);
signal stated: state_t;
subtype shiftWidth is integer range -32 to 31;
type rarray is array (0 to 3) of DataVec;
type carray is array (0 to 3) of std_ulogic;
signal result: rarray;
signal presult: rarray;
signal carry: carray;
signal pindex: unsigned(1 downto 0);
signal quotientf, quotient, OperandA, OperandB: std_ulogic_vector(63 downto 0);
signal ha32, hb32: std_ulogic_vector(31 downto 0);
signal flag, qbit, ClkDiv: std_ulogic;
signal Opready: std_ulogic_vector(2 downto 0);
signal product: unsigned(127 downto 0);
signal counter, ldA, ldB: unsigned(5 downto 0);
signal diffexp: unsigned(6 downto 0);
signal dividend, divisor: unsigned(OperandB'range);
signal switch: std_ulogic_vector(1 downto 0);

begin
  
  Fwait <= '1' when unsigned(Opready) /= 0 else '0';
  
  op: process(nReset, Clock) is
  variable ready: std_ulogic_vector(2 downto 0);
  begin
    if nReset /= '1' then
	   switch <= (others => '0');
		ClkDiv <= '0';
		Opready <= (others => '0');
		pindex <= (others => '1');
		OperandA <= (others => '1');
		OperandB <= (others => '1');
	 elsif rising_edge(Clock) then
	   ready := Opready;
		
		if unsigned(Opready) /= 0 then
		  if stated = finish then
	       ready := (others => '0');
		  end if;
	   elsif AluFunc = Op64 then
		  OperandA <= OperandB;
		  OperandB <= SideA & SideB;
	   elsif AluFunc = FPUcode then
		  pindex(0) <= '1';
		  pindex(1) <= '1';
		  case SideA(1 downto 0) is
		    when "01" =>
			   ready(0) := '1';
		    when "10" =>
			   ready(1) := '1';
		    when "11" =>
			   ready(2) := '1';
		    when others =>
	         ready := (others => '0');
		  end case;
	     switch <= SideA(1 downto 0);
	   elsif AluFunc = Prod32 then
		  pindex <= pindex - 1;
		end if;
	
      if Opready(0) = '1' then
        presult(0) <= std_ulogic_vector(product(31 downto 0));
        presult(1) <= std_ulogic_vector(product(63 downto 32));
        presult(2) <= std_ulogic_vector(product(95 downto 64));
        presult(3) <= std_ulogic_vector(product(127 downto 96));
      elsif Opready(1) = '1' then
        presult(0) <= std_ulogic_vector(quotient(31 downto 0));
        presult(1) <= std_ulogic_vector(quotient(63 downto 32));
        presult(2) <= std_ulogic_vector(dividend(31 downto 0));
        presult(3) <= std_ulogic_vector(dividend(63 downto 32));
      elsif Opready(2) = '1' then
        presult(0) <= std_ulogic_vector((31 downto diffexp'high + 1 => std_ulogic(diffexp(diffexp'high))) & diffexp);
        presult(1) <= (others => std_ulogic(diffexp(diffexp'high)));
        presult(2) <= std_ulogic_vector(quotientf(31 downto 0));
        presult(3) <= std_ulogic_vector(quotientf(63 downto 32));
		end if;
				
		ClkDiv <= not ClkDiv;
		Opready <= ready;
	 end if;
  end process op;
  
  multiply: process(nReset, ClkDiv)
  variable diff, div: unsigned(OperandA'length downto 0);
  variable ldDiff: unsigned(ldB'length downto 0);
  variable lastbit, x: std_ulogic;
  variable shifted: unsigned(divisor'range);
  variable a: unsigned(0 downto 0);
  begin
    if nReset /= '1' then
	   stated <= init;
         qbit <= '0';
		--for i in presult'range loop presult(i) <= (others => '0'); end loop;
	 elsif rising_edge(ClkDiv) then
         if qbit = '1' then
           div := unsigned(not std_ulogic_vector('0' & divisor));
         else
           div := unsigned('0' & divisor);
         end if;

         a(0) := qbit;
         diff := unsigned('0' & dividend) + div + a;
         lastbit := qbit xor diff(diff'high);

	   case stated is
		  when init =>
		    if Opready(0) = '1' then
	         product <= unsigned(OperandA) * unsigned(OperandB);
            stated <= finish;
		    elsif unsigned(Opready) /= 0 then
			   if unsigned(OperandB(63 downto 32)) /= 0 then
				  hb32 <= OperandB(63 downto 32);
				  ldB(5) <= '1';
				else
				  hb32 <= OperandB(31 downto 0);
				  ldB(5) <= '0';
				end if;
				
			   if unsigned(OperandA(63 downto 32)) /= 0 then
				  ha32 <= OperandA(63 downto 32);
				  ldA(5) <= '1';
				else
				  ha32 <= OperandA(31 downto 0);
				  ldA(5) <= '0';
				end if;
				
				divisor <= unsigned(OperandA);
				dividend <= unsigned(OperandB);
				
				stated <= ld4;
			 end if;
		  when ld4 =>
			 if unsigned(hb32(31 downto 16)) /= 0 then
			   hb32(15 downto 0) <= hb32(31 downto 16);
			   ldB(4) <= '1';
			 else
			   ldB(4) <= '0';
			 end if;
				
			 if unsigned(ha32(31 downto 16)) /= 0 then
			   ha32(15 downto 0) <= ha32(31 downto 16);
			   ldA(4) <= '1';
			 else
			   ldA(4) <= '0';
			 end if;
				
			 stated <= ld3;
		  when ld3 =>
			 if unsigned(hb32(15 downto 8)) /= 0 then
			   hb32(7 downto 0) <= hb32(15 downto 8);
			   ldB(3) <= '1';
			 else
			   ldB(3) <= '0';
			 end if;
				
			 if unsigned(ha32(15 downto 8)) /= 0 then
			   ha32(7 downto 0) <= ha32(15 downto 8);
			   ldA(3) <= '1';
			 else
			   ldA(3) <= '0';
			 end if;
				
			 stated <= ld2;
		  when ld2 =>
			 if unsigned(hb32(7 downto 4)) /= 0 then
			   hb32(3 downto 0) <= hb32(7 downto 4);
			   ldB(2) <= '1';
			 else
			   ldB(2) <= '0';
			 end if;
				
			 if unsigned(ha32(7 downto 4)) /= 0 then
			   ha32(3 downto 0) <= ha32(7 downto 4);
			   ldA(2) <= '1';
			 else
			   ldA(2) <= '0';
			 end if;
				
			 stated <= ld1;
		  when ld1 =>
			 if unsigned(hb32(3 downto 2)) /= 0 then
			   --hb32(1 downto 0) <= hb32(3 downto 2);
			   ldB(1) <= '1';
		      ldB(0) <= hb32(3);
			 else
			   ldB(1) <= '0';
		      ldB(0) <= hb32(1);
			 end if;
				
			 if unsigned(ha32(3 downto 2)) /= 0 then
			   --ha32(1 downto 0) <= ha32(3 downto 2);
			   ldA(1) <= '1';
		      ldA(0) <= ha32(3);
			 else
			   ldA(1) <= '0';
		      ldA(0) <= ha32(1);
			 end if;
				
			 stated <= prepare;
		  when prepare =>
		    ldDiff := unsigned('0' & ldB) - unsigned('0' & ldA);
			 diffexp <= ldDiff;
			 if (ldB = 0 and dividend(0) = '0') or (ldA = 0 and divisor(0) = '0') then
			  stated <= finish;
			 elsif ldDiff(ldDiff'high) = '0' then
			   shifted := SHL(divisor, ldDiff(ldDiff' high - 1 downto 0));
				if Opready(1) = '1' then
				  divisor <= shifted;
				  counter <= ldDiff(ldDiff' high - 1 downto 0);
				  stated <= divi;
				else
				  divisor <= shifted;
				  counter <= to_unsigned(53, counter'length);
				  stated <= divf;
				end if;
			 else
			   ldDiff := 0 - ldDiff;
				if Opready(1) = '1' then
			     --divisor <= unsigned(OperandA);
				  counter <= ldDiff(ldDiff' high - 1 downto 0);
				  stated <= finish;
				else
			     dividend <= SHL(dividend, ldDiff(ldDiff' high - 1 downto 0));
				  counter <= to_unsigned(53, counter'length);
				  stated <= divf;
				end if;
			 end if;
			 quotient <= (others => '0');
			 quotientf <= (others => '0');
			 qbit <= '1';
			 flag <= '0';
		  when divi =>
			 dividend <= diff(dividend'range);
			 qbit <= lastbit;
			 
			 if counter = 0 then
				if flag = '0' and lastbit = '0' then
			     flag <= '1';
				else
				  stated <= finish;
				end if;
			 else
			   counter <= counter - 1;
				divisor <= unsigned('0' & divisor(divisor'high downto 1));
			 end if;
			 if flag = '0' then
			   quotient <= quotient(quotient'high - 1 downto 0) & lastbit;
			 end if;
		  when divf =>
			 dividend <= unsigned(diff(dividend'high - 1 downto 0) & '0');
			 x := flag or lastbit;
			 flag <= x;
			 if x = '1' then
			   counter <= counter - 1;
			 else
			   diffexp <= diffexp - 1;
			 end if;
			 quotientf <= quotientf(quotientf'high - 1 downto 0) & lastbit;
			 qbit <= lastbit;
			 
			 if counter = 0 then
			   stated <= finish;
			 end if;
		  when finish =>
		    if unsigned(Opready) = 0 then
            stated <= init;
		    end if;
		end case;
	 end if;
  end process multiply;
  
  arith: process(nReset, SideA, SideB, CarryIn, AluFunc, result, carry) is
  variable op: unsigned(DataVec'high + 2 downto 0);
  variable temp: unsigned(DataVec'high + 2 downto 1);
  begin
    if nReset /= '1' then
	   Carry(0) <= '0';
		result(0) <= (others => '0');
	 else
      op := unsigned('0' & SideB & (CarryIn and AluFunc(0) and not AluFunc(3)));
	 
      -- invert for subtraction
      if AluFunc(1) = '1' or AluFunc(3) = '1' then
        op := unsigned(not std_ulogic_vector(op));
      end if;

      temp := unsigned('0' & SideA) + op(temp'range) + unsigned(op(0 downto 0));
	 
      if AluFunc(2) = '1' or AluFunc(3) = '1' then
	     Carry(0) <= temp(temp'high);
      else
        Carry(0) <= CarryIn;
      end if;

	   result(0) <= DataVec(temp(temp'high - 1 downto 1));
	 end if;
  end process arith;
  
  relop: process(nReset, SideA, CarryIn, AluFunc, carry) is
  variable res, zflag, sign, borrow: std_ulogic;
  begin
    if nReset /= '1' then
	   Carry(1) <= '0';
		result(1) <= (others => '0');
	 else
      borrow := CarryIn;
	   sign := SideA(DataVec'high);
	 
	   zflag := '1';
	   for i in SideA'range loop
	     zflag := zflag and not SideA(i);
	   end loop;
	 
      case AluFunc(2 downto 0) is
      -- <
      when "000" =>
	     res := sign;
      -- >
      when "001" =>
	     res := sign nor zflag;
      -- =
      when "010" =>
	     res := zflag;
      -- <>
      when "011" =>
	     res := not zflag;
      -- U<
      when "100" =>
	     res := borrow;
      -- U>
      when "101" =>
	     res := borrow nor zflag;
      when others =>
		  res := '0';
      end case;

      result(1) <= (others => res);
	   Carry(1) <= CarryIn;
	 end if;
  end process relop;
  
  shifts: process(nReset, SideA, SideB, CarryIn, AluFunc, result, carry) is
  variable temp: unsigned(DataVec'high + 2 downto 0);
  variable a: std_ulogic;
  begin
    if nReset /= '1' then
	   Carry(2) <= '0';
		result(2) <= (others => '0');
	 else
	   a := (CarryIn and AluFunc(0)) or (SideA(SideA'high) and AluFunc(2));
      temp := unsigned(a & SideA & a);
	 
	   if unsigned(SideB(SideB'high downto 5)) /= 0 then
	     if AluFunc(2) /= '1' and AluFunc(1) /= '1' then
          -- <<
  	       Carry(2) <= SideA(0);
        else
          -- >>
	       Carry(2) <= SideA(SideA'high);
        end if;
        result(2) <= (others => '0');
      else
	     if AluFunc(2) /= '1' and AluFunc(1) /= '1' then
          -- <<
		    temp := SHL(temp, unsigned(SideB(4 downto 0)));
  	       Carry(2) <= temp(temp'high);
        else
          -- >>
		    temp := SHR(temp, unsigned(SideB(4 downto 0)));
	       Carry(2) <= temp(0);
        end if;
        result(2) <= DataVec(temp(DataVec'high + 1 downto 1));
	   end if;
	 end if;
  end process shifts;
	
  logic: process(nReset, SideA, SideB, CarryIn, AluFunc, result, carry) is
  variable a, b: unsigned(17 downto 0);
  variable c: unsigned(35 downto 0);
  begin
    if nReset /= '1' then
	   Carry(3) <= '0';
		result(3) <= (others => '0');
	 else
	   Carry(3) <= '0';
	   case AluFunc(2 downto 0) is
      -- and
	   when "000" =>
	     result(3) <= SideA and SideB;
		-- multiply
		--when "001" =>
		--  a(17 downto 16) := (others => '0');
		--  a(15 downto 0) := unsigned(SideA(15 downto 0));
		--  b(17 downto 16) := (others => '0');
		--  b(15 downto 0) := unsigned(SideB(15 downto 0));
		--  c := a * b;
		--  result(3) <= DataVec(c(DataVec'range));
	   -- or
	   when "010" =>
	     result(3) <= SideA or SideB;
	   -- xor
	   when others =>
	     result(3) <= SideA xor SideB;
      end case;
	 end if;
  end process logic;
  
  AluResult <= (others => '0') when nReset /= '1' else
               --result(to_integer(unsigned(AluFunc(4 downto 3))));
					presult(to_integer(std_ulogic_vector(pindex))) when AluFunc = Prod32 else
               result(to_integer(std_ulogic_vector(AluFunc(4 downto 3))));
  CarryOut <= '0' when nReset /= '1' else
              --carry(to_integer(unsigned(AluFunc(4 downto 3))));
				  CarryIn when AluFunc = Op64 or AluFunc = Prod32 or AluFunc = FPUcode else
              carry(to_integer(std_ulogic_vector(AluFunc(4 downto 3))));
  
end architecture Rtl;

