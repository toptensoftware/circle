//
// string.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2020  R. Stange <rsta2@o2online.de>
//
// ftoa() inspired by Arjan van Vught <info@raspberrypi-dmx.nl>
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
#include <circle/string.h>
#include <circle/util.h>

#define FORMAT_RESERVE		64	// additional bytes to allocate

CString::CString (void)
:	m_pBuffer (0),
	m_nSize (0)
{
}

CString::CString (const char *pString)
{
	m_nSize = strlen (pString)+1;

	m_pBuffer = new char[m_nSize];

	strcpy (m_pBuffer, pString);
}

CString::~CString (void)
{
	delete [] m_pBuffer;
	m_pBuffer = 0;
}

CString::operator const char *(void) const
{
	if (m_pBuffer != 0)
	{
		return m_pBuffer;
	}

	return "";
}

const char *CString::operator = (const char *pString)
{
	delete [] m_pBuffer;

	m_nSize = strlen (pString)+1;

	m_pBuffer = new char[m_nSize];

	strcpy (m_pBuffer, pString);

	return m_pBuffer;
}

const CString &CString::operator = (const CString &rString)
{
	delete [] m_pBuffer;

	m_nSize = strlen (rString)+1;

	m_pBuffer = new char[m_nSize];

	strcpy (m_pBuffer, rString);

	return *this;
}

size_t CString::GetLength (void) const
{
	if (m_pBuffer == 0)
	{
		return 0;
	}
	
	return strlen (m_pBuffer);
}

void CString::Append (const char *pString)
{
	m_nSize = 1;		// for terminating '\0'
	if (m_pBuffer != 0)
	{
		m_nSize += strlen (m_pBuffer);
	}
	m_nSize += strlen (pString);

	char *pBuffer = new char[m_nSize];

	if (m_pBuffer != 0)
	{
		strcpy (pBuffer, m_pBuffer);
		delete [] m_pBuffer;
	}
	else
	{
		*pBuffer = '\0';
	}

	strcat (pBuffer, pString);

	m_pBuffer = pBuffer;
}

int CString::Compare (const char *pString) const
{
	return strcmp (m_pBuffer, pString);
}

int CString::Find (char chChar) const
{
	int nPos = 0;

	for (char *p = m_pBuffer; *p; p++)
	{
		if (*p == chChar)
		{
			return nPos;
		}

		nPos++;
	}

	return -1;
}

int CString::Replace (const char *pOld, const char *pNew)
{
	int nResult = 0;

	if (*pOld == '\0')
	{
		return nResult;
	}

	CString OldString (m_pBuffer);

	delete [] m_pBuffer;
	m_nSize = FORMAT_RESERVE;
	m_pBuffer = new char[m_nSize];
	m_pInPtr = m_pBuffer;

	const char *pReader = OldString.m_pBuffer;
	const char *pFound;
	while ((pFound = strchr (pReader, pOld[0])) != 0)
	{
		while (pReader < pFound)
		{
			PutChar (*pReader++);
		}

		const char *pPattern = pOld+1;
		const char *pCompare = pFound+1;
		while (   *pPattern != '\0'
		       && *pCompare == *pPattern)
		{
			pCompare++;
			pPattern++;
		}

		if (*pPattern == '\0')
		{
			PutString (pNew);
			pReader = pCompare;
			nResult++;
		}
		else
		{
			PutChar (*pReader++);
		}
	}

	PutString (pReader);

	*m_pInPtr = '\0';

	return nResult;
}

void CString::Format (const char *pFormat, ...)
{
	va_list var;
	va_start (var, pFormat);

	FormatV (pFormat, var);

	va_end (var);
}


extern "C"
void _vcbprintf(void (*write)(void*, char), void* arg, const char* format, va_list args);

void CString::FormatV (const char *pFormat, va_list Args)
{
	delete [] m_pBuffer;

	m_nSize = FORMAT_RESERVE;
	m_pBuffer = new char[m_nSize];
	m_pInPtr = m_pBuffer;

	_vcbprintf(vcbprintf_callback, this, pFormat, Args);

	*m_pInPtr = '\0';
}

void CString::PutChar (char chChar, size_t nCount)
{
	ReserveSpace (nCount);

	while (nCount--)
	{
		*m_pInPtr++ = chChar;
	}
}

void CString::PutString (const char *pString)
{
	size_t nLen = strlen (pString);
	
	ReserveSpace (nLen);
	
	strcpy (m_pInPtr, pString);
	
	m_pInPtr += nLen;
}

void CString::ReserveSpace (size_t nSpace)
{
	if (nSpace == 0)
	{
		return;
	}
	
	size_t nOffset = m_pInPtr - m_pBuffer;
	size_t nNewSize = nOffset + nSpace + 1;
	if (m_nSize >= nNewSize)
	{
		return;
	}
	
	nNewSize += FORMAT_RESERVE;
	char *pNewBuffer = new char[nNewSize];
		
	*m_pInPtr = '\0';
	strcpy (pNewBuffer, m_pBuffer);
	
	delete [] m_pBuffer;

	m_pBuffer = pNewBuffer;
	m_nSize = nNewSize;

	m_pInPtr = m_pBuffer + nOffset;
}


char* CString::Detach()
{
	char* temp = m_pBuffer;
	m_pBuffer = 0;
	m_nSize = 0;
	return temp;
}

void CString::Attach(char* pString)
{
	delete [] m_pBuffer;
	m_nSize = strlen (pString)+1;
	m_pBuffer = pString;
}
