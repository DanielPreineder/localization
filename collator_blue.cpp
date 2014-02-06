#include "stdafx.h"
#include "collator.h"

BLUE_DEFINE( Collator );

// DOM-IGNORE-BEGIN
const Be::ClassInfo* Collator::ExposeToBlue()
{
	EXPOSURE_BEGIN(Collator, "A collator is an object you can use to compare two strings in a locale-aware way." )
		MAP_INTERFACE( Collator )
		MAP_METHOD_AND_WRAP( "Compare", Compare, 
							 "The comparison function compares the character data stored in two different strings.\n\n"
							 "Returns information about whether a string is less than, greater than or equal to"
							 "another string.\n"
							 "Parameters:\n"
							 "source - the string to be compared with.\n"
							 "target - the string that is to be compared with the source string.\n\n"
							 "Returns:\n"
							 "-1 if source is lesser than target, 0 if equal and 1 if source is greater than target." )
		MAP_PROPERTY( "locale", GetLocale, SetLocale, "The locale for this collator." )
	EXPOSURE_END()
}
// DOM-IGNORE=END
