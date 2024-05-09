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
	// GetPixBaseAddr() to get where the ptr is
	// pmTable would the indexed colour table?
	// need to dump the cTable (pointed to by pmTable), as everything uses a default table by default
	// pixelsize - colour depth
	// pixeltype - indexed or not
	// packtype - pixel colour layout, which is likely to be 'chunky' - rgbrgbrgb

	// TODO write a BMP...
	// https://en.wikipedia.org/wiki/BMP_file_format

	write_char(fid, 'B');
	write_char(fid, 'M');

	err = FSClose(fid);
	if (err != noErr)
	{
		SysBeep(1);
		return err;
	}

	return noErr;
}