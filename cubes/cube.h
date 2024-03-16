#include "vectormath2.h"
#include "bubblesort.h"

#ifndef CUBE
#define CUBE

class Cube
{
    private:
        // location
        Vector3 cube[8];
        // location in screen coordiantes
        Vector3 screenCube[8];
        float size;
        int cubeCircleSize;

    public:
        // unit cube
        static const Vector3 verticies[8];
        // mapping of vertices to make quads (polys) for rendering
        static const int faces[6][5];
        // colors of the faces
	    static const long colors[6];
	    
        // rotation speed
        float dxangle = 0.01;
        float dyangle = 0.005;
        float dzangle = 0.01;

        void Cube(float size);

        void increaseSize();

        void decreaseSize();

        void updateSize(float size);

        // set the size of cube by copying from base cube
        void scale(float scale);

        void translate(Vector3 offset);

        // rotate the cube
        void rotate();

        // calculate where the cube is in screen space
        void preCalculate(int xRes, int yRes);

        // render the cube (wireframe)
        void draw(Boolean color);

        // render a face, filled in (face 0 to 5)
        void solidFace(int face);

        // render the cube filled cube
        // void solidCube(Vector3 *rotatedCube, int faces[][5], Boolean color, long *colors)
        void solidCube(Boolean color);

        // identify the region on screen that the cube is using
        void bounds(RgnHandle rgn);

};

#endif