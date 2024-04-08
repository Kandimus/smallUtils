
#include "translatorManager.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#ifdef TRANSLATOR_USING_RAPIDJSON
#include "rapidjson/document.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/writer.h"
#endif

#include "stringex.h"

namespace su
{

#define PARSE_GET_NEX_CHAR     { ch = line[pos++]; }
#define PARSE_CHECK_DELIMER    if (!ch || ch == delimer) { state = STATE::DELIMER; break; }

static bool parseLine(const std::string& line, char delimer, VectorOfStrings& out)
{
    enum class STATE
    {
        START = 0,
        PLAIN_TEXT,
        ESCAPED_TEXT,
        ESCAPED_QUOTES,
        DELIMER,
        FINISH,
    };
    
    STATE state = STATE::START;
    std::string str;
    char ch = 0;
    size_t pos = 0;

    out.clear();

    while (state != STATE::FINISH)
    {
        switch (state)
        {
            case STATE::START:
                str = "";
                PARSE_GET_NEX_CHAR;
                PARSE_CHECK_DELIMER;

                if (ch == '"')
                {
                    state = STATE::ESCAPED_TEXT;
                    break;
                }

                --pos;
                state = STATE::PLAIN_TEXT;
                break;

            case STATE::PLAIN_TEXT:
                PARSE_GET_NEX_CHAR;
                PARSE_CHECK_DELIMER;

                str += ch;
                break;

            case STATE::ESCAPED_TEXT:
                PARSE_GET_NEX_CHAR;

                if (!ch)
                {
                    return false;
                }

                if (ch == '"')
                {
                    state = STATE::ESCAPED_QUOTES;
                    break;
                }

                str += ch;
                break;

            case STATE::ESCAPED_QUOTES:
                PARSE_GET_NEX_CHAR;
                PARSE_CHECK_DELIMER;

                if (ch == '"')
                {
                    str += '"';
                    state = STATE::ESCAPED_TEXT;
                    break;
                }

                out.clear();
                return false;

            case STATE::DELIMER:
                out.push_back(str);
                state = ch ? STATE::START : STATE::FINISH;
                break;
        }
    }

    return true;
}

static std::string escapeLine(char delimer, const VectorOfStrings& strings)
{
    std::string out = "";
    bool needComma = false;

    for (const auto& item : strings)
    {
        if (needComma)
        {
            out += ',';
        }
        needComma = true;

        if (std::string::npos == item.find(delimer) && std::string::npos == item.find('"'))
        {
            out += item;
            continue;
        }
        
        std::string escaped = "\"";
        for (auto ch : item)
        {
            if (ch == '"')
            {
                escaped += '"';
            }
            escaped += ch;
        }
        escaped += '"';

        out += escaped;
    }

    return out + "\n";
}

static std::string getDictionaryFilename(const std::string& filename, const TranslatorCreator* tr)
{
    return filename + "." + tr->getMarker() + ".csv";
}

static std::string getTranslatorVersion()
{
    return std::to_string(TranslatorVersion::major) + "." + std::to_string(TranslatorVersion::minor);
}

static VectorOfStrings getPluralsFromText(const std::string& text)
{
    VectorOfStrings out;

    // check a text for dictionary markers
    std::size_t begPos = 0;
    std::size_t endPos = 0;
    while ((begPos = text.find("@{", endPos)) != std::string::npos)
    {
        if ((endPos = text.find("}", begPos)) != std::string::npos)
        {
            out.push_back(text.substr(begPos + 2, endPos - begPos - 2));
        }
    }

    return out;
}

bool TranslatorCreator::addText(const std::string& sid, const std::string& text, bool overwrite)
{
    bool contains = m_data.find(sid) != m_data.end();

    if (!contains || overwrite)
    {
        m_data[sid] = text;
    }

    return contains;
}

bool TranslatorCreator::addPlural(const std::string& sid)
{
    bool contains = m_dictionary.find(sid) != m_dictionary.end();

    if (contains)
    {
        return true;
    }

    m_dictionary[sid].resize(m_pluralForms);
    for (auto& item : m_dictionary[sid])
    {
        item = "<# Plural missing #>";
    }

    return false;
}

size_t TranslatorCreator::clearSidByPrefix(const std::string& prefix)
{
    size_t out = 0;

    for (auto iter = m_data.begin(); iter != m_data.end(); )
    {
        const auto& sid = (*iter).first;

        if (sid.find(prefix) == 0)
        {
            iter = m_data.erase(iter);

            ++out;
        }
        else
        {
            iter++;
        }
    }

    return out;
}

bool TranslatorCreator::save(const std::string& filename, char delimer) const
{
    std::string out = "";
    VectorOfStrings element;

    out += escapeLine(delimer, { TranslatorVersion::headerDict, getTranslatorVersion() });

    element.push_back("sid");
    for (int ii = 0; ii < getPluralForms(); ++ii)
    {
        element.push_back("Form" + std::to_string(ii + 1));
    }
    out += escapeLine(delimer, element);

    for (const auto& item : m_dictionary)
    {
        auto sid = item.first;
        auto plurals = item.second;

        element.clear();
        element.push_back(sid);
        element.insert(element.end(), plurals.begin(), plurals.end());

        out += escapeLine(delimer, element);
    }

    std::ofstream outFile(filename);
    if (!outFile.is_open())
    {
        return false;
    }
    outFile.write(out.c_str(), out.size());
    outFile.close();

    return true;
}

bool TranslatorCreator::load(const std::string& filename, char delimer)
{
    std::ifstream inFile(filename/*, std::ios_base::app*/);
    if (!inFile.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();

    std::string line;
    uint32_t lineno = 1;

    m_dictionary.clear();

    while (std::getline(buffer, line))
    {
        VectorOfStrings element;

        if (!parseLine(line, delimer, element))
        {
            return false;
        }

        if (lineno == 1)
        {
            if (element.size() < 2)
            {
                return false;
            }

            if (element[0] != TranslatorVersion::headerDict)
            {
                return false;
            }

            if (element[1] != getTranslatorVersion())
            {
                return false;
            }
        }
        else if (lineno == 2)
        {
            if (element.size() != uint64_t(getPluralForms()) + 1)
            {
                return false;
            }

            if (element[0] != "sid")
            {
                return false;
            }
        }
        else
        {
            if (element.size() != uint64_t(getPluralForms()) + 1)
            {
                return false;
            }

            std::vector<std::string> v;
            for (size_t ii = 0; ii < getPluralForms(); ++ii)
            {
                v.push_back(std::move(element[ii + 1]));
            }
            m_dictionary[element[0]] = v;
        }

        ++lineno;
    }

    return true;
}

std::string TranslatorCreator::checkDictionary(const std::unordered_set<std::string>& plurals) const
{
    std::string out = "";
    std::unordered_set<std::string> tmpPlurals = plurals;

    for (auto item : m_dictionary)
    {
        auto key = item.first;

        if (tmpPlurals.find(key) != tmpPlurals.end())
        {
            tmpPlurals.erase(key);
            continue;
        }
        
        out += "The '" + key + "' plural is unused\n";
    }

    if (tmpPlurals.size())
    {
        for (const std::string& item : tmpPlurals)
        {
            out += "Cant find plurals for '" + item + "' spell\n";
        }
    }

    return out;
}

std::string TranslatorCreator::fixDictionary(const std::unordered_set<std::string>& plurals)
{
    std::string out = "";
    std::unordered_set<std::string> tmpPlurals = plurals;

    for (auto iter = m_dictionary.begin(); iter != m_dictionary.end(); )
    {
        auto key = (*iter).first;

        if (tmpPlurals.find(key) != tmpPlurals.end())
        {
            tmpPlurals.erase(key);
            ++iter;
            continue;
        }

        iter = m_dictionary.erase(iter);
        out += "The '" + key + "' plural deleted.\n";
    }

    if (tmpPlurals.size())
    {
        for (const std::string& item : tmpPlurals)
        {
            addPlural(item);
            out += "The '" + item + "' plural added.\n";
        }
    }

    return out;
}

#ifdef TRANSLATOR_USING_RAPIDJSON
bool TranslatorCreator::doExportJson(const std::string& filename) const
{
    rapidjson::Document document;

    rapidjson::Value strings(rapidjson::kObjectType);
    for (const auto& item : m_data)
    {
        const auto& key = item.first;
        const auto& text = item.second;

        rapidjson::Value json_key;
        rapidjson::Value json_text;

        json_key.SetString(rapidjson::StringRef(key.c_str(), key.size()));
        json_text.SetString(rapidjson::StringRef(text.c_str(), text.size()));
        strings.AddMember(json_key, json_text, document.GetAllocator());
    }

    rapidjson::Value dict(rapidjson::kArrayType);
    for (const auto& item : m_dictionary)
    {
        const auto& key = item.first;
        const auto& plurals = item.second;

        rapidjson::Value json_value(rapidjson::kObjectType);
        rapidjson::Value json_plurals(rapidjson::kArrayType);
        rapidjson::Value json_key;
        rapidjson::Value json_text;

        for (const auto& p : plurals)
        {
            rapidjson::Value json_p;
            json_p.SetString(rapidjson::StringRef(p.c_str(), p.size()));
            json_plurals.PushBack(json_p, document.GetAllocator());
        }

        json_key.SetString(rapidjson::StringRef(key.c_str(), key.size()));
        json_value.AddMember(json_key, json_plurals, document.GetAllocator());

        dict.PushBack(json_value, document.GetAllocator());
    }

    rapidjson::Value json_lang;
    json_lang.SetString(rapidjson::StringRef(getMarker().c_str(), getMarker().size()));

    document.SetObject();
    document.AddMember("language", json_lang, document.GetAllocator());
    document.AddMember("version", TranslatorVersion::Json, document.GetAllocator());
    document.AddMember("strings", strings, document.GetAllocator());
    document.AddMember("dictionary", dict, document.GetAllocator());

    std::ofstream file(filename);
    rapidjson::OStreamWrapper osw(file);
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(osw);
    document.Accept(writer);

    return true;
}
#endif

TranslatorManager::TranslatorManager()
{
    addTranslator(Language::EN_EN);
}

TranslatorManager::~TranslatorManager()
{
    clearTranslators();
}

const Translator* TranslatorManager::getTranslator(Language l) const
{
    for (const auto tr : m_translator)
    {
        if (tr->getLanguage() == l)
        {
            return tr;
        }
    }
    return nullptr;
}

const Translator* TranslatorManager::addTranslator(Language l)
{
    auto tr = getTranslator(l);
    if (tr)
    {
        return tr;
    }

    auto t = new TranslatorCreator(l);
    m_translator.push_back(t);
    return t;
}

VectorOfStrings TranslatorManager::getTranslatorsList() const
{
    VectorOfStrings out;

    for (auto tr : m_translator)
    {
        out.push_back(tr->getMarker());
    }

    return out;
}

bool TranslatorManager::addText(const std::string& sid, const std::string& text)
{
    bool contains = m_translator[0]->contains(sid);
    bool overwrite = contains && m_translator[0]->getText(sid) != text;

    auto plurals = getPluralsFromText(text);
    for (const std::string& plural : plurals)
    {
        for (const auto translator : m_translator)
        {
            translator->addPlural(plural);
        }
    }

    for (const auto translator : m_translator)
    {
        translator->addText(sid, m_translationMissing, overwrite);
    }
    m_translator[0]->addText(sid, text, true);

    return contains;
}

size_t TranslatorManager::clearSidByPrefix(const std::string& prefix)
{
    size_t out = 0;
    for (auto tr : m_translator)
    {
        out = tr->clearSidByPrefix(prefix);
    }

    return out;
}

std::string TranslatorManager::check() const
{
    std::string out = "";
    auto plurals = getAllPlurals();

    for (auto item : m_translator)
    {
        std::string tr_out = item->checkDictionary(plurals);
        if (tr_out.size())
        {
            out += "Language " + item->getMarker() + ":\n" + tr_out;
        }
    }

    return out;
}

std::string TranslatorManager::fix()
{
    std::string out = "";
    auto plurals = getAllPlurals();

    for (auto item : m_translator)
    {
        std::string tr_out = item->fixDictionary(plurals);
        if (tr_out.size())
        {
            out += "Language " + item->getMarker() + ":\n" + tr_out;
        }
    }
    return out;

}

bool TranslatorManager::save(const std::string& filename) const
{
    std::string out = "";
    VectorOfStrings element;

    out += escapeLine(m_delimer, { TranslatorVersion::header, getTranslatorVersion() });

    element.clear();
    element.push_back("sid");
    for (const auto translator : m_translator)
    {
        element.push_back(translator->getMarker());
    }
    out += escapeLine(m_delimer, element);

    for (const auto& item : m_translator[0]->getRawData())
    {
        const std::string& key = item.first;

        element.clear();
        element.push_back(key);
        for (const auto translator : m_translator)
        {
            element.push_back(translator->getText(key));
        }
        out += escapeLine(m_delimer, element);
    }

    std::ofstream outFile(filename);
    if (!outFile.is_open())
    {
        return false;
    }
    outFile.write(out.c_str(), out.size());
    outFile.close();

    auto posDot = filename.find_last_of(".");
    std::string filenameWOEx = posDot != std::string::npos ? filename.substr(0, posDot) : filename;
    for (auto tr : m_translator)
    {
        tr->save(getDictionaryFilename(filenameWOEx, tr), m_delimer);
    }

    return true;
}

bool TranslatorManager::load(const std::string& filename)
{
    clearTranslators();

    std::ifstream inFile(filename/*, std::ios_base::app*/);
    if (!inFile.is_open())
    {
        return false;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();

    std::string line;
    uint32_t lineno = 1;
    while (std::getline(buffer, line))
    {
        std::vector<std::string> element;

        if (!parseLine(line, m_delimer, element))
        {
            return false;
        }

        if (lineno == 1)
        {
            if (element.size() < 2)
            {
                return false;
            }

            if (element[0] != TranslatorVersion::header)
            {
                return false;
            }

            if (element[1] != getTranslatorVersion())
            {
                return false;
            }
        }
        else if (lineno == 2)
        {
            if (element.size() < 2)
            {
                return false;
            }
            if (element[0] != "sid")
            {
                return false;
            }

            for (auto iter = element.begin() + 1; iter != element.end(); iter++)
            {
                addTranslator(Translator::MarkerToLanguage(*iter));
            }
        }
        else
        {
            if (element.size() != m_translator.size() + 1)
            {
                clearTranslators();
                return false;
            }

            for (size_t ii = 0; ii < m_translator.size(); ++ii)
            {
                m_translator[ii]->addText(element[0], element[ii + 1], true);
            }
        }

        ++lineno;
    }

    auto posDot = filename.find_last_of(".");
    std::string filenameWOEx = posDot != std::string::npos ? filename.substr(0, posDot) : filename;

    for (auto tr : m_translator)
    {
        if (!tr->load(getDictionaryFilename(filenameWOEx, tr), m_delimer))
        {
            return false;
        }
    }

    return true;
}

#ifdef TRANSLATOR_USING_RAPIDJSON
bool TranslatorManager::doExportJson(const std::string& path) const
{
    for (const auto& tr : m_translator)
    {
        std::string filename = path + "translator_" + tr->getMarker() + ".json";
        if (!tr->doExportJson(filename))
        {
            return false;
        }
    }
    return true;
}
#endif

TranslatorCreator* TranslatorManager::findTranslator(const std::string& name)
{
    for (auto item : m_translator)
    {
        if (item->getMarker() == name)
        {
            return item;
        }
    }

    return nullptr;
}

void TranslatorManager::clearTranslators()
{
    for (auto item : m_translator)
    {
        delete item;
    }
    m_translator.clear();
}

std::unordered_set<std::string> TranslatorManager::getAllPlurals() const
{
    std::unordered_set<std::string> out;

    for (const auto& item : m_translator[0]->getRawData())
    {
        auto key = item.first;
        const std::string& text = item.second;

        auto v = getPluralsFromText(text);

        out.insert(v.begin(), v.end());
    }

    return out;
}

}
