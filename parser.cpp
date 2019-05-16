#include "stdafx.h"
#include "parser.h"
#include <algorithm>
#include <iostream>
#include <string>
#include "localization.h"
#include "formatters.h"

PyObject* CallPropertyHandler( const Token& token, LanguageID langID, PyObject* value, PyObject* kwargs );
bool ProcessToken( const Token& token, const Language& lang, PyObject* kwargs, std::wstringstream& retVal );
bool ProcessConditional( const Token& token, const Language& lang, PyObject* value, std::wstringstream& retVal );

// -------------------------------------------------------------
// Description:
//   The parse method replaces Cerberus markup with actual values that should go into them.
// Arguments:
//   sourceText - The source string to parse.
//   lang - The language sourceText is in.
//   tokens - All tokens associated with this sourceText.
//   kwargs - The keyword argument dictionary as passed in from python.
//   retVal - A unicode stream that will be filled with the result of the parsing process.
// Return value:
//   True on success, false on failure
// -------------------------------------------------------------
bool Parse( const std::wstring& sourceText, const Language& lang, const TokenContainer& tokens, PyObject* kwargs, std::wstringstream& retVal )
{
	size_t pos = -1, oldpos = 0, tokenIndex = 0;
	while ( std::wstring::npos != ( pos = sourceText.find( L'{', ++pos ) ) )
	{
		// find closing bracket
		size_t epos = sourceText.find( L'}', pos );

		if ( epos == std::wstring::npos )
		{
			PyErr_SetString( PyExc_SyntaxError, "Missing a closing curly bracket." );
			return false;
		}

		++epos;
		TokenContainerCit token = tokens.find( sourceText.substr( pos, epos - pos ) );

		if ( token == tokens.end() )
		{
			PyErr_SetString( PyExc_RuntimeError, "Missing a token." );
			return false;
		}

		retVal << sourceText.substr( oldpos, pos-oldpos );

		if ( NULL == ProcessToken( *token->second, lang, kwargs, retVal ) )
		{
			// ProcessToken sets the exception, we just need to bubble it up.
			return false;
		}
		
		oldpos = epos;
	}

	retVal << sourceText.substr( oldpos, sourceText.size() - oldpos );
	return true;
}

// -------------------------------------------------------------
// Description:
//   ProcessToken determines the actual value for markup detected during the parsing process.
// Arguments:
//   token - Token to be used for determining the actual value.
//   lang - Language the token is in. This is the same as the lang parameter passed into Parse.
//   kwargs - Keyword arguments passed in from Python. This is the same as the kwargs parameter passed into Parse.
//   retval - Unicode stream that will receive the determined value. This is the same as the retval parameter passed into Parse.
// Return value:
//   True on success, false on failure
// See Also:
//   Parse
// -------------------------------------------------------------
bool ProcessToken( const Token& token, const Language& lang, PyObject* kwargs, std::wstringstream& retVal )
{
	PyObject* value = PyDict_GetItemString( kwargs, token.variableName.c_str() );

	// Call out to the python implemented property handler. This cannot be moved to C++ since properties need access to Cfg.
	if ( token.propertyName.size() )
	{
		value = CallPropertyHandler( token, lang.id, value, kwargs );
	}

	if ( ! value )
	{
		// report error if we have not done so.
		if ( ! PyErr_Occurred() )
		{
		    std::stringstream errStr;
		    errStr << "Failed to find the value for '" << token.variableName << "' in kwargs or via handler for of type " << token.variableType << " for property with name '" << token.propertyName << "'.";
		    PyErr_SetString( PyExc_RuntimeError, errStr.str().c_str() );
		}
		retVal << "ERROR parsing message: Token has no value";
		return false;
	}

	bool ret = true;

	if ( TOKENFLAG_CONDITIONAL == ( token.flags & TOKENFLAG_CONDITIONAL ) )
	{
		ret = ProcessConditional( token, lang, value, retVal );
	}
	else
	{
		switch ( token.variableType )
		{
			case VARIABLETYPE_NUMERIC:
				ret = NumericFormatter( token, lang, value, kwargs, retVal );
				break;
			case VARIABLETYPE_MESSAGE:
				ret = MessageFormatter( token, lang, value, kwargs, retVal );
				break;
			case VARIABLETYPE_DATETIME:
			case VARIABLETYPE_FORMATTEDTIME:
				ret = DateTimeFormatter( token, lang, value, kwargs, retVal );
				break;
			default:
				ret = SimpleValueFormatter( token, lang, value, kwargs, retVal );
				break;
		};
	}
	return ret;
}

// -------------------------------------------------------------
// Description:
//   Calls out to python to retrieve the value for the given token's property. This will also manipulate
//   the kwargs dictionary in case we are dealing with quantities, because that is where it's expected.
// Arguments:
//   token - Token we are currently processing.
//   lang - Language the token is in. This is the same as the lang parameter passed into Parse.
//   value - PyObject containing the value from kwargs that we are going to operate upon.
//   kwargs - Keyword arguments passed in from Python. This is the same as the kwargs parameter passed into Parse.
// Return value:
//   A PyObject* containing the value retrieved from Python. This should be unicode encoded.
// -------------------------------------------------------------
PyObject* CallPropertyHandler( const Token& token, LanguageID langID, PyObject* value, PyObject* kwargs )
{
	PropertyHandlerMapCit cit = g_settings.propertyHandlerMap.find( token.variableType );
	if ( cit == g_settings.propertyHandlerMap.cend() )
	{
		PyErr_SetString( PyExc_KeyError, "We are missing a property handler!" );
		return NULL;
	}

	PyObject* tokenKwargs = PyDict_New();
	if ( token.kwargs )
	{
		for ( KeywordArgs::const_iterator cit = token.kwargs->begin(); cit != token.kwargs->end(); ++cit )
		{
			PyDict_SetItemString( tokenKwargs, cit->first.c_str(), cit->second );
			// We can specify quantity as being not equal to the variableName, but rather
			// a kwarg in the token that tells us which kwarg from the method call to take.
			// So we just use that. This works because these things to not have the quantity flag set.
			if ( strcmp( cit->first.c_str(), "quantity" ) == 0 )
			{
				PyObject* tmp = PyDict_GetItem( kwargs, cit->second );
				if ( tmp )
				{
					PyDict_SetItemString( tokenKwargs, "dereferencedQuantity", tmp );
				}
			}
		}
	}

	// special casing: We need to inject the quantity value as "dereferencedQuantity"
	if ( TOKENFLAG_QUANTITY == ( token.flags & TOKENFLAG_QUANTITY ) )
	{
		// According to mark up rules then we can avoid specifying the actual quantity value, 
		// in which case the property handler will assume that it should go for 1 as a value.
		PyObject* quantity = PyDict_GetItemString( kwargs, token.variableName.c_str() );
		if ( quantity )
		{
			PyDict_SetItemString( tokenKwargs, "dereferencedQuantity", quantity );
		}
	}

	PyObject* ret = NULL;
	PyObject* args = Py_BuildValue( "(sOs)", token.propertyName.c_str(), value, LanguageIDToCode( langID ) );
	if ( args )
	{
		PyObject* method = PyObject_GetAttrString( cit->second, "GetProperty" );
		ret = PyObject_Call( method, args, tokenKwargs );
		if ( ret && ( token.flags & TOKENFLAG_LINKIFY ) == TOKENFLAG_LINKIFY )
		{
			ret = PyObject_CallMethod( cit->second, (char*)"Linkify", (char*)"OO", value, ret );
		}
		Py_XDECREF( method );
	}
	Py_XDECREF( args );
	Py_XDECREF( tokenKwargs );
	return ret;
}

// -------------------------------------------------------------
// Description:
//   In case of a conditional markup tag (f.e. {[numeric]sec -> "second", "seconds"}) we are determining which of the
//   conditional values to use depending upon the value that was passed in.
//   Which conditional value to choose is eventually decided by the quantity category handlers.
// Arguments:
//   token - Token we are currently processing.
//   lang - Language the token is in. This is the same as the lang parameter passed into Parse.
//   value - PyObject containing the value from kwargs that we are going to operate upon.
//   retVal - Unicode stream we append the conditional value to. This is the same as the retVal parameter passed into Parse.
// Return value:
//   True on success, false on failure
// -------------------------------------------------------------
bool ProcessConditional( const Token& token, const Language& lang, PyObject* value, std::wstringstream& retVal )
{
	// In this case, value indicates an integer. Or should at least.
	size_t index = 0;

	int val = 0;

	if ( PyFloat_Check ( value ) )
	{
		val = (int) ceil( PyFloat_AS_DOUBLE( value ) );
	}
	else if ( PyInt_Check ( value ) )
	{
		val = PyInt_AS_LONG( value );
	}
	else if ( PyLong_Check ( value ) )
	{
		val = (int) PyLong_AsLongLong( value );
	}
	else
	{
		PyErr_SetString( PyExc_TypeError, "Conditional value only be determined for integer or float values." );
		return false;
	}

	if ( TOKENFLAG_QUANTITY == ( token.flags & TOKENFLAG_QUANTITY ) )
	{
		index = lang.quantityCategoryFunc( val );
	}
	else
	{
		// figure out how many conditionalValues we actually have.
		int numValues = -1;
		for ( int i = 0; i < 3; ++i )
		{
			if ( token.conditionalValues[i].empty() )
			{
				break;
			}

			++numValues;
		}
		// Not a quantity, so we assume that we can simply use value.
		index = (size_t) max( (int)0, min( numValues, val ) );
	}

	retVal << (token.conditionalValues[index]);
	return true;
}

// -------------------------------------------------------------
// Description:
//   Quantity category selector for languages that only have one quantity category.
// Arguments:
//   quantity - the value to base the category decision on.
// Return value:
//   Index in the token's conditionalValues to use. In this particular case always 0.
// See Also:
//   ProcessConditional
// -------------------------------------------------------------
size_t GetType1QuantityCategory( __int64 quantity )
{
	return 0;
}

// -------------------------------------------------------------
// Description:
//   Quantity category selector for languages that have two quantity categories.
// Arguments:
//   quantity - the value to base the category decision on.
// Return value:
//   Index in the token's conditionalValues to use.
// See Also:
//   ProcessConditional
// -------------------------------------------------------------
size_t GetType2QuantityCategory( __int64 quantity )
{
	if ( 1 == quantity || -1 == quantity )
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

// -------------------------------------------------------------
// Description:
//   Quantity category selector for languages that have three quantity categories.
// Arguments:
//   quantity - the value to base the category decision on.
// Return value:
//   Index in the token's conditionalValues to use. In this particular case always 0.
// See Also:
//   ProcessConditional
// -------------------------------------------------------------
size_t GetType3QuantityCategory( __int64 quantity )
{
	quantity = abs( quantity );

	if ( quantity % 10 == 1 && quantity % 100 != 11 )
	{
		return 0;
	}
	else if ( ( ( 2 <= quantity % 10 ) && ( quantity % 10 <= 4 ) ) && ( ( quantity < 12 ) || ( quantity > 14 ) ) )
	{
		return 1;
	}
	else
	{
		return 2;
	}
}
