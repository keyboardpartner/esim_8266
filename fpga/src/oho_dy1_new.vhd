----------------------------------------------------------------------------------
-- Company:        OHO-Elektronik
-- Engineer:       M.Randelzhofer
-- 
-- Create Date:    17.01.09 
-- Design Name: 
-- Module Name:    Demo_Dy1 - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 1.00 - File Created

-- Additional Comments: 
-- Revision 1.01 - Brightness support
-- Revision 1.02 - Added display test
-- Revision 1.03 - display update support, new interface signal names
--
----------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all ;
use work.OhoPack.all ;


entity demo_dy1_new is
Port (
  SYSCLK50:    in  std_logic ;
  LED_BLINK:   out  std_logic ;
  BLINK_COUNT: out  std_logic_vector(7 downto 0);
  TICK_12HZ:   out  std_logic ; -- 11,71875 Hz
--  DEBUG_ADDR:  in std_logic_vector(15 downto 0); -- erstes Display
--  DEBUG_DATA:  in std_logic_vector(7 downto 0);  -- zweites Display
--  DEBUG_LOADING:  in std_logic;  -- Dezimalpunkt
  DIGIT_L:      in std_logic_vector(3 downto 0) ;
  DIGIT_M:      in std_logic_vector(3 downto 0) ;
  DIGIT_R:      in std_logic_vector(3 downto 0) ;
  RCLK, SCLK, SER:   out  std_logic 
) ;
end demo_dy1_new ;


architecture Behavioral of demo_dy1_new is

signal dy_update:                       std_logic ;
signal dy_frameend:                     std_logic ;
signal dy_frameend_q:                   std_logic ;
signal dy_frameend_c:                   std_logic ;
signal dy_frame:                        std_logic ;
signal dp0,dp1,dp2,dp3,dp4,dp5:         std_logic := '0';

signal dy_data:                         y2d_type ;
signal clkdiv:			                    std_logic_vector (31 downto 0) ;
signal dy_pwm:                          std_logic_vector(3 downto 0) ;

signal countbit, countbit_del: std_logic;

component Oho_Dy1
port (
  dy_clock:         in std_logic ;
  dy_rst_n:         in std_logic ;
  dy_data:          in y2d_type ;
  dy_update:        in std_logic ;
  dy_frame:         out std_logic ;
  dy_frameend:      out std_logic ;
  dy_frameend_c:    out std_logic ;
  dy_pwm:           in std_logic_vector(3 downto 0) ;
  dy_counter:       out std_logic_vector(31 downto 0) ;
  dy_sclk:          out std_logic ;
  dy_ser:           out std_logic ;
  dy_rclk:          out std_logic
) ;
end component ;


begin

-- instantiate OHO_DY1 core
DY1: Oho_Dy1 port map (
  dy_clock => SYSCLK50,
  dy_rst_n => '1',
  dy_data => dy_data,
  dy_update  => dy_update,
  dy_frame  => dy_frame,
  dy_frameend  => dy_frameend,
  dy_frameend_c  => dy_frameend_c,
  dy_pwm => dy_pwm,
  dy_counter => clkdiv,
  dy_sclk => SCLK,
  dy_ser => SER,
  dy_rclk => RCLK
) ;

-- brightness adjust
dy_pwm <= "1111";
-- continuous display update
dy_update <= '1' ;


-- define string with 16 digits respectively
dy_data(0) <= hex & dp0 & "000" & DIGIT_R; -- rechte Ziffer
dy_data(1) <= hex & dp1 & "000" & DIGIT_M; -- Mitte
dy_data(2) <= hex & dp2 & "000" & DIGIT_L; -- linke Ziffer
dp0 <=  clkdiv(23);

---- erstes Display
--dy_data(0) <= hex & dp0 & "000" & DEBUG_ADDR(7 downto 4); -- rechte Ziffer
--dy_data(1) <= hex & dp1 & "000" & DEBUG_ADDR(11 downto 8); -- Mitte
--dy_data(2) <= hex & dp2 & "000" & DEBUG_ADDR(15 downto 12); -- linke Ziffer
--dp0 <=  clkdiv(22) when (DEBUG_LOADING = '1') else clkdiv(24);
--
---- zweites Display
--dy_data(3) <= hex & dp3 & "000" & DEBUG_DATA(3 downto 0); -- rechte Ziffer
--dy_data(4) <= hex & dp4 & "000" & DEBUG_DATA(7 downto 4); -- Mitte
--dy_data(5) <= (others => '0'); -- linke Ziffer
--dp3 <=  clkdiv(23);


-- blink init led
BLINK_COUNT <= clkdiv(27 downto 20);
LED_BLINK <= clkdiv(23); -- 2,93 Hz
countbit <=  clkdiv(21); -- 11,71875 Hz

tickproc: process(SYSCLK50)
begin
  if rising_edge(SYSCLK50) then
    TICK_12HZ <= '0';
    if (countbit = '1') and (countbit_del = '0') then
      TICK_12HZ <= '1'; -- 11,71875 Hz
	 end if;
    countbit_del <= countbit;
  end if;
end process tickproc;


end Behavioral;

