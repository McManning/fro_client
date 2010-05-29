
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
