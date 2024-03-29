
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
		mRed->SetImageBase("assets/gui/rgbscroller/red");
	y += 25;
	mGreen = new Scrollbar(this, "", rect(10,y,150,20), HORIZONTAL, 9, 1, 9, callback_updateColor);
		mGreen->mHoverText = "Green Value";
		mGreen->SetImageBase("assets/gui/rgbscroller/green");

	y += 25;
	mBlue = new Scrollbar(this, "", rect(10,y,150,20), HORIZONTAL, 9, 1, 9, callback_updateColor);
		mBlue->mHoverText = "Blue Value";
		mBlue->SetImageBase("assets/gui/rgbscroller/blue");
	y += 25;
	
	new Label(this, "", rect(10,y), "Code");
	mResult = new Input(this, "", rect(60, y, 102, 20), "", 32, true, NULL);
		mResult->SetText( "\\c999" );
		mResult->SetMenuEnabled(false);
		mResult->mReadOnly = true;
		
	//add a button that'll send our color to the input
	if (mCreator)
	{
		b = new Button(this, "ok", rect(180,y,20,20), "", callback_sendColor);
		b->mHoverText = "Okay";
		b->SetImage("assets/buttons/okay.png");
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
	FlagRender();
}
