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
    auto& col = std::use_facet<std::collate<wchar_t>>( m_locale );
    return col.compare(
       source.c_str(), source.c_str() + source.length(),
       target.c_str(), target.c_str() + target.length() );
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
    try
    {
        m_locale = std::locale( LanguageIDToLocaleName( m_localeID ) );
    }
    catch ( std::runtime_error )
    {
        m_locale = std::locale();
    }
}
