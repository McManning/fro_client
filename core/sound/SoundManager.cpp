
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


#include "SoundManager.h"
#include "../widgets/Console.h" //for debugging
#include <SDL/SDL.h>

SoundManager* sound;

void callback_wavMixer(void *unused, Uint8 *stream, int len) 
{ 
	ASSERT(sound);
	sound->Mix(stream, len);
}

SoundManager::SoundManager() 
{
	mInitialized = false;
	mVolume = 0;
	
	sound = this;
}

SoundManager::~SoundManager() 
{
	Flush();
	if (mInitialized)
		SDL_CloseAudio();
	sound = NULL;
}
	
bool SoundManager::Initialize() 
{
	SDL_AudioSpec fmt;

	// Set 16-bit stereo audio at 22Khz
	fmt.freq = 22050;
	fmt.format = AUDIO_S16;
	fmt.channels = 2; //1 = mono, 2 = stereo
	fmt.samples = 512; //don't need a large buffer
	fmt.callback = callback_wavMixer;
	fmt.userdata = NULL;

	//Open the audio device and start playing sound! 
	if ( SDL_OpenAudio(&fmt, NULL) < 0 ) 
	{
		printf("Unable to initialize audio: %s\n", SDL_GetError());
		return false;
	}
	
	SDL_PauseAudio(0); //start audio playing (if there is anything)
	
	mInitialized = true;
	return true;
}

bool SoundManager::Play(string id, int playCount, point2d position, int strength)
{
	for (int i = 0; i < mLoadedSamples.size(); i++)
	{
		if (mLoadedSamples.at(i)->id == id)
		{
			//found, copy into playing samples
			wavsample* wav = new wavsample(mLoadedSamples.at(i));
			wav->playCount = playCount;
			wav->position = position;
			wav->strength = strength;

			//lock, add to list of samples to be mixed, unlock.
			SDL_LockAudio();

			mPlayingSamples.push_back(wav);

			SDL_UnlockAudio();
	
			return true;
		}
	}

	return false; //not found
}
	
bool SoundManager::Load(string id, string filename)
{
	if (!mInitialized) return false; //Ignore below if our engine isn't even initialized
	
	PRINT("Loading sample " + id + " from " + filename);
	
	Unload(id);

	SDL_AudioSpec wave;
	Uint8 *data;
	Uint32 dlen;
	SDL_AudioCVT cvt;
	
	//Load the sound file and convert it to 16-bit stereo at 22kHz 
	if (SDL_LoadWAV(filename.c_str(), &wave, &data, &dlen) == NULL) 
	{
		WARNING("Load Fail [" + filename + "]: " + SDL_GetError());
		return false;
	}
	
	PRINT("Building CVT");
	SDL_BuildAudioCVT(&cvt, wave.format, wave.channels, wave.freq,
						AUDIO_S16, 2, 22050);
	cvt.buf = (Uint8*)malloc(dlen * cvt.len_mult);
	memcpy(cvt.buf, data, dlen);
	cvt.len = dlen;
	SDL_ConvertAudio(&cvt);
	SDL_FreeWAV(data);

	PRINT("Adding");
	//add it to the list 
	wavsample* sample = new wavsample;
	sample->id = id;
	sample->data = cvt.buf;
	sample->dlen = cvt.len_cvt;
	sample->dpos = 0;
	mLoadedSamples.push_back(sample);

	PRINTF("Added. Len:%i Data:%p\n", sample->dlen, sample->data);

	return true;
}

void SoundManager::Unload(string id)
{
	int i;
	for ( i = 0; i < mLoadedSamples.size(); i++ )
	{
		if (mLoadedSamples.at(i)->id == id)
		{
			if (mLoadedSamples.at(i)->data)
				free(mLoadedSamples.at(i)->data);
			delete mLoadedSamples.at(i);
			
			mLoadedSamples.erase(mLoadedSamples.begin() + i);
			break;
		}
	}
	
	for ( i = 0; i < mPlayingSamples.size(); i++ )
	{
		if (mPlayingSamples.at(i)->id == id)
		{
			//lock, delete, erase, unlock. 
			SDL_LockAudio();
			delete mPlayingSamples.at(i);
			mPlayingSamples.erase(mPlayingSamples.begin() + i);
			SDL_UnlockAudio();
			break;
		}
	}

}

void SoundManager::Flush()
{
	int i;
	for ( i = 0; i < mLoadedSamples.size(); i++ )
	{
		if (mLoadedSamples.at(i)->data)
			free(mLoadedSamples.at(i)->data);
		delete mLoadedSamples.at(i);
	}
	
	SDL_LockAudio();
	for ( i = 0; i < mPlayingSamples.size(); i++ )
	{
		delete mPlayingSamples.at(i);
	}
	SDL_UnlockAudio();
	
	mLoadedSamples.clear();
	mPlayingSamples.clear();
}

//Careful, this is called from SDLs audio threads
void SoundManager::Mix(void *stream, int len)
{
    Uint32 amount; 
	double volume;
	double distance;
		
	//go through the wave list
    for ( int i = 0; i < mPlayingSamples.size(); i++ )
	{
        amount = (mPlayingSamples.at(i)->dlen - mPlayingSamples.at(i)->dpos);
        if ( amount > len ) //if we got more shit than room, cut down
		{
            amount = len;
        }

		/*
			Volume should depend on distance from a point. Each sample should be given a point2d to test against 
			our players location. Grab a # indicating distance, then reduce the volume depending on that distance.
			The sound should be heard at the same distance even if global volume is lower.. so it needs to factor in properly.
			If point is 0,0, the sound will be played at full volume. 
			
			wVol = d * (mVol / maxDistance) 
				Where 	d = distance between the two points
						mVol = max volume (in our case 128)
						maxDistance = maximum auditory distance from our point
			(wVol / mVol) * volume
				Where	wVol = wave volume (from above)
						mVol = max volume
						volume = current volume (0 to mVol)
		*/		

		PRINTF("sound:%i,%i\n",
					mPlayingSamples.at(i)->position.x,
					mPlayingSamples.at(i)->position.y);

		//if we don't care about distance, just play normally
		if (isDefaultPoint2d(mPlayingSamples.at(i)->position))
		{
			volume = (double)mVolume;
		}
		else //factor in distance from our player
		{
			distance = getDistance(mListenerPosition, mPlayingSamples.at(i)->position);	

			PRINTF("Listener:%i,%i\n",
					mListenerPosition.x,
					mListenerPosition.y);
			PRINTF("DIST:%f -> %i\n", distance, (int)distance);
		
			if (distance < (double)mPlayingSamples.at(i)->strength)
			{
				volume = (double)MAX_VOLUME - (distance / ((double)mPlayingSamples.at(i)->strength / (double)MAX_VOLUME));
				PRINTF("wVol:%f\n", volume);
				volume = (volume / MAX_VOLUME) * (double)mVolume;
				PRINTF("vol:%f\n", volume);
			}
			else
			{
				volume = 0.0;
			}
		}
		
		if (volume > 0.0)
		{
			//splice into the stream
	        SDL_MixAudio((Uint8*)stream, &((Uint8*)(mPlayingSamples.at(i)->data))[mPlayingSamples.at(i)->dpos], 
						amount, 
						(int)volume); 
		}
		
        mPlayingSamples.at(i)->dpos += amount;
		
		//check for a completed playback and erase
		if (mPlayingSamples.at(i)->dpos >= mPlayingSamples.at(i)->dlen)
		{
			mPlayingSamples.at(i)->dpos = 0;
			if (mPlayingSamples.at(i)->playCount != -1) //if we're not looping forever
			{
				mPlayingSamples.at(i)->playCount--;
				if (mPlayingSamples.at(i)->playCount < 1) //played it enough, we're done.
				{
					delete mPlayingSamples.at(i);
					mPlayingSamples.erase(mPlayingSamples.begin() + i);
					i--;
				}
			}
		}
    }
}
