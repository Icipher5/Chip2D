#ifndef MEMORY_H
#define MEMORY_H

#pragma once

#include <Windows.h>
#include <stdio.h>

#define ROM_START 0x200

class memory
{
public:
	memory();
	~memory();

	void initalize();
	void writeMem(int location, BYTE data);
	BYTE readMem(int location);
	void writeReg(int reg, BYTE data);
	BYTE readReg(int reg);
	void writeStack(int position, WORD data);
	WORD readStack(int position);
	bool openRom(LPWSTR cpFile);
	DWORD getRomSize();

private:
	FILE	*fp;							// file pointer to the rom
	DWORD	dwRomSize;						// size of the rom
	BYTE	mem[0xFFF];						// memory
	BYTE	v[16];							// registers
	WORD	stack[16];						// stack
};

#endif