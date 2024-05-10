// needed for GWorldPtr
#include <QDOffscreen.h>

OSErr writeData(short fid, long *bytesToWrite, void *dataToWrite)
{
	OSErr err = FSWrite(fid, bytesToWrite, dataToWrite);
	if (err != noErr)
	{
		SysBeep(1);
	}
	return err;
}

// BMP needs data in LSB, while 68k as in MSB, so we need to swap byte order
#define WRITE_TYPE(TYPE)                                       \
	OSErr write_##TYPE(short fid, TYPE data)                   \
	{                                                          \
		long bytesToWrite = sizeof(TYPE);                      \
		unsigned char dataToWrite[sizeof(TYPE)];               \
		for (short i = 0; i < sizeof(data); i++)               \
		{                                                      \
			dataToWrite[i] = (unsigned char)(data >> (i * 8)); \
		}                                                      \
		return writeData(fid, &bytesToWrite, dataToWrite);     \
	}

// this gives us write_char(fid, byte), write_type(fid, short), write_type(fid, long)
WRITE_TYPE(char)
WRITE_TYPE(short)
WRITE_TYPE(long)

// save a cpopy of the offScreen world to a bitmap file
// default response to errors will be to beep and return
OSErr takeScreenshot(GWorldPtr offScreen)
{
	short i, j;

	// StandardPutFile to open a file save dialog
	StandardFileReply myReply;

	// might technically want to check this exists with gestalt first
	StandardPutFile(NULL, "\pUntitled.BMP", &myReply);

	if (!myReply.sfGood)
	{
		// user cancelled
		SysBeep(1);
		return fnfErr;
	}

	if (myReply.sfIsFolder || myReply.sfIsVolume)
	{
		// user selected a folder or selected a volume, neither will work here
		SysBeep(1);
		return notAFileErr;
	}

	// checked to make sure it's a file, now not that it's something that already exists
	if (myReply.sfReplacing)
	{
		// user elected to replace a file
		// not supported for now
		SysBeep(1);
		// A file with the specified name and version number already exists
		return dupFNErr;
		// if replacing, might need SetEOF to control file size?
	}

	OSErr err = noErr;
	FSSpec outFile = myReply.sfFile;
	short fid;

	// create empty file with data and resource forks?
	// 'CUBE' is the creator, which is whatever we want
	// 'bmap' should be the 4 character resource fork id/code (via the ResEdit Reference)
	// don't do this if we're replacing
	err = FSpCreate(&outFile, 'CUBE', 'bmap', myReply.sfScript);
	if (err != noErr)
	{
		SysBeep(1);
		return err;
	}

	err = FSpOpenDF(&outFile, fsWrPerm, &fid);
	if (err != noErr)
	{
		SysBeep(1);
		return err;
	}

	/*

		// From ChatGPT
		// "How could I save a copy of the image in my offscreen graphics world GWorldPtr onto disk?"

		// Get the size of the Picture data
		size = GetHandleSize((Handle)picture);

		// Write the Picture data to the file
		err = FSWrite(fileRefNum, &size, *(Handle *)picture);

	*/

	// pixmap is a ptr with all of the image info in it
	PixMapHandle pixmap; // this is a **
	pixmap = GetGWorldPixMap(offScreen);
//	pixmap = offScreen->portPixMap;


	unsigned long myRowBytes = (**pixmap).rowBytes & 0x3fff;

	// top and left often 0, but not always so
//	unsigned long width = (**pixmap).bounds.right - (**pixmap).bounds.left;
	unsigned long width = myRowBytes; // need to take into account bpp
	unsigned long height = (**pixmap).bounds.bottom - (**pixmap).bounds.top;
	
//	width = width + 16;
//	height = height + 6;
	
	Boolean indexedColor = (**pixmap).pixelType == 0;
//	Boolean indexedColor = false;


//	write_long(fid, (**pixmap).packType); // should be 0 for RGB
//	write_char(fid, 0x99);
	
	
	long numBytes = myRowBytes*height;
	
	short paletteSize = 0;
	if (indexedColor)
	{
		paletteSize = (**((**pixmap).pmTable)).ctSize;
	}
	
	short pixelSize = (**pixmap).pixelSize;
	
	long fileSize = numBytes + paletteSize * 3 + 0x36; // 0x36 is the size of the header
	
	long headerLength = 0x36 + paletteSize*3;
	
	short paddingAfterHeader = 4 - headerLength % 4;
	// ^ shoudl always be > 0
	headerLength = headerLength + paddingAfterHeader;
	

	// GetPixBaseAddr() to get where the ptr is
	// pmTable would be the indexed colour table?
	// need to dump the cTable (pointed to by pmTable), as everything uses a default table by default
	// pixelsize - colour depth
	// pixeltype - indexed or not
	// packtype - pixel colour layout, which is likely to be 'chunky' - rgbrgbrgb
	// pmVersion to check if address is accessible in 32bit mode only

	// TODO write a BMP...
	// https://en.wikipedia.org/wiki/BMP_file_format
	// https://upload.wikimedia.org/wikipedia/commons/f/fd/WinBmpHeaders.png

	// header field
	write_char(fid, 'B'); // 0x00, bfType
	write_char(fid, 'M');

	// TODO file size
	write_long(fid, fileSize); // 0x02, bfSize

	// reserved
	write_short(fid, 0); // 0x06, bfReserved1
	write_short(fid, 0); // 0x08, bfReserved2

	// TODO offset of pixel array
	write_long(fid, headerLength); // 0x0A, biOffBits

	// we're writing a BITMAPINFOHEADER
	write_long(fid, 40); // 0x0E, biSize

	// x and y image resolution
	write_long(fid, width); // 0x12, biWidth
	write_long(fid, height); // 0x16, biHeight

	// number of colour planes
	write_short(fid, 1); // 0x1A, biPlanes

	// colour depth (bits per pixel)
	write_short(fid, pixelSize); // 0x1C, biBitCount

	// compression method // 0x1E, biCompression
	//if (pixmap->pmTable == NULL)
//	if ((*((*pixmap)->pmTable)) == NULL)
//	if ((**pixmap).pmTable == NULL)
//	if (!indexedColor)
//	{
		// 0 is BI_RGB, no compression
		write_long(fid, 0);
//	}
//	else
//	{
//		// 3 is BI_BITFIELDS, indexed colour
//		write_long(fid, 3);
//	}

	// the size of the raw bitmap data - this can just be 0 with BI_RGB
	write_long(fid, numBytes); // 0x22, biSizeImage

	// the horizontal and vertical resolution of the image (pixels/metre)
	// magic number from gimp
	write_long(fid, 0xB13); // 0x26, biXPelsPerMeter
	write_long(fid, 0xB13); // 0x2A, biYPelsPerMeter

	// number of colours in the color palette
	if (!indexedColor)
	{
		write_long(fid, 0); // 0x2E, biClrUsed
	}
	else
	{
		write_long(fid, paletteSize);
	}
	// number of important colors in the palette
	if (!indexedColor)
	{
		write_long(fid, 0); // 0x32, biClrImportant
	}
	else
	{
		write_long(fid, paletteSize);
	}
	// match the colour depth?
	
	// padding?
//		write_char(fid,0);
//		write_char(fid,0);
//		write_char(fid,0);

	// indexed colour table
	// R, G, B for palette colors?

	// Programming QuickDraw, page 105
	// ctSeed, ctSize, ctTable

	// R, G, B, 0x00 for 4 byte alignment?
	for (i = 0; i < paletteSize; i++)
	{

		Boolean found = false;
		for (j = 0; j < paletteSize; j++)
		{
			// value inside is the index color to match against?

			if ( (**((**pixmap).pmTable)).ctTable[j].value == i)
			{

				write_char(fid, (**((**pixmap).pmTable)).ctTable[j].rgb.red);
				write_char(fid, (**((**pixmap).pmTable)).ctTable[j].rgb.green);
				write_char(fid, (**((**pixmap).pmTable)).ctTable[j].rgb.blue);
				
				
				write_char(fid, (**((**pixmap).pmTable)).ctTable[j].rgb.blue);
				write_char(fid, (**((**pixmap).pmTable)).ctTable[j].rgb.green);
				write_char(fid, (**((**pixmap).pmTable)).ctTable[j].rgb.red);
				
		write_char(fid,0); // not needed? I think it is based on outfile.c
				
				
				found = true;
				break;
			}
		
		
		}
		if (!found)
		{
				write_char(fid,0);
				write_char(fid,0);
				write_char(fid,0);
		write_char(fid,0); // not needed?
		}

		
	}
	
	// pixel data for palette
	
	// padding to align on 4 byte boundary
	for (i = 0; i < paddingAfterHeader; i++)
	{
		write_char(fid, 0);
	}
	
//	LockPixels(pixmap); // already locked in main()
	
	long *bitsPtr = (long*)GetPixBaseAddr(pixmap);
	char mmuMode = true32b;
	SwapMMUMode(&mmuMode);
	
	// TODO the image comes out upside down from this
	err = FSWrite(fid, &numBytes, bitsPtr);
	
	
/*	long t = 1;
	
	for (j = 0; j < height; j++)
	{
		for (i = 0; i < width; i++)
		{
//			write_char(fid,
//			 *(bitsPtr + (j - height) * myRowBytes + ((i - width) * pixelSize) / 8));
//			 *(bitsPtr + (height - j) * myRowBytes + i);


			FSWrite(fid, &t, bitsPtr + (height - j) * myRowBytes + i);

		}
	}*/
	
	
	
//	if (err != noErr)
//	{
//		SysBeep(1);
//	}
	
	
	
	SwapMMUMode(&mmuMode);
	
//	UnlockPixels(pixmap); // already locked in main()
	

	err = FSClose(fid);
	if (err != noErr)
	{
		SysBeep(1);
		return err;
	}

	return noErr;
}
