-- #############################################################################
--
--       __ ________  _____  ____  ___   ___  ___
--      / //_/ __/\ \/ / _ )/ __ \/ _ | / _ \/ _ \
--     / ,< / _/   \  / _  / /_/ / __ |/ , _/ -- /
--    /_/|_/___/_  /_/____/\____/_/_|_/_/|_/____/
--      / _ \/ _ | / _ \/_  __/ |/ / __/ _ \
--     / ___/ __ |/ , _/ / / /    / _-- , _/
--    /_/  /_/ |_/_/|_| /_/ /_/|_/___/_/|_|
--
-- #############################################################################
--
-- Company:  KeyboardPartner UG
-- Engineer: Carsten Meyer
-- 
-- Create Date: 22.06.2026
-- Module Name: pb_outport - Behavioral
--
-- Description:
-- Picoblaze output ports
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.NUMERIC_STD.ALL;

entity pb_outport is
  Port ( 
    SYSCLK:   in STD_LOGIC;
    PORT_WR:  in STD_LOGIC;
    PORT_ID:  in STD_LOGIC_VECTOR(7 DOWNTO 0);
    PORT_DATA:  in STD_LOGIC_VECTOR(7 DOWNTO 0);
    PORT_0:  out STD_LOGIC_VECTOR(7 DOWNTO 0);
    PORT_1:  out STD_LOGIC_VECTOR(7 DOWNTO 0)
  );
end pb_outport;

architecture Behavioral of pb_outport is

begin

proc_shift_create_ticks: process(SYSCLK)
begin
	if rising_edge(SYSCLK) then
		if PORT_WR = '1' then
			case PORT_ID is
				when x"00" => -- set port 0
					PORT_0 <= PORT_DATA;
				when x"01" => -- set port 1
					PORT_1 <= PORT_DATA;
			  when others =>
				 -- do nothing
			end case;
		end if;
	end if;
end process;

end Behavioral;
