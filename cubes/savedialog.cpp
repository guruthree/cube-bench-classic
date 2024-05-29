// Save dialogue functions

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

	return err;
}
