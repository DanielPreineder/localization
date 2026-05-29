// Copyright © 2014 CCP ehf.
#pragma once
#ifndef Parser_H
#define Parser_H

#include <Python.h>
#include "localization.h"

extern bool Parse( const std::wstring& sourceText, const Language& lang, const TokenContainer& tokens, PyObject* kwargs, std::wstringstream& retVal );
extern PyObject* PyParse( PyObject* module, PyObject* args, PyObject* kwargs );

#endif // Parser_H
