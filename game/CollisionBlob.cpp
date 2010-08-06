
#include "CollisionBlob.h"
#include "../core/io/FileIO.h" // compressFile/decompressFile
#include "../core/ResourceManager.h"
#include "../core/Image.h"
#include "../core/widgets/Console.h"

/**
	@param x starting x position of a 16x16 square
	@param y starting y position of a 16x16 square
	@return true if the tile consists of more than 10% white pixels
*/
bool _isTileWhiteEnough(Image* img, int x, int y)
{
	int count = 0, iy, ix;
	color c;
	
	for (iy = y; iy < y + 16; ++iy)
	{
		for (ix = x; ix < x + 16; ++ix)
		{
			c = img->GetPixel(ix, iy);
			if (c.r == 255 && c.g == 255 && c.b == 255)
			{
				++count;
				if (count > (2.56 * 25))
					return true;
			}
		}
	}

	// Went through all pixels, not enough white to be conclusive.
	return false;
}

/**
	@param file The input image, all white pixels will be used to determine solidity
	@param printResults if 1, it will output an image consisting of the original image plus all collision rects drawn over it
	@return 1 if success, 0 otherwise
*/
int createCollisionBlob(string file, int printResults)
{
	string colfile = "saved/" + timestamp(true) + ".col";
	string luafile = "saved/" + timestamp(true) + ".lua";
	
	Image* img = resman->LoadImg(file);
	if (!img)
	{
		console->AddMessage("Failed to load image: " + file);
		return 0;
	}
	
	/*
		So how this works is..
		
		METHOD A:
			1. Go through all tiles, if it is white, write a 1 to the output file, otherwise write a 0
			2. Compress file.
		
		METHOD B (Preferred, integrates better with current systems)
			1. go through all tiles, left->right, and expand a rectangle over all white tiles
			2. Optimize rectangles (If too lazy to add optimizer, however, just have scanline-like rectangles)
			3. Compress file.
				This method can later be easily read into a dummy collision entity, or converted into lua code.
	*/
	
	FILE* fp = fopen(colfile.c_str(), "w");
	if (!fp)
	{
		console->AddMessage("Failed to create output: " + colfile);
		resman->Unload(img);
		return 0;
	}

	bool white;
	int x, y;
	rect r;
	r.w = 0;
	r.h = 16;
	for (y = 0; y < img->Height(); y+=16)
	{
		r.y = y;
		r.x = -1;
		for (x = 0; x < img->Width(); x+=16)
		{
			white = _isTileWhiteEnough(img, x, y);
			if (white)
			{
				if (r.x < 0) //start a new rect
				{
					r.x = x;
					r.w = 16;
				}
				else //expand our rect
				{
					r.w += 16;
				}
			}
			else // hit black, save previous rect
			{
				if (r.x > -1) // save rect
				{
					fwrite((void*)&r.x, sizeof(r.x), 1, fp);
					fwrite((void*)&r.y, sizeof(r.y), 1, fp);
					fwrite((void*)&r.w, sizeof(r.w), 1, fp);
					fwrite((void*)&r.h, sizeof(r.h), 1, fp);
				}
				r.x = -1; //tell it to start a new rect later
			}
		}
	}
	
	fclose(fp);

	if (printResults)
	{
		fp = fopen(colfile.c_str(), "r");
		if (fp)
		{
			while (true)
			{
				if (fread((void*)&r.x, sizeof(r.x), 1, fp) != 1)
					break;
				if (fread((void*)&r.y, sizeof(r.y), 1, fp) != 1)
					break;
				if (fread((void*)&r.w, sizeof(r.w), 1, fp) != 1)
					break;
				if (fread((void*)&r.h, sizeof(r.h), 1, fp) != 1)
					break;

				// Doodle rect to image
				img->DrawRect(r, color(255, 0, 0), false);
			}
			fclose(fp);
			
			// save a new image
			img->SavePNG(colfile + ".png");
			console->AddMessage("Saved debug image to " + colfile + ".png");
		}
	}

	// Package our final collision map
	string tmpFile = getTemporaryCacheFilename();
	compressFile(colfile, tmpFile);
	copyFile(tmpFile, colfile);
	removeFile(tmpFile);
	
	if (printResults) //hey, might as well print some lua code too!
	{
		convertCollisionBlobToLuaTable(colfile, luafile);
		console->AddMessage("Saved lua table to " + luafile);
	}
	
	console->AddMessage("Saved collision blob to " + colfile);
		
	resman->Unload(img);
	return 1;
}

/**
	Take a collision blob file and turn it into the collision array commonly used for 
	entities within lua map scripts
	
	@param colFile the collision file to load in
	@param luaFile the lua file to write to
	@return 1 if success, 0 otherwise.
*/

int convertCollisionBlobToLuaTable(string colFile, string luaFile)
{
	string tmpFile = getTemporaryCacheFilename();
	if (!decompressFile(colFile, tmpFile))
		return 0;
	
	FILE* fin = fopen(tmpFile.c_str(), "r");
	FILE* fout = fopen(luaFile.c_str(), "w");
	
	if (!fin || !fout)
	{
		removeFile(tmpFile);
		return 0;
	}
/*
	Output in the form (including tabbing): 
Collision = 
{
	x,y,w,h,
	x,y,w,h,
}
*/
	fprintf(fout, "Collision = \n{\n");
		
	rect r; // HACK: Using a rect so that we don't have to worry about the sizeof() being screwed up when rect is changed
	int c = 0;
	while (true)
	{
		if (fread((void*)&r.x, sizeof(r.x), 1, fin) != 1)
			break;
			
		if (c == 0)
			fprintf(fout, "\t");
		
		fprintf(fout, "%i,", r.x);
		
		++c;
		if (c > 3) //do a lil prettifying
		{
			fprintf(fout, "\n");
			c = 0;
		}
	}
	
	fprintf(fout, "}\n");
	
	fclose(fin);
	fclose(fout);
	removeFile(tmpFile);
	
	return 1;
}



