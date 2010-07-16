
#include "ColorPicker.h"

#include "Input.h"
#include "Scrollbar.h"
#include "Label.h"
#include "Button.h"

#include "../GuiManager.h"
#include "../Screen.h"
#include "../ResourceManager.h"

void callback_updateColor(Scrollbar* s)
{
	ColorPicker* c = (ColorPicker*)s->GetParent();
	c->Update();
}

void callback_sendColor(Button* b)
{
	ColorPicker* c = (ColorPicker*)b->GetParent();
	
	if (c->mCreator)
		c->mCreator->AddText(c->mResult->GetText());
	
	c->Die();
}

ColorPicker::ColorPicker(Input* creator) :
	Frame(NULL, "color", rect(0,0,0,0), "Color Picker", true, false, true, true)
{
	uShort y = 30;

	mCreator = creator;

	Button* b;
	/*
		10, 120 standard w
		10 + 120 + 5 + 120 + 10 = width  (265)
		y += 25 for each level of widget
		135 is second set
	*/

	mRed = new Scrollbar(this, "", rect(10,y,150,20), HORIZONTAL, 9, 1, 9, callback_updateColor);
		mRed->mHoverText = "Red Value";
	//	mRed->mTabImage->mSrc.x = 90;
	y += 25;
	mGreen = new Scrollbar(this, "", rect(10,y,150,20), HORIZONTAL, 9, 1, 9, callback_updateColor);
		mGreen->mHoverText = "Green Value";
//		mGreen->mTabImage->mSrc.x = 105;
	y += 25;
	mBlue = new Scrollbar(this, "", rect(10,y,150,20), HORIZONTAL, 9, 1, 9, callback_updateColor);
		mBlue->mHoverText = "Blue Value";
//		mBlue->mTabImage->mSrc.x = 120;
	y += 25;
	
	new Label(this, "", rect(10,y), "Code");
	mResult = new Input(this, "", rect(60, y, 102, 20), "", 32, true, NULL);
		mResult->SetText( "\\c999" );
		mResult->SetMenuEnabled(false);
	
	//add a button that'll send our color to the input
	if (mCreator)
	{
		b = new Button(this, "ok", rect(180,y,20,20), "", callback_sendColor);
		b->mHoverText = "Okay";
		b->mImage = resman->LoadImg("assets/buttons/okay.png");
	}
	
	y += 25;

	SetSize(220, y);
	Center();
	ResizeChildren();

	if (mCreator)
	{
		DemandFocus();
	}
	
}

ColorPicker::~ColorPicker()
{

}

void ColorPicker::Render()
{
	//if (gui->GetDemandsFocus() == this)
	//	gui->RenderDarkOverlay();

	Frame::Render();
	
	//draw color box
	rect r = GetScreenPosition();
	r.x += 165;
	r.y += 35;
	r.w = 45;
	r.h = r.w;

	Screen::Instance()->DrawRect(r, slashCtoColor(mResult->GetText()));
}

void ColorPicker::Update()
{
	mResult->SetText("\\c" + its(mRed->GetValue()) + its(mGreen->GetValue()) + its(mBlue->GetValue()));
}
