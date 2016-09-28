// ****************************************************************************
// By Antonino Perricone
// http://amicoperry.altervista.org/luce/
//
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial-ShareAlike 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc-sa/3.0/.
// ****************************************************************************

#pragma once

// A better template for acquiring and release suites.
// Depends on a globally defined CPhotoshopLuce::GetBasicSuite() pointer.
// Illustrator example
// AutoSuite<AITextPathSuite> sAITextPath(kAITextPathSuite, kAITextPathSuiteVersion);
// AutoSuite will throw on bad access
// if you were using MySuite switch to AutoSuite
template<class T> class AutoSuite
{
private:
	T * suite;
	const long suiteVersion;
	const char * suiteName;
	SPErr error;

	// make sure the compiler doesn't create this
	AutoSuite();
	
	void AcquireSuite(void)
	{
		if (CPhotoshopLuce::GetBasicSuite() != NULL)
		{
			error = CPhotoshopLuce::GetBasicSuite()->AcquireSuite(suiteName, 
				                           suiteVersion, 
				 						   (const void**)&suite);
		}
		else
		{
			error = kSPBadParameterError;
		}
	}

public:
	AutoSuite(const char * name, const long version) : 
	  suite(NULL), suiteName(name), suiteVersion(version), error(0)
	{ }

	~AutoSuite()
	{
		Unload();
	}

	const T * operator->()
	{ 
		if (suite == NULL)
			AcquireSuite();
		return suite; 
	}

	const T * operator *()
	{
		if (suite == NULL)
			AcquireSuite();
		return suite;
	}
	
	void Unload(void)
	{
		if (suite != NULL && suiteName != NULL && 
			CPhotoshopLuce::GetBasicSuite() != NULL)
		{
			CPhotoshopLuce::GetBasicSuite()->ReleaseSuite(suiteName, suiteVersion);
			suite = NULL;
		}
	}
	
	bool IsSupported()
	{
		if( suite != NULL ) return true;
		AcquireSuite();
		return ( suite != NULL );
	}
	SPErr GetError() { return error; }
};
