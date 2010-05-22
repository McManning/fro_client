
#include "Image.h"
#include "ResourceManager.h"
#include "SDL/SDL_Misc.h"
#include "SDL/SDL_rotozoom.h"
#include "SDL/SDL_Image.h"
#include "widgets/Console.h"
#include "TimerManager.h"
#include "Screen.h"

/*	TODO: Something much more efficient, where it would only add a timer
	if the image is animated and has multiple frames. 
	
*/
uShort timer_imageThink(timer* t, uLong ms)
{
	Image* i = (Image*)t->userData;

	if (i->mPlaying)
	{
		t->interval = i->ForwardCurrentFrameset();
#ifdef OPTIMIZED
		Screen::Instance()->Update();
#endif
	}
		
	//if (ms > i->mNextFrameChange && i->mPlaying)	
		//i->mNextFrameChange = ms + i->ForwardCurrentFrameset();
	return TIMER_CONTINUE;
}

SDL_Image::SDL_Image()
{
	refCount = 1;
	state = NOIMAGE;
	format = IMG_FORMAT_UNKNOWN;
}

SDL_Image::~SDL_Image()
{
	if (refCount > 0) //can't actually stop them, but warn then.
	{
		WARNING("Deleting SDL_Image [" + filename + "] With " + its(refCount) + " References!");
	}

//	PRINT("~SDL_Image Freeing Framesets");

	int a, b;
	for (a = 0; a < framesets.size(); a++)
	{
//		PRINT("Next Frameset");
		for (b = 0; b < framesets.at(a).frames.size(); b++)
		{
//			PRINT("Freeing framesets.at(" + its(a) + ").frames.at(" + its(b) + ").surf = " + pts(framesets.at(a).frames.at(b).surf));
			if ( framesets.at(a).frames.at(b).surf )
				SDL_FreeSurface( framesets.at(a).frames.at(b).surf );

//			PRINT("Freeing framesets.at(" + its(a) + ").frames.at(" + its(b) + ").key = " + pts(framesets.at(a).frames.at(b).key));
			if ( framesets.at(a).frames.at(b).key )
				free ( framesets.at(a).frames.at(b).key );
				
//			PRINT("Moving On");
		}
	}

//	PRINT("~SDL_Image Done");
}

bool SDL_Image::Load(string file, string pass)
{
	IMG_File imgf;
	SDL_Surface* temp;
	bool result;
	
	state = LOADING;

	//TODO: load boltFile (and decrypt), then load an SDL RW handle from that memory, then pass to image loader

	PRINT("Loading " + file);
	if ( !IMG_Load(file.c_str(), &imgf) || !imgf.frames || imgf.count < 1 )
	{
		//TODO: Possible memory leak here?
		result = false;
	}
	else
	{
		color c;
		SDL_GetRGBA( SDL_GetPixel(imgf.frames[0].surf, 0, 0), imgf.frames[0].surf->format, 
						&c.r, &c.g, &c.b, &c.a );

		if (imgf.format == IMG_FORMAT_BMP 
			|| (imgf.format == IMG_FORMAT_PNG && c.a == ALPHA_OPAQUE))
				//&& imgf.frames[0].surf->format->BytesPerPixel < 4))
		{
			//pngs/bmps only have one frame, so we don't need to loop
			SDL_SetColorKey(imgf.frames[0].surf, SDL_SRCCOLORKEY | SDL_RLEACCEL, 
								SDL_GetPixel(imgf.frames[0].surf, 0, 0));
								
		//	if (alphaChannel)
				temp = SDL_DisplayFormatAlpha(imgf.frames[0].surf);
		//	else //Yes. It matters.
		//		temp = SDL_DisplayFormat(imgf.frames[0].surf);
		
			SDL_FreeSurface(imgf.frames[0].surf);
			imgf.frames[0].surf = temp;
		}
	
		result = resman->FramesToImage(this, imgf.frames, imgf.count, file);
		format = imgf.format;
		filename = file;
		password = pass;
	}
	
	state = (result) ? LOADED : BADIMAGE;

	return result;
}

void SDL_Image::PrintInfo() const
{
	printf("Image 0x%p Filename:%s RefCount:%i \n", this, filename.c_str(), refCount);
	for (uShort i = 0; i < framesets.size(); i++)
	{
		printf("\tFrameset[%i]: Key:%s Loop:%i\n", i, 
				framesets.at(i).key.c_str(), framesets.at(i).loop);
				
		for (uShort u = 0; u < framesets.at(i).frames.size(); u++)
		{
			printf("\t\tFrame: Surf:0x%p W:%i H:%i Delay:%i\n", 
					framesets.at(i).frames.at(u).surf,
					framesets.at(i).frames.at(u).surf->w,
					framesets.at(i).frames.at(u).surf->h,
					framesets.at(i).frames.at(u).delay
				);
		}
	}
	printf("----End Image----\n");
}

void SDL_Image::_copyFramesets(SDL_Image* dst, rect clip) const
{
	SDL_Rect r;
	r.x = clip.x;
	r.y = clip.y;
	r.w = clip.w;
	r.h = clip.h;

	for (int a = 0; a < dst->framesets.size(); a++)
	{
		for (int b = 0; b < dst->framesets.at(a).frames.size(); b++)
		{
			ASSERT(dst->framesets.at(a).frames.at(b).surf);
			
			if (r.w == 0)
				r.w = dst->framesets.at(a).frames.at(b).surf->w;
				
			if (r.h == 0)
				r.h = dst->framesets.at(a).frames.at(b).surf->h;
				
			dst->framesets.at(a).frames.at(b).surf 
				= SDL_CopySurface( dst->framesets.at(a).frames.at(b).surf, r );
		}
	}
}

SDL_Image* SDL_Image::Clone(rect clip) const
{
	SDL_Image* dst;
	
	ASSERT(state != LOADING);
	
	dst = new SDL_Image;

	dst->framesets = framesets;
	dst->filename = ""; //no longer the original
	dst->format = format;
	
	_copyFramesets(dst, clip);
	
	resman->mImages.push_back(dst);

	return dst;
}

int SDL_Image::CountFrames()
{
	int count = 0;
	for (int i = 0; i < framesets.size(); i++)
		count += framesets.at(i).frames.size();
	return count;
}

SDL_Frameset* SDL_Image::GetFrameset(string key)
{
	for (int i = 0; i < framesets.size(); i++)
	{
		if (framesets.at(i).key == key)
			return &framesets.at(i);
	}
	return NULL;
}

Image::Image()
{
	mImage = NULL;
	mAnimationTimer = NULL;
	mFramesetIndex = 0;
	mFrameIndex = 0;
	mNextFrameChange = 0;
	mPlaying = true;
}

Image::~Image()
{
	if (resman)
		resman->Unload(mImage); //if it no longer has any links to other Image's, will remove from memory.
	else
		delete mImage;
		
	if (mAnimationTimer)
		timers->Remove(mAnimationTimer);
}

SDL_Frameset* Image::Frameset() const
{
	if (mImage && mFramesetIndex < mImage->framesets.size())
		return &mImage->framesets.at(mFramesetIndex);

	return NULL;
}

SDL_Frame* Image::Frame() const
{
	SDL_Frameset* fs = Frameset();
	if (fs)
	{
		if (mFrameIndex < fs->frames.size())
			return &fs->frames.at(mFrameIndex);
	}
	
	return NULL;
}

SDL_Surface* Image::Surface() const
{
	SDL_Frame* f = Frame();
	if (f)
		return f->surf;
		
	return NULL;
}

SDL_Frameset* Image::GetFrameset(string key) const
{
	if (mImage)
		return mImage->GetFrameset(key);
	
	return NULL;
}

bool Image::SetFrameset(string key)
{
	SDL_Frameset* fs = Frameset();
	if (fs && fs->key == key) //don't set if we're already on it
		return true; 
	
	uShort i;
	if (mImage)
	{
		for (i = 0; i < mImage->framesets.size(); i++)
		{
			if (mImage->framesets.at(i).key == key)
			{
				PRINT("[Image::SetFrameset] Setting To Key " + key);
				mFramesetIndex = i;
				mFrameIndex = 0;
				Reset();
				return true;
			}
		}
	}
	
	PRINT("[Image::SetFrameset] Key " + key + " Not Found!");
	return false;
}

bool Image::IsOnLastFrame()
{
	SDL_Frameset* fs = Frameset();
	if (fs && Frame())
	{
		return (mFrameIndex == fs->frames.size() - 1);
	}
	
	return false;
}

//return delay to next change. ULONG_MAX if the current frame is null 
uLong Image::ForwardCurrentFrameset(bool forceLoop)
{
	SDL_Frameset* fs = Frameset();
	if (fs && Frame())
	{
		if (mFrameIndex == fs->frames.size() - 1)
		{
			if (fs->loop || forceLoop)
				mFrameIndex = 0;
		}
		else
		{
			mFrameIndex++;
		}
	}
	
	SDL_Frame* f = Frame();

	if (f)
		return f->delay;

	return ULONG_MAX;
}

void Image::Play()
{
	if (!mPlaying)
	{
		mPlaying = true;
		//TODO: Enable timer
	}
}

void Image::Stop()
{
	if (mPlaying)
	{
		mPlaying = false;
		//TODO: Disable timer
	}
}

void Image::Forward(bool forceLoop)
{
	Stop();
	ForwardCurrentFrameset(forceLoop);
}

void Image::Reset()
{
	mFrameIndex = 0;
	
	SDL_Frame* f = Frame();
	/*if (f)
		mNextFrameChange = SDL_GetTicks() + f->delay;
	else
		mNextFrameChange = SDL_GetTicks();*/
		
	if (mAnimationTimer)
	{
		mAnimationTimer->lastMs = SDL_GetTicks();
		mAnimationTimer->interval = (f) ? f->delay : 0;
	}
}

void Image::UpdateTimer()
{
	//we have more than one frame, assume this image is animated, add linked timer.
	if (mImage && mImage->CountFrames() > 1 && !mAnimationTimer)
	{
		mAnimationTimer = timers->Add("", 100, true, timer_imageThink, NULL, this);
	}	
}

void Image::ConvertToAlphaFormat()
{
	SDL_Surface* surf;
	
	if (mImage)
	{
		for (int a = 0; a < mImage->framesets.size(); a++)
		{
			for (int b = 0; b < mImage->framesets.at(a).frames.size(); b++)
			{
				surf = SDL_DisplayFormatAlpha( mImage->framesets.at(a).frames.at(b).surf );
				SDL_FreeSurface( mImage->framesets.at(a).frames.at(b).surf );
				mImage->framesets.at(a).frames.at(b).surf = surf;
			}
		}
	}
}

bool Image::ConvertToHorizontalAnimation(rect clip, uShort delay)
{	
	if (!mImage || mImage->framesets.empty()
		|| mImage->framesets.at(0).frames.empty()
		|| !mImage->framesets.at(0).frames.at(0).surf)
	{
		PRINT("[Image::CTHA] Bad Source");
		return false;
	}
	
	//if it's already animated, don't deal with this.
	if (mImage->CountFrames() > 1)
	{
		PRINT("[Image::CTHA] Not single frame image!");
		return false;
	}
	
	PRINT("[Image::CTHA] Converting");
	
	SDL_Surface* src = mImage->framesets.at(0).frames.at(0).surf;
	mImage->framesets.clear();
	
	SDL_Frame f;
	f.key = NULL;
	SDL_Frameset fs;
	
	//set common properties
	f.delay = delay;
	fs.loop = true;
	
	SDL_Rect r;
	r.x = clip.x;
	r.y = clip.y;
	r.w = clip.w;
	r.h = clip.h;
	
	//Copy each frame of this row into the frameset
	for (r.x = 0; r.x <= src->w - r.w; r.x += r.w)
	{
		f.surf = SDL_CopySurface( src, r );
		fs.frames.push_back(f);
	}
	mImage->framesets.push_back(fs);

	SDL_FreeSurface(src); //done with the original
	
	Play();

	UpdateTimer();
	
	PRINT("[Image::CTHA] Done");
	return true;
}

bool Image::ConvertToAvatarFormat(uShort w, uShort h, uShort delay, bool loopStand, bool loopSit)
{
	if (!mImage || mImage->state == SDL_Image::BADIMAGE || mImage->framesets.empty()
		|| mImage->framesets.at(0).frames.empty()
		|| !mImage->framesets.at(0).frames.at(0).surf)
	{
		PRINT("[ConvertToAvatarFormat] Bad Source");
		return false;
	}
	
	if (mImage->CountFrames() == 1)
	{
		if (w < 1) 
			w = Width();
			
		if (h < 1)
			h = Height();
			
		if (w > Width())
			w = Width();
			
		if (h > Height())
			h = Height();

		SDL_Surface* src = mImage->framesets.at(0).frames.at(0).surf;
		mImage->framesets.clear();
		
		uShort rowCounter = 0; //keep tabs on the row we're on
		uShort frameCounter = 0;
		SDL_Frame f;
		SDL_Frameset fs;
		
		//set common properties that won't get modified
		f.delay = delay;
		f.key = NULL;
	
		SDL_Rect r;
		r.w = w;
		r.h = h;
	
		bool quit = false;
		//Go through all rows and add a frameset for each
		for (r.y = 0; r.y <= src->h - h && frameCounter < MAX_AVATAR_FRAMES; r.y += h)
		{
			frameCounter++;
			
			fs.frames.clear();
			
			switch (rowCounter) //determine set id by which row we're on
			{
				case 0: fs.key = "_move_2"; break;
				case 1: fs.key = "_move_8"; break;
				case 2: fs.key = "_move_4"; break;
				case 3: fs.key = "_move_6"; break;
				
				case 4: fs.key = "_sit_2"; break;
				case 5: fs.key = "_sit_8"; break; //Drm does not use Kyat-formatted _JUMP_2, _LEFT_2, _RIGHT_2
				case 6: fs.key = "_sit_4"; break;
				case 7: fs.key = "_sit_6"; break;

				default: quit = true; break;
			}
			
			if (quit)
				break;
			
			if (rowCounter < 4)
				fs.loop = loopStand;
			else if (rowCounter < 8)
				fs.loop = loopSit;
			else
				fs.loop = false;
	
			PRINTF("Y(%i) Key(%s) Loop(%i)\n", r.y, fs.key.c_str(), fs.loop);
			
			for (r.x = 0; r.x <= src->w - w; r.x += w)
			{
				PRINTF("\tr(%i,%i,%i,%i) sr(%ix%i)\n", r.x, r.y, r.w, r.h, src->w, src->h);
				//Copy each frame of this row into the frameset
				f.surf = SDL_CopySurface( src, r );
				fs.frames.push_back(f);
			}
			mImage->framesets.push_back(fs);
			rowCounter++;
		}
		SDL_FreeSurface(src); //done with the original
		
		UpdateTimer();
	}
	else //more than one frame
	{
		//defaults for gif/mng behavior
	/*	loopStand = true;
		loopSit = false;
*/
		//Need to still set loopSit/loopStand properties for mngs
		SDL_Frameset* pfs;
		
		//sit loops
		pfs = GetFrameset("_sit_2");
		if (pfs) pfs->loop = loopSit;
		pfs = GetFrameset("_sit_4");
		if (pfs) pfs->loop = loopSit;
		pfs = GetFrameset("_sit_6");
		if (pfs) pfs->loop = loopSit;
		pfs = GetFrameset("_sit_8");
		if (pfs) pfs->loop = loopSit;
		
		//kyat conversions
		pfs = GetFrameset("_left_2");
		if (pfs) pfs->loop = loopSit;
		pfs = GetFrameset("_right_2");
		if (pfs) pfs->loop = loopSit;
		pfs = GetFrameset("_jump_2");
		if (pfs) pfs->loop = loopSit;
		
		//stand loops
		pfs = GetFrameset("_stop_2");
		if (pfs) pfs->loop = loopStand;
		pfs = GetFrameset("_stop_4");
		if (pfs) pfs->loop = loopStand;
		pfs = GetFrameset("_stop_6");
		if (pfs) pfs->loop = loopStand;
		pfs = GetFrameset("_stop_8");
		if (pfs) pfs->loop = loopStand;
		
		//move loops
		pfs = GetFrameset("_move_2");
		if (pfs) pfs->loop = true;
		pfs = GetFrameset("_move_4");
		if (pfs) pfs->loop = true;
		pfs = GetFrameset("_move_6");
		if (pfs) pfs->loop = true;
		pfs = GetFrameset("_move_8");
		if (pfs) pfs->loop = true;
	}
	
	return true;
}

Image* Image::Clone(bool fullClone, rect clip) const
{
	Image* dst = new Image;
	
	if (mImage)
	{
		//if we're copying a section, or want a complete unique copy, clone SDL_Image
		if (fullClone || !isDefaultRect(clip))
		{
			dst->mImage = mImage->Clone(clip);
		}
		else //just reference the same SDL_Image
		{
			dst->mImage = mImage;
			mImage->refCount++;
		}
	}
	
	dst->mFramesetIndex = mFramesetIndex;
	dst->mFrameIndex = mFrameIndex;
	dst->mNextFrameChange = mNextFrameChange;
	dst->mPlaying = mPlaying;
	
	dst->UpdateTimer();
	
	return dst;
}

uShort Image::Width() const
{
	SDL_Surface* surf = Surface();
	if (surf)
		return surf->w;
		
	return 0;
}

uShort Image::Height() const
{
	SDL_Surface* surf = Surface();
	if (surf)
		return surf->h;
		
	return 0;
}

uShort Image::MaxWidth() const
{
	uShort w = 0;
	if (mImage)
	{
		for (int a = 0; a < mImage->framesets.size(); a++)
		{
			for (int b = 0; b < mImage->framesets.at(a).frames.size(); b++)
			{
				if (mImage->framesets.at(a).frames.at(b).surf->w > w)
					w = mImage->framesets.at(a).frames.at(b).surf->w;
			}
		}
	}
	return w;
}

uShort Image::MaxHeight() const
{
	uShort h = 0;
	if (mImage)
	{
		for (int a = 0; a < mImage->framesets.size(); a++)
		{
			for (int b = 0; b < mImage->framesets.at(a).frames.size(); b++)
			{
				if (mImage->framesets.at(a).frames.at(b).surf->h > h)
					h = mImage->framesets.at(a).frames.at(b).surf->h;
			}
		}
	}
	return h;	
}

bool Image::Render(Image* dst, sShort x, sShort y, rect clip)
{
	return Render(dst->Surface(), x, y, clip);
}

bool Image::Render(SDL_Surface* dst, sShort x, sShort y, rect clip)
{
	SDL_Surface* src = Surface();

	if (!src || !dst) 
	{
		WARNING("Invalid Src:" + pts(src) + " or Dst:" + pts(dst) + " File:" + Filename());
		return false;
	}

	if (clip.w == 0)
		clip.w = src->w;
		
	if (clip.h == 0)
		clip.h = src->h;
		
	SDL_Rect rDst = { x, y, clip.w, clip.h };
	SDL_Rect rSrc = { clip.x, clip.y, clip.w, clip.h };

	//OPTIMIZETODO: Optimized version
	if ( SDL_BlitSurface(src, &rSrc, dst, &rDst) < 0 )
	{
        WARNING(SDL_GetError());
        return false;
	}

	return true;
}

void Image::SetClip(rect clip)
{
	SDL_Surface* surf = Surface();
	
	if (surf)
	{
		if (isDefaultRect(clip))
		{
			SDL_SetClipRect(surf, NULL);
		}
		else
		{
			SDL_Rect r = { clip.x, clip.y, clip.w, clip.h };
			SDL_SetClipRect(surf, &r);
		}
	}
}

rect Image::GetClip() const
{
	SDL_Surface* surf = Surface();
	if (!surf)
		return rect();
	
	SDL_Rect r;
	SDL_GetClipRect(surf, &r);
	
	return rect( r.x, r.y, r.w, r.h );
}

void Image::StateToString(string& s) const
{
	s += "This: " + pts((void*)this) + " SDL_Image: " + pts(mImage);

	SDL_Frameset* fs = Frameset();
	if (fs)
	{
		s += "\\nFrameset key: " + fs->key + " loop: " + its(fs->loop);
	}
	else
	{
		s += "\\nNo current frameset";
	}

	s += "\\nFrame index: " + its(mFrameIndex);
	s += "\\nNext frame change in " + its(mNextFrameChange - SDL_GetTicks()) + "ms";
	s += "\\nPlaying: " + its(mPlaying);
}

string Image::Filename() const
{
	if (mImage)
		return mImage->filename;

	return string();
}

color Image::GetPixel(sShort x, sShort y) const
{
    color c;
    SDL_Surface* surf = Surface();
    
    if (surf)
    	SDL_GetRGBA( SDL_GetPixel(surf, x, y), surf->format, &c.r, &c.g, &c.b, &c.a );
    	
    return c;
}

bool Image::SetPixel(sShort x, sShort y, color rgb, bool copy)
{
	SDL_Surface* surf = Surface();
	if (!surf) return false;

	return SDL_SetPixel(surf, x, y, rgb.r, rgb.g, rgb.b, rgb.a, copy);
}

bool Image::IsPixelTransparent(int x, int y)
{
	SDL_Surface* surf = Surface();
	if (!surf) return false;

	color c = GetPixel(x, y);
	
	if (c.a == 0)
		return true;
		
	//Some images don't have alpha channel. Check for colorkey
	if (SDL_GetPixel(surf, x, y) == surf->format->colorkey)
		return true;
		
	return false;
}

bool Image::DrawRect(rect r, color c, bool filled) 
{
	if (!filled)
		return DrawRound(r, 0, c);

	SDL_Surface* surf = Surface();
	if (!surf) return false;
	
	if (c.a == 255) //fast draw
	{
		SDL_Rect dr = { r.x, r.y, r.w, r.h };

		SDL_FillRect(surf, &dr, SDL_MapRGBA(surf->format, 
										(Uint8)c.r, (Uint8)c.g, 
										(Uint8)c.b, (Uint8)c.a));
	}
	else //slow assed per-pixel blending (TODO: faster technique)
	{
		for (int y = r.y; y < r.y+r.h; y++)
		{
			for (int x = r.x; x < r.x+r.w; x++)
				SetPixel(x, y, c);
		}
	}	
	return true;
}

bool Image::DrawRound(rect r, uShort corner, color rgb)
{
	return DrawRound(r.x, r.y, r.w, r.h, corner, rgb);
}

bool Image::DrawRound(sShort x0, sShort y0, uShort w, 
								uShort h, uShort corner, color rgb) 
{
	int dx, dy;
    int Xcenter, Ycenter, X2center, Y2center;
    int r, d, x = 0;
    int diagonalInc, rightInc = 6;

	SDL_Surface* surf = Surface();
	if (!surf) return false;
	
    if(w == 0 || h == 0)
        return false;

    if (corner != 0) 
	{
        d = w < h ? w : h;
        corner--;
        if (corner != 0 && corner + 2 >= d) 
		{
            if (corner + 2 == d)
                corner--;
            else
                corner = 0;
        }
    }

    d = 3 - (corner << 1);
    diagonalInc = 10 - (corner << 2);

    // Rectangles
    dx = w - (corner << 1);
    Xcenter = x0 + corner;
    dy = h - (corner << 1);
    Ycenter = y0 + corner;

    // Centers
    X2center = Xcenter + dx - 1;
    Y2center = Ycenter + dy - 1;

	LOCKSURF(surf);

    r = 0;
    while (dx--) 
	{
        SetPixel(x0 + corner + r, y0, rgb);
        SetPixel(x0 + corner + r, y0 + h - 1, rgb);
        r++;
    }

    r = 0;
    while (dy--) 
	{
        SetPixel(x0, y0 + corner + r, rgb);
        SetPixel(x0 + w - 1, y0 + corner + r, rgb);
        r++;
    }

    while (x < corner) 
	{
        SetPixel(Xcenter - corner, Ycenter - x, rgb);
        SetPixel(Xcenter - x, Ycenter - corner, rgb);
        SetPixel(X2center + x, Ycenter - corner, rgb);
        SetPixel(X2center + corner, Ycenter - x, rgb);
        SetPixel(X2center + x, Y2center + corner, rgb);
        SetPixel(X2center + corner, Y2center + x, rgb);
        SetPixel(Xcenter - x, Y2center + corner, rgb);
        SetPixel(Xcenter - corner, Y2center + x, rgb);

        if (d >= 0) 
		{
            d += diagonalInc;
            diagonalInc += 8;
            corner--;
        } 
		else 
		{
            d += rightInc;
            diagonalInc += 4;
        }
        rightInc += 4;
        x++;
    }
    
    UNLOCKSURF(surf);
	return true;
}

bool Image::DrawLine(sShort x1, sShort y1, 
						sShort x2, sShort y2, color rgb, byte thickness) 
{
    int i, dx, dy, sdx, sdy, dxabs, dyabs, x, y, px, py;
	int t;
	
	SDL_Surface* surf = Surface();
	if (!surf) return false;

    // Will speed up processing so that we don't use unecessary loops for lines
    //	that won't draw
    if(x1 > surf->w) x1 = surf->w - 1;
    if(x2 > surf->w) x2 = surf->w - 1;
    if(y1 > surf->h) y1 = surf->h - 1;
    if(y2 > surf->h) y2 = surf->h - 1;
    
    dx = x2 - x1;               /* the horizontal distance of the line */
    dy = y2 - y1;               /* the vertical distance of the line */
    dxabs = abs(dx);
    dyabs = abs(dy);
    sdx = sgn(dx);
    sdy = sgn(dy);
    x = dyabs >> 1;
    y = dxabs >> 1;
    px = x1;
    py = y1;
    
    LOCKSURF(surf);

    if (dxabs >= dyabs) /* the line is more horizontal than vertical */
	{          
        for(i = 0; i < dxabs; i++) 
		{
            y += dyabs;
            if(y >= dxabs) 
			{
                y -= dxabs;
                py += sdy;
            }
            px += sdx;
           	
           	for (t = 0; t < thickness; t++)
           		SetPixel(px, py+t, rgb);
        }
    } 
	else /* the line is more vertical than horizontal */
	{                        
        for(i = 0; i < dyabs; i++) 
		{
            x += dxabs;
            if(x >= dyabs) 
			{
                x -= dyabs;
                px += sdx;
            }
            py += sdy;
            for (t = 0; t < thickness; t++)
           		SetPixel(px+t, py, rgb);
        }
    }
    
    UNLOCKSURF(surf);
	
	return true;
}

bool Image::RenderPattern(Image* dst, rect rSrc, rect rDst)
{ 
    if (rSrc.w == 0 || rSrc.h == 0 || !Surface()) 
		return false;

    uShort px = 0; // X position on pattern plane
    uShort py = 0; // Y position on pattern plane
	uShort dw, dh;
    while (py < rDst.h) 
	{
        while (px < rDst.w) 
		{
            //uShort dw = (px + iw >= w) ? w - px : iw;
           // uShort dh = (py + ih >= h) ? h - py : ih;
            if (px + rSrc.w >= rDst.w) 
				dw = rDst.w - px;
			else 
				dw = rSrc.w; 
			
			if (py + rSrc.h >= rDst.h)
				dh = rDst.h - py;
			else
				dh = rSrc.h;

			if (!Render(dst, 
						rDst.x + px, rDst.y + py,
						rect( rSrc.x, rSrc.y, dw, dh )
						)) return false;
			
            px += rSrc.w;
        }
        py += rSrc.h;
        px = 0;
    }
	
	return true;
}

bool Image::RenderHorizontalEdge(Image* dst, rect rSrc, rect rDst)
{
	if (rDst.w > rSrc.w * 2) //center
	{
		if (!RenderPattern(dst,
					rect( rSrc.w + rSrc.x, 0 + rSrc.y, rSrc.w, rSrc.h ),
					rect( rDst.x + rSrc.w, rDst.y, rDst.w - rSrc.w * 2, rDst.h)
		)) return false;
	}

	//left
	if (!RenderPattern(dst,
				rSrc,
				rect( rDst.x, rDst.y, rSrc.w, rDst.h )
		)) return false;

	//right
	if (!RenderPattern(dst,
				rect( rSrc.w * 2 + rSrc.x, rSrc.y, rSrc.w, rSrc.h ),
				rect( rDst.x + rDst.w - rSrc.w, rDst.y, rSrc.w, rDst.h )
		)) return false;
	
	return true;
}

bool Image::RenderVerticalEdge(Image* dst, rect rSrc, rect rDst)
{
	if (rDst.h > rSrc.h * 2) //center
	{
		if (!RenderPattern(dst,
					rect( 0 + rSrc.x, rSrc.h + rSrc.y, rSrc.w, rSrc.h ),
					rect( rDst.x, rDst.y + rSrc.h, rDst.w, rDst.h - rSrc.h * 2)
			)) return false;
	}
	
	//top
	if (!RenderPattern(dst,
				rSrc,
				rect( rDst.x, rDst.y, rDst.w, rSrc.h )
		)) return false;

	//bottom
	if (!RenderPattern(dst,
				rect( rSrc.x, rSrc.h * 2 + rSrc.y, rSrc.w, rSrc.h ),
				rect( rDst.x, rDst.y + rDst.h - rSrc.h, rDst.w, rSrc.h )
		)) return false;
		
	return true;
}

bool Image::RenderBox(Image* dst, rect rSrc, rect rDst)
{
	//this function will only work with images that are exact boxes Such as:
	//an image with 32x32 frames, full image = 96x96 pixels

    // Draw the center area
    if (rDst.w > rSrc.w * 2 && rDst.h > rSrc.h * 2) 
	{
		if (!RenderPattern(dst, 
						rect( rSrc.w + rSrc.x, rSrc.h + rSrc.y, rSrc.w, rSrc.h ),
						rect( rDst.x + rSrc.w, rDst.y + rSrc.h, rDst.w - rSrc.w * 2, rDst.h - rSrc.h * 2)
						)) return false;
	}

    // Draw the sides
    if (rDst.w > rSrc.w * 2) 
	{	
		//Top
		if (!RenderPattern(dst, 
						rect( rSrc.w + rSrc.x, 0 + rSrc.y, rSrc.w, rSrc.h ),
						rect( rDst.x + rSrc.w, rDst.y, rDst.w - rSrc.w * 2, rSrc.h)
						)) return false; 
		//Bottom
		if (!RenderPattern(dst, 
						rect( rSrc.w + rSrc.x, rSrc.h * 2 + rSrc.y, rSrc.w, rSrc.h ),
						rect( rDst.x + rSrc.w, rDst.y + rDst.h - rSrc.h, rDst.w - rSrc.w * 2, rSrc.h)
						)) return false; 
	}

	if (rDst.h > rSrc.h * 2)
	{
		//left
		if (!RenderPattern(dst, 
						rect( 0 + rSrc.x, rSrc.h + rSrc.y, rSrc.w, rSrc.h ),
						rect( rDst.x, rDst.y + rSrc.h, rSrc.w, rDst.h - rSrc.h * 2)
						)) return false; 
		//right
		if (!RenderPattern(dst, 
						rect( rSrc.w * 2 + rSrc.x, rSrc.h + rSrc.y, rSrc.w, rSrc.h ),
						rect( rDst.x + rDst.w - rSrc.w, rDst.y + rSrc.h, rSrc.w, rDst.h - rSrc.h * 2)
						)) return false;
	}

    // Draw the corners

	//topleft
	if (!Render(dst, 
				rDst.x, rDst.y,
				rSrc
			)) return false;
			
	//topright
	if (!Render(dst, 
				rDst.x + rDst.w - rSrc.w, rDst.y,
				rect( rSrc.w * 2 + rSrc.x, rSrc.y, rSrc.w, rSrc.h )
			)) return false;

	//bottomleft
	if (!Render(dst, 
				rDst.x, rDst.y + rDst.h - rSrc.h,
				rect( rSrc.x, rSrc.h * 2 + rSrc.y, rSrc.w, rSrc.h )
			)) return false;
	
	//bottomright
	if (!Render(dst, 
				rDst.x + rDst.w - rSrc.w, rDst.y + rDst.h - rSrc.h,
				rect( rSrc.w * 2 + rSrc.x, rSrc.h * 2 + rSrc.y, rSrc.w, rSrc.h )
			)) return false;

	return true;
}

bool Image::Rotate(double degree, double zoom, int aa)
{
	SDL_Surface* surf = NULL;
	//go through all frames and ROTOZOOM
	for (int a = 0; a < mImage->framesets.size(); a++)
	{
		for (int b = 0; b < mImage->framesets.at(a).frames.size(); b++)
		{
			surf = rotozoomSurface(mImage->framesets.at(a).frames.at(b).surf, 	
									degree, zoom, aa);
			SDL_FreeSurface(mImage->framesets.at(a).frames.at(b).surf);
			mImage->framesets.at(a).frames.at(b).surf = surf;	
		}
	}
	return true;
}

bool Image::Scale(double xzoom, double yzoom, int aa)
{
	SDL_Surface* surf = NULL;
	//go through all frames and ZOOM
	for (int a = 0; a < mImage->framesets.size(); a++)
	{
		for (int b = 0; b < mImage->framesets.at(a).frames.size(); b++)
		{
			surf = zoomSurface(mImage->framesets.at(a).frames.at(b).surf, 	
								xzoom, yzoom, aa);
			SDL_FreeSurface(mImage->framesets.at(a).frames.at(b).surf);
			mImage->framesets.at(a).frames.at(b).surf = surf;	
		}
	}
	return true;
}

void Image::SavePNG(string file) const
{
	IMG_SavePNG(file.c_str(), Surface());
}

//TODO: Make this work with all frames!
void Image::ColorizeGreyscale(color modifier)
{
	double r, g, b;

	int w = Width();
	int h = Height();
	color c;
	SDL_Surface* surf = Surface();

	for (int y = 0; y < surf->h; y++)
	{
		for (int x = 0; x < surf->w; x++)
		{
			c = GetPixel(x, y);
			
			//If greyscale, perform pixel adjustment
			if ( isGreyscale(c) )
			{
				r = (double)modifier.r + ( (double)(255 - c.r) / 255 * (0.0 - (double)modifier.r) );
				g = (double)modifier.g + ( (double)(255 - c.g) / 255 * (0.0 - (double)modifier.g) );
				b = (double)modifier.b + ( (double)(255 - c.b) / 255 * (0.0 - (double)modifier.b) );
				c.r = (byte)r;
				c.g = (byte)g;
				c.b = (byte)b;
				SetPixel(x, y, c);
			}
		}
	}

}

void Image::ReduceAlpha(byte amount)
{
	if (amount == 0)
		return;
		
	//TODO: Optimize. Maybe if all pixels have 255 alpha, use SDL_SetAlpha?
	SDL_Surface* surf;
	int i, ii, x, y;
	color c;
	for (i = 0; i < mImage->framesets.size(); i++)
	{
		for (ii = 0; ii < mImage->framesets.at(i).frames.size(); ii++)
		{
			surf = mImage->framesets.at(i).frames.at(ii).surf;
			
			for (y = 0; y < surf->h; y++)
			{
				for (x = 0; x < surf->w; x++)
				{
					SDL_GetRGBA( SDL_GetPixel(surf, x, y), surf->format, &c.r, &c.g, &c.b, &c.a );
					
					if (c.a - amount < 0)
						c.a = 0;
					else
						c.a -= amount;

					SDL_SetPixel(surf, x, y, c.r, c.g, c.b, c.a, true);
				}
			}
		}
	}
}
