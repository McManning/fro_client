
#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "Common.h"
#include "Image.h"

#define SCREEN_WIDTH 800 //640
#define SCREEN_HEIGHT 600 //480

/**	
	Main application screen, where most of the rendering is done. 
*/
class Screen : public Image
{
  public:
	void Resize(uShort w, uShort h);
	
	void PreRender();
	void PostRender();
	
	/**	Adds a rect clip to the list of rects that can be drawn to during 
		the next render phase
	*/
	void AddRect(rect r);
	
	void Update() { mNeedUpdate = true; };
	bool NeedsUpdate() const { return mNeedUpdate; };
	
	static Screen* Instance();
	static void Destroy();

	bool mNoDraw;

  private:
  	Screen();
	~Screen();

	bool mNeedUpdate;
};

extern Screen* g_screen;
	
//Define SDL Flags for our screen. Will require Resize() in order take effect.	
void SetScreenFlags(Uint32 flags);

#endif // _SCREEN_H_
