#include "stdafx.h"

#include "parser.h"
#include "localization.h"

// -------------------------------------------------------------
// Description:
//   Registers a property handler that is being used to retrieve the value for properties of a given variable type.
// Arguments:
//   module - ignored
//   args - argument tuple. Must contain the variable type and a python object that implements a GetProperty method.
// -------------------------------------------------------------
PyObject* PyRegisterPropertyHandler( PyObject* module, PyObject* args )
{
	VariableType varType;
	PyObject* handler;
	if ( PyArg_ParseTuple( args, "iO", &varType, &handler ) )
	{
		if ( varType < 0 || varType >= VARIABLETYPE_MAX )
		{
			PyErr_SetString( PyExc_TypeError, "Unknown variable type" );
			return NULL;
		}

		Py_XINCREF( handler );
		PropertyHandlerMapIt it = g_settings.propertyHandlerMap.find( varType );
		if ( it != g_settings.propertyHandlerMap.end() )
		{
			Py_XDECREF( it->second );
			it->second = handler;
		}
		else
		{
			g_settings.propertyHandlerMap.insert( std::pair< VariableType, PyObject* >( varType, handler ) );
		}
	}

	Py_RETURN_NONE;
}
MAP_FUNCTION( "RegisterPropertyHandler", PyRegisterPropertyHandler, "Registers a handler class instance for a specific variable type." );

// -------------------------------------------------------------
// Description:
//   Parses the passed in string with provided language and tokens instead of fetching them from the messagedata storage.
//   The language must have been loaded before, e.g. by loading messagedata for said language, otherwise a KeyError will be raised.
// Arguments:
//   module - ignored
//   args - argument tuple. Must contain sourceText, languageID and token dictionary
//   kwargs - keyword argument dictionary.
// -------------------------------------------------------------
PyObject* PyParse( PyObject* module, PyObject* args, PyObject* kwargs )
{
	std::wstringstream retVal;
	TokenContainer tokens;
	wchar_t* sourceText;
	PyObject* tokenDict;
	char* languageCode;

	if ( PyArg_ParseTuple( args, "usO", &sourceText, &languageCode, &tokenDict ) )
	{
		LanguageID langID = CodeToLanguageID( languageCode );

		LanguageMapCit lang = g_settings.languages.find( langID );
		if ( lang == g_settings.languages.cend() )
		{
			char tmp[128];
			sprintf_s(tmp, "Language %s does not exist.", languageCode );
			PyErr_SetString( PyExc_KeyError, tmp );
			return NULL;
		}

		MessageData md;
		if ( LoadTokens( tokenDict, md ) )
		{
			std::wstringstream text;
			if ( md.tokens )
			{
				bool ret = Parse( sourceText, *(lang->second), *( md.tokens ), kwargs, text );
				if ( ! ret )
				{
					// Actual exception object should be set in Parse.
					return NULL;
				}
			}
			else
			{
				text << sourceText;
			}
			std::wstring tmp = text.str();
			return PyUnicode_FromWideChar( tmp.c_str(), tmp.size() );			
		}
	}

	return NULL;
}
// Mapping from localization.cpp due to kwargs
