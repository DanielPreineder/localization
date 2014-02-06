#pragma once
#ifndef Formatters_H
#define Formatters_H

#include <iostream>
#include <Blue/Include/Blue.h>

struct Token;
struct Language;

extern bool SimpleValueFormatter( const Token& token, const Language& lang, PyObject* value, PyObject* kwargs, std::wstringstream& retVal );
extern bool SimpleValueFormatter( const Token& token, const Language& lang, std::wstring& val, PyObject* kwargs, std::wstringstream& retVal );
extern bool DateTimeFormatter( const Token& token, const Language& lang, PyObject* value, PyObject* kwargs, std::wstringstream& retVal );
extern bool MessageFormatter( const Token& token, const Language& lang, PyObject* value, PyObject* kwargs, std::wstringstream& retVal );
extern bool NumericFormatter( const Token& token, const Language& lang, PyObject* value, PyObject* kwargs, std::wstringstream& retVal );

#endif // Formatters_H