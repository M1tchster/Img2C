
#include "ImgNames.h"

// I prefer to use these more compact types
typedef unsigned long	tU32;
typedef unsigned short	tU16;
typedef unsigned char	tU8;

typedef signed long		tS32;
typedef signed short	tS16;
typedef signed char		tS8;
// -----

typedef struct {

	int W, H;
	HBITMAP Bitmap;

} resImageData;


// Creates a bitmap from an image included by Img2C
resImageData resLoadBitmap(const tU8 aImg);
