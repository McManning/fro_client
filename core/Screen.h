
#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "Common.h"
#include "Image.h"

#define SCREEN_WIDTH 800 //640
#define SCREEN_HEIGHT 600 //480

/*	Main application screen, where most of the rendering is done. 
	There is only one of these that should exist, and Screen::Instance() should be done before any SDL calls.
	Trivial Usage:
		Screen* scr = Screen::Instance();
		Image* img = resman->LoadImg("somefile.png");
		img->Render(scr, scr->Width() / 2 - img->Height() / 2, scr->Height() / 2 - img->Height() / 2);
		Screen::Destroy();
*/
class Screen : public Image
{
  public:
	void Resize(uShort w, uShort h);
	
	void Flip();
	
	void Update() { mNeedUpdate = true; };
	bool NeedsUpdate() const { return mNeedUpdate; };
	
	static Screen* Instance();
	static void Destroy();

  private:
  	Screen();
	~Screen();

	bool mNeedUpdate;
};
	
//Define SDL Flags for our screen. Will require Resize() in order take effect.	
void SetScreenFlags(Uint32 flags);

#endif 
