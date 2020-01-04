
#include <Windows.h>
#include <stdio.h>

#include "Res.h"


// Start with the first image
tU8 gImgActive = 0;

// Image data generated in Res.c with files from Img2c
resImageData gImgData;


LRESULT CALLBACK WndProc (HWND hWnd, UINT aMsg, WPARAM wParam, LPARAM lParam) {

	static int lClientW, lClientH;

	switch (aMsg) {

		case WM_CREATE:
		{
			gImgData = resLoadBitmap(gImgActive);
		}	
		return 0;

		case WM_SIZE:
		{
			lClientW = LOWORD(lParam);
			lClientH = HIWORD(lParam);
		}
		return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT lPStr;
			HDC lHDC, lMem;

			lHDC = BeginPaint (hWnd, &lPStr);

			lMem = CreateCompatibleDC(lHDC);
			SelectObject(lMem, gImgData.Bitmap);

			BitBlt(
				lHDC,
				(lClientW-gImgData.W)/2, (lClientH-gImgData.H)/2,
				gImgData.W, gImgData.H,
				lMem, 0, 0, SRCCOPY
			);

			{	// Draw text
				const char lStr[] = "Press any key to cycle images.\nArt embedded into the program via Img2C.";
				const int lLen = sizeof(lStr);
				RECT lRect = {0, lClientH-48, lClientW, lClientH};
				SetBkMode(lHDC, TRANSPARENT);
				DrawText(lHDC, lStr, lLen, &lRect, DT_CENTER|DT_NOCLIP);
			}

			DeleteDC(lMem);
			EndPaint(hWnd, &lPStr);
		}
		return 0;

		case WM_KEYDOWN:
		{
			// Cycle an image, looping back if we hit the end
			gImgActive = (gImgActive+1)%imgTOTAL;

			// Load the new bitmap
			DeleteObject(gImgData.Bitmap);
			gImgData = resLoadBitmap(gImgActive);

			InvalidateRect(hWnd, NULL, 1);
		}
		return 0;


		case WM_DESTROY:
		{
			DeleteObject(gImgData.Bitmap);
			PostQuitMessage (0);
		}
		return 0;

	}

	return DefWindowProc (hWnd, aMsg, wParam, lParam) ;

}


//
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {

	HWND         hWnd;
	MSG          lMsg;
	WNDCLASS     wndClass;
	static TCHAR szAppName [] = TEXT("Demo2c");

	wndClass.style         = CS_HREDRAW | CS_VREDRAW ;
	wndClass.lpfnWndProc   = WndProc ;
	wndClass.cbClsExtra    = 0 ;
	wndClass.cbWndExtra    = 0 ;
	wndClass.hInstance     = hInstance ;
	wndClass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
	wndClass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndClass.hbrBackground = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	wndClass.lpszMenuName  = NULL ;
	wndClass.lpszClassName = szAppName ;

	if (!RegisterClass(&wndClass)) {
		MessageBox (NULL, TEXT ("Windows NT or later"), szAppName, MB_ICONERROR) ;
		return 0 ;
	}

	hWnd = CreateWindow (
		szAppName, TEXT ("Img2C Demo"),
		WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, CW_USEDEFAULT,
		640, 640,
		NULL, NULL, hInstance, NULL
	);

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&lMsg, NULL, 0, 0)) {

		TranslateMessage(&lMsg);
		DispatchMessage(&lMsg);

	}
	return lMsg.wParam;

}
