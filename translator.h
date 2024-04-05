#ifndef _SMALLUTILL_TRANSLATOR_H_
#define _SMALLUTILL_TRANSLATOR_H_
#pragma once

#define TRANSLATOR_USING_RAPIDJSON


#include <assert.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#ifdef TRANSLATOR_USING_RAPIDJSON
#include "rapidjson/document.h"
#endif

using VectorOfStrings = std::vector<std::string>;

namespace su
{

enum Language
{
    UNDEF = 0,
    EN_EN,
    RU_RU,
};

namespace TranslatorVersion
{
const int major = 1;
const int minor = 0;
const std::string header = "TRANSLATOR";
const std::string headerDict = "TRANSLATOR-DICTIONARY";
#ifdef TRANSLATOR_USING_RAPIDJSON
const int Json = 1;
#endif
}

class Translator
{
public:
    // Constructor for all file formats
    Translator(const std::string& filename)
        { init(filename); }
    Translator(const std::string& text, const std::string& format)
        { init(text, format); }
#ifdef TRANSLATOR_USING_RAPIDJSON
    Translator(const rapidjson::Document& root)
        { init(root); }
#endif
    virtual ~Translator() = default;

    const std::string& getMarker() const
        { return m_marker; }
    Language getLanguage() const
        { return m_language; }

    bool isInit() const
        { return m_isInit; }

    inline std::string getText(const std::string& sid) const;
    std::string getText(const std::string& sid, int value) const;
    bool contains(const std::string& sid) const
        { return m_data.find(sid) != m_data.end(); }
    
    std::string getPlural(const std::string& sid, int value) const;

    inline std::string operator [](const std::string& sid) const
        { return getText(sid); }

    static Language MarkerToLanguage(const std::string& marker);
    static std::string LanguageName(const std::string& marker);

    static std::string stringToSID(const std::string& text);

protected:
    Translator(Language l)
        { assert(l != Language::UNDEF); init(l); }

    inline void clear();

    void init(Language l);
    void init(const std::string& filename);
    void init(const std::string& filename, const std::string& format);
#ifdef TRANSLATOR_USING_RAPIDJSON
    void init(const rapidjson::Document& root);
#endif

    std::string getMissingSid(const std::string& sid) const
        { return "<# Missing sid '" + sid + "' #>"; }
    std::string getMissingPluralSid(const std::string& sid) const
        { return "<# Missing plural '" + sid + "' #>"; }

    uint8_t getPluralRule() const
        { return m_pluralRule; }
    uint8_t getPluralForms() const
        { return m_pluralForms; }
    uint8_t getPluralForm(int v) const;

protected:
    Language m_language;
    uint8_t m_pluralRule;
    uint8_t m_pluralForms;
    std::string m_marker;
    bool m_isInit = false;

    std::unordered_map<std::string, VectorOfStrings> m_dictionary;
    std::unordered_map<std::string, std::string> m_data;
};

inline std::string Translator::getText(const std::string& sid) const
{
    return (m_data.find(sid) == m_data.end()) ? getMissingSid(sid) : m_data.at(sid);
}

inline void Translator::clear()
{
    m_data.clear();
    m_dictionary.clear();
}

}
#endif