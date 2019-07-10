// localization.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

#include "localization.h"

#include <Blue/Include/Blue.cxx>

#include "parser.h"

#ifndef BLUE_DLL_NAME
#error Please add BLUE_DLL_NAME=<PythonModuleName> to compiler preprocessor definitions (/D)
#endif

const char* g_moduleName = "_evelocalization";
LocalizationSettings g_settings;

// -------------------------------------------------------------
// Description:
//   Constructs a language object with all information for the passed in locale
// Arguments:
//   langID - The languageID for which to retrieve locale information.
// -------------------------------------------------------------
Language::Language( LanguageID langID /*= Default_Language*/ ) : id( langID ),  quantityCategoryFunc( NULL )
{
	switch ( langID )
	{
	case LANGUAGEID_CHINESE_SIMPLIFIED:
	case LANGUAGEID_JAPANESE:
		quantityCategoryFunc = GetType1QuantityCategory;
		break;
	case LANGUAGEID_RUSSIAN:
		quantityCategoryFunc = GetType3QuantityCategory;
		break;
	default:
		quantityCategoryFunc = GetType2QuantityCategory;
		break;
	};

	locale = std::locale( LanguageIDToLocaleName( langID ) );

	decimalSep = L'.';
	thousandSep = L',';

	// MSDN says: maximum 4 characters for LOCALE_SDECIMAL and LOCALE_STHOUSAND, including terminating NULL
	wchar_t temp[4];
	// Note that we are only supporting a single character as grouping and decimal symbol,
	// and thus we check that we get more than just the terminating NULL returned.
	if ( GetLocaleInfoW( langID, LOCALE_SDECIMAL, temp, 4 ) > 1 )
	{
		decimalSep = temp[0];
	}
	if ( GetLocaleInfoW( langID, LOCALE_STHOUSAND, temp, 4 ) > 1 )
	{
		thousandSep = temp[0];
	}
}

// -------------------------------------------------------------
// Description:
//   Load a single token from its dictionary representation into a Token structure.
// Arguments:
//   dict - PyObject containing the dictionary with the token information.
//   token - Token instance in which we store the token information.
// Return value:
//   True if the token could be loaded, False otherwise.
// -------------------------------------------------------------
bool LoadToken( PyObject* dict, Token& token )
{
	Py_ssize_t p = 0;
	PyObject* key;
	PyObject* value;

	if ( ! PyDict_Check ( dict ) )
	{
		PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - LoadToken expects a dictionary." );
		return false;
	}

	while ( PyDict_Next( dict, &p, &key, &value ) )
	{
		if ( ! PyString_Check( key ) )
		{
			PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - token must be keyed on ascii string" );
			return false;
		}
		char* keyText = PyString_AsString( key );

		if ( 0 == strcmp( keyText, "args" ) )
		{
			if ( ! PyLong_Check( value ) &&  ! PyInt_Check( value ) )
			{
				PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - args must be integer value." );
				return false;
			}
			token.flags = PyLong_AsLong( value );
		}
		else if ( 0 == strcmp( keyText, "variableType" ) && Py_None != value )
		{
			if ( ! PyInt_Check( value ) )
			{
				PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - variableType must be an integer." );
				return false;
			}
			token.variableType = (VariableType) PyInt_AsLong( value );
		}
		else if ( 0 == strcmp( keyText, "variableName" ) && Py_None != value )
		{
			if ( ! PyString_Check( value ) )
			{
				PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - variableName must be ascii string." );
				return false;
			}
			token.variableName = std::string( PyString_AS_STRING( value ) );
		}
		else if ( 0 == strcmp( keyText, "propertyName" ) && Py_None != value )
		{
			if ( ! PyString_Check( value ) )
			{
				PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - propertyName must be ascii string." );
				return false;
			}
			token.propertyName = std::string( PyString_AS_STRING( value ) );
		}
		else if ( 0 == strcmp( keyText, "kwargs" ) )
		{
			if ( ! PyDict_Check( value ) )
			{
				PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - kwargs must be a dictionary." );
				return false;
			}
			PyObject* k;
			PyObject* v;
			Py_ssize_t s = 0;
			while ( PyDict_Next( value, &s, &k, &v ) )
			{
				if ( ! PyString_Check( k ) )
				{
					PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - kwarg key must be ascii string." );
					return false;
				}
				
				if ( ! PyString_Check( v ) && ! PyInt_Check( v ) && ! PyUnicode_Check( v ) )
				{
					PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - kwarg value must be unicode, string or int." );
					return false;
				}

				std::string key( PyString_AS_STRING( k ) );

				if ( ! token.kwargs )
				{
					token.kwargs = CCP_NEW( "eveLocalization/KeywordArgs" ) KeywordArgs();
				}

				Py_XINCREF( v );
				token.kwargs->insert( KeywordArgs::value_type( key, v ) );
			}
		}
		else if ( 0 == strcmp( keyText, "conditionalValues" ) )
		{
			if ( ! PyList_Check( value ) )
			{
				PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - conditionalValues must be a list." );
				return false;
			}

			Py_ssize_t len = PyList_Size( value );

			if ( len > 3 )
			{
				PyErr_SetString( PyExc_ValueError, "Localization::ParseToken - too many conditionalValues to unpack" );
				return false;
			}

			for ( Py_ssize_t i = 0; i < len; ++i )
			{
				PyObject* item = PyList_GetItem( value, i );
				if ( ! PyUnicode_Check ( item ) )
				{
					PyErr_SetString( PyExc_TypeError, "Localization::ParseToken - conditionalValues must be unicode strings" );
					return false;
				}
				std::wstring result( PyUnicode_AS_UNICODE( item ), PyUnicode_GET_SIZE( item ) );
				token.conditionalValues[i] = result;
			}
		}
	}

	// -- BEGIN NO TOKENIZER UPDATES POSSIBLE WORKAROUND --
	// Temporary work around until we update the pickle generator again
	if ( token.variableType == VARIABLETYPE_DATETIME || token.variableType == VARIABLETYPE_FORMATTEDTIME )
	{
		// inject the formatting string
		if ( ! token.kwargs )
		{
			token.kwargs = CCP_NEW( "eveLocalization/KeywordArgs" ) KeywordArgs();
		}
		if ( token.kwargs->find( "format" ) == token.kwargs->cend() )
		{
			token.kwargs->insert( KeywordArgs::value_type( "format", PyUnicode_FromString( "%Y.%m.%d %H:%M" ) ) );
		}
	}

	// timeinterval needs special handling as it's the only property handler with a default property that does not return the input value
	if ( token.variableType == VARIABLETYPE_TIMEINTERVAL )
	{
		// inject the default property name
		if ( token.propertyName.empty() )
		{
			token.propertyName = "shortForm";
		}
	}

	// see if we need leading zeroes
	if ( token.kwargs && token.kwargs->find( "leadingZeroes" ) != token.kwargs->cend() )
	{
		token.flags |= TOKENFLAG_LEADINGZEROES;
	}

	// Quantity kwarg superseeds a quantity flag because the quantity comes from kwargs
	if ( token.kwargs && token.kwargs->find( "quantity" ) != token.kwargs->cend() )
	{
		if ( ( token.flags & TOKENFLAG_QUANTITY ) == TOKENFLAG_QUANTITY )
		{
			token.flags ^= TOKENFLAG_QUANTITY;
		}
	}
	// -- END WORKAROUND --

	return true;
}

// -------------------------------------------------------------
// Description:
//   Loads tokens from a python dictionary into a MessageData structure.
// Arguments:
//   dict - Dictionary in the form of {messageID: tokenDictionary, ...}.
//   messageData - reference to a MessageData instance which will be populated with the parsed tokens.
// -------------------------------------------------------------
bool LoadTokens( PyObject* dict, MessageData& messageData )
{
	Py_ssize_t p = 0;
	PyObject* key;
	PyObject* value;

	p = PyDict_Size( dict );
	if ( p == 0 )
	{
		// We probably want to log a warning here as this case should never happen
		return true;
	}

	p = 0;

	while ( PyDict_Next( dict, &p, &key, &value ) )
	{
		if ( ! ( PyUnicode_Check( key ) ) )
		{
			PyErr_SetString( PyExc_TypeError, "Localization::ParseTokens - token dictionary must be keyed on unicode string." );
			return false;
		}

		Token* token = CCP_NEW( "LoadTokens/token" ) Token;
		if ( ! LoadToken( value, *token ) )
		{
			CCP_DELETE token;

			// error message was set from within ParseToken
			return false;
		}

		token->tagName = std::wstring( PyUnicode_AS_UNICODE( key ), PyUnicode_GET_SIZE( key ) );

		// temporarily keep support for linkify
		// TODO: Does this really have to happen on load time? <snorri>
		if ( token->tagName.find( L"linkify" ) != std::wstring::npos )
		{
			token->flags |= TOKENFLAG_LINKIFY;
		}

		if ( token->tagName.find( L"linkinfo" ) != std::wstring::npos )
		{
			token->flags |= TOKENFLAG_LINKINFO;
		}

		if ( ! messageData.tokens )
		{
			messageData.tokens = CCP_NEW( "eveLocalization/Tokens" ) TokenContainer();
		}

		messageData.tokens->insert( TokenContainer::value_type( token->tagName, token ) );
	}

	return true;
}

// -------------------------------------------------------------
// Description:
//   Convert python language codes into a LanguageID
// Arguments:
//   code - string containing the python language code.
// Return value:
//   The LanguageID
// -------------------------------------------------------------
LanguageID CodeToLanguageID( const char* code )
{
	if ( 0 == _stricmp( code, "en-us" ) )   return LANGUAGEID_ENGLISH_US;
	if ( 0 == _stricmp( code, "en" ) )      return LANGUAGEID_ENGLISH_US;
	if ( 0 == _stricmp( code, "ja" ) )      return LANGUAGEID_JAPANESE;
	if ( 0 == _stricmp( code, "ko" ) )      return LANGUAGEID_KOREAN;
	if ( 0 == _stricmp( code, "zh" ) )      return LANGUAGEID_CHINESE_SIMPLIFIED;
	if ( 0 == _stricmp( code, "de" ) )      return LANGUAGEID_GERMAN;
	if ( 0 == _stricmp( code, "fr" ) )		return LANGUAGEID_FRENCH;
	if ( 0 == _stricmp( code, "it" ) )		return LANGUAGEID_ITALIAN;
	if ( 0 == _stricmp( code, "es" ) )		return LANGUAGEID_SPANISH;
	if ( 0 == _stricmp( code, "ru" ) )      return LANGUAGEID_RUSSIAN;
	if ( 0 == _stricmp( code, "" ) )        return LANGUAGEID_SYSTEM_DEFAULT;


	// default to english
	assert ( 0 && "There is a missing language code mapping in the localization module, please update enum and mapping." );
	return LANGUAGEID_ENGLISH_US;
}

// -------------------------------------------------------------
// Description:
//   Convert a LanguageID to its default Python representation.
// Arguments:
//   languageID - the LanguageID to convert.
// Return value:
//   String containing the python language code.
// -------------------------------------------------------------
const char* LanguageIDToCode( LanguageID languageID )
{
	switch( languageID )
	{
		case LANGUAGEID_CHINESE_SIMPLIFIED:
			return "zh";
		case LANGUAGEID_GERMAN:
			return "de";
		case LANGUAGEID_RUSSIAN:
			return "ru";
		case LANGUAGEID_FRENCH:
			return "fr";
		case LANGUAGEID_ITALIAN:
			return "it";
		case LANGUAGEID_SPANISH:
			return "es";
		case LANGUAGEID_JAPANESE:
			return "ja";
		case LANGUAGEID_KOREAN:
		    return "ko";
		case LANGUAGEID_SYSTEM_DEFAULT:
			return "";
		case LANGUAGEID_ENGLISH_US:
		default:
			return "en-us";
	};
}

// -------------------------------------------------------------
// Description:
//   Converts a LanguageID to its locale name.
//   For a list of locale names see: http://msdn.microsoft.com/en-us/library/39cwe7zf.aspx
// Arguments:
//   languageID - LanguageID for which we want to get the locale name
// Return value:
//   String containing the locale name for the passed in LanguageID.
// -------------------------------------------------------------
const char* LanguageIDToLocaleName( LanguageID languageID )
{
	switch( languageID )
	{
		case LANGUAGEID_CHINESE_SIMPLIFIED:
			return "chinese-simplified";
		case LANGUAGEID_GERMAN:
			return "german";
		case LANGUAGEID_RUSSIAN:
			return "russian";
		case LANGUAGEID_FRENCH:
			return "french";
		case LANGUAGEID_ITALIAN:
			return "italian";
		case LANGUAGEID_SPANISH:
			return "spanish";
		case LANGUAGEID_JAPANESE:
			return "japanese";
		case LANGUAGEID_KOREAN:
		    return "korean";
		case LANGUAGEID_ENGLISH_US:
		default:
			return "english";
	};
};

//-----------------------------------------------------------------------------
// The globals
//-----------------------------------------------------------------------------
HINSTANCE gInstance = NULL;

static void StartDLL(HINSTANCE instance)
{	
	BeClasses->RegisterClasses( BlueRegistration::GetClassRegs() );
	
	PyObject* module = Py_InitModule( CCP_STRINGIZE( BLUE_DLL_NAME ), NULL);
	
	PyMethodDef pmd[] = { 
							{ 
								"GetMessageByID", 
								( PyCFunction ) PyGetMessageByID, 
								METH_VARARGS | METH_KEYWORDS, 
								"C implementation of Cerberus' _GetByMessageID." 
							},
							{ 
								"FormatNumeric", 
								( PyCFunction ) PyFormatNumeric, 
								METH_VARARGS | METH_KEYWORDS, 
								"C implementation of FormatNumeric." 
							},
							{
								"Parse",
								( PyCFunction ) PyParse,
								METH_VARARGS | METH_KEYWORDS,
								"C implementation of the cerberus parser."
							},
						};
	for ( size_t i = 0; i < ( sizeof( pmd ) / sizeof( pmd[0] ) ); ++i )
	{
		BlueRegistration::GetFuncRegs().push_back( pmd[i] );
	}

	BlueRegisterToModule( module, 
						  BlueRegistration::GetClassRegs(), 
						  BlueRegistration::GetFuncRegs(), 
						  BlueRegistration::GetEnumRegs() );
	
	//Init 3rd party libs here.
}


BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		gInstance = instance;
		DisableThreadLibraryCalls(instance);
	}
	else if (reason == DLL_PROCESS_DETACH)
	{
		;
	}
    return TRUE;
}

//-----------------------------------------------------------------------------
// init - python dll module entry function
//-----------------------------------------------------------------------------
#define CONCAT( a, b ) MY_CONCAT( a, b )
#define MY_CONCAT( a, b ) a##b
extern "C" void __declspec(dllexport) CONCAT( init, BLUE_DLL_NAME )()
{
	StartDLL(gInstance);
}
