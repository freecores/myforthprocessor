----------------------------------------------------------------------------------
-- Company: RIIC
-- Engineer: Gerhard Hohner Mat.nr.: 7555111
-- 
-- Create Date:    01/07/2004 
-- Design Name:    Diplomarbeit
-- Module Name:    theStacks - Rtl 
-- Project Name:   32 bit FORTH processor
-- Target Devices: Spartan 3
-- Tool versions:  ISE 8.2
-- Description: implements stack, stack management, and interface ALU
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

entity theStacks is
   generic (constant CacheIndexBitWidth: integer := 9);             	-- log. of depth of cache
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
end entity theStacks;

architecture Rtl of theStacks is
constant Depth: integer := 2 ** CacheIndexBitWidth;
subtype indexValues is natural range 0 to (2 * Depth - 1);
subtype Cacherange is natural range 0 to (Depth - 1);
signal IndexR, IndexI, oldf, olds: IndexValues;
signal AluResult, TheFirst, TheSecond, BufferedInput, bank0, bank1,
       SideA, SideB, ImmediatePop, ImmediateStep, BufferedOutput: DataVec;
signal targetStep: unsigned(RAMrange'high - 1 downto 0);
signal aIOCode: std_ulogic_vector(2 downto 0);
signal early, vfast, fast, afast: std_ulogic_vector(1 downto 0);
signal aTarget: std_ulogic_vector(Target'range);
signal theStackFunc, StackFuncPop, StackFuncStep, StackFunc: StackFuncType;
signal stall2, store, stall1, fetch, ReadPop, WritePop, ReadStep, WriteStep,
       oldstall1, preStore, put, WriteOperate, WriteOperate1, demultiplex: std_ulogic; 
type DataType is array (Cacherange) of DataVec;
signal Cache0, Cache1: DataType;
signal prevalid, SelectData, toPop: std_ulogic_vector(1 downto 0);
signal SelectedOp, SelectedPop, Selected, SelectedStep, aSelect: BinRange;
subtype indexLast is natural range 0 to 3;
type ReloadType is (check, load, inhibit);
type StackType is record
  StackPtr : unsigned(RAMrange'high - 1 downto 0);
  StackLength : unsigned(RAMrange'high - 1 downto 0);
  Cached : unsigned(CacheIndexBitWidth downto 0);
  Top : unsigned(CacheIndexBitWidth - 1 downto 0);
  Tail : unsigned(CacheIndexBitWidth - 1 downto 0);
  Append : unsigned(CacheIndexBitWidth - 1 downto 0);
  ReloadState: ReloadType;
end record;
type StackArray is array (BinRange) of StackType;
signal Stack: StackArray;
type twoarray is array (BinRange) of DataVec;
type threearray is array (0 to 1) of DataVec;
signal overload: threearray;
signal visible, Pre, modified: twoarray;
type TOSarray is array (BinRange) of twoarray;
signal Actual: TOSarray;
signal Phantom: TOSarray;
signal Source, SourcePop, SourceStep: std_ulogic_vector(3 downto 0);
signal ByteSelect: std_ulogic_vector(2 downto 0);
signal saverunning, Carry, CarryOut: std_ulogic;
signal SelectAluPop: InputOrder_t;
signal AluFuncPop, AluFunc: AluFuncType;
signal minuend, Cachedminor: unsigned(CacheIndexBitWidth downto 0);
signal relative: unsigned(RAMrange'high downto 0);
signal aStackLength: unsigned(RAMrange'high - 1 downto 0);
signal aStackPtr: unsigned(RAMrange'high - 1 downto 0);
signal aAppend, aTop, aTail: unsigned(CacheIndexBitWidth - 1 downto 0);

  component ALU is
	port (nReset: in std_ulogic;					    -- reset
	      SideA: in DataVec;						    -- first Operand
		   SideB: in DataVec;						    -- second Operand
		   AluResult: out DataVec;					    -- result
		   CarryIn: in std_ulogic;					    -- Carry in
		   CarryOut: out std_ulogic;			       -- Carry out
		   AluFunc: in AluFuncType); 				    -- Opcode
  end component ALU;

  for myALU:ALU
	use entity work.ALU(Rtl)
		port map(nReset => nReset,
		         SideA => SideA,
		         SideB => SideB,
		         AluResult => AluResult,
			      CarryIn => CarryIn,
			      CarryOut => CarryOut,
			      AluFunc => AluFunc);

begin

	myALU: component ALU
		port map(nReset => nReset,
		         SideA => SideA,
		         SideB => SideB,
		         AluResult => AluResult,
		  	      CarryIn => Carry,
			      CarryOut => CarryOut,
			      AluFunc => AluFunc);

  SideA <= BufferedInput when afast(0) = '1' and fast(aSelect) = '1' else
           modified(0);

  SideB <= BufferedInput when afast(1) = '1' and fast(0) = '1' else
           modified(1);

  Selected <= SelectedPop when stall1 = '1' else
              SelectedStack;

  Source <= SourcePop when stall1 = '1' else
            SelectSource;

  toPop <= (others => '0') when stall1 = '1' else		-- clear for following cycles
           Pop;

  First <= visible(0);

  -- select correct datum(0) for level 0
  Pre(0) <= BufferedInput when fast(0) = '1' else	-- input(t) overwrittes level 2 datum(t)
            Phantom(SelectedOp)(0);

  -- select correct datum(1) for level 0
  Pre(1) <= BufferedInput when fast(1) = '1' else
            Phantom(SelectedOp)(1);

  Actual(0) <= Pre when SelectedOp = 0 else
               Phantom(0);
  Actual(1) <= Pre when SelectedOp = 1 else
               Phantom(1);

  TOSvalid <= SourceStep(3);
  
  visible <= Actual(SelectedStep);

  Ready <= nReset and not (stall1 or stall2);

  DataOut <= BufferedOutput;
				 
  StackFunc <= Func when stall1 = '0' else			 		-- new series of cycles
               theStackFunc;								      -- additional cycle

  Target <= aTarget;
  IOCode <= aIOCode;
  
  TheFirst <= bank0 when demultiplex = '0' else bank1;
  TheSecond <= bank0 when demultiplex = '1' else bank1;
  
  -- the cache
  theCache: process(Clock) is
  variable index0, index1, indexf, indexs, index: IndexValues;
  variable deny, newearly: std_ulogic_vector(1 downto 0);
  variable w, x, y: twoarray;
  variable tostore: DataVec;
  variable flag: std_ulogic;
  begin
    if rising_edge(Clock) then
	   y := Pre;

		newearly := "00";
		deny := "11";

		indexf := to_integer(unsigned(to_unsigned(SelectedPop, 1) & Stack(SelectedPop).Top));
		indexs := to_integer(unsigned(to_unsigned(SelectedPop, 1) & (Stack(SelectedPop).Top - 1)));
		
		if Fetch = '1' then
		  index := IndexR;
		else
		  index := IndexF;
		end if;
		
		-- update last read values
		if early(0) = '1' then
		  x(0) := overload(0);
		else
		  x(0) := theFirst;
		end if;
		
		if early(1) = '1' then
		  x(1) := overload(1);
		else
		  x(1) := theSecond;
		end if;
		
	   -- take currently valid datum
		if SelectData = "11" then
		  tostore := TheFirst; -- random read cache
		elsif SelectData = "10" then
		  tostore := BufferedInput;
		else -- read memory
		  tostore := (others => '0');
		  case ByteSelect is
		    when "010" =>
			   tostore(7 downto 0) := DataIn(23 downto 16);
			 when "011" =>
			   tostore(7 downto 0) := DataIn(31 downto 24);
			 when "100" =>
			   tostore(15 downto 0) := DataIn(31 downto 16);
			 when others =>
			   tostore := DataIn;
		  end case;
      end if;
		
		if nReset = '1' and (stall2 = '0' or fetch = '1') then
		  -- top element overwritten?
		  if Index = IndexI and store = '1' and fetch = '0' then
		    newearly(0) := '1';
		    overload(0) <= tostore;
		  else
		    overload(0) <= (others => '0');
		    newearly(0) := not prevalid(0);
		    deny(0) := '0';
		  end if;
		end if;

		if nReset = '1' and (stall2 = '0') then
		  -- second top element overwritten?
		  if IndexS = IndexI and store = '1' then
			 newearly(1) := '1';
		    overload(1) <= tostore;
		  else
		    overload(1) <= (others => '0');
		    newearly(1) := not prevalid(1);
		    deny(1) := '0';
		  end if;
		end if;

      if index mod 2 = 0 then
		  index0 := index;
		  index1 := indexs;
		  demultiplex <= '0';
		else
		  index0 := indexs;
		  index1 := index;
		  deny := deny(0) & deny(1);
		  demultiplex <= '1';
		end if;

		if deny(0) = '0' then
		  bank0 <= Cache0(index0 / 2);
	   end if;
		
		if deny(1) = '0' then
		  bank1 <= Cache1(index1 / 2);
		end if;

		if nReset = '1' and (stall2 = '0' or put = '1') then
	     if store = '1' then
		    if IndexI mod 2 = 0 then
	         Cache0(IndexI / 2) <= tostore;
			 else
	         Cache1(IndexI / 2) <= tostore;
			 end if;
	     end if;
		end if;
		
		-- update cache and retrieve data from cache
	   if nReset = '1' and stall2 = '0' then
	     -- update with currently generated values
		  if store = '1' and indexI = oldf then
		    x(0) := tostore;
		  elsif store = '1' and indexI = olds then
		    x(1) := tostore;
		  end if;
		  
	     -- update phantom
		  if vfast(0) = '1' then
	       y(0) := tostore;
		  elsif vfast(1) = '1' then
	       y(1) := tostore;
		  end if;

        if SelectedOp = SelectedStep then
		    if vfast(0) = '0' then
			   y(0) := x(0);
			 end if;
		    if vfast(1) = '0' then
			   y(1) := x(1);
			 end if;
		  end if;

		  early <= newearly;
		  oldf <= indexf;
		  olds <= indexs;
		  
		  --if oldstall1 = '0' then
			 if SelectedPop = SelectedStep then
			   flag := '1';
				if SelectedPop = SelectedOp then
			     w := y;
			   else
			     w := x;
			   end if;
			 else
				flag := '0';
				if SelectedPop = SelectedOp then
			     w := y;
			   else
			     w := Actual(SelectedPop);
			   end if;
			 end if;
			 
			 if SelectAluPop = ImF or SelectAluPop = ImIm then
			   modified(0) <= ImmediatePop;
				afast(0) <= '0';
			 else
			   if SelectAluPop = SF then
				  afast(0) <= flag;
				  modified(0) <= w(1);
				  aSelect <= 1;
				else
				  afast(0) <= flag;
				  modified(0) <= w(0);
				  aSelect <= 0;
				end if;
			 end if;
			 
			 if SelectAluPop = FIm or SelectAluPop = ImIm then
			   modified(1) <= ImmediatePop;
				afast(1) <= '0';
			 else
			   afast(1) <= flag;
				modified(1) <= w(0);
			 end if;
		  --end if;
		end if;

      if SelectedOp /= SelectedStep then
		  Phantom(SelectedStep) <= x;
		end if;
		
		Phantom(SelectedOp) <= y;
    end if;
  end process theCache;

  Indicate: process(Clock, nReset) is
  variable newCached, incrCached: unsigned(CacheIndexBitWidth downto 0);
  variable newTop: unsigned(CacheIndexBitWidth-1 downto 0);
  variable newStackLength: unsigned(RAMrange'high - 1 downto 0);
  variable newStackFunc, aStackFunc: StackFuncType;
  variable help, newReady, newRead, newSave: std_ulogic;
  variable newToPop: unsigned(1 downto 0);
  variable nextReloadState, ReloadState: ReloadType;
  variable newvalid: std_ulogic_vector(1 downto 0);
  begin
    if nReset /= '1' then
      for i in BinRange loop
	     Stack(i).Cached <= (others => '0');
	     Stack(i).Top <= (others => '1');
		  Stack(i).ReloadState <= check;
      end loop;
	   ReadPop <= '0';
	   WritePop <= '0';
	   stall1 <= '0';
      SelectedPop <= 0;
	   SourcePop <= (others => '0');
	   SelectAluPop <= ImIm;
	   AluFuncPop <= AddALUnC;
	   ImmediatePop <= (others => '0');
		prevalid <= (others => '0');
		theStackFunc <= (others => '0');
		StackFuncPop <= (others => '0');
		saverunning <= '0';
    elsif rising_edge(Clock) then
	 -- second stage unblocked
      if stall2 = '0' then
        newReady := '1';
	     newRead := '0';
	     newSave := '0';
	     help := '0';
	     incrCached := (others => '0');
	     newStackFunc := StackFunc;
	     aStackFunc := StackFunc;
		  newToPop := unsigned(toPop);

	     if StackFunc = SetSPStack then
		    if stall1 = '0' or oldstall1 = '0' then
            newReady := '0';
				newToPop := (others => '0');
            newStackLength := Stack(Selected).StackLength;
	         newCached := Stack(Selected).Cached;
		      ReloadState := Stack(Selected).ReloadState;
		      newTop := Stack(Selected).Top;
				nextReloadState := ReloadState;
			   newvalid := "11";
			 else
	         -- set Stackpointer and length
	         newStackLength := unsigned(visible(1)(RAMrange'high - 1 downto 0));
	         newCached := (others => '0');
	         newTop := (others => '1');
			   ReloadState := check;
		      aStackFunc := NopStack;
				
		      if newStackLength /= 0 then
			     newReady := '0';	-- insert stall
			     nextReloadState := load;
				else
				  nextReloadState := ReloadState;
            end if;

			   newvalid := "00";
			 end if;
        else
		    newTop := Stack(Selected).Top;
          newStackLength := Stack(Selected).StackLength;
	       newCached := Stack(Selected).Cached;
		    ReloadState := Stack(Selected).ReloadState;
			 nextReloadState := ReloadState;

			 newvalid := "11";
			 
			 if ReloadState = load then
				newReady := '0';	-- insert stall
		      aStackFunc := NopStack;
				if ReadPop = '0' then
	           newRead := '1';
		        if newStackLength(newStackLength'high downto 1) = 0 or
				     unsigned(not newCached(newCached'high - 1 downto 1)) = 0 then
				    nextReloadState := check;
              end if;
				  incrCached(0) := '1';
				end if;
			 else
	         -- update length of cache

	         if newCached > newToPop then
	           newCached :=  newCached - newToPop;
				else
	           -- cache empty
		        newToPop := newCached(newToPop'range);
	           newCached := (others => '0');
	         end if;

			   -- update top position in cache
	         if StackFunc = PushStack then
		        -- prepare cache for push operation
	           newToPop := newToPop - 1;
              incrCached(0) := not newCached(newCached'high);
              newSave := newCached(newCached'high);
			 	  newvalid(0) := newCached(newCached'high);
	           aStackFunc := NopStack;
				  if newCached = 0 then
				    newvalid(1) := '0';
				  end if;
            else
			     if newCached(newCached'high downto 1) = 0 then
				    newvalid(0) := newCached(0);
				    newvalid(1) := '0';
				  end if;
			     if StackFunc = GetPut or StackFunc = LoadStore then
	             aStackFunc := NopStack;
	             newReady := '0';
				  elsif StackFunc = SaveStack or saverunning = '1' then
				    if newCached /= 0 then
				      incrCached := (others => not WritePop);
				      newSave := not WritePop;
	               newReady := '0';
			         ReloadState := load;
						saverunning <= '1';
					 else
					   saverunning <= '0';
					 end if;
              end if;
	         end if;

		      -- pop
	         newTop := newTop - unsigned((newTop'high downto newToPop'high + 1 => newToPop(1) and newToPop(0)) & newToPop);

	         -- save command or cache full at push
	         if newSave = '1' then
	           newReady := '0';
	           aStackFunc := NopStack;
			     nextReloadState := inhibit;
              -- try to restore Cache
	         elsif ReloadState = check then
		        -- restauration automaton
		        -- restauration possible, if part of stack in RAM, cache not full, currently no stall executed
		        if newStackLength /= 0 and newCached(newCached'high) = '0' and unsigned(not newCached(newCached'high - 1 downto 1)) /= 0 and stall1 = '0' then
			       newReady := '0';	-- insert stall
		          aStackFunc := NopStack;
			       nextReloadState := load;
              end if;
		      elsif ReloadState = inhibit then
              -- drop executed?
	           if unsigned(toPop) /= 0 and (StackFunc = NopStack or unsigned(newCached) = 0) then
			       nextReloadState := check;
			     end if;
	         end if;
			 end if;
	     end if;

		  prevalid <= newvalid;

	     newCached := newCached + incrCached;

	     Stack(Selected).Cached <=  newCached(newCached'range);
	     Stack(Selected).Top <= newTop;
		  Stack(Selected).ReloadState <= nextReloadState;

	     theStackFunc <= aStackFunc;
	     StackFuncPop <= newStackFunc;

	     SourcePop <= Source;
	     SelectedPop <= Selected;
	     ReadPop <= newRead;
	     WritePop <= newSave;
	     stall1 <= not newReady;

		  if stall1 = '0' then
	       ImmediatePop <= Immediate;
	       SelectAluPop <= SelectAlu;
	       AluFuncPop <= AFunc;
		  end if;  
      end if;
    end if;
  end process Indicate;

  Step: process(Clock, nReset) is
  variable newCached: unsigned(CacheIndexBitWidth downto 0);
  variable newTail, newAppend: unsigned(CacheIndexBitWidth-1 downto 0);
  variable newStackPtr, newStackLength: unsigned(RAMrange'high - 1 downto 0);
  variable newTarget: unsigned(RAMrange'high - 1 downto 0);
  begin
    if nReset /= '1' then
      for i in BinRange loop
        Stack(i).StackPtr <= (others => '0');
        Stack(i).StackLength <= (others => '0');
	     Stack(i).Tail <= (others => '0');
	     Stack(i).Append <= (others => '1');
      end loop;

		Cachedminor <= (others => '0');
		minuend <= (others => '0');
      aStackLength <= (others => '0');
	   aStackPtr <= (others => '0');
	   aTop <= (others => '1');
	   aTail <= (others => '0');
	   aAppend <= (others => '1');
		SourceStep <= (others => '0');
		targetStep <= (others => '0');
	   SelectedStep <= 1;
		ReadStep <= '0';
		WriteStep <= '0';
		StackFuncStep <= (others => '0');
	   Carry <= '0';
	   AluFunc <= AddALUnC;
		ImmediateStep <= (others => '0');
		oldstall1 <= '0';
	 elsif rising_edge(Clock) then
	   oldstall1 <= stall1;
		
		if stall2 = '0' then
		  -- 
	     if StackFuncPop = SetSPStack and oldstall1 = '1' then
	       -- set Stackpointer and length
	       newStackPtr := unsigned(visible(0)(RAMrange'high + 1 downto 2));
	       newStackLength := unsigned(visible(1)(RAMrange'high - 1 downto 0));
	       newTail := (others => '0');
	       newAppend := (others => '1');
        else
	       newStackPtr := Stack(SelectedPop).StackPtr;
	       newStackLength := Stack(SelectedPop).StackLength;
		    newTail := Stack(SelectedPop).Tail;
	       newAppend := Stack(SelectedPop).Append;
		  end if;
		  
		  newTarget := newStackPtr;
		  
		  if WritePop = '1' then
	       newStackPtr := newStackPtr - 1;
	       newTarget := newStackPtr;
          newStackLength := newStackLength + 1;
			 newAppend := newTail;
		    newTail := newTail + 1;
		  elsif ReadPop = '1' then
	       newStackPtr := newStackPtr + 1;
          newStackLength := newStackLength - 1;
	       newTail := newAppend;
	       newAppend := newAppend - 1;
		  end if;
		  
        Stack(SelectedPop).StackLength <= newStackLength;
	     Stack(SelectedPop).StackPtr <= newStackPtr;
	     Stack(SelectedPop).Append <= newAppend;
	     Stack(SelectedPop).Tail <= newTail;
		  
        aStackLength <= newStackLength;
	     aStackPtr <= newStackPtr;
	     aTop <= Stack(SelectedPop).Top;
	     aTail <= newTail;
	     aAppend <= newAppend;
		
		  ImmediateStep <= ImmediatePop;
		  SourceStep(2 downto 0) <= SourcePop(2 downto 0);
		  SourceStep(3) <= SourcePop(3) and not oldstall1;
	     SelectedStep <= SelectedPop;
		  targetStep <= newTarget;
		  ReadStep <= ReadPop;
		  WriteStep <= WritePop;
		  StackFuncStep <= StackFuncPop;
		  
	     newCached := Stack(SelectedPop).Cached - 1;
		  Cachedminor <= newCached;
		  
		  if SourcePop(2) = '0' then
		    minuend <= newCached;
		  else
		    minuend <= Stack(SelectedPop).Cached;
		  end if;
		  
		  if oldstall1 = '0' then
	       Carry <= CarryOut;
	       AluFunc <= AluFuncPop;
		  end if;
		end if;
	 end if;
  end process Step;
  
  --Cachedminor <= Stack(SelectedPop).Cached - 1;
  --minuend <= Cachedminor when SourcePop(2) = '0' else
  --           Stack(SelectedPop).Cached;
  relative <= unsigned(visible(0)(RAMrange'high downto 0)) - minuend;

  Operate: process(Clock, nReset) is
  variable dummy, anewCached: unsigned(CacheIndexBitWidth downto 0);
  variable newTop, newTail, inCache, newIndexR, newIndexI: unsigned(CacheIndexBitWidth-1 downto 0);
  variable inStack, newStackPtr, newStackLength: unsigned(RAMrange'high - 1 downto 0);
  variable newTarget: std_ulogic_vector(aTarget'range);
  variable step, newStall, newput, newFetch, newStore, newLocked: std_ulogic;
  variable newIOCOde: std_ulogic_vector(2 downto 0);
  variable newSelectData: std_ulogic_vector(2 downto 0);
  variable c, d: unsigned(0 to 0);
  variable newvfast, newfast: std_ulogic_vector(1 downto 0);
  begin
    if nReset /= '1' then
	   SelectedOp <= 1;
	   stall2 <= '0';
		--WriteStall <= '0';
	   aIOCode <= (others => '0');
	   preStore <= '0';
	   IndexI <= 0;
	   IndexR <= 0;
	   store <= '0';
	   aTarget <= (others => '0');
		fast <= (others => '0');
		vfast <= (others => '0');
		fetch <= '0';
		put <= '0';
		WriteOperate <= '0';
		WriteOperate1 <= '0';
		BufferedInput <= (others => '0');
		BufferedOutput <= (others => '0');
		SelectData <= (others => '0');
		ByteSelect <= (others => '0');
    elsif rising_edge(Clock) then
      newStackLength := aStackLength;
	   anewCached := Cachedminor;
	   newStackPtr := aStackPtr;
	   newTop := aTop;
	   newTail := aTail;
	   newIndexI := newTop;
	   newIndexR := newTop;
	   newSelectData := "110";
		newfast := "00";
		newvfast := "00";

	   newIOCode := (others => '0');
	   newFetch := '0';
		newput := '0';
		newStall := '0';
		newLocked := stall2;
	   newStore := preStore;
	   newTarget := (others => '0');
		step := '0';

	   if stall2 = '0' then
	     newStore := '0';

		  if StackFuncStep = LoadStore or (StackFuncStep = GetPut and relative(relative'high) = '0') then
		    -- load/store
			 if StackFuncStep = LoadStore then
		      newTarget := visible(0)(newTarget'range);
			   newStall := '1';--visible(0)(DataVec'high) nand visible(0)(6);
			 else
	         inStack := newStackPtr + unsigned(relative(inStack'range));
	         newTarget(newTarget'high downto 2) := std_ulogic_vector(inStack);
				newStall := '1';
			 end if;
		    BufferedOutput <= visible(1);
		    newIOCode := SourceStep(newIOCode'range);
			 if SourceStep(2) = '0' then
		      newSelectData := "100";
		      newStore := '1';
				newvfast := "01";
			 end if;
	     elsif StackFuncStep = GetPut then
		    -- get/put
		    BufferedInput <= visible(1);
	       inCache := newTail + unsigned(not relative(inCache'range));
		    if SourceStep(2) = '0' then
	         newIndexR := inCache;
				newvfast := "01";
		    else
	         newIndexI := inCache;
		    end if;
	       newFetch := not SourceStep(2);
	       newput := SourceStep(2);
		    newSelectData := "11" & not SourceStep(2);
	       newStore := '1';
	     elsif StackFuncStep = PushStack then
	       -- (preceeding) push, getSP, getLenght
		    newStore := '1';
			 newfast := "01";
		    case SourceStep(2 downto 0) is
		      when "111" =>
		        BufferedInput <= AluResult;
		      when "110" =>
		        BufferedInput <= ImmediateStep;
		      when "101" =>
		        BufferedInput <= DataVec((DataVec'high downto newStackPtr'high + 3 => '0') & newStackPtr & "00");
		      when "100" =>
		        BufferedInput <= DataVec((DataVec'high downto newStackLength'high + 1 => '0') & (newStackLength + anewCached));
            when others =>
			     c(0) := SourceStep(1);
				  d(0) := SourceStep(0);
		        BufferedInput <= Actual(to_integer(c))(to_integer(d));
		    end case;
        end if;

	     if ReadStep = '1' then
	       newTarget := std_ulogic_vector(TargetStep) & "00";
		    newIOCode := "011";
	       newIndexI := newTail;
	       newStore := '1';
	       newSelectData := "100";
			 if anewCached(anewCached'high downto 1) = 0 then
				newvfast := anewCached(0) & (not anewCached(0));
			 end if;
			 newStall := '1';
	     elsif WriteStep = '1' then
	       newTarget := std_ulogic_vector(TargetStep) & "00";
	       newFetch := '1';
          newIndexR := aAppend;
			 step := '1';
		  end if;
		  
		  if newStore = '1' then
		    IndexI <= to_integer(unsigned(to_unsigned(SelectedStep, 1) & newIndexI));
		  end if;

        IndexR <= to_integer(unsigned(to_unsigned(SelectedStep, 1) & newIndexR));
		  
		  fast <= newfast;
		  vfast <= newvfast;

	     SelectData(1) <= newSelectData(2) and newSelectData(1);
	     SelectData(0) <= newSelectData(2) and newSelectData(0);

	     SelectedOp <= SelectedStep;
	     aTarget <= newTarget(aTarget'range);
		  ByteSelect <= newIOCOde(1 downto 0) & newTarget(0);
	   end if;

      WriteOperate <= step;
      WriteOperate1 <= WriteOperate;
		
	   if WriteOperate1 = '1' then
		  BufferedOutput <= theFirst;
		  newIOCode := "111";
		  newStall := '1';
		end if;
		
      if stall2 = '0' or (Fetch = '1' and WriteOperate = '0') or put = '1' or memstall = '0' then
	     aIOCode <= newIOCOde;
	     Fetch <= newFetch;
		  put <= newput;
	     preStore <= newStore;

	     newLocked := newStall or newFetch or newput;
	     store <= newStore and not newLocked;

	     stall2 <= newLocked;
		end if;
    end if;
  end process Operate;

end architecture Rtl;
