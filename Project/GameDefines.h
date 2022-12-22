#pragma once

// Defines for Unicode
#include <string>		
#include <sstream>
#include <iostream>
#include <fstream>
#include <tchar.h>
using namespace std;

#ifdef _UNICODE								// extra unicode defines
	#define tstring			wstring
	#define tcin			wcin
	#define tcout			wcout
	#define tstringstream	wstringstream
	#define tofstream		wofstream
	#define tifstream		wifstream
	#define tfstream		wfstream
	#define tostream		wostream
	#define to_tstring		to_wstring
#else
	#define tstring			string
	#define tcin			cin
	#define tcout			cout
	#define tstringstream	stringstream
	#define tofstream		ofstream
	#define tifstream		ifstream
	#define tfstream		fstream
	#define tostream		ostream
	#define to_tstring		to_string
#endif

//64 bit defines
#ifdef _WIN64
#define GWLA_WNDPROC	GWLP_WNDPROC
#define GWLA_HINSTANCE	GWLP_HINSTANCE
#define GWLA_HWNDPARENT GWLP_HWNDPARENT
#define GWLA_USERDATA	GWLP_USERDATA
#else if
#define GWLA_WNDPROC	GWL_WNDPROC
#define GWLA_HINSTANCE	GWL_HINSTANCE
#define GWLA_HWNDPARENT GWL_HWNDPARENT
#define GWLA_USERDATA	GWL_USERDATA
#endif

// ASSERT macro
#ifndef NDEBUG
#ifdef _WIN64
#define ASSERT \
if ( false ) {} \
else \
struct LocalAssert { \
	int mLine; \
	LocalAssert(int line=__LINE__) : mLine(line) {} \
	LocalAssert(bool isOK, const TCHAR* message=_T("")) { \
		if ( !isOK ) { \
			tstringstream buffer; \
			buffer << _T("ERROR!! Assert failed on line ") << LocalAssert().mLine << _T(" in file '") << __FILE__ << _T("'\nMessage: \"") << message << _T("\"\n"); \
			OutputDebugString(buffer.str().c_str()); \
				} \
	} \
} myAsserter = LocalAssert
#else
#define ASSERT \
if ( false ) {} \
else \
struct LocalAssert { \
	int mLine; \
	LocalAssert(int line=__LINE__) : mLine(line) {} \
	LocalAssert(bool isOK, const TCHAR* message=_T("")) { \
		if ( !isOK ) { \
			tstringstream buffer; \
			buffer << _T("ERROR!! Assert failed on line ") << LocalAssert().mLine << _T(" in file '") << __FILE__ << _T("'\nMessage: \"") << message << _T("\"\n"); \
			OutputDebugString(buffer.str().c_str()); \
			__asm { int 3 } \
						} \
		} \
} myAsserter = LocalAssert
#endif
#else
#define ASSERT \
if ( true ) {} else \
struct NoAssert { \
	NoAssert(bool isOK, const TCHAR* message=_T("")) {} \
} myAsserter = NoAssert
#endif

// GAME_ENGINE define
#define GAME_ENGINE (GameEngine::GetSingleton())
