
#include <Windows.h>
#include "ImgSizes.h"
#include "Res.h"

/*
	By including the image data inside the functions,
	we avoid having to keep it on the stack the entire time.
*/


// Decompresses Img2C generated 'aData' into 'aTexture' (XRGB8888)
void resDecompressImage(tU32* aTexture, const tU8* aData, const tU16 aWidth, const tU16 aHeight) {

	int lPos, lOff, lLength;

	// Defines "lPalette" variable for us
	#include "Palette.h"

	// Decompress and fill aTexture with the data:
	for (lPos = lOff = 0; ; lOff += 2)
	for (lLength = aData[lOff+1]+1; lLength; --lLength, ++lPos) {

		if (lPos/aWidth == aHeight)
			return;

		if (lPalette[aData[lOff]][0] == 255						// Color key
		&& lPalette[aData[lOff]][1] == 0
		&& lPalette[aData[lOff]][2] == 255) {
			aTexture[lPos] = GetSysColor(COLOR_WINDOW);			// Translucency
			continue;
		}

		// Modify this format for your purposes. Output formatted for COLORREF and GDI (XXBBGGRR).
		aTexture[lPos] = (lPalette[aData[lOff]][0]<<16)+(lPalette[aData[lOff]][1]<<8)+(lPalette[aData[lOff]][2]);

	}

}


// Loads the image "aImg" (Img2C creates enums from the file names) into a bitmap
resImageData resLoadBitmap(const tU8 aImg) {

	resImageData lReturnData;
	HBITMAP lBitmap;

	{
		// Contains "lImgData" to decompress
		#include "ImgData.h"

		tU32* lData = (tU32*)malloc(imgSize[(aImg<<1)]*imgSize[(aImg<<1)+1]*4);

		resDecompressImage(lData, lImgData[aImg], imgSize[(aImg<<1)], imgSize[(aImg<<1)+1]);
		lBitmap = CreateBitmap(imgSize[(aImg<<1)], imgSize[(aImg<<1)+1], 1, 32, lData);

		free(lData);
	}

	{
		#include "ImgSizes.h"
		lReturnData.W = imgSize[(aImg<<1)];
		lReturnData.H = imgSize[(aImg<<1)+1];
		lReturnData.Bitmap = lBitmap;
	}

	return lReturnData;

}

