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
use work.vhdl_syn_bl4_parameters_0.all;
use work.global.all;

entity MYCPU is
  generic (constant Cores : integer := Cores;
           constant TableBitWidth : integer := 4;
           constant IndexBitWidth: integer := IndexBitWidth);
  port (Reset: in std_ulogic;												   -- system reset
        SysClock: in std_ulogic;												-- system clock
	     RxD2: in std_ulogic;													-- UART input
	     TxD2: out std_ulogic;													-- UART output
	     RxD1: in std_ulogic;													-- UART input
	     TxD1: out std_ulogic;													-- UART output
	     source: in std_ulogic_vector(0 to 3);							-- CTC input signals
	     Interrupt: in std_ulogic_vector(0 to 7);    					-- interrupt sources
	     LED: out std_ulogic_vector(0 to 7);    							-- 
	     SD_A: out std_logic_vector(12 downto 0);						-- memory address
	     SD_BA: out std_logic_vector(1 downto 0);						-- bank address
	     SD_DQ: inout std_logic_vector(15 downto 0);					-- databus
		  SD_RAS: out std_logic;
		  SD_CAS: out std_logic;
		  SD_CK_N: out std_logic;
		  SD_CK_P: out std_logic;
		  SD_CKE: out std_logic;
		  SD_ODT: out std_logic;
		  SD_CS: out std_logic;
		  SD_WE: out std_logic;
		  SD_LDM: out std_logic;
		  SD_LOOP_IN: in std_logic;
		  SD_LOOP_OUT: out std_logic;
		  SD_UDM: out std_logic;
		  SD_LDQS_N: inout std_logic;
		  SD_LDQS_P: inout std_logic;
		  SD_UDQS_N: inout std_logic;
		  SD_UDQS_P: inout std_logic);
end MYCPU;

architecture Rtl of MYCPU is
  type RAMstate_t is (Start, Idle, Refresh, Clear, Delay, Delay1, Delay2, Ack, Readcmd, Writecmd, Sweep);
  signal DDRstate: RAMstate_t;
  
  signal captured: std_ulogic_vector(15 downto 0);
  signal newCode, Input, DataOutStack, ROM, DBin: DataVec;
  signal AddressStack: std_ulogic_vector(RAMrange'high + 1 downto 0);
  signal IOCode: std_ulogic_vector(2 downto 0);
  signal ReadyR, ReadyT, ReadBlocked, ReadPPQ, Writes, ReadUART, Writecounter, WriteInt, Quit, CS, softint, CSCTC,
         CSInt, CSUART, sh, VectorValid, ReadInt, illegal, MemStall, abortload, nReady, ROMtoStack,
			CEROM, MasterReset, highlow, oldROMtoStack, olddata_valid_out, clockconstant,
			CToS, RxD, TxD, Token: std_ulogic;
  signal stack, PPQ: std_ulogic_vector(1 downto 0);
  signal AddressPPQ: std_ulogic_vector(RAMrange'high + 1 downto 2);
  signal DataOutUART: std_ulogic_vector(23 downto 0);
  signal DataOutcounter: std_ulogic_vector(26 downto 0);
  signal Tint: std_ulogic_vector(3 downto 0);
  signal Vector: std_ulogic_vector(TableBitWidth - 1 downto 0);
  signal DataOutInt: std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
  signal ROMAddress: std_ulogic_vector(ROMrange);
  signal onehot: std_ulogic_vector(3 downto 0);
  signal chain: std_ulogic_vector(5 downto 0);
  signal user_command_register: std_logic_vector(3 downto 0);
  signal user_data_mask: std_logic_vector(((data_mask_width*2)-1) downto 0);
  signal user_input_data, user_output_data: std_logic_vector(((2*data_width)-1) downto 0);
  signal user_input_address: std_logic_vector(25 downto 0);
  signal burst_done, init_val, ar_done, auto_ref_req, user_cmd_ack, data_valid_out, Clock, Clock90, sys_rst_tb: std_logic;
  signal dcm_lock: std_logic;
  
  component dcm1 is
   generic ( constant basefrequency : integer := 50000000;
	          constant multiplicator : integer := 2;
				 constant divisor : integer := 3);
   port ( CLKIN_IN        : in    std_logic; 
          RST_IN          : in    std_logic; 
          CLKFX_OUT       : out   std_logic; 
          CLKIN_IBUFG_OUT : out   std_logic; 
          CLK0_OUT        : out   std_logic; 
          LOCKED_OUT      : out   std_logic);
  end component dcm1;
  
  for mydcm:dcm1
   use entity work.dcm1(BEHAVIORAL)
    generic map(basefrequency => basefrequency,
	             multiplicator => multiplicator,
				    divisor => divisor)
	  port map(CLKIN_IN => CLKIN_IN,
	           RST_IN => RST_IN,
				  CLKFX_OUT => CLKFX_OUT,
				  CLKIN_IBUFG_OUT => CLKIN_IBUFG_OUT,
				  CLK0_OUT => CLK0_OUT,
				  LOCKED_OUT => LOCKED_OUT);

  component theCore is
  generic (constant Cores : integer := 2;
           constant CacheIndexBitWidth: integer := 10;						-- ld(Cachesize)
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
	     IOCode: out std_ulogic_vector(2 downto 0);		     		 	   	-- memory operation
		  illegal: out std_ulogic;														-- illegal opcode
		  Token: out std_ulogic;                                          -- update pending interrupts
	     Quit: out std_ulogic;															-- vector processed
	     Vector: in std_ulogic_vector(TableBitWidth - 1 downto 0);			-- interrupt vector
	     VectorValid: in std_ulogic);												-- vector valid
  end component theCore;

  for myCore:theCore
	use entity work.theCore(Rtl)
	   generic map(Cores => Cores,
		            CacheIndexBitWidth => CacheIndexBitWidth,
	               TableBitWidth => TableBitWidth)
		port map(nReset => nReset,
		         Clock => Clock,
					abortload => abortload,
					MemStall => MemStall,
			      DataIn => DataIn,
			      ReadBlocked => ReadBlocked,
			      DataROM => DataROM,
			      AddressROM => AddressROM,
			      Reads => Reads,
			      Address => Address,
			      DataOut => DataOut,
			      IOCode => IOCode,
					illegal => illegal,
					Token => Token,
			      Quit => Quit,
			      Vector => Vector,
			      VectorValid => VectorValid);

  component UART is
      port (nReset: in std_ulogic;										 -- reset
            Clk: in std_ulogic;							 				 -- system clock
	         RxD: in std_ulogic;							 				 -- UART input
	         TxD: out std_ulogic;							 				 -- UART output
	         RTS: out std_ulogic;											 -- request to send
	         CToS: in std_ulogic;											 -- clear to send
		      ReadyR: out std_ulogic;										 -- receiver ready
		      ReadyT: out std_ulogic;										 -- transmiter ready
		      Address: in std_ulogic_vector(2 downto 0);			 -- port address
		      DataIn: in std_ulogic_vector(23 downto 0);			 -- parallel input
		      DataOut: out std_ulogic_vector(23 downto 0);			 -- parallel output
		      Writes: in std_ulogic;										 -- read from UART
				Reads: in std_ulogic);
  end component UART;

  for oneUART:UART
	use entity work.UART(Rtl)
		port map(nReset => nReset,
		         Clk => Clk,
		         RxD => RxD,
			      TxD => TxD,
			      RTS => RTS,
			      CToS => CToS,
			      ReadyR => ReadyR,
			      ReadyT => ReadyT,
			      Address => Address,
			      DataIn => DataIn,
			      DataOut => DataOut,
			      Writes => Writes,
					Reads => Reads);

  component counter is
  port (nReset: in std_ulogic;											-- system reset
        Clock: in std_ulogic;												-- system clock
		  Address: in std_ulogic_vector(1 downto 0);             -- address
	     source: in std_ulogic_vector(0 to 3);						-- counter input
	     DataIn: in std_ulogic_vector(26 downto 0);					-- incomming data
	     DataOut: out std_ulogic_vector(26 downto 0);				-- outgoing data
	     Writes: in std_ulogic;							          		-- write operation (active high)
	     Tint: out std_ulogic_vector(3 downto 0));						-- interrupt request
  end component counter;

  for onecounter:counter
	use entity work.counter(Rtl)
		port map(nReset => nReset,
		         Clock => Clock,
					Address => Address,
		         source => source,
			      DataIn => DataIn,
			      DataOut => DataOut,
			      Writes => Writes,
			      Tint => Tint);

  component IntVectors is
  generic (constant TableBitWidth : integer := 4);
  port (nReset: in std_ulogic;
        Clock: in std_ulogic;
	     Quit: in std_ulogic;
		  Token: in std_ulogic;                                              -- update pending
	     IntSignal: in std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
	     Writes: in std_ulogic;
	     Reads: in std_ulogic;
	     Address: in std_ulogic_vector(1 downto 0);
	     DataIn: in std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
	     DataOut: out std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
	     Vector: out std_ulogic_vector(TableBitWidth - 1 downto 0);
	     VectorValid: out std_ulogic);
  end component IntVectors;

  for myIntVectors:IntVectors
	use entity work.IntVectors(Rtl)
	   generic map(TableBitWidth => TableBitWidth)
		port map(nReset => nReset,
		         Clock => Clock,
		         Quit => Quit,
					Token => Token,
			      IntSignal => IntSignal,
			      Writes => Writes,
					Reads => Reads,
			      Address => Address,
			      DataOut => DataOut,
			      DataIn => DataIn,
			      Vector => Vector,
			      VectorValid => VectorValid);

  component ROMcode is
  port (Clock: in std_ulogic;
        Address: in std_ulogic_vector(ROMrange);
		  Data: out DataVec);
  end component ROMcode;

  for myROMcode:ROMcode
    use entity work.ROMcode(Rtl)
	   port map(Clock => Clock,
		         Address => Address,
					Data => Data);

  component vhdl_syn_bl4 is
  port (
    cntrl0_DDR2_DQ                       : inout  std_logic_vector(15 downto 0);
    cntrl0_DDR2_A                        : out  std_logic_vector(12 downto 0);
    cntrl0_DDR2_BA                       : out  std_logic_vector(1 downto 0);
    cntrl0_DDR2_CK                       : out std_logic;
    cntrl0_DDR2_CK_N                     : out std_logic;
    cntrl0_DDR2_CKE                      : out std_logic;
    cntrl0_DDR2_CS_N                     : out std_logic;
    cntrl0_DDR2_RAS_N                    : out std_logic;
    cntrl0_DDR2_CAS_N                    : out std_logic;
    cntrl0_DDR2_WE_N                     : out std_logic;
    cntrl0_DDR2_ODT                      : out std_logic;
    cntrl0_DDR2_DM                       : out  std_logic_vector(1 downto 0);
    cntrl0_rst_dqs_div_in                : in std_logic;
    cntrl0_rst_dqs_div_out               : out std_logic;
    --SYS_CLKb                             : in std_logic;
    SYS_CLK                              : in std_logic;
    reset_in_n                           : in std_logic;
    cntrl0_burst_done                    : in std_logic;
    cntrl0_init_val                      : out std_logic;
    cntrl0_ar_done                       : out std_logic;
    cntrl0_user_data_valid               : out std_logic;
    cntrl0_auto_ref_req                  : out std_logic;
    cntrl0_user_cmd_ack                  : out std_logic;
    cntrl0_user_command_register         : in  std_logic_vector(3 downto 0);
    cntrl0_clk_tb                        : out std_logic;
    cntrl0_clk90_tb                      : out std_logic;
    cntrl0_sys_rst_tb                    : out std_logic;
    cntrl0_sys_rst90_tb                  : out std_logic;
    cntrl0_sys_rst180_tb                 : out std_logic;
    cntrl0_user_output_data              : out  std_logic_vector(31 downto 0);
    cntrl0_user_input_data               : in  std_logic_vector(31 downto 0);
    cntrl0_user_data_mask                : in  std_logic_vector(3 downto 0);
    cntrl0_user_input_address            : in  std_logic_vector(25 downto 0);
    cntrl0_DDR2_DQS                      : inout  std_logic_vector(1 downto 0);
    cntrl0_DDR2_DQS_N                    : inout  std_logic_vector(1 downto 0)
    );
  end component vhdl_syn_bl4;
  
  for myRAM:vhdl_syn_bl4
    use entity work.vhdl_syn_bl4(arc_mem_interface_top)
     port map(cntrl0_DDR2_DQ => cntrl0_DDR2_DQ,
              cntrl0_DDR2_A => cntrl0_DDR2_A,
              cntrl0_DDR2_BA => cntrl0_DDR2_BA,
              cntrl0_DDR2_CK => cntrl0_DDR2_CK,
              cntrl0_DDR2_CK_N => cntrl0_DDR2_CK_N,
              cntrl0_DDR2_CKE => cntrl0_DDR2_CKE,
              cntrl0_DDR2_CS_N => cntrl0_DDR2_CS_N,
              cntrl0_DDR2_RAS_N => cntrl0_DDR2_RAS_N,
              cntrl0_DDR2_CAS_N => cntrl0_DDR2_CAS_N,
              cntrl0_DDR2_WE_N => cntrl0_DDR2_WE_N,
              cntrl0_DDR2_ODT => cntrl0_DDR2_ODT,
              cntrl0_DDR2_DM => cntrl0_DDR2_DM,
              cntrl0_rst_dqs_div_in => cntrl0_rst_dqs_div_in,
              cntrl0_rst_dqs_div_out => cntrl0_rst_dqs_div_out,
				  --SYS_CLKb => clockb,
              SYS_CLK => SYS_CLK,
				  cntrl0_user_data_valid => cntrl0_user_data_valid,
              reset_in_n => reset_in_n,
              cntrl0_DDR2_DQS => cntrl0_DDR2_DQS,
              cntrl0_DDR2_DQS_N => cntrl0_DDR2_DQS_N,
              cntrl0_burst_done => cntrl0_burst_done,
              cntrl0_init_val => cntrl0_init_val,
              cntrl0_ar_done => cntrl0_ar_done,
              cntrl0_auto_ref_req => cntrl0_auto_ref_req,
              cntrl0_user_cmd_ack => cntrl0_user_cmd_ack,
              cntrl0_user_command_register => cntrl0_user_command_register,
              cntrl0_clk_tb => cntrl0_clk_tb,
              cntrl0_clk90_tb => cntrl0_clk90_tb,
              cntrl0_sys_rst_tb => cntrl0_sys_rst_tb,
              cntrl0_sys_rst90_tb => cntrl0_sys_rst90_tb,
              cntrl0_sys_rst180_tb => cntrl0_sys_rst180_tb,
              cntrl0_user_data_mask => cntrl0_user_data_mask,
              cntrl0_user_output_data => cntrl0_user_output_data,
              cntrl0_user_input_data => cntrl0_user_input_data,
              cntrl0_user_input_address => cntrl0_user_input_address);

begin
					 
   mydcm: component dcm1
    generic map(basefrequency => oscillator,
	             multiplicator => multiplicator,
				    divisor => divisor)
	  port map(CLKIN_IN => SysClock,
	           RST_IN => Reset,
				  CLKFX_OUT => Clock,
				  CLKIN_IBUFG_OUT => open,
				  CLK0_OUT => open,
				  LOCKED_OUT => dcm_lock);
		
	myCore: component theCore
	   generic map(Cores => Cores,
		            CacheIndexBitWidth => IndexBitWidth,
	               TableBitWidth => TableBitWidth)
		port map(nReset => MasterReset,
		         Clock => Clock,
					abortload => abortload,
					MemStall => MemStall,
			      DataIn => Input,
			      ReadBlocked => ReadBlocked,
			      DataROM => newCode,
			      AddressROM => AddressPPQ,
			      Reads => ReadPPQ,
			      Address => AddressStack,
			      DataOut => DataOutStack,
			      IOCode => IOCode,
					illegal => illegal,
					Token => Token,
			      Quit => Quit,
			      Vector => Vector,
			      VectorValid => VectorValid);

	oneUART: component UART
		port map(nReset => MasterReset,
		         Clk => Clock,
		         RxD => RxD,
			      TxD => TxD,
			      --DTR => DTR,
			      --DSR => DSR,
			      RTS => open,
			      CToS => CToS,
			      ReadyR => ReadyR,
			      ReadyT => ReadyT,
			      Address => AddressStack(4 downto 2),
			      DataIn => DataOutStack(23 downto 0),
			      DataOut => DataOutUART,
			      Writes => Writes,
					Reads => ReadUART);

	onecounter: component counter
		port map(nReset => MasterReset,
		         Clock => Clock,
					Address => AddressStack(3 downto 2),
		         source => source,
			      DataIn => DataOutStack(26 downto 0),
			      DataOut => DataOutcounter,
			      Writes => Writecounter,
			      Tint => Tint);

	myIntVectors: component IntVectors
	   generic map(TableBitWidth => TableBitWidth)
		port map(nReset => MasterReset,
		         Clock => Clock,
		         Quit => Quit,
					Token => Token,
			      IntSignal(0) => illegal,
			      IntSignal(1) => softint,
			      IntSignal(2) => Interrupt(0),
			      IntSignal(3) => Interrupt(1),
			      IntSignal(4) => Interrupt(2),
			      IntSignal(5) => Interrupt(3),
			      IntSignal(6) => Interrupt(4),
			      IntSignal(7) => Interrupt(5),
			      IntSignal(8) => Interrupt(6),
			      IntSignal(9) => Interrupt(7),
			      IntSignal(10) => Tint(0),
			      IntSignal(11) => Tint(1),
			      IntSignal(12) => Tint(2),
			      IntSignal(13) => ReadyR,
			      IntSignal(14) => ReadyT,
			      IntSignal(15) => Tint(3),
			      Writes => WriteInt,
					Reads => ReadInt,
			      Address => AddressStack(3 downto 2),
			      DataOut => DataOutInt,
			      DataIn => DataOutStack(DataOutInt'range),
			      Vector => Vector,
			      VectorValid => VectorValid);

   myROMcode: component ROMcode
	  port map(Clock => Clock,
		        Address => ROMAddress,
				  Data => ROM);

   myRAM: component vhdl_syn_bl4
     port map(cntrl0_DDR2_DQ => SD_DQ,
              cntrl0_DDR2_A => SD_A,
              cntrl0_DDR2_BA => SD_BA,
              cntrl0_DDR2_CK => SD_CK_P,
              cntrl0_DDR2_CK_N => SD_CK_N,
              cntrl0_DDR2_CKE => SD_CKE,
              cntrl0_DDR2_CS_N => SD_CS,
              cntrl0_DDR2_RAS_N => SD_RAS,
              cntrl0_DDR2_CAS_N => SD_CAS,
              cntrl0_DDR2_WE_N => SD_WE,
              cntrl0_DDR2_ODT => SD_ODT,
              cntrl0_DDR2_DM(0) => SD_LDM,
              cntrl0_DDR2_DM(1) => SD_UDM,
              cntrl0_rst_dqs_div_in => SD_LOOP_IN,
              cntrl0_rst_dqs_div_out => SD_LOOP_OUT,
				  --SYS_CLKb => clockb,
              SYS_CLK => Clock,
				  cntrl0_user_data_valid => data_valid_out,
              reset_in_n => dcm_lock,
              cntrl0_DDR2_DQS(0) => SD_LDQS_P,
              cntrl0_DDR2_DQS(1) => SD_UDQS_P,
              cntrl0_DDR2_DQS_N(0) => SD_LDQS_N,
              cntrl0_DDR2_DQS_N(1) => SD_UDQS_N,
              cntrl0_burst_done => burst_done,
              cntrl0_init_val => init_val,
              cntrl0_ar_done => ar_done,
              cntrl0_auto_ref_req => auto_ref_req,
              cntrl0_user_cmd_ack => user_cmd_ack,
              cntrl0_user_command_register => user_command_register,
              cntrl0_clk_tb => open,
              cntrl0_clk90_tb => Clock90,
              cntrl0_sys_rst_tb => sys_rst_tb,
              cntrl0_sys_rst90_tb => open,
              cntrl0_sys_rst180_tb => open,
              cntrl0_user_data_mask => user_data_mask,
              cntrl0_user_output_data => user_output_data,
              cntrl0_user_input_data => user_input_data,
              cntrl0_user_input_address => user_input_address);

  RxD <= RxD1 and RxD2;
  TxD1 <= TxD;
  TxD2 <= TxD;
  CToS <= '0';
  
  LED <= (others => '0');
  
  MasterReset <= not sys_rst_tb;
  
  CS <= '0' when MasterReset /= '1' else
        sh when unsigned(not AddressStack(AddressStack'high downto 6)) = 0 and IOCode(1 downto 0) /= "00" else
        '0';
		  
  -- CTC has addresses 0xffffffc0, 0xffffffc4, 0xffffffc8, 0xffffffcc
  CSCTC <= CS and (AddressStack(5) nor AddressStack(4));
  Writecounter <= CSCTC and IOCode(2);
  -- Interrupt arbiter has addresses 0xffffffd0, 0xffffffd4, 0xffffffd8, 0xffffffdc
  CSInt <= CS and (AddressStack(4) and not AddressStack(5));
  WriteInt <= CSInt and IOCode(2);
  ReadInt <= CSInt and not IOCode(2);
  -- UART has addresses 0xffffffe0, 0xffffffe4, 0xffffffe8, 0xffffffec, 0xfffffff0, 0xfffffff4, 0xfffffff8
  CSUART <= CS and (AddressStack(5) and not (AddressStack(4) and AddressStack(3) and AddressStack(2)));
  ReadUART <= CSUART and not IOCode(2);
  Writes <= CSUART and IOCode(2);
  -- Software interrupt 0xfffffffc
  clockconstant <= CS and (AddressStack(5) and AddressStack(4) and AddressStack(3) and AddressStack(2));
  softint <= clockconstant and IOCode(2);

  MemStall <= sh;
  
  ReadBlocked <= (nReady or abortload) and not (AddressPPQ(AddressPPQ'high) and ReadPPQ and not ROMtoStack);
  
  ROMtoStack <= '0' when MasterReset /= '1' else
                '1' when IOCode(1 downto 0) /= "00" and CS = '0' and AddressStack(AddressStack'high) = '1' else
					 '0';
					 
  ROMAddress <= AddressStack(ROMAddress'range) when ROMtoStack = '1' else
                AddressPPQ(ROMAddress'range);
					 
  newCode <= ROM when CEROM = '1' else
             DBin;
				 
  -- data to stack
  Input <= DBin;
		
  burst: process(MasterReset, Clock90) is
  variable dummy: std_logic_vector(user_output_data'range);
  begin
    if MasterReset /= '1' then
	   chain <= (others => '0');
		burst_done <= '0';
		olddata_valid_out <= '0';
		captured <= (others => '0');
    elsif rising_edge(Clock90) then
		if user_cmd_ack = '1' then
		  chain <= chain(chain'high - 1 downto 0) & '0';
		  burst_done <= chain(chain'high);
		else
	     chain <= (1 downto 0 => '1', others => '0');
		end if;
		if data_valid_out = '1' and olddata_valid_out = '0' then
		  captured <= std_ulogic_vector(user_output_data(captured'range));
		else
		  dummy := user_output_data;
		end if;
		olddata_valid_out <= data_valid_out;
	 end if;
  end process burst;
  
  fetch: process(MasterReset, Clock) is
  variable newCEROM, ce, wr, oe, sa, sb, newReadBlocked, newStall: std_ulogic;
  variable RAMAddress: std_ulogic_vector(RAMrange'high downto 1);
  variable dummy: DataVec;
  variable newppq: std_ulogic_vector(PPQ'range);
  begin
    if MasterReset /= '1' then
		DBin <= (others => '0');
		DDRstate <= Start;
		sh <= '1';
		CEROM <= '0';
		nReady <= '1';
		stack <= (others => '0');
		PPQ <= (others => '0');
		oldROMtoStack <= '0';
		user_command_register <= "0010"; -- initialize memory
		user_input_data <= (others => '0');
		user_data_mask <= (others => '0');
		user_input_address <= (others => '0');
		onehot <= (others => '1');
    elsif rising_edge(Clock) then
	 
	   newStall := '1';
		newReadBlocked := '1';
		if abortload = '1' then
		  newppq := "00";
		else
		  newppq := PPQ;
		end if;
		
      -- update RAM machine
		
		case DDRstate is
		  when Start =>
			 Stack <= "00";
			 newppq := "00";
			 user_command_register <= "00" & onehot(3) & "0";
			 onehot <= onehot(2 downto 0) & '0';
 		    if init_val = '1' then
			   DDRstate <= Sweep;
			 end if;
		  when Idle =>
		    if user_cmd_ack = '0' then
		      if auto_ref_req = '1' then
			     DDRstate <= Refresh;
		        user_command_register <= "0000";
			   elsif AddressStack(AddressStack'high) = '0' and (IOCode(0) = '1' or IOCode(1) = '1') and newppq(1) = '0' then
				  if IOCode(1) = '1' and IOCode(0) = '1' then
				    RAMAddress(RAMAddress'high downto 2) := AddressStack(RAMAddress'high downto 2);
					 if Stack(1) = '1' then
					   RAMAddress(1) := '1';
					   dummy := DataOutStack(31 downto 16) & DataOutStack(31 downto 16);
					 else
					   RAMAddress(1) := '0';
						Stack <= "11";
					   dummy := DataOutStack(15 downto 0) & DataOutStack(15 downto 0);
					 end if;
				  else
				    RAMAddress := AddressStack(RAMAddress'range);
					 Stack <= "10";
					 dummy := DataOutStack(15 downto 0) & DataOutStack(15 downto 0);
				  end if;
				  newppq := "00";
				  if IOCode(1) = '1' then
				    sa := '0';
					 sb := '0';
				  else
				    sa := AddressStack(0);
				    sb := not AddressStack(0);
				  end if;
				  wr := not IOCode(2);
		        user_input_address <= std_logic_vector(RAMAddress(RAMAddress'high - 2 downto 11) & "0" & RAMAddress(10 downto 1) & RAMAddress(RAMAddress'high downto RAMAddress'high - 1));
		        if wr = '0' then
				    DDRstate <= Delay;
		          user_data_mask <= '1' & '1' & (sb) & (sa);
		          user_command_register <= "0100";
		          if sa = '0' and sb = '0' then
		            user_input_data <= std_logic_vector(dummy);
			       elsif sb = '0' then
		            user_input_data(15 downto 8) <= std_logic_vector(dummy(7 downto 0));
			       elsif sa = '0' then
		            user_input_data(7 downto 0) <= std_logic_vector(dummy(7 downto 0));
			       end if;
		        else
				    DDRstate <= Ack;
		          user_command_register <= "0110";
					 user_data_mask <= (others => '1');
		        end if;
			   elsif AddressPPQ(AddressPPQ'high) = '0' and ReadPPQ = '1' and abortload = '0' then
				  RAMAddress := AddressPPQ(RAMAddress'high downto AddressPPQ'low) & PPQ(1);
				  DDRstate <= Ack;
		        user_input_address <= std_logic_vector(RAMAddress(RAMAddress'high - 2 downto 11) & "0" & RAMAddress(10 downto 1) & RAMAddress(RAMAddress'high downto RAMAddress'high - 1));
				  user_data_mask <= (others => '1');
		        user_command_register <= "0110";
				  if newppq(1) = '0' then
				    newppq := "11";
				  end if;
				  Stack <= "00";
			   else
				  newppq := "00";
				  Stack <= "00";
		        user_command_register <= "0000";
			   end if;
			 end if;
		  when Refresh =>
		    if ar_done = '1' then
			   DDRstate <= Clear;
		      user_command_register <= "0000";
			 end if;
		  when Clear =>
		    if data_valid_out = '1' then
			   null;
			 else
			   DDRstate <= Idle;
			 end if;
		  when Delay =>
		    if user_cmd_ack = '1' then
			   DDRstate <= Delay1;
			 end if;
		  when Delay1 =>
		    DDRstate <= Delay2;
		  when Delay2 =>
		    DDRstate <= Writecmd;
		    user_data_mask <= (others => '1'); -- do not write the last 2 dummies!
		  when Writecmd =>
		    newStall := Stack(0);
			 DDRstate <= Sweep;
		  when Ack =>
		    DDRstate <= Readcmd;
		  when Readcmd =>
		    if olddata_valid_out = '1' then
		      user_command_register <= "0000";
				if Stack(1) = '1' then
				  if Stack(0) = '0' then
				    DBin(31 downto 16) <= std_ulogic_vector(captured);
					 newStall := '0';
				  else
				    DBin(15 downto 0) <= std_ulogic_vector(captured);
				  end if;
				elsif newppq(1) = '1' then
				  if newppq(0) = '0' then
				    DBin(31 downto 16) <= std_ulogic_vector(captured);
					 newReadBlocked := '0';
				  else
				    DBin(15 downto 0) <= std_ulogic_vector(captured);
				  end if;
				end if;
			   DDRstate <= Sweep;
			 end if;
		  when Sweep =>
		    user_command_register <= "0000";
		    if auto_ref_req = '1' then
			   DDRstate <= Refresh;
				Stack <= Stack(0) & '0';
				newppq := newppq(0) & '0';
		    elsif data_valid_out = '0' then
			   DDRstate <= Idle;
				Stack <= Stack(0) & '0';
				newppq := newppq(0) & '0';
			 end if;
		end case;
		
		PPQ <= newppq;
		-- update ROM access
		
		newCEROM := '0';

		if  ROMtoStack = '1' then
		  highlow <= (IOCode(0) and IOCode(1)) or AddressStack(1);
		elsif AddressPPQ(AddressPPQ'high) = '1' and ReadPPQ = '1' then
		  newCEROM := '1';
		end if;
		
		oldROMtoStack <= ROMtoStack;
		CEROM <= newCEROM;
		
		-- stall fetching units
		
      nReady <= newReadBlocked or abortload;
		
		sh <= (newStall and (CS nor ROMtoStack)) or not sh;
		
		if IOCode(2) = '0' then
		  if CSCTC = '1' then
		    DBin <= (Input'high downto DataOutcounter'high + 1 => '0') & DataOutcounter;
        elsif clockconstant = '1' then
          DBin <= DataVec(to_unsigned(integer(theClock), DataVec'length));
		  elsif CSUART = '1' then
          DBin <= (Input'high downto DataOutUART'high + 1 => '0') & DataOutUART;
		  elsif CSInt = '1' then
          DBin <= (Input'high downto DataOutInt'high + 1 => '0') & DataOutInt;
		  elsif oldROMtoStack = '1' and highlow = '1' then
			 DBin <= ROM;
		  elsif oldROMtoStack = '1' and highlow = '0' then
			 DBin <= ROM(15 downto 0) & ROM(31 downto 16);
		  end if;
		end if;
    end if;
  end process fetch;

end Rtl;
