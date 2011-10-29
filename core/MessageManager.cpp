
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


#include "MessageManager.h"
#include "widgets/Console.h"

MessageManager messenger;

MessageManager::MessageManager()
{
	mDebugMode = false;
	mCurrentHandle = 1;
}

MessageManager::~MessageManager()
{
	for (int i = 0; i < mListeners.size(); i++)
		_delete(i);
}

void MessageManager::Process(uLong ms)
{
	for (int i = 0; i < mListeners.size(); i++)
	{
		if ( !mListeners.at(i) ) //Erase any nulled MessageListeners
		{
			mListeners.erase(mListeners.begin() + i);
			i--;
		}
	}
}

MessageListener* MessageManager::AddListener(string id,
					void (*onActivate)(MessageListener*, MessageData&, void*),
					void (*onDestroy)(MessageListener*),
					void* userdata)
{

	MessageListener* m = new MessageListener;
	m->id = id;
	m->onActivate = onActivate;
	m->onDestroy = onDestroy;
	m->userdata = userdata;
	m->handle = ++mCurrentHandle;

	mListeners.push_back(m);
	return m;
}

bool MessageManager::RemoveListener(void (*onActivate)(MessageListener*, MessageData&, void*))
{
	bool found = false;

	for (int i = 0; i < mListeners.size(); i++)
	{
		if ( mListeners.at(i) && mListeners.at(i)->onActivate == onActivate )
		{
			_delete(i);
			found = true;
		}
	}

	return found;
}

bool MessageManager::RemoveListener(string id, void* userdata)
{
	bool found = false;

	for (int i = 0; i < mListeners.size(); i++)
	{
		if ( mListeners.at(i) && (id.empty() || mListeners.at(i)->id == id) )
		{
			//Check for userdata match if it's included in the search
			if (userdata == NULL || mListeners.at(i)->userdata == userdata)
			{
				_delete(i);
				found = true;
			}
		}
	}

	return found;
}

bool MessageManager::RemoveListener(MessageListener* m)
{
	if (!m)
		return true;
		
	for (int i = 0; i < mListeners.size(); i++)
	{
		if ( mListeners.at(i) == m ) 
		{
			_delete(i);
			return true;
		}
	}

	return false;
}


bool MessageManager::RemoveListenerByHandle(uLong handle)
{
	if (handle != INVALID_LISTENER_HANDLE)
	{
		for (int i = 0; i < mListeners.size(); i++)
		{
			if ( mListeners.at(i)->handle == handle ) 
			{
				_delete(i);
				return true;
			}
		}
	}
	return false;
}

void MessageManager::_delete(int index)
{
	if (!mListeners.at(index)) return;

	if (mListeners.at(index)->onDestroy)
		mListeners.at(index)->onDestroy( mListeners.at(index) );

	SAFEDELETE(mListeners.at(index));
}

void MessageManager::Dispatch(MessageData& data, void* sender)
{
	if (mDebugMode)
		console->AddMessage("\\c777::Dispatch " + data.mId);
	
	MessageListener* m;
	for (int i = 0; i < mListeners.size(); i++)
	{
		m = mListeners.at(i);
		if ( m && wildmatch(m->id.c_str(), data.mId.c_str()) )
		{
			if (!m->onActivate)
				FATAL("No activate for id " + data.mId);
				
			m->onActivate(m, data, sender);
		}
	}
}

