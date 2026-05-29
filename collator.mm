// Copyright © 2021 CCP ehf.
#include "stdafx.h"
#include "collator.h"

#if __APPLE__

#import "Foundation/Foundation.h"

// -------------------------------------------------------------
// Description:
//   The comparison function compares the character data stored in two different strings.
//   Returns information about whether a string is less than, greater than or equal to another string.
// Arguments:
//   source - the string to be compared with.
//   target - the string that is to be compared with the source string.
// Return Value:
//   -1 if source is lesser than target, 0 if equal and 1 if source is greater than target.
// -------------------------------------------------------------
int Collator::Compare( const std::wstring& source, const std::wstring& target ) const
{
	// Workaround for std::collate bug on macOS
	// https://jira.ccpgames.com/browse/EO-14241
	NSString *nsSource = [[NSString alloc] initWithBytes:source.data()
	                                              length:source.size() * sizeof(wchar_t)
	                                            encoding:NSUTF32LittleEndianStringEncoding];
	NSString *nsTarget = [[NSString alloc] initWithBytes:target.data()
	                                              length:target.size() * sizeof(wchar_t)
	                                            encoding:NSUTF32LittleEndianStringEncoding];
	
	NSInteger ret = [nsSource compare:nsTarget
	                          options:NSCaseInsensitiveSearch | NSNumericSearch
	                            range:NSMakeRange(0, nsSource.length)
	                           locale:m_locale];

#if !__has_feature(objc_arc)
	[nsSource release];
	[nsTarget release];
#endif

	return ret;
}

// -------------------------------------------------------------
// Description:
//   Sets the locale for this collator.
// Arguments:
//   languageCode - string representation of the locale as used in Python
// -------------------------------------------------------------
void Collator::SetLocale( const std::string& languageCode )
{
	m_localeID = CodeToLanguageID( languageCode.c_str() );

#if !__has_feature(objc_arc)
	[m_locale release];
	[m_localeName release];
#endif
	m_localeName = [NSString stringWithUTF8String:LanguageIDToLocaleName( m_localeID )];
	m_locale = [NSLocale localeWithLocaleIdentifier:m_localeName];
}

#endif
