
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


#ifndef _SOUNDMANAGER_H_
#define _SOUNDMANAGER_H_

#include "../Common.h"

#define MAX_VOLUME 128

struct wavsample
{
	wavsample()
	{
		data = NULL;
		dpos = dlen = 0;
		playCount = 1;
		strength = 700;
	};
	
	wavsample(wavsample* from)
	{
		if (from)
		{
			data = from->data;
			dlen = from->dlen;
			dpos = 0; //don't copy position in a new instance
			id = from->id;
			playCount = from->playCount;
			position = from->position;
			strength = from->strength;
		}
	};
	
    void *data;
    int dpos; //position in data
    int dlen; //data length
	int playCount;
	string id;
	
	//point on a 2D plane, used to create the sense of distance from a center
	point2d position; 
	int strength; //how far we can reach
};

/*	Single manager that loads and maintains playback of all sounds
	TODO: A REAL audio playback engine with the following:
		-Multiple format playback (mp3, aac, wav, ogg, etc)
		-Preloading 
		-3D Sound!!! (Or at least, 2D)
*/
class SoundManager 
{
  public:
	SoundManager();
	~SoundManager();

	bool Initialize();
	bool Play(string id, int playCount = 1, point2d position = point2d(), int strength = 700); //I like 700, no other reason
	bool Load(string id, string filename);
	void Unload(string id);
	void Flush();
	void Mix(void *stream, int len);
	
	void SetVolume(byte vol) 
	{
		mVolume = (vol > MAX_VOLUME) ? MAX_VOLUME : vol; 
	};

	byte GetVolume() {
		return mVolume;
	};
	
	std::vector<wavsample*> mLoadedSamples;
	std::vector<wavsample*> mPlayingSamples;

	byte mVolume; //0->MAX_VOLUME, 0 being off. 
	bool mInitialized; //true if the engine we're wrapping is working successfully
	
	//To create 3D sound, we need an origin to use to offset other audio
	//In a case of a game, this would be player position in a 2D world.
	point2d mListenerPosition; 
};

extern SoundManager* sound;

#endif //_SOUNDMANAGER_H_
