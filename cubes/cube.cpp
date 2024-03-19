#include "vectormath2.h"
#include "matrix.h"
#include "bubblesort.h"
#include "cube.h"

const Vector3 Cube::verticies[8] = {
    Vector3(-0.5f, -0.5f,  0.5f),
    Vector3( 0.5f, -0.5f,  0.5f),
    Vector3( 0.5f,  0.5f,  0.5f),
    Vector3(-0.5f,  0.5f,  0.5f),
    Vector3(-0.5f, -0.5f, -0.5f),
    Vector3( 0.5f, -0.5f, -0.5f),
    Vector3( 0.5f,  0.5f, -0.5f),
    Vector3(-0.5f,  0.5f, -0.5f)};


const int Cube::faces[6][5] = {
    {0, 1, 2, 3, 0},
    {4, 5, 6, 7, 4},
    {0, 4, 5, 1, 0},
    {0, 4, 7, 3, 0},
    {2, 6, 7, 3, 2},
    {2, 6, 5, 1, 2}};

const long Cube::colors[6] = {redColor, blueColor, greenColor, magentaColor, cyanColor, yellowColor};

Cube::Cube(float newSize)
{
    // fill up cube just to make sure we don't accidentally stuff anything up
    int i;
    for (i = 0; i < 8; i++)
    {
        cube[i] = verticies[i];
    }
    centre.x = 0;
    centre.y = 0;
    centre.z = 10;
    dangle.x = 0.01f;
    dangle.y = 0.005f;
    dangle.z = 0.01f;
    velocity.x = 1;
    velocity.y = 0.8;
    velocity.z = 0; //-1; // +ve is closer to the screen
    size = 1;
    updateSize(newSize);
}

void Cube::increaseSize()
{
    updateSize(size + 10);
}

void Cube::decreaseSize()
{
    if (size > 10)
        updateSize(size - 10);
}

void Cube::updateSize(float newSize)
{
    scale(newSize / size);
    size = newSize;
    cubeCircleSize = size * 80.0f / (size/16.0f + 40.0f); // from calculate()
}

// set the size of cube by copying from base cube
void Cube::scale(float scale)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        cube[i] = cube[i].scale(scale);
    }
}

void Cube::translate(Vector3 offset)
{
//    int i;
//    for (i = 0; i < 8; i++)
//    {
//        cube[i] = cube[i].add(offset);
//    }
	centre = centre.add(offset);
}

// rotate the cube
void Cube::rotate(Vector3 dangle)
{
    int i;
    Matrix3 rot = Matrix3::getRotationMatrix(dangle);
    for (i = 0; i < 8; i++)
    {
        cube[i] = rot.preMultiply(cube[i]);
    }
}

void Cube::autoRotate()
{
    rotate(dangle);
}

void Cube::autoTranslate()
{
    translate(velocity);
}

// calculate where the cube is in screen space
void Cube::preCalculate(int xRes, int yRes)
{
    int i;
    Matrix3 rot = Matrix3::getRotationMatrix(dangle);
    for (i = 0; i < 8; i++)
    {
        screenCube[i] = cube[i].add(centre);
        screenCube[i] = screenCube[i].scale(1000.0f / (-screenCube[i].z + 1000.0f));
//        screenCube[i] = screenCube[i].scale(2);
        
//        screenCube[i] = (cube[i].add(centre)).scale(40.0f / 
 //       	(-cube[i].z / 50.0f + 40.0f));
//        	(-cube[i].z / 8.0f + 40.0f));
        screenCube[i].x = screenCube[i].x + xRes / 2;
        screenCube[i].y = screenCube[i].y + yRes / 2;
    }
}

// render the cube (wireframe)
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

// render a face, filled in (face 0 to 5)
void Cube::solidFace(int face)
{
    int j;
    PolyHandle poly = OpenPoly();
    MoveTo(screenCube[faces[face][0]].x, screenCube[faces[face][0]].y);
    for (j = 1; j < 5; j++)
    {
        LineTo(screenCube[faces[face][j]].x, screenCube[faces[face][j]].y);
    }
    ClosePoly();
    PaintPoly(poly);
    KillPoly(poly);
}

// render the cube filled cube
// void solidCube(Vector3 *rotatedCube, int faces[][5], Boolean color, long *colors)
void Cube::solidCube(Boolean color)
{
    // we need to depth sort to try and get rendering right
    // only need to draw the closest 3 sides?
    int i, j;
    // just need one vector3 to use for calculation, then store the result in averageDepths
    Vector3 faceCenter;
    float averageDepths[6] = {0, 0, 0, 0, 0, 0};
    int indexes[6] = {0, 1, 2, 3, 4, 5}; // the faces
    
    for (i = 0; i < 6; i++)
    {
        faceCenter.x = 0;
        faceCenter.y = 0;
        faceCenter.z = 0;
        for (j = 1; j < 5; j++)
        {
            faceCenter = faceCenter.add(screenCube[faces[i][j]]);
        }
//        averageDepths[i] = faceCenter.scale(0.25).z; // only works face on
		averageDepths[i] = faceCenter.scale(0.25).z;
    }
    
    bubbleSort(averageDepths, indexes, 6);

    for (i = 3; i < 6; i++)
    {
        if (color)
        {
            ForeColor(colors[indexes[i]]);
        }
        solidFace(indexes[i]);
    }
}

// identify the region on screen that the cube is using
void Cube::calculateBounds()
{
    int i;
    leftBound = 1000, rightBound = 0, upperBound = 1000, lowerBound = 0;

    for (i = 0; i < 8; i++)
    {
        if (screenCube[i].x < leftBound) leftBound = screenCube[i].x;
        if (screenCube[i].x > rightBound) rightBound = screenCube[i].x;
            
        if (screenCube[i].y < upperBound) upperBound = screenCube[i].y;
        if (screenCube[i].y > lowerBound) lowerBound = screenCube[i].y;
    }

}

// the cube will be somwhere in a circle of cubeCircleSize
void Cube::roughBounds(RgnHandle rgn, int xRes, int yRes)
{
    SetRectRgn(rgn,
    	xRes/2-cubeCircleSize, 
    	yRes/2-cubeCircleSize, 
    	xRes/2+cubeCircleSize, 
    	yRes/2+cubeCircleSize);

    // a little padding for the cube is spinning fast?
    //SetRectRgn(rgn, left-3, top-3, right+3, bottom+3);
}

