#ifndef CHIP2D_H
#define CHIP2D_H

#pragma once

#ifndef UNICODE
#define UNICODE
#endif

// Exclude rarely-used items from Windows headers.
#define WIN32_LEAN_AND_MEAN

// Windows Header Files:
#include <windows.h>
#include <WindowsX.h>
#include <Commdlg.h>
#include <commctrl.h>

// C RunTime Header Files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

// Direct2D header
#include <d2d1.h>

#include "resource.h"
#include "cpu.h"

extern bool bKey[255];

enum { SCREEN_WIDTH = 64, SCREEN_HEIGHT = 32 };

// The windows procedure.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
// The AboutBox procedure
INT_PTR CALLBACK About(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
// The Debugger procedure
INT_PTR CALLBACK Debugger(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
// updates the Debugger window
void updateDebugger();
void fillInstructions(HWND hwnd);
wchar_t* getInstruction(WORD opcode);
void setupMemList(HWND hwnd);

#endif