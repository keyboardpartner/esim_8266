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
-- Create Date: 22.03.2026
-- Module Name: qspi_connect - Behavioral
--
-- Description:
-- Classic SPI Interface for MODE 3 (used in HX3 FPGA)
-- Einfache Version, die nur 32-Bit-Transfers unterstützt 

-- 19.08.2026: MISO tri-stated
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.NUMERIC_STD.ALL;

entity fast_spi32 is
  Port ( 
    SYSCLK:   in STD_LOGIC;
    SCK:      in STD_LOGIC;     -- Verbindung zum Controller ST32
    SS:       in STD_LOGIC;     -- Slave Select
    MOSI:     in STD_LOGIC;
    MISO:     inout STD_LOGIC;   
    DATA_RX:  OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
    DATA_TX:  in STD_LOGIC_VECTOR(31 DOWNTO 0);
    -- START_TICK:  out STD_LOGIC;
    END_TICK:  out STD_LOGIC
  );
end fast_spi32;

architecture Behavioral of fast_spi32 is

signal miso_int: std_logic;
signal ss_del, ss_sync: std_logic:= '1';
signal shift_in: std_logic_vector(31 downto 0):= (others => '0');
signal shift_out: std_logic_vector(31 downto 0):= (others => '0');

begin

MISO <= miso_int when SS = '0' else 'Z';

-- DATA_TX muss vor der SS-Flanke aktualisiert sein
proc_shift_out: process(SCK, SS, DATA_TX)
begin
  if SS = '1' then
    shift_out <= DATA_TX; -- Daten für nächsten Transfer latchen, wenn SS inaktiv ist
    miso_int <= DATA_TX(31);
  else -- SS = '0', aktiver Transfer
    if rising_edge(SCK) then
      -- change MISO output on rising SCK edge (MODE0 or MODE3)
      miso_int <= shift_out(31); -- MSB zuerst senden
      shift_out <= (shift_out(30 downto 0) & '0'); -- Shift nach links
    end if;
  end if;
end process;


proc_shift_in: process(SCK, SS, MOSI)
begin
  if SS = '0' then
    if rising_edge(SCK) then
      -- sample MOSI on rising SCK edge (MODE0 or MODE3)
      shift_in <= (shift_in(30 downto 0) & MOSI);
    end if;
  end if;
end process;

proc_shift_create_ticks: process(SYSCLK)
begin
  if rising_edge(SYSCLK) then
    ss_sync <= SS;
    ss_del <= ss_sync;
    -- START_TICK <= '0';
    -- if (ss_del = '1') and (ss_sync = '0') then
    --   START_TICK <= '1'; -- Start-Tick generieren, wenn SS von 1 auf 0 wechselt
    -- end if;

    END_TICK <= '0';
    if (ss_del = '0') and (ss_sync = '1') then
      END_TICK <= '1';
      DATA_RX <= shift_in; -- Empfangene Daten bereitstellen
    end if;
  end if;
end process;

end Behavioral;
