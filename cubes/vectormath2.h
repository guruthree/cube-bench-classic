/*
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 guruthree, Blayzeing
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

// thanks to @Blayzeing for his help with this!


#pragma once

#ifndef VECTORMATH
#define VECTORMATH

struct Vector3 {
    float x, y, z;
    Vector3 add(Vector3);
    Vector3 subtract(Vector3);
    Vector3 scale(float);
    Vector3 scalarMultiply(Vector3);
    float dotProduct(Vector3);
};

struct Matrix3{
    float ii, ij, ik,
          ji, jj, jk,
          ki, kj, kk;
    
    Matrix3 multiply(Matrix3);
    Vector3 preMultiply(Vector3);

    // Gets a rotation matrix
    static Matrix3 getRotationMatrix(float alpha, float beta, float gamma);

    // Gets a perspective scaling matrix, scaled along the z-axis
    // static Matrix3 getPerspMatrix(Vector3 surfacePos);

    // Gets a camera transform matrix
    /*static Matrix3 getCameraMaterix(Vector3 cameraPosition,
                                    Vector3 cameraRotation,
                                    float surfaceDistanceIntoZ);*/
};


#endif
