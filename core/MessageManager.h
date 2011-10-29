
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
