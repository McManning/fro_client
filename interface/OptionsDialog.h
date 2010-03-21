
#ifndef _OPTIONSDIALOG_H_
#define _OPTIONSDIALOG_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class OptionsDialog : public Frame 
{
  public:
	OptionsDialog();
	~OptionsDialog();

	void Save();
	
	void Toggle(string id);

  private:
	void _buildFrameUser();
	void _buildFrameNetwork();	
	void _buildFrameAudio();
	void _buildFrameGraphics();
	
	Frame* mFrameUser;
	Frame* mFrameNetwork;
	Frame* mFrameAudio;
	Frame* mFrameGraphics;

};
extern OptionsDialog* optionsDialog;

#endif //_OPTIONSDIALOG_H_

