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
-- Create Date: 30.07.2026
-- Module Name: bram_wrapper - Behavioral
--
-- Description:
-- Emulates a SRAM or EPROM
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.NUMERIC_STD.ALL;

entity type_select is
  Port ( 
    TYPE_SEL:  in STD_LOGIC_VECTOR(3 DOWNTO 0);
    ADDR_10: in STD_LOGIC_VECTOR(10 DOWNTO 0);
    PIN_1:   in STD_LOGIC;
    PIN_2:   in STD_LOGIC;
	 PIN_20:   in STD_LOGIC; -- meist /CE bis auf 2532
    PIN_22:   in STD_LOGIC;
    PIN_23:  in STD_LOGIC;
    PIN_26:  in STD_LOGIC;
    PIN_27:  in STD_LOGIC;
    ADDR_OUT: out STD_LOGIC_VECTOR(15 DOWNTO 0);
    RAM_CE: out STD_LOGIC;
    RAM_OE: out STD_LOGIC;
    RAM_WR: out STD_LOGIC
  );
end type_select;

architecture Behavioral of type_select is

begin

-- TYPE_SEL:
-- 0: 2716
-- 1: 2732
-- 2: 2764
-- 3: 27128
-- 4: 27256
-- 5: 27512

-- 6: 2532
-- 7: 2564/2364

-- 8: 6116  SRAM
-- 9: 6264  SRAM
-- A: 62256 SRAM

ADDR_OUT(10 downto 0) <= ADDR_10;

selproc: process(TYPE_SEL, ADDR_10, PIN_1, PIN_2, PIN_20, PIN_22, PIN_23, PIN_26, PIN_27)
begin
	case TYPE_SEL is
		when x"0" => -- 2716
			ADDR_OUT(15 downto 11) <= (others => '0');
			RAM_CE <= PIN_20;
			RAM_OE <= PIN_22;
			RAM_WR <= '1';
		when x"1" => -- 2732
			ADDR_OUT(11) <= PIN_23;
			ADDR_OUT(15 downto 12) <= (others => '0');
			RAM_CE <= PIN_20;
			RAM_OE <= PIN_22;
			RAM_WR <= '1';
		when x"2" => -- 2764
			ADDR_OUT(11) <= PIN_23;
			ADDR_OUT(12) <= PIN_2;
			ADDR_OUT(15 downto 13) <= (others => '0');
			RAM_CE <= PIN_20;
			RAM_OE <= PIN_22;
			RAM_WR <= '1';
		when x"3" => -- 27128
			ADDR_OUT(11) <= PIN_23;
			ADDR_OUT(12) <= PIN_2;
			ADDR_OUT(13) <= PIN_26;
			ADDR_OUT(15 downto 14) <= (others => '0');
			RAM_CE <= PIN_20;
			RAM_OE <= PIN_22;
			RAM_WR <= '1';
		when x"4" => -- 27256
			ADDR_OUT(11) <= PIN_23;
			ADDR_OUT(12) <= PIN_2;
			ADDR_OUT(13) <= PIN_26;
			ADDR_OUT(14) <= PIN_27;
			ADDR_OUT(15) <= '0';
			RAM_CE <= PIN_20;
			RAM_OE <= PIN_22;
			RAM_WR <= '1';
		when x"5" => -- 27512
			ADDR_OUT(11) <= PIN_23;
			ADDR_OUT(12) <= PIN_2;
			ADDR_OUT(13) <= PIN_26;
			ADDR_OUT(14) <= PIN_27;
			ADDR_OUT(15) <= PIN_1;
			RAM_CE <= PIN_20;
			RAM_OE <= PIN_22;
			RAM_WR <= '1';
			
		when x"6" => -- 2532
			ADDR_OUT(11) <= PIN_20;
			ADDR_OUT(15 downto 12) <= (others => '0');
			RAM_CE <= PIN_22;
			RAM_OE <= PIN_22;
			RAM_WR <= '1';
		when x"7" => -- 2564
			ADDR_OUT(11) <= PIN_20;
			ADDR_OUT(12) <= PIN_23;
			ADDR_OUT(15 downto 13) <= (others => '0');
			RAM_CE <= PIN_22;
			RAM_OE <= PIN_22;
			RAM_WR <= '1';
			
		when x"8" => -- 6116
			ADDR_OUT(15 downto 11) <= (others => '0');
			RAM_CE <= PIN_20;
			RAM_OE <= PIN_22;
			RAM_WR <= PIN_23;
		when x"9" => -- 6264
			ADDR_OUT(11) <= PIN_23;
			ADDR_OUT(12) <= PIN_2;
			ADDR_OUT(15 downto 13) <= (others => '0');
			RAM_CE <= PIN_20;
			RAM_OE <= PIN_22;
			RAM_WR <= PIN_27;
		when x"A" => -- 62256
			ADDR_OUT(11) <= PIN_23;
			ADDR_OUT(12) <= PIN_2;
			ADDR_OUT(13) <= PIN_26;
			ADDR_OUT(14) <= PIN_1;
			ADDR_OUT(15) <= '0';
			RAM_CE <= PIN_20;
			RAM_OE <= PIN_22;
			RAM_WR <= PIN_27;
		when others =>
		-- do nothing
	end case;

end process selproc;

end Behavioral;
