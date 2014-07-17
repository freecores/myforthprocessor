----------------------------------------------------------------------------------
-- Company: RIIC
-- Engineer: Gerhard Hohner Mat.nr.: 7555111
-- 
-- Create Date:    01/07/2004 
-- Design Name:    Diplomarbeit
-- Module Name:    ProgramCounter - Rtl 
-- Project Name:   32 bit FORTH processor
-- Target Devices: Spartan 3
-- Tool versions:  ISE 8.2
-- Description: implements a program prefetch queue
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

entity ProgramCounter is
  generic (constant Cores : integer := 2;
           constant TableBitWidth : integer := 4);
  port (nReset: in std_ulogic;													-- system reset
        Clock: in std_ulogic;														-- system clock
		  abortload: out std_ulogic;                                   -- abort current fetch cycle
	     ReadBlocked: in std_ulogic;												-- fetch cycle delayed
	     Opcode: out std_ulogic_vector(7 downto 0);							-- coded operation
	     DataIn: in DataVec;														-- incoming data
	     Address: out std_ulogic_vector(RAMrange'high + 1 downto 2);	-- memory address
	     Immediate: out DataVec;													-- immediate operand
	     Reads: out std_ulogic;							         			-- memory operation
	     Done: in std_ulogic;						            			   -- core ready for next operation
	     TOS: in DataVec;															-- return address
        TOSValid: in std_ulogic;													-- return address available
		  Token: out std_ulogic;                                       -- service routine running
	     Quit: out std_ulogic;														-- vector processed
	     Vector: in std_ulogic_vector(TableBitWidth - 1 downto 0);		-- interrupt vector
	     VectorValid: in std_ulogic);											-- interrupt vector present
end ProgramCounter;

architecture Rtl of ProgramCounter is
  type parray is array (0 to 2 ** Cores - 1) of unsigned(Address'high downto 0);
  signal PC: parray;
  type ftype is array (0 to 3) of std_ulogic_vector(39 downto 0);
  signal frames: ftype;
  signal saved, frame: std_ulogic_vector(39 downto 0);
  signal nextnextPC, nextPC: unsigned(Address'high downto 0);
  signal predictedAddress: unsigned(Address'range);
  signal Im, aOperand, aFirst, aSecond, aThird, thefirst, thesecond, thethird: DataVec;
  signal valid: std_ulogic_vector(2 downto 0);
  signal running, delayed, present, load, indirect, halt, Quitted, execute, switch: std_ulogic;
  signal aOpcode, nextOpcode, SavedOpcode: std_ulogic_vector(7 downto 0);
  signal service: unsigned(3 downto 0);
  signal lastNibble: unsigned(3 downto 0);
  signal nibble, load1, load3, load5: unsigned(3 downto 0);
  signal core: std_ulogic_vector(Cores - 1 downto 0);
begin
  -- quit interrupt vector
  Quit <= Quitted;
  -- force reading
  Reads <= load;
  -- address of location to read
  Address <= std_ulogic_vector(predictedAddress);
  -- immediate Operand
  Immediate <= Im;
  
  -- assign queue
  aFirst <= theFirst when valid(0) = '1' else
            DataIn;

  aSecond <= theSecond when valid(1) = '1' else
             DataIn;

  aThird <= theThird when valid(2) = '1' else
            DataIn;
				
  frames(0) <= theSecond (7 downto 0) & theFirst ;
  frames(1) <= theSecond (15 downto 0) & theFirst (31 downto 8);
  frames(2) <= theSecond (23 downto 0) & theFirst (31 downto 16);
  frames(3) <= theSecond & theFirst (31 downto 24);
  
  frame <= (others => '0') when nReset /= '1' else
           frames(to_integer(std_ulogic_vector(nextPC(1 downto 0))) mod 4);

  -- new last nibble of PC
  lastNibble <= load5 when frame(6 downto 0) = opcTRAP or frame(6 downto 0) = opcVALUE else
				    load3 when frame(6 downto 0) = opcCALL or frame(6 downto 0) = opcBRANCH or
               			   frame(6 downto 0) = opc0BRANCHN or frame(6 downto 0) = opc0BRANCH or
									--frame(6 downto 0) = opcVALH else
									frame(6 downto 0) = opcVALH or frame(6 downto 0) = opcSETPC or
									frame(6 downto 0) = opcSWITCH else
				    load1;

  nibble(3 downto 2) <= (others => '0');
  nibble(1 downto 0) <= nextPC(1 downto 0);
  
  load1 <= nibble + 1;
  load3 <= nibble + 3;
  load5 <= nibble + 5;
  
  nextnextPC <= (nextPC(nextPC'high downto 3) + 1) & (lastNibble(3) and nextPC(2)) & lastNibble(1 downto 0) when
		                       lastNibble(3) = '1' or (lastNibble(2) = '1' and nextPC(2) = '1') else
			       nextPC(nextPC'high downto 3) & (lastNibble(2) or nextPC(2)) & lastNibble(1 downto 0);
			  
  -- fetch new opcode
  aOpcode <= saved(7 downto 0);
  -- fetch new operand
  aOperand <= saved(39 downto 8);

  cycle: process(nReset, Clock) is
  variable newcore: std_ulogic_vector(core'range);
  variable newnextPC, newPC, x: unsigned(PC(to_integer(core))'range);
  variable newOperand: DataVec;
  variable newvalid: std_ulogic_vector(2 downto 0);
  variable newOpcode: std_ulogic_vector(7 downto 0);
  variable OpcodePresent, newdelayed, newrunning, newpresent, newload, newIndirect, newhalt, newabort,
           updatefull, forceload, cut1, cut2, newquit, interrupt, newexecute, empty, y, z, newswitch: std_ulogic;
  begin
    if nReset /= '1' then
	   for i in PC'range loop
        PC(i) <= (PC(i)'high => '1', others => '0');
		end loop;
		core <= (others => '0');
		nextPC <= (newPC'high => '1', others => '0');
		predictedAddress <= (predictedAddress'high => '1', others => '0');
		saved <= (others => '0');
		theFirst <= (others => '0');
		theSecond <= (others => '0');
		theThird <= (others => '0');
      Opcode <= '0' & opcNOP;
      SavedOpcode <= '0' & opcNOP;
		nextOpcode <= '0' & opcNOP;
      Indirect <= '0';
      load <= '0';
		abortload <= '0';
      present <= '0';
		valid <= (others => '0');
		running <= '1';
		delayed <= '0';
		halt <= '0';
		Im <= (others => '0');
		service <= (others => '0');
		Quitted <= '0';
		Token <= '0';
		execute <= '0';
    elsif rising_edge(Clock) then
	   newswitch := '0';
	   empty := switch;
		newquit := '0';
		updatefull := '0';
		newcore := core;
      newPC := PC(to_integer(core));
      newIndirect := Indirect;
		newhalt := halt;
		newOperand := Im;
		newnextPC := nextPC;
		newexecute := execute;

      -- datum will arrive next clock
		newpresent := load and (not ReadBlocked);
		-- signal load longer necessary?
      newload := load and ReadBlocked;
		-- possibly abort load
		newabort := newload;

      -- load data
      if present = '1' then
	     -- word present
	     if Indirect = '1' then
	       -- vector of trap, interrupt
	       newPC := unsigned(DataIn(newPC'range));
			 empty := '1';
	       newIndirect := '0';
        end if;
		  newvalid := valid(1 downto 0) & (not Indirect);
		else
		  newvalid := valid;
      end if;
		
      -- enough data in queue?
		interrupt := Vectorvalid and Done and (Quitted nor newIndirect);
		OpcodePresent := running and execute and Done and not (Indirect or halt or delayed or interrupt);

      newrunning := running;
		newdelayed := delayed;
			 
		y := (valid(0) and (valid(1) or (((lastNibble(0) nor lastNibble(1)) or not lastNibble(2)) and not lastNibble(3))));
		
		if execute = '1' then
		  z := OpcodePresent;
		else
		  z := running and not (Indirect or halt or delayed or interrupt);
		end if;
		
		cut1 := lastNibble(2) and y and z;
		cut2 := lastNibble(3) and y and z;
		
		if Done = '1' then
		  newOpcode := SavedOpcode(7) & opcNOP;
		else
		  newOpcode := SavedOpcode;
		end if;

		if running = '0' then
		  if TOSValid = '1' then
	       newrunning := '1'; 
		    updatefull := '1';
	       if nextOpcode(6 downto 0) = opcEXIT then
	         -- restore PC from stack
	         newPC := unsigned(TOS(newPC'range));
			   empty := '1';
	       elsif nextOpcode(6 downto 0) = opcSETPC then
			   -- set PC of a distinct core
				PC(to_integer(Im(Cores - 1 downto 0))) <= unsigned(TOS(newPC'range));
	       elsif unsigned(TOS) = 0 then
	         -- change PC
				x(newPC'high downto 16) := (others => Im(15));
				x(15 downto 0) := unsigned(Im(15 downto 0));
		      newPC := newPC + x;
			   empty := '1';
          end if;
	     end if;
      elsif delayed = '1' then
		  -- complete BRANCH, CALL
		  newOpcode := nextOpcode;
		  newdelayed := '0';
	     newOperand(DataVec'high downto newPC'high + 1) := (others => '0');
	     newOperand(newPC'range) := std_ulogic_vector(newPC);
		  x(nextPC'high downto 16) := (others => Im(15));
		  x(15 downto 0) := unsigned(Im(15 downto 0));
		  newPC := newPC + x;
		  empty := '1';
      elsif interrupt = '1' then
		  -- force reading of vector
		  newOpcode := '0' & opcTRAP;
	     newOperand(DataVec'high downto newPC'high + 1) := (others => '0');
	     newOperand(newPC'range) := std_ulogic_vector(newPC);
	     newPC(newPC'high downto Vector'high + 2) := (others => '0');
	     newPC(Vector'high + 2 downto 2) := unsigned(Vector);
	     newPC(1 downto 0) := "00";
		  empty := '1';
		  newhalt := '0';
	     newIndirect := '1';
		  newquit := '1';
	   elsif OpcodePresent = '1' then
		  newOpcode := aOpcode;
		  newOperand := aOperand;
		  newPC := nextPC;

		  if aOpcode(6 downto 0) = opcCALL or aOpcode(6 downto 0) = opcBRANCH then
		    -- add cycle to compute new PC
          newdelayed := '1';
			 nextOpcode <= aOpcode;
	       newOpcode(6 downto 0) := opcNOP;
			 empty := '1';
        elsif aOpcode(6 downto 0) = opc0BRANCHN or aOpcode(6 downto 0) = opc0BRANCH or
		        aOpcode(6 downto 0) = opcEXIT or aOpcode(6 downto 0) = opcSETPC then
	       -- force synchronisation with stack
          newrunning := '0';
			 nextOpcode <= aOpcode;
        elsif aOpcode(6 downto 0) = opcTRAP then
			 -- force reading of vector
	       newIndirect := '1';
	       newOperand(DataVec'high downto newPC'high + 1) := (others => '0');
	       newOperand(newPC'range) := std_ulogic_vector(newPC);
	       newPC := unsigned(aOperand(newPC'range));
			 empty := '1';
        elsif aOpcode(6 downto 0) = opcHALT then
		    -- no advance of PC
		    newhalt := '1';
        elsif aOpcode(6 downto 0) = opcGETCOREID then
		    -- get core ID
			 newOperand(newOperand'high downto core'high + 1) := (others => '0');
			 newOperand(core'range) := core;
			 updatefull := '1';
		  elsif aOpcode(6 downto 0) = opcSWITCH then
		    -- switch core
			 newOperand(newOperand'high downto core'length) := (others => '0');
			 newcore := aOperand(core'range);
			 newswitch := '1';
			 empty := '1';
		  else
			 updatefull := '1';
        end if;
	   elsif Indirect = '0' then
		  updatefull := not halt;
      end if;

      if z = '1' then
		  newexecute := y;
		  
		  if y = '1' then
		    saved <= frame;
		    newnextPC := nextnextPC;
		  end if;
		end if;
		
      -- adjust prefetch queue
      if cut2 = '1' then
        theFirst <= aThird;
        newvalid := "00" & newvalid(2);
		elsif cut1 = '1' then
        theFirst <= aSecond;
        theSecond <= aThird;
        newvalid := '0' & newvalid(2 downto 1);
		else
        theFirst <= aFirst;
        theSecond <= aSecond;
        theThird <= aThird;
		end if;
		
		if Done = '1' then
		  Token <= service(service'high);
		  service <= service(service'high - 1 downto 0) & newquit;
		else
 		  Token <= '0';
		end if;
		
		if empty = '1' then
		  newnextPC := newPC;
		  newexecute := '0';
		  newpresent := '0';
	     newload := '0';
		  newvalid := (others => '0');
		  updatefull := not (newdelayed or newswitch);
		  predictedAddress <= newPC(predictedAddress'range);
		elsif newpresent = '1' then
		  predictedAddress <= predictedAddress + 1;
		end if;
		
      forceload := (newvalid(1) nand (newvalid(2) or newpresent)) and updatefull;

		valid <= newvalid;
	   load <= newload or forceload;
		
      -- next state
		abortload <= newabort and not newload;
      running <= newrunning;
		delayed <= newdelayed;
		halt <= newhalt;
      Im <= newOperand;
		switch <= newswitch;
      PC(to_integer(core)) <= newPC;
		core <= newcore;
		nextPC <= newnextPC;
		Opcode <= newOpcode;
	   SavedOpcode <= newOpcode;
		present <= newpresent;
		execute <= newexecute;
      -- indirect fetch
      Indirect <= newIndirect;
		Quitted <= newquit;
    end if;
  end process cycle;

end Rtl;
