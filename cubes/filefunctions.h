// Common routines for dealing with files

#pragma once

#ifndef FILEFUCNTIONS_H
#define FILEFUCNTIONS_H

// write an array of data
OSErr writeData(short fid, long *bytesToWrite, void *dataToWrite);

// BMP needs data in LSB, while 68k as in MSB, so we need to swap byte order
#define WRITE_TYPE(TYPE) OSErr write_##TYPE(short fid, TYPE data);

// this gives us declarations for write_char(fid, byte), write_type(fid, short), write_type(fid, long)
WRITE_TYPE(char)
WRITE_TYPE(short)
WRITE_TYPE(long)

// Open up a save dialogue
OSErr saveDialog(StandardFileReply *myReply, const unsigned char defaultName[]);

#endif
