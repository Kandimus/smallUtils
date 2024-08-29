
#include "fileex.h"

#include <filesystem>
#include <fstream>
#include <stdio.h>

#include "stringex.h"

namespace su
{
namespace fs
{

Result getErrno(Result deferrno)
{
    switch (errno)
	{
		case ENODEV:
		case ENOENT:
		case ENXIO:
            return NotFound;

		case EPERM:
		case EACCES:
            return EAccess;

		case EISDIR:
            return IsDir;

		case ETXTBSY:
		case EWOULDBLOCK:
            return Blocked;

		case EROFS:
            return ReadOnly;

		default:
			return deferrno;
	}
}

Result createFolder(const std::string& path)
{
#if __cplusplus >= 202002L
    return std::filesystem::create_directories(path) ? OK : EDir;
#else
	std::string path = "";

    for (auto ch : filename)
    {
		path += ch;

        if (ch == PathSeparator)
		{
            if (access(path.c_str(), F_OK))
			{
                if (mkdir(path.c_str(), 0744))
				{
                    return EDir;
				}
			}
		}
	}
    return OK;
#endif
}

Result deleteFile(const std::string& filename)
{
#if __cplusplus >= 202002L
    return std::filesystem::remove(filename) ? OK : CantDelete;
#else
    if (unlink(filename.c_str()))
    {
        return getErrno(CantDelete);
	}

    return OK;
#endif
}

Result load(const std::string& filename, std::vector<uint8_t>& data)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        return getErrno(CantOpen);
    }

    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    data = std::move(buffer);
    return OK;
}

Result load(const std::string& filename, std::string& data)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        return getErrno(CantOpen);
    }

    std::string buffer(std::istreambuf_iterator<char>(file), {});
    data = std::move(buffer);
    return OK;
}

Result save(const std::string& filename, const std::vector<uint8_t>& data)
{
    createFolder(String_filenamePath(filename));

    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        return getErrno(CantOpen);
    }

    return file.write(reinterpret_cast<const char*>(data.data()), data.size()) ? OK : getErrno(CantWrite);
}

Result save(const std::string& filename, const std::string& data)
{
    createFolder(String_filenamePath(filename));

    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        return getErrno(CantOpen);
    }

    return file.write(data.c_str(), data.size()) ? OK : getErrno(CantWrite);
}

/*

unsigned int simpleFileSaveExt(const std::string& filename, const std::string& text, const std::string& mode)
{
	// Проверка на доступность файла
	if(access(filename.c_str(), F_OK))
	{
		// Попытка создать требуемые директории
		unsigned int result = simpleFileCreateDir(filename);

		if (result) {
			return result;
		}
	}

	FILE* file = fopen(filename.c_str(), mode.c_str());
	if (!file) {
		return simpleFileErrno(FILE_RESULT_CANTOPEN);
	}

	unsigned int fw = fwrite(text.c_str(), 1, text.size(), file);
	fclose(file);

	if (fw != text.size()) {
		return FILE_RESULT_IOERROR;
	}

	return TRITONN_RESULT_OK;
}


unsigned int simpleFileSave(const std::string& filename, const std::string& text)
{
	return simpleFileSaveExt(filename, text, "wt");
}

unsigned int simpleFileAppend(const std::string& filename, const std::string& text)
{
	return simpleFileSaveExt(filename, text, "at");
}

unsigned int simpleFileGuaranteedSave(const std::string& filename, const std::string& text)
{
	std::string  nametmp = filename + ".temp";
	unsigned int result  = simpleFileSave(nametmp, text);

	if (result != TRITONN_RESULT_OK) {
		return result;
	}

	::rename(nametmp.c_str(), filename.c_str());
	return simpleFileErrno(FILE_RESULT_CANTOPEN);
}
*/

} //namespace FileSystem
} // namespace su
