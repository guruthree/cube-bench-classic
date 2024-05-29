// Common routines for opening up a save dialog

#pragma once

#ifndef SAVEDIALOG_H
#define SAVEDIALOG_H

// Open up a save dialogue
OSErr saveDialog(StandardFileReply *myReply, const unsigned char defaultName[]);

#endif
