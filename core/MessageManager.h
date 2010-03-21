
#ifndef _MESSAGEMANAGER_H_
#define _MESSAGEMANAGER_H_

#include "Common.h"
#include "MessageData.h"

#define INVALID_LISTENER_HANDLE (0)

class MessageListener
{
  public:
	MessageListener()
	{
		userdata = NULL;
		onActivate = NULL;
		onDestroy = NULL;
		handle = INVALID_LISTENER_HANDLE;
	};
	string id;
	void (*onActivate)(MessageListener*, MessageData&, void*);
	void (*onDestroy)(MessageListener*); //Commonly used for freeing the userdata.
	
	void* userdata; //whatever the user wants to attach to this listener
	uLong handle; //our unique handle
};

class MessageManager
{
  public:
	MessageManager();
	~MessageManager();
	
	/*	TODO: Replace this with a better cleaning system. Maybe on an X second timer? */
	void Process(uLong ms);
	
	/*	Attach a new listener function to the specified message id. When this message is fired from another class, this
		attach function will be called, along with whatever functions are attached to the same id. 
	*/
	MessageListener* AddListener(string id,
					void (*onActivate)(MessageListener*, MessageData&, void*),
					void (*onDestroy)(MessageListener*) = NULL,
					void* userdata = NULL);
	
	/*	Remove first listener found with the matching callback */
	bool RemoveListener(void (*onActivate)(MessageListener*, MessageData&, void*));
	bool RemoveListener(string id, void* userData = NULL); //Remove by ID, userdata optional
	bool RemoveListener(MessageListener* m);
	bool RemoveListenerByHandle(uLong handle);

	/*	Dispatch the specified message to all functions listening to the id. 
		Sender is a link to the class (or structure, or other) that ordered the message out .
	*/
	void Dispatch(MessageData& data, void* sender = NULL); 

	std::vector<MessageListener*> mListeners;

	bool mDebugMode;
	uLong mCurrentHandle; //Continuously increment for each new listener
	
  private:
	void _delete(int index);
};

extern MessageManager messenger;

#endif //_MESSAGEMANAGER_H_
