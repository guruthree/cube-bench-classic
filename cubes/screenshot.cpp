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

// save a copy of the offScreen world to a bitmap file
// default response to errors will be to beep and return
OSErr takeScreenshot(GWorldPtr offScreen)
{
	short i; // for looping

	/* Figure out file stuff */

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

	/* Figure out image stuff */

	// Programming QuickDraw (1992) by David A Surovell, Frederick M Hall, and Konstantin Othmer
	// has been absolutely invaluable for figuring out how to do this

	// pixmap is a ptr with all of the image info in it (page 136)
	PixMapHandle pixmap; // this is a **
	pixmap = GetGWorldPixMap(offScreen);
	//	pixmap = offScreen->portPixMap;

	// pixelSize indicates the colour depth (bits per pixel)
	short pixelSize = (**pixmap).pixelSize;
	unsigned long myRowBytes = (**pixmap).rowBytes & 0x3fff;

	// the resolution of the back buffer is usually larger than the actually displayed
	// image due to byte (boundary) alignment, it's most convenient to write it out
	// in this form so we're writing out a slightly larger image and there will be a
	// little bit of garbage in the frame as a result
	// note top and left are often 0, but not always so
	unsigned long width = (myRowBytes * 8) / pixelSize; // takes into account bpp (page 138)
	unsigned long height = (**pixmap).bounds.bottom - (**pixmap).bounds.top;

	// pixelType indicates whether the buffer is indexed or not
	Boolean indexedColor = (**pixmap).pixelType == 0;
	// if we're 16/24 bit and not indexed we need to do some tricks down the line
	Boolean customBitfields = pixelSize > 8 && !indexedColor;

	// size of the image data
	long numBytes = myRowBytes * height;

	// how many colours are in the palette (if a palette is used)
	short paletteSize = 0;
	if (indexedColor)
	{
		// it seems like BMP expects palette size to be the < type,
		// similar to for (i = 0; i < paletteSize; i++) so that if the
		// palette was black and white, paletteSize = 2
		paletteSize = (**((**pixmap).pmTable)).ctSize + 1;
		// note pmTable is a CTabHandle, which is a struct that contains
		// all of the info about the colour together pallete (page 104)
		// lots of deferencing to extract values out of it
	}

	// note somwhere packType should probably checked to verify the pixel colour layout
	// this should be 'chunky' - rgbrgbrgb

	/* Write a BMP image */
	// https://en.wikipedia.org/wiki/BMP_file_format
	// https://upload.wikimedia.org/wikipedia/commons/f/fd/WinBmpHeaders.png
	// https://en.wikipedia.org/wiki/File:BitfieldsSLN.svg

	// calculate the total size of the header (BITMAPINFOHEADER + pallette info)
	// BITMAPINFOHEADERis always 0x36 bytes
	long headerLength = 0x36 + paletteSize * 4;
	// plus also bitfields information
	if (customBitfields)
	{
		// space for RGB pixel masks
		headerLength += 12; // 3 * 4
	}

	// calculate the final file size
	long fileSize = numBytes + headerLength;

	// header field
	write_char(fid, 'B'); // 0x00, bfType
	write_char(fid, 'M');

	// file size
	write_long(fid, fileSize); // 0x02, bfSize

	// reserved
	write_short(fid, 0); // 0x06, bfReserved1
	write_short(fid, 0); // 0x08, bfReserved2

	// offset of pixel array
	write_long(fid, headerLength); // 0x0A, biOffBits

	// we're writing a BITMAPINFOHEADER
	write_long(fid, 40); // 0x0E, biSize

	// x and y image resolution
	write_long(fid, width);	 // 0x12, biWidth
	write_long(fid, height); // 0x16, biHeight

	// number of colour planes
	write_short(fid, 1); // 0x1A, biPlanes

	// colour depth (bits per pixel)
	write_short(fid, pixelSize); // 0x1C, biBitCount

	// compression method
	// 0 is BI_RGB, indexed colour
	// 3 is BI_BITFIELDS, different bits of bytes mean different colors
	if (indexedColor)
	{
		write_long(fid, 0); // 0x1E, biCompression
	}
	else
	{
		// when not indexed different bits mean different colors
		write_long(fid, 3);
	}

	// the size of the raw bitmap data - this can just be 0 with BI_RGB
	write_long(fid, numBytes); // 0x22, biSizeImage

	// the horizontal and vertical resolution of the image (pixels/metre)
	write_long(fid, 0); // 0x26, biXPelsPerMeter
	write_long(fid, 0); // 0x2A, biYPelsPerMeter

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

	// write out the indexed colour table if that's a thing
	// (almost everything uses a default table and is indexed colour)
	// R, G, B, 0x00 for 4 byte alignment
	// we need to dump the cTable (pointed to by pmTable)
	for (i = 0; i < paletteSize; i++)
	{
		// Macs store colour as 16-bit integers (page 97)
		write_char(fid, (**((**pixmap).pmTable)).ctTable[i].rgb.blue >> 8);
		write_char(fid, (**((**pixmap).pmTable)).ctTable[i].rgb.green >> 8);
		write_char(fid, (**((**pixmap).pmTable)).ctTable[i].rgb.red >> 8);

		// padding
		write_char(fid, 0);
	}

	if (customBitfields)
	{
		if (pixelSize == 16)
		{
			// 16 bit color bits are 0b0RRR RRGG GGGB BBBB
			// endianness really plays heck with this one

			// red mask 0x36
			write_char(fid, 0x7C); // 0b01111100
			write_char(fid, 0x00); // 0b00000000
			write_char(fid, 0);
			write_char(fid, 0);

			// green mask 0x3A
			write_char(fid, 0x03); // 0b00000011
			// we can't have the bitfield split like this
			// so we just take the most significant bits to indicate some green
			// and ignore the rest for now
			// write_char(fid, 0xE0); // 0b11100000
			write_char(fid, 0);
			write_char(fid, 0);
			write_char(fid, 0);

			// note the actual solution would be to write out edian swapped
			// or to convert to a different pallette before export. hah.

			// blue mask 0x3E
			write_char(fid, 0x00); // 0b00000000
			write_char(fid, 0x1F); // 0b00011111
			write_char(fid, 0);
			write_char(fid, 0);

			// // padding
			// write_char(fid, 0);
			// write_char(fid, 0);
		}
		else if (pixelSize == 32)
		{
			// 00 DD 08 06 is 0RGB, but normally RGB0 is expected
			// swap around for endianess

			// red mask 0x36
			write_char(fid, 0);
			write_char(fid, 255);
			write_char(fid, 0);
			write_char(fid, 0);

			// green mask 0x3A
			write_char(fid, 0);
			write_char(fid, 0);
			write_char(fid, 255);
			write_char(fid, 0);

			// blue mask 0x3E
			write_char(fid, 0);
			write_char(fid, 0);
			write_char(fid, 0);
			write_char(fid, 255);
		}
	}

	//	LockPixels(pixmap); // already locked in main()

	// GetPixBaseAddr() to get where the pointer to the buffer is
	// (the pointer should never be accessed directly out of the struct)
	long *bitsPtr = (long *)GetPixBaseAddr(pixmap);

	// it's possible bitsPtr is a 32-bit address and if so we need to swap
	// the memory management unit (mmu) to 32-bit in order to access it
	// pmVersion to check if address is accessible in 32bit mode only (page)
	// http://preserve.mactech.com/articles/develop/issue_08/095-098_Guest_column.html
	Boolean highmem = (**pixmap).pmVersion == baseAddr32;
	char mmuMode = true32b;
	if (highmem)
	{
		// swap to 32-bit memory mode
		SwapMMUMode(&mmuMode);
	}

	// flip the image since BMP expects the data for rows to go from bottom to top
	// and the mac stores it the other way around
	long *rowBuffer = (long *)NewPtr(myRowBytes);
	for (i = 0; i < height / 2; i++)
	{
		long *topRow = bitsPtr + i * myRowBytes / 4; // divide by 4 because LONG addresses are 4 bytes
		long *bottomRow = bitsPtr + (height - i - 1) * myRowBytes / 4;
		BlockMove(bottomRow, rowBuffer, myRowBytes);
		BlockMove(topRow, bottomRow, myRowBytes);
		BlockMove(rowBuffer, topRow, myRowBytes);
	}
	DisposePtr((Ptr)rowBuffer);

	// write out pixel data
	err = FSWrite(fid, &numBytes, bitsPtr);
	if (err != noErr)
	{
		SysBeep(1);
		SysBeep(1);
	}

	if (highmem)
	{
		// swap back to the previous MMU mode
		SwapMMUMode(&mmuMode);
	}

	//	UnlockPixels(pixmap); // already locked in main()

	// the main routine isn't expecting the buffer to be flipped
	// clear it so everything can be drawn new fresh
	SetGWorld(offScreen, NULL);
	Rect bounds = (**pixmap).bounds;
	EraseRect(&bounds);

	err = FSClose(fid);
	if (err != noErr)
	{
		SysBeep(1);
		return err;
	}

	return noErr;
}
