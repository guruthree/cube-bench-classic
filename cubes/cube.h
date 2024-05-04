#include "vectormath2.h"
#include "bubblesort.h"

#pragma once

#ifndef CUBE
#define CUBE

class Cube
{
private:
	// location
	Vector3 cube[8];
	// centre of the cube in coordiante space
	Vector3 centre;
	// location in screen coordiantes
	Vector3 screenCube[8];

	// size of the cube (used to make & for collisions)
	float size;

	// render a face, filled in (face numbers 0 to 5)
	void solidFace(short face, Boolean one_bit);

public:
	// unit cube
	static const Vector3 verticies[8];
	// mapping of vertices to make quads (polys) for rendering
	static const short faces[6][5];
	// colors of the faces
	static const long colors[6];

	// velocity of spin
	Vector3 dangle;
	// velocity of movement
	Vector3 velocity;

	// the edges of the bounding rectangle in screen space
	short leftBound, rightBound, upperBound, lowerBound;

	Cube(float size);

	// go back to initial settings
	void reset();

	// make bigger
	void increaseSize();

	// make smaller
	void decreaseSize();

	// set to specific size
	void updateSize(float size);

	// adjust the size of cube by a factor
	void scale(float scale);

	// translate the cube
	void translate(Vector3 offset);

	// rotate the cube
	void rotate(Vector3 dangle);

	// apply velocities
	void autoRotate();
	void autoTranslate();

	// calculate where the cube is in screen space
	void preCalculate(short xRes, short yRes);

	// render the cube as wireframe
	void draw(Boolean color);

	// render the cube filled cube
	void solidCube(Boolean color, Boolean one_bit);

	// identify & update the region on screen that the cube is using
	void calculateBounds();

	// convert bounds to Region, the cube will be somwhere within
	void roughBounds(RgnHandle rgn, short xRes, short yRes);

	// z-coordinate of the cube (1000-z) is the distance away from the camera
	float getZ();
};

#endif
