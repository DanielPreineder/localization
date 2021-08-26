#include "stdafx.h"
#include "formatters.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <time.h>
#include <cstring>
#include "localization.h"
#include "parser.h"

static_assert( sizeof( wchar_t) == sizeof( PY_UNICODE_TYPE ), "EveLocalization expects PY_UNICODE_TYPE to match wchar_t" );

const unsigned int GROUPING_VALUE = 3;

// -------------------------------------------------------------
// Description:
//   Provides formatting of simple unicode strings.
// Arguments:
//   token - Token to be used for information about the formatting to use.
//   lang - Language the token is in. This is the same as the lang parameter passed into Parse.
//   value - The value we are formatting. This must be convertable to a PyUnicodeObject.
//   kwargs - Keyword arguments passed in from Python. This is the same as the kwargs parameter passed into Parse.
//   retval - Unicode stream that will receive the formatted value. This is the same as the retval parameter passed into Parse.
// Returns:
//   true on success, false if it encountered an error
// -------------------------------------------------------------
bool SimpleValueFormatter( const Token& token, const Language& lang, PyObject* value, PyObject* kwargs, std::wstringstream& retVal )
{
	PyObject* unicodeValue = PyObject_Unicode( value );
	if ( ! unicodeValue )
	{
		retVal << "Error converting value in SimpleValueFormatter";
		return false;
	}

	std::wstring val( PyUnicodeToWString( unicodeValue ) );
	Py_DECREF( unicodeValue );
	return SimpleValueFormatter( token, lang, val, kwargs, retVal );
}

// -------------------------------------------------------------
// Description:
//   Overloaded version of the SimpleValueFormatter that accepts a std::wstring instead of a PyObject as value.
// Arguments:
//   token - Token to be used for information about the formatting to use.
//   lang - Language the token is in. This is the same as the lang parameter passed into Parse.
//   value - The value we are formatting.
//   kwargs - Keyword arguments passed in from Python. This is the same as the kwargs parameter passed into Parse.
//   retval - Unicode stream that will receive the formatted value. This is the same as the retval parameter passed into Parse.
// Returns:
//   true on success, false if it encountered an error
// -------------------------------------------------------------
bool SimpleValueFormatter( const Token& token, const Language& lang, std::wstring& val, PyObject* kwargs, std::wstringstream& retVal )
{
	//std::wstring::iterator iter = val.begin();

	//if ( CAPITALIZE == ( token.flags & CAPITALIZE ) )
	//{	
	//	*val.begin() = std::toupper( *val.begin(), lang.locale );
	//	while ( ++iter != val.end() )
	//	{
	//		*iter = std::tolower( *iter, lang.locale );
	//	}
	//}
	//else if ( UPPERCASE == ( token.flags & UPPERCASE ) )
	//{
	//	while ( iter != val.end() )
	//	{
	//		*iter++ = std::toupper( *iter, lang.locale );
	//	}
	//}
	//else if ( LOWERCASE == ( token.flags & LOWERCASE ) )
	//{
	//	while ( iter != val.end() )
	//	{
	//		*iter++ = std::tolower( *iter, lang.locale );
	//	}
	//}
	//else if ( TITLECASE == ( token.flags & TITLECASE ) )
	//{
	//	bool toUpper = true;

	//	while ( iter != val.end() )
	//	{
	//		if ( toUpper )
	//		{
	//			*iter = std::toupper( *iter, lang.locale );
	//		}
	//		else
	//		{
	//			*iter = std::tolower( *iter, lang.locale );
	//		}

	//		toUpper = ! std::isalpha( *iter++, lang.locale );
	//	}
	//}

	if ( TOKENFLAG_LINKINFO == ( token.flags & TOKENFLAG_LINKINFO ) )
	{
		if ( ! token.kwargs )
		{
			PyErr_SetString( PyExc_RuntimeError, "Token has no kwargs." );
			return false;
		}

		KeywordArgsCit cit = token.kwargs->find( "linkinfo" );
		if ( cit == token.kwargs->cend() )
		{
			PyErr_SetString( PyExc_SyntaxError, "Token has a linkinfo tag but no linkinfo parameter." );
			return false;
		}

		PyObject* linkData = PyDict_GetItem( kwargs, cit->second );
		if ( ! linkData )
		{
			return false;
		}

		if ( PySequence_Check ( linkData ) )
		{
			linkData = PySequence_Fast( linkData, "linkData must be a sequence type!" );
			if ( ! linkData )
			{
				return false;
			}

			size_t len = PySequence_Fast_GET_SIZE( linkData );
			if (len == 0)
			{
				Py_DECREF( linkData );
				return false;
			}

			PyObject* tmp = PyObject_Unicode( PySequence_Fast_GET_ITEM( linkData, 0 ) );
			if ( ! tmp )
			{
				Py_DECREF( linkData );
				return false;
			}
			retVal << L"<a href=" << reinterpret_cast<const wchar_t*>( PyUnicode_AS_UNICODE( tmp ) ) << L":";

			Py_XDECREF( tmp );

			for ( size_t i = 1; i < len; ++i )
			{
				if ( i > 1 )
				{
					retVal << L"//";
				}

				PyObject* tmp = PyObject_Unicode( PySequence_Fast_GET_ITEM( linkData, i ) );
				if ( ! tmp )
				{
					Py_DECREF( linkData );
					return false;
				}
				retVal << reinterpret_cast<const wchar_t*>( PyUnicode_AS_UNICODE( tmp ) );
				Py_XDECREF( tmp );
			}

			Py_DECREF( linkData );

			retVal << L">" << val << "</a>";
		}
		else
		{
			PyErr_SetString( PyExc_TypeError, "linkinfo parameter must be of sequence type" );
			return false;
		}
	}
	else
	{
		retVal << val;
	}
	return true;
}

// -------------------------------------------------------------
// Description:
//   Provides formatting of date time values.
//   NOTE: This needs to stay in sync with localizationUtil.FormatDateTime !
// Arguments:
//   token - Token to be used for information about the formatting to use.
//   lang - Language the token is in. This is the same as the lang parameter passed into Parse.
//   value - The value we are formatting. This must be either a tuple, 64 bit value or float.
//   kwargs - Keyword arguments passed in from Python. This is the same as the kwargs parameter passed into Parse.
//   retval - Unicode stream that will receive the formatted value. This is the same as the retval parameter passed into Parse.
// Returns:
//   true on success, false if it encountered an error
// -------------------------------------------------------------
bool DateTimeFormatter( const Token& token, const Language& lang, PyObject* value, PyObject* kwargs, std::wstringstream& retVal )
{
	wchar_t buffer[STACK_BUFFER_SIZE_LARGE];

	wchar_t* format;
	struct tm timeInfo;

	if ( ! token.kwargs )
	{
		PyErr_SetString( PyExc_RuntimeError, "Token has no kwargs in datetimeformatter." );
		return false;
	}

	KeywordArgsCit cit = token.kwargs->find( "format" );
	if ( cit == token.kwargs->cend() )
	{
		PyErr_SetString( PyExc_SyntaxError, "Missing format parameter." );
		return false;
	}

	format = reinterpret_cast<wchar_t*>( PyUnicode_AsUnicode( cit->second ) );
	if ( ! format )
	{
		return false;
	}

	if ( PyTuple_Check( value ) && PyTuple_GET_SIZE( value ) == 9 )
	{
		timeInfo.tm_year  = ( int ) PyInt_AS_LONG ( PyTuple_GET_ITEM( value, 0 ) );
		timeInfo.tm_mon   = ( int ) PyInt_AS_LONG ( PyTuple_GET_ITEM( value, 1 ) );
		timeInfo.tm_mday  = ( int ) PyInt_AS_LONG ( PyTuple_GET_ITEM( value, 2 ) );
		timeInfo.tm_hour  = ( int ) PyInt_AS_LONG ( PyTuple_GET_ITEM( value, 3 ) );
		timeInfo.tm_min   = ( int ) PyInt_AS_LONG ( PyTuple_GET_ITEM( value, 4 ) );
		timeInfo.tm_sec   = ( int ) PyInt_AS_LONG ( PyTuple_GET_ITEM( value, 5 ) );
		timeInfo.tm_wday  = ( int ) PyInt_AS_LONG ( PyTuple_GET_ITEM( value, 6 ) );
		timeInfo.tm_yday  = ( int ) PyInt_AS_LONG ( PyTuple_GET_ITEM( value, 7 ) );
		timeInfo.tm_isdst = ( int ) PyInt_AS_LONG ( PyTuple_GET_ITEM( value, 8 ) );
		// Normalize to years since 1900, months since january, days since Sunday
		timeInfo.tm_year -= 1900;
		timeInfo.tm_mon  -= 1;
		timeInfo.tm_wday += 1;
	}
	else if ( PyLong_Check( value ) )
	{
		Be::Time timeVal = PyLong_AsLongLong( value );

		CcpDateTime dt;
		TimeAsDateTime( dt, timeVal );

		timeInfo.tm_year = dt.year - 1900;
		timeInfo.tm_mon = dt.month - 1;
		timeInfo.tm_mday = dt.day;
		timeInfo.tm_hour = dt.hour;
		timeInfo.tm_min = dt.minute;
		timeInfo.tm_sec = dt.second;
		timeInfo.tm_wday = dt.dayOfWeek;
		timeInfo.tm_yday = 1;
		timeInfo.tm_isdst = -1;
	}
	//Python maxint on macOS is 64bit which produces numbers of type int64 when parsed through YAML rather than long produced on Windows.
    else if ( ( PyInt_Check( value ) ) && ( sizeof ( long int ) == 8 ) )
    {
        Be::Time timeVal = PyInt_AsLong( value );

        CcpDateTime dt;
        TimeAsDateTime( dt, timeVal );

        timeInfo.tm_year = dt.year - 1900;
        timeInfo.tm_mon = dt.month - 1;
        timeInfo.tm_mday = dt.day;
        timeInfo.tm_hour = dt.hour;
        timeInfo.tm_min = dt.minute;
        timeInfo.tm_sec = dt.second;
        timeInfo.tm_wday = dt.dayOfWeek;
        timeInfo.tm_yday = 1;
        timeInfo.tm_isdst = -1;
    }
	else if ( PyFloat_Check( value ) )
	{
		time_t x = ( time_t ) PyFloat_AS_DOUBLE( value );
		gmtime_s ( &timeInfo, (const time_t*) & x );
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "Time value must be float (aka python time), long (aka blue time) or tuple (from time info)." );
		return false;
	}

	// see if we need to apply a delta, for instance China wants to display GMT+8 despite us running GMT.
	if ( g_settings.timeDelta != 0 )
	{
#ifdef _WIN32
		time_t x = _mkgmtime( &timeInfo );
#else
        time_t x = timegm( &timeInfo );
#endif
		x += g_settings.timeDelta;
		gmtime_s( &timeInfo, (const time_t*) &x );
	}

	setlocale( LC_TIME, LanguageIDToLocaleName( lang.id ) );
#if !_WIN32
	// Format may contain Windows-specific %# formats (%#c and %#x). We have to replace them with %x and %c.
	if( auto found = wcsstr( format, L"%#" ) )
	{
		std::wstring fmt = format;
		fmt.erase( found - format + 1, 1 );
		wcsftime( buffer, STACK_BUFFER_SIZE_LARGE, fmt.c_str(), &timeInfo );
	}
	else
#endif
	{
		wcsftime( buffer, STACK_BUFFER_SIZE_LARGE, format, &timeInfo );
	}
	std::wstring buf(buffer);

	return SimpleValueFormatter( token, lang, buf, kwargs, retVal );
}

// -------------------------------------------------------------
// Description:
//   Replaces a {[message]msgID]} with it's actual message.
// Arguments:
//   token - Token to be used for information about the formatting to use.
//   lang - Language the token is in. This is the same as the lang parameter passed into Parse.
//   value - The value we are formatting. This must be a valid message ID.
//   kwargs - Keyword arguments passed in from Python. This is the same as the kwargs parameter passed into Parse.
//   retval - Unicode stream that will receive the formatted value. This is the same as the retval parameter passed into Parse.
// Returns:
//   true on success, false if it encountered an error
// -------------------------------------------------------------
bool MessageFormatter( const Token& token, const Language& lang, PyObject* value, PyObject* kwargs, std::wstringstream& retVal )
{
	MessageID msgID;

	if ( PyLong_Check( value ) )
	{
		msgID = PyLong_AsUnsignedLongLong( value );
	}
	else if ( PyInt_Check( value ) )
	{
		msgID = PyInt_AsUnsignedLongLongMask( value );
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "messageid variable type expects a valid messageID as parameter." );
		return false;
	}

	PyObject* val = NULL;
	bool ret = NULL;

	MessageMapCit cit = lang.data.find( msgID );

	if ( cit == lang.data.cend() )
	{
		const MessageMap& mm = g_settings.languages[g_settings.defaultLanguage]->data;
		cit = mm.find(msgID);
		if ( cit == mm.cend() )
		{
			char tmp[128];
			sprintf_s(tmp, "Message ID %llu does not exists neither in requested nor in fallback language.", msgID );
			PyErr_SetString( PyExc_KeyError, tmp );
			return false;
		}
	}

	const MessageData& msg = *cit->second;

	if ( ! msg.tokens )
	{
		val = PyUnicode_FromWideChar( msg.text.c_str(), msg.text.size() );
	}
	else
	{
		TokenContainerCit titer = msg.tokens->cbegin();
		while (titer != msg.tokens->cend())
		{
			if (titer->second->variableType == VARIABLETYPE_MESSAGE)
			{
				// we do not allow nested message tokens to avoid endless recursion
				char tmp[128];
				sprintf_s(tmp, "Message ID %llu contains a [messageid] tag and therefore cannot be used as a [messageid] value", msgID );
				PyErr_SetString( PyExc_ValueError, tmp );
				return false;
			}
			titer++;
		}
		std::wstringstream text;
		if ( Parse( msg.text, lang, *( msg.tokens ), kwargs, text ) )
		{
			std::wstring tmp = text.str();
			val = PyUnicode_FromWideChar( tmp.c_str(), tmp.size() );
		}
	}

	if ( ! val )
	{
		return false;
	}

	ret = SimpleValueFormatter( token, lang, val, kwargs, retVal );
	Py_XDECREF( val );
	return true;
}

// -------------------------------------------------------------
// Description:
//   Round a double value to the requested precision.
//   IMPORTANT NOTE: 
//   It should be obvious, but in case it isn't to you: Once tempval > _UI64_MAX the rounding is unreliable.
// Arguments:
//   val - double value that should be rounded
//   decimals - the amount of digits we want to round to
// Return value:
//   The double value rounded to decimals.
// -------------------------------------------------------------
double round(double val, unsigned int decimals)
{
	//we obtain the sign to calculate positive always
	double sign = val != 0.0 ? fabs( val ) / val : 1.0;
	//shift decimal places
	double tempval = fabs( val * pow( (double) 10, (double) decimals ) );
	uint64_t tempint = (uint64_t) tempval;
	double decimalpart = tempval-tempint;

	// actual rounding
	if( decimalpart >= 0.5 )
	{
		tempval = ceil( tempval );
	}
	else
	{
		tempval = floor( tempval );
	}

	// and return the unshifted, rounded value
	return ( tempval * pow( (double) 10, -(double) decimals ) ) * sign;
}

// -------------------------------------------------------------
// Description:
//   Provides numeric formatting.
// Arguments:
//   out - wchar_t buffer that will contain the formatted number
//   value - The value we are formatting. This must be either an integer or float value.
//   numDigits - the amount of decimal places we want to round to.
//   leadingZeroes - amount of leading zeroes to consider.
//   useGrouping - whether the number should be formatted with thousand separators or not.
// Return value:
//   Length of the output buffer
// -------------------------------------------------------------
size_t FormatNumber( wchar_t (&out)[STACK_BUFFER_SIZE_LARGE], 
					 PyObject* value, 
					 int numDigits /*= 2*/, 
					 int leadingZeroes /*= 0*/, 
					 bool useGrouping /*= false*/ )
{
	const Language& lang = *g_settings.languages[LANGUAGEID_SYSTEM_DEFAULT];
	size_t len = 0;
	wchar_t number[STACK_BUFFER_SIZE_LARGE];
	number[0] = 0x0000;
	out[0] = 0x0000;

	// wsprintf does not support floating point numbers correctly, alas we need to use _snwprintf
	if ( PyInt_Check( value ) )
	{
#ifdef _WIN32
		len = _snwprintf_s( number, STACK_BUFFER_SIZE_LARGE, L"%I32d", PyInt_AS_LONG( value ) );
#else
        len = swprintf( number, STACK_BUFFER_SIZE_LARGE, L"%ld", PyInt_AS_LONG( value ) );
#endif
	}
	else if ( PyLong_Check( value ) )
	{
#ifdef _WIN32
		len = _snwprintf_s( number, STACK_BUFFER_SIZE_LARGE, L"%I64d", PyLong_AsLongLong( value ) );
#else
		static_assert( sizeof( PY_LONG_LONG ) == sizeof( long long int ) );
        len = swprintf( number, STACK_BUFFER_SIZE_LARGE, L"%lld", PyLong_AsLongLong( value ) );
#endif
	}
	else if ( PyFloat_Check( value ) )
	{
#ifdef _WIN32
		len = _snwprintf_s( number, STACK_BUFFER_SIZE_LARGE, L"%.*f", numDigits, round( PyFloat_AS_DOUBLE( value ), numDigits ) );
#else
        len = swprintf( number, STACK_BUFFER_SIZE_LARGE, L"%.*f", numDigits, round( PyFloat_AS_DOUBLE( value ), numDigits ) );
#endif
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "Numeric Formatter expects floating point or signed integer types." );
		return NULL;
	}

	// find the position of the decimal point
	size_t posDecimalSep = 0;
	wchar_t *p = &number[0];
	if ( *p == L'-' )
	{
		++p;
	}
	for ( ; *p != L'\0'; ++p, ++posDecimalSep ) 
	{
		if ( *p < L'0' || *p > L'9' )
		{
			break;
		}
	}
	
	wchar_t* pOut = &out[0];
	p = &number[0];
	if ( *p == L'-' )
	{
		*pOut++ = *p++;
	}

	// inject leading zeroes, if required
	int requiredLeadingZeroes = std::max( 0, leadingZeroes - (int) posDecimalSep );
	size_t remainder = requiredLeadingZeroes == 0 ? posDecimalSep % GROUPING_VALUE : leadingZeroes % GROUPING_VALUE;
	size_t groupPos = remainder > 0 ? remainder : GROUPING_VALUE;

	// Do the actual conversion
	for ( size_t i = 0 ; *p != L'\0'; ++i )
	{
		if ( ( *p < L'0' || *p > L'9' ) )
		{
			// replace decimal symbol
			*pOut++ = lang.decimalSep;
			++p;
		}
		else if ( i == groupPos && useGrouping )
		{
			// we need to inject the separator symbol
			*pOut++ = lang.thousandSep;
			groupPos += GROUPING_VALUE;
			++len;
		}

		if ( requiredLeadingZeroes-- > 0 )
		{
			*pOut++ = L'0';
			++len;
		}
		else
		{
			*pOut++ = *p++;
		}
	}

	*pOut = L'\0';

	return std::min( len , size_t( STACK_BUFFER_SIZE_SMALL ) );
}

// -------------------------------------------------------------
// Description:
//   Provides numeric formatting.
// Arguments:
//   token - Token to be used for information about the formatting to use.
//   lang - Language the token is in. This is the same as the lang parameter passed into Parse.
//   value - The value we are formatting. This must be either an integer or float value.
//   kwargs - Keyword arguments passed in from Python. This is the same as the kwargs parameter passed into Parse.
//   retval - Unicode stream that will receive the formatted value. This is the same as the retval parameter passed into Parse.
// Returns:
//   true on success, false if it encountered an error
// -------------------------------------------------------------
bool NumericFormatter( const Token& token, const Language& lang, PyObject* value, PyObject* kwargs, std::wstringstream& retVal )
{
	int decimalPlaces = 2;
	int leadingZeroes = 0;
	wchar_t out[STACK_BUFFER_SIZE_LARGE];
	size_t result = 0;

	// If the propertyhandler calls out to GetByLabel (f.e. when we have .isk or .aur properties)
	// then this can happen. Alas, safe to ignore for the moment, but this might be detectable in the tokenizer.
	if ( PyUnicode_Check( value ) )
	{
		std::wstring tmp( PyUnicodeToWString( value ) );
		retVal << tmp;
		return true;
	}
	else if ( PyString_Check( value ) )
	{
		PyObject* o = PyObject_Unicode( value );
		if ( o )
		{
			std::wstring tmp( PyUnicodeToWString( o ) );
			retVal << tmp;
		}
		else
		{
			return false;
		}
		Py_XDECREF( o );
		return true;
	}

	if ( TOKENFLAG_DECIMALPLACES == ( token.flags & TOKENFLAG_DECIMALPLACES ) )
	{
		decimalPlaces = std::max( 0, std::min( 9, int( PyInt_AS_LONG( token.kwargs->find("decimalPlaces")->second ) ) ) );
	}

	if ( TOKENFLAG_LEADINGZEROES == ( token.flags & TOKENFLAG_LEADINGZEROES ) )
	{
		leadingZeroes = std::max( 0, std::min( 9, int( PyInt_AS_LONG( token.kwargs->find("leadingZeroes")->second ) ) ) );
	}

	result = FormatNumber( out, value, decimalPlaces, leadingZeroes, ( TOKENFLAG_USEGROUPING == ( token.flags & TOKENFLAG_USEGROUPING ) ) );

	if ( 0 == result )
	{
		return false;
	}

	retVal << out;
	return true;
}
