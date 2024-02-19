-- md5_data.vhd
library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity md5_data is
	port (
		avs_s0_address  : in  std_logic_vector(8 downto 0)  := (others => '0'); --          s0.address
		avs_s0_read     : in  std_logic                     := '0';             --            .read
		avs_s0_write    : in  std_logic                     := '0';             --            .write
		avs_s0_readdata : out std_logic_vector(31 downto 0);                    --            .readdata
		avs_s0_writedata  : in  std_logic_vector(31 downto 0)  := (others => '0'); --          s0.address
		clk             : in  std_logic                     := '0';             --       clock.clk
		reset           : in  std_logic                     := '0';             --       reset.reset
		md5_datain        : out std_logic_vector(31 downto 0);                    -- md5_input.m_in1
		md5_addrin        : out std_logic_vector(31 downto 0);                    -- md5_input.m_in2
		md5_digest0    : in  std_logic_vector(31 downto 0) := (others => '0') -- md5_output.m_result
);
end entity md5_data;

architecture rtl of md5_data is

	SIGNAL in1, in2 : STD_logic_vector(31 DOWNTO 0);
	
begin
	PROCESS(clk, reset, avs_s0_read, avs_s0_write, avs_s0_address, avs_s0_writedata)
	BEGIN
		IF(reset = '1')THEN
			avs_s0_readdata <= (OTHERS => '0');
			in1 <= (OTHERS => '0');
			in2 <= (OTHERS => '0');
			
		ELSIF(rising_edge(clk))THEN
			IF(avs_s0_read = '1')THEN
				CASE avs_s0_address (4 downto 0)IS
					WHEN "00000" =>
						avs_s0_readdata <= md5_digest0;
					WHEN "00010" =>
						avs_s0_readdata <= in1;
					WHEN "10010" =>
						avs_s0_readdata <= in1;
					WHEN "00011" =>
						avs_s0_readdata <= in2;
					WHEN "10011" =>
						avs_s0_readdata <= in2;
					WHEN "11111" =>
						avs_s0_readdata (31 downto 5)<= (OTHERS => '0');
						avs_s0_readdata (4 downto 0)<= avs_s0_address (8 downto 4); --gives me which md5 and digest
					WHEN OTHERS =>
						avs_s0_readdata <= (OTHERS => '0');
				END CASE;
			ELSIF(avs_s0_write = '1')THEN
				CASE avs_s0_address (3 downto 0)IS
					WHEN "0000" =>
						in1 <= avs_s0_writedata; --datain
					WHEN "0001" =>
						in2 <= avs_s0_writedata ; --address
					WHEN OTHERS =>
					
					END CASE;
			
			END iF;
		END IF;
	END PROCESS;
	
	md5_datain <= in1;
	md5_addrin <= in2;
	
end architecture rtl; -- of md5_output
