-----------------------------------------------------------------------------------
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
use work.mig_21_parameters_0.all;
use work.global.all;

entity MYCPU is
  generic (constant TableBitWidth : integer := 4;
           constant IndexBitWidth: integer := IndexBitWidth);
  port (nReset: in std_ulogic;
        SysClock: in std_ulogic;												-- system clock
	     TxD: out std_ulogic;													-- UART output
	     RxD: in std_ulogic;													-- UART input
	     source: in std_ulogic_vector(0 to 3);							-- CTC input signals
	     Interrupt: in std_ulogic_vector(0 to 7);    					-- interrupt sources
	     LED: out std_ulogic_vector(0 to 7);    							-- 
	     SD_A: out std_logic_vector((ROW_ADDRESS-1) downto 0);		-- memory address
	     SD_BA: out std_logic_vector((BANK_ADDRESS-1) downto 0);	-- bank address
	     SD_DQ: inout std_logic_vector((DATA_WIDTH-1) downto 0);	-- databus
		  SD_RAS: out std_logic;
		  SD_CAS: out std_logic;
		  SD_CK_N: out std_logic_vector((CLK_WIDTH-1) downto 0);
		  SD_CK_P: out std_logic_vector((CLK_WIDTH-1) downto 0);
		  SD_CKE: out std_logic;
		  SD_ODT: out std_logic;
		  SD_WE: out std_logic;
		  SD_CS: out std_logic;
		  SD_DM: out std_logic_vector((DATA_MASK_WIDTH-1) downto 0);
		  SD_LOOP_IN: in std_logic;
		  SD_LOOP_OUT: out std_logic;
		  MB_FB_CLK_IN: in std_logic;
		  MB_FB_CLK_OUT: out std_logic;
		  SD_DQS: inout std_logic_vector((DATA_STROBE_WIDTH-1) downto 0);
		  SD_DQS_N: inout std_logic_vector((DATA_STROBE_WIDTH-1) downto 0));
end MYCPU;

architecture Rtl of MYCPU is
  type RAMstate_t is (Start, Idle, Refresh, Clear, Delay, Delay1, Delay2, Ack, Readcmd, Writecmd, Sweep);
  signal DDRstate: RAMstate_t;
  
  signal captured: std_ulogic_vector(31 downto 0);
  signal Input, DataOutStack, ROM, DBin: DataVec;
  signal newCode: DataVec;
  signal AddressStack: std_ulogic_vector(RAMrange'high + 1 downto 0);
  signal IOCode: std_ulogic_vector(2 downto 0);
  signal ReadyR, ReadyT, ReadBlocked, ReadPPQ, Writes, ReadUART, Writecounter, WriteInt, Quit, CS, softint, CSCTC,
         CSInt, CSUART, VectorValid, ReadInt, illegal, MemStall, abortload, nReady, ROMtoStack,
			CEROM, MasterReset, highlow, oldROMtoStack, clockconstant,
			CToS, Token, sh, Reset: std_ulogic;
  signal stack, PPQ: std_ulogic;
  signal AddressPPQ: std_ulogic_vector(RAMrange'high + 1 downto 2);
  signal DataOutUART: std_ulogic_vector(23 downto 0);
  signal DataOutcounter: std_ulogic_vector(26 downto 0);
  signal Tint: std_ulogic_vector(3 downto 0);
  signal Vector: std_ulogic_vector(TableBitWidth - 1 downto 0);
  signal DataOutInt: std_ulogic_vector(2 ** TableBitWidth - 1 downto 0);
  signal ROMAddress: std_ulogic_vector(ROMrange);
  signal user_command_register: std_logic_vector(2 downto 0);
  signal mymask, user_data_mask: std_logic_vector((DATA_MASK_WIDTH*2-1) downto 0);
  signal user_input_data, user_output_data: std_logic_vector((DATA_WIDTH*2-1) downto 0);
  signal user_input_address: std_logic_vector(24 downto 0);
  signal olddata_valid_out, data_valid_out, Clock, Clock0, Clock90, sys_rst_tb: std_logic;
  signal burst_done, init_done, ar_done, auto_ref_req, user_cmd_ack, 
         dcm_lock, CLKIN_IBUFG, gnds: std_logic;
  signal pattern: unsigned(LED'range);
  signal onehot: std_ulogic_vector(3 downto 0);
  signal chain: std_ulogic_vector(3 downto 0);
  
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
	     IOCode: out std_ulogic_vector(2 downto 0);		     		 	   	-- memory operation
		  illegal: out std_ulogic;														-- illegal opcode
		  Token: out std_ulogic;                                          -- update pending interrupts
	     Quit: out std_ulogic;															-- vector processed
	     Vector: in std_ulogic_vector(TableBitWidth - 1 downto 0);			-- interrupt vector
	     VectorValid: in std_ulogic);												-- vector valid
  end component theCore;

  for myCore:theCore
	use entity work.theCore(Rtl)
	   generic map(CacheIndexBitWidth => CacheIndexBitWidth,
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

  component mig_21 is
  port (
      cntrl0_ddr2_dq                : inout std_logic_vector(31 downto 0);
      cntrl0_ddr2_a                 : out   std_logic_vector(12 downto 0);
      cntrl0_ddr2_ba                : out   std_logic_vector(1 downto 0);
      cntrl0_ddr2_cke               : out   std_logic;
      cntrl0_ddr2_cs_n              : out   std_logic;
      cntrl0_ddr2_ras_n             : out   std_logic;
      cntrl0_ddr2_cas_n             : out   std_logic;
      cntrl0_ddr2_we_n              : out   std_logic;
      cntrl0_ddr2_odt               : out   std_logic;
      cntrl0_ddr2_dm                : out   std_logic_vector(3 downto 0);
      cntrl0_rst_dqs_div_in         : in    std_logic;
      cntrl0_rst_dqs_div_out        : out   std_logic;
      sys_clkb                      : in    std_logic;
      sys_clk_in                    : in    std_logic;
      reset_in_n                    : in    std_logic;
      cntrl0_burst_done             : in    std_logic;
      cntrl0_init_done              : out   std_logic;
      cntrl0_ar_done                : out   std_logic;
      cntrl0_user_data_valid        : out   std_logic;
      cntrl0_auto_ref_req           : out   std_logic;
      cntrl0_user_cmd_ack           : out   std_logic;
      cntrl0_user_command_register  : in    std_logic_vector(2 downto 0);
      cntrl0_clk_tb                 : out   std_logic;
      cntrl0_clk90_tb               : out   std_logic;
      cntrl0_sys_rst_tb             : out   std_logic;
      cntrl0_sys_rst90_tb           : out   std_logic;
      cntrl0_sys_rst180_tb          : out   std_logic;
      cntrl0_user_output_data       : out   std_logic_vector(63 downto 0);
      cntrl0_user_input_data        : in    std_logic_vector(63 downto 0);
      cntrl0_user_data_mask         : in    std_logic_vector(7 downto 0);
      cntrl0_user_input_address     : in    std_logic_vector(24 downto 0);
      cntrl0_ddr2_dqs               : inout std_logic_vector(3 downto 0);
      cntrl0_ddr2_dqs_n             : inout std_logic_vector(3 downto 0);
      cntrl0_ddr2_ck                : out   std_logic_vector(1 downto 0);
      cntrl0_ddr2_ck_n              : out   std_logic_vector(1 downto 0)
    );
  end component mig_21;
  
  for myRAM:mig_21
    use entity work.mig_21(arc_mem_interface_top)
      port map(cntrl0_ddr2_dq => cntrl0_ddr2_dq,
               cntrl0_ddr2_a => cntrl0_ddr2_a,
               cntrl0_ddr2_ba => cntrl0_ddr2_ba,
               cntrl0_ddr2_cke => cntrl0_ddr2_cke,
               cntrl0_ddr2_cs_n => cntrl0_ddr2_cs_n,
               cntrl0_ddr2_ras_n => cntrl0_ddr2_ras_n,
               cntrl0_ddr2_cas_n => cntrl0_ddr2_cas_n,
               cntrl0_ddr2_we_n => cntrl0_ddr2_we_n,
               cntrl0_ddr2_odt => cntrl0_ddr2_odt,
               cntrl0_ddr2_dm => cntrl0_ddr2_dm,
               cntrl0_rst_dqs_div_in => cntrl0_rst_dqs_div_in,
               cntrl0_rst_dqs_div_out => cntrl0_rst_dqs_div_out,
					sys_clkb => sys_clkb,
               sys_clk_in => sys_clk_in,
               reset_in_n => reset_in_n,
               cntrl0_burst_done => cntrl0_burst_done,
               cntrl0_init_done => cntrl0_init_done,
               cntrl0_ar_done => cntrl0_ar_done,
               cntrl0_user_data_valid => cntrl0_user_data_valid,
               cntrl0_auto_ref_req => cntrl0_auto_ref_req,
               cntrl0_user_cmd_ack => cntrl0_user_cmd_ack,
               cntrl0_user_command_register => cntrl0_user_command_register,
               cntrl0_clk_tb => cntrl0_clk_tb,
               cntrl0_clk90_tb => cntrl0_clk90_tb,
               cntrl0_sys_rst_tb => cntrl0_sys_rst_tb,
               cntrl0_sys_rst90_tb => cntrl0_sys_rst90_tb,
               cntrl0_sys_rst180_tb => cntrl0_sys_rst180_tb,
               cntrl0_user_output_data => cntrl0_user_output_data,
               cntrl0_user_input_data => cntrl0_user_input_data,
               cntrl0_user_data_mask => cntrl0_user_data_mask,
               cntrl0_user_input_address => cntrl0_user_input_address,
               cntrl0_ddr2_dqs => cntrl0_ddr2_dqs,
               cntrl0_ddr2_dqs_n => cntrl0_ddr2_dqs_n,
               cntrl0_ddr2_ck => cntrl0_ddr2_ck,
               cntrl0_ddr2_ck_n => cntrl0_ddr2_ck_n);

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
				  
   Reset <= not nReset;
		
	myCore: component theCore
	   generic map(CacheIndexBitWidth => IndexBitWidth,
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
			      IntSignal(1) => illegal,--softint,
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
			      IntSignal(13) => Tint(3),
			      IntSignal(14) => ReadyR,
			      IntSignal(15) => ReadyT,
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

   myRAM: component mig_21
      port map(cntrl0_ddr2_dq => SD_DQ,
               cntrl0_ddr2_a => SD_A,
               cntrl0_ddr2_ba => SD_BA,
               cntrl0_ddr2_cke => SD_CKE,
               cntrl0_ddr2_cs_n => SD_CS,
               cntrl0_ddr2_ras_n => SD_RAS,
               cntrl0_ddr2_cas_n => SD_CAS,
               cntrl0_ddr2_we_n => SD_WE,
               cntrl0_ddr2_odt => SD_ODT,
               cntrl0_ddr2_dm => SD_DM,
               cntrl0_rst_dqs_div_in => SD_LOOP_IN,
               cntrl0_rst_dqs_div_out => SD_LOOP_OUT,
					sys_clkb => gnds,
               sys_clk_in => Clock,
               reset_in_n => dcm_lock,
               cntrl0_burst_done => burst_done,
               cntrl0_init_done => init_done,
               cntrl0_ar_done => ar_done,
               cntrl0_user_data_valid => data_valid_out,
               cntrl0_auto_ref_req => auto_ref_req,
               cntrl0_user_cmd_ack => user_cmd_ack,
               cntrl0_user_command_register => user_command_register,
               cntrl0_clk_tb => open,
               cntrl0_clk90_tb => Clock90,
               cntrl0_sys_rst_tb => sys_rst_tb,
               cntrl0_sys_rst90_tb => open,
               cntrl0_sys_rst180_tb => open,
               cntrl0_user_output_data => user_output_data,
               cntrl0_user_input_data => user_input_data,
               cntrl0_user_data_mask => user_data_mask,
               cntrl0_user_input_address => user_input_address,
               cntrl0_ddr2_dqs => SD_DQS,
               cntrl0_ddr2_dqs_n => SD_DQS_N,
               cntrl0_ddr2_ck => SD_CK_P,
               cntrl0_ddr2_ck_n => SD_CK_N);

  gnds <= '0';
  MB_FB_CLK_OUT <= MB_FB_CLK_IN;
  
  CToS <= '0';
  
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
		
  LED <= std_ulogic_vector(pattern);
		
  burst: process(MasterReset, Clock90) is
  variable dummy: std_logic_vector(user_output_data'range);
  begin
    if MasterReset /= '1' then
	   chain <= (others => '0');
		burst_done <= '0';
		olddata_valid_out <= '0';
		captured <= (others => '0');
		user_data_mask <= (others => '1');
    elsif rising_edge(Clock90) then
		if user_cmd_ack = '1' then
		  chain <= chain(chain'high - 1 downto 0) & '0';
		  burst_done <= chain(chain'high);
		else
	     chain <= (1 downto 0 => '1', others => '0');
		end if;
		if chain(3) = '0' then
		  user_data_mask <= mymask;
		else
		  user_data_mask <= (others => '1');
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
  variable newCEROM, ce, wr, oe, sa, sb, sc, sd, newReadBlocked, newStall: std_ulogic;
  variable RAMAddress: std_ulogic_vector(RAMrange'high downto 2);
  variable dummy: DataVec;
  variable newppq: std_ulogic;
  variable mycmd: std_logic_vector(user_command_register'range);
  begin
    if MasterReset /= '1' then
		DBin <= (others => '0');
		DDRstate <= Start;
		CEROM <= '0';
		nReady <= '1';
		stack <= '0';
		PPQ <= '0';
		oldROMtoStack <= '0';
		user_command_register <= "010";
		user_input_data <= (others => '0');
		mymask <= (others => '1');
		user_input_address <= (others => '0');
		onehot <= (others => '1');
		sh <= '1';
		pattern <= (others => '1');
    elsif rising_edge(Clock) then
	   newStall := '1';
		newReadBlocked := '1';
		newppq := PPQ and not abortload;
		
		if burst_done = '1' then
		  mycmd := (others => '0');
		else
		  mycmd := user_command_register;
		end if;
		
		if softint = '1' then
		  pattern <= unsigned(DataOutStack(7 downto 0));
		end if;
		
      -- update RAM machine
		
		case DDRstate is
		  when Start =>
			 Stack <= '0';
			 newppq := '0';
			 mycmd := "0" & onehot(3) & "0";
			 onehot <= onehot(2 downto 0) & '0';
 		    if init_done = '1' then
			   DDRstate <= Idle;
				--pattern <= (0 => '1', others => '0');
			 end if;
		  when Idle =>
		    if auto_ref_req = '1' then
			   DDRstate <= Refresh;
		    elsif user_cmd_ack = '0' then
		      if AddressStack(AddressStack'high) = '0' and (IOCode(0) = '1' or IOCode(1) = '1') then
				  RAMAddress := AddressStack(RAMAddress'range);
				  case IOCode(1 downto 0) is
				    when "01" =>
					   if AddressStack(1) = '0' then
					     sa := AddressStack(0);
						  sb := not AddressStack(0);
						  sc := '1';
						  sd := '1';
						else
					     sc := AddressStack(0);
						  sd := not AddressStack(0);
						  sa := '1';
						  sb := '1';
						end if;
						dummy := DataOutStack(7 downto 0) & DataOutStack(7 downto 0) & DataOutStack(7 downto 0) & DataOutStack(7 downto 0);
					 when "10" =>
					   if AddressStack(1) = '0' then
                    sa := '0';
						  sb := '0';
						  sc := '1';
						  sd := '1';
						else
                    sc := '0';
						  sd := '0';
						  sa := '1';
						  sb := '1';
						end if;
						dummy := DataOutStack(15 downto 0) & DataOutStack(15 downto 0);
					 when "11" =>
					   sa := '0';
						sb := '0';
						sc := '0';
						sd := '0';
						dummy := DataOutStack;
					 when others =>
					   sa := '1';
						sb := '1';
						sc := '1';
						sd := '1';
						dummy := DataOutStack;
				  end case;
				  newppq := '0';
				  stack <= '1';
				  wr := not IOCode(2);
		        user_input_address(RAMAddress'length - 1 downto 0) <= std_logic_vector(RAMAddress(RAMAddress'high - 2 downto RAMAddress'low) & RAMAddress(RAMAddress'high downto RAMAddress'high - 1));
		        if wr = '0' then
				    DDRstate <= Delay;
		          mymask <= (3 => sd, 2 => sc, 1 => (sb), 0 => (sa), others => '1');
		          user_input_data(user_input_data'high downto DataVec'length) <= (others => '1');
		          user_input_data(DataVec'range) <= std_logic_vector(dummy);
		          mycmd := "100";--"110";
		        else
				    DDRstate <= Ack;
					 mymask <= (others => '1');
		          mycmd := "110";
		        end if;
			   elsif AddressPPQ(AddressPPQ'high) = '0' and ReadPPQ = '1' and abortload = '0' then
				  RAMAddress := AddressPPQ(RAMAddress'high downto RAMAddress'low);
				  newppq := '1';
				  stack <= '0';
				  DDRstate <= Ack;
		        user_input_address(RAMAddress'length - 1 downto 0) <= std_logic_vector(RAMAddress(RAMAddress'high - 2 downto RAMAddress'low) & RAMAddress(RAMAddress'high downto RAMAddress'high - 1));
				  mymask <= (others => '1');
		        mycmd := "110";
			   else
				  newppq := '0';
				  Stack <= '0';
		        mycmd := "000";
			   end if;
			 end if;
		  when Refresh =>
		    mymask <= (others => '1'); -- do not write the last 2 dummies!
		    if ar_done = '1' then
			   DDRstate <= Clear;
		      mycmd := "000";
			 end if;
		  when Clear =>
		    if data_valid_out = '1' then
			   null;
			 else
			   DDRstate <= Idle;
			 end if;
		  when Delay =>
			 DDRstate <= Delay1;
		  when Delay1 =>
		    DDRstate <= Delay2;
		  when Delay2 =>
		    DDRstate <= Writecmd;
		    mymask <= (others => '1'); -- do not write the last 2 dummies!
		  when Writecmd =>
		    --user_command_register <= "0000";
		    newStall := '0';
			 DDRstate <= Sweep;
		  when Ack =>
		    DDRstate <= Readcmd;
		  when Readcmd =>
			 if olddata_valid_out = '1' then
				if Stack = '1' then
				  if IOCode(1 downto 0) = "11" or AddressStack(1) = '1' then
				    DBin <= std_ulogic_vector(captured(DBin'range));
				  else
				    DBin(31 downto 16) <= std_ulogic_vector(captured(15 downto 0));
				  end if;
				  newStall := '0';
				elsif newppq = '1' then
				  DBin <= std_ulogic_vector(captured(DBin'range));
				  newReadBlocked := '0';
				end if;
			   DDRstate <= Sweep;
			 end if;
		  when Sweep =>
		    if auto_ref_req = '1' then
			   DDRstate <= Refresh;
		    elsif data_valid_out = '0' then
			   DDRstate <= Idle;
				Stack <= '0';
				newppq := '0';
		      mycmd := "000";
			 end if;
		end case;
		
		user_command_register <= mycmd;
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
