
#define ENABLE_COMM
#include <Config.h>

#ifdef CORE_SUPPORT_LIBC

////////////////////////////////////////////////////////////////////////////////////
//  
//  LibC.c: Missing or extended C API
//
////////////////////////////////////////////////////////////////////////////////////

#include <CoreTypes.h>
#include <string.h>
#include <ctype.h>

#ifdef WIN32
	#define win_or_linux
#else
	#if linux
		#define win_or_linux
	#endif
#endif

#ifdef WIN32
#pragma warning(disable : 4996) // CRT Secure - off
#endif

////////////////////////////////////////////////////////////////////////////////////
//
// Function: strltrim 
// Description: left trim 
// Parameters: str (null terminated, outpit trim
// Return:
//
////////////////////////////////////////////////////////////////////////////////////
#ifndef win_or_linux

uint8w *strtrim(uint8_tw *str)
{
    uint8_tw *ibuf = str, *obuf = str;
    int32_tw i = 0, cnt = 0;

    if (str)
    {
        // Remove leading spaces (from RMLEAD.C)

        for (ibuf = str; *ibuf && isspace(*ibuf); ++ibuf)
            ;
        if (str != ibuf)
            memmove(str, ibuf, ibuf - str);

        // Collapse embedded spaces (from LV1WS.C)

        while (*ibuf)
        {
            if (isspace(*ibuf) && cnt)
                ibuf++;
            else
            {
                if (!isspace(*ibuf))
                    cnt = 0;
                else
                {
                    *ibuf = ' ';
                    cnt = 1;
                }
                obuf[i++] = *ibuf++;
            }
        }
        obuf[i] = NULL;

        // Remove trailing spaces (from RMTRAIL.C)

        while (--i >= 0)
        {
            if (!isspace(obuf[i]))
                break;
        }
        obuf[++i] = NULL;
    }
    return str;
}


////////////////////////////////////////////////////////////////////////////////////
//
// Function: strupr  
// Description: force string to uppercase  
// Parameters:null terminated string
// Return:  returns its argument
//
////////////////////////////////////////////////////////////////////////////////////

uint8_tw *strupr (uint8_tw *a)
{
    uint8_tw *ret = a;

    while (*a != '\0')
    {
        if (islower (*a))
            *a = toupper (*a);
        ++a;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: strrepl 
// Description: Replace OldStr by NewStr in string Str. 
// Parameters:
// Return: pointer to first location behind where NewStr was inserted  
//          or NULL if OldStr was not found.
//
////////////////////////////////////////////////////////////////////////////////////

uint8_tw *strrepl(uint8_tw *Str, uint8_tw *OldStr, uint8_tw *NewStr)
{
    int OldLen, NewLen;
    uint8_tw *p, *q;

    if(NULL == (p = (uint8_tw*)strstr((char*)Str, (char*)OldStr)))
        return p;

    OldLen = strlen((char*)OldStr);
    NewLen = strlen((char*)NewStr);

    if(OldLen != NewLen)
        return 0;

    memmove(q = p+NewLen, p+OldLen, strlen((char*)p+OldLen)+1);
    memcpy(p, NewStr, NewLen);
    return q;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: strnicmp  
// Description: Case insensitive strncmp. Non-ISO, deprecated.  
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

uint32_tw strnicmp(const int8_tw *pStr1, const int8_tw *pStr2, uint32_tw Count)
{
    int8_tw c1, c2;
    uint32_tw v;

    if (Count == 0)
        return 0;

    do {
        c1 = *pStr1++;
        c2 = *pStr2++;
        // the casts are necessary when pStr1 is shorter & char is signed 
        v = (uint8_tw) tolower(c1) - (uint8_tw) tolower(c2);
    } while ((v == 0) && (c1 != '\0') && (--Count > 0));

    return v;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: strnicmp  
// Description: Compare characters of two strings without regard to case.
// Parameters: 2 null terminated strings
// Return: 0 when are the same.
//
////////////////////////////////////////////////////////////////////////////////////

int32_tw stricmp(const int8_tw *s1, const int8_tw *s2)
{
    int s1_len = strlen((char*)s1);
    int s2_len = strlen((char*)s2);

    if(!s1_len && !s2_len)
        return 0; // they are the same ( 0 bytes)

    if(s1_len < s2_len)
        return -1;

    if(s1_len > s2_len)
        return 1;

    return strnicmp(s1,s2,s1_len);
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: strlwr 
// Description: force string to lower case  
// Parameters: null terminated string
// Return: returns its argument
//
////////////////////////////////////////////////////////////////////////////////////

uint8_tw *strlwr (uint8_tw *a)
{
    uint8_tw *ret = a;

    while (*a != '\0')
    {
        if (isupper (*a))
            *a = tolower (*a);
        ++a;
    }

    return ret;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: itoa 
// Description: 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

#define INT_DIGITS 19		// Enough for 64 bit integer 

uint8_tw *itoa(int32_tw i)
{
    // Room for INT_DIGITS digits, - and '\0' 
    static uint8_tw buf[INT_DIGITS + 2];
    uint8_tw *p = buf + INT_DIGITS + 1;	// points to terminating '\0'
    if (i >= 0) {
        do {
            *--p = '0' + (i % 10);
            i /= 10;
        } while (i != 0);
        return p;
    }
    else {			// i < 0 
        do {
            *--p = '0' - (i % 10);
            i /= 10;
        } while (i != 0);
        *--p = '-';
    }
    return p;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: stristr 
// Description: This function is an ANSI version of strstr() with case insensitivity.
// Parameters:
// Return: char *pointer if Pattern is found in String, else pointer to 0
//
////////////////////////////////////////////////////////////////////////////////////

int8_tw *stristr (const int8_tw *String, const int8_tw *Pattern)
{
    int8_tw *pptr, *sptr, *start;
    uint32_tw  slen, plen;

    for (start = (int8_tw*)String,
        pptr  = (int8_tw*)Pattern,
        slen  = strlen((char*)String),
        plen  = strlen((char*)Pattern);

    /* while string length not shorter than pattern length */

    slen >= plen;

    start++, slen--)
    {
        /* find start of pattern in string */
        while (toupper(*start) != toupper(*Pattern))
        {
            start++;
            slen--;

            /* if pattern longer than string */

            if (slen < plen)
                return(NULL);
        }

        sptr = start;
        pptr = (int8_tw *)Pattern;

        while (toupper(*sptr) == toupper(*pptr))
        {
            sptr++;
            pptr++;

            /* if end of pattern then pattern was found */

            if ('\0' == *pptr)
                return (start);
        }
    }
    return(NULL);
}


#endif // Not Win32


////////////////////////////////////////////////////////////////////////////////////
//
// Function: memrev 
// Description: reverse "count" bytes starting at "buf" 
// Parameters: 
// Return: void
//
////////////////////////////////////////////////////////////////////////////////////

void memrev(uint8_tw *buf, uint32_tw count)
{
    uint8_tw *r;

    for (r = buf + count - 1; buf < r; buf++, r--)
    {
        *buf ^= *r;
        *r   ^= *buf;
        *buf ^= *r;
    }
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: MSB2LSB  
// Description: Convert MSB byte to LSB 
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

uint8_tw MSB2LSB(uint8_tw data)
{
    uint8_tw formattedData = 0;
    uint8_tw i;

    for( i = 0; i < 8; i++)
    {

        formattedData = formattedData << 1;      
        if( data & (0x01 << i) )
        {
            formattedData = (formattedData + 0x01);
        }
    }
    return formattedData;
}

////////////////////////////////////////////////////////////////////////////////////
//
// Function: LSB2MSB 
// Description:  Convert LSB byte to MSB
// Parameters:
// Return:
//
////////////////////////////////////////////////////////////////////////////////////

uint8_tw LSB2MSB(uint8_tw val)
{
    uint8_tw vet [] = {0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};
    uint8_tw i, j, value = 0;

    for(i=0, j=7; i<8; i++, j--) 
    {
        if (val & vet[i])
            value +=vet[j];
    }
    return value;
}


#endif // CORE_SUPPORT_LIBC
