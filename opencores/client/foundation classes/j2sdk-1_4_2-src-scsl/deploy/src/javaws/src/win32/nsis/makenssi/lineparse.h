/*
 * Nullsoft "SuperPimp" Installation System 
 * version 1.0j - November 12th 2000
 *
 * Copyright (C) 1999-2000 Nullsoft, Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Justin Frankel
 * justin@nullsoft.com
 *
 * This source distribution includes portions of zlib. see zlib/zlib.h for
 * its license and so forth. Note that this license is also borrowed from zlib.
 *
 *
 * Portions Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)lineparse.h	1.5 03/01/23
 */


class LineParser {
  public:

    LineParser()
    {
      m_nt=0;
      m_tokens=0;
    }

    ~LineParser()
    {
      freetokens();
    }

    void parse(LPTSTR line)
    {
      LPTSTR p=line;
      while (*p) p=CharNext(p);
      if (p > line) p=CharPrev(line, p);
      while (p >= line && (*p == _T('\r') || *p == _T('\n') || *p == _T(' ') || *p == _T('\t'))) if ((p=CharPrev(line, p)) == line) p--;
	  p++;
      *p=_T('\0');

      freetokens();
      doline(line);
      if (m_nt) 
      {
        m_tokens=(LPTSTR*)malloc(sizeof(TCHAR*)*m_nt);
        doline(line);
      }
    }

    int getnumtokens() { return m_nt; }

    double gettoken_float(int token, int *success=0)
    {
      if (token < 0 || token >= m_nt) 
      {
        if (success) *success=0;
        return 0.0;
      }
      if (success)
      {
        LPTSTR t=m_tokens[token];    
        *success=*t?1:0;
		/* This is questionable code for I18n
        while (*t) 
        {
          if ((*t < '0' || *t > '9')&&*t != '.') *success=0;
          t++;
        }
		*/
      }
      return atof(m_tokens[token]);
    }
    int gettoken_int(int token, int *success=0) 
    { 
      if (token < 0 || token >= m_nt) 
      {
        if (success) *success=0;
        return 0;
      }
      if (success)
      {
        LPTSTR t=m_tokens[token];    
        *success=*t?1:0;
		/*
        while (*t) 
        {
          if ((*t < '0' || *t > '9')) *success=0;
          t++;
        }
		*/
      }
      return _ttoi(m_tokens[token]);
    }
	int gettoken_hex(int token)
	{
		TCHAR temp[MAX_PATH];
		LPTSTR t;
		int num=0;
		strcpy(temp, m_tokens[token]);
		t=temp;
		while (*t && *t == ' ') t++;
		while (*t && *t == '\"') t++;
		while (*t && *t == ' ') t++;
		if (t[0]=='0' && (t[1]=='x' || t[1]=='X')) {
			t++;t++;
			// valid
			while (*t && *t!=' ' && *t!='\"') {
				if (*t >= '0' && *t <='9') {
					num=num*16 + (*t - '0');
				} else {
					switch(*t) {
					case 'a':
					case 'A':
						num=num*16+10;
					break;
					case 'b':
					case 'B':
						num=num*16+11;
					break;

					case 'c':
					case 'C':
						num=num*16+12;
					break;
					case 'd':
					case 'D':
						num=num*16+13;
					break;
					case 'e':
					case 'E':
						num=num*16+14;
					break;
					case 'f':
					case 'F':
						num=num*16+15;
					break;
					default:
						return 0;
					break;
					}
				}
				t++;
			}
		} else {
			return 0;
		}
		return num;
	}


    LPTSTR gettoken_str(int token) 
    { 
      if (token < 0 || token >= m_nt) return "";
      return m_tokens[token]; 
    }
    int gettoken_enum(int token, LPTSTR strlist) // null seperated list
    {
      int x=0;
      LPTSTR tt=gettoken_str(token);
      if (tt && *tt) while (*strlist)
      {
        if (!_tcsicmp(tt,strlist)) return x;
        strlist+=_tcslen(strlist)+1;
        x++;
      }
      return -1;
    }
  private:
    void freetokens()
    {
      if (m_tokens)
      {
        int x;
        for (x = 0; x < m_nt; x ++)
          free(m_tokens[x]);
        free(m_tokens);
      }
      m_tokens=0;
      m_nt=0;
    }

    void doline(LPTSTR line)
    {
      m_nt=0;
      while (*line == _T(' ') || *line == _T('\t')) line=CharNext(line);
      if (*line != _T(';') && *line != _T('#')) while (*line) 
      {
		LPTSTR linesave;
        int lstate=0; // 1=", 2=`, 4='
        if (*line == _T('\"')) lstate=1;
        else if (*line == _T('\'')) lstate=2;
        else if (*line == _T('`')) lstate=4;
        if (lstate) line=CharNext(line);
        int nc=0;
		int nb=0;
        while (*line)
        {
          if (lstate==1 && *line ==_T('\"')) break;
          if (lstate==2 && *line ==_T('\'')) break;
          if (lstate==4 && *line ==_T('`')) break;
          if (!lstate && (*line == _T(' ') || *line == _T('\t'))) break;
          linesave=CharNext(line);
		  nb += linesave-line;
		  line=linesave;
          nc++;
        }
        if (m_tokens)
        {
          m_tokens[m_nt]=(LPTSTR)malloc(nb+1);
          _tcsncpy(m_tokens[m_nt],line-nb,nb);
          m_tokens[m_nt][nc]=0;
        }
        m_nt++;
        if (lstate && *line) line=CharNext(line);
        while (*line == _T(' ') || *line == _T('\t')) line=CharNext(line);
      }
    }
    
    int m_nt;
    LPTSTR *m_tokens;
};
