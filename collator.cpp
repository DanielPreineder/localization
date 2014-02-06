#include "stdafx.h"
#include "collator.h"

// -------------------------------------------------------------
// Description:
//   Constructs a collator instance for the given locale.
// Arguments:
//   languageCode - string representation of the locale as used in Python
// SeeAlso:
//   LanguageID
// -------------------------------------------------------------
Collator::Collator( const std::string& languageCode, IRoot* lockObj /* = 0 */ )
{
	SetLocale( languageCode );
}

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
	int ret = CompareStringW( m_localeID, 0, source.c_str(), -1, target.c_str(), -1 );

	switch ( ret )
	{
		case CSTR_LESS_THAN:
			return -1;
		case CSTR_GREATER_THAN:
			return 1;
		case CSTR_EQUAL:
		default:
			return 0;
	};
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
}
