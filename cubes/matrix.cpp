/*
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022,2024 guruthree, Blayzeing
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

#include "vectormath2.h"
#include "matrix.h"

Matrix3 Matrix3::multiply(Matrix3 m)
{
	Matrix3 output = {m.ii * ii + m.ji * ij + m.ki * ik, m.ij * ii + m.jj * ij + m.kj * ik, m.ik * ii + m.jk * ij + m.kk * ik,
					  m.ii * ji + m.ji * jj + m.ki * jk, m.ij * ji + m.jj * jj + m.kj * jk, m.ik * ji + m.jk * jj + m.kk * jk,
					  m.ii * ki + m.ji * kj + m.ki * kk, m.ij * ki + m.jj * kj + m.kj * kk, m.ik * ki + m.jk * kj + m.kk * kk};
	return output;
}

Vector3 Matrix3::preMultiply(Vector3 v)
{
	return Vector3(v.x * ii + v.y * ij + v.z * ik,
				   v.x * ji + v.y * jj + v.z * jk,
				   v.x * ki + v.y * kj + v.z * kk);
}

// rotation matrix, angles about rotate(x, y, z)
// note angles are in RADIANS
Matrix3 Matrix3::getRotationMatrix(float alpha, float beta, float gamma)
{
	Matrix3 Rz = {cos(gamma), -sin(gamma), 0,
				  sin(gamma), cos(gamma), 0,
				  0, 0, 1};
	Matrix3 Ry = {cos(beta), 0, sin(beta),
				  0, 1, 0,
				  -sin(beta), 0, cos(beta)};
	Matrix3 Rx = {1, 0, 0,
				  0, cos(alpha), -sin(alpha),
				  0, sin(alpha), cos(alpha)};
	return Rz.multiply(Ry.multiply(Rx));
}

Matrix3 Matrix3::getRotationMatrix(Vector3 angles)
{
	return Matrix3::getRotationMatrix(angles.x, angles.y, angles.z);
}
