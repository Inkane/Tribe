/*  Copyright 1986-1992 Emmet P. Gray.
 *  Copyright 1996-1998,2001,2002,2008,2009 Alain Knaff.
 *  Copyright 2010 Volker Lanz (vl@fidra.de)
 *  This file is part of mtools.
 *
 *  Mtools is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Mtools is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Mtools.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Do shell-style pattern matching for '?', '\', '[..]', and '*' wildcards.
 * Returns 1 if match, 0 if not.
 */

#include "mtools.h"

#include <wctype.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static int casecmp(wchar_t a, wchar_t b)
{
	return towupper(a) == towupper(b);
}

static int exactcmp(wchar_t a,wchar_t b)
{
	return a == b;
}

static int parse_range(const wchar_t **p, const wchar_t *s, wchar_t *out, int (*compfn)(wchar_t a, wchar_t b))
{
	wchar_t table[256];
	int reverse;
	int i;

	if (**p == '^')
	{
		reverse = 1;
		(*p)++;
	}
	else
		reverse = 0;

	for (i = 0; i < 256; i++)
		table[i] = 0;

	while (**p != ']')
	{
		if(!**p)
			return 0;

		if((*p)[1] == '-')
		{
			short first = **p;
			(*p) += 2;

			short last = (**p == ']') ? 256 : *((*p)++);

			for(i = first; i <= last; i++)
				table[i] = 1;
		}
		else
			table[(int) *((*p)++)] = 1;
	}

	if(out)
		*out = *s;

	if(table[(int) *s])
		return 1 ^ reverse;

	if(compfn == exactcmp)
		return reverse;

	if(table[tolower(*s)])
	{
		if(out)
			*out = tolower(*s);

		return 1 ^ reverse;
	}

	if(table[toupper(*s)])
	{
		if(out)
			*out = toupper(*s);

		return 1 ^ reverse;
	}

	return reverse;
}


static int _match(const wchar_t *s, const wchar_t *p, wchar_t *out, int Case, int length, int (*compfn) (wchar_t a, wchar_t b))
{
	for (; *p != '\0' && length; )
	{
		switch (*p)
		{
			case '?': /* match any one character */
				if (*s == '\0')
					return(0);
				if(out)
					*(out++) = *s;
				break;

			case '*': /* match everything */
				while (*p == '*' && length)
				{
					p++;
					length--;
				}

					/* search for next char in pattern */
				while(*s)
				{
					if(_match(s, p, out, Case, length,
						  compfn))
						return 1;
					if(out)
						*out++ = *s;
					s++;
				}
				continue;

			case '[': /* match range of characters */
				p++;
				length--;
				if(!parse_range(&p, s, out++, compfn))
					return 0;
				break;

			case '\\': /* Literal match with next character */
				p++;
				length--;
				/* fall thru */

			default:
				if (!compfn(*s,*p))
					return(0);

				if(out)
					*(out++) = *p;

				break;
		}

		p++;
		length--;
		s++;
	}

	if(out)
		*out = '\0';

	/* string ended prematurely ? */
	return *s != '\0' ? 0 : 1;
}


int match(const wchar_t *s, const wchar_t *p, wchar_t *out, int Case, int length)
{
	int (*compfn)(wchar_t a, wchar_t b);

	if(Case)
		compfn = casecmp;
	else
		/*compfn = exactcmp;*/
		compfn = casecmp;

	return _match(s, p, out, Case, length, compfn);
}

