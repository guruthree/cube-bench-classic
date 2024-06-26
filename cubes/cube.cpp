#include "vectormath2.h"
#include "matrix.h"
#include "bubblesort.h"
#include "cube.h"

// unit cube
const Vector3 Cube::verticies[8] = {
	Vector3(-0.5f, -0.5f, 0.5f),
	Vector3(0.5f, -0.5f, 0.5f),
	Vector3(0.5f, 0.5f, 0.5f),
	Vector3(-0.5f, 0.5f, 0.5f),
	Vector3(-0.5f, -0.5f, -0.5f),
	Vector3(0.5f, -0.5f, -0.5f),
	Vector3(0.5f, 0.5f, -0.5f),
	Vector3(-0.5f, 0.5f, -0.5f)};

// mapping of vertices to make quads (polys) for rendering
const short Cube::faces[6][5] = {
	{0, 1, 2, 3, 0},
	{4, 5, 6, 7, 4},
	{0, 4, 5, 1, 0},
	{0, 4, 7, 3, 0},
	{2, 6, 7, 3, 2},
	{2, 6, 5, 1, 2}};

// colours of the faces
const long Cube::colors[6] = {redColor, blueColor, greenColor, magentaColor, cyanColor, yellowColor};

Cube::Cube(float newSize)
{
	// fill up cube just to make sure we don't accidentally stuff anything up
	reset();
	updateSize(newSize);
}

// go back to initial settings
void Cube::reset()
{
	short i;
	for (i = 0; i < 8; i++)
	{
		cube[i] = verticies[i];
	}
	centre.x = 0;
	centre.y = 0;
	centre.z = 0;
	dangle.x = 0.01f;
	dangle.y = 0.005f;
	dangle.z = 0.01f;
	velocity.x = 1;
	velocity.y = 0.8;
	velocity.z = 0.2; //-1; // +ve is closer to the screen
	size = 1;
}

// make bigger
void Cube::increaseSize()
{
	updateSize(size + 10);
}

// make smaller
void Cube::decreaseSize()
{
	if (size > 10)
		updateSize(size - 10);
}

// set to specific size
void Cube::updateSize(float newSize)
{
	scale(newSize / size);
	size = newSize;
}

// adjust the size of cube by a factor
void Cube::scale(float scale)
{
	short i;
	for (i = 0; i < 8; i++)
	{
		cube[i] = cube[i].scale(scale);
	}
}

// translate the cube
void Cube::translate(Vector3 offset)
{
	centre = centre.add(offset);
}

// rotate the cube
void Cube::rotate(Vector3 dangle)
{
	short i;
	Matrix3 rot = Matrix3::getRotationMatrix(dangle);
	for (i = 0; i < 8; i++)
	{
		cube[i] = rot.preMultiply(cube[i]);
	}
}

// apply velocities
void Cube::autoRotate()
{
	rotate(dangle);
}

void Cube::autoTranslate()
{
	translate(velocity);
}

// calculate where the cube is in screen space
void Cube::preCalculate(short xRes, short yRes)
{
	short i;
	Matrix3 rot = Matrix3::getRotationMatrix(dangle);
	for (i = 0; i < 8; i++)
	{
		screenCube[i] = cube[i].add(centre);
		// pretty sure the 1000 here means the screen is 1000 away
		screenCube[i] = screenCube[i].scale(1000.0f / (-screenCube[i].z + 1000.0f));
		screenCube[i].x = screenCube[i].x + xRes / 2;
		screenCube[i].y = screenCube[i].y + yRes / 2;
	}
}

// render the cube as wireframe
void Cube::draw(Boolean color)
{
	// front face
	if (color)
	{
		ForeColor(redColor);
	}
	MoveTo(screenCube[0].x, screenCube[0].y);
	LineTo(screenCube[1].x, screenCube[1].y);
	LineTo(screenCube[2].x, screenCube[2].y);
	LineTo(screenCube[3].x, screenCube[3].y);
	LineTo(screenCube[0].x, screenCube[0].y);

	// back face
	if (color)
	{
		ForeColor(blueColor);
	}
	MoveTo(screenCube[4].x, screenCube[4].y);
	LineTo(screenCube[5].x, screenCube[5].y);
	LineTo(screenCube[6].x, screenCube[6].y);
	LineTo(screenCube[7].x, screenCube[7].y);
	LineTo(screenCube[4].x, screenCube[4].y);

	// connecting bits
	if (color)
	{
		ForeColor(greenColor);
	}
	MoveTo(screenCube[0].x, screenCube[0].y);
	LineTo(screenCube[4].x, screenCube[4].y);
	MoveTo(screenCube[1].x, screenCube[1].y);
	LineTo(screenCube[5].x, screenCube[5].y);
	MoveTo(screenCube[2].x, screenCube[2].y);
	LineTo(screenCube[6].x, screenCube[6].y);
	MoveTo(screenCube[3].x, screenCube[3].y);
	LineTo(screenCube[7].x, screenCube[7].y);
}

// render a face, filled in (face numbers 0 to 5)
void Cube::solidFace(short face, Boolean one_bit)
{
	//	short j;
	PolyHandle poly = OpenPoly();
	MoveTo(screenCube[faces[face][0]].x, screenCube[faces[face][0]].y);
	LineTo(screenCube[faces[face][1]].x, screenCube[faces[face][1]].y);
	if (!one_bit)
	{
		// colour
		LineTo(screenCube[faces[face][2]].x, screenCube[faces[face][2]].y);
		LineTo(screenCube[faces[face][3]].x, screenCube[faces[face][3]].y);
	}
	else
	{
		// black and white
		LineTo(screenCube[faces[face][3]].x, screenCube[faces[face][3]].y);
		LineTo(screenCube[faces[face][2]].x, screenCube[faces[face][2]].y);
	}
	LineTo(screenCube[faces[face][4]].x, screenCube[faces[face][4]].y);
	//	for (j = 1; j < 5; j++)
	//	{
	//		LineTo(screenCube[faces[face][j]].x, screenCube[faces[face][j]].y);
	//	}
	ClosePoly();
	PaintPoly(poly);
	KillPoly(poly);
}

// render the cube filled cube
void Cube::solidCube(Boolean color, Boolean one_bit)
{
	// we need to depth sort to try and get rendering right
	// only need to draw the closest 3 sides?
	short i, j;
	// just need one vector3 to use for calculation, then store the result in averageDepths
	Vector3 faceCenter;
	float averageDepths[6] = {0, 0, 0, 0, 0, 0};
	short indexes[6] = {0, 1, 2, 3, 4, 5}; // the faces (to be re-ordered)

	for (i = 0; i < 6; i++)
	{
		faceCenter.x = 0;
		faceCenter.y = 0;
		faceCenter.z = 0;
		for (j = 1; j < 5; j++)
		{
			// faceCenter = faceCenter.add(screenCube[faces[i][j]]);
			faceCenter = faceCenter.add(cube[faces[i][j]]);
		}
		// averageDepths[i] = faceCenter.scale(0.25).z;
		faceCenter = faceCenter.scale(0.25).add(centre);

		// distance between faceCenter location and camera location
		// camera is at (0, 0, 1000)
		averageDepths[i] = 1000 - faceCenter.z;
		averageDepths[i] = faceCenter.x * faceCenter.x +
						   faceCenter.y * faceCenter.y +
						   averageDepths[i] * averageDepths[i];
	}

	bubbleSort(averageDepths, indexes, 6);

	// technically only 3 are required straight on or if distances are perfect
	// but if off axis rendering with a cludge 4 faces is more robust and 6 is best
	for (i = 3; i < 6; i++)
	{
		if (color)
		{
			ForeColor(colors[indexes[i]]);
		}
		solidFace(indexes[i], one_bit);
	}
}

// identify & update the region on screen that the cube is using
void Cube::calculateBounds()
{
	short i;
	leftBound = 1000, rightBound = 0, upperBound = 1000, lowerBound = 0;

	for (i = 0; i < 8; i++)
	{
		if (screenCube[i].x < leftBound)
			leftBound = screenCube[i].x;
		if (screenCube[i].x > rightBound)
			rightBound = screenCube[i].x;

		if (screenCube[i].y < upperBound)
			upperBound = screenCube[i].y;
		if (screenCube[i].y > lowerBound)
			lowerBound = screenCube[i].y;
	}
}

// convert bounds to Region, the cube will be somwhere within
void Cube::roughBounds(RgnHandle rgn, short xRes, short yRes)
{
	// this might want a little padding when the cube is spinning fast?
	// calculateBounds() must be called earlier in the rendering pipeline
	SetRectRgn(rgn, leftBound, upperBound, rightBound, lowerBound);
}

// distance from the camera to cube
float Cube::getZ()
{
	return centre.z;
}

// distance to another cube
float Cube::dist(Cube *other)
{
	// d = x - y
	Vector3 d = centre.subtract(other->centre);
	// s = sum(d .^ 2)
	float s = d.dotProduct(d);
	s = sqrt(s);
	// take into account 'radius' of the cube
	// diameter * sqrt(2) to make it circular
	// divide by two to make it radius
	// simplify and just multiply by 0.7071
	s -= 0.7071 * size;
	s -= 0.7071 * other->size;
	if (s < 0)
	{
		s = 0;
	}
	return s;
}
