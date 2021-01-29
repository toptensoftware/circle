//
// vcbsprintf.cpp
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
#include <stdarg.h>

#ifndef __ccw__

#if AARCH == 32
	#define MAX_NUMBER_LEN		22	// 64 bit octal number
	#define MAX_PRECISION		9	// floor (log10 (ULONG_MAX))
#else
	#define MAX_NUMBER_LEN		22	// 64 bit octal number
	#define MAX_PRECISION		19	// floor (log10 (ULONG_MAX))
#endif

#define MAX_FLOAT_LEN		(1+MAX_NUMBER_LEN+1+MAX_PRECISION)



static char *ntoa (char *pDest, unsigned long ulNumber, unsigned nBase, boolean bUpcase)
{
	unsigned long ulDigit;

	unsigned long ulDivisor = 1UL;
	while (1)
	{
		ulDigit = ulNumber / ulDivisor;
		if (ulDigit < nBase)
		{
			break;
		}

		ulDivisor *= nBase;
	}

	char *p = pDest;
	while (1)
	{
		ulNumber %= ulDivisor;

		*p++ = ulDigit < 10 ? '0' + ulDigit : '0' + ulDigit + 7 + (bUpcase ? 0 : 0x20);

		ulDivisor /= nBase;
		if (ulDivisor == 0)
		{
			break;
		}

		ulDigit = ulNumber / ulDivisor;
	}

	*p = '\0';

	return pDest;
}

#if STDLIB_SUPPORT >= 1
static char *lltoa (char *pDest, unsigned long long ullNumber, unsigned nBase, boolean bUpcase)
{
	unsigned long long ullDigit;

	unsigned long long ullDivisor = 1ULL;
	while (1)
	{
		ullDigit = ullNumber / ullDivisor;
		if (ullDigit < nBase)
		{
			break;
		}

		ullDivisor *= nBase;
	}

	char *p = pDest;
	while (1)
	{
		ullNumber %= ullDivisor;

		*p++ = ullDigit < 10 ? '0' + ullDigit : '0' + ullDigit + 7 + (bUpcase ? 0 : 0x20);

		ullDivisor /= nBase;
		if (ullDivisor == 0)
		{
			break;
		}

		ullDigit = ullNumber / ullDivisor;
	}

	*p = '\0';

	return pDest;
}
#endif

static char *ftoa (char *pDest, double fNumber, unsigned nPrecision)
{
	char *p = pDest;

	if (fNumber < 0)
	{
		*p++ = '-';

		fNumber = -fNumber;
	}

	if (fNumber > (unsigned long) -1)
	{
		strcpy (p, "overflow");

		return pDest;
	}

	unsigned long iPart = (unsigned long) fNumber;
	ntoa (p, iPart, 10, FALSE);

	if (nPrecision == 0)
	{
		return pDest;
	}

	p += strlen (p);
	*p++ = '.';

	if (nPrecision > MAX_PRECISION)
	{
		nPrecision = MAX_PRECISION;
	}

	unsigned long nPrecPow10 = 10;
	for (unsigned i = 2; i <= nPrecision; i++)
	{
		nPrecPow10 *= 10;
	}

	fNumber -= (double) iPart;
	fNumber *= (double) nPrecPow10;
	
	char Buffer[MAX_PRECISION+1];
	ntoa (Buffer, (unsigned long) fNumber, 10, FALSE);

	nPrecision -= strlen (Buffer);
	while (nPrecision--)
	{
		*p++ = '0';
	}

	strcpy (p, Buffer);
	
	return pDest;
}

void write_str(void (*write)(void*, char), void* arg, const char* str)
{
    while (*str)
    {
        write(arg, *str++);
    }
}

void write_chrs(void (*write)(void*, char), void* arg, char ch, size_t count)
{
    while (count--)
    {
        write(arg, ch);
    }
}


extern "C"
__attribute__ ((weak))
void vcbprintf(void (*write)(void*, char), void* arg, const char* pFormat, va_list Args)
{
	while (*pFormat != '\0')
	{
		if (*pFormat == '%')
		{
			if (*++pFormat == '%')
			{
				write (arg, '%');
				
				pFormat++;

				continue;
			}

			boolean bAlternate = FALSE;
			if (*pFormat == '#')
			{
				bAlternate = TRUE;

				pFormat++;
			}

			boolean bLeft = FALSE;
			if (*pFormat == '-')
			{
				bLeft = TRUE;

				pFormat++;
			}

			boolean bNull = FALSE;
			if (*pFormat == '0')
			{
				bNull = TRUE;

				pFormat++;
			}

			size_t nWidth = 0;
			while ('0' <= *pFormat && *pFormat <= '9')
			{
				nWidth = nWidth * 10 + (*pFormat - '0');

				pFormat++;
			}

			unsigned nPrecision = 6;
			if (*pFormat == '.')
			{
				pFormat++;

				nPrecision = 0;
				while ('0' <= *pFormat && *pFormat <= '9')
				{
					nPrecision = nPrecision * 10 + (*pFormat - '0');

					pFormat++;
				}
			}

#if STDLIB_SUPPORT >= 1
			boolean bLong = FALSE;
			boolean bLongLong = FALSE;
			unsigned long long ullArg;
			long long llArg = 0;

			if (*pFormat == 'l')
			{
				if (*(pFormat+1) == 'l')
				{
					bLongLong = TRUE;

					pFormat++;
				}
				else
				{
					bLong = TRUE;
				}

				pFormat++;
			}
#else
			boolean bLong = FALSE;
			if (*pFormat == 'l')
			{
				bLong = TRUE;

				pFormat++;
			}
#endif
			char chArg;
			const char *pArg;
			unsigned long ulArg;
			size_t nLen;
			unsigned nBase;
			char NumBuf[MAX_FLOAT_LEN+1];
			boolean bMinus = FALSE;
			long lArg = 0;
			double fArg;

			switch (*pFormat)
			{
			case 'c':
				chArg = (char) va_arg (Args, int);
				if (bLeft)
				{
					write (arg, chArg);
					if (nWidth > 1)
					{
						write_chrs (write, arg, ' ', nWidth-1);
					}
				}
				else
				{
					if (nWidth > 1)
					{
						write_chrs (write, arg, ' ', nWidth-1);
					}
					write (arg, chArg);
				}
				break;

			case 'd':
			case 'i':
#if STDLIB_SUPPORT >= 1
				if (bLongLong)
				{
					llArg = va_arg (Args, long long);
				}
				else if (bLong)
#else
				if (bLong)
#endif
				{
					lArg = va_arg (Args, long);
				}
				else
				{
					lArg = va_arg (Args, int);
				}
				if (lArg < 0)
				{
					bMinus = TRUE;
					lArg = -lArg;
				}
#if STDLIB_SUPPORT >= 1
				if (llArg < 0)
				{
					bMinus = TRUE;
					llArg = -llArg;
				}
				if (bLongLong)
					lltoa (NumBuf, (unsigned long long) llArg, 10, FALSE);
				else
					ntoa (NumBuf, (unsigned long) lArg, 10, FALSE);
#else
				ntoa (NumBuf, (unsigned long) lArg, 10, FALSE);
#endif
				nLen = strlen (NumBuf) + (bMinus ? 1 : 0);
				if (bLeft)
				{
					if (bMinus)
					{
						write (arg, '-');
					}
					write_str (write, arg, NumBuf);
					if (nWidth > nLen)
					{
						write_chrs (write, arg, ' ', nWidth-nLen);
					}
				}
				else
				{
					if (!bNull)
					{
						if (nWidth > nLen)
						{
							write_chrs (write, arg, ' ', nWidth-nLen);
						}
						if (bMinus)
						{
							write (arg, '-');
						}
					}
					else
					{
						if (bMinus)
						{
							write (arg, '-');
						}
						if (nWidth > nLen)
						{
							write_chrs (write, arg, '0', nWidth-nLen);
						}
					}
					write_str (write, arg, NumBuf);
				}
				break;

			case 'f':
				fArg = va_arg (Args, double);
				ftoa (NumBuf, fArg, nPrecision);
				nLen = strlen (NumBuf);
				if (bLeft)
				{
					write_str (write, arg, NumBuf);
					if (nWidth > nLen)
					{
						write_chrs (write, arg, ' ', nWidth-nLen);
					}
				}
				else
				{
					if (nWidth > nLen)
					{
						write_chrs (write, arg, ' ', nWidth-nLen);
					}
					write_str (write, arg, NumBuf);
				}
				break;

			case 'o':
				if (bAlternate)
				{
					write (arg, '0');
				}
				nBase = 8;
				goto FormatNumber;

			case 's':
				pArg = va_arg (Args, const char *);
				nLen = strlen (pArg);
				if (bLeft)
				{
					write_str (write, arg, pArg);
					if (nWidth > nLen)
					{
						write_chrs (write, arg, ' ', nWidth-nLen);
					}
				}
				else
				{
					if (nWidth > nLen)
					{
						write_chrs (write, arg, ' ', nWidth-nLen);
					}
					write_str (write, arg, pArg);
				}
				break;

			case 'u':
				nBase = 10;
				goto FormatNumber;

			case 'x':
			case 'X':
			case 'p':
				if (bAlternate)
				{
					write_str (write, arg, *pFormat == 'X' ? "0X" : "0x");
				}
				nBase = 16;
				goto FormatNumber;

			FormatNumber:
#if STDLIB_SUPPORT >= 1
				if (bLongLong)
				{
					ullArg = va_arg (Args, unsigned long long);
				}
				else if (bLong)
#else
				if (bLong)
#endif
				{
					ulArg = va_arg (Args, unsigned long);
				}
				else
				{
					ulArg = va_arg (Args, unsigned);
				}
#if STDLIB_SUPPORT >= 1
				if (bLongLong)
					lltoa (NumBuf, ullArg, nBase, *pFormat == 'X');
				else
					ntoa (NumBuf, ulArg, nBase, *pFormat == 'X');
#else
				ntoa (NumBuf, ulArg, nBase, *pFormat == 'X');
#endif
				nLen = strlen (NumBuf);
				if (bLeft)
				{
					write_str (write, arg, NumBuf);
					if (nWidth > nLen)
					{
						write_chrs (write, arg, ' ', nWidth-nLen);
					}
				}
				else
				{
					if (nWidth > nLen)
					{
						write_chrs (write, arg, bNull ? '0' : ' ', nWidth-nLen);
					}
					write_str (write, arg, NumBuf);
				}
				break;

			default:
				write (arg, '%');
				write (arg, *pFormat);
				break;
			}
		}
		else
		{
			write (arg, *pFormat);
		}

		pFormat++;
	}
}

#endif		// __ccw__
