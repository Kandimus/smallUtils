#include <ctime>
#include <stdarg.h>
#include <vector>
#include <fstream>
#include <iostream>
#include "Log.h"

namespace su
{

const uint32_t MAX_TEXT_BUFF = 2048;

Log& Log::instance()
{
	static Log Singleton;
	return Singleton;
}

void Log::put(Level level, const char* source, uint32_t lineno, const std::string& text)
{
	static char mark[Level::LevelLog__END] = { 'E', 'W', 'I', 'N', 'D' };

	char postfix[32] = { 0 };
	char datetimeMark[64] = { 0 };
	std::time_t t = std::time(nullptr);
	std::tm dt;
	
	localtime_s(&dt, &t);
	sprintf_s(postfix, "_%04i.%02i.%02i.log", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday);
	sprintf_s(datetimeMark, "%02i.%02i.%04i %02i:%02i:%02i [%c",
		dt.tm_mday, dt.tm_mon + 1, dt.tm_year + 1900,
		dt.tm_hour, dt.tm_min, dt.tm_sec,
		mark[static_cast<int>(level)]);

	std::string filename = m_dir + m_filename + postfix;
	std::string fulltext = datetimeMark;

	if (source)
	{
		fulltext += ":" + std::string(source) + ":" + std::to_string(lineno);
	}

	fulltext += "] " + text + "\n";

	std::lock_guard<std::mutex> guard(m_mutex);

	if (m_toFile)
	{
		std::ofstream file(filename, std::ios_base::app);
		if (file.is_open())
		{
			file << fulltext;
			file.close();
		}
	}

	if (level > m_level)
	{
		return;
	}

	if (m_toTerminal)
	{
		std::cout << fulltext;
	}

	m_list.push_back(fulltext);
}

void Log::putFormat(Level level, const char* source, uint32_t lineno, const char* format, ...)
{
	char* buff = new char[MAX_TEXT_BUFF];

	va_list(args);
	va_start(args, format);
	int result = vsnprintf(buff, MAX_TEXT_BUFF, format, args);
	va_end(args);

	put(level, source, lineno, buff);

	delete[] buff;
}

void Log::setDir(const std::string& dir)
{
#ifdef _WIN32
	char sep = '\\';
#else
	char sep = '/'
#endif

	std::lock_guard<std::mutex> guard(m_mutex);

	m_dir = dir;

	if (m_dir.back() != sep)
	{
		m_dir += sep;
	}
}

const std::string& Log::getDir() const
{
	return m_dir;
}

void Log::setFilename(const std::string& filename)
{
	std::lock_guard<std::mutex> guard(m_mutex);

	m_filename = filename;
}

const std::string& Log::getFilename(void) const
{
	return m_filename;
}

void Log::setLevel(Level level)
{
	m_level = level;
}

Log::Level Log::getLevel(void) const
{
	return m_level;
}

void Log::setTerminal(bool toTerminal)
{
	m_toTerminal = toTerminal;
}

bool Log::toTerminal(void) const
{
	return m_toTerminal;
}

bool Log::toFile() const
{
	return m_toFile;
}

void Log::setFile(bool toFile)
{
	m_toFile = toFile;
}

std::list<std::string> Log::getNews()
{
	std::lock_guard<std::mutex> guard(m_mutex);

	return std::move(m_list);
}

} // namespace su

