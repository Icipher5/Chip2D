#include "gpu.h"

// class constructor
gpu::gpu() :
	m_pDirect2dFactory(NULL),
    m_pRenderTarget(NULL),
    m_pWhiteBrush(NULL),
	m_pBlackBrush(NULL)
{

}

// class deconstructor
gpu::~gpu()
{
	SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pWhiteBrush);
    SafeRelease(&m_pBlackBrush);
}

void gpu::initalize(HWND hwnd)
{
	// intializes D2D stuff
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);
	RECT rc;
	GetClientRect(hwnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(640, 320);
	m_pDirect2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hwnd, size),
            &m_pRenderTarget
            );
	m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::White), &m_pWhiteBrush);
	m_pRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);

	// draws the background
	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	m_pRenderTarget->EndDraw();
}

void gpu::render(unsigned char *screen, int scale)
{
	int index = 0;
	int screen_index = 0;

	m_pRenderTarget->BeginDraw();

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			if (screen[screen_index] == 1)
			{
				D2D1_RECT_F rectangle = D2D1::RectF(
           			(float) (x * scale), (float) (y * scale),
            		(float) (x * scale) + scale, (float) (y * scale) + scale
            		);
				m_pRenderTarget->FillRectangle(&rectangle, m_pWhiteBrush);
			}
			else
			{
				D2D1_RECT_F rectangle = D2D1::RectF(
           			(float) (x * scale), (float) (y * scale),
            		(float) (x * scale) + scale, (float) (y * scale) + scale
            		);
				m_pRenderTarget->FillRectangle(&rectangle, m_pBlackBrush);
			}
			screen_index++;
		}
	}

	m_pRenderTarget->EndDraw();
}

void gpu::clearScreen()
{
	// clears the background
	m_pRenderTarget->BeginDraw();
	m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
	m_pRenderTarget->EndDraw();
}