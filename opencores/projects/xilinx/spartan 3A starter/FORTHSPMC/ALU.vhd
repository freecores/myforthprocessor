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


library IEEE, work, UNISIM;
use IEEE.std_logic_1164.all;
--use IEEE.numeric_std.all;
--use IEEE.std_logic_unsigned.all;
use IEEE.std_logic_arith.all;
use work.global.all;
use UNISIM.VCOMPONENTS.all;

entity ALU is
   generic (constant Cores : integer := 2);
	port (nReset: in std_ulogic;					    -- reset
	      Clock: in std_ulogic;                   -- clock
			Core: in std_ulogic_vector(Cores-1 downto 0);
	      SideA: in DataVec;						    -- first Operand
		   SideB: in DataVec;						    -- second Operand
		   AluResult: out DataVec;					    -- result
		   CarryIn: in std_ulogic;					    -- Carry in
		   CarryOut: out std_ulogic;			       -- Carry out
		   AluFunc: in AluFuncType; 				    -- Opcode
			Fwait: out std_ulogic);
end entity ALU;

architecture Rtl of ALU is
type state_t is (init, finish);
signal stated: state_t;
subtype shiftWidth is integer range -32 to 31;
type rarray is array (0 to 3) of DataVec;
type carray is array (0 to 3) of std_ulogic;
signal presult, result: rarray;
signal carry: carray;
signal ClkDiv, half: std_ulogic;
signal corei: std_ulogic_vector(core'range);
type iarray is array (0 to 2 ** Cores - 1) of unsigned(1 downto 0);
signal pindex: iarray;
type oarray is array (0 to 2 ** Cores - 1) of std_ulogic_vector(63 downto 0);
signal OperandA, OperandB: oarray;
type sarray is array (0 to 2 ** Cores - 1) of std_ulogic;
signal haveone, Opready: sarray;
type parray is array (0 to 2 ** Cores - 1) of unsigned(127 downto 0);
signal product: parray;

begin
  
  Fwait <= Opready(to_integer(corei));
  
  op: process(nReset, Clock) is
  variable ready: std_ulogic;
  begin
    if nReset /= '1' then
		ClkDiv <= '0';
		corei <= (others => '0');
		for i in parray'range loop
		  haveone(i) <= '0';
		  Opready(i) <= '0';
		  pindex(i) <= (others => '1');
		  OperandA(i) <= (others => '1');
		  OperandB(i) <= (others => '1');
		end loop;
	 elsif rising_edge(Clock) then
	   ready := Opready(to_integer(core));
		
		if Opready(to_integer(corei)) = '1' or ready = '1' then
		  if stated = finish then
	       ready := '0';
		    Opready(to_integer(corei)) <= '0';
		  end if;
	   elsif AluFunc = Op64 then
		  OperandA(to_integer(core)) <= OperandB(to_integer(core));
		  OperandB(to_integer(core)) <= SideA & SideB;
		  ready := haveone(to_integer(core));
		  haveone(to_integer(core)) <= not haveone(to_integer(core));
		  corei <= core;
	   elsif AluFunc = Prod32 then
		  pindex(to_integer(core)) <= pindex(to_integer(core)) - 1;
		end if;
  
      presult(0) <= std_ulogic_vector(product(to_integer(core))(31 downto 0));
      presult(1) <= std_ulogic_vector(product(to_integer(core))(63 downto 32));
      presult(2) <= std_ulogic_vector(product(to_integer(core))(95 downto 64));
      presult(3) <= std_ulogic_vector(product(to_integer(core))(127 downto 96));
		
		ClkDiv <= not ClkDiv;
		Opready(to_integer(core)) <= ready;
	 end if;
  end process op;
  
  multiply: process(nReset, ClkDiv)
  begin
    if nReset /= '1' then
	   stated <= init;
		--for i in presult'range loop presult(i) <= (others => '0'); end loop;
	 elsif rising_edge(ClkDiv) then
	   case stated is
		  when init =>
		    if Opready(to_integer(corei)) = '1' then
	         product(to_integer(corei)) <= unsigned(OperandA(to_integer(corei))) * unsigned(OperandB(to_integer(corei)));
            stated <= finish;
			 end if;
		  when finish =>
		    if Opready(to_integer(corei)) = '0' then
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
					presult(to_integer(std_ulogic_vector(pindex(to_integer(core))))) when AluFunc = Prod32 else
               result(to_integer(std_ulogic_vector(AluFunc(4 downto 3))));
  CarryOut <= '0' when nReset /= '1' else
              --carry(to_integer(unsigned(AluFunc(4 downto 3))));
				  CarryIn when AluFunc = Op64 or AluFunc = Prod32 else
              carry(to_integer(std_ulogic_vector(AluFunc(4 downto 3))));
  
end architecture Rtl;
