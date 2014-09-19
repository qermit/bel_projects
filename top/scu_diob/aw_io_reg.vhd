--TITLE "'aw_io_reg' Autor: R.Hartmann, Stand: 07.08.2014, Vers: V02 ";

-- Version 2, W.Panschow, d. 23.11.2012
--	Ausgang 'AWOut_Reg_Rd_active' hinzugefügt. Kennzeichnet, dass das Macro Daten zum Lesen aum Ausgang 'Data_to_SCUB' bereithält. 'AWOut_Reg_Rd_active' kann übergeordnet zur Steuerung des
--	am 'SCU_Bus_Slave' vorgeschalteten Multiplexers verendet werden. Dieser ist nötig, wenn verschiedene Makros Leseregister zum 'SCU_Bus_Slave'-Eingang 'Data_to_SCUB' anlegen müssen.
--
library IEEE;
USE IEEE.std_logic_1164.all;
USE IEEE.numeric_std.all;
--USE IEEE.std_logic_arith.all;

ENTITY aw_io_reg IS
	generic
		(
		AW_Base_addr:	INTEGER := 16#0200#;
		TAG_Base_addr:	INTEGER := 16#0280#
		);
		
	port(
		Adr_from_SCUB_LA:		in		std_logic_vector(15 downto 0);		-- latched address from SCU_Bus
		Data_from_SCUB_LA:	in		std_logic_vector(15 downto 0);		-- latched data from SCU_Bus 
		Ext_Adr_Val:			in		std_logic;							-- '1' => "ADR_from_SCUB_LA" is valid
		Ext_Rd_active:			in		std_logic;							-- '1' => Rd-Cycle is active
		Ext_Rd_fin:				in		std_logic;							-- marks end of read cycle, active one for one clock period of sys_clk
		Ext_Wr_active:			in		std_logic;							-- '1' => Wr-Cycle is active
		Ext_Wr_fin:				in		std_logic;							-- marks end of write cycle, active one for one clock period of sys_clk
      Timing_Pattern_LA:   in    std_logic_vector(31 downto 0);-- latched timing pattern from SCU_Bus for external user functions
      Timing_Pattern_RCV:  in    std_logic;							-- timing pattern received
		clk:						in		std_logic;							-- should be the same clk, used by SCU_Bus_Slave
		nReset:					in		std_logic;
		AWIn1:					in		std_logic_vector(15 downto 0);	-- Input-Port 1
		AWIn2:					in		std_logic_vector(15 downto 0);	-- Input-Port 2
		AWIn3:					in		std_logic_vector(15 downto 0);	-- Input-Port 3
		AWIn4:					in		std_logic_vector(15 downto 0);	-- Input-Port 4
		AWIn5:					in		std_logic_vector(15 downto 0);	-- Input-Port 5
		AWIn6:					in		std_logic_vector(15 downto 0);	-- Input-Port 6
		AWIn7:					in		std_logic_vector(15 downto 0);	-- Input-Port 7
		AW_Config:				out 	std_logic_vector(15 downto 0);	-- Anwender-Config-Register
		AWOut_Reg1:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut1
		AWOut_Reg2:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut2
		AWOut_Reg3:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut3
		AWOut_Reg4:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut4
		AWOut_Reg5:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut5
		AWOut_Reg6:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut6
		AWOut_Reg7:				out	std_logic_vector(15 downto 0);	-- Daten-Reg. AWOut7
		AW_Config_Wr:			out	std_logic;											-- write Config-Reg. 
		AWOut_Reg1_Wr:			out	std_logic;											-- write Data-Reg. 1 
		AWOut_Reg2_Wr:			out	std_logic;											-- write Data-Reg. 2 
		AWOut_Reg3_Wr:			out	std_logic;											-- write Data-Reg. 3 
		AWOut_Reg4_Wr:			out	std_logic;											-- write Data-Reg. 4 
		AWOut_Reg5_Wr:			out	std_logic;											-- write Data-Reg. 5 
		AWOut_Reg6_Wr:			out	std_logic;											-- write Data-Reg. 6 
		AWOut_Reg7_Wr:			out	std_logic;											-- write Data-Reg. 7 
		AWOut_Reg_rd_active:	out std_logic;									-- read data available at 'Data_to_SCUB'-AWOut
		Data_to_SCUB:			out std_logic_vector(15 downto 0);	-- connect read sources to SCUB-Macro
		Dtack_to_SCUB:			out std_logic;											-- connect Dtack to SCUB-Macro
      LA_aw_io_reg:        out std_logic_vector(15 downto 0)
		);	
	end aw_io_reg;


ARCHITECTURE Arch_aw_io_reg OF aw_io_reg IS

constant	addr_width:						INTEGER := Adr_from_SCUB_LA'length;
constant	AW_Config_addr_offset:		INTEGER := 0;		-- Offset zur Base_addr zum Setzen oder Rücklesen des Config-Registers
constant	AWOut_Reg_1_addr_offset:	INTEGER := 1;		-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_1 Registers
constant	AWOut_Reg_2_addr_offset:	INTEGER := 2;		-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_2 Registers
constant	AWOut_Reg_3_addr_offset:	INTEGER := 3;		-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_3 Registers
constant	AWOut_Reg_4_addr_offset:	INTEGER := 4;		-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_4 Registers
constant	AWOut_Reg_5_addr_offset:	INTEGER := 5;		-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_5 Registers
constant	AWOut_Reg_6_addr_offset:	INTEGER := 6;		-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_6 Registers
constant	AWOut_Reg_7_addr_offset:	INTEGER := 7;		-- Offset zur Base_addr zum Setzen oder Rücklesen des AWOut_Reg_7 Registers
constant	AWIn_1_addr_offset:			INTEGER := 17;		-- Offset zur Base_addr zum Rücklesen des AWIN_Port1
constant	AWIn_2_addr_offset:			INTEGER := 18;		-- Offset zur Base_addr zum Rücklesen des AWIN_Port2
constant	AWIn_3_addr_offset:			INTEGER := 19;		-- Offset zur Base_addr zum Rücklesen des AWIN_Port3
constant	AWIn_4_addr_offset:			INTEGER := 20;		-- Offset zur Base_addr zum Rücklesen des AWIN_Port4
constant	AWIn_5_addr_offset:			INTEGER := 21;		-- Offset zur Base_addr zum Rücklesen des AWIN_Port5
constant	AWIn_6_addr_offset:			INTEGER := 22;		-- Offset zur Base_addr zum Rücklesen des AWIN_Port6
constant	AWIn_7_addr_offset:			INTEGER := 23;		-- Offset zur Base_addr zum Rücklesen des AWIN_Port7

constant	C_AW_Config_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AW_Config_addr_offset),   addr_width);	-- Adresse zum Setzen oder Rücklesen des AW_Config_Registers
constant	C_AWOut_Reg_1_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_1_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_1 Registers
constant	C_AWOut_Reg_2_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_2_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_2 Registers
constant	C_AWOut_Reg_3_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_3_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_3 Registers
constant	C_AWOut_Reg_4_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_4_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_4 Registers
constant	C_AWOut_Reg_5_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_5_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_5 Registers
constant	C_AWOut_Reg_6_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_6_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_6 Registers
constant	C_AWOut_Reg_7_Addr: 	unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWOut_Reg_7_addr_offset), addr_width);	-- Adresse zum Setzen oder Rücklesen des AWOut_Reg_7 Registers

constant	C_AWIN_1_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_1_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn1
constant	C_AWIN_2_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_2_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn2
constant	C_AWIN_3_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_3_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn3
constant	C_AWIN_4_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_4_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn4
constant	C_AWIN_5_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_5_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn5
constant	C_AWIN_6_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_6_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn6
constant	C_AWIN_7_Addr: 		unsigned(addr_width-1 downto 0) := to_unsigned((AW_Base_addr + AWIn_7_addr_offset), addr_width);	-- Adresse zum Lesen des AWIn7


signal		S_AW_Config:		std_logic_vector(15 downto 0);
signal		S_AW_Config_Rd:	std_logic;
signal		S_AW_Config_Wr:	std_logic;

signal		S_AWOut_Reg_1:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_1_Rd:	std_logic;
signal		S_AWOut_Reg_1_Wr:	std_logic;

signal		S_AWOut_Reg_2:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_2_Rd:	std_logic;
signal		S_AWOut_Reg_2_Wr:	std_logic;

signal		S_AWOut_Reg_3:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_3_Rd:	std_logic;
signal		S_AWOut_Reg_3_Wr:	std_logic;

signal		S_AWOut_Reg_4:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_4_Rd:	std_logic;
signal		S_AWOut_Reg_4_Wr:	std_logic;

signal		S_AWOut_Reg_5:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_5_Rd:	std_logic;
signal		S_AWOut_Reg_5_Wr:	std_logic;

signal		S_AWOut_Reg_6:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_6_Rd:	std_logic;
signal		S_AWOut_Reg_6_Wr:	std_logic;

signal		S_AWOut_Reg_7:		std_logic_vector(15 downto 0);
signal		S_AWOut_Reg_7_Rd:	std_logic;
signal		S_AWOut_Reg_7_Wr:	std_logic;

signal		S_AWIn1_Rd:			std_logic;
signal		S_AWIn2_Rd:			std_logic;
signal		S_AWIn3_Rd:			std_logic;
signal		S_AWIn4_Rd:			std_logic;
signal		S_AWIn5_Rd:			std_logic;
signal		S_AWIn6_Rd:			std_logic;
signal		S_AWIn7_Rd:			std_logic;


constant	Tag_Base_0_addr_offset:	INTEGER := 00; -- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant	Tag_Base_1_addr_offset:	INTEGER := 16;		-- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-1 Datensatzes
constant	Tag_Base_2_addr_offset:	INTEGER := 32;		-- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-2 Datensatzes
constant	Tag_Base_3_addr_offset:	INTEGER := 48;		-- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-3 Datensatzes
constant	Tag_Base_4_addr_offset:	INTEGER := 64;		-- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-4 Datensatzes
constant	Tag_Base_5_addr_offset:	INTEGER := 80;		-- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-5 Datensatzes
constant	Tag_Base_6_addr_offset:	INTEGER := 96;		-- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-6 Datensatzes
constant	Tag_Base_7_addr_offset:	INTEGER := 112;		-- Offset zur Tag_Base_addr zum Setzen oder Rücklesen des Tag-7 Datensatzes

constant	C_Tag_Base_0_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_0_addr_offset), addr_width);	-- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant	C_Tag_Base_1_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_1_addr_offset), addr_width);	-- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant	C_Tag_Base_2_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_2_addr_offset), addr_width);	-- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant	C_Tag_Base_3_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_3_addr_offset), addr_width);	-- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant	C_Tag_Base_4_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_4_addr_offset), addr_width);	-- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant	C_Tag_Base_5_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_5_addr_offset), addr_width);	-- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant	C_Tag_Base_6_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_6_addr_offset), addr_width);	-- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes
constant	C_Tag_Base_7_Addr: unsigned(addr_width-1 downto 0) := to_unsigned((Tag_base_addr + Tag_Base_7_addr_offset), addr_width);	-- Base-Adr zum Setzen oder Rücklesen des Tag-0 Datensatzes


signal		S_Tag_Base_0_Addr_Rd:	std_logic;
signal		S_Tag_Base_0_Addr_Wr:	std_logic;
signal		S_Tag_Base_1_Addr_Rd:	std_logic;
signal		S_Tag_Base_1_Addr_Wr:	std_logic;
signal		S_Tag_Base_2_Addr_Rd:	std_logic;
signal		S_Tag_Base_2_Addr_Wr:	std_logic;
signal		S_Tag_Base_3_Addr_Rd:	std_logic;
signal		S_Tag_Base_3_Addr_Wr:	std_logic;
signal		S_Tag_Base_4_Addr_Rd:	std_logic;
signal		S_Tag_Base_4_Addr_Wr:	std_logic;
signal		S_Tag_Base_5_Addr_Rd:	std_logic;
signal		S_Tag_Base_5_Addr_Wr:	std_logic;
signal		S_Tag_Base_6_Addr_Rd:	std_logic;
signal		S_Tag_Base_6_Addr_Wr:	std_logic;
signal		S_Tag_Base_7_Addr_Rd:	std_logic;
signal		S_Tag_Base_7_Addr_Wr:	std_logic;

signal		S_Dtack:				std_logic;
signal		S_Read_Port:		std_logic_vector(Data_to_SCUB'range);




constant	i_Tag_HB:        INTEGER := 0; -- Index Tag-Data: High-Byte
constant	i_Tag_LB:        INTEGER := 1; -- Index Tag-Data: Low-Byte
constant	i_Tag_Maske:     INTEGER := 2; -- Index Tag-Level und Tag-Maske
constant	i_Tag_Lev_Reg:   INTEGER := 3; -- Index Tag-Data: High-Byte


TYPE   t_Tag_Element is array (0 to 15) of std_logic_vector(15 downto 0);
TYPE   t_Tag_Array 	 is array (0 to 70) of t_Tag_Element;
signal Tag_Array:  t_Tag_Array;

signal  Tag_cnt:	      integer range 0 to 9  := 0; -- Tag-Pointer
signal  Tag_Level:		std_logic := '0';
signal  Tag_Maske:		std_logic_vector(15 downto 0);
signal  Tag_New_Data:	std_logic_vector(15 downto 0);
signal  Bit_Level:		std_logic; ------------------ Bit-Level

attribute   keep: boolean;
--attribute   keep of Tag_cnt: signal is true;
attribute   keep of Tag_Level: signal is true;


TYPE    t_AWOut_Reg is array (0 to 7) of std_logic_vector(15 downto 0);
signal  S_AWOut_Reg_Array:  t_AWOut_Reg; -- Copy der AWOut-Register 
signal  AWOut_Reg_Nr:			integer range 0 to 7 := 0; -- AWOut-Reg-Pointer
signal  Tag_Loop:           integer range 0 to 3 := 0; -- Loop-Counter
signal  Tag_New_AWOut_Data: boolean := false; -- AWOut-Reg. werden mit S_AWOut_Reg_Array-Daten überschrieben
signal  Tag_New:            std_logic := '0'; -- Tag Auswerte-Loop



begin

P_Adr_Deco:	process (nReset, clk)
	begin
		if nReset = '0' then

			S_AW_Config_Rd <= '0';
			S_AW_Config_Wr <= '0';
			S_AWOut_Reg_1_Rd <= '0';
			S_AWOut_Reg_1_Wr <= '0';
			S_AWOut_Reg_2_Rd <= '0';
			S_AWOut_Reg_2_Wr <= '0';
			S_AWOut_Reg_3_Rd <= '0';
			S_AWOut_Reg_3_Wr <= '0';
			S_AWOut_Reg_4_Rd <= '0';
			S_AWOut_Reg_4_Wr <= '0';
			S_AWOut_Reg_5_Rd <= '0';
			S_AWOut_Reg_5_Wr <= '0';
			S_AWOut_Reg_6_Rd <= '0';
			S_AWOut_Reg_6_Wr <= '0';
			S_AWOut_Reg_7_Rd <= '0';
			S_AWOut_Reg_7_Wr <= '0';

			S_AWIn1_Rd <= '0';
			S_AWIn2_Rd <= '0';
			S_AWIn3_Rd <= '0';
			S_AWIn4_Rd <= '0';
			S_AWIn5_Rd <= '0';
			S_AWIn6_Rd <= '0';
			S_AWIn7_Rd <= '0';

			S_Tag_Base_0_Addr_Rd <= '0';
			S_Tag_Base_0_Addr_Wr <= '0';
			S_Tag_Base_1_Addr_Rd <= '0';
			S_Tag_Base_1_Addr_Wr <= '0';
			S_Tag_Base_2_Addr_Rd <= '0';
			S_Tag_Base_2_Addr_Wr <= '0';
			S_Tag_Base_3_Addr_Rd <= '0';
			S_Tag_Base_3_Addr_Wr <= '0';
			S_Tag_Base_4_Addr_Rd <= '0';
			S_Tag_Base_4_Addr_Wr <= '0';
			S_Tag_Base_5_Addr_Rd <= '0';
			S_Tag_Base_5_Addr_Wr <= '0';
			S_Tag_Base_6_Addr_Rd <= '0';
			S_Tag_Base_6_Addr_Wr <= '0';
			S_Tag_Base_7_Addr_Rd <= '0';
			S_Tag_Base_7_Addr_Wr <= '0';

			S_Dtack <= '0';
			AWOut_Reg_rd_active <= '0';
		
		elsif rising_edge(clk) then
			S_AW_Config_Rd <= '0';
			S_AW_Config_Wr <= '0';
			S_AWOut_Reg_1_Rd <= '0';
			S_AWOut_Reg_1_Wr <= '0';
			S_AWOut_Reg_2_Rd <= '0';
			S_AWOut_Reg_2_Wr <= '0';
			S_AWOut_Reg_3_Rd <= '0';
			S_AWOut_Reg_3_Wr <= '0';
			S_AWOut_Reg_4_Rd <= '0';
			S_AWOut_Reg_4_Wr <= '0';
			S_AWOut_Reg_5_Rd <= '0';
			S_AWOut_Reg_5_Wr <= '0';
			S_AWOut_Reg_6_Rd <= '0';
			S_AWOut_Reg_6_Wr <= '0';
			S_AWOut_Reg_7_Rd <= '0';
			S_AWOut_Reg_7_Wr <= '0';

			S_AWIn1_Rd <= '0';
			S_AWIn2_Rd <= '0';
			S_AWIn3_Rd <= '0';
			S_AWIn4_Rd <= '0';
			S_AWIn5_Rd <= '0';
			S_AWIn6_Rd <= '0';
			S_AWIn7_Rd <= '0';

			S_Tag_Base_0_Addr_Rd <= '0';
			S_Tag_Base_0_Addr_Wr <= '0';
			S_Tag_Base_1_Addr_Rd <= '0';
			S_Tag_Base_1_Addr_Wr <= '0';
			S_Tag_Base_2_Addr_Rd <= '0';
			S_Tag_Base_2_Addr_Wr <= '0';
			S_Tag_Base_3_Addr_Rd <= '0';
			S_Tag_Base_3_Addr_Wr <= '0';
			S_Tag_Base_4_Addr_Rd <= '0';
			S_Tag_Base_4_Addr_Wr <= '0';
			S_Tag_Base_5_Addr_Rd <= '0';
			S_Tag_Base_5_Addr_Wr <= '0';
			S_Tag_Base_6_Addr_Rd <= '0';
			S_Tag_Base_6_Addr_Wr <= '0';
			S_Tag_Base_7_Addr_Rd <= '0';
			S_Tag_Base_7_Addr_Wr <= '0';

			S_Dtack <= '0';
			AWOut_Reg_rd_active <= '0';
			
			if Ext_Adr_Val = '1' then

				CASE unsigned(ADR_from_SCUB_LA) IS

					when C_AW_Config_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AW_Config_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AW_Config_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;
				
					when C_AWOut_Reg_1_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_1_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_1_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

					when C_AWOut_Reg_2_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_2_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_2_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

					when C_AWOut_Reg_3_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_3_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_3_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

					when C_AWOut_Reg_4_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_4_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_4_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWOut_Reg_5_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_5_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_5_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWOut_Reg_6_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_6_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_6_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWOut_Reg_7_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_7_Wr <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_AWOut_Reg_7_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						
						when C_AWIN_1_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn1_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWIN_2_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn2_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWIN_3_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn3_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWIN_4_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn4_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWIN_5_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn5_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWIN_6_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn6_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_AWIN_7_Addr =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '0';				-- kein DTACK beim Lese-Port
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack 		<= '1';
							S_AWIn7_Rd 	<= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						
						
					when others => 

						S_AW_Config_Rd <= '0';
						S_AW_Config_Wr <= '0';
						S_AWOut_Reg_1_Rd <= '0';
						S_AWOut_Reg_1_Wr <= '0';
						S_AWOut_Reg_2_Rd <= '0';
						S_AWOut_Reg_2_Wr <= '0';
						S_AWOut_Reg_3_Rd <= '0';
						S_AWOut_Reg_3_Wr <= '0';
						S_AWOut_Reg_4_Rd <= '0';
						S_AWOut_Reg_4_Wr <= '0';
						S_AWOut_Reg_5_Rd <= '0';
						S_AWOut_Reg_5_Wr <= '0';
						S_AWOut_Reg_6_Rd <= '0';
						S_AWOut_Reg_6_Wr <= '0';
						S_AWOut_Reg_7_Rd <= '0';
						S_AWOut_Reg_7_Wr <= '0';

						S_AWIn1_Rd <= '0';
						S_AWIn2_Rd <= '0';
						S_AWIn3_Rd <= '0';
						S_AWIn4_Rd <= '0';
						S_AWIn5_Rd <= '0';
						S_AWIn6_Rd <= '0';
						S_AWIn7_Rd <= '0';
											
--						S_Dtack <= '0';
--						AWOut_Reg_rd_active <= '0';

				end CASE;

				
				
				CASE unsigned(ADR_from_SCUB_LA(15 downto 4)) IS

						when C_Tag_Base_0_Addr(15 downto 4) =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_0_Addr_WR <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_0_Addr_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_Tag_Base_1_Addr(15 downto 4) =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_1_Addr_WR <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_1_Addr_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_Tag_Base_2_Addr(15 downto 4) =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_2_Addr_WR <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_2_Addr_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_Tag_Base_3_Addr(15 downto 4) =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_3_Addr_WR <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_3_Addr_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_Tag_Base_4_Addr(15 downto 4) =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_4_Addr_WR <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_4_Addr_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_Tag_Base_5_Addr(15 downto 4) =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_5_Addr_WR <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_5_Addr_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_Tag_Base_6_Addr(15 downto 4) =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_6_Addr_WR <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_6_Addr_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;

						when C_Tag_Base_7_Addr(15 downto 4) =>
						if Ext_Wr_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_7_Addr_WR <= '1';
						end if;
						if Ext_Rd_active = '1' then
							S_Dtack <= '1';
							S_Tag_Base_7_Addr_Rd <= '1';
							AWOut_Reg_rd_active <= '1';
						end if;
						
					when others => 

						S_Tag_Base_0_Addr_Rd <= '0';
						S_Tag_Base_0_Addr_Wr <= '0';
						S_Tag_Base_1_Addr_Rd <= '0';
						S_Tag_Base_1_Addr_Wr <= '0';
						S_Tag_Base_2_Addr_Rd <= '0';
						S_Tag_Base_2_Addr_Wr <= '0';
						S_Tag_Base_3_Addr_Rd <= '0';
						S_Tag_Base_3_Addr_Wr <= '0';
						S_Tag_Base_4_Addr_Rd <= '0';
						S_Tag_Base_4_Addr_Wr <= '0';
						S_Tag_Base_5_Addr_Rd <= '0';
						S_Tag_Base_5_Addr_Wr <= '0';
						S_Tag_Base_6_Addr_Rd <= '0';
						S_Tag_Base_6_Addr_Wr <= '0';
						S_Tag_Base_7_Addr_Rd <= '0';
						S_Tag_Base_7_Addr_Wr <= '0';
						
--						S_Dtack <= '0';
--						AWOut_Reg_rd_active <= '0';

				end CASE;

	end if;
		end if;
	
	end process P_Adr_Deco;

P_Tag_Deco:	process (clk)
	begin
		if rising_edge(clk) then

			LA_aw_io_reg  <=   Timing_Pattern_RCV & Timing_Pattern_LA(14 downto 0);-- Testport für Logic-Analysator
			

		if (Tag_Loop = 0) then

        S_AWOut_Reg_Array(0)	<=	(others => '0');	
        S_AWOut_Reg_Array(1)	<=	s_AWOut_Reg_1;		-- copy Daten-Reg. AWOut1
        S_AWOut_Reg_Array(2)	<=	s_AWOut_Reg_2;		-- copy Daten-Reg. AWOut2
        S_AWOut_Reg_Array(3)	<=	s_AWOut_Reg_3;		-- copy Daten-Reg. AWOut3
        S_AWOut_Reg_Array(4)	<=	s_AWOut_Reg_4;		-- copy Daten-Reg. AWOut4
        S_AWOut_Reg_Array(5)	<=	s_AWOut_Reg_5;		-- copy Daten-Reg. AWOut5
        S_AWOut_Reg_Array(6)	<=	s_AWOut_Reg_6;		-- copy Daten-Reg. AWOut6
        S_AWOut_Reg_Array(7)	<=	s_AWOut_Reg_7;		-- copy Daten-Reg. AWOut6

        if (Timing_Pattern_RCV = '1') then


----------------------------------- TAG 0 -----------------------------------------------------------
         IF Timing_Pattern_LA(31 downto 0) = (Tag_Array(0)(i_Tag_HB) & Tag_Array(0)(i_Tag_LB)) then

           Tag_Maske	   <=	Tag_Array(0)(i_Tag_Maske);                                     -- Tag_Array(0)(i_Tag_Maske)   = Maske für Output-Bits
           Tag_Level	   <=	Tag_Array(0)(i_Tag_Lev_Reg)(15);                               -- Tag_Array(0)(i_Tag_Lev_Reg) = Level für Output-Bits
           AWOut_Reg_Nr	<=	to_integer(unsigned(Tag_Array(0)(i_Tag_Lev_Reg))(3 downto 0)); -- Tag_Array(0)(i_Tag_Lev_Reg) = Output-Reg. Nr. 0..7				 
			  
           IF Tag_Level = '0' then
              Tag_New_Data     <= (S_AWOut_Reg_Array(AWOut_Reg_Nr) and (not Tag_Maske));  -- alle Bits der Maske,  werden auf 0 gesetzt
           else
              Tag_New_Data     <=  (S_AWOut_Reg_Array(AWOut_Reg_Nr) or Tag_Maske);        -- alle Bits der Maske,  werden auf 1 gesetzt
           end if;				 

           S_AWOut_Reg_Array(AWOut_Reg_Nr) <= Tag_New_Data;                               -- neue Daten zurückschreiben
			  
----------------------------------- TAG 1 -----------------------------------------------------------
         ELSIF Timing_Pattern_LA(31 downto 0) = (Tag_Array(1)(i_Tag_HB) & Tag_Array(1)(i_Tag_LB)) then

           Tag_Maske	   <=	Tag_Array(1)(i_Tag_Maske);                                     -- Tag_Array(1)(i_Tag_Maske)   = Maske für Output-Bits
           Tag_Level	   <=	Tag_Array(1)(i_Tag_Lev_Reg)(15);                               -- Tag_Array(1)(i_Tag_Lev_Reg) = Level für Output-Bits
           AWOut_Reg_Nr	<=	to_integer(unsigned(Tag_Array(1)(i_Tag_Lev_Reg))(3 downto 0)); -- Tag_Array(1)(i_Tag_Lev_Reg) = Output-Reg. Nr. 0..7				 
			  
           IF Tag_Level = '0' then
              Tag_New_Data     <= (S_AWOut_Reg_Array(AWOut_Reg_Nr) and (not Tag_Maske));  -- alle Bits der Maske,  werden auf 0 gesetzt
           else
              Tag_New_Data     <=  (S_AWOut_Reg_Array(AWOut_Reg_Nr) or Tag_Maske);        -- alle Bits der Maske,  werden auf 1 gesetzt
           end if;				 

           S_AWOut_Reg_Array(AWOut_Reg_Nr) <= Tag_New_Data;                               -- neue Daten zurückschreiben

----------------------------------- TAG 2 -----------------------------------------------------------
         ELSIF Timing_Pattern_LA(31 downto 0) = (Tag_Array(2)(i_Tag_HB) & Tag_Array(2)(i_Tag_LB)) then

           Tag_Maske	   <=	Tag_Array(2)(i_Tag_Maske);                                     -- Tag_Array(2)(i_Tag_Maske)   = Maske für Output-Bits
           Tag_Level	   <=	Tag_Array(2)(i_Tag_Lev_Reg)(15);                               -- Tag_Array(2)(i_Tag_Lev_Reg) = Level für Output-Bits
           AWOut_Reg_Nr	<=	to_integer(unsigned(Tag_Array(2)(i_Tag_Lev_Reg))(3 downto 0)); -- Tag_Array(2)(i_Tag_Lev_Reg) = Output-Reg. Nr. 0..7				 
			  
           IF Tag_Level = '0' then
              Tag_New_Data     <= (S_AWOut_Reg_Array(AWOut_Reg_Nr) and (not Tag_Maske));  -- alle Bits der Maske,  werden auf 0 gesetzt
           else
              Tag_New_Data     <=  (S_AWOut_Reg_Array(AWOut_Reg_Nr) or Tag_Maske);        -- alle Bits der Maske,  werden auf 1 gesetzt
           end if;				 

           S_AWOut_Reg_Array(AWOut_Reg_Nr) <= Tag_New_Data;                               -- neue Daten zurückschreiben

----------------------------------- TAG 3 -----------------------------------------------------------
         ELSIF Timing_Pattern_LA(31 downto 0) = (Tag_Array(3)(i_Tag_HB) & Tag_Array(3)(i_Tag_LB)) then

           Tag_Maske	   <=	Tag_Array(3)(i_Tag_Maske);                                     -- Tag_Array(3)(i_Tag_Maske)   = Maske für Output-Bits
           Tag_Level	   <=	Tag_Array(3)(i_Tag_Lev_Reg)(15);                               -- Tag_Array(3)(i_Tag_Lev_Reg) = Level für Output-Bits
           AWOut_Reg_Nr	<=	to_integer(unsigned(Tag_Array(3)(i_Tag_Lev_Reg))(3 downto 0)); -- Tag_Array(3)(i_Tag_Lev_Reg) = Output-Reg. Nr. 0..7				 
			  
           IF Tag_Level = '0' then
              Tag_New_Data     <= (S_AWOut_Reg_Array(AWOut_Reg_Nr) and (not Tag_Maske));  -- alle Bits der Maske,  werden auf 0 gesetzt
           else
              Tag_New_Data     <=  (S_AWOut_Reg_Array(AWOut_Reg_Nr) or Tag_Maske);        -- alle Bits der Maske,  werden auf 1 gesetzt
           end if;				 

           S_AWOut_Reg_Array(AWOut_Reg_Nr) <= Tag_New_Data;                               -- neue Daten zurückschreiben

----------------------------------- TAG 4 -----------------------------------------------------------
         ELSIF Timing_Pattern_LA(31 downto 0) = (Tag_Array(4)(i_Tag_HB) & Tag_Array(4)(i_Tag_LB)) then

           Tag_Maske	   <=	Tag_Array(4)(i_Tag_Maske);                                     -- Tag_Array(4)(i_Tag_Maske)   = Maske für Output-Bits
           Tag_Level	   <=	Tag_Array(4)(i_Tag_Lev_Reg)(15);                               -- Tag_Array(4)(i_Tag_Lev_Reg) = Level für Output-Bits
           AWOut_Reg_Nr	<=	to_integer(unsigned(Tag_Array(4)(i_Tag_Lev_Reg))(3 downto 0)); -- Tag_Array(4)(i_Tag_Lev_Reg) = Output-Reg. Nr. 0..7				 
			  
           IF Tag_Level = '0' then
              Tag_New_Data     <= (S_AWOut_Reg_Array(AWOut_Reg_Nr) and (not Tag_Maske));  -- alle Bits der Maske,  werden auf 0 gesetzt
           else
              Tag_New_Data     <=  (S_AWOut_Reg_Array(AWOut_Reg_Nr) or Tag_Maske);        -- alle Bits der Maske,  werden auf 1 gesetzt
           end if;				 

           S_AWOut_Reg_Array(AWOut_Reg_Nr) <= Tag_New_Data;                               -- neue Daten zurückschreiben

----------------------------------- TAG 5 -----------------------------------------------------------
         ELSIF Timing_Pattern_LA(31 downto 0) = (Tag_Array(5)(i_Tag_HB) & Tag_Array(5)(i_Tag_LB)) then

           Tag_Maske	   <=	Tag_Array(5)(i_Tag_Maske);                                     -- Tag_Array(5)(i_Tag_Maske)   = Maske für Output-Bits
           Tag_Level	   <=	Tag_Array(5)(i_Tag_Lev_Reg)(15);                               -- Tag_Array(5)(i_Tag_Lev_Reg) = Level für Output-Bits
           AWOut_Reg_Nr	<=	to_integer(unsigned(Tag_Array(5)(i_Tag_Lev_Reg))(3 downto 0)); -- Tag_Array(5)(i_Tag_Lev_Reg) = Output-Reg. Nr. 0..7				 
			  
           IF Tag_Level = '0' then
              Tag_New_Data     <= (S_AWOut_Reg_Array(AWOut_Reg_Nr) and (not Tag_Maske));  -- alle Bits der Maske,  werden auf 0 gesetzt
           else
              Tag_New_Data     <=  (S_AWOut_Reg_Array(AWOut_Reg_Nr) or Tag_Maske);        -- alle Bits der Maske,  werden auf 1 gesetzt
           end if;				 

           S_AWOut_Reg_Array(AWOut_Reg_Nr) <= Tag_New_Data;                               -- neue Daten zurückschreiben

----------------------------------- TAG 6 -----------------------------------------------------------
         ELSIF Timing_Pattern_LA(31 downto 0) = (Tag_Array(6)(i_Tag_HB) & Tag_Array(6)(i_Tag_LB)) then

           Tag_Maske	   <=	Tag_Array(6)(i_Tag_Maske);                                     -- Tag_Array(6)(i_Tag_Maske)   = Maske für Output-Bits
           Tag_Level	   <=	Tag_Array(6)(i_Tag_Lev_Reg)(15);                               -- Tag_Array(6)(i_Tag_Lev_Reg) = Level für Output-Bits
           AWOut_Reg_Nr	<=	to_integer(unsigned(Tag_Array(6)(i_Tag_Lev_Reg))(3 downto 0)); -- Tag_Array(6)(i_Tag_Lev_Reg) = Output-Reg. Nr. 0..7				 
			  
           IF Tag_Level = '0' then
              Tag_New_Data     <= (S_AWOut_Reg_Array(AWOut_Reg_Nr) and (not Tag_Maske));  -- alle Bits der Maske,  werden auf 0 gesetzt
           else
              Tag_New_Data     <=  (S_AWOut_Reg_Array(AWOut_Reg_Nr) or Tag_Maske);        -- alle Bits der Maske,  werden auf 1 gesetzt
           end if;				 

           S_AWOut_Reg_Array(AWOut_Reg_Nr) <= Tag_New_Data;                               -- neue Daten zurückschreiben

----------------------------------- TAG 7 -----------------------------------------------------------
         ELSIF Timing_Pattern_LA(31 downto 0) = (Tag_Array(7)(i_Tag_HB) & Tag_Array(7)(i_Tag_LB)) then

           Tag_Maske	   <=	Tag_Array(7)(i_Tag_Maske);                                     -- Tag_Array(7)(i_Tag_Maske)   = Maske für Output-Bits
           Tag_Level	   <=	Tag_Array(7)(i_Tag_Lev_Reg)(15);                               -- Tag_Array(7)(i_Tag_Lev_Reg) = Level für Output-Bits
           AWOut_Reg_Nr	<=	to_integer(unsigned(Tag_Array(7)(i_Tag_Lev_Reg))(3 downto 0)); -- Tag_Array(7)(i_Tag_Lev_Reg) = Output-Reg. Nr. 0..7				 
			  
           IF Tag_Level = '0' then
              Tag_New_Data     <= (S_AWOut_Reg_Array(AWOut_Reg_Nr) and (not Tag_Maske));  -- alle Bits der Maske,  werden auf 0 gesetzt
           else
              Tag_New_Data     <=  (S_AWOut_Reg_Array(AWOut_Reg_Nr) or Tag_Maske);        -- alle Bits der Maske,  werden auf 1 gesetzt
           end if;				 

           S_AWOut_Reg_Array(AWOut_Reg_Nr) <= Tag_New_Data;                               -- neue Daten zurückschreiben
			end if;
----------------------------------------------------------------------------------------------------
			  
         Tag_New_AWOut_Data <= true;  -- set Tag_New_AWOut_Data => AWOut-Reg. werden mit S_AWOut_Reg_Array-Daten überschrieben
         Tag_Loop <= 1;               -- set Tag-Loop
			end if; -- Timing_Pattern_RCV

        else
         Tag_New_AWOut_Data <= false; -- reset Tag_New_AWOut_Data
		   Tag_Loop <= 0;               -- reset Tag_Loop
		  end if; -- Tag_Loop
     end if; -- rising_edge(clk)
						
	end process P_Tag_Deco;
	

	
	
P_AWOut_Reg:	process (nReset, clk)
	begin
		if nReset = '0' then
			S_AW_Config	  <= (others => '0');
			S_AWOut_Reg_1 <= (others => '0');
			S_AWOut_Reg_2 <= (others => '0');
			S_AWOut_Reg_3 <= (others => '0');
			S_AWOut_Reg_4 <= (others => '0');
			S_AWOut_Reg_5 <= (others => '0');
			S_AWOut_Reg_6 <= (others => '0');
			S_AWOut_Reg_7 <= (others => '0');
		
		elsif rising_edge(clk) then
			if S_AW_Config_Wr = '1' 	then	S_AW_Config <= Data_from_SCUB_LA;
			end if;

			if S_AWOut_Reg_1_Wr = '1' then	S_AWOut_Reg_1 <= Data_from_SCUB_LA;
			elsif Tag_New_AWOut_Data  then   S_AWOut_Reg_1 <= S_AWOut_Reg_Array(1);
			end if;

			if S_AWOut_Reg_2_Wr = '1' then	S_AWOut_Reg_2 <= Data_from_SCUB_LA;
			elsif Tag_New_AWOut_Data  then   S_AWOut_Reg_2 <= S_AWOut_Reg_Array(2);
			end if;

			if S_AWOut_Reg_3_Wr = '1' then	S_AWOut_Reg_3 <= Data_from_SCUB_LA;
			elsif Tag_New_AWOut_Data  then   S_AWOut_Reg_3 <= S_AWOut_Reg_Array(3);
			end if;

			if S_AWOut_Reg_4_Wr = '1' then	S_AWOut_Reg_4 <= Data_from_SCUB_LA;
			elsif Tag_New_AWOut_Data  then   S_AWOut_Reg_4 <= S_AWOut_Reg_Array(4);
			end if;

			if S_AWOut_Reg_5_Wr = '1' then	S_AWOut_Reg_5 <= Data_from_SCUB_LA;
			elsif Tag_New_AWOut_Data  then   S_AWOut_Reg_5 <= S_AWOut_Reg_Array(5);
			end if;

			if S_AWOut_Reg_6_Wr = '1' then	S_AWOut_Reg_6 <= Data_from_SCUB_LA;
			elsif Tag_New_AWOut_Data  then   S_AWOut_Reg_6 <= S_AWOut_Reg_Array(6);
			end if;

			if S_AWOut_Reg_7_Wr = '1' then	S_AWOut_Reg_7 <= Data_from_SCUB_LA;
			elsif Tag_New_AWOut_Data  then   S_AWOut_Reg_7 <= S_AWOut_Reg_Array(7);
			end if;


			                                                 
			if S_Tag_Base_0_Addr_Wr = '1' then	             
--                                                        
--                 +--- Zeilen-Nr. im Array
--                 |  +-------------------- Adresse der "Wordposition" -+
--                 |  |                                                 |
			Tag_Array(0)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
			end if;

			if S_Tag_Base_1_Addr_Wr = '1' then	
			Tag_Array(1)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
			end if;
			if S_Tag_Base_2_Addr_Wr = '1' then	
			Tag_Array(2)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
			end if;
			if S_Tag_Base_3_Addr_Wr = '1' then	
			Tag_Array(3)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
			end if;
			if S_Tag_Base_4_Addr_Wr = '1' then	
			Tag_Array(4)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
			end if;
			if S_Tag_Base_5_Addr_Wr = '1' then	
			Tag_Array(5)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
			end if;
			if S_Tag_Base_6_Addr_Wr = '1' then	
			Tag_Array(6)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
			end if;
			if S_Tag_Base_7_Addr_Wr = '1' then	
			Tag_Array(7)(to_integer(unsigned (Adr_from_SCUB_LA(3 downto 0)))) <= Data_from_SCUB_LA;
			end if;

	end if;
  end process P_AWOut_Reg;
	

	

	P_read_mux:	process (S_AW_Config_Rd,	  S_AW_Config,
											 S_AWOut_Reg_1_Rd,  S_AWOut_Reg_1,
											 S_AWOut_Reg_2_Rd,  S_AWOut_Reg_2,
											 S_AWOut_Reg_3_Rd,  S_AWOut_Reg_3,
											 S_AWOut_Reg_4_Rd,  S_AWOut_Reg_4,
											 S_AWOut_Reg_5_Rd,  S_AWOut_Reg_5,
											 S_AWOut_Reg_6_Rd,  S_AWOut_Reg_6,
											 S_AWOut_Reg_7_Rd,  S_AWOut_Reg_7,
											 S_AWIn1_Rd,				AWIn1,
											 S_AWIn2_Rd,				AWIn2,
											 S_AWIn3_Rd,				AWIn3,
											 S_AWIn4_Rd,				AWIn4,
											 S_AWIn5_Rd,				AWIn5,
											 S_AWIn6_Rd,				AWIn6,
											 S_AWIn7_Rd,				AWIn7,
											 S_Tag_Base_0_Addr_Rd, S_Tag_Base_1_Addr_Rd,
											 S_Tag_Base_2_Addr_Rd, S_Tag_Base_3_Addr_Rd,
											 S_Tag_Base_4_Addr_Rd, S_Tag_Base_5_Addr_Rd,
											 S_Tag_Base_6_Addr_Rd, S_Tag_Base_7_Addr_Rd)

	begin
		if S_AW_Config_Rd 	  = '1' then	S_Read_port <= S_AW_Config;
		elsif S_AWOut_Reg_1_Rd = '1' then	S_Read_port <= S_AWOut_Reg_1;
		elsif S_AWOut_Reg_2_Rd = '1' then	S_Read_port <= S_AWOut_Reg_2;
		elsif S_AWOut_Reg_3_Rd = '1' then	S_Read_port <= S_AWOut_Reg_3;
		elsif S_AWOut_Reg_4_Rd = '1' then	S_Read_port <= S_AWOut_Reg_4;
		elsif S_AWOut_Reg_5_Rd = '1' then	S_Read_port <= S_AWOut_Reg_5;
		elsif S_AWOut_Reg_6_Rd = '1' then	S_Read_port <= S_AWOut_Reg_6;
		elsif S_AWOut_Reg_7_Rd = '1' then	S_Read_port <= S_AWOut_Reg_7;

		elsif S_AWIn1_Rd = '1' then	S_Read_port <= AWIn1;		-- read Input-Port1
		elsif S_AWIn2_Rd = '1' then	S_Read_port <= AWIn2;
		elsif S_AWIn3_Rd = '1' then	S_Read_port <= AWIn3;
		elsif S_AWIn4_Rd = '1' then	S_Read_port <= AWIn4;
		elsif S_AWIn5_Rd = '1' then	S_Read_port <= AWIn5;
		elsif S_AWIn6_Rd = '1' then	S_Read_port <= AWIn6;
		elsif S_AWIn7_Rd = '1' then	S_Read_port <= AWIn7;

--                                                                    +--- Zeilen-Nr. im Array
--                                                                    |  +---- Adresse der "Wordposition" in der Zeile ----+
--                                                                    |  |                                                 |
		elsif S_Tag_Base_0_Addr_Rd = '1' then	S_Read_port <= Tag_Array(0)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
		elsif S_Tag_Base_1_Addr_Rd = '1' then	S_Read_port <= Tag_Array(1)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
		elsif S_Tag_Base_2_Addr_Rd = '1' then	S_Read_port <= Tag_Array(2)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
		elsif S_Tag_Base_3_Addr_Rd = '1' then	S_Read_port <= Tag_Array(3)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
		elsif S_Tag_Base_4_Addr_Rd = '1' then	S_Read_port <= Tag_Array(4)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
		elsif S_Tag_Base_5_Addr_Rd = '1' then	S_Read_port <= Tag_Array(5)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
		elsif S_Tag_Base_6_Addr_Rd = '1' then	S_Read_port <= Tag_Array(6)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));
		elsif S_Tag_Base_7_Addr_Rd = '1' then	S_Read_port <= Tag_Array(7)(to_integer(unsigned (Adr_from_SCUB_LA(2 downto 0))));



	else
			S_Read_Port <= (others => '-');
		end if;
	end process P_Read_mux;

	
Dtack_to_SCUB <= S_Dtack;

Data_to_SCUB <= S_Read_Port;


AW_Config   <= S_AW_Config;		-- Configurations-Reg.
AWOut_Reg1 <= S_AWOut_Reg_1;		-- Daten-Reg. AWOut1
AWOut_Reg2 <= S_AWOut_Reg_2;		-- Daten-Reg. AWOut2
AWOut_Reg3 <= S_AWOut_Reg_3;		-- Daten-Reg. AWOut3
AWOut_Reg4 <= S_AWOut_Reg_4;		-- Daten-Reg. AWOut4
AWOut_Reg5 <= S_AWOut_Reg_5;		-- Daten-Reg. AWOut5
AWOut_Reg6 <= S_AWOut_Reg_6;		-- Daten-Reg. AWOut6
AWOut_Reg7 <= S_AWOut_Reg_7;		-- Daten-Reg. AWOut7


AW_Config_Wr	<= S_AW_Config_Wr;			-- write Configurations-Reg.
AWOut_Reg1_Wr <= S_AWOut_Reg_1_Wr;		-- write Daten-Reg. AWOut1
AWOut_Reg2_Wr <= S_AWOut_Reg_2_Wr;		-- write Daten-Reg. AWOut2
AWOut_Reg3_Wr <= S_AWOut_Reg_3_Wr;		-- write Daten-Reg. AWOut3
AWOut_Reg4_Wr <= S_AWOut_Reg_4_Wr;		-- write Daten-Reg. AWOut4
AWOut_Reg5_Wr <= S_AWOut_Reg_5_Wr;		-- write Daten-Reg. AWOut5
AWOut_Reg6_Wr <= S_AWOut_Reg_6_Wr;		-- write Daten-Reg. AWOut6
AWOut_Reg7_Wr <= S_AWOut_Reg_7_Wr;		-- write Daten-Reg. AWOut7


end Arch_AW_IO_Reg;