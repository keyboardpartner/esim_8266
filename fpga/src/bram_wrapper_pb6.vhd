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
-- Loadable Picoblaze instruction ROM
----------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
use IEEE.NUMERIC_STD.ALL;

entity bram_wrapper_pb6 is
  Port ( 
    SYSCLK:   in STD_LOGIC;
    CMD_TICK: in STD_LOGIC;
    CMD_BYTE:  in STD_LOGIC_VECTOR(7 DOWNTO 0);
    END_TICK: in STD_LOGIC;
    SPI_RX:   in STD_LOGIC_VECTOR(31 DOWNTO 0);
    SPI_TX:   out STD_LOGIC_VECTOR(31 DOWNTO 0);
    CFG_PORT: out STD_LOGIC_VECTOR(3 DOWNTO 0);
    VERSION:  in STD_LOGIC_VECTOR(31 DOWNTO 0);
    OUTPORT_0: out STD_LOGIC_VECTOR(7 DOWNTO 0);
    OUTPORT_1: out STD_LOGIC_VECTOR(7 DOWNTO 0);
    OUTPORT_2: out STD_LOGIC_VECTOR(7 DOWNTO 0);
    OUTPORT_3: out STD_LOGIC_VECTOR(7 DOWNTO 0);
    RESET:    out STD_LOGIC;
    PB_ADDR:  in STD_LOGIC_VECTOR(11 DOWNTO 0);
    PB_DATA:  inout STD_LOGIC_VECTOR(17 DOWNTO 0)
  );
end bram_wrapper_pb6;

architecture Behavioral of bram_wrapper_pb6 is

-- Picoblaze-ROM, 1 KByte x 32
COMPONENT pb_ram
  PORT (
    clka : IN STD_LOGIC;
    wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    addra : IN STD_LOGIC_VECTOR(13 DOWNTO 0);
    dina : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
    douta : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    clkb : IN STD_LOGIC;
    web : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    addrb : IN STD_LOGIC_VECTOR(11 DOWNTO 0);
    dinb : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
    doutb : OUT STD_LOGIC_VECTOR(31 DOWNTO 0)
  );
END COMPONENT;

signal addr_host: std_logic_vector(15 downto 0);
signal data_spi_wr, data_spi_rd, command: std_logic_vector(7 downto 0);
signal data_host_rd: std_logic_vector(31 downto 0);
signal wea_spi: std_logic_vector(0 downto 0) := "0";

signal write_tick, increment: std_logic := '0';
signal autoinc_enable: std_logic := '0';
signal end_tick_d1, end_tick_d2: std_logic;

signal addr_counter: std_logic_vector(23 downto 0);

begin

-- Dual port BRAM for Picoblaze
-- We take 4 Bytes for PB interface instead of 18 bits to simplify connection to ESP

pb_instructions: pb_ram
  PORT MAP (
    clka => SYSCLK,
    wea => wea_spi,
    addra => addr_counter(13 downto 0),
    dina => data_spi_wr,
    douta => data_spi_rd,
    clkb => SYSCLK,
    web => "0",
    addrb => PB_ADDR,
    dinb => (others => '0'),
    doutb => data_host_rd
  );

PB_DATA <= data_host_rd(17 downto 0);

cmdproc: process(SYSCLK)
begin
  if rising_edge(SYSCLK) then
    -- x"00 AA AA AA" = set address command
    -- x"02 xx xx DD" = write RAM data command
    -- x"03 xx xx DD" = write RAM data command with autoinc after write
    -- x"04 xx xx xx" = read RAM data command
    -- x"05 xx xx xx" = read RAM data command with autoinc after read
    -- x"08 xx xx xx" = read ID/version command
    -- x"0A xx xx xx" = Set Type Select (CFG output nibble)
    -- x"0F xx xx xB" = Set Reset line to Bit 0, B = 0 oder 1
    -- x"4P xx xx DD" = Write Port P = 0..3
    -- x"4P xx xx DD" = Write Port P = 0..3
 
    end_tick_d1 <= END_TICK;
    end_tick_d2 <= end_tick_d1;
    write_tick <= '0';

    if CMD_TICK = '1' then
      -- CMD ist vollständig, Reads können jetzt vorbereitet werden, 
      -- Writes werden erst bei END_TICK verarbeitet
      command <= CMD_BYTE; -- latch command byte
      case CMD_BYTE is
        when x"04" => -- read 8 Bit data command
          SPI_TX(31 downto 24) <= data_spi_rd; -- first 8 bits to be read in MSB!
        when x"05" => -- read 8 Bit data command with autoinc after read
          SPI_TX(31 downto 24) <= data_spi_rd; -- first 8 bits to be read in MSB!
        when x"08" => -- read ID/version command
          SPI_TX <= VERSION;
        when others =>
          -- do nothing
      end case;
    end if;

    if END_TICK = '1' then     
      -- Daten sind vollständig empfangen, CMD für Writes kann jetzt verarbeitet werden 
      increment <= '0';
      case command is
        when x"00" => -- set address command
          addr_counter <= SPI_RX(23 downto 0);

        when x"02" => -- write data command
          data_spi_wr <= SPI_RX(7 downto 0);
          write_tick <= '1';
        when x"03" => -- write RAM data command with autoinc after write
          data_spi_wr <= SPI_RX(7 downto 0);
          write_tick <= '1';
          increment <= '1';

        when x"05" => -- read RAM data command with autoinc after read
          increment <= '1';
			 
        when x"40" => -- write port 0 command
          OUTPORT_0 <= SPI_RX(7 downto 0);
        when x"41" => -- write port 1 command
          OUTPORT_1 <= SPI_RX(7 downto 0);
        when x"42" => -- write port 2 command
          OUTPORT_2 <= SPI_RX(7 downto 0);
        when x"43" => -- write port 3 command
          OUTPORT_3 <= SPI_RX(7 downto 0);
			 
       when x"0A" => -- Set Type Select
          CFG_PORT <= SPI_RX(3 downto 0);
        when x"0F" => -- Set/Clear Reset Command
			    RESET <= SPI_RX(0);
        when others =>
          -- do nothing
      end case;
    end if;

    wea_spi(0) <= write_tick;

    -- delayed increment address counter if autoinc bit is set and increment signal is set
    if (end_tick_d2 = '1') and (increment = '1') then
      addr_counter <= addr_counter + 1;
      increment <= '0';
    end if;

  end if;
end process cmdproc;

end Behavioral;
