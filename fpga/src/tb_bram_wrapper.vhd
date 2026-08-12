--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   03:09:30 08/01/2026
-- Design Name:   
-- Module Name:   E:/fpga_dualport_sram/dualport_sram/src/tb_bram_wrapper.vhd
-- Project Name:  dualport_sram
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: bram_wrapper
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
 
ENTITY tb_bram_wrapper IS
END tb_bram_wrapper;
 
ARCHITECTURE behavior OF tb_bram_wrapper IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT bram_wrapper
    PORT(
         SYSCLK : IN  std_logic;
         END_TICK : IN  std_logic;
         DATA_SPI_WR : IN  std_logic_vector(31 downto 0);
         DATA_SPI_RD : OUT  std_logic_vector(31 downto 0);
         RAM_ADDR : IN  std_logic_vector(14 downto 0);
         RAM_DATA : INOUT  std_logic_vector(7 downto 0);
         RAM_CE : IN  std_logic;
         RAM_OE : IN  std_logic;
         RAM_WR : IN  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal SYSCLK : std_logic := '0';
   signal END_TICK : std_logic := '0';
   signal DATA_SPI_WR : std_logic_vector(31 downto 0) := (others => '0');
   signal RAM_ADDR : std_logic_vector(14 downto 0) := (others => '0');
   signal RAM_CE : std_logic := '0';
   signal RAM_OE : std_logic := '0';
   signal RAM_WR : std_logic := '0';

	--BiDirs
   signal RAM_DATA : std_logic_vector(7 downto 0);

 	--Outputs
   signal DATA_SPI_RD : std_logic_vector(31 downto 0);

   -- Clock period definitions
   constant SYSCLK_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: bram_wrapper PORT MAP (
          SYSCLK => SYSCLK,
          END_TICK => END_TICK,
          DATA_SPI_WR => DATA_SPI_WR,
          DATA_SPI_RD => DATA_SPI_RD,
          RAM_ADDR => RAM_ADDR,
          RAM_DATA => RAM_DATA,
          RAM_CE => RAM_CE,
          RAM_OE => RAM_OE,
          RAM_WR => RAM_WR
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
      -- hold reset state for 100 ns.
		RAM_OE <= '0';
		RAM_CE <= '1';
		RAM_WR <= '1';
		RAM_ADDR <= "000" & x"005";
		RAM_DATA <= x"A5";
      wait for 55 ns;	
		RAM_CE <= '0';
      wait for 55 ns;	
		RAM_WR <= '0';
      wait for 55 ns;	
		RAM_WR <= '1';
      wait for 55 ns;	
		RAM_CE <= '1';
      wait for 55 ns;	
		RAM_DATA <= (others => 'Z');
		RAM_ADDR <= "000" & x"003";
      wait for 55 ns;	
		RAM_CE <= '0';
		--RAM_OE <= '0';
      wait for 55 ns;	
		--RAM_OE <= '1';
		RAM_CE <= '1';
      wait for 55 ns;	
		RAM_ADDR <= "000" & x"005";
      wait for 55 ns;	
		RAM_CE <= '0';
		--RAM_OE <= '0';
      wait for 55 ns;	
		--RAM_OE <= '1';
		RAM_CE <= '1';
      wait for 100 ns;	

      wait;
   end process;

END;
