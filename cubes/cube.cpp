#include "vectormath2.h"
#include "bubblesort.h"
#include "cube.h"

const Vector3 Cube::verticies[8] = {
    {-0.5f, -0.5f,  0.5f},
    { 0.5f, -0.5f,  0.5f},
    { 0.5f,  0.5f,  0.5f},
    {-0.5f,  0.5f,  0.5f},
    {-0.5f, -0.5f, -0.5f},
    { 0.5f, -0.5f, -0.5f},
    { 0.5f,  0.5f, -0.5f},
    {-0.5f,  0.5f, -0.5f}};


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
    centre.z = 0;
    size = 1;
    dxangle = 0.01f;
    dyangle = 0.005f;
    dzangle = 0.01f;
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
    cubeCircleSize = size * 80.0f / (size/16.0f + 40.0f); // from this->calculate()
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
    int i;
    for (i = 0; i < 8; i++)
    {
        cube[i] = cube[i].add(offset);
    }
}

// rotate the cube
void Cube::rotate(float dxangle, float dyangle, float dzangle)
{
    int i;
    Matrix3 rot = Matrix3::getRotationMatrix(dxangle, dyangle, dzangle);
    for (i = 0; i < 8; i++)
    {
        cube[i] = rot.preMultiply(cube[i]);
    }
}

void Cube::autoRotate()
{
    rotate(dxangle, dyangle, dzangle);
}

// calculate where the cube is in screen space
void Cube::preCalculate(int xRes, int yRes)
{
    int i;
    Matrix3 rot = Matrix3::getRotationMatrix(dxangle, dyangle, dzangle);
    for (i = 0; i < 8; i++)
    {
        screenCube[i] = cube[i].add(centre).scale(40.0f / 
        	(-cube[i].z / 8.0f + 40.0f));
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
void Cube::bounds(RgnHandle rgn)
{
    int i;
    int left = 1000, right = 0, top = 1000, bottom = 0;

    for (i = 0; i < 8; i++)
    {
        if (cube[i].x < left) left = cube[i].x;
        if (cube[i].x > right) right = cube[i].x;
            
        if (cube[i].y < top) top = cube[i].y;
        if (cube[i].y > bottom) bottom = cube[i].y;
    }

    // this isn't enough when the cube is spinning fast
    SetRectRgn(rgn, left-3, top-3, right+3, bottom+3);
}

// the cube will be somwhere in a circle of cubeCircleSize
void Cube::roughBounds(RgnHandle rgn, int xRes, int yRes)
{
    SetRectRgn(rgn,
    	xRes/2-cubeCircleSize, 
    	yRes/2-cubeCircleSize, 
    	xRes/2+cubeCircleSize, 
    	yRes/2+cubeCircleSize);
}

