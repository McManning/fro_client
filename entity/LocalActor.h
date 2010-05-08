
#ifndef _LOCALACTOR_H_
#define _LOCALACTOR_H_

#include "Actor.h" 

#define DEFAULT_ACTION_BUFFER_SEND_DELAY 3000

class LocalActor : public Actor
{
  public:
	LocalActor();
	~LocalActor();
	
	void Render();
	
	/* Overloaded to do _checkInput when we are capable of accepting new input */
	bool ProcessMovement();
	void PostMovement();
	bool StepForward();
	
	/* Overloaded to duplicate the data into mOutputActionBuffer */
	void AddToActionBuffer(string data);
	void ClearActionBuffer() { mActionBuffer.clear(); mOutputActionBuffer.clear(); };
	
	/*	Overloaded to save our avatar to GameManager's xml config */
	bool LoadAvatar(string file, string pass, uShort w, uShort h, uShort delay, 
					bool loopStand, bool loopSit);

	/*	index - Index of the stack where our new value for the property should be */
	int LuaSetProp(lua_State* ls, string& prop, int index);
	int LuaGetProp(lua_State* ls, string& prop);
					
	void NetSendState(string targetNick, string header);
	void NetSendActionBuffer();
	
	/*	 Tell the network what our current avatar mod is so that they can apply it locally */
	void NetSendAvatarMod();

	void LoadFlags();
	void SaveFlags();
	void PrintFlags();
	
	void Warp(string id, point2d position, string targetObjectName);

	/* Information that was sent out in the last NetSendActionBuffer */
	point2d mLastSavedPosition;
	byte mLastSavedSpeed;
	
	/* 	flag that, if trying to send the buffer mid-jump (or similar state), 
		is set to true and sends buffer out after landing. */
	bool mNeedsToSendBuffer; 
	
	string mOutputActionBuffer; //outgoing action data
	
	//keeps track of when to send mOutputActionBuffer over the network
	timer* mActionBufferTimer;

	bool mIsLocked; //if true, user input is ignored

	int mActionBufferSendDelayMs;

  private:
	void _checkInput();
	
	bool mShiftDown;
};

#endif //_LOCALACTOR_H_
