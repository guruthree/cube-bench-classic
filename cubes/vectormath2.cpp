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

Vector3 Vector3::add(Vector3 v)
{
	return Vector3(x + v.x,
				   y + v.y,
				   z + v.z);
}

Vector3 Vector3::subtract(Vector3 v)
{
	return Vector3(x - v.x,
				   y - v.y,
				   z - v.z);
}

Vector3 Vector3::scale(float s)
{
	return Vector3(x * s,
				   y * s,
				   z * s);
}

Vector3 Vector3::scalarMultiply(Vector3 v)
{
	return Vector3(x * v.x,
				   y * v.y,
				   z * v.z);
}

float Vector3::dotProduct(Vector3 v)
{
	return x * v.x + y * v.y + z * v.z;
}
