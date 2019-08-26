// amstest.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

#include "amstest.h"

#include <exception>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

MImage	gImage;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_AMSTEST, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AMSTEST));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AMSTEST));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_AMSTEST);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// extra handlers

void OpenImageFile(HWND hWnd)
{
	OPENFILENAME ofn;
	TCHAR szFile[MAX_PATH] = { 0 };

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = _T("BMP files\0*.bmp\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = nullptr;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		if ( !gImage.loadImage(ofn.lpstrFile))
		{
			MessageBox(0, _T("FAILED to load image"), _T("Error"), MB_ICONSTOP | MB_OK);
		}
		InvalidateRect(hWnd, 0, TRUE); // redraw the main window
	}
}

//////////////////////////////////////////////////////////////////////////
// MImage implementation
MImage::MImage()
{
	m_hBMP = NULL;
	m_pBlueMap = nullptr;
	m_pPixels = nullptr;
	m_nWidth = 0;
	m_nHeight = 0;
	m_nStride = 0;
}

MImage::~MImage()
{
	clear();
}

// TBD OPT, fix levels
bool MImage::isDomBlue(hsvColor_t& val)
{
	bool bRet = false;

	hsvColor_t bMin = { 100, 150, 0};
	hsvColor_t bMax = { 140, 255, 255};

	if ((val.H >= bMin.H && val.H <= bMax.H) &&
		(val.S >= bMin.S && val.S <= bMax.S) &&
		(val.V >= bMin.V && val.V <= bMax.V))
	{
		bRet = true;
	}

	return bRet;
}

// ints

rgbColor_t _hsv2rgb(hsvColor_t hsv)
{
	rgbColor_t rgb;
	unsigned char region, remainder, p, q, t;

	if (hsv.S == 0)
	{
		rgb.R = hsv.V;
		rgb.G = hsv.V;
		rgb.B = hsv.V;
		return rgb;
	}

	region = hsv.H / 43;
	remainder = (hsv.H - (region * 43)) * 6;

	p = (hsv.V * (255 - hsv.S)) >> 8;
	q = (hsv.V * (255 - ((hsv.S * remainder) >> 8))) >> 8;
	t = (hsv.V * (255 - ((hsv.S * (255 - remainder)) >> 8))) >> 8;

	switch (region)
	{
		case 0: rgb.R = hsv.V; rgb.G = t; rgb.B = p; break;
		case 1: rgb.R = q; rgb.G = hsv.V; rgb.B = p; break;
		case 2: rgb.R = p; rgb.G = hsv.V; rgb.B = t; break;
		case 3: rgb.R = p; rgb.G = q; rgb.B = hsv.V; break;
		case 4: rgb.R = t; rgb.G = p; rgb.B = hsv.V; break;
		default: rgb.R = hsv.V; rgb.G = p; rgb.B = q; break;
	}

	return rgb;
}

hsvColor_t _rgb2hsv( rgbColor_t rgb)
{
	hsvColor_t hsv;
	unsigned char rgbMin, rgbMax;

	rgbMin = rgb.R < rgb.G ? (rgb.R < rgb.B ? rgb.R : rgb.B) : (rgb.G < rgb.B ? rgb.G : rgb.B);
	rgbMax = rgb.R > rgb.G ? (rgb.R > rgb.B ? rgb.R : rgb.B) : (rgb.G > rgb.B ? rgb.G : rgb.B);

	hsv.V = rgbMax;
	if (hsv.V == 0)
	{
		hsv.H = 0;
		hsv.S = 0;
		return hsv;
	}

	hsv.S = 255 * long(rgbMax - rgbMin) / hsv.V;
	if (hsv.S == 0)
	{
		hsv.H = 0;
		return hsv;
	}

	if (rgbMax == rgb.R)
	{
		hsv.H = 0 + 43 * (rgb.G - rgb.B) / (rgbMax - rgbMin);
	}
	else if (rgbMax == rgb.G)
	{
		hsv.H = 85 + 43 * (rgb.B - rgb.R) / (rgbMax - rgbMin);
	}
	else
	{
		hsv.H = 171 + 43 * (rgb.R - rgb.G) / (rgbMax - rgbMin);
	}

	return hsv;
}

bool MImage::loadImage(LPCTSTR spName)
{
	bool bRet = false;
	clear();

	m_hBMP = (HBITMAP)LoadImage( nullptr, spName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	bRet = isValid();
	if (bRet)
	{
		// prepare blue index
		BITMAP Bmp = { 0 };

		HDC DC = CreateCompatibleDC(NULL);
		HBITMAP OldBitmap = (HBITMAP)SelectObject(DC, m_hBMP);
		GetObject( m_hBMP, sizeof(Bmp), &Bmp);

		m_bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		m_bmpInfo.bmiHeader.biWidth = m_nWidth = Bmp.bmWidth;
		m_bmpInfo.bmiHeader.biHeight = m_nHeight = Bmp.bmHeight;
		m_bmpInfo.bmiHeader.biPlanes = 1;
		m_bmpInfo.bmiHeader.biBitCount = Bmp.bmBitsPixel;
		m_bmpInfo.bmiHeader.biCompression = BI_RGB;

		m_nStride = ((m_nWidth * Bmp.bmBitsPixel + 31) / 32) * 4;

		m_bmpInfo.bmiHeader.biSizeImage = m_nStride * m_nHeight;
		try
		{
			m_pPixels = new byte[m_bmpInfo.bmiHeader.biSizeImage];
		}
		catch (const std::exception& e)
		{
			// handle e
			(e);
			bRet = false;
		}

		if (bRet)
		{
			int nRet = GetDIBits(DC, m_hBMP, 0, m_nHeight, m_pPixels, &m_bmpInfo, DIB_RGB_COLORS);
			SelectObject(DC, OldBitmap);

			m_nHeight = std::abs(m_nHeight);

			byte* pPix = m_pPixels;

			try
			{
				m_pBlueMap = new byte[m_nWidth * m_nHeight];
			}
			catch (const std::exception& e)
			{
				// handle e
				(e);
				bRet = false;
			}
			if (bRet)
			{
				for (int y = 0; y < m_nHeight; ++y)
				{
					pPix = m_pPixels + y * m_nStride;
					for (int x = 0; x < m_nWidth; ++x)
					{
						rgbColor_t rgb;
						rgb.B = *pPix++;
						rgb.G = *pPix++;
						rgb.R = *pPix++;
						*pPix++; 

						hsvColor_t hsv = _rgb2hsv(rgb);

						m_pBlueMap[y* m_nWidth + x] = isDomBlue(hsv) ? 1 : 0;
					}
				}
			}
		}
		DeleteDC(DC);
	}
	return bRet;
}

void MImage::updateImage()
{
	if (isValid())
	{
		HDC DC = CreateCompatibleDC(NULL);
		int nRet = SetDIBits(DC, m_hBMP, 0, m_nHeight, m_pPixels, &m_bmpInfo, DIB_RGB_COLORS);
		DeleteDC(DC);
	}
}

//TBD REF
bool MImage::incBSat( byte nStep /*= 1*/)
{
	if (isValid())
	{
		for (int y = 0; y < m_nHeight; ++y)
		{
			for (int x = 0; x < m_nWidth; ++x)
			{
				if (m_pBlueMap[y* m_nWidth + x]>0)
				{
					byte* pPix = m_pPixels + m_nStride * y + x * 4;

					rgbColor_t rgb;
					rgb.B = *pPix;
					rgb.G = *(pPix+1);
					rgb.R = *(pPix+2);

					hsvColor_t hsv = _rgb2hsv(rgb);

					hsv.S += nStep;

					rgb = _hsv2rgb(hsv);

					// set pixel
					*pPix = rgb.B; // b 
					*(pPix+1) = rgb.G; // g
					*(pPix+2) = rgb.R; // r
				}
			}
		}

		updateImage();
	}
	return true;
}

//TBD REF
bool MImage::decBSat( byte nStep /*= 1*/)
{
	if (isValid())
	{
		for (int y = 0; y < m_nHeight; ++y)
		{
			for (int x = 0; x < m_nWidth; ++x)
			{
				if (m_pBlueMap[y* m_nWidth + x] > 0)
				{
					byte* pPix = m_pPixels + m_nStride * y + x * 4;

					rgbColor_t rgb;
					rgb.B = *pPix;
					rgb.G = *(pPix + 1);
					rgb.R = *(pPix + 2);

					hsvColor_t hsv = _rgb2hsv(rgb);

					hsv.S -= nStep;

					rgb = _hsv2rgb(hsv);

					// set pixel
					*pPix = rgb.B; // b 
					*(pPix + 1) = rgb.G; // g
					*(pPix + 2) = rgb.R; // r
				}
			}
		}

		updateImage();
	}
	return true;
}

void MImage::clear()
{
	if (isValid())
	{
		DeleteObject(m_hBMP);
		m_hBMP = NULL;
	}

	if (m_pBlueMap)
	{
		delete[] m_pBlueMap;
		m_pBlueMap = nullptr;
	}

	if (m_pPixels)
	{
		delete[] m_pPixels;
		m_pPixels = nullptr;
	}

	memset( &m_bmpInfo, 0, sizeof(BITMAPINFO));
}

//////////////////////////////////////////////////////////////////////////

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case ID_FILE_OPEN:
				OpenImageFile(hWnd);
				break;
			case ID_FILE_INCBSAT:
				gImage.incBSat();
				InvalidateRect(hWnd, 0, TRUE); 
				break;
			case ID_FILE_DECBSAT:
				gImage.decBSat();
				InvalidateRect(hWnd, 0, TRUE); 
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

			if (gImage.isValid())
			{
				BITMAP          bitmap;
				HDC             hdcMem;
				HGDIOBJ         oldBitmap;

				hdcMem = CreateCompatibleDC(hdc);
				oldBitmap = SelectObject(hdcMem, gImage.getHBMP());

				GetObject(gImage.getHBMP(), sizeof(bitmap), &bitmap);
				BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

				SelectObject(hdcMem, oldBitmap);
				DeleteDC(hdcMem);
			}
			
			EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

