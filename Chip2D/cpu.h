#ifndef CPU_H
#define CPU_H

#pragma once

#include <Windows.h>

#include "Chip2D.h"
#include "gpu.h"
#include "memory.h"

#define ROM_START 0x200

typedef unsigned char BYTE;
typedef unsigned short WORD;

class cpu
{
public:
	cpu();
	~cpu();
	
	void initalize(HWND hwnd, int tempScale);
	void emulate();
	void runOpcode();
	bool openRom(LPWSTR);
	void clearScreen();

	// helper functions to talk to the memory class
	BYTE readMem(int location);
	BYTE readReg(int reg);
	WORD readStack(int position);
	WORD readI();
	WORD readPC();
	BYTE readSoundTimer();
	BYTE readDelayTimer();
	WORD readOpCode();
	DWORD getRomSize();
	void render();

private:

	WORD	I, pc;							// Instruction register and program counter
	BYTE	soundTimer, delayTimer;			// sound and delay timers
	WORD	opCode, vTemp;					// currrent opcode and temp register
	int		stackPointer;					// stack pointer
	BYTE	screen[64 * 32];				// screen array

	gpu		c8GPU;
	memory	c8MEM;

	int		scale;

	// helper function to get key presses
	int getKey();
};

#endif