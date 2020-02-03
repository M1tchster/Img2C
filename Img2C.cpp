#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <GdiPlus.h>												// GDI is just used to load the images conveniently.

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <vector>													// This would be a C file, but vector and GdiPlus were too convenient.

#pragma comment (lib, "GdiPlus.lib")


// I prefer to use these more compact types
typedef unsigned long	tU32;
typedef unsigned short	tU16;
typedef unsigned char	tU8;

typedef signed long		tS32;
typedef signed short	tS16;
typedef signed char		tS8;


#define dGetR(Color)	(Color & 0x000000FF)
#define dGetG(Color)	((Color & 0x0000FF00) >> 8)
#define dGetB(Color)	((Color & 0x00FF0000) >> 16)


// ----------------------------------------------
// -------- Software config ---------------------

#define dAccurateColors	1
// Use 1 for slightly more accurate colors.
// Use 0 to compute a lot faster with nearly accurate colors.
// Setting this off uses a psudo color comparing trick for extra performance.


#define dColorCutoff (4+(gColorNum>>3))
// Colors closer than this are considered the same when constructing the palette.
// Lower values make the color palette up faster, ideal for images with gradients.
// This can function well as a static number or an expression.
// **Equations can have exponential effect on performance here. Higher numbers can also slow the process.
// -----------------------------------------------



// Global variables are not a best practice, but this is a very small program.
// I prefer to prefix variables. 'g' denotes a global variable. This is to avoid naming conflicts.

Gdiplus::Bitmap* gImageBmp;			// The current image loaded into GDI
std::vector<tU8> gImageData;		// The data - after RLE/palette compression

COLORREF gColorList[256];			// A color palette. Used with RLE to further reduce data size
int gColorNum;						// The number of colors

COLORREF gBGColor;					// The dummy palette entry to consider transparent aka the "color key"




// Returns the closest color to Pixel in the palette.
// Adds to the palette if needed and if room is available.
tU8 i2cAddToPalette(const COLORREF aPixel) {

	// The goal is to find a palette entry with RGB values closer than this value, else add a new one.
	int lGoal;
	tU8 lPalIndex;

	// No free space, so the goal is to find the closest, no matter what.
	if (gColorNum == 256) {
		lGoal = 0xFFFF;
	} else {
		lGoal = dColorCutoff;
	}

	// Go through each character in the palette and return the closest one
	for (int lPal = 0; lPal < gColorNum; ++lPal) {

		const COLORREF lColor = gColorList[lPal];

#if dAccurateColors
		const int lDiff =						// Accurate color detection
			sqrt(
				(dGetR(lColor)-dGetR(aPixel))*(dGetR(lColor)-dGetR(aPixel))+
				(dGetG(lColor)-dGetG(aPixel))*(dGetG(lColor)-dGetG(aPixel))+
				(dGetB(lColor)-dGetB(aPixel))*(dGetB(lColor)-dGetB(aPixel))
			);
#else
		const int Diff =						// Faster/cheap estimate
			abs(dGetR(lColor)-dGetR(aPixel))+
			abs(dGetG(lColor)-dGetG(aPixel))+
			abs(dGetB(lColor)-dGetB(aPixel));
#endif

		if (lDiff < lGoal) {
			lPalIndex = lPal;					// The closest color in palette to the pixel found so far
			lGoal = lDiff;						// Let's set a harder goal, in case there is something better
		}

	}

	if (gColorNum != 256)						// Have free palette space
	if (lGoal == dColorCutoff) {				// Couldn't find a close match
		gColorList[gColorNum++] = aPixel;		// So add this one to the palette
		return gColorNum;						// Return the new entry
	}

	return lPalIndex;

}


// Finds the closest palette entry to the color at pixel aXP/aYP in the current image.
// Adds the color as a palette entry if necessary (see above).
inline tU8 i2cGetPixel(const tU16 aXP, const tU16 aYP) {

	static int lBGIndx = 256;					// Did we ever use the color key in the image yet?
	static Gdiplus::Color lBmpPixel;			// Actual pixel raw data
	tU8 lPalIndx;								// The palette entry (may be a little different)

	gImageBmp->GetPixel(aXP, aYP, &lBmpPixel);

	// If the color is transparent, use BGColor (color key).
	if (lBmpPixel.GetAlpha() == 0) {

		if (lBGIndx == 256) {							// Have not yet added the BG color/color key
			if (gColorNum != 256) {
				gColorList[gColorNum] = gBGColor;		// Add the exact color in the palette
				lBGIndx = gColorNum++;
			} else {									// No room for the color key?
				gColorList[255] = gBGColor;
				lBGIndx = 255;
			}
		}

		return lBGIndx;

	} else {
		lPalIndx = i2cAddToPalette(lBmpPixel.ToCOLORREF());
	}

	return lPalIndx;

}


int main(int aArgC, char* aArgV[]) {

	Gdiplus::GdiplusStartupInput	GDIStartupInput;
	ULONG_PTR						GDIToken;

	WCHAR GDIPath[MAX_PATH];
	char FilePath[MAX_PATH];
	char WriteBuf[64];

	FILE* WFile;
	HANDLE WFind;
	WIN32_FIND_DATA WFileFound;
	// -----

	Gdiplus::GdiplusStartup(&GDIToken, &GDIStartupInput, NULL);

	if (aArgC < 2 || aArgC > 3) {						// Software required parameters
		printf(
			"Usage: IMG2C.exe <PATH_TO_IMAGES> <COLORKEY> (format: \"BBGGRR\" - defaults to FF00FF)\n"
			"Supports .PNG, .GIF, .BMP and more.\n"
		);
		exit(0);
	}

	printf(
		"Designed for simple graphics, not high resolution images.\n"
		"https://github.com/gogeekery/Img2C/\n\n"
	);

	if (strlen(aArgV[1]) > MAX_PATH-64) {
		printf("Path is too long!");
		exit(0);
	}


	if (aArgC == 3) {									// User supplied a color key

		int Scan;										// Check color key input value
		for (Scan = 0; Scan < 6; ++Scan) {
			if (!isxdigit(aArgV[2][Scan]))
				break;
		}

		if (aArgC == 3 && (strlen(aArgV[2]) != 6 || Scan != 6)) {
			printf("Color Key must be a 24-bit hexadecimal in the fomat BBGGRR");
			exit(0);
		}

		sscanf(aArgV[2], "%x", &gBGColor);

	} else {											// Default color key
		gBGColor = 0x00FF00FF;
	}

	// The user defined file path to parse.
	strcpy(FilePath, aArgV[1]);
	strcat(FilePath, "\\");

	// We'll search for any and every file (and let GDI deduce if it's an image).
	strcat(FilePath, "*.*");

	// Start finding files in the directory.
	WFind = FindFirstFile(FilePath, &WFileFound);
	if (WFind == INVALID_HANDLE_VALUE) {
		printf("Directory does not exist");
		exit(0);
	}

	printf("Scanning \"%s\" for graphics to convert...\n", aArgV[1]);
	int gImgNum = 0;
	gColorNum = 0;

	do {

		if (WFileFound.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			continue;

		strcpy(FilePath, aArgV[1]);
		strcat(FilePath, WFileFound.cFileName);

		mbstowcs(GDIPath, FilePath, MAX_PATH);
		gImageBmp = new Gdiplus::Bitmap(GDIPath);

		// This file isn't a supported file type (perhaps not even a graphic).
		if (gImageBmp->GetLastStatus() != Gdiplus::Ok) {
			delete gImageBmp;
			continue;
		}

		printf("Added: %s\n", FilePath);

		// First image successfully detected;
		// Prepare the output files for further writing.
		if (gImgNum == 0) {

			WFile = fopen("ImgData.h", "w");
				fputs("//https://github.com/gogeekery/Img2C/\n\n", WFile);
			fclose(WFile);
			WFile = fopen("ImgSizes.h", "w");
				fputs("//https://github.com/gogeekery/Img2C/\n", WFile);
				fputs("#pragma once\n\n", WFile);
				fputs("const int imgSize[] = {", WFile);
			fclose(WFile);
			WFile = fopen("ImgNames.h", "w");
				fputs("//https://github.com/gogeekery/Img2C/\n", WFile);
				fputs("#pragma once\n\n", WFile);
				fputs("enum {\n", WFile);
			fclose(WFile);

		}


		// Loop throught the image, creating a palette and collecting RLE compressed data.
		for (tU32 Pos = 0; Pos < gImageBmp->GetWidth()*gImageBmp->GetHeight(); ++Pos) {

			tU8 PalIndex;				// The current pixel
			tU8 Num = 0;				// Repeats for RLE

			// Returns the closest color in the palette, adding to it if needed
			PalIndex = i2cGetPixel(tU16(Pos%gImageBmp->GetWidth()), tU16(Pos/gImageBmp->GetWidth()));

			while (i2cGetPixel(tU16((Pos+1)%gImageBmp->GetWidth()), tU16((Pos+1)/gImageBmp->GetWidth())) == PalIndex) {
				if (++Pos == gImageBmp->GetWidth()*gImageBmp->GetHeight())	break;
				if (++Num == 255)										break;
			}

			gImageData.push_back(PalIndex);
			gImageData.push_back(Num);

		}


		// Prepare to append the collected data for this image to the files (this gets messy)
		// --------------------------------------------------------------------------------------
		char ImageName[MAX_PATH];

		// Copy the file name (without extension).
		strcpy(ImageName, WFileFound.cFileName);

		if (strrchr(ImageName, '.')) {
			ImageName[strrchr(ImageName, '.')-ImageName] = '\0';
		}
		// --------

		// Append image identifier to the enumeration list:
		WFile = fopen("ImgNames.h", "a");
		fprintf(WFile, "\timg%s,\n", ImageName);
		fclose(WFile);
		// ----

		// Write the image dimensions:
		WFile = fopen("ImgSizes.h", "a");

			fprintf(
				WFile, "%c%s%hu,%hu",
				gImgNum?',': ' ',
				(!(gImgNum%16))?"\n\t":" ",
				gImageBmp->GetWidth(), gImageBmp->GetHeight()
			);

		fclose(WFile);
		// ---

		// Output the RLE compressed image data:
		WFile = fopen("ImgData.h", "a");

			fprintf(WFile, "const unsigned char cImgData%d[] = {\n", gImgNum);

			for (tU32 Pos = 0; Pos < gImageData.size(); ++Pos) {
				fprintf(WFile, "%s%hhu%c", (!(Pos % 32))?"\n\t":"", gImageData[Pos], Pos+1<gImageData.size()?',':'\n');
			}

			fputs("\n};\n\n", WFile);

		fclose(WFile);
		// ---

		gImageData.clear();
		delete gImageBmp;

		++gImgNum;

	} while (FindNextFile(WFind, &WFileFound));

	FindClose(WFind);


	// Finish up the files -----------------
	WFile = fopen("ImgData.h", "a");

		fputs("const unsigned char* lImgData[] = {", WFile);

		// Fill the pointer array with references to the different images.
		for (int Img = 0; Img < gImgNum; ++Img) {
			fprintf(WFile, "%scImgData%d%c", (!(Img%4))?"\n\t":" ", Img, (Img+1<gImgNum)?',':'\n');
		}

		fputs("};\n", WFile);

	fclose(WFile);

	// Close off the image size file:
	WFile = fopen("ImgSizes.h", "a");
		fputs("\n};\n", WFile);
	fclose(WFile);

	// Close off the image name file:
	WFile = fopen("ImgNames.h", "a");
		fputs("\timgTOTAL\n};\n", WFile);
	fclose(WFile);

	// Write the palette data that the images use:
	WFile = fopen("Palette.h", "w");

		fputs("const unsigned char lPalette[][3] = {\n\n", WFile);

		for (int Pal = 0; Pal < gColorNum; ++Pal) {
			fprintf(
				WFile,
				"\t{%hhu, %hhu, %hhu}%c\n",
				dGetR(gColorList[Pal]), dGetG(gColorList[Pal]), dGetB(gColorList[Pal]), (Pal+1<gColorNum)?',':'\n'
			);
		}

		fputs("};\n", WFile);

	fclose(WFile);
	//-----------------------------

	if (gImgNum) {
		printf("Graphics decoded and RLE compressed into ImageData.h.");
	} else {
		printf("No supported graphics found.");
	}

	Gdiplus::GdiplusShutdown(GDIToken);
	return 0;

}
