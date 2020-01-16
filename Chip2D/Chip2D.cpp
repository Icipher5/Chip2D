#include "Chip2D.h"

HINSTANCE	hInst;			// copy of the main instance
HWND 		hwndDebugger;	// handle to the debugger
HWND		hIList;			// handle to the List Box of instructions
HWND		hMList;			// handle to the List View of the memory
bool		bActive;		// is the emulator running?
int			scale;			// video scale
cpu			c8CPU;
bool		bKey[255];

// application entry point.
int WINAPI WinMain(HINSTANCE hInstance,
    			   HINSTANCE hPrevInstance,
    			   LPSTR lpCmdLine,
   				   int nCmdShow)
{
	bActive = false;
	scale	= 10;

	HWND hWnd;
	WNDCLASSEX wc;
	hInst = hInstance;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= WndProc;
	wc.hInstance		= hInstance;
    wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName		= MAKEINTRESOURCE(IDR_CHIP2D);
    wc.lpszClassName	= L"WindowClass";

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL,
                          L"WindowClass",
                          L"Chip2D",
                          WS_OVERLAPPEDWINDOW,
						  CW_USEDEFAULT, 0,
                          (SCREEN_WIDTH * scale), (SCREEN_HEIGHT * scale),
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

	ShowWindow(hWnd, nCmdShow);

	// initalize parts
	hwndDebugger = NULL;
	c8CPU.initalize(hWnd, scale);

	MSG msg;

    while(TRUE)
    {
        DWORD starting_point = GetTickCount();

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
		
		if (bActive)
		{
			c8CPU.emulate();
		}
    }

    return msg.wParam;
}

// Handles window messages.
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

    switch (message)
    {
	case WM_COMMAND:
		wmId	= LOWORD(wParam);
		wmEvent	= HIWORD(wParam);

		switch (wmId)
		{
		case ID_FILE_OPEN40003:
			OPENFILENAME ofn;
			char szFile[260];

			ZeroMemory(&ofn, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hwnd;
			ofn.lpstrFile = LPWSTR(szFile);
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			if (GetOpenFileName(&ofn) == TRUE)
			{
				if (!(c8CPU.openRom(ofn.lpstrFile)))
				{
					MessageBox(hwnd, ofn.lpstrFile, L"Unable to open:", 0);
				}
				else
				{
					c8CPU.initalize(hwnd, scale);
					fillInstructions(hwndDebugger);
					bActive = true;
				}
			}
			break;
		case ID_SETTINGS_DEBUGGER:
			if (!IsWindow(hwndDebugger))
			{
				hwndDebugger = CreateDialog(hInst, 
								MAKEINTRESOURCE(IDD_DEBUGGER), hwnd, Debugger);
				ShowWindow(hwndDebugger, SW_SHOW);
				setupMemList(hwndDebugger);
			}
			else
			{
				ShowWindow(hwndDebugger, SW_HIDE);
				DestroyWindow(hwndDebugger);
				hwndDebugger = NULL;
			}
			break;
		case ID_HELP_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
			break;
		case ID_FILE_EXIT:
			DestroyWindow(hwnd);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
		break;
		case WM_PAINT:
			c8CPU.render();
			break;
		case WM_KEYDOWN:
			bKey[wParam] = TRUE;
			break;
        case WM_KEYUP:
			bKey[wParam] = FALSE;
			break;
		case WM_DESTROY:
        {
        	PostQuitMessage(0);
			return 0;
        }
        break;       
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

INT_PTR CALLBACK About(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hwnd, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}

	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Debugger(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		fillInstructions(hwnd);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		wmId	= LOWORD(wParam);
		wmEvent	= HIWORD(wParam);

		switch (wmId)
		{
		case IDOK:
			EndDialog(hwnd, LOWORD(wParam));
			DestroyWindow(hwndDebugger);
			break;
		case IDC_BGO:
			bActive = true;
			break;
		case IDC_BPAUSE:
			bActive = false;
			updateDebugger();
			break;
		case IDC_BSTEP:
			if (bActive == false)
			{
				c8CPU.emulate();
				updateDebugger();
			}
			break;
		default:
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void fillInstructions(HWND hwnd)
{
	WORD pc;
	wchar_t* temp;
	wchar_t instruction[255];
	wchar_t buffer[255];
	hIList = GetDlgItem(hwnd, IDC_INSTRUCTIONS);
	DWORD romSize = c8CPU.getRomSize();

	if (romSize != 0)
	{
		WORD firstOp = c8CPU.readMem(0x200);
		firstOp <<= 8;
		firstOp += c8CPU.readMem(0x200 + 1);
		WORD jmp = firstOp & 0xF000;
		jmp >>= 12;
		WORD nnn = firstOp & 0x0FFF;
		if (jmp == 0x1 && nnn % 2 != 0)
		{
			// insert the first instruction which will be a jump to an odd pc
			pc = 0x200;
			WORD opcode = c8CPU.readMem(0x200);
			opcode <<= 8;
			opcode += c8CPU.readMem(0x200 + 1);
			temp = getInstruction(opcode);
			// convert wchar_t* to wchar_t[] so it can be used in swprintf
			for (int j = 0; j < 255; j++)
			{
				instruction[j] = temp[j];
			}
			swprintf(buffer, 255, L"%.4X     %.4X     %s", pc, opcode, instruction);
			ListBox_InsertString(hIList, 0, buffer);

			for (unsigned int i = 1; i < ((romSize - 1) / 2); i++)
			{
				// sets the pc
				pc = 0x200 + (i * 2) + 1;
				// gets the opcode from memory
				WORD opcode = c8CPU.readMem(0x200 + (i * 2) + 1);
				opcode <<= 8;
				opcode += c8CPU.readMem(0x200 + (i * 2) + 2);
				// gets the english of the instruction
				temp = getInstruction(opcode);
				// convert wchar_t* to wchar_t[] so it can be used in swprintf
				for (int j = 0; j < 255; j++)
				{
					instruction[j] = temp[j];
				}
				// formats the info into a string
				swprintf(buffer, 255, L"%.4X     %.4X     %s", pc, opcode, instruction);
				ListBox_InsertString(hIList, i, buffer);
			}

		}
		else
		{
			for (unsigned int i = 0; i < romSize / 2; i++)
			{
				// sets the pc
				pc = 0x200 + (i * 2);
				// gets the opcode from memory
				WORD opcode = c8CPU.readMem(0x200 + (i * 2));
				opcode <<= 8;
				opcode += c8CPU.readMem(0x200 + (i * 2) + 1);
				// gets the english of the instruction
				temp = getInstruction(opcode);
				// convert wchar_t* to wchar_t[] so it can be used in swprintf
				for (int j = 0; j < 255; j++)
				{
					instruction[j] = temp[j];
				}
				// formats the info into a string
				swprintf(buffer, 255, L"%.4X     %.4X     %s", pc, opcode, instruction);
				ListBox_InsertString(hIList, i, buffer);
			}
		}
	}
}

void updateDebugger()
{
	wchar_t buffer[255];

	// Shows register information
	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x0));
	SetDlgItemText(hwndDebugger, IDC_V0, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x1));
	SetDlgItemText(hwndDebugger, IDC_V1, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x2));
	SetDlgItemText(hwndDebugger, IDC_V2, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x3));
	SetDlgItemText(hwndDebugger, IDC_V3, buffer);
	
	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x4));
	SetDlgItemText(hwndDebugger, IDC_V4, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x5));
	SetDlgItemText(hwndDebugger, IDC_V5, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x6));
	SetDlgItemText(hwndDebugger, IDC_V6, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x7));
	SetDlgItemText(hwndDebugger, IDC_V7, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x8));
	SetDlgItemText(hwndDebugger, IDC_V8, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0x9));
	SetDlgItemText(hwndDebugger, IDC_V9, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0xA));
	SetDlgItemText(hwndDebugger, IDC_VA, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0xB));
	SetDlgItemText(hwndDebugger, IDC_VB, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0xC));
	SetDlgItemText(hwndDebugger, IDC_VC, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0xD));
	SetDlgItemText(hwndDebugger, IDC_VD, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0xE));
	SetDlgItemText(hwndDebugger, IDC_VE, buffer);

	swprintf(buffer, 255, L"%X", c8CPU.readReg(0xF));
	SetDlgItemText(hwndDebugger, IDC_VF, buffer);

	// Shows opcode
	swprintf(buffer, 255, L"%X", c8CPU.readOpCode());
	SetDlgItemText(hwndDebugger, IDC_OPCODE, buffer);

	// Shows program counter
	swprintf(buffer, 255, L"%X", c8CPU.readPC());
	SetDlgItemText(hwndDebugger, IDC_PC, buffer);

	// Shows I
	swprintf(buffer, 255, L"%X", c8CPU.readI());
	SetDlgItemText(hwndDebugger, IDC_I, buffer);

	// Shows delay timer
	swprintf(buffer, 255, L"%X", c8CPU.readDelayTimer());
	SetDlgItemText(hwndDebugger, IDC_DELAY, buffer);

	// Shows sound timer
	swprintf(buffer, 255, L"%X", c8CPU.readSoundTimer());
	SetDlgItemText(hwndDebugger, IDC_SOUND, buffer);

	// selects the current instructionin the list box
	ListBox_SetCurSel(hIList, (c8CPU.readPC() - 0x200) / 2);

	// updates the memory
	wchar_t mem[3] = L"";
	LVITEM lvi;
	memset(&lvi, 0, sizeof(lvi));
	lvi.mask = LVIF_TEXT;
	lvi.state = 0;
	lvi.stateMask = 0;
	for (int row = 0; row < 256; row++)
	{
		for (int col = 0; col < 16; col++)
		{
			swprintf(mem, 7, L"%X", c8CPU.readMem(row * col));
			lvi.pszText = mem;
			lvi.iItem = row;					//row
			lvi.iSubItem = col + 1;				//column
			ListView_SetItem(hMList, &lvi);
		}
	}

	// brings the value of I into view
	// can't get it to select a row
	SendMessage(hMList, LVM_ENSUREVISIBLE, (WPARAM)c8CPU.readI() / 16, FALSE);
}

wchar_t* getInstruction(WORD opcode)
{
	wchar_t buffer[255];

	switch (opcode & 0xF000)
	{
	case 0x0000:
	{
		switch (opcode & 0x00FF)
		{
		case 0xE0:
			{
				// 0x00E0 - clear the screen
				swprintf(buffer, 255, L"%s", L"Clear the screen");
				break;
			}
		case 0xEE:
			{
				// 0x00EE - return from a Chip-8 sub-routine
				swprintf(buffer, 255, L"%s", L"Return from a subroutine");
				break;
			}
		default:
			{
				swprintf(buffer, 255, L"Not a valid instruction");
				break;
			}
		}
	break;
	}
	case 0x1000:
		{
			// 0x1NNN - Jump to NNN
			swprintf(buffer, 255, L"%s%X", L"Jump to ",
					(opcode & 0x0FFF));
			break;
		}
	case 0x2000:
		{
			// 0x2NNN - Call CHIP-8 sub-routine at NNN (16 successive calls max)
			swprintf(buffer, 255, L"%s%X", L"Call subroutine at ",
					(opcode & 0x0FFF));
			break;
		}
	case 0x3000:
		{
			// 0x3XKK - Skip next instruction if VX == KK
			swprintf(buffer, 255, L"Skip next instruction if V%X = %X",
					(opcode & 0x0F00) >> 8,
					(opcode & 0x00FF));
			break;
		}
	case 0x4000:
		{
			// 0x4XKK - Skip next instruction if VX != KK
			swprintf(buffer, 255, L"Skip next instruction if V%X != %X",
					(opcode & 0x0F00) >> 8,
					(opcode & 0x00FF));
			break;
		}
	case 0x5000:
		{
			// 0x5XY0 - Skip next instruction if VX == VY
			swprintf(buffer, 255, L"Skip next instruction if V%X = V%X",
					(opcode & 0x0F00) >> 8,
					(opcode & 0x00F0) >> 4);
			break;
		}
	case 0x6000:
		{
			// 0x6XKK - VX = KK
			swprintf(buffer, 255, L"V%X = %X",
					(opcode & 0x0F00) >> 8,
					(opcode & 0x00FF));
			break;
		}
	case 0x7000:
		{
			// 0x7XKK - VX = VX + KK
			swprintf(buffer, 255, L"V%X = V%X + %X",
					(opcode & 0x0F00) >> 8,
					(opcode & 0x0F00) >> 8,
					(opcode & 0x00FF));
			break;
		}
	case 0x8000:
		switch (opcode & 0xF)
		{
		case 0x0:
			{
				// 0x8XY0 - VX = VY
				swprintf(buffer, 255, L"V%X = V%X",
						(opcode & 0x0F00) >> 8,
						(opcode & 0x00F0) >> 4);
				break;
			}
		case 0x1:
			{
				// 0x8XY1 - VX = VX OR VY
				swprintf(buffer, 255, L"V%X = V%X or V%X",
						(opcode & 0x0F00) >> 8,
						(opcode & 0x0F00) >> 8,
						(opcode & 0x00F0) >> 4);
				break;
			}
		case 0x2:
			{
				// 0x8XY2 - VX = VX AND VY
				swprintf(buffer, 255, L"V%X = V%X and V%X",
						(opcode & 0x0F00) >> 8,
						(opcode & 0x0F00) >> 8,
						(opcode & 0x00F0) >> 4);
				break;
			}
		case 0x3:
			{
				// 0x8XY3 - VX = VX XOR VY
				swprintf(buffer, 255, L"V%X = V%X xor V%X",
						(opcode & 0x0F00) >> 8,
						(opcode & 0x0F00) >> 8,
						(opcode & 0x00F0) >> 4);
				break;
			}
		case 0x4:
			{
				// 0x8XY4 - VX = VX + VY, VF = carry
				swprintf(buffer, 255, L"V%X = V%X + V%X",
						(opcode & 0x0F00) >> 8,
						(opcode & 0x0F00) >> 8,
						(opcode & 0x00F0) >> 4);
				break;
			}
		case 0x5:
			{
				// 0x8XY5 - VX = VX - VY, VF = not borrow
				swprintf(buffer, 255, L"V%X = V%X - V%X",
						(opcode & 0x0F00) >> 8,
						(opcode & 0x0F00) >> 8,
						(opcode & 0x00F0) >> 4);
				break;
			}
		case 0x6:
			{
				// 0x8XY6 - VX = VX SHR 1 (VX=VX/2), VF = carry
				swprintf(buffer, 255, L"V%X = V%X shift right 1",
						(opcode & 0x0F00) >> 8,
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0x7:
			{
				// 0x8XY7 - VX = VY - VX, VF = not borrow
				swprintf(buffer, 255, L"V%X = V%X - V%X",
						(opcode & 0x0F00) >> 8,
						(opcode & 0x00F0) >> 4,
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0xE:
			{
				// 0x8XYE - VX = VX SHL 1 (VX=VX*2), VF = carry
				swprintf(buffer, 255, L"V%X = V%X shifts left 1",
						(opcode & 0x0F00) >> 8,
						(opcode & 0x0F00) >> 8);
				break;
			}
		default:
			// Shouldn't be here, error reading opcode!!!!
			swprintf(buffer, 255, L"Not a valid instruction");
			break;
		}
		break;
	case 0x9000:
		{
			// 0x9XY0 - Skip next instruction if VX != VY
			swprintf(buffer, 255, L"Skips next  instruction if V%X != V%X",
					(opcode & 0x0F00) >> 8,
					(opcode & 0x00F0) >> 4);
			break;
		}
	case 0xA000:
		{
			// 0xANNN - I = NNN
			swprintf(buffer, 255, L"I = %X",
					(opcode & 0x0FFF));
			break;
		}
	case 0xB000:
		{
			// 0xBNNN - Jump to NNN + V0
			swprintf(buffer, 255, L"Jump to %X + V0",
					(opcode & 0x0FFF));
			break;
		}
	case 0xC000:
		{
			// 0xCXKK - VX = Random number AND KK
			swprintf(buffer, 255, L"V%X = Random # and %X",
					(opcode & 0x0F00) >> 8,
					(opcode & 0x00FF));
			break;
		}
	case 0xD000:
		{
			// 0xDXYN - Draws a 8 x N sprite at (VX,VY) starting at M(I). VF = collision.
			swprintf(buffer, 255, L"Draw a 8x%X sprite at (V%X, V%X)",
					(opcode & 0x000F),
					(opcode & 0x0F00) >> 8,
					(opcode & 0x00F0) >> 4);
			break;
		}
	case 0xE000:
		switch (opcode & 0xFF)
		{
		case 0x9E:
			{
				// 0xEX9E - Skip next instruction if key VX pressed
				swprintf(buffer, 255, L"Skip next instruction if key V%X is pressed",
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0xA1:
			{
				// 0xEXA1 - Skip next instruction if key VX not pressed
				swprintf(buffer, 255, L"Skip next instruction if key V%X is not pressed",
						(opcode & 0x0F00) >> 8);
				break;
			}
		default:
			// Shouldn't be here, error reading opcode!!!!
			swprintf(buffer, 255, L"Not a valid instruction");
			break;
		}
		break;
	case 0xF000:
		switch (opcode & 0xFF)
		{
		case 0x07:
			{
				// 0xFX07 - VX = Delay timer
				swprintf(buffer, 255, L"V%X = Delay timer",
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0x0A:
			{
				// 0xFX0A - Waits a keypress and stores it in VX
				swprintf(buffer, 255, L"Waits for a keypress and store it in V%X",
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0x15:
			{
				// 0xFX15 - Delay timer = VX
				swprintf(buffer, 255, L"Delay timer = V%X",
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0x18:
			{
				// 0xFX18 - Sound timer = VX
				swprintf(buffer, 255, L"Sound timer = V%X",
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0x1E:
			{
				// 0xFX1E - I = I + VX
				swprintf(buffer, 255, L"I = I + V%X",
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0x29:
			{
				// 0xFX29 - I points to the 4 x 5 font sprite of hex char in VX
				swprintf(buffer, 255, L"Sets I to the location of the sprite for the character in V%X",
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0x33:
			{
				// 0xFX33 - Store BCD representation of VX in M(I)...M(I+2)
				swprintf(buffer, 255, L"Store BCD representation of V%X",
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0x55:
			{
				// 0xFX55 - Save V0...VX in memory starting at M(I)
				swprintf(buffer, 255, L"Store V0 to V%X in memory at I",
						(opcode & 0x0F00) >> 8);
				break;
			}
		case 0x65:
			{
				// 0xFX65 - Load V0...VX from memory starting at M(I)
				swprintf(buffer, 255, L"Load V0 to V%X from memory at I",
						(opcode & 0x0F00) >> 8);
				break;
			}
		default:
			// Shouldn't be here, error reading opcode!!!!
			swprintf(buffer, 255, L"Not a valid instruction");
			break;
		}
		break;
	default:
		// Shouldn't be here, error reading opcode!!!!
		swprintf(buffer, 255, L"Not a valid instruction");
		break;
	}

	return buffer;
}

void setupMemList(HWND hwnd)
{
	hMList = GetDlgItem(hwnd, IDC_MEMLIST);
	int address = 0;
	wchar_t add[7] = L"";
	wchar_t col[3] = L"";

	// adds the first column
	LVCOLUMN lvcol;
	memset(&lvcol, 0, sizeof(lvcol));
	lvcol.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
	lvcol.fmt = LVCFMT_CENTER;
	lvcol.pszText = L"";
	lvcol.iSubItem = 0;
	lvcol.cx = 40;
	ListView_InsertColumn(hMList, 0, &lvcol);

	// adds the rest of the columns
	for (int i = 0; i < 16; i++)
	{
		swprintf(col, 3, L"%X", i);
		lvcol.pszText = col;
		lvcol.iSubItem = i + 1;
		lvcol.cx = 34;
		ListView_InsertColumn(hMList, i + 1, &lvcol);
	}

	// adds the hex address in the first column
	LVITEM lvi;
	memset(&lvi, 0, sizeof(lvi));
	lvi.mask = LVIF_TEXT;
	lvi.state = 0;
	lvi.stateMask = 0;
	for (int i = 0; i < 256; i++)
	{
		swprintf(add, 7, L"0x%.3X", address);
		lvi.pszText = add;
		lvi.iItem = i;				//row
		lvi.iSubItem = 0;			//column
		ListView_InsertItem(hMList, &lvi);
		address += 16;
	}
}