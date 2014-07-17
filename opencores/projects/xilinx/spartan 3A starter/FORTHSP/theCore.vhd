----------------------------------------------------------------------------------
-- Company: RIIC
-- Engineer: Gerhard Hohner Mat.nr.: 7555111
-- 
-- Create Date:    01/07/2004 
-- Design Name:    Diplomarbeit
-- Module Name:    theCore - Rtl 
-- Project Name:   32 bit FORTH processor
-- Target Devices: Spartan 3
-- Tool versions:  ISE 8.2
-- Description: implement decoder and integrate all units building the core
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
use IEEE.std_logic_arith.all;
use work.global.all;

entity theCore is
  generic (constant CacheIndexBitWidth: integer := 10;						-- ld(Cachesize)
           constant TableBitWidth : integer := 4);								-- ld(Interrupttable)
  port (nReset: in std_ulogic;														-- system reset
        Clock: in std_ulogic;															-- system clock
		  abortload: out std_ulogic;                                      -- abort memory cycle
		  MemStall: in std_ulogic;                                        -- memory stall
	     DataIn: in DataVec;															-- incoming data
	     ReadBlocked: in std_ulogic;													-- Code fetch blocked
	     DataROM: in std_ulogic_vector(DataVec'range);							-- incomming ROM code
	     AddressROM: out std_ulogic_vector(RAMrange'high + 1 downto 2);	-- ROM address
	     Reads: out std_ulogic;														-- read ROM
	     Address: out std_ulogic_vector(RAMrange'high + 1 downto 0);		-- memory address
	     DataOut: out DataVec;															-- data to memory
	     IOCode: out std_ulogic_vector(2 downto 0);		          			-- memory operation
		  illegal: out std_ulogic;														-- illegal opcode
		  Token: out std_ulogic;                                          -- service routine running
	     Quit: out std_ulogic;															-- vector processed
	     Vector: in std_ulogic_vector(TableBitWidth - 1 downto 0);			-- interrupt vector
	     VectorValid: in std_ulogic);												-- vector valid
end theCore;

architecture Rtl of theCore is

  type decoderec is record
    Func : StackFuncType;
    toPop : std_ulogic_vector(1 downto 0);
    SelectSource : std_ulogic_vector(3 downto 0);
    SelectAlu : InputOrder_t;
    AFunc : AluFuncType;
	 Im : DataVec;
  end record;

  signal Opcode: std_ulogic_vector(7 downto 0);
  signal toPop: std_ulogic_vector(1 downto 0);
  signal IOCodeStack: std_ulogic_vector(2 downto 0);
  signal SelectSource: std_ulogic_vector(3 downto 0);
  signal SelectedStack: BinRange;
  signal TOS, Im, DataOutStack, Immediate, First: DataVec;
  signal Target: std_ulogic_vector(Address'range);
  signal TOSValid, Done, Ready: std_ulogic;
  signal Func: StackFuncType;
  signal SelectAlu: InputOrder_t;
  signal AFunc: AluFuncType;

  component theStacks is
   generic (constant CacheIndexBitWidth: integer);                      -- ld(cachesize)
	port (nReset: in std_ulogic;						          					-- Reset
	      Clock: in std_ulogic;						         					-- clock signal
		   MemStall: in std_ulogic;                                       -- memory stall
		   Pop: in std_ulogic_vector(1 downto 0);								   -- pop count
		   SelectSource: in std_ulogic_vector(3 downto 0);						-- select a visible element of the stacks
		   SelectedStack: in BinRange;												-- selects the target stack
	      DataIn: in DataVec;						          						-- datum entering
	      DataOut: out DataVec;					             					-- datum leaving
		   Immediate: in DataVec;						         					-- immediate value
		   Target: out std_ulogic_vector(RAMrange'high + 1 downto 0);		-- memmory address
		   IOCode: out std_ulogic_vector(2 downto 0);		        			-- memory operation
		   TOSValid: out std_ulogic;													-- return address is on top of stack
		   Ready: out std_ulogic;						      					   -- stack ready for next operation
		   Func: in StackFuncType;		              								-- Opcode
		   First: out DataVec;															-- first operand on stack
		   AFunc: in AluFuncType;														-- Opcode ALU
			SelectALU: in InputOrder_t);										     	-- input selection for ALU
  end component theStacks;

  for myStacks:theStacks
	use entity work.theStacks(Rtl)
	   generic map(CacheIndexBitWidth => CacheIndexBitWidth)
		port map(nReset => nReset,
		         Clock => Clock,
		         Pop => Pop,
					MemStall => MemStall,
			      SelectSource => SelectSource,
			      SelectedStack => SelectedStack,
			      DataIn => DataIn,
			      DataOut => DataOut,
			      Immediate => Immediate,
			      Target => Target,
			      IOCode => IOCode,
			      TOSValid => TOSValid,
			      Ready => Ready,
			      Func => Func,
		         First => First,
		         AFunc => AFunc,
		         SelectALU => SelectALU);

  component ProgramCounter is
  generic (constant TableBitWidth : integer);
  port (nReset: in std_ulogic;													-- system reset
        Clock: in std_ulogic;														-- system clock
		  abortload: out std_ulogic;                                   -- abort current fetch cycle
	     ReadBlocked: in std_ulogic;												-- fetch cycle delayed
	     Opcode: out std_ulogic_vector(7 downto 0);							-- coded operation
	     DataIn: in DataVec;														-- incoming data
	     Address: out std_ulogic_vector(RAMrange'high + 1 downto 2);	-- memory address
	     Immediate: out DataVec;													-- immediate operand
	     Reads: out std_ulogic;		         									-- memory operation
	     Done: in std_ulogic;						       				      -- core ready for next operation
	     TOS: in DataVec;															-- return address
        TOSValid: in std_ulogic;													-- return address available
		  Token: out std_ulogic;
	     Quit: out std_ulogic;														-- vector processed
	     Vector: in std_ulogic_vector(TableBitWidth - 1 downto 0);		-- interrupt vector
	     VectorValid: in std_ulogic);
  end component ProgramCounter;

  for myProgramCounter:ProgramCounter
	use entity work.ProgramCounter(Rtl)
	   generic map(TableBitWidth => TableBitWidth)
		port map(nReset => nReset,
		         Clock => Clock,
					abortload => abortload,
		         ReadBlocked => ReadBlocked,
			      Opcode => Opcode,
			      DataIn => DataIn,
			      Address => Address,
			      Immediate => Immediate,
			      Reads => Reads,
			      Done => Done,
			      TOS => TOS,
			      TOSValid => TOSValid,
					Token => Token,
			      Quit => Quit,
			      Vector => Vector,
			      VectorValid => VectorValid);

begin
	myStacks: component	theStacks
	   generic map(CacheIndexBitWidth => CacheIndexBitWidth)
		port map(nReset => nReset,
		         Clock => Clock,
					MemStall => MemStall,
		         Pop => toPop,
			      SelectSource => SelectSource,
			      SelectedStack => SelectedStack,
			      DataIn => DataIn,
			      DataOut => DataOutStack,
			      Immediate => Immediate,
			      Target => Target,
			      IOCode => IOCodeStack,
			      TOSValid => TOSValid,
			      Ready => Ready,
			      Func => Func,
		         First => First,
		         AFunc => AFunc,
		         SelectALU => SelectALU);

	myProgramCounter: component ProgramCounter
      generic map (TableBitWidth => TableBitWidth)
		port map(nReset => nReset,
		         Clock => Clock,
					abortload => abortload,
		         ReadBlocked => ReadBlocked,
			      Opcode => Opcode,
			      DataIn => DataROM,
			      Address => AddressROM,
			      Immediate => Im,
			      Reads => Reads,
			      Done => Done,
			      TOS => TOS,
			      TOSValid => TOSValid,
					Token => Token,
			      Quit => Quit,
			      Vector => Vector,
			      VectorValid => VectorValid);
  
  IOCode <= IOCodeStack;

  DataOut <= DataOutStack;

  Address <= Target;

  Done <= Ready;																	-- stack ready

  TOS <= First;

  Decode: process(nReset, Clock) is
  variable rec: decoderec;
  variable a: unsigned(0 downto 0);
  variable ill: std_ulogic;
  begin
    if nReset /= '1' then
	   Func <= NopStack;
	   toPop <= (others => '0');
	   SelectSource <= "0100";
	   AFunc <= AddALUnC;
	   SelectAlu <= ImIm;
	   SelectedStack <= 0;
		Immediate <= (others => '0');
		illegal <= '0';
    elsif rising_edge(Clock) then
      ill := '0';	 
	   if Ready = '1' then
	     a(0) := Opcode(7);

	     case Opcode(6 downto 0) is
	     when opcSTORE =>
          rec := (LoadStore, "10", "0111", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- STORE
        when opcLOAD =>
          rec := (LoadStore, "00", "0011", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- LOAD
        when opcSTOREC =>
          rec := (LoadStore, "10", "0101", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- STOREC
        when opcLOADC =>
          rec := (LoadStore, "00", "0001", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- LOADC
        when opcSTOREH =>
          rec := (LoadStore, "10", "0110", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- STOREH
        when opcLOADH =>
          rec := (LoadStore, "00", "0010", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- LOADH
        when opcDEPTH =>
          rec := (PushStack, "00", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- DEPTH
        when opcDROP =>
          rec := (NopStack, "01", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- DROP
        when opc2DROP =>
          rec := (NopStack, "10", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- 2DROP
        when opcNIP =>
          rec := (PushStack, "10", "00" & Opcode(7) & '0', ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));-- NIP
        when opcPICK =>
          rec := (GetPut, "00", "0011", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                   -- PICK
        when opcPUT =>
          rec := (GetPut, "10", "0111", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                   -- PUT
        when opcVALUE =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, Im);                -- VALUE
        when opcS0 =>
          rec := (PushStack, "00", "00" & Opcode(7) & "0", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- DUP
        when opcS1 =>
          rec := (PushStack, "00", "00" & Opcode(7) & "1", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- OVER
        when opcSA0 =>
          rec := (PushStack, "00", "00" & (not Opcode(7)) & "0", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));          -- R@
        when opcSA1 =>
          rec := (PushStack, "00", "00" & (not Opcode(7)) & "1", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));          -- R1@
        when opcSAVE =>
          rec := (SaveStack, "00", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- SAVE
        when opcSETSP =>
          rec := (SetSPStack, "00", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));               -- SETSTACK
        when opcGETSP =>
          rec := (PushStack, "00", "0101", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                -- GETSTACK
        --when opcMULT =>
        --  rec := (PushStack, "10", "0111", SF, MultALU, DataVec(to_unsigned(0, DataVec'length)));                  -- *
        when opcADD =>
          rec := (PushStack, "10", "0111", SF, AddALU, DataVec(to_unsigned(0, DataVec'length)));                  -- +
        when opcSUB =>
          rec := (PushStack, "10", "0111", SF, SubALU, DataVec(to_unsigned(0, DataVec'length)));                  -- -
        when opcCMP =>
          rec := (PushStack, "00", "0111", SF, SubALU, DataVec(to_unsigned(0, DataVec'length)));                  -- cmp
        when opcINC =>
          rec := (PushStack, "01", "0111", FIm, AddALUnC, DataVec(to_unsigned(1, DataVec'length)));                -- inc
        when opcDEC =>
          rec := (PushStack, "01", "0111", FIm, SubALUnC, DataVec(to_unsigned(1, DataVec'length)));                -- dec
        when opc2MUL =>
          rec := (PushStack, "01", "0111", FIm, LshiftALU, DataVec(to_unsigned(1, DataVec'length)));               -- 2*
        when opc2DIV =>
          rec := (PushStack, "01", "0111", FIm, RshiftAALU, DataVec(to_unsigned(1, DataVec'length)));              -- 2/
        when opcAND =>
          rec := (PushStack, "10", "0111", SF, AndALU, DataVec(to_unsigned(0, DataVec'length)));                  -- and
        when opcCELLP =>
          rec := (PushStack, "01", "0111", FIm, AddALUnC, DataVec(to_unsigned(4, DataVec'length)));                -- CELL+
        when opcHALFP =>
          rec := (PushStack, "01", "0111", FIm, AddALUnC, DataVec(to_unsigned(2, DataVec'length)));                -- HALF+
        when opcINVERT =>
          rec := (PushStack, "01", "0111", FIm, XorALU, DataVec(not std_ulogic_vector(to_unsigned(0, DataVec'length))));                  -- INVERT
        when opcLSHIFT =>
          rec := (PushStack, "10", "0111", SF, LshiftALU, DataVec(to_unsigned(0, DataVec'length)));               -- LSHIFT
        when opcNEGATE =>
          rec := (PushStack, "01", "0111", ImF, SubALU, DataVec(to_unsigned(0, DataVec'length)));                  -- NEGATE
        when opcOR =>
          rec := (PushStack, "10", "0111", SF, OrALU, DataVec(to_unsigned(0, DataVec'length)));                   -- or
        when opcRSHIFT =>
          rec := (PushStack, "10", "0111", SF, RshiftALU, DataVec(to_unsigned(0, DataVec'length)));               -- RSHIFT
        when opcXOR =>
          rec := (PushStack, "10", "0111", SF, XorALU, DataVec(to_unsigned(0, DataVec'length)));                  -- xor
        when opcADC =>
          rec := (PushStack, "10", "0111", SF, AddCALU, DataVec(to_unsigned(0, DataVec'length)));                 -- +B
        when opcSBC =>
          rec := (PushStack, "10", "0111", SF, SubCALU, DataVec(to_unsigned(0, DataVec'length)));                 -- -B
        when opcLSHIFTC =>
          rec := (PushStack, "10", "0111", SF, LshiftCALU, DataVec(to_unsigned(0, DataVec'length)));              -- LSHIFTC
        when opcRSHIFTC =>
          rec := (PushStack, "10", "0111", SF, RshiftCALU, DataVec(to_unsigned(0, DataVec'length)));              -- RSHIFTC
        when opc0LTN =>
          rec := (PushStack, "00", "0111", FIm, LtALU, DataVec(to_unsigned(0, DataVec'length)));                   -- 0<!
        when opc0LT =>
          rec := (PushStack, "01", "0111", FIm, LtALU, DataVec(to_unsigned(0, DataVec'length)));                   -- 0<
        when opcU0LT =>
          rec := (PushStack, "01", "0111", FIm, ULtALU, DataVec(to_unsigned(0, DataVec'length)));                  -- U0<
        when opc0EQN =>
          rec := (PushStack, "00", "0111", FIm, EquALU, DataVec(to_unsigned(0, DataVec'length)));                  -- 0=!
        when opc0EQ =>
          rec := (PushStack, "01", "0111", FIm, EquALU, DataVec(to_unsigned(0, DataVec'length)));                  -- 0=
        when opc0NEN =>
          rec := (PushStack, "00", "0111", FIm, nEquALU, DataVec(to_unsigned(0, DataVec'length)));                 -- 0<>!
        when opc0NE =>
          rec := (PushStack, "01", "0111", FIm, nEquALU, DataVec(to_unsigned(0, DataVec'length)));                 -- 0<>
        when opc0GTN =>
          rec := (PushStack, "00", "0111", FIm, GtALU, DataVec(to_unsigned(0, DataVec'length)));                   -- 0>!
        when opc0GT =>
          rec := (PushStack, "01", "0111", FIm, GtALU, DataVec(to_unsigned(0, DataVec'length)));                   -- 0>
        when opcU0GT =>
          rec := (PushStack, "01", "0111", FIm, UGtALU, DataVec(to_unsigned(0, DataVec'length)));                  -- U0>
        when opcEXIT =>
	       a(0) := not Opcode(7);
          rec := (NopStack, "01", "1000", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- EXIT
        when opcCALL =>
	       a(0) := not Opcode(7);
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, Im);                -- CALL
        when opcTRAP =>
	       a(0) := not Opcode(7);
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, Im);                -- TRAP
        when opcBRANCH =>
          rec := (NopStack, "00", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- BRANCH
        when opc0BRANCHN =>
          rec := (NopStack, "00", "1000", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- 0BRANCH!
        when opc0BRANCH =>
          rec := (NopStack, "01", "1000", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- 0BRANCH
        when opcHALT =>
          rec := (NopStack, "00", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- HALT
        when opcCARRY =>
          rec := (PushStack, "00", "0111", ImIm, SubCALU, DataVec(to_unsigned(0, DataVec'length)));                 -- B@
        when opcCARRYST =>
          rec := (NopStack, "01", "0111", ImF, SubALU, DataVec(to_unsigned(0, DataVec'length)));                   -- B!
	     when opcBREAK =>
          rec := (NopStack, "00", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- BREAK
	     when opcNOP =>
          rec := (NopStack, "00", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- NOP
        when opcVALH =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, (DataVec'high downto 16 => Im(15)) & Im(15 downto 0));                -- VALH
	     when opcCONST15 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(15, DataVec'length)));
	     when opcCONST14 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(14, DataVec'length)));
	     when opcCONST13 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(13, DataVec'length)));
	     when opcCONST12 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(12, DataVec'length)));
	     when opcCONST11 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(11, DataVec'length)));
	     when opcCONST10 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(10, DataVec'length)));
	     when opcCONST9 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(9, DataVec'length)));
	     when opcCONST8 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(8, DataVec'length)));
	     when opcCONST7 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(7, DataVec'length)));
	     when opcCONST6 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(6, DataVec'length)));
	     when opcCONST5 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(5, DataVec'length)));
	     when opcCONST4 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(4, DataVec'length)));
	     when opcCONST3 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(3, DataVec'length)));
	     when opcCONST2 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(2, DataVec'length)));
	     when opcCONST1 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(1, DataVec'length)));
	     when opcCONST0 =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));
	     when opcCONST15M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "0001"));
	     when opcCONST14M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "0010"));
	     when opcCONST13M =>
		    rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "0011"));
	     when opcCONST12M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "0100"));
	     when opcCONST11M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "0101"));
	     when opcCONST10M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "0110"));
	     when opcCONST9M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "0111"));
	     when opcCONST8M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "1000"));
	     when opcCONST7M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "1001"));
	     when opcCONST6M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "1010"));
	     when opcCONST5M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "1011"));
	     when opcCONST4M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "1100"));
	     when opcCONST3M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "1101"));
	     when opcCONST2M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "1110"));
	     when opcCONST1M =>
          rec := (PushStack, "00", "0110", ImIm, AddALUnC, ((DataVec'high downto 4 => '1') & "1111"));
        --when opcSETPC =>
        --  rec := (NopStack, "01", "1000", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- SETPC
        --when opcSWITCH =>
        --  rec := (SwitchCore, "00", "0100", ImIm, AddALUnC, Im);               -- Switch core
	     when others =>
		    ill := '1';
          rec := (NopStack, "00", "0100", ImIm, AddALUnC, DataVec(to_unsigned(0, DataVec'length)));                 -- illegal
        end case;

	     Func <= rec.Func;
	     toPop <= rec.toPop;
	     SelectSource <= rec.SelectSource;
	     SelectAlu <= rec.SelectAlu;
	     AFunc <= rec.AFunc;
	     Immediate <= rec.Im;

	     SelectedStack <= BinRange(to_integer(std_ulogic_vector(a)));
	   end if;
		
		illegal <= ill;
    end if;
  end process Decode;

end Rtl;
