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
-- Description: contains declarations valid for every source
-- Dependencies: none
-- 
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.std_logic_1164.all;
--use IEEE.numeric_std.all;
use IEEE.std_logic_arith.all;

---------------------------------------------------------------------------------------------------
package Global is
function To_Integer(Arg : in std_ulogic_vector) return integer;
function To_Integer(Arg : in std_ulogic) return integer;
function TO_UNSIGNED (ARG, SIZE: natural) return UNSIGNED;

constant oscillator : integer := 50000000;
constant multiplicator : integer := 25;
constant divisor : integer := 20;
constant theClock : integer := oscillator * multiplicator / divisor;


constant BitWidth      : integer := 32;
constant IndexBitWidth : integer := 8;

subtype BinRange     is natural range 0 to 1;
subtype DataRange    is natural range (BitWidth-1) downto 0;
subtype IndexRange   is natural range (IndexBitWidth-1) downto 0;
subtype IndexValues  is natural range (2**IndexBitWidth-1) downto 0;
subtype RAMrange     is natural range 25 downto 0;
subtype ROMrange     is natural range 14 downto 2;
subtype DataVec      is std_ulogic_vector(DataRange);

subtype InputOrder_t is std_ulogic_vector(1 downto 0);
constant ImIm: InputOrder_t := "00";
constant FIm: InputOrder_t := "01";
constant ImF: InputOrder_t := "10";
constant SF: InputOrder_t := "11";
---------------------------------------------------------------------------------------------------
-- ALU operations
---------------------------------------------------------------------------------------------------
subtype AluFuncType  is std_ulogic_vector(4 downto 0);

constant AddALUnC  : AluFuncType := "00000";
constant SubALUnC  : AluFuncType := "00010";
constant AddALU    : AluFuncType := "00100";
constant SubALU    : AluFuncType := "00110";
constant AddcALU   : AluFuncType := "00101";
constant SubcALU   : AluFuncType := "00111";

constant LtALU     : AluFuncType := "01000";
constant GtALU     : AluFuncType := "01001";
constant EquALU    : AluFuncType := "01010";
constant NequALU   : AluFuncType := "01011";
constant UltALU    : AluFuncType := "01100";
constant UgtALU    : AluFuncType := "01101";

constant LshiftALU : AluFuncType := "10000";
constant LshiftCALU: AluFuncType := "10001";
constant RshiftALU : AluFuncType := "10010";
constant RshiftCALU: AluFuncType := "10011";
constant RshiftAALU: AluFuncType := "10100";

constant AndALU    : AluFuncType := "11000";
constant OrALU     : AluFuncType := "11010";
constant XorALU    : AluFuncType := "11011";
constant FPUcode   : AluFuncType := "11001";
constant Prod32    : AluFuncType := "11100";
constant Op64      : AluFuncType := "11101";

---------------------------------------------------------------------------------------------------
-- Stack operations
---------------------------------------------------------------------------------------------------
subtype StackFuncType is std_ulogic_vector(2 downto 0);

constant NopStack           : StackFuncType := "000";
constant PushStack          : StackFuncType := "001";
constant GetPut             : StackFuncType := "010";
constant SetSPStack         : StackFuncType := "011";
constant LoadStore          : StackFuncType := "100";
constant SaveStack          : StackFuncType := "101";
constant SwitchCore         : StackFuncType := "110";

---------------------------------------------------------------------------------------------------
-- Opcodes
---------------------------------------------------------------------------------------------------
constant opcNOP       : std_ulogic_vector(6 downto 0) := "0000000";
constant opcSTORE     : std_ulogic_vector(6 downto 0) := "0000001";
constant opcLOAD      : std_ulogic_vector(6 downto 0) := "0000010";
constant opcSTOREC    : std_ulogic_vector(6 downto 0) := "0000011";
constant opcLOADC     : std_ulogic_vector(6 downto 0) := "0000100";
constant opcSTOREH    : std_ulogic_vector(6 downto 0) := "0000101";
constant opcLOADH     : std_ulogic_vector(6 downto 0) := "0000110";
constant opcDEPTH     : std_ulogic_vector(6 downto 0) := "0000111";
constant opcDROP      : std_ulogic_vector(6 downto 0) := "0001000";
constant opc2DROP     : std_ulogic_vector(6 downto 0) := "0001001";
constant opcNIP       : std_ulogic_vector(6 downto 0) := "0001010";
constant opcPICK      : std_ulogic_vector(6 downto 0) := "0001011";
constant opcPUT       : std_ulogic_vector(6 downto 0) := "0001100";
constant opcVALUE     : std_ulogic_vector(6 downto 0) := "0001101";
constant opcS0        : std_ulogic_vector(6 downto 0) := "0001110";
constant opcS1        : std_ulogic_vector(6 downto 0) := "0001111";
constant opcSA0       : std_ulogic_vector(6 downto 0) := "0010000";
constant opcSA1       : std_ulogic_vector(6 downto 0) := "0010001";
constant opcSAVE      : std_ulogic_vector(6 downto 0) := "0010010";
constant opcHALT      : std_ulogic_vector(6 downto 0) := "0010011";
constant opcSETSP     : std_ulogic_vector(6 downto 0) := "0010100";
constant opcGETSP     : std_ulogic_vector(6 downto 0) := "0010101";
constant opcADD       : std_ulogic_vector(6 downto 0) := "0010110";
constant opcSUB       : std_ulogic_vector(6 downto 0) := "0010111";
constant opcINC       : std_ulogic_vector(6 downto 0) := "0011000";
constant opcDEC       : std_ulogic_vector(6 downto 0) := "0011001";
constant opc2MUL      : std_ulogic_vector(6 downto 0) := "0011010";
constant opc2DIV      : std_ulogic_vector(6 downto 0) := "0011011";
constant opcAND       : std_ulogic_vector(6 downto 0) := "0011100";
constant opcCELLP     : std_ulogic_vector(6 downto 0) := "0011101";
constant opcHALFP     : std_ulogic_vector(6 downto 0) := "0011110";
constant opcINVERT    : std_ulogic_vector(6 downto 0) := "0011111";
constant opcLSHIFT    : std_ulogic_vector(6 downto 0) := "0100000";
constant opcNEGATE    : std_ulogic_vector(6 downto 0) := "0100001";
constant opcOR        : std_ulogic_vector(6 downto 0) := "0100010";
constant opcRSHIFT    : std_ulogic_vector(6 downto 0) := "0100011";
constant opcXOR       : std_ulogic_vector(6 downto 0) := "0100100";
constant opcADC       : std_ulogic_vector(6 downto 0) := "0100101";
constant opcSBC       : std_ulogic_vector(6 downto 0) := "0100110";
constant opcLSHIFTC   : std_ulogic_vector(6 downto 0) := "0100111";
constant opcRSHIFTC   : std_ulogic_vector(6 downto 0) := "0101000";
constant opc0LTN      : std_ulogic_vector(6 downto 0) := "0101001";
constant opc0LT       : std_ulogic_vector(6 downto 0) := "0101010";
constant opc0EQN      : std_ulogic_vector(6 downto 0) := "0101011";
constant opc0EQ       : std_ulogic_vector(6 downto 0) := "0101100";
--constant opcLTN       : std_ulogic_vector(6 downto 0) := "0101101";
constant opcCMP       : std_ulogic_vector(6 downto 0) := "0101101";
constant opcCONST10M  : std_ulogic_vector(6 downto 0) := "0101110";
--constant opcEQN       : std_ulogic_vector(6 downto 0) := "0101111";
constant opcCONST15M  : std_ulogic_vector(6 downto 0) := "0101111";
constant opcCONST5M   : std_ulogic_vector(6 downto 0) := "0110000";
--constant opcGTN       : std_ulogic_vector(6 downto 0) := "0110001";
constant opcCONST14M  : std_ulogic_vector(6 downto 0) := "0110001";
constant opcCONST9M   : std_ulogic_vector(6 downto 0) := "0110010";
--constant opcULTN      : std_ulogic_vector(6 downto 0) := "0110011";
constant opcCONST13M  : std_ulogic_vector(6 downto 0) := "0110011";
constant opcCONST8M   : std_ulogic_vector(6 downto 0) := "0110100";
constant opc0NEN      : std_ulogic_vector(6 downto 0) := "0110101";
constant opc0NE       : std_ulogic_vector(6 downto 0) := "0110110";
constant opc0GTN      : std_ulogic_vector(6 downto 0) := "0110111";
constant opc0GT       : std_ulogic_vector(6 downto 0) := "0111000";
--constant opcNEN       : std_ulogic_vector(6 downto 0) := "0111001";
constant opcCONST12M  : std_ulogic_vector(6 downto 0) := "0111001";
constant opcCONST7M   : std_ulogic_vector(6 downto 0) := "0111010";
--constant opcUGTN      : std_ulogic_vector(6 downto 0) := "0111011";
constant opcCONST11M  : std_ulogic_vector(6 downto 0) := "0111011";
constant opcCONST6M   : std_ulogic_vector(6 downto 0) := "0111100";
constant opcEXIT      : std_ulogic_vector(6 downto 0) := "0111101";
constant opcCALL      : std_ulogic_vector(6 downto 0) := "0111110";
constant opcTRAP      : std_ulogic_vector(6 downto 0) := "0111111";
constant opcBRANCH    : std_ulogic_vector(6 downto 0) := "1000000";
constant opc0BRANCHN  : std_ulogic_vector(6 downto 0) := "1000001";
constant opc0BRANCH   : std_ulogic_vector(6 downto 0) := "1000010";
constant opcCARRY     : std_ulogic_vector(6 downto 0) := "1000011";
constant opcCARRYST   : std_ulogic_vector(6 downto 0) := "1000100";
constant opcBREAK     : std_ulogic_vector(6 downto 0) := "1000101";
constant opcVALH      : std_ulogic_vector(6 downto 0) := "1000110";
constant opcCONST15   : std_ulogic_vector(6 downto 0) := "1000111";
constant opcCONST14   : std_ulogic_vector(6 downto 0) := "1001000";
constant opcCONST13   : std_ulogic_vector(6 downto 0) := "1001001";
constant opcCONST12   : std_ulogic_vector(6 downto 0) := "1001010";
constant opcCONST11   : std_ulogic_vector(6 downto 0) := "1001011";
constant opcCONST10   : std_ulogic_vector(6 downto 0) := "1001100";
constant opcCONST9    : std_ulogic_vector(6 downto 0) := "1001101";
constant opcCONST8    : std_ulogic_vector(6 downto 0) := "1001110";
constant opcCONST7    : std_ulogic_vector(6 downto 0) := "1001111";
constant opcCONST6    : std_ulogic_vector(6 downto 0) := "1010000";
constant opcCONST5    : std_ulogic_vector(6 downto 0) := "1010001";
constant opcCONST4    : std_ulogic_vector(6 downto 0) := "1010010";
constant opcCONST3    : std_ulogic_vector(6 downto 0) := "1010011";
constant opcCONST2    : std_ulogic_vector(6 downto 0) := "1010100";
constant opcCONST1    : std_ulogic_vector(6 downto 0) := "1010101";
constant opcCONST0    : std_ulogic_vector(6 downto 0) := "1010110";
constant opcCONST1M   : std_ulogic_vector(6 downto 0) := "1010111";
constant opcCONST2M   : std_ulogic_vector(6 downto 0) := "1011000";
constant opcCONST3M   : std_ulogic_vector(6 downto 0) := "1011001";
constant opcCONST4M   : std_ulogic_vector(6 downto 0) := "1011010";
constant opcU0LT      : std_ulogic_vector(6 downto 0) := "1011011";
constant opcU0GT      : std_ulogic_vector(6 downto 0) := "1011100";
constant opcFPU       : std_ulogic_vector(6 downto 0) := "1011101";
constant opcSETPC     : std_ulogic_vector(6 downto 0) := "1011110";
constant opcSWITCH    : std_ulogic_vector(6 downto 0) := "1011111";
constant opcGETCOREID : std_ulogic_vector(6 downto 0) := "1100000";
constant opcOP64      : std_ulogic_vector(6 downto 0) := "1100001";
constant opcProd32    : std_ulogic_vector(6 downto 0) := "1100010";


end Global;


---------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------
package body Global is
function To_Integer(Arg : in std_ulogic) return integer is
begin
  return CONV_INTEGER(Arg);
end;


---------------------------------------------------------------------------------------------------
function To_Integer(Arg : in std_ulogic_vector) return integer is
begin
 return CONV_INTEGER(unsigned(Arg));
end;

---------------------------------------------------------------------------------------------------
  function TO_UNSIGNED (ARG, SIZE: natural) return UNSIGNED is
    variable RESULT: UNSIGNED(SIZE-1 downto 0);
    variable I_VAL: NATURAL := ARG;
  begin
    for I in 0 to RESULT'LEFT loop
      if (I_VAL mod 2) = 0 then
        RESULT(I) := '0';
      else RESULT(I) := '1';
      end if;
      I_VAL := I_VAL/2;
    end loop;
    return RESULT;
  end TO_UNSIGNED;

end Global;

---------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------
