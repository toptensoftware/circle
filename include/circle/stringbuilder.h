//
// string.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2020  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _circle_stringbuilder_h
#define _circle_stringbuilder_h

#include <circle/stdarg.h>
#include <circle/types.h>

class CStringBuilder
{
public:
	CStringBuilder (void);
	virtual ~CStringBuilder (void);

	int Replace (const char* psz, const char *pOld, const char *pNew); // returns number of occurrences

    void Format (const char *pFormat, ...);
	void FormatV (const char *pFormat, va_list Args);

	void PutChar (char chChar, size_t nCount = 1);
	void PutString (const char *pString);
	void ReserveSpace (size_t nSpace);

    char* Detach();
    void Attach(char* pString);

	static char *ntoa (char *pDest, unsigned long ulNumber, unsigned nBase, boolean bUpcase);
#if STDLIB_SUPPORT >= 1
	static char *lltoa (char *pDest, unsigned long long ullNumber, unsigned nBase, boolean bUpcase);
#endif
	static char *ftoa (char *pDest, double fNumber, unsigned nPrecision);

private:
	char 	 *m_pBuffer;
	unsigned  m_nSize;
	char	 *m_pInPtr;
};

extern "C" int string_replace(char** pVal, const char* psz, const char* pOld, const char* pNew);
extern "C" char* string_vsprintf(const char *pFormat, va_list Args);

#endif
