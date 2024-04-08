
#include "translator.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "stringex.h"

namespace su
{

static std::vector<std::string> gMarker = { "UNDEF", "EN_EN", "RU_RU" };

uint8_t Translator::getPluralForm(int v) const
{
    int v_a = abs(v);
    int v_a_m10 = v_a % 10;
    //int v_m10 = v % 10;

    switch (m_pluralRule)
    {
        // RULE 7 Families: Slavic (Belarusian, Bosnian, Croatian, Serbian, Russian, Ukrainian)
        case 7:
            if (v_a != 11 && v_a_m10 == 1) return 0;
            if (v_a != 12 && v_a != 13 && v_a != 14 && (v_a_m10 == 2 || v_a_m10 == 3 || v_a_m10 == 4)) return 1;
            return 2;

        // FORM 1 Families: Germanic (Danish, Dutch, English, Faroese, Frisian, German, Norwegian, Swedish),
        //                  Finno-Ugric (Estonian, Finnish, Hungarian), Language isolate (Basque),
        //                  Latin/Greek (Greek),
        //                  Semitic (Hebrew), Romanic (Italian, Portuguese, Spanish, Catalan), Vietnamese
        default:
            return (v_a == 1) ? 0 : 1;
    }
}

std::string Translator::getPlural(const std::string& sid, int value) const
{
    if (m_dictionary.find(sid) == m_dictionary.end())
    {
        return getMissingPluralSid(sid);
    }

    auto v = &m_dictionary.at(sid);
    auto form = getPluralForm(value);
    return (form < 0 || form >= v->size())
        ? getMissingPluralSid(sid)
        : v->at(form);
}

std::string Translator::getText(const std::string& sid, int value) const
{
    std::string text = getText(sid);

    // check a text for dictionary markers
    std::size_t begPos = 0;
    std::size_t endPos = 0;
    while ((begPos = text.find("@{", endPos)) != std::string::npos)
    {
        if ((endPos = text.find("}", begPos)) != std::string::npos)
        {
            std::string plural = text.substr(begPos + 2, endPos - begPos - 2);

            text.replace(text.begin() + begPos, text.begin() + endPos + 1, std::to_string(value) + " " + getPlural(plural, value));
        }
        begPos = endPos = 0;
    }

    return text;
}

Language Translator::MarkerToLanguage(const std::string& marker)
{
    for (int ii = 0; ii < gMarker.size(); ++ii)
    {
        if (marker == gMarker[ii])
        {
            return static_cast<Language>(ii);
        }
    }
    return Language::UNDEF;
}

std::string Translator::LanguageName(const std::string& marker)
{
    Language l = Translator::MarkerToLanguage(marker);

    switch (l)
    {
        case Language::UNDEF: return "Undefined";
        case Language::RU_RU: return "Русский";

        default: return "English";
    }
}

std::string Translator::stringToSID(const std::string& text)
{
    std::string out = "";

    for (auto ch : text)
    {
        if (std::isalnum(ch))
        {
            out += std::tolower(ch);
        }
        else
        {
            if (out.size() && out[out.size() - 1] == '_')
            {
                continue;
            }
            out += "_";
        }
    }

    return out;
}

void Translator::init(Language l)
{
    clear();
    m_language = l;
    m_marker = gMarker[l];

    switch (m_language)
    {
    case Language::RU_RU:
        m_pluralRule = 7;
        m_pluralForms = 3;
        break;

    default:
        m_pluralRule = 1;
        m_pluralForms = 2;
        break;
    }
}

void Translator::init(const std::string& filename)
{
    init(Language::UNDEF);

    auto pos = filename.find_last_of(".");

    if (pos == std::string::npos)
    {
        return;
    }

    std::string ext = filename.substr(pos + 1);

#ifdef TRANSLATOR_USING_RAPIDJSON
    if (ext == "json")
    {
        std::ifstream file(filename/*, std::ios_base::app*/);
        if (file.is_open())
        {
            std::stringstream buffer;
            buffer << file.rdbuf();
            init(buffer.str(), "json");
            return;
        }
    }
#endif
 }

void Translator::init(const std::string& text, const std::string& format)
{
    clear();

#ifdef TRANSLATOR_USING_RAPIDJSON
    if (format == "json")
    {
        rapidjson::Document document;
        document.Parse(text.c_str());

        if (document.IsObject())
        {
            init(document);
        }
        return;
    }
#endif
}

#ifdef TRANSLATOR_USING_RAPIDJSON
void Translator::init(const rapidjson::Document& root)
{
    clear();
    if (!root.HasMember("version"))
    {
        return;
    }

    if (root["version"].GetInt() != TranslatorVersion::Json)
    {
        return;
    }

    if (!root.HasMember("language") || !root.HasMember("strings") || !root.HasMember("dictionary"))
    {
        return;
    }

    std::string lang = root["language"].GetString();

    init(MarkerToLanguage(lang));

    if (getLanguage() == Language::UNDEF)
    {
        return;
    }

    const rapidjson::Value& json_strs = root["strings"];
    const rapidjson::Value& json_dict = root["dictionary"];

    if (!json_strs.IsObject() || !json_dict.IsObject())
    {
        return;
    }

    for (auto val = json_strs.MemberBegin(); val != json_strs.MemberEnd(); ++val)
    {
        std::string key = val->name.GetString();
        std::string text = val->value.GetString();

        m_data[key] = text;
    }

    for (auto val = json_dict.MemberBegin(); val != json_dict.MemberEnd(); ++val)
    {
        std::string key = val->name.GetString();

        if (!val->value.IsArray() || val->value.Size() != getPluralForms())
        {
            init(Language::UNDEF);
            return;
        }

        std::vector<std::string> v;
        for (rapidjson::SizeType jj = 0; jj < val->value.Size(); ++jj)
        {
            v.push_back(val->value[jj].GetString());
        }
        m_dictionary[key] = v;
    }

    m_isInit = true;
}
#endif

}

