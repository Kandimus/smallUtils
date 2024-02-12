
#include <string>
#include <vector>

namespace su
{

class TString
{
public:
	TString() = default;
	TString(const TString& t) { *this = t; }
	TString(uint32_t v) { *this = v; }
	virtual ~TString() = default;

	uint32_t getId() const { return id; }
	const std::string getText() const { return text; }

	uint32_t operator = (uint32_t v) { id = v; text = ""; return id; }
	std::string& operator = (const char* t) { id = -1; text = t; return text; }
	std::string& operator = (const std::string& t) { id = -1; text = t; return text; }
	TString& operator = (const TString& t) { id = t.getId(); text = t.getText(); return *this; }

	//const uint32_t Undef = 0xffffffffU;
	static inline uint32_t Undef() { return 0xffffffffU; }

protected:
	uint32_t id;
	std::string text;
};

class Language
{
	std::string name;
	uint32_t setup;
	std::vector<TString> texts;
};

//class Translator
//{
//public:
//	Translator();
//	virtual ~Translator();
//
//public:
//	bool load(const std::string& filename);
//	bool setCurLang(const std::string& lang);
//	std::string getCurLang() const;
//
//	bool get(STRID id, const std::string& lang, std::string& text) const;
//	bool        get(STRID id, std::string& text) const;
//	const std::string* getPtr(STRID id, const std::string& lang) const;
//	const std::string* getPtr(STRID id) const;
//
//	void getListLang(std::vector<std::string>& list) const;
//	//void getListSID(const std::string& lang, std::vector<rTextItem>& list) const;
//
//private:
//	UDINT         loadLang(tinyxml2::XMLElement* root, UDINT create, rError& err);
//	UDINT         deleteUnused();
//	rTextLang* getLangPtr(const std::string& name) const;
//	const std::string* getPtr(STRID id, rTextLang* lang) const;
//
//private:
//	std::list<Language>  m_langs;
//	Language* m_curLang;
//};

}