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

entity bram_wrapper_pb6 is
  Port ( 
    SYSCLK:   in STD_LOGIC;
    END_TICK: in STD_LOGIC; 
    VERSION:  in STD_LOGIC_VECTOR(31 DOWNTO 0);
    SPI_RX:   in STD_LOGIC_VECTOR(31 DOWNTO 0);
    SPI_TX:   out STD_LOGIC_VECTOR(31 DOWNTO 0);
    CFG_PORT: out STD_LOGIC_VECTOR(3 DOWNTO 0);
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
signal cmd_spi: std_logic_vector(7 downto 0):= (others => '0');
signal data_spi_wr, data_spi_rd: std_logic_vector(7 downto 0);
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

cmd_spi <= SPI_RX(31 downto 24);

cmdproc: process(SYSCLK)
begin
  if rising_edge(SYSCLK) then
    -- sample command and data on END_TICK signal
    -- "00100000 aaaaaaaa aaaaaaaa aaaaaaaa" = set address command
    -- "00110000 aaaaaaaa aaaaaaaa aaaaaaaa" = set address command with autoinc after next read or write
    -- "10000000 xxxxxxxx xxxxxxxx dddddddd" = write data command
    -- "01000000 xxxxxxxx xxxxxxxx xxxxxxxx" = read data command
    -- "11000000 xxxxxxxx xxxxxxxx xxxxxxxx" = read ID/version command
    -- "10100000 xxxxxxxx xxxxxxxx xxxxxxxx" = Set Type Select (CFG output nibble)
    -- "00000000 xxxxxxxx xxxxxxxx xxxxxxxx" = SPI read cycle, return data from last read command

    end_tick_d1 <= END_TICK;
    end_tick_d2 <= end_tick_d1;
    write_tick <= '0';

    if END_TICK = '1' then      
      case cmd_spi is
        when x"00" => -- SPI read cycle, return data from last read command
          increment <= autoinc_enable;
        when x"20" => -- set address command
          addr_counter <= SPI_RX(23 downto 0);
          autoinc_enable <= '0';
          increment <= '0';
        when x"30" => -- set address command with autoinc after read or write
          addr_counter <= SPI_RX(23 downto 0);
          autoinc_enable <= '1';
          increment <= '0';
        when x"40" => -- read data command
          SPI_TX <= x"000000" & data_spi_rd;
          increment <= '0'; -- do not increment address counter on read command, only on SPI read cycle
			 
        when x"60" => -- write port 0 command
          OUTPORT_0 <= SPI_RX(7 downto 0);
        when x"61" => -- write port 1 command
          OUTPORT_1 <= SPI_RX(7 downto 0);
        when x"62" => -- write port 2 command
          OUTPORT_2 <= SPI_RX(7 downto 0);
        when x"63" => -- write port 3 command
          OUTPORT_3 <= SPI_RX(7 downto 0);
			 
        when x"80" => -- write data command
          data_spi_wr <= SPI_RX(7 downto 0);
          write_tick <= '1';
          increment <= autoinc_enable;
       when x"A0" => -- Set Type Select
          CFG_PORT <= SPI_RX(3 downto 0);
          autoinc_enable <= '0';
          increment <= '0'; -- do not increment address counter on read command, only on SPI read cycle
        when x"C0" => -- read ID/version command
          SPI_TX <= VERSION;
          autoinc_enable <= '0';
          increment <= '0'; -- do not increment address counter on read command, only on SPI read cycle
        when x"E0" => -- Clear Reset Command
			 RESET <= '0';
        when x"F0" => -- Set Reset Command
			 RESET <= '1';
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
