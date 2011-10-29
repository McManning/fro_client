
/*
 * Copyright (c) 2011 Chase McManning
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
 */


#ifndef _COLLISIONBLOB_H_
#define _COLLISIONBLOB_H_

#include "../core/Common.h"

/**
	@param file The input image, all white pixels will be used to determine solidity
	@param printResults if 1, it will output an image consisting of the original image plus all collision rects drawn over it
	@return 1 if success, 0 otherwise
*/
int createCollisionBlob(string file, int printResults);

/**
	Take a collision blob file and turn it into the collision array commonly used for 
	entities within lua map scripts
	
	@param colFile the collision file to load in
	@param luaFile the lua file to write to
	@return 1 if success, 0 otherwise.
*/

int convertCollisionBlobToLuaTable(string colFile, string luaFile);

#endif //_COLLISIONBLOB_H_
