#pragma once

typedef unsigned char u8;
typedef unsigned short u16;

#if __STDC_VERSION__ < 202311L
typedef _Bool bool;
#endif

// Condition Flags
typedef enum ConditionFlag
{
	// Zero, Sign, Parity, Carry, and Auxiliary Carry
	Z, S, P, CY, AC,
} ConditionFlag;

// Registers
typedef enum Register
{
	// Valid Register Pairs: B (BC), D (DE), H (HL)
	A, B, C, D, E, H, L,
} Register;

typedef struct Intel8080
{
	u8 registers[sizeof(Register)];
	bool conditionFlags[sizeof(ConditionFlag)];

	u16 stackPointer;
	u16 programCounter;
	u8* memory;
} Intel8080;

void initialize8080(Intel8080* const cpu);

void execute8080Opcode(Intel8080* const cpu);

// HELPERS

static void writeToMemory(Intel8080* const cpu, const u16 address, const u8 data);
static u16 readRegisteredPair8080(Intel8080* const cpu, const Register r);
static void writeRegisteredPair8080(Intel8080* const cpu, const Register r, const u16 data);
static void setConditionFlags_r(Intel8080* const cpu, const Register r, const u16 data);
static void setConditionFlags_M(Intel8080* const cpu, const u16 data);
static void setConditionFlags_data(Intel8080* const cpu, const u8 data);

// INSTRUCTIONS

/* 
Data Transfer Group
*/

static void MOV_r1_r2(Intel8080* const cpu, const Register r1, const Register r2);
static void MOV_r_M(Intel8080* const cpu, const Register r);
static void MOV_M_r(Intel8080* const cpu, const Register r);
static void MVI_r_data(Intel8080* const cpu, const Register r, const u8 data);
static void MVI_M_data(Intel8080* const cpu, const u8 data);
static void LXI_rp_data_16(Intel8080* const cpu, const Register r, const u16 data);
static void LDA_addr(Intel8080* const cpu, const u8 byte3, const u8 byte2);
static void STA_addr(Intel8080* const cpu, const u8 byte3, const u8 byte2);
static void LHLD_addr(Intel8080* const cpu, const u8 byte3, const u8 byte2);
static void SHLD_addr(Intel8080* const cpu, const u8 byte3, const u8 byte2);
static void LDAX_rp(Intel8080* const cpu, const Register r);
static void STAX_rp(Intel8080* const cpu, const Register r);
static void XCHG(Intel8080* const cpu);

/*
Arithmetic Group
*/

static bool checkParity(const u8 byte);
static void ADD_r(Intel8080* const cpu, const Register r);
static void ADD_M(Intel8080* const cpu);
static void ADI_data(Intel8080* const cpu, const u8 data);
static void ADC_r(Intel8080* const cpu, const Register r);
static void ADC_M(Intel8080* const cpu);
static void ACI_data(Intel8080* const cpu, const u8 data);