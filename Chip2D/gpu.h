#ifndef GPU_H
#define GPU_H

#pragma once

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

// taken from Microsoft's Direct2D Demo App
template<class Interface>
inline void SafeRelease(Interface **ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = NULL;
    }
}

class gpu
{
public:
	gpu();
	~gpu();
	void initalize(HWND hwnd);
	void initalizeScreen();
	void render(unsigned char *screen, int scale);
	void clearScreen();

private:
	ID2D1Factory*			m_pDirect2dFactory;
    ID2D1HwndRenderTarget*	m_pRenderTarget;
    ID2D1SolidColorBrush*	m_pWhiteBrush;
	ID2D1SolidColorBrush*	m_pBlackBrush;
	unsigned char*			screen;
};

#endif