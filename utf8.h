
#pragma once

#include <string>

#include "defines.h"

namespace su
{

namespace
{
    static const int gUtf8Length[64] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                         1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,       // bit pattern 10xxxxxx invalid start of UTF8 character
                                         2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 5, 6};
}

inline bool convertUtf8ToWide(const char *sourceUtf8, std::wstring& destination) noexcept
{
    destination.clear();

    while (*sourceUtf8 != 0)
    {
        switch(gUtf8Length[*(byte *)sourceUtf8 >> 2])
        {
            case 0:
            case 5:
            case 6:
            {
                SU_ASSERT(0);      // improperly formatted UTF8 string or UTF length not supported yet
                return false;
            }
            case 1:
            {
                destination += (wchar_t)*sourceUtf8++;
                break;
            }
            case 2:
            {
                wchar_t c = (wchar_t)((*sourceUtf8++ & 0x1f) << 6);
                c |= (*sourceUtf8++ & 0x3f);
                destination += c;
                break;
            }
            case 3:
            {
                wchar_t c = (wchar_t)((*sourceUtf8++ & 0x0f) << 12);
                c |= ((*sourceUtf8++ & 0x3f) << 6);
                c |= (*sourceUtf8++ & 0x3f);
                destination += c;
                break;
            }
            case 4:
            {
                int32_t c = (int32_t)((*sourceUtf8++ & 0x07) << 18);
                c |= ((*sourceUtf8++ & 0x3f) << 12);
                c |= ((*sourceUtf8++ & 0x3f) << 6);
                c |= (*sourceUtf8++ & 0x3f);

                if (sizeof(wchar_t) == 2)
                {
                    // UTF-16 encode the value
                    c -= 0x10000;
                    wchar_t ch = (wchar_t)(0xd800 | (c >> 10));
                    wchar_t cl = (wchar_t)(0xdc00 | (c & 0x3ff));
                    destination += ch;
                    destination += cl;
                }
                else
                {
                    // direct emit the value
                    destination += (wchar_t)c;
                }
                break;
            }
        }
    }
    return true;
}

inline bool convertWideToUtf8(const wchar_t *source, std::string& destinationUtf8) noexcept
{
    destinationUtf8.clear();

    while (*source != 0)
    {
        int32_t c = *source++;
        if (c <= 0x7f)
        {
            // 1 byte UTF-8 encoding (ie. ascii)
            destinationUtf8 += (char)c;
        }
        else if (c <= 0x7ff)
        {
            // 2 byte UTF-8 encoding
            destinationUtf8 += (char)(0xc0 + ((c >> 6) & 0x3f));
            destinationUtf8 += (char)(0x80 + (c & 0x003f));
        }
        else if (c >= 0xd800 && c <= 0xdfff)
        {
            // UTF-16 encoding found, translate into 4 byte UTF-8 encoding

            // there is no unicode characters in this range as per Unicode and ISO/IEC 10646 standard
            // this range is reserved for UTF-16 encoding, so if we see values in this range, we can
            // assume they are UTF-16 encoded characters and convert them as such.
            int32_t cfull = (c & 0x3ff) << 10;
            c = *source++;
            if (c < 0xdc00 || c > 0xdfff)
            {
                return false;       // invalid sequence found, conversion aborted
            }
            cfull |= (c & 0x3ff);
            cfull += 0x10000;

            destinationUtf8 += (char)(0xf0 + ((cfull >> 18) & 0x07));
            destinationUtf8 += (char)(0x80 + ((cfull >> 12) & 0x3f));
            destinationUtf8 += (char)(0x80 + ((cfull >> 6) & 0x3f));
            destinationUtf8 += (char)(0x80 + (cfull & 0x3f));
        }
        else if (c <= 0xffff)
        {
            // 3 byte UTF-8 encoding
            destinationUtf8 += (char)(0xe0 + ((c >> 12) & 0x0f));
            destinationUtf8 += (char)(0x80 + ((c >> 6) & 0x3f));
            destinationUtf8 += (char)(0x80 + (c & 0x3f));
        }
        else if (c <= 0x10ffff)     // technically 4 byte encoding can handle 21 bits of value, but the standard stops at this value, so anything larger than this is illegal, it's also worth noting that UTF-16 encoding is limited to 20 bits + 0x10000 code-point values, which gives you this number
        {
            // 4 byte UTF-8 encoding
            // note: this section can only be gotten to if sizeof(wchar_t) is greater than 2.
            // if sizeof(wchar_t) is 2, then the only way 4-byte encodings can be gotten is through UTF-16 encoded as interpretted above.
            c = *source;
            destinationUtf8 += (char)(0xf0 + ((c >> 18) & 0x07));
            destinationUtf8 += (char)(0x80 + ((c >> 12) & 0x3f));
            destinationUtf8 += (char)(0x80 + ((c >> 6) & 0x3f));
            destinationUtf8 += (char)(0x80 + (c & 0x3f));
        }
        else
        {
            return false;   // 5/6 byte encodings not supported yet
        }
    }
    return true;
}

inline int utf8Length(const char *utf8String) noexcept
{
    int len = 0;
    while (*utf8String != 0)
    {
        int clen = gUtf8Length[*(byte *)utf8String >> 2];
        utf8String += (clen ? clen : 1);
        len++;
    }
    return len;
}

inline bool utf8IsAscii(const char *utf8String) noexcept
{
    while (*utf8String != 0)
    {
        int clen = gUtf8Length[*(byte *)utf8String >> 2];
        if (clen != 1)
        {
            return false;
        }
        utf8String++;
    }
    return true;
}

inline bool utf8IsValid(const char *utf8String) noexcept
{
    while (*utf8String != 0)
    {
        switch(gUtf8Length[*(byte *)utf8String >> 2])
        {
            case 0:
                return false;       // misformatted lead byte
            case 5:
            case 6:
                return false;       // not supported yet, so call it invalid for now
            case 1:
                utf8String++;
                break;
            case 2:
            {
                utf8String++;
                if ((*(byte *)utf8String >> 6) != 2)
                {
                    return false;       // second byte must start with 10xxxxxx bit pattern
                }
                utf8String++;
                break;
            }
            case 3:
            {
                utf8String++;
                if ((*(byte *)utf8String >> 6) != 2)
                {
                    return false;       // second byte must start with 10xxxxxx bit pattern
                }
                utf8String++;
                if ((*(byte *)utf8String >> 6) != 2)
                {
                    return false;       // third byte must start with 10xxxxxx bit pattern
                }
                utf8String++;
                break;
            }
            case 4:
            {
                utf8String++;
                if ((*(byte *)utf8String >> 6) != 2)
                {
                    return false;       // second byte must start with 10xxxxxx bit pattern
                }
                utf8String++;
                if ((*(byte *)utf8String >> 6) != 2)
                {
                    return false;       // third byte must start with 10xxxxxx bit pattern
                }
                utf8String++;
                if ((*(byte *)utf8String >> 6) != 2)
                {
                    return false;       // fourth byte must start with 10xxxxxx bit pattern
                }
                utf8String++;
                break;
            }
        }
    }
    return true;
}


}   // namespace


