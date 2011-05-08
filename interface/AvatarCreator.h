
/*
	NOTES:
		-When base is changed, it should change the base model between child/teen/female/male and set 
			other sections to defaults for that base. (Default hair, default head, clothing, etc)
		-Files will be named as: base.section.filename.png
			Examples:
				child.body.underpants.png
				male.body.underpants.png
				child.head.default.png
				male.head.default.png
				child.hair.default.png
				male.hair.default.png
				etc
		-When cycling body parts, will only search for: DIR_AVA + mBase + ".body.*.png";
			Same for head/hair, but replacing body.
		-Url structured as: avy://base/head/rgbhex/body/rgbhex/hair/rgbhex/timestamp
			Not using the full filename for head/body/hair, but just the part in wildcard. We can reconstruct it clientside.
		-Is it necessary.. to have multiple bases? Can we remove the height difference between men and women, and have the same hairlines,
			but allow long/short hair, manly/feminine faces, manly/feminine clothing, to go together? Big tits, afro, scarred face? The way the head
			bobs in animating.. can it be converted? Then again there are height positioning differences between adult/child. So that concept is out.
			Unless.. we change dst coordinates when painting on.. that's another technique. Or, we do combine male/female to make that section easier,
			but still keeping the 3 height bases. And this technique IS extendable later on to different races... different creatures/races and whatnot. 
		-Each part will have 2 frames, 5 rows. (4 directions + 1 sit) All should have the same size. 32x64 is a bit.. small. 
			So let's improve it and set it to 40x70. 
			
	Unrelated Note:
		Convert to avatar format. After it splits into frames it should scan the first pixel of every frame. If the first pixel is 
			NOT the transparent color we have set up, change the transparent color for that frame to that pixel. (of course given
			it uses colorkeys rather than alpha channels) This way we can have each frame in pngs have a different colored background
			and make it easier for editors to see the lines between frames.
			To avoid having to check every pixel of every frame for alpha, the initial load should set a flag in Image* stating if it USES
			its alpha channel or not. (Not necessary having an alpha channel, but USING it since some editors save pngs with an unused alpha)
*/

#ifndef _AVATARCREATOR_H_
#define _AVATARCREATOR_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class Label;
class AvatarCreator : public Frame
{
  public:
	AvatarCreator(int workingIndex = 0, string url = "");
	~AvatarCreator();
	
	void CreateControls();
	void Toggle(string id);
	
	void CycleBase(bool forward);
	void CycleHead(bool forward);
	void CycleBody(bool forward);
	void CycleHair(bool forward);
	
	void Update();
	void Render();
	
	void SetError(string error);
	
	/*	Inserts our current avatar into our favorites list */
	void AddDesignToFavorites();
	
	/*	Create a PNG of our creation */
	void SaveToFile();
		
	/*
		Imports an avy:// url, breaks it up, configures the current settings to match, 
		and then stores the original to be replaced later when saved back to Avatar Favorites
	*/
	void ImportUrl(string url);
	
	string mBase; //string containing head, body, or hair.
	Frame* mBaseFrame;
	
	Image* mHead;
	Frame* mHeadFrame;
	string mHeadFile;
	color mHeadColor;

	Image* mBody;
	Frame* mBodyFrame;
	string mBodyFile;
	color mBodyColor;

	Image* mHair;
	Frame* mHairFrame;
	string mHairFile;
	color mHairColor;

	Image* mCompositePreview;
	Image* mFullComposite; //the avatar before being split into pieces
	
	Label* mErrorLabel;

	int mWorkingIndex;
	
  private:
	void _redraw();
	
	bool mRedraw;
};

#endif //_AVATARCREATOR_H_
