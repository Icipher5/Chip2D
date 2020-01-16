#include "cpu.h"

// class constructor
cpu::cpu()
{

}

// class deconstructor
cpu::~cpu()
{

}

// initalizes the varablies of the class
void cpu::initalize(HWND hwnd, int tempScale)
{
	I				= 0;
	pc				= ROM_START;
	soundTimer		= 0;
	delayTimer		= 0;
	opCode			= 0;
	vTemp			= 0;
	stackPointer	= 0;
	screen[64 * 32]	= 0;

	scale = tempScale;

	c8GPU.initalize(hwnd);
	c8MEM.initalize();

	clearScreen();
}

void cpu::emulate()
{
	if(delayTimer > 0)
	{
		delayTimer--;
	}
    
	if(soundTimer > 0)
	{
		PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC | SND_NOSTOP);
		soundTimer--;
	}

	opCode = ((c8MEM.readMem(pc) << 8) + c8MEM.readMem(pc + 1));
	pc += 2;
	runOpcode();
}

// executes one cpu cycle
void cpu::runOpcode()
{
	switch (opCode & 0xF000)
	{
	case 0x0000:
		{
			switch (opCode & 0x00FF)
			{
			case 0xE0:							// 0x00E0 - clear the screen
				{
					clearScreen();
					c8GPU.render(screen, scale);
					break;
				}
			case 0xEE:							// 0x00EE - return from a Chip-8 sub-routine
				{
					stackPointer--;
					pc = c8MEM.readStack(stackPointer);
					break;
				}
			default:
				break;
			}
		break;
		}
	case 0x1000:							// 0x1NNN - Jump to NNN
		{
			WORD nnn = (opCode & 0x0FFF);
			pc = nnn;
			break;
		}
	case 0x2000:							// 0x2NNN - Call CHIP-8 sub-routine at NNN (16 successive calls max)
		{
			WORD nnn = (opCode & 0x0FFF);
			c8MEM.writeStack(stackPointer, pc);
			stackPointer++;
			pc = nnn;
			break;
		}
	case 0x3000:							// 0x3XKK - Skip next instruction if VX == KK
		{
			WORD x = opCode & 0x0F00;
			x >>= 8;
			WORD kk = opCode & 0x00FF;

			if (c8MEM.readReg(x) == kk)
			{
				pc += 2;
			}
			break;
		}
	case 0x4000:							// 0x4XKK - Skip next instruction if VX != KK
		{
			WORD x = opCode & 0x0F00;
			x >>= 8;
			WORD kk = opCode & 0x00FF;

			if (c8MEM.readReg(x) != kk)
			{
				pc += 2;
			}

			break;
		}
	case 0x5000:							// 0x5XY0 - Skip next instruction if VX == VY
		{
			WORD x = opCode & 0x0F00;
			x >>= 8;
			WORD y = opCode & 0x00F0;
			y >>= 4;

			if (c8MEM.readReg(x) == c8MEM.readReg(y))
			{
				pc += 2;
			}
			break;
		}
	case 0x6000:							// 0x6XKK - VX = KK
		{
			WORD x = opCode & 0x0F00;
			x >>= 8;
			BYTE kk = opCode & 0x00FF;

			c8MEM.writeReg(x, kk);
			break;
		}
	case 0x7000:							// 0x7XKK - VX = VX + KK
		{
			WORD x = opCode & 0x0F00;
			x >>= 8;
			WORD kk = opCode & 0x00FF;

			c8MEM.writeReg(x, c8MEM.readReg(x) + kk);
			break;
		}
	case 0x8000:
		switch (opCode & 0xF)
		{
		case 0x0:							// 0x8XY0 - VX = VY
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;
				WORD y = opCode & 0x00F0;
				y >>= 4;

				c8MEM.writeReg(x, c8MEM.readReg(y));
				break;
			}
		case 0x1:							// 0x8XY1 - VX = VX OR VY
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;
				WORD y = opCode & 0x00F0;
				y >>= 4;

				c8MEM.writeReg(x, c8MEM.readReg(x) | c8MEM.readReg(y));
				break;
			}
		case 0x2:							// 0x8XY2 - VX = VX AND VY
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;
				WORD y = opCode & 0x00F0;
				y >>= 4;

				c8MEM.writeReg(x, c8MEM.readReg(x) & c8MEM.readReg(y));
				break;
			}
		case 0x3:							// 0x8XY3 - VX = VX XOR VY
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;
				WORD y = opCode & 0x00F0;
				y >>= 4;

				c8MEM.writeReg(x, c8MEM.readReg(x) ^ c8MEM.readReg(y));
				break;
			}
		case 0x4:							// 0x8XY4 - VX = VX + VY, VF = carry
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;
				WORD y = opCode & 0x00F0;
				y >>= 4;

				vTemp = c8MEM.readReg(x);
				c8MEM.writeReg(x, c8MEM.readReg(x) + c8MEM.readReg(y));
				c8MEM.writeReg(0xF,	vTemp > (c8MEM.readReg(x)));
				break;
			}
		case 0x5:							// 0x8XY5 - VX = VX - VY, VF = not borrow							(**)
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;
				WORD y = opCode & 0x00F0;
				y >>= 4;

				c8MEM.writeReg(0xF,	c8MEM.readReg(x) >= c8MEM.readReg(y));
				c8MEM.writeReg(x, c8MEM.readReg(x) - c8MEM.readReg(y));
				break;
			}
		case 0x6:							// 0x8XY6 - VX = VX SHR 1 (VX=VX/2), VF = carry
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;
				WORD y = opCode & 0x00F0;
				y >>= 4;

				c8MEM.writeReg(0xF, c8MEM.readReg(x) & 0x1);
				c8MEM.writeReg(x, c8MEM.readReg(x) >> 1);
				break;
			}
		case 0x7:							// 0x8XY7 - VX = VY - VX, VF = not borrow							(**)
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;
				WORD y = opCode & 0x00F0;
				y >>= 4;

				c8MEM.writeReg(0xF, c8MEM.readReg(y) >= c8MEM.readReg(x));
				c8MEM.writeReg(x, c8MEM.readReg(y) - c8MEM.readReg(x));
				break;
			}
		case 0xE:							// 0x8XYE - VX = VX SHL 1 (VX=VX*2), VF = MSB before shift
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;
				WORD y = opCode & 0x00F0;
				y >>= 4;

				c8MEM.writeReg(0xF, c8MEM.readReg(x) >> 0x7);
				c8MEM.writeReg(x, c8MEM.readReg(x) << 1);
				break;
			}
		default:							// Shouldn't be here, error reading opcode!!!!
			break;
		}
		break;
// (**): When you do VX - VY, VF is set to the negation of the borrow.
// This means that if VX is superior or equal to VY, VF will be set to 01, as the borrow is 0.
// If VX is inferior to VY, VF is set to 00, as the borrow is 1.
	case 0x9000:							// 0x9XY0 - Skip next instruction if VX != VY
		{
			WORD x = opCode & 0x0F00;
			x >>= 8;
			WORD y = opCode & 0x00F0;
			y >>= 4;

			if (c8MEM.readReg(x) != c8MEM.readReg(y))
			{
				pc += 2;
			}
			break;
		}
	case 0xA000:							// 0xANNN - I = NNN
		{
			WORD nnn = opCode & 0x0FFF;

			I = nnn;
			break;
		}
	case 0xB000:							// 0xBNNN - Jump to NNN + V0
		{
			WORD nnn = opCode & 0x0FFF;

			pc = nnn + c8MEM.readReg(0);
			break;
		}
	case 0xC000:							// 0xCXKK - VX = Random number AND KK
		{
			WORD x = opCode & 0x0F00;
			x >>= 8;
			BYTE kk = opCode & 0x00FF;

			c8MEM.writeReg(x, rand() & kk);
			break;
		}
	case 0xD000:							// 0xDXYN - Draws an 8 x N sprite at (VX,VY) starting at M(I). VF = collision.
		{
			WORD x = opCode & 0x0F00;
			x >>= 8;
			WORD y = opCode & 0x00F0;
			y >>= 4;
			BYTE n = opCode & 0x000F;
			unsigned char data;

			c8MEM.writeReg(0xF, 0);
			for (int yline = 0; yline < n; yline++)
			{
				data = c8MEM.readMem(I + yline);
				for (int xpix = 0; xpix < 8; xpix++)
				{
					if ((data & (0x80 >> xpix)) != 0)
					{
						if (screen[(c8MEM.readReg(x) + xpix) + ((c8MEM.readReg(y) + yline) * 64)] == 0x1)
						{
							c8MEM.writeReg(0xF, 1);
						}
						screen[(c8MEM.readReg(x) + xpix) + ((c8MEM.readReg(y) + yline) * 64)] ^= 0x1;
					}
				}
			}

			c8GPU.render(screen, scale);
			break;
		}
	case 0xE000:
		switch (opCode & 0xFF)
		{
		case 0x9E:							// 0xEX9E - Skip next instruction if key VX pressed
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				if (c8MEM.readReg(x) == getKey())
				{
					pc += 2;
				}
				break;
			}
		case 0xA1:							// 0xEXA1 - Skip next instruction if key VX not pressed
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				if (c8MEM.readReg(x) != getKey())
				{
					pc += 2;
				}
				break;
			}
		default:							// Shouldn't be here, error reading opcode!!!!
			break;
		}
		break;
	case 0xF000:
		switch (opCode & 0xFF)
		{
		case 0x07:							// 0xFX07 - VX = Delay timer
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				c8MEM.writeReg(x, delayTimer);
				break;
			}
		case 0x0A:							// 0xFX0A - Waits a keypress and stores it in VX
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				int key = getKey();
				if (key != -1)
				{
					c8MEM.writeReg(x, key);
				}
				else
				{
					pc -= 2;
				}
				break;
			}
		case 0x15:							// 0xFX15 - Delay timer = VX
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				delayTimer = c8MEM.readReg(x);
				break;
			}
		case 0x18:							// 0xFX18 - Sound timer = VX
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				soundTimer = c8MEM.readReg(x);
				break;
			}
		case 0x1E:							// 0xFX1E - I = I + VX
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				I += c8MEM.readReg(x);
				break;
			}
		case 0x29:							// 0xFX29 - I points to the 4 x 5 font sprite of hex char in VX
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				I = c8MEM.readReg(x) * 5;
				break;
			}
		case 0x33:							// 0xFX33 - Store BCD representation of VX in M(I)...M(I+2)
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				int hundreds	= c8MEM.readReg(x) / 100;
				int tens		= (c8MEM.readReg(x) % 100) / 10;
				int units		= c8MEM.readReg(x) % 10;

				c8MEM.writeMem(I, hundreds);
				c8MEM.writeMem(I + 1, tens);
				c8MEM.writeMem(I + 2, units);
				break;
			}
		case 0x55:							// 0xFX55 - Save V0...VX in memory starting at M(I)
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				for (int i = 0; i <= x; i++)
				{
					c8MEM.writeMem(I + i, c8MEM.readReg(i));
				}
				//I = I + ((opCode & 0x0F00) >> 8) + 1;
				break;
			}
		case 0x65:							// 0xFX65 - Load V0...VX from memory starting at M(I)
			{
				WORD x = opCode & 0x0F00;
				x >>= 8;

				for (int i = 0; i <= x; i++)
				{
					c8MEM.writeReg(i, c8MEM.readMem(I + i));
				}
				//I = I + ((opCode & 0x0F00) >> 8) + 1;
				break;
			}
		default:							// Shouldn't be here, error reading opcode!!!!
			break;
		}
		break;
	default:								// Shouldn't be here, error reading opcode!!!!
		break;
	}
}

// opens the rom and loads it into the memory
bool cpu::openRom(LPWSTR cpFile)
{
	return c8MEM.openRom(cpFile);
}

// clears the screen
void cpu::clearScreen()
{
	for (int i = 0; i < 2048; i++)
	{
		screen[i] = 0;
	}
}

BYTE cpu::readMem(int location)
{
	return c8MEM.readMem(location);
}

BYTE cpu::readReg(int reg)
{
	return c8MEM.readReg(reg);
}

WORD cpu::readStack(int position)
{
	return c8MEM.readStack(position);
}

WORD cpu::readI()
{
	return I;
}

WORD cpu::readPC()
{
	return pc;
}

BYTE cpu::readSoundTimer()
{
	return soundTimer;
}

BYTE cpu::readDelayTimer()
{
	return delayTimer;
}

WORD cpu::readOpCode()
{
	WORD opcode = ((c8MEM.readMem(pc) << 8) + c8MEM.readMem(pc + 1));
	return opcode;
}

DWORD cpu::getRomSize()
{
	return c8MEM.getRomSize();
}

int cpu::getKey()
{
	if(bKey[VK_DECIMAL])	return 0x0;
    if(bKey[VK_NUMPAD7])	return 0x1;
    if(bKey[VK_NUMPAD8])	return 0x2;
    if(bKey[VK_NUMPAD9])	return 0x3;
    if(bKey[VK_NUMPAD4])	return 0x4;
    if(bKey[VK_NUMPAD5])	return 0x5;
    if(bKey[VK_NUMPAD6])	return 0x6;
    if(bKey[VK_NUMPAD1])	return 0x7;
    if(bKey[VK_NUMPAD2])	return 0x8;
    if(bKey[VK_NUMPAD3])	return 0x9;
    if(bKey[VK_NUMPAD0])	return 0xA;
    if(bKey[VK_RETURN])		return 0xB;
    if(bKey[VK_DIVIDE])		return 0xC;
    if(bKey[VK_MULTIPLY])	return 0xD;
    if(bKey[VK_SUBTRACT])	return 0xE;
    if(bKey[VK_ADD])		return 0xF;
    
    return -1; 
}

void cpu::render()
{
	c8GPU.render(screen, scale);
}