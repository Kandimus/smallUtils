//=================================================================================================
//===
//=== stringex.h
//===
//=== Copyright (c) 2019-2020 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaliy O.
//===
//=================================================================================================
//===
//=== Расширение функций стандартных строк С++
//===
//=================================================================================================

#pragma once

#include <string>
#include <vector>

namespace su
{

extern std::vector<std::string> String_slit(const std::string& text, char separator);
extern int         String_isValidHex(const char *str, unsigned int &val);
extern int         String_BufferSize(const char *format, va_list arg_ptr);
extern std::string String_format(const char *format, ...);
extern std::string String_format2(const char* format, ...);
extern std::string String_vaformat(const char *format, va_list arg_ptr);
extern std::string String_FileTime(time_t t);
extern std::string String_tolower(const std::string &str);
extern std::string String_toupper(const std::string &str);
extern bool        String_equali(const std::string &str1, const std::string &str2);
extern std::string String_deletewhite(const std::string &str);
extern int         String_ToBuffer(const char *str, unsigned char *buf, unsigned int bufsize);
extern std::string String_FromBuffer(const unsigned char *buf, unsigned int bufsize);
extern std::string String_ltrim(const std::string& str, const std::string& whitespace);
extern std::string String_rtrim(const std::string& str, const std::string& whitespace);
extern std::string String_trim(const std::string& str, const std::string& whitespace);
extern std::string String_replace(const std::string& source, const std::string& oldstr, const std::string& newstr, bool all);

extern std::string String_printfHexBuffer(const void* buf, size_t count, size_t size, const std::string& prefix);

extern std::string String_rawFilename(const std::string& filename);
extern std::string String_extensionFilename(const std::string& filename);
}
