#pragma once

#include "stdafx.h"

// use ints
typedef struct rgbColor_t
{
	unsigned char R;
	unsigned char G;
	unsigned char B;
} rgbColor_t;

typedef struct hsvColor_t
{
	unsigned char H;
	unsigned char S;
	unsigned char V;
} hsvColor_t;

// single threaded
class MImage
{
public:

	MImage();
	~MImage();

	bool loadImage(LPCTSTR spName);

	HBITMAP getHBMP() { return m_hBMP; }
	
	bool incBSat( byte nStep = 1);

	bool decBSat( byte nStep = 1);

	bool isValid() { return m_hBMP != NULL; };

private:
	void clear();
	HBITMAP m_hBMP;
	
	bool isDomBlue( hsvColor_t& val);

	void updateImage();

	int			m_nWidth;
	int			m_nHeight;
	int			m_nStride;
	BITMAPINFO	m_bmpInfo;

	byte*		m_pBlueMap;
	byte*		m_pPixels;
};
