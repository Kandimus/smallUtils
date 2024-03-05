#ifndef _SMALLUTILL_TRANSLATOR_MANAGER_H_
#define _SMALLUTILL_TRANSLATOR_MANAGER_H_
#pragma once

#include "translator.h"

namespace su
{

class TranslatorCreator : public Translator
{
public:
    TranslatorCreator(Language l) : Translator(l) {}
    virtual ~TranslatorCreator() = default;

    bool addText(const std::string& sid, const std::string& text, bool overwrite);
    bool addPlural(const std::string& sid);
    size_t clearSidByPrefix(const std::string& prefix);

    bool save(const std::string& filename, char delimer) const;
    bool load(const std::string& filename, char delimer);

#ifdef TRANSLATOR_USING_RAPIDJSON
    bool doExportJson(const std::string& filename) const;
#endif

    std::string checkDictionary(const std::unordered_set<std::string>& plurals) const;

    const std::unordered_map<std::string, std::string>& getRawData() const { return m_data; }
};

class TranslatorManager
{
public:
    TranslatorManager();
    virtual ~TranslatorManager();

    const Translator* getTranslator(Language l) const;
    const Translator* addTranslator(Language l);
    std::vector<std::string> getTranslatorsList() const;

    bool addText(const std::string& sid, const std::string& text);
    inline std::string getText(Language l, std::string& sid) const
        { return m_translator[0]->getText(sid); };
    size_t clearSidByPrefix(const std::string& prefix);

    inline void setDelimer(char delimer)
        { m_delimer = delimer; };
    inline char getDelimer() const
        { return m_delimer; }

    inline void setTranslationMissing(char text)
        { m_translationMissing = text; };
    inline const std::string& getTranslationMissing() const
        { return m_translationMissing; }

    std::string check() const;

    bool save(const std::string& filename) const;
    bool load(const std::string& filename);

#ifdef TRANSLATOR_USING_RAPIDJSON
    bool doExportJson(const std::string& path) const;
#endif

protected:
    TranslatorCreator* findTranslator(const std::string& name);
    void clearTranslators();

protected:
    std::vector<TranslatorCreator*> m_translator;
    char m_delimer = '|';
    std::string m_translationMissing = "<# Translation missing #>";
};

inline std::string TranslatorManager::getText(Language l, std::string& sid) const
{
    auto tr = getTranslator(l);
    return tr ? tr->getText(sid) : m_translator[0]->getText(sid);
}

}

#endif