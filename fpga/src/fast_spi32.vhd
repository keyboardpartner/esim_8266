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
-- Module Name: fast_spi32 - Behavioral
--
-- Description:
-- Classic SPI Interface for MODE 3 (used in HX3 FPGA)
-- Erstes empfangenes Byte ist immer das CMD-Byte, danach folgen 1 bis 4 Datenbytes (MSB zuerst)
-- nach 8 Bits geht CMD_TICK auf 1 und das Command-Byte steht in BYTE_RX bereit.

-- Nach jeweils 8 Bits geht BYTE_TICK auf 1 und das soeben empfangene Datenbyte steht in BYTE_RX bereit. 

-- nach dem letzten Bit (SS wechselt von 0 auf 1) geht END_TICK auf 1 
-- und die empfangenen Daten (bis 32 Bit) stehen in SPI_RX bereit
-- Wurden weniger als 32 Bit empfangen, stehen die Daten in den oberen Bits (wg. MSB first)!
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
    START_TICK:  out STD_LOGIC; -- Tick, wenn SS von 1 auf 0 wechselt (Start des Transfers)
    SPI_RX:  OUT STD_LOGIC_VECTOR(31 DOWNTO 0); -- empfangene Daten bis 32 Bit, mit END_TICK gültig
    SPI_TX:  in STD_LOGIC_VECTOR(31 DOWNTO 0);  -- zu sendende Daten, MSB zuerst, werden auf MISO ausgegeben
    CMD_TICK:  out STD_LOGIC;   -- Tick, wenn erste 8 Bits (CMD-Byte) empfangen wurden
    BYTE_TICK:  out STD_LOGIC;  -- Tick, wenn je 8 Datenbits empfangen wurden
    BYTE_RX:  OUT STD_LOGIC_VECTOR(7 DOWNTO 0); -- zuletzt empfangenes Byte, nach je 8 SCKs mit BYTE_TICK gültig
    END_TICK:  out STD_LOGIC    -- Tick, wenn SS von 0 auf 1 wechselt (alle Daten empfangen)
  );
end fast_spi32;

architecture Behavioral of fast_spi32 is

signal miso_int: std_logic;
signal ss_del, ss_sync: std_logic:= '1';
signal cmd_8, cmd_8_sync, cmd_8_del: std_logic:= '0';
signal data_8, data_8_sync, data_8_del: std_logic:= '0';
signal cmd_8_changed, data_8_changed: std_logic;
signal shift_in, shift_out: std_logic_vector(31 downto 0):= (others => '0');
signal cmd_bit_counter: std_logic_vector(3 downto 0):= (others => '0');
signal data_bit_counter: std_logic_vector(4 downto 0):= (others => '0');

begin

-- wir verwenden hier einen Multiplexer statt eines SR
miso_int <= SPI_TX(31 - to_integer(unsigned(data_bit_counter)));

MISO <= miso_int when SS = '0' else 'Z';

proc_shift_in: process(SCK, SS)
begin
  if SS = '1' then
    cmd_bit_counter <= (others => '0'); -- Zähler zurücksetzen, wenn SS inaktiv ist
    data_bit_counter <= (others => '0'); 
  else -- SS = '0', aktiver Transfer
    if rising_edge(SCK) then
      -- sample MOSI on rising SCK edge (MODE0 or MODE3)
      shift_in <= (shift_in(30 downto 0) & MOSI);
    end if;
    if falling_edge(SCK) then
      if cmd_bit_counter(3) = '1' then
        -- 8. Bit empfangen, jetzt Datenbits empfangen
        data_bit_counter <= data_bit_counter + 1;
      else
        cmd_bit_counter <= cmd_bit_counter + 1;
      end if;
    end if;
  end if;
end process;

cmd_8 <= cmd_bit_counter(3);
data_8 <= data_bit_counter(3);

cmd_8_changed <= cmd_8_sync xor cmd_8_del;
data_8_changed <= data_8_sync xor data_8_del;

proc_shift_create_ticks: process(SYSCLK)
begin
  if rising_edge(SYSCLK) then
    cmd_8_sync <= cmd_8;
    cmd_8_del <= cmd_8_sync;

    CMD_TICK <= cmd_8_changed;
    if cmd_8_changed = '1' then
      BYTE_RX <= shift_in(7 downto 0); -- CMD früh bereitstellen, vor den Datenbytes
    end if;
    
    data_8_sync <= data_8;
    data_8_del <= data_8_sync;

    BYTE_TICK <= data_8_changed; -- Tick, wenn jeweils 8 Datenbits empfangen wurden
    if data_8_changed = '1' then
      BYTE_RX <= shift_in(7 downto 0);
    end if;

    ss_sync <= SS;
    ss_del <= ss_sync;

    START_TICK <= '0';
    if (ss_del = '1') and (ss_sync = '0') then
      START_TICK <= '1'; -- Start-Tick generieren, wenn SS von 1 auf 0 wechselt
    end if;

    END_TICK <= '0';
    if (ss_del = '0') and (ss_sync = '1') then
      END_TICK <= '1';
      SPI_RX <= shift_in; -- Empfangene Daten bereitstellen
    end if;
  end if;
end process;

end Behavioral;
