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
-- Create Date: 22.07.2026
-- Module Name: pb_inport - Behavioral
--
-- Description:
-- Picoblaze input ports
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.NUMERIC_STD.ALL;

entity pb_inport is
  Port ( 
    SYSCLK:   in STD_LOGIC;
    PORT_RD:  in STD_LOGIC;
    PORT_ID:  in STD_LOGIC_VECTOR(7 DOWNTO 0);
    PORT_DATA:  OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    PORT_0:  in STD_LOGIC_VECTOR(7 DOWNTO 0);
    PORT_1:  in STD_LOGIC_VECTOR(7 DOWNTO 0)
  );
end pb_inport;

architecture Behavioral of pb_inport is

begin

proc_port: process(SYSCLK)
begin
	if rising_edge(SYSCLK) then
		case PORT_ID is
			when x"00" => -- get port 0
				PORT_DATA <= PORT_0;
			when x"01" => -- get port 1
				PORT_DATA <= PORT_1;
		  when others =>
			 -- do nothing
		end case;
	end if;
end process;

end Behavioral;
