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

entity bram_wrapper is
  Port ( 
    SYSCLK:   in STD_LOGIC;
    END_TICK: in STD_LOGIC; 
    VERSION:  in STD_LOGIC_VECTOR(31 DOWNTO 0);
    SPI_RX:  in STD_LOGIC_VECTOR(31 DOWNTO 0);
    SPI_TX:  out STD_LOGIC_VECTOR(31 DOWNTO 0);
    RAM_ADDR: in STD_LOGIC_VECTOR(15 DOWNTO 0);
    RAM_DATA: inout STD_LOGIC_VECTOR(7 DOWNTO 0);
    RAM_CE: in STD_LOGIC;
    RAM_OE: in STD_LOGIC;
    RAM_WR: in STD_LOGIC
  );
end bram_wrapper;

architecture Behavioral of bram_wrapper is

--COMPONENT sram_32k
--  PORT (
--    clka : IN STD_LOGIC;
--    wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
--    addra : IN STD_LOGIC_VECTOR(14 DOWNTO 0);
--    dina : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
--    douta : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
--    clkb : IN STD_LOGIC;
--    web : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
--    addrb : IN STD_LOGIC_VECTOR(14 DOWNTO 0);
--    dinb : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
--    doutb : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
--  );
--END COMPONENT;

COMPONENT sram_64k
  PORT (
    clka : IN STD_LOGIC;
    wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    addra : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    dina : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
    douta : OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    clkb : IN STD_LOGIC;
    web : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
    addrb : IN STD_LOGIC_VECTOR(15 DOWNTO 0);
    dinb : IN STD_LOGIC_VECTOR(7 DOWNTO 0);
    doutb : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)
  );
END COMPONENT;

signal addr_host: std_logic_vector(15 downto 0);
signal cmd_spi: std_logic_vector(3 downto 0):= (others => '0');
signal data_spi_wr, data_spi_rd: std_logic_vector(7 downto 0);
signal data_host_wr, data_host_rd: std_logic_vector(7 downto 0);
signal wea_spi: std_logic_vector(0 downto 0) := "0";

signal ram_wr_host_tick: std_logic_vector(0 downto 0) := "0";
signal debug_timeout: std_logic_vector(3 downto 0);

signal write_tick, increment: std_logic := '0';
signal autoinc_enable: std_logic := '0';
signal end_tick_d1, end_tick_d2: std_logic;

signal addr_counter: std_logic_vector(23 downto 0);

begin

-- Dual port BRAM for EPROM/SRAM simulation
--dp_ram32k : sram_32k
--  PORT MAP (
--    clka => SYSCLK,
--    wea => wea_spi,
--    addra => addr_counter(14 downto 0),
--    dina => data_spi_wr,
--    douta => data_spi_rd,
--    clkb => SYSCLK,
--    web => ram_wr_host_tick, -- std_logic_vector(0 downto 0)!
--    addrb => addr_host,
--    dinb => data_host_wr,
--    doutb => data_host_rd
--  );

dp_ram64k : sram_64k
  PORT MAP (
    clka => SYSCLK,
    wea => wea_spi,
    addra => addr_counter(15 downto 0),
    dina => data_spi_wr,
    douta => data_spi_rd,
    clkb => SYSCLK,
    web => ram_wr_host_tick, -- std_logic_vector(0 downto 0)!
    addrb => addr_host,
    dinb => data_host_wr,
    doutb => data_host_rd
  );


RAM_DATA <= data_host_rd when (RAM_WR = '1') and (RAM_OE = '0') and (RAM_CE = '0')  else (others => 'Z');

cmd_spi <= SPI_RX(31 downto 28);

cmdproc: process(SYSCLK)
begin
  if rising_edge(SYSCLK) then
    -- sample command and data on END_TICK signal
    -- "0010xxxx aaaaaaaa aaaaaaaa aaaaaaaa" = set address command
    -- "0011xxxx aaaaaaaa aaaaaaaa aaaaaaaa" = set address command with autoinc after next read or write
    -- "1000xxxx xxxxxxxx xxxxxxxx dddddddd" = write data command
    -- "0100xxxx xxxxxxxx xxxxxxxx xxxxxxxx" = read data command
    -- "1100xxxx xxxxxxxx xxxxxxxx xxxxxxxx" = read ID/version command
    -- "0000xxxx xxxxxxxx xxxxxxxx xxxxxxxx" = SPI read cycle, return data from last read command

    end_tick_d1 <= END_TICK;
    end_tick_d2 <= end_tick_d1;
    write_tick <= '0';

    if END_TICK = '1' then      
      case cmd_spi is
        when "0010" => -- set address command
          addr_counter <= SPI_RX(23 downto 0);
          autoinc_enable <= '0';
          increment <= '0';
        when "0011" => -- set address command with autoinc after read or write
          addr_counter <= SPI_RX(23 downto 0);
          autoinc_enable <= '1';
          increment <= '0';
        when "1000" => -- write data command
          data_spi_wr <= SPI_RX(7 downto 0);
          write_tick <= '1';
          increment <= autoinc_enable;
        when "0100" => -- read data command
          SPI_TX <= x"000000" & data_spi_rd;
          increment <= '0'; -- do not increment address counter on read command, only on SPI read cycle
        when "1100" => -- read ID/version command
          SPI_TX <= VERSION;
          autoinc_enable <= '0';
          increment <= '0'; -- do not increment address counter on read command, only on SPI read cycle
        when "0000" => -- SPI read cycle, return data from last read command
          increment <= autoinc_enable;
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

syncproc: process(SYSCLK)
begin
  if rising_edge(SYSCLK) then
    -- sync external SRAM signals to SYSCLK domain
    if RAM_CE = '0' then
      -- latch address on RAM_CE signal
      addr_host <= RAM_ADDR;
    end if;
    if RAM_WR = '0' then
      -- latch data to be written on ram_wr_host_tick
      data_host_wr <= RAM_DATA;
    end if;
    -- write on inverted RAM_WR signal
    ram_wr_host_tick(0) <= not (RAM_WR or RAM_CE);
  end if;
end process syncproc;

end Behavioral;
