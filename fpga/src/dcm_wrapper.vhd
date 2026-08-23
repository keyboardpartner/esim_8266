----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    10:12:33 08/15/2026 
-- Design Name: 
-- Module Name:    dcm_wrapper - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity dcm_wrapper is
    Port ( CLK_50_IN : in  STD_LOGIC;
           CLK_100_OUT : out  STD_LOGIC;
           CLK_50_OUT : out  STD_LOGIC);
end dcm_wrapper;

architecture Behavioral of dcm_wrapper is

component dcm_x2
port
 (-- Clock in ports
  CLK_IN1           : in     std_logic;
  -- Clock out ports
  CLK_OUT1          : out    std_logic;
  CLK_OUT2          : out    std_logic
 );
 
end component;

begin

dcm_50_100 : dcm_x2
port map
   (-- Clock in ports
    CLK_IN1 => CLK_50_IN,
    -- Clock out ports
    CLK_OUT1 => CLK_100_OUT,
    CLK_OUT2 => CLK_50_OUT
);


end Behavioral;

