-- md5_control.vhd

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

entity md5_control is
	port (
		avs_s0_address   : in  std_logic_vector(8 downto 0)  := (others => '0'); --         s0.address
		avs_s0_write     : in  std_logic                     := '0';             --           .write
		avs_s0_writedata : in  std_logic_vector(31 downto 0) := (others => '0'); --           .writedata
		avs_s0_read      : in  std_logic                     := '0';             --           .read
		avs_s0_readdata  : out std_logic_vector(31 downto 0);                    --           .readdata
		clk              : in  std_logic                     := '0';             --      clock.clk
		reset            : in  std_logic                     := '0';             --      reset.reset
		md5_start       : out std_logic_vector(31 downto 0);                    --           .m_start
		md5_reset       : out std_logic_vector(31 downto 0);                    --           .m_reset
		md5_wen       	: out std_logic_vector(31 downto 0);                    --           .m_reset
		md5_done        : in  std_logic_vector(31 downto 0) := (others => '0')  --            .m_done
	);
end entity md5_control;

architecture rtl of md5_control is

	SIGNAL start, reset2, wen : STD_logic_vector(31 DOWNTO 0);

begin

		PROCESS(clk, reset, avs_s0_address, avs_s0_write, avs_s0_writedata, avs_s0_read)
	BEGIN
		IF(reset = '1')THEN
			start <= (OTHERS => '0');
			reset2 <= (OTHERS => '0');
		ELSIF(rising_edge(clk))THEN

			IF(avs_s0_write = '1')THEN
				CASE avs_s0_address (3 downto 0) IS
					WHEN "0000" =>
						start <= avs_s0_writedata;
					WHEN "0001" => 
						reset2 <= avs_s0_writedata;
					WHEN "0010" => 
						wen <= avs_s0_writedata;
					WHEN OTHERS =>

				END CASE;			
			ELSIF(avs_s0_read = '1')THEN
				reset2 <= (OTHERS => '0');
				CASE avs_s0_address (3 downto 0)IS
					WHEN "0000" =>
						avs_s0_readdata <= start;
					WHEN "0001" => 
						avs_s0_readdata <= reset2;
					WHEN "0010" =>
						avs_s0_readdata <= md5_done;
					WHEN OTHERS => 
					
				END CASE;
			END IF;
		END IF;
	END PROCESS;
	
	md5_wen <= wen;
	md5_start <= start;
	md5_reset <= reset2;
	
end architecture rtl; -- of md5_input
