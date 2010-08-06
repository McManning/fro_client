
#include <lua.hpp>
#include "Lunem.h"
#include "../core/io/FileIO.h"

void callback_lunemAvatarDownloadSuccess(downloadData* data)
{
	Lunem* lu = (Lunem*)data->userData;
	if (lu)
	{
		// Set species again, this time using the local file 
		lu->SetSpecies(lu->m_sSpecies);
	}
}

void callback_lunemAvatarDownloadFailure(downloadData* data)
{
	/* What to do?
	Lunem* lu = (Lunem*)data->userData;
	if (lu)
	{
		
	}*/
}

Lunem::Lunem()
{
	mType = ENTITY_LUNEM;
	mShadow = true;
}

Lunem::~Lunem()
{

}

void Lunem::SetSpecies(string s)
{
	m_sSpecies = s;
	if (mName.empty())
		mName = m_sSpecies;

	/*
		Try to load our avatar from cache/lunem/species (no extension, due to being any format)
		Assuming the dimensions are 128x128 (if png)
		If we don't have one local, load a temporary replacement and download the avatar
	*/
	
	if (m_sSpecies == "none") // Don't use a custom download if we have a custom species
		return;
	
	XmlFile xf;
	string url, file = DIR_CACHE;
	file += "lunem/" + m_sSpecies;
	
	LoadAvatar("assets/unknown_lunem.png", "", 48, 48, 100, true, true);
	SwapAvatars();
	
	if (fileExists(file)) // load from disk
	{	
		LoadAvatar(file, "", 128, 128, 100, true, true);
		
		// if the image file is invalid, delete it from cache and let it try to redownload later
		if (!SwapAvatars())
		{
			removeFile(file);
		}
	}
	else // Need to download
	{
		// Download from master
		if (!xf.LoadFromFile("assets/connections.cfg"))
		{
			FATAL(xf.GetError());	
		}
		
		TiXmlElement* e = xf.mDoc.FirstChildElement();
		if (e)
			e = e->FirstChildElement("lunem");
		
		if (e)
			url = xf.GetText(e);
		
		if (url.empty())
		{
			FATAL("Invalid Url");	
		}
		
		url += m_sSpecies; // Something like http://site.com/dir/SPECIES_NAME
		
		downloader->QueueDownload(url, file, this, callback_lunemAvatarDownloadSuccess, 
									callback_lunemAvatarDownloadFailure, false);
	}
}

/*	index - Index of the stack where our new value for the property should be */
int Lunem::LuaSetProp(lua_State* ls, string& prop, int index)
{
	if (prop == "species") SetSpecies(lua_tostring(ls, index));

	else return Actor::LuaSetProp(ls, prop, index);

	return 1;
}

int Lunem::LuaGetProp(lua_State* ls, string& prop)
{
	if (prop == "species") lua_pushstring( ls, m_sSpecies.c_str() );
	else return Actor::LuaGetProp(ls, prop);

	return 1;
}

void Lunem::ReadFromFile(FILE* f)
{
	char buffer[128];
	unsigned char c;
	
	// read species string
	fread(&c, sizeof(c), 1, f); //length
	ASSERT(c > 0);

	if (c > 127) c = 127;
	fread(&buffer, c, 1, f);
	m_sSpecies.append(buffer, c);
	
	// read name string
	fread(&c, sizeof(c), 1, f); //length
	ASSERT(c > 0);

	if (c > 127) c = 127;
	fread(&buffer, c, 1, f);
	mName.append(buffer, c);

	console->AddMessage("Reading in, name: " + mName + " species: " + m_sSpecies);

	fread(&m_bLevel, sizeof(m_bLevel), 1, f);
	fread(&m_bGene, sizeof(m_bGene), 1, f);
	
	fread(&m_bType1, sizeof(m_bType1), 1, f);
	fread(&m_bType2, sizeof(m_bType2), 1, f);
	fread(&m_bType3, sizeof(m_bType3), 1, f);
	
	fread(&m_bBaseAttack, sizeof(m_bBaseAttack), 1, f);
	fread(&m_bBaseDefense, sizeof(m_bBaseDefense), 1, f);
	fread(&m_bBaseSpeed, sizeof(m_bBaseSpeed), 1, f);
	fread(&m_bBaseHealth, sizeof(m_bBaseHealth), 1, f);
	fread(&m_iExp, sizeof(m_iExp), 1, f);
	fread(&m_iCurrentHealth, sizeof(m_iCurrentHealth), 1, f);

	//read skills
	for (int i = 0; i < 5; ++i)
	{
		fread(&c, sizeof(c), 1, f); //length
		if (c > 0)
		{
			if (c > 127) c = 127;
			fread(&buffer, c, 1, f);
			m_sSkills[i].id.append(buffer, c);
		}
	}
	
	SetSpecies(m_sSpecies);

}

void Lunem::WriteToFile(FILE* f) const
{
	unsigned char c;
	
	c = (m_sSpecies.length() > 127) ? 127 : m_sSpecies.length();
	fwrite(&c, 1, 1, f);
	fwrite(m_sSpecies.c_str(), c, 1, f);

	c = (mName.length() > 127) ? 127 : mName.length();
	fwrite(&c, 1, 1, f);
	fwrite(mName.c_str(), c, 1, f);

	fwrite(&m_bLevel, sizeof(m_bLevel), 1, f);
	fwrite(&m_bGene, sizeof(m_bGene), 1, f);
	
	fwrite(&m_bType1, sizeof(m_bType1), 1, f);
	fwrite(&m_bType2, sizeof(m_bType2), 1, f);
	fwrite(&m_bType3, sizeof(m_bType3), 1, f);
	
	fwrite(&m_bBaseAttack, sizeof(m_bBaseAttack), 1, f);
	fwrite(&m_bBaseDefense, sizeof(m_bBaseDefense), 1, f);
	fwrite(&m_bBaseSpeed, sizeof(m_bBaseSpeed), 1, f);
	fwrite(&m_bBaseHealth, sizeof(m_bBaseHealth), 1, f);
	fwrite(&m_iExp, sizeof(m_iExp), 1, f);
	fwrite(&m_iCurrentHealth, sizeof(m_iCurrentHealth), 1, f);
	
	//write skills
	for (int i = 0; i < 5; ++i)
	{
		c = (m_sSkills[i].id.length() > 127) ? 127 : m_sSkills[i].id.length();
		fwrite(&c, 1, 1, f);
		fwrite(m_sSkills[i].id.c_str(), c, 1, f);
	}
}

