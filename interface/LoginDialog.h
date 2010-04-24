
#ifndef _LOGINDIALOG_H_
#define _LOGINDIALOG_H_

#include "../core/Core.h"
#include "../core/widgets/Frame.h"

class Multiline;
class LoginDialog : public Frame 
{
  public:
	LoginDialog();
	~LoginDialog();
	
	void Render(uLong ms);
	
	/*	If skip = true, will not send id & pass to the login server */
	void SendLoginQuery(bool skip);
	void SetControlState(bool enabled);
	void SendLogin();
	void Skip();

	string mUsername;
	string mPassword;
	
	Image* mBackgroundImage;
	Multiline* mText;
};
extern LoginDialog* loginDialog;

#endif //_LOGINDIALOG_H_
