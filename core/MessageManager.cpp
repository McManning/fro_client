
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
		if (m && m->id == data.mId)
		{
			if (!m->onActivate)
				FATAL("No activate for id " + data.mId);
				
			m->onActivate(m, data, sender);
		}
	}
}

