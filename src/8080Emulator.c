#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "8080Emulator.h"

void initialize8080(Intel8080* const cpu)
{
	for (int i = 0; i < sizeof(cpu->registers); i++)
	{
		cpu->registers[i] = 0;
	}

	for (int i = 0; i < sizeof(cpu->conditionFlags); i++)
	{
		cpu->conditionFlags[i] = 0;
	}

	cpu->stackPointer = 0;
	cpu->programCounter = 0;
	cpu->memory = 0;
}

void execute8080Opcode(Intel8080* const cpu)
{
	unsigned char* opcode = &cpu->memory[cpu->programCounter];

	switch(*opcode)
	{
		case 0x00:
			break;
	}

	cpu->programCounter++;
}

// HELPERS

static void writeToMemory(Intel8080* const cpu, const u16 address, const u8 data)
{
	if (address < 0x2000)
	{
		printf("Cannot write to ROM memory. Address: %x\n", address);
		return;
	}

	if (address < 0x4000)
	{
		printf("Cannot write to RAM memory. Address: %x\n", address);
		return;
	}

	cpu->memory[address] = data;
}

static u16 readRegisteredPair8080(Intel8080* const cpu, const Register r)
{
	u16 result = (cpu->registers[r] << 8) | cpu->registers[r+1];

	return result;
}

static void writeRegisteredPair8080(Intel8080* const cpu, const Register r, const u16 data)
{
	cpu->registers[r] = data >> 8;
	cpu->registers[r+1] = data & 0xFF;
}

static void setConditionFlags_r(Intel8080* const cpu, const Register r, const u16 data)
{
	u16 carryResult = data;
	u8 result = carryResult & 0xFF;

	cpu->conditionFlags[Z] = result == 0;
	cpu->conditionFlags[S] = (result & 0x80) != 0;
	cpu->conditionFlags[P] = checkParity(result);
	cpu->conditionFlags[CY] = carryResult > 0xFF;
	cpu->conditionFlags[AC] = ((cpu->registers[A] ^ cpu->registers[r] ^ result) & 0x10) != 0;
}

static void setConditionFlags_M(Intel8080* const cpu, const u16 data)
{
	u16 carryResult = data;
	u8 result = carryResult & 0xFF;

	cpu->conditionFlags[Z] = result == 0;
	cpu->conditionFlags[S] = (result & 0x80) != 0;
	cpu->conditionFlags[P] = checkParity(result);
	cpu->conditionFlags[CY] = carryResult > 0xFF;
	cpu->conditionFlags[AC] = ((cpu->registers[A] ^ cpu->registers[readRegisteredPair8080(cpu, H)] ^ result) & 0x10) != 0;
}

static void setConditionFlags_data(Intel8080* const cpu, const u8 data)
{
	u16 carryResult = cpu->registers[A] + data;
	u8 result = carryResult & 0xFF;

	cpu->conditionFlags[Z] = result == 0;
	cpu->conditionFlags[S] = (result & 0x80) != 0;
	cpu->conditionFlags[P] = checkParity(result);
	cpu->conditionFlags[CY] = carryResult > 0xFF;
	cpu->conditionFlags[AC] = ((cpu->registers[A] ^ data ^ result) & 0x10) != 0;
}

// INSTRUCTIONS

/* 
Data Transfer Group
*/

static void MOV_r1_r2(Intel8080* const cpu, const Register r1, const Register r2)
{
	cpu->registers[r1] = cpu->registers[r2];
}

static void MOV_r_M(Intel8080* const cpu, const Register r)
{
	cpu->registers[r] = cpu->memory[readRegisteredPair8080(cpu, H)];
}

static void MOV_M_r(Intel8080* const cpu, const Register r)
{
	//cpu->memory[readRegisteredPair8080(cpu, H)] = cpu->registers[r];
	writeToMemory(cpu, readRegisteredPair8080(cpu, H), r);
}

static void MVI_r_data(Intel8080* const cpu, const Register r, const u8 data)
{
	cpu->registers[r] = data;
	cpu->programCounter++;
}

static void MVI_M_data(Intel8080* const cpu, const u8 data)
{
	//cpu->memory[readRegisteredPair8080(cpu, H)] = data;
	writeToMemory(cpu, readRegisteredPair8080(cpu, H), data);

	cpu->programCounter++;
}

static void LXI_rp_data_16(Intel8080* const cpu, const Register r, const u16 data)
{
	writeRegisteredPair8080(cpu, r, data);

	cpu->programCounter += 2;
}

static void LDA_addr(Intel8080* const cpu, const u8 byte3, const u8 byte2)
{
	u16 address = (byte3 << 8) | byte2;

	cpu->registers[A] = cpu->memory[address];
	cpu->programCounter += 2;
}

static void STA_addr(Intel8080* const cpu, const u8 byte3, const u8 byte2)
{
	u16 address = (byte3 << 8) | byte2;

	//cpu->memory[address] = cpu->registers[A];
	writeToMemory(cpu, address, cpu->registers[A]);

	cpu->programCounter += 2;
}

static void LHLD_addr(Intel8080* const cpu, const u8 byte3, const u8 byte2)
{
	u16 address = (byte3 << 8) | byte2;

	cpu->registers[L] = cpu->memory[address];
	cpu->registers[H] = cpu->memory[address+1];
	cpu->programCounter += 2;
}

static void SHLD_addr(Intel8080* const cpu, const u8 byte3, const u8 byte2)
{
	u16 address = (byte3 << 8) | byte2;

	//cpu->memory[address] = cpu->registers[L];
	//cpu->memory[address+1] = cpu->registers[H];
	writeToMemory(cpu, address, cpu->registers[L]);
	writeToMemory(cpu, address+1, cpu->registers[H]);

	cpu->programCounter += 2;
}

static void LDAX_rp(Intel8080* const cpu, const Register r)
{
	if (r != B || D)
	{
		printf("Invalid register pair operation.");
	}

	cpu->registers[A] = cpu->memory[readRegisteredPair8080(cpu, r)];
}

static void STAX_rp(Intel8080* const cpu, const Register r)
{
	if (r != B || D)
	{
		printf("Invalid register pair operation.");
	}
	
	//cpu->memory[readRegisteredPair8080(cpu, r)] = cpu->registers[A];
	writeToMemory(cpu, readRegisteredPair8080(cpu, r), cpu->registers[A]);
}

static void XCHG(Intel8080* const cpu)
{
	u8 tempH = cpu->registers[H];
	u8 tempL = cpu->registers[L];

	cpu->registers[H] = cpu->registers[D];
	cpu->registers[L] = cpu->registers[E];
	cpu->registers[D] = tempH;
	cpu->registers[E] = tempL;
}

/*
Arithmetic Group
*/

static bool checkParity(const u8 byte)
{
	u8 bitCount = 0;
	
	for (int i = 0; i < 8; i++)
	{
		u8 bit = (byte >> i) & 1;
		bitCount += bit;
	}

	return ((bitCount & 1) == 0);
}

static void ADD_r(Intel8080* const cpu, const Register r)
{
	u16 carryResult = cpu->registers[A] + cpu->registers[r];
	u8 result = carryResult & 0xFF;

	cpu->registers[A] = result;

	// cpu->conditionFlags[Z] = result == 0;
	// cpu->conditionFlags[S] = (result & 0x80) != 0;
	// cpu->conditionFlags[P] = checkParity(result);
	// cpu->conditionFlags[CY] = carryResult > 0xFF;
	// cpu->conditionFlags[AC] = ((cpu->registers[A] ^ cpu->registers[r] ^ result) & 0x10) != 0;
	setConditionFlags_r(cpu, r, carryResult);
}

static void ADD_M(Intel8080* const cpu)
{
	u16 carryResult = cpu->registers[A] + cpu->memory[readRegisteredPair8080(cpu, H)];
	u8 result = carryResult & 0xFF;

	cpu->registers[A] = result;

	// cpu->conditionFlags[Z] = result == 0;
	// cpu->conditionFlags[S] = (result & 0x80) != 0;
	// cpu->conditionFlags[P] = checkParity(result);
	// cpu->conditionFlags[CY] = carryResult > 0xFF;
	// cpu->conditionFlags[AC] = ((cpu->registers[A] ^ cpu->memory[readRegisteredPair8080(cpu, H)] ^ result) & 0x10) != 0;
	setConditionFlags_M(cpu, carryResult);
}

static void ADI_data(Intel8080* const cpu, const u8 data)
{
	u16 carryResult = cpu->registers[A] + data;
	u8 result = carryResult & 0xFF;

	cpu->registers[A] = result;

	// cpu->conditionFlags[Z] = result == 0;
	// cpu->conditionFlags[S] = (result & 0x80) != 0;
	// cpu->conditionFlags[P] = checkParity(result);
	// cpu->conditionFlags[CY] = carryResult > 0xFF;
	// cpu->conditionFlags[AC] = ((cpu->registers[A] ^ data ^ result) & 0x10) != 0;
	setConditionFlags_data(cpu, data);
}

static void ADC_r(Intel8080* const cpu, const Register r)
{
	u16 carryResult = cpu->registers[A] + (cpu->registers[r] + cpu->conditionFlags[CY]);
	u8 result = carryResult & 0xFF;

	cpu->registers[A] = result;

	setConditionFlags_r(cpu, r, carryResult);

	cpu->conditionFlags[AC] = ((cpu->registers[A] ^ cpu->registers[r] ^ cpu->conditionFlags[CY] ^ result) & 0x10) != 0;
}

static void ADC_M(Intel8080* const cpu)
{
	u16 carryResult = cpu->registers[A] + (cpu->memory[readRegisteredPair8080(cpu, H)] + cpu->conditionFlags[CY]);
	u8 result = carryResult & 0xFF;

	cpu->registers[A] = result;

	setConditionFlags_M(cpu, carryResult);

	cpu->conditionFlags[AC] = ((cpu->registers[A] ^ cpu->memory[readRegisteredPair8080(cpu, H)] ^ cpu->conditionFlags[CY] ^ result) & 0x10) != 0;
}

static void ACI_data(Intel8080* const cpu, const u8 data)
{
	u16 carryResult = cpu->registers[A] + data + cpu->conditionFlags[CY];
	u8 result = carryResult & 0xFF;

	cpu->registers[A] = result;

	setConditionFlags_data(cpu, data);
	
	cpu->conditionFlags[AC] = ((cpu->registers[A] ^ data ^ cpu->conditionFlags[CY] ^ result) & 0x10) != 0;
}