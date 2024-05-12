//=================================================================================================
//===
//=== stringex.cpp
//===
//=== Copyright (c) 2019 by VeduN.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Расширение функций стандартных строк С++
//===
//=================================================================================================

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <algorithm>
#include <string.h>
#include <vector>
//#include <cmath>
//#include <unistd.h>
//#include <iostream>

//#include "time64.h"
//#include "def.h"

#ifdef _WIN32
#include <time.h>
#endif

namespace su
{

std::vector<std::string> String_slit(const std::string& text, char separator)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while((pos = text.find(separator, pos)) != std::string::npos)
    {
        std::string substring(text.substr(prev_pos, pos - prev_pos));

        output.push_back(substring);
        prev_pos = ++pos;
    }

    output.push_back(text.substr(prev_pos, pos - prev_pos));
    return output;
}

int String_isValidHex(const char *str, unsigned int &val)
{
    unsigned int ii = 0;
    unsigned int xx = 0;

    if(nullptr == str) return false;

    for(val = 0; *str; ++str, ++ii)
    {
        if(ii >= 8) return val = 0;

              if(*str >= '0' && *str <= '9') xx = *str - 0x30;
        else if(*str >= 'a' && *str <= 'f') xx = *str - 0x57;
        else if(*str >= 'A' && *str <= 'F') xx = *str - 0x37;
        else return val = 0;

        val  = (val << 4) + xx;
    }

    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
int String_BufferSize___(const char *format, va_list arg_ptr)
{
    return vsnprintf(NULL, 0, format, arg_ptr) + 1; // safe byte for \0
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Форматирование строки и выдача класса string



std::string String_vaformat(const char *format, va_list arg_ptr)
{
    std::string result = "";
    char       *buff   = nullptr;

    size_t s = _vsnprintf_s(NULL, 0, 0xffffffff, format, arg_ptr) + 1;
    buff = new char[s];
    if(buff)
    {
        vsprintf_s(buff, s, format, arg_ptr);
    }

    if(buff)
    {
        result = buff;

        delete[] buff;
    }

    return result;
}

std::string String_format(const char *format, ...)
{
    va_list arg;
    va_start(arg, format);
    std::string result = String_vaformat(format, arg);
    va_end(arg);

    return result;
}

std::string String_format2(const char* format, ...)
{
    va_list arg;
//	std::string result = String_vaformat(format, arg); //WARNING Тут падает на x64
//	char *buff = new char[vsnprintf(NULL, 0, format, arg) + 2];
    char *buff = new char[4096];
    memset(buff, 0, 4096);

    //	va_list arg2;
    //	va_start(arg2, format);
    //	vsprintf(buff, format, arg2);
    //	va_end(arg2);
    va_start(arg, format);
    vsnprintf_s(buff, 4096, 4096, format, arg);
    va_end(arg);

    std::string result = buff;
    delete[] buff;

    return result;
}



std::string String_FileTime(time_t t)
{
    struct tm stm;

#ifdef _WIN32
    localtime_s(&stm, &t);
#else
    localtime_r(&t, &stm);
#endif
    return String_format("%02i.%02i.%04i %02i:%02i:%02i", stm.tm_mday, stm.tm_mon, stm.tm_year, stm.tm_hour, stm.tm_min, stm.tm_sec);
}


//
std::string String_tolower(const std::string &str)
{
    std::string result = str;

    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){ return std::tolower(c); } );

    return result;
}

std::string String_toupper(const std::string &str)
{
    std::string result = str;

    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c){ return std::toupper(c); } );

    return result;
}


//
bool String_equali(const std::string &str1, const std::string &str2)
{
    return std::equal(str1.begin(), str1.end(), str2.begin(), [] (const char ch1, const char ch2) { return std::tolower(ch1) == ch2; });
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Удаление из строки символов
// !!!!! Функция не учитывает UTF-8
std::string String_deletewhite(const std::string &str)
{
    std::string whitechars = "\t\n ";
    std::string result     = "";

    result.reserve(str.size() + 1);

    for(unsigned int ii = 0; ii < str.size(); ++ii)
    {
        if(std::string::npos == whitechars.find(str[ii]))
        {
            result += str[ii];
        }
    }

    return result;
}


// Перевод строки в числовой массив
int String_ToBuffer(const char *str, unsigned char *buf, unsigned int bufsize)
{
    char val;

    if(nullptr == str || nullptr == buf || !bufsize) return -2;

    bufsize <<= 1; // Так как каждый байт занимает два символа, то увеличим размер в два раза

    for(unsigned int ii = 0; ii < bufsize; ++ii)
    {
        if(!str[ii]) return (ii >> 1);

        if(str[ii] >= 0x30 && str[ii] <= 0x39)
        {
            val = str[ii] - 0x30;
        }
        else if(str[ii] >= 0x41 && str[ii] <= 0x46)
        {
            val = str[ii] - 0x37;
        }
        else if(str[ii] >= 0x61 && str[ii] <= 0x66)
        {
            val = str[ii] - 0x57;
        }
        else
        {
            return -1;
        }

        if(ii & 0x01)
        {
            buf[ii >> 1] |= val;
        }
        else
        {
            buf[ii >> 1] = val << 4;
        }
    }

    return bufsize >> 1;
}


// Перевод числового массива в строку
std::string String_FromBuffer(const unsigned char *buf, unsigned int bufsize)
{
    std::string result = "";

    result.reserve(bufsize * 2 + 1);

    for(unsigned int ii = 0; ii < bufsize; ++ii)
    {
        result += String_format("%02X", buf[ii]);
    }

    return result;
}

std::string String_ltrim(const std::string& str, const std::string& whitespace)
{
    if (str.empty())
    {
        return str;
    }

    int ii = 0;
    for (; ii < str.size(); ++ii)
    {
        if (whitespace.find(str[ii]) == std::string::npos)
        {
            break;
        }
    }

    if (ii == str.size())
    {
        return "";
    }

    return (ii == 0) ? str : str.substr(ii);
}

std::string String_rtrim(const std::string& str, const std::string& whitespace)
{
    if (str.empty())
    {
        return str;
    }

    int ii = int(str.size() - 1);
    for (; ii >= 0; --ii)
    {
        if (whitespace.find(str[ii]) == std::string::npos)
        {
            break;
        }
    }

    if (ii < 0)
    {
        return "";
    }

    return (ii == str.size() - 1) ? str : str.substr(0, ii + 1);
}

std::string String_trim(const std::string& str, const std::string& whitespace)
{
    std::string out = String_ltrim(str, whitespace);
    return out.size() ? String_rtrim(out, whitespace) : out;
}

std::string String_rawFilename(const std::string& filename)
{
    size_t lastIndex = filename.find_last_of(".");
    return lastIndex < filename.size() ? filename.substr(0, lastIndex) : filename;
}

}
