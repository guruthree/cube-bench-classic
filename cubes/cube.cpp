#include "vectormath2.h"
#include "bubblesort.h"
#include "cube.h"

static const Vector3 Cube::verticies[8] = {
    {-0.5f, -0.5f,  0.5f},
    { 0.5f, -0.5f,  0.5f},
    { 0.5f,  0.5f,  0.5f},
    {-0.5f,  0.5f,  0.5f},
    {-0.5f, -0.5f, -0.5f},
    { 0.5f, -0.5f, -0.5f},
    { 0.5f,  0.5f, -0.5f},
    {-0.5f,  0.5f, -0.5f}};


static const int Cube::faces[6][5] = {
    {0, 1, 2, 3, 0},
    {4, 5, 6, 7, 4},
    {0, 4, 5, 1, 0},
    {0, 4, 7, 3, 0},
    {2, 6, 7, 3, 2},
    {2, 6, 5, 1, 2}};

static const long Cube::colors[6] = {redColor, blueColor, greenColor, magentaColor, cyanColor, yellowColor};

void Cube::Cube(float size)
{
    // fill up cube just to make sure we don't accidentally stuff anything up
    int i;
    for (i = 0; i < 8; i++)
    {
        this.cube[i] = Cube.verticies[i];
    }
    this.updateSize(size);
}

void Cube::increaseSize()
{
    this.updateSize(this.size + 10);
}

void Cube::decreaseSize()
{
    if (this.size > 10)
        this.updateSize(this.size - 10);
}

void Cube::updateSize(float size)
{
    this.size = size;
    this.scale(size);
    this.cubeCircleSize = size * 80.0f / (size/16.0f + 40.0f); // from this.calculate()
}

// set the size of cube by copying from base cube
void Cube::scale(float scale)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        this.cube[i] = this.cube[i].scale(scale);
    }
}

void Cube::translate(Vector3 offset)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        this.cube[i] = this.cube[i].add(offset);
    }
}

// rotate the cube
void Cube::rotate()
{
    int i;
    Matrix3 rot = Matrix3::getRotationMatrix(dxangle, dyangle, dzangle);
    for (i = 0; i < 8; i++)
    {
        this.cube[i] = rot.preMultiply(this.cube[i]);
    }
}

// calculate where the cube is in screen space
void Cube::preCalculate(int xRes, int yRes)
{
    int i;
    Matrix3 rot = Matrix3::getRotationMatrix(xangle, yangle, zangle);
    for (i = 0; i < 8; i++)
    {
        this.screenCube[i] = this.cube[i].scale(40.0f / (-this.cube[i].z / 8.0f + 40.0f));
        this.screenCube[i].x = this.screenCube[i].x + xRes / 2;
        this.screenCube[i].y = this.screenCube[i].y + yRes / 2;
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
    MoveTo(this.screenCube[0].x, this.screenCube[0].y);
    LineTo(this.screenCube[1].x, this.screenCube[1].y);
    LineTo(this.screenCube[2].x, this.screenCube[2].y);
    LineTo(this.screenCube[3].x, this.screenCube[3].y);
    LineTo(this.screenCube[0].x, this.screenCube[0].y);

    // back face
    if (color)
    {
        ForeColor(blueColor);
    }
    MoveTo(this.screenCube[4].x, this.screenCube[4].y);
    LineTo(this.screenCube[5].x, this.screenCube[5].y);
    LineTo(this.screenCube[6].x, this.screenCube[6].y);
    LineTo(this.screenCube[7].x, this.screenCube[7].y);
    LineTo(this.screenCube[4].x, this.screenCube[4].y);
    
    // connecting bits
    if (color)
    {
        ForeColor(greenColor);
    }
    MoveTo(this.screenCube[0].x, this.screenCube[0].y);
    LineTo(this.screenCube[4].x, this.screenCube[4].y);
    MoveTo(this.screenCube[1].x, this.screenCube[1].y);
    LineTo(this.screenCube[5].x, this.screenCube[5].y);
    MoveTo(this.screenCube[2].x, this.screenCube[2].y);
    LineTo(this.screenCube[6].x, this.screenCube[6].y);
    MoveTo(this.screenCube[3].x, this.screenCube[3].y);
    LineTo(this.screenCube[7].x, this.screenCube[7].y);
}

// render a face, filled in (face 0 to 5)
void Cube::solidFace(int face)
{
    int j;
    PolyHandle poly = OpenPoly();
    MoveTo(this.screenCube[Cube.faces[face][0]].x, this.screenCube[Cube.faces[face][0]].y);
    for (j = 1; j < 5; j++)
    {
        LineTo(this.screenCube[Cube.faces[face][j]].x, this.screenCube[Cube.faces[face][j]].y);
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
            faceCenter = faceCenter.add(this.screenCube[Cube.faces[i][j]]);
        }
        averageDepths[i] = faceCenter.scale(0.25).z;
    }
    
    bubbleSort(averageDepths, indexes, 6);

    for (i = 3; i < 6; i++)
    {
        if (color)
        {
            ForeColor(Cube.colors[indexes[i]]);
        }
        this.solidFace(indexes[i]);
    }
}

// identify the region on screen that the cube is using
void Cube::bounds(RgnHandle rgn)
{
    int i;
    int left = 1000, right = 0, top = 1000, bottom = 0;

    for (i = 0; i < 8; i++)
    {
        if (this.cube[i].x < left) left = this.cube[i].x;
        if (this.cube[i].x > right) right = this.cube[i].x;
            
        if (this.cube[i].y < top) top = this.cube[i].y;
        if (this.cube[i].y > bottom) bottom = this.cube[i].y;
    }

    // this isn't enough when the cube is spinning fast
    SetRectRgn(rgn, left-3, top-3, right+3, bottom+3);
}

// the cube will be somwhere in a circle of cubeCircleSize
void Cube::roughBounds(RgnHandle rgn, int xRes, int yRes)
{
    SetRectRgn(rgn, xRes/2-this.cubeCircleSize, yRes/2-this.cubeCircleSize, xRes/2+this.cubeCircleSize, yRes/2+this.cubeCircleSize);
}

