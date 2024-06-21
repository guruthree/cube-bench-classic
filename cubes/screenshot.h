#pragma once

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

// save a cpopy of the offScreen world to a bitmap file
OSErr takeScreenshot(GWorldPtr offScreen, Boolean one_bit);

#endif
