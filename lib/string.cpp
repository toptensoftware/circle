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
#include <circle/stringbuilder.h>
#include <malloc.h>

CString::CString (void)
:	m_pBuffer (0),
	m_nSize (0)
{
}

CString::CString (const char *pString)
{
	m_nSize = strlen (pString)+1;

	m_pBuffer = (char*)malloc(m_nSize);

	strcpy (m_pBuffer, pString);
}

CString::~CString (void)
{
	free(m_pBuffer);
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
	free(m_pBuffer);

	m_nSize = strlen (pString)+1;

	m_pBuffer = (char*)malloc(m_nSize);

	strcpy (m_pBuffer, pString);

	return m_pBuffer;
}

const CString &CString::operator = (const CString &rString)
{
	free(m_pBuffer);

	m_nSize = strlen (rString)+1;

	m_pBuffer = (char*)malloc(m_nSize);

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

	char *pBuffer = (char*)malloc(m_nSize);

	if (m_pBuffer != 0)
	{
		strcpy (pBuffer, m_pBuffer);
		free(m_pBuffer);
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

	char* pFinal;
	nResult = string_replace(&pFinal, m_pBuffer, pOld, pNew);

	free(m_pBuffer);
	m_pBuffer = pFinal;
	return nResult;
}

void CString::Format (const char *pFormat, ...)
{
	va_list var;
	va_start (var, pFormat);
	FormatV (pFormat, var);
	va_end (var);
}

void CString::FormatV (const char *pFormat, va_list Args)
{
	free(m_pBuffer);
	m_pBuffer = string_vsprintf(pFormat, Args);
}

