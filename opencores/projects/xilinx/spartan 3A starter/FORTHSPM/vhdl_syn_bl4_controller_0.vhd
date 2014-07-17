-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_controller_0.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/03/08 05:06:10 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : THis is main controller block. This includes the following
--                    features:
--                    - The controller state machine that controls the
--                    initialization process upon power up, as well as the
--                    read, write, and refresh commands.
--                    - Accepts and decodes the user commands.
--                    - Generates the address and Bank address signals
--                    - Generates control signals for other modules, including
--                    the control signals for the dqs_en block.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use UNISIM.VCOMPONENTS.all;
use work.global.all;
use work.vhdl_syn_bl4_parameters_0.all;

entity vhdl_syn_bl4_controller_0 is
  port(
    clk               : in std_logic;
    rst0              : in std_logic;
    rst180            : in std_logic;
    address           : in std_logic_vector(((row_address + col_ap_width)-1)
                                            downto 0);
    bank_addr         : in  std_logic_vector((bank_address-1) downto 0);
    command_register  : in  std_logic_vector(3 downto 0);
    burst_done        : in  std_logic;
    ddr_rasb_cntrl    : out std_logic;
    ddr_casb_cntrl    : out std_logic;
    ddr_web_cntrl     : out std_logic;
    ddr_ba_cntrl      : out std_logic_vector((bank_address-1) downto 0);
    ddr_address_cntrl : out std_logic_vector((row_address-1) downto 0);
    ddr_cke_cntrl     : out std_logic;
    ddr_csb_cntrl     : out std_logic;
    ddr_ODT_cntrl     : out std_logic;
    dqs_enable        : out std_logic;
    dqs_reset         : out std_logic;
    write_enable      : out std_logic;
    rst_calib         : out std_logic;
    rst_dqs_div_int   : out std_logic;
    cmd_ack           : out std_logic;
    init              : out std_logic;
    ar_done           : out std_logic;
    wait_200us        : in  std_logic;
    auto_ref_req      : out std_logic
    );
end vhdl_syn_bl4_controller_0;


architecture arc of vhdl_syn_bl4_controller_0 is

  type s_m is (IDLE, PRECHARGE, AUTO_REFRESH, ACTIVE,
               FIRST_WRITE, WRITE_WAIT, BURST_WRITE, 
	       PRECHARGE_AFTER_WRITE, PRECHARGE_AFTER_WRITE_2, READ_WAIT,
               BURST_READ,ACTIVE_WAIT);

  type s_m1 is (INIT_IDLE, INIT_PRECHARGE,
               INIT_AUTO_REFRESH, INIT_LOAD_MODE_REG);
  signal next_state, current_state           : s_m;
  signal init_next_state, init_current_state : s_m1;

  
  signal ack_reg                : std_logic;
  signal ack_o                  : std_logic;
  signal auto_ref               : std_logic;
  signal auto_ref1              : std_logic;
  signal AUTOREF_value          : std_logic;
  signal AUTO_REF_detect1       : std_logic;
  signal AUTOREF_COUNT          : std_logic_vector((max_ref_width-1) downto 0);
  signal AR_done_p              : std_logic;
  signal Auto_Ref_issued        : std_logic;
  signal Auto_Ref_issued_p      : std_logic;
  signal BA_address_reg         : std_logic_vector((bank_address-1) downto 0);
  signal burst_length           : std_logic_vector(2 downto 0);
  signal burst_cnt_max          : std_logic_vector(2 downto 0);
  signal CAS_COUNT              : std_logic_vector(2 downto 0);
  signal COLUMN_ADDRESS_reg     : std_logic_vector((row_address-1) downto 0);
  signal command_reg            : std_logic_vector(3 downto 0);
  signal ddr_rasb1              : std_logic;
  signal ddr_casb1              : std_logic;
  signal ddr_web1               : std_logic;
  signal ddr_ba1                : std_logic_vector((bank_address-1) downto 0);
  signal ddr_address1           : std_logic_vector((row_address-1) downto 0);
  signal DQS_enable_out         : std_logic;
  signal DQS_reset_out          : std_logic;
  signal DLL_RST_COUNT_value    : std_logic_vector(7 downto 0);
  signal DLL_RST_COUNT          : std_logic_vector(7 downto 0);
  signal INIT_COUNT_value       : std_logic_vector(3 downto 0);
  signal INIT_COUNT             : std_logic_vector(3 downto 0);
  signal INIT_DONE              : std_logic;
  signal INIT_DONE_value        : std_logic;
  signal INIT_MEMORY            : std_logic;
  signal init_mem               : std_logic;
  signal initialize_memory      : std_logic;
  signal INIT_PRE_COUNT         : std_logic_vector(6 downto 0);
  signal REF_FREQ_CNT           : std_logic_vector((max_ref_width-1) downto 0);
  signal PRECHARGE_cmd          : std_logic;
  signal read_cmd               : std_logic;
  signal read_cmd1              : std_logic;
  signal read_cmd2              : std_logic;
  signal read_cmd3              : std_logic;
  signal RCDR_COUNT             : std_logic_vector(2 downto 0);
  signal RCDW_COUNT             : std_logic_vector(2 downto 0);
  signal RP_cnt_value           : std_logic_vector(2 downto 0);
  signal RFC_COUNT_reg          : std_logic;
  signal AR_Done_reg            : std_logic;
  signal rdburst_end_cnt        : std_logic_vector(2 downto 0);
  signal rdburst_end_1          : std_logic;
  signal rdburst_end_2          : std_logic;
  signal rdburst_end            : std_logic;
  signal RP_COUNT               : std_logic_vector(2 downto 0);
  signal RFC_COUNT              : std_logic_vector(5 downto 0);
  signal ROW_ADDRESS_reg        : std_logic_vector((row_address-1) downto 0);
  signal rst_dqs_div_r          : std_logic;
  signal rst_dqs_div_r1         : std_logic;
  signal wrburst_end_cnt        : std_logic_vector(2 downto 0);
  signal wrburst_end            : std_logic;
  signal wrburst_end_1          : std_logic;
  signal wrburst_end_2          : std_logic;
  signal wrburst_end_3          : std_logic;
  signal WR_COUNT               : std_logic_vector(2 downto 0);
  signal Write_enable_out       : std_logic;
  signal write_cmd_in           : std_logic;
  signal write_cmd2             : std_logic;
  signal write_cmd3             : std_logic;
  signal write_cmd1             : std_logic;
  signal GO_TO_ACTIVE_value     : std_logic;
  signal GO_TO_ACTIVE           : std_logic;
  signal dqs_div_cascount       : std_logic_vector(2 downto 0);
  signal dqs_div_rdburstcount   : std_logic_vector(2 downto 0);
  signal DQS_enable1            : std_logic;
  signal DQS_enable2            : std_logic;
  signal DQS_enable3            : std_logic;
  signal DQS_reset1_clk0        : std_logic;
  signal DQS_reset2_clk0        : std_logic;
  signal DQS_reset3_clk0        : std_logic;
  signal DQS_enable_int         : std_logic;
  signal DQS_reset_int          : std_logic;
  signal rst180_r               : std_logic;
  signal rst0_r                 : std_logic;
  signal EMR                    : std_logic_vector(row_address - 1 downto 0);
  signal LMR                    : std_logic_vector(row_address - 1 downto 0);
  signal LMR_DLL_rst            : std_logic_vector(row_address - 1 downto 0);
  signal LMR_DLL_set            : std_logic_vector(row_address - 1 downto 0);
  signal ddr_ODT1               : std_logic;
  signal ddr_ODT2               : std_logic;
  signal rst_dqs_div_int1       : std_logic;
  signal accept_cmd_in          : std_logic;
  signal dqs_enable_i           : std_logic;
  signal auto_ref_wait          : std_logic;
  signal auto_ref_wait1         : std_logic;
  signal auto_ref_wait2         : std_logic;
  signal rpCnt0                 : std_logic;
  signal address_reg            : std_logic_vector(((row_address +
                                                     col_ap_width)-1) downto 0);
  signal ddr_rasb2              : std_logic;
  signal ddr_casb2              : std_logic;
  signal ddr_web2               : std_logic;
  signal count6                 : std_logic_vector(5 downto 0);
  signal clk180                 : std_logic;
  signal odt_deassert           : std_logic; 
  constant cntnext     : std_logic_vector(5 downto 0)  := "101000";
  constant addr_const1 : std_logic_vector(13 downto 0) := "00010000000000";
  constant addr_const2 : std_logic_vector(13 downto 0) := "00001110000000";  --380
  constant addr_const3 : std_logic_vector(13 downto 0) := "00110001111111";  --C7F
  constant ba_const1   : std_logic_vector(2 downto 0)  := "010";
  constant ba_const2   : std_logic_vector(2 downto 0)  := "011";
  constant ba_const3   : std_logic_vector(2 downto 0)  := "001";

  attribute syn_preserve : boolean;
  attribute syn_keep : boolean;
  
  attribute syn_preserve of LMR_DLL_rst : signal is true;
  attribute syn_preserve of LMR_DLL_set : signal is true;
  attribute syn_preserve of BA_address_reg : signal is true;
  attribute syn_preserve of COLUMN_ADDRESS_reg : signal is true;
  attribute syn_preserve of ROW_ADDRESS_reg : signal is true;


begin
  
  clk180           <= not clk;
  EMR              <= ext_load_mode_register;                                    
  LMR		           <= load_mode_register;                                  
  LMR_DLL_rst	     <= LMR(row_address - 1 downto  9) & '1' & LMR(7 downto 0);    
  LMR_DLL_set      <= LMR(row_address - 1 downto  9) & '0' & LMR(7 downto 0);  
  

-- Input : COMMAND REGISTER FORMAT
--          0000  - NOP
--          0001  - Precharge
--          0010  - Initialize memory
--          0100  - Write Request
--          0110  - Read request


-- Input : Address format
-- row address  = address((row_address + col_ap_width) -1 downto col_ap_width)
-- column addrs = address(row_address-1 downto 0)

  ddr_csb_cntrl     <= '0';
  ddr_cke_cntrl     <= not wait_200us;
  init              <= init_done;
  ddr_rasb_cntrl    <= ddr_rasb2;
  ddr_casb_cntrl    <= ddr_casb2;
  ddr_web_cntrl     <= ddr_web2;
  rst_dqs_div_int   <= rst_dqs_div_int1;
  ddr_address_cntrl <= ddr_address1;
  ddr_ba_cntrl      <= ddr_ba1;


  process(clk)
  begin
    if clk'event and clk = '0' then
      rst180_r <= rst180;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '1' then
      rst0_r <= rst0;
    end if;
  end process;


--******************************************************************************
-- register input commands from the user
--******************************************************************************
  process(clk)
  begin
    if clk'event and clk = '0' then
        command_reg        <= command_register;
        ROW_ADDRESS_reg    <= address_reg(((row_address + col_ap_width)-1) downto
                                          col_ap_width);
        COLUMN_ADDRESS_reg <= address_reg((row_address-1) downto 11) & '0' & address_reg(9 downto 0);
        BA_address_reg     <= bank_addr;
        address_reg        <= address;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        burst_length <= "000";
      else
        burst_length <= LMR(2 downto 0);
      end if;
    end if;
  end process;

  process(clk)
  begin
    if (clk'event and clk = '0') then
      if rst180_r = '1' then
        accept_cmd_in <= '0';
      elsif (current_state = IDLE and ((rpCnt0 and RFC_COUNT_reg and
                                        not(auto_ref_wait) and
                                        not(Auto_Ref_issued)) = '1')) then
        accept_cmd_in <= '1';
      else
        accept_cmd_in <= '0';
      end if;
    end if;
  end process;
  PRECHARGE_cmd     <= '1' when (command_register = "0001" and
                             accept_cmd_in = '1') else '0';
  initialize_memory <= '1' when (command_register = "0010") else '0';
  write_cmd_in      <= '1' when (command_register = "0100" and
                            accept_cmd_in = '1') else '0';
  read_cmd          <= '1' when (command_reg = "0110" and
                        accept_cmd_in = '1') else '0';


--**************************************************************************
-- write_cmd is used to determine when there is a new write request
--**************************************************************************

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        write_cmd1 <= '0';
        write_cmd2 <= '0';
        write_cmd3 <= '0';
      else
        if (accept_cmd_in = '1') then
          write_cmd1 <= write_cmd_in;
        end if;
        write_cmd2 <= write_cmd1;
        write_cmd3 <= write_cmd2;
      end if;
    end if;
  end process;


--************************************************************************
-- register read cmd until READ command needs to be issued
--************************************************************************

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        read_cmd1    <= '0';
        read_cmd2    <= '0';
        read_cmd3    <= '0';
      else
        if (accept_cmd_in = '1') then
          read_cmd1 <= read_cmd;
        end if;
        read_cmd2    <= read_cmd1;
        read_cmd3    <= read_cmd2;
      end if;
    end if;
  end process;

--******************************************************************************
-- RFC Counter
-- an executable command can be issued only after Trfc(60 ns => 60/5 = 12 cycles
-- for 200MHz,
-- 60 ns => 60/3.75 = 16 cycles for 266MHz )
-- after a AUTOREFRESH command is issued
--******************************************************************************
  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        RFC_COUNT <= "000000";
      elsif(current_state = AUTO_REFRESH) then
        RFC_COUNT <= rfc_count_value;
      elsif(RFC_COUNT /= "000000") then
        RFC_COUNT <= RFC_COUNT - '1';
      end if;
    end if;
  end process;
--******************************************************************************
-- RP Counter
-- an executable command can be issued only after Trp(20 ns for a -5 device =>
-- 4 cycles)
-- after a PRECHARGE command is issued
--******************************************************************************
  RP_cnt_value <= "100" when (next_state = PRECHARGE) else
                  (RP_COUNT - "001") when (rpCnt0     /=     '1') else
                  "000";


  process(clk)
  begin
    if (clk'event and clk = '0') then
      if (rst180_r = '1') then
        RCDR_COUNT <= "000";
      elsif (current_state = ACTIVE) then
        RCDR_COUNT <= "001";          
      elsif (RCDR_COUNT /= "000") then
        RCDR_COUNT <= RCDR_COUNT - '1';
      end if;
    end if;
  end process;

  process(clk)
  begin
    if (clk'event and clk = '0') then
      if (rst180_r = '1') then
        RCDW_COUNT <= "000";
      elsif (current_state = ACTIVE) then
        RCDW_COUNT <= "001";           
      elsif (RCDW_COUNT /= "000") then
        RCDW_COUNT <= RCDW_COUNT - '1';
      end if;
    end if;
  end process;

--******************************************************************************
-- WR Counter
-- a PRECHARGE command can be applied only after 2 cycles after a WRITE command 
-- has finished executing
--******************************************************************************
  process (clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        WR_COUNT <= "000";
      else
        if (DQS_enable_int = '1') then
          WR_COUNT <= twr_count_value;
        elsif (WR_COUNT /= "000") then
          WR_COUNT <= WR_COUNT - "001"; 
        end if;
      end if;
    end if;
  end process;

--******************************************************************************
-- Auto refresh counter - the design uses AUTO REFRESH
-- the DDR SDRAM requires AUTO REFRESH cycles at an average interval of 7.8 us
-- Hence, a counter value to obtain a 7.7 us clock for Auto Refresh
-- (Refresh Request is raised for every 7.7 us to allow for termination of any 
-- ongoing bus transfer)
-- Hence for 200MHz frequency,The Refresh_count_value = freq * 
-- refresh_time_period = 200*7.7 = 1540
--******************************************************************************
  REF_FREQ_CNT <= max_ref_cnt;          -- AUTOREF_COUNT  (7.7 us)

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        AUTOREF_value <= '0';
      elsif (AUTOREF_COUNT = REF_FREQ_CNT) then
        AUTOREF_value <= '1';
      else
        AUTOREF_value <= '0';
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        AUTOREF_COUNT <= (others => '0');
      elsif (AUTOREF_value = '1') then
        AUTOREF_COUNT <= (others => '0');
      else
        AUTOREF_COUNT <= AUTOREF_COUNT + '1';
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        AUTO_REF_detect1   <= '0';
        AUTO_REF1          <= '0';
      else
        AUTO_REF_detect1   <= AUTOREF_value and INIT_DONE;
        AUTO_REF1          <= AUTO_REF_detect1;
      end if;
    end if;
  end process;

  AR_done_p <= '1' when AR_Done_reg = '1' else '0';

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        auto_ref_wait   <= '0';
        AR_done         <= '0';
        Auto_Ref_issued <= '0';
      else
        if (auto_ref1 = '1' and auto_ref_wait = '0') then
          auto_ref_wait <= '1';
        elsif (Auto_Ref_issued = '1') then
          auto_ref_wait <= '0';
        else
          auto_ref_wait <= auto_ref_wait;
        end if;
        AR_done         <= AR_done_p;
        Auto_Ref_issued <= Auto_Ref_issued_p;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        auto_ref_wait1 <= '0';
        auto_ref_wait2 <= '0';
        auto_ref       <= '0';        
      else
        if (Auto_Ref_issued_p = '1') then
          auto_ref_wait1 <= '0';
          auto_ref_wait2 <= '0';
          auto_ref       <= '0';
        else
          auto_ref_wait1 <= auto_ref_wait;
          auto_ref_wait2 <= auto_ref_wait1;
          auto_ref       <= auto_ref_wait2;
        end if;
      end if;
    end if;
  end process;

  auto_ref_req      <= auto_ref_wait;
  Auto_Ref_issued_p <= '1' when (current_state = AUTO_REFRESH) else '0';

--******************************************************************************
-- Common counter for the Initialization sequence
--******************************************************************************
  process(clk)
  begin
    if (clk'event and clk = '0') then
      if (rst180_r = '1') then
        count6 <= "000000";
      elsif(init_current_state = INIT_AUTO_REFRESH or init_current_state
            = INIT_PRECHARGE or init_current_state = INIT_LOAD_MODE_REG) then
        count6 <= cntnext;
      elsif(count6 /= "000000") then
        count6 <= count6 - '1';
      else
        count6 <= "000000";
      end if;
    end if;
  end process;

--******************************************************************************
-- Burst count value counter when there are cosecutive READs or WRITEs
-- While doing consecutive READs or WRITEs, the Burst_count value determines 
-- when the next READ or WRITE command should be issued. The burst length is 
-- determined while loading the Load Mode Register burst_cnt_max shows the 
-- number of clock cycles for each burst burst_cnt_max = 1 for a burst length 
-- of 2, since it is ddr
--******************************************************************************
  burst_cnt_max <= "010" when burst_length = "010" else
                   "100" when burst_length = "011" else
                   "000";

--******************************************************************************
-- CAS latency counter
-- CAS latencies of 3,4 can be set using Mode register bits M(6 downto 4)
-- M6 M5 M4 CAS latency
-- 0 1 1 - 3
-- 1 0 0 - 4
-- others - reserved
-- This design uses a CAS latency of 3 for a clock rate of 200 MHz
--******************************************************************************
  process(clk)
  begin
    if (clk'event and clk = '0') then
      if (rst180_r = '1') then
        CAS_COUNT <= "000";
      elsif(current_state = BURST_READ) then
        CAS_COUNT <= burst_cnt_max - '1';
      elsif(CAS_COUNT /= "000") then
        CAS_COUNT <= CAS_COUNT - '1';
      end if;
    end if;
  end process;

  process(clk)
  begin
    if (clk'event and clk = '0') then
      if (rst180_r = '1') then
        RDBURST_END_CNT <= "000";
      elsif (CAS_COUNT = "001") then
        RDBURST_END_CNT <= burst_cnt_max;
      elsif (RDBURST_END_CNT /= "000") then
        RDBURST_END_CNT <= RDBURST_END_CNT - '1';
      end if;
    end if;
  end process;

  process(clk)
  begin
    if (clk'event and clk = '0') then
      if (rst180_r = '1') then
        wrburst_end_cnt <= "000";
      elsif ((current_state = FIRST_WRITE) or (current_state = BURST_WRITE)) then
        wrburst_end_cnt <= burst_cnt_max;
      elsif (wrburst_end_cnt /= "000") then
        wrburst_end_cnt <= wrburst_end_cnt - '1';
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        rdburst_end_1 <= '0';
      else
        if(burst_done = '1') then       
          rdburst_end_1 <= '1';
        else
          rdburst_end_1 <= '0';
        end if;
        rdburst_end_2 <= rdburst_end_1;
      end if;
    end if;
  end process;

  rdburst_end <= rdburst_end_1 or rdburst_end_2;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        wrburst_end_1 <= '0';
      else
        if (burst_done = '1') then      
          wrburst_end_1 <= '1';
        else
          wrburst_end_1 <= '0';
        end if;
        wrburst_end_2 <= wrburst_end_1;
        wrburst_end_3 <= wrburst_end_2;
      end if;
    end if;
  end process;

  wrburst_end <= wrburst_end_1 or wrburst_end_2 or wrburst_end_3;

--******************************************************************************
-- To generate the Data Strobe enable and reset signal
-- The DQS signal needs to be generated center aligned with the data.
-- The controller generates the DQS enable signal when the state machine 
-- is in the FIRST_WRITE state,to take care of the write preamble
--******************************************************************************
  DQS_enable_out <= '1' when ((current_state = FIRST_WRITE) or
                              (current_state = BURST_WRITE) or
                              (WRburst_end_cnt /= "000")) else '0';
  DQS_reset_out <= '1' when current_state = FIRST_WRITE else '0';
  dqs_enable    <= dqs_enable_i;
  
    dqs_enable_i <= DQS_enable2;
    dqs_reset    <= DQS_reset2_clk0;

  

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        DQS_enable_int <= '0';
        DQS_reset_int  <= '0';
      else
        DQS_enable_int <= DQS_enable_out;
        DQS_reset_int  <= DQS_reset_out;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '1' then
      if rst0_r = '1' then
        DQS_enable1     <= '0';
        DQS_enable2     <= '0';
        DQS_enable3     <= '0';
        DQS_reset1_clk0 <= '0';
        DQS_reset2_clk0 <= '0';
        DQS_reset3_clk0 <= '0';
      else
        DQS_enable1     <= DQS_enable_int;
        DQS_enable2     <= DQS_enable1;
        DQS_enable3     <= DQS_enable2;
        DQS_reset1_clk0 <= DQS_reset_int;
        DQS_reset2_clk0 <= DQS_reset1_clk0;
        DQS_reset3_clk0 <= DQS_reset2_clk0;
      end if;
    end if;
  end process;
--******************************************************************************
--Generating WRITE and READ enable signals
--******************************************************************************

  write_enable_out <= '1' when (wrburst_end_cnt /= "000")else '0';


  
  
  process(clk)
  begin
   if clk'event and clk = '0' then
    if rst180_r = '1' then
     write_enable <= '0';
    else
     write_enable <= write_enable_out;
    end if;
   end if;
  end process;

  cmd_ack <= ack_reg;
  ACK_REG_INST1 : FD port map (
    Q => ack_reg,
    D => ack_o,
    C => clk180);

  ack_o <= '1' when ((write_cmd_in = '1') or (write_cmd1 = '1') or
                     (read_cmd = '1') or (read_cmd1 = '1')) else  '0';

--******************************************************************************
-- To initialize memory
--******************************************************************************

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        init_memory <= '0';
        init_done   <= '0';
      else
        init_memory <= init_mem;
        if ((init_done_value = '1')and (INIT_COUNT = "1011")) then
          init_done <= '1';
        else
          init_done <= '0';
        end if;
      end if;
    end if;
  end process;

  process (clk)
  begin
    if clk'event and clk = '0' then
      if initialize_memory = '1' or rst180_r = '1' then
        INIT_PRE_COUNT <= "1010000";
      else
        INIT_PRE_COUNT <= INIT_PRE_COUNT - "0000001"; 
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        init_mem <= '0';
      elsif (initialize_memory = '1') then
        init_mem <= '1';
      elsif ((INIT_COUNT = "1011") and (count6 = "000001")) then
        init_mem <= '0';
      else
        init_mem <= init_mem;
      end if;
    end if;
  end process;

-- Counter for Memory Initialization sequence

  INIT_COUNT_value <= (INIT_COUNT + "0001") when
                      ((init_current_state = INIT_PRECHARGE) or
                       (init_current_state = INIT_LOAD_MODE_REG) or
                       (init_current_state = INIT_AUTO_REFRESH)) 
                      else INIT_COUNT;


  INIT_DONE_value <= '1' when ((INIT_COUNT = "1011") and
                               (DLL_RST_COUNT = "00000001")) else '0';



--Counter for DLL Reset complete
  DLL_RST_COUNT_value <= "11001000" when (INIT_COUNT = "0011") else  --200
                         (DLL_RST_COUNT - "00000001") when 
                         (DLL_RST_COUNT /= "00000001") else "00000001";


-- State machine goes to ACTIVE state directly with every read command
  GO_TO_ACTIVE_value <= '1' when ((write_cmd_in = '1') and (write_cmd1 /= '1'))
                        or ((read_cmd = '1') and (read_cmd1 /= '1'))
                        else '0';

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        GO_TO_ACTIVE <= '0';
      else
        GO_TO_ACTIVE <= GO_TO_ACTIVE_value;
      end if;
    end if;
  end process;

--******************************************************************************
-- Register counter values
--******************************************************************************
  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        INIT_COUNT    <= "0000";
        DLL_RST_COUNT <= "00000000";
        RP_COUNT      <= "000";
        AR_Done_reg   <= '0';
        RFC_COUNT_reg <= '0';
        rpCnt0        <= '1';
      else
        if(RP_COUNT(2) = '0' and RP_COUNT(1) = '0' and RP_CNT_value(2) = '0') then
          rpCnt0 <= '1';
        else
          rpCnt0 <= '0';
        end if;
        INIT_COUNT    <= INIT_COUNT_value;
        DLL_RST_COUNT <= DLL_RST_COUNT_value;
        RP_COUNT      <= RP_CNT_value;
        if(RFC_COUNT = "000010") then   --2
          AR_Done_reg <= '1';
        else
          AR_Done_reg <= '0';
        end if;
        if(AR_Done_reg = '1') then
          RFC_COUNT_reg <= '1';
        elsif (INIT_DONE = '1' and init_mem = '0' and RFC_COUNT = "000000")
        then
          RFC_COUNT_reg <= '1';
        elsif (Auto_Ref_issued = '1') then
          RFC_COUNT_reg <= '0';
        else
          RFC_COUNT_reg <= RFC_COUNT_reg;
        end if;
      end if;
    end if;
  end process;

--******************************************************************************
-- To check current state for the address bus
--******************************************************************************
  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        current_state <= IDLE;
      else
        current_state <= next_state;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        init_current_state <= INIT_IDLE;
      else
        init_current_state <= init_next_state;
      end if;
    end if;
  end process;

--******************************************************************************
-- Initialization state machine
--******************************************************************************

  process (rst180_r, INIT_COUNT, init_current_state, INIT_MEMORY, count6,
           INIT_PRE_COUNT)
  begin
    if rst180_r = '1' then
      init_next_state <= INIT_IDLE;
    else
      case init_current_state is
        when INIT_IDLE =>
          if INIT_MEMORY = '1' then
            case INIT_COUNT is
              when "0000" =>
                if(INIT_PRE_COUNT = "0000001") then
                  init_next_state <= INIT_PRECHARGE;
                else
                  init_next_state <= INIT_IDLE;
                end if;
                
              when "0001" =>
                if (count6 = "000001") then
                  init_next_state <= INIT_LOAD_MODE_REG;
                else
                  init_next_state <= INIT_IDLE;
                end if;
              when "0010" =>
                -- for reseting DLL in Base Mode register
                if (count6 = "000001") then
                  init_next_state <= INIT_LOAD_MODE_REG;
                else
                  init_next_state <= INIT_IDLE;
                end if;
              when "0011" =>
                if (count6 = "000001") then
                  init_next_state <= INIT_LOAD_MODE_REG;  -- For EMR
                else
                  init_next_state <= INIT_IDLE;
                end if;
              when "0100" =>
                if (count6 = "000001") then
                  init_next_state <= INIT_LOAD_MODE_REG;  -- For EMR
                else
                  init_next_state <= INIT_IDLE;
                end if;
              when "0101" =>
                if (count6 = "000001") then
                  init_next_state <= INIT_PRECHARGE;
                else
                  init_next_state <= INIT_IDLE;
                end if;

              when "0110" =>
                if (count6 = "000001") then
                  init_next_state <= INIT_AUTO_REFRESH;
                else
                  init_next_state <= INIT_IDLE;
                end if;

              when "0111" =>
                if (count6 = "000001") then
                  init_next_state <= INIT_AUTO_REFRESH;
                else
                  init_next_state <= INIT_IDLE;
                end if;
              when "1000" =>
                -- to deactivate the rst DLL bit in the LMR
                if (count6 = "000001") then
                  init_next_state <= INIT_LOAD_MODE_REG;
                else
                  init_next_state <= INIT_IDLE;
                end if;
              when "1001" =>
                -- to set OCD to default in EMR
                if (count6 = "000001") then
                  init_next_state <= INIT_LOAD_MODE_REG;
                else
                  init_next_state <= INIT_IDLE;
                end if;
              when "1010" =>
                if (count6 = "000001") then
                  init_next_state <= INIT_LOAD_MODE_REG;  --  OCD exit in EMR
                else
                  init_next_state <= INIT_IDLE;
                end if;
              when "1011" =>
                if (count6 = "000001") then
                  init_next_state <= INIT_IDLE;
                else
                  init_next_state <= init_current_state;
                end if;
              when others =>
                init_next_state <= INIT_IDLE;
            end case;

          else
            init_next_state <= INIT_IDLE;
          end if;

        when INIT_PRECHARGE =>
          init_next_state <= INIT_IDLE;

        when INIT_LOAD_MODE_REG =>
          init_next_state <= INIT_IDLE;

        when INIT_AUTO_REFRESH =>
          init_next_state <= INIT_IDLE;

        when others =>
          init_next_state <= INIT_IDLE;
      end case;
    end if;
  end process;

--******************************************************************************
-- MAIN state machine
--******************************************************************************

  process (rst180_r, RP_COUNT, RFC_COUNT, CAS_COUNT, WR_COUNT,
           GO_TO_ACTIVE, write_cmd1, read_cmd3, current_state,
           wrburst_end, wrburst_end_cnt, burst_length,
           rdburst_end, INIT_MEMORY, RCDW_COUNT, RCDR_COUNT, PRECHARGE_CMD,
           auto_ref, RFC_COUNT_reg, rpCnt0, burst_done)
  begin
    if rst180_r = '1' then
      next_state <= IDLE;
    else
      case current_state is
        when IDLE =>
          if (INIT_MEMORY = '0') then
            if(auto_ref = '1' and RFC_COUNT_reg = '1' and rpCnt0 = '1') then
              next_state <= AUTO_REFRESH;  -- normal Refresh in the IDLE state
            elsif PRECHARGE_CMD = '1' then
              next_state <= PRECHARGE;
            elsif GO_TO_ACTIVE = '1' then
              next_state <= ACTIVE;
            else
              next_state <= IDLE;
            end if;
          else
            next_state <= IDLE;
          end if;

        when PRECHARGE =>
          next_state <= IDLE;

        when AUTO_REFRESH =>
          next_state <= IDLE;

        when ACTIVE =>
          next_state <= ACTIVE_WAIT;

        when ACTIVE_WAIT =>
          if (RCDW_COUNT = "000" and write_cmd1 = '1') then
            next_state <= FIRST_WRITE;
          elsif (RCDR_COUNT = "000" and read_cmd3 = '1') then
            next_state <= BURST_READ;
          else
            next_state <= ACTIVE_WAIT;
          end if;

        when FIRST_WRITE =>
          next_state <= WRITE_WAIT;

        when WRITE_WAIT =>
          case wrburst_end is
            when '1' =>
              next_state <= PRECHARGE_AFTER_WRITE;

            when '0' =>
              if WRburst_end_cnt = "010" then
                next_state <= BURST_WRITE;
              else
                next_state <= WRITE_WAIT;
              end if;
            when others =>
              next_state <= WRITE_WAIT;

          end case;

        when BURST_WRITE =>
          next_state <= WRITE_WAIT;


        when PRECHARGE_AFTER_WRITE =>
          next_state <= PRECHARGE_AFTER_WRITE_2;

        when PRECHARGE_AFTER_WRITE_2 =>
          if(WR_COUNT = "00") then
            next_state <= PRECHARGE;
          else
            next_state <= PRECHARGE_AFTER_WRITE_2;
          end if;

        when READ_WAIT =>
          case rdburst_end is
            when '1' =>
              next_state <= PRECHARGE_AFTER_WRITE;
            when '0' =>
              if CAS_COUNT = "001" then
                next_state <= BURST_READ;
              else
                next_state <= READ_WAIT;
              end if;
            when others =>
              next_state <= READ_WAIT;
          end case;

        when BURST_READ =>
          next_state <= READ_WAIT;

        when others =>
          next_state <= IDLE;
      end case;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        ddr_address1 <= (others => '0');
      elsif (INIT_MEM = '1') then
        case (INIT_COUNT) is
          when "0000" | "0101" =>
            ddr_address1 <= addr_const1((row_address - 1) downto 0);
          when "0001" =>
            ddr_address1 <= (others => '0');
          when "0010" =>
            ddr_address1 <= (others => '0');
          when "0011" =>
            ddr_address1 <= EMR;
          when "0100" =>
            ddr_address1 <= LMR_DLL_RST;
          when "1000" =>
            ddr_address1 <= LMR_DLL_SET;
          when "1001" =>
            ddr_address1 <= EMR or addr_const2((row_address - 1) downto 0);
          when "1010" =>
            ddr_address1 <= EMR and addr_const3((row_address - 1) downto 0);
          when others =>
            ddr_address1 <= (others => '0');
        end case;
      elsif (current_state = PRECHARGE) then
        ddr_address1 <= addr_const1((row_address - 1) downto 0);
      elsif (current_state = ACTIVE) then
        ddr_address1 <= row_address_reg;
      elsif (current_state = BURST_WRITE or current_state = FIRST_WRITE or
             current_state = BURST_READ) then
        ddr_address1 <= column_address_reg((row_address - 1) downto 0);
      else
        ddr_address1 <= (others => '0');
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        ddr_ba1 <= (others => '0');
      elsif (INIT_MEM = '1') then
        case (INIT_COUNT) is
          when "0001" =>
            ddr_ba1 <= ba_const1((bank_address -1) downto 0);
          when "0010" =>
            ddr_ba1 <= ba_const2((bank_address -1) downto 0);
          when "0011" | "1001" | "1010" =>
            ddr_ba1 <= ba_const3((bank_address -1) downto 0);
          when others =>
            ddr_ba1 <= (others => '0');
        end case;
      elsif (current_state = ACTIVE or current_state = FIRST_WRITE or
             current_state = BURST_WRITE or current_state = BURST_READ) then
        ddr_ba1 <= BA_address_reg;
      else
        ddr_ba1 <= (others => '0');
      end if;
    end if;
  end process;


 process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        odt_deassert <= '0';
      elsif(wrburst_end_3 = '1') then
        odt_deassert <= '1';
      elsif(write_cmd3 = '0') then
        odt_deassert <= '0';
      else 
        odt_deassert <= odt_deassert;
      end if;
    end if;
  end process;


  ddr_ODT1 <= '1' when (write_cmd3 = '1' and (EMR(6) = '1' or
                                              EMR(2) = '1') and odt_deassert = '0') else '0';

--******************************************************************************
--Pipeline stages for ddr_address and ddr_ba
--******************************************************************************

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        ddr_ODT2  <= '0';
        ddr_rasb2 <= '1';
        ddr_casb2 <= '1';
        ddr_web2  <= '1';
      else
        ddr_ODT2     <= ddr_ODT1;
        ddr_rasb2    <= ddr_rasb1;
        ddr_casb2    <= ddr_casb1;
        ddr_web2     <= ddr_web1;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if clk'event and clk = '0' then
      if rst180_r = '1' then
        ddr_ODT_cntrl <= '0';
      else
        ddr_ODT_cntrl <= ddr_ODT2;
      end if;
    end if;
  end process;

--******************************************************************************
-- control signals to the Memory
--******************************************************************************

  ddr_rasb1 <= '0' when (current_state = ACTIVE or current_state = PRECHARGE or
                         current_state = AUTO_REFRESH or
                         init_current_state = INIT_PRECHARGE or
                         init_current_state = INIT_AUTO_REFRESH or
                         init_current_state = INIT_LOAD_MODE_REG) else   '1';

  ddr_casb1 <= '0' when (current_state = BURST_READ or
                         current_state = BURST_WRITE or
                         current_state = FIRST_WRITE or
                         current_state = AUTO_REFRESH or
                         init_current_state = INIT_AUTO_REFRESH or
                         init_current_state = INIT_LOAD_MODE_REG) else  '1';

  ddr_web1 <= '0' when (current_state = BURST_WRITE or
                        current_state = FIRST_WRITE or
                        current_state = PRECHARGE or
                        init_current_state = INIT_PRECHARGE or
                        init_current_state = INIT_LOAD_MODE_REG) else   '1';

-------------------------------------------------------------------------------

  
  process(clk)
  begin
    if(clk'event and clk = '0') then
      if rst180_r = '1' then
        dqs_div_cascount <= "000";
      else
        if(ddr_rasb2 = '1' and ddr_casb2 = '0' and ddr_web2 = '1') then
          dqs_div_cascount <= burst_cnt_max;
        else
          if dqs_div_cascount /= "000" then
            dqs_div_cascount <= dqs_div_cascount - "001";  
          else
            dqs_div_cascount <= dqs_div_cascount;
          end if;
        end if;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if(clk'event and clk = '0') then
      if rst180_r = '1' then
        dqs_div_rdburstcount <= "000";
      else
        if (dqs_div_cascount = "001" and burst_length = "010") then
          dqs_div_rdburstcount <= "010";  --100
        elsif (dqs_div_cascount = "011" and burst_length = "011") then
          dqs_div_rdburstcount <= "100";  --100
        else
          if dqs_div_rdburstcount /= "000" then
            dqs_div_rdburstcount <= dqs_div_rdburstcount - "001";  
          else
            dqs_div_rdburstcount <= dqs_div_rdburstcount;
          end if;
        end if;
      end if;
    end if;
  end process;

  process(clk)
  begin
    if(clk'event and clk = '0') then
      if rst180_r = '1' then
        rst_dqs_div_r <= '0';
      else
        if (dqs_div_cascount = "001" and burst_length = "010")then     --011
          rst_dqs_div_r <= '1';
        elsif (dqs_div_cascount = "011" and burst_length = "011")then  --011
          rst_dqs_div_r <= '1';
        else
          if (dqs_div_rdburstcount = "001" and dqs_div_cascount = "000") then
            rst_dqs_div_r <= '0';
          else
            rst_dqs_div_r <= rst_dqs_div_r;
          end if;
        end if;
      end if;
    end if;
  end process;

  process(clk)                          -- For Reg dimm
  begin
    if(clk'event and clk = '0') then
      rst_dqs_div_r1 <= rst_dqs_div_r;
    end if;
  end process;

  process (clk)
  begin
    if (clk'event and clk = '0') then
      if (dqs_div_cascount /= "000" or dqs_div_rdburstcount /= "000") then
        rst_calib <= '1';
      else
        rst_calib <= '0';
      end if;
    end if;
  end process;
  
  rst_iob_out : FD 
    port map (
      Q => rst_dqs_div_int1,
      D    => rst_dqs_div_r, 
      C => clk);

end arc;
