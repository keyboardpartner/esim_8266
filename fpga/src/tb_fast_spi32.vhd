--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   04:07:30 08/22/2026
-- Design Name:   
-- Module Name:   E:/MojoFPGA_Picoblaze/src/tb_fast_spi32.vhd
-- Project Name:  MojoFPGA_Picoblaze
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: fast_spi32
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY tb_fast_spi32 IS
END tb_fast_spi32;
 
ARCHITECTURE behavior OF tb_fast_spi32 IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
COMPONENT fast_spi32
  PORT(
    SYSCLK : IN  std_logic;
    SCK : IN  std_logic;
    SS : IN  std_logic;
    MOSI : IN  std_logic;
    MISO : INOUT  std_logic;
    SPI_RX : OUT  std_logic_vector(31 downto 0);
    SPI_TX : IN  std_logic_vector(31 downto 0);
    START_TICK:  out STD_LOGIC;
    BYTE_RX:  OUT STD_LOGIC_VECTOR(7 DOWNTO 0);
    BYTE_TICK:  out STD_LOGIC;
    CMD_TICK:  out STD_LOGIC;
    END_TICK : OUT  std_logic
  );
  END COMPONENT;
    

   --Inputs
   signal SYSCLK : std_logic := '0';
   signal SCK : std_logic := '0';
   signal SS : std_logic := '0';
   signal MOSI : std_logic := '0';
   signal SPI_TX : std_logic_vector(31 downto 0) := (others => '0');

	--BiDirs
   signal MISO : std_logic;

 	--Outputs
   signal SPI_RX : std_logic_vector(31 downto 0);
   signal START_TICK : std_logic;
   signal BYTE_TICK : std_logic;
   signal CMD_TICK : std_logic;
   signal BYTE_RX : std_logic_vector(7 downto 0);
   signal END_TICK : std_logic;

  -- Clock period definitions
  constant SYSCLK_period : time := 10 ns;
  constant SCK_period : time := 30 ns;
 
  procedure send_byte (
    -- Sende Byte in SPI Mode 3, MSB zuerst
    constant BYTE: in std_logic_vector(7 downto 0);
    constant SCK_period: in time;
    signal SCK: inout std_logic;
    signal MOSI: inout std_logic ) is
  begin
    for i in 0 to 7 loop
      SCK <= '0';
      wait for 5ns;
      MOSI <= BYTE(7 - i); -- MSB zuerst
      wait for SCK_period/2;
      SCK <= '1';
      wait for 5ns;
      wait for SCK_period/2;
    end loop;
    -- MOSI <= '0';
    SCK <= '0';
  end procedure send_byte;

BEGIN
 
-- Instantiate the Unit Under Test (UUT)
uut: fast_spi32 PORT MAP (
      SYSCLK => SYSCLK,
      SCK => SCK,
      SS => SS,
      MOSI => MOSI,
      MISO => MISO,
      SPI_RX => SPI_RX,
      SPI_TX => SPI_TX,
      START_TICK => START_TICK,
      CMD_TICK => CMD_TICK,
      BYTE_TICK => BYTE_TICK,
      BYTE_RX => BYTE_RX,
      END_TICK => END_TICK
    );

-- Clock process definitions
SYSCLK_process :process
begin
  SYSCLK <= '0';
  wait for SYSCLK_period/2;
  SYSCLK <= '1';
  wait for SYSCLK_period/2;
end process;


-- Stimulus process
stim_proc: process
begin		
  SS <= '1';
  SPI_TX <= x"00000000";
  wait for 100 ns;	
  SS <= '0';
  wait for 30 ns;	
  send_byte(x"80", SCK_period, SCK, MOSI); -- Command Byte
  wait until CMD_TICK = '1';	
  wait until CMD_TICK = '0';	
  SPI_TX <= x"A1234567";
  wait for 10 ns;	
  send_byte(x"12", SCK_period, SCK, MOSI); -- Adresse MSB
  wait for 30 ns;	
  send_byte(x"34", SCK_period, SCK, MOSI); 
  wait for 45 ns;	
  send_byte(x"56", SCK_period, SCK, MOSI);
  wait for 50 ns;	
  send_byte(x"78", SCK_period, SCK, MOSI); -- Adresse LSB
  wait for 60 ns;	
  SS <= '1';
  wait for 300 ns;  
  report "simulation finished successfully" severity FAILURE;

  wait;
end process;

END;
