// Save dialogue functions

#include <string.h> // strlen

// write an array of data
OSErr writeData(short fid, long *bytesToWrite, void *dataToWrite)
{
	OSErr err = FSWrite(fid, bytesToWrite, dataToWrite);
	if (err != noErr)
	{
		SysBeep(1);
	}
	return err;
}

// x86 (BMP) needs data in LSB, while 68k as in MSB, so we need to swap byte order
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

// write a string to the file
OSErr writeString(short fid, char stringToWrite[])
{
	// if the string is a pstring convert to a regular string
	if (stringToWrite[0] == '\p' || stringToWrite[0] == strlen(stringToWrite + 1))
	{
		// this detection will false positive if the first character happens to equal the length
		// e.g. if the string starts with '0' and is 41 bytes long
		stringToWrite++;
	}
	long bytesToWrite = strlen(stringToWrite);
	return writeData(fid, &bytesToWrite, stringToWrite);
}

// Open up a save dialogue
OSErr saveDialog(StandardFileReply *myReply, const unsigned char defaultName[])
{
	OSErr err = noErr;
	short fid;

	// StandardPutFile to open a file save dialogue
	// might technically want to check this function exists with gestalt before calling
	StandardPutFile(NULL, defaultName, myReply);

	if (!myReply->sfGood)
	{
		// user cancelled
		return fnfErr;
	}

	if (myReply->sfIsFolder || myReply->sfIsVolume)
	{
		// user selected a folder or selected a volume, neither will work here
		SysBeep(1);
		return notAFileErr;
	}

	FSSpec outFile = myReply->sfFile;

	// checked to make sure it's a file, now not that it's something that already exists
	if (myReply->sfReplacing)
	{
		// user elected to replace a file, delete the existing one to put a new
		err = FSpDelete(&(myReply->sfFile));
		if (err != noErr)
		{
			SysBeep(1);
			return err;
		}
	}

	return noErr;
}
