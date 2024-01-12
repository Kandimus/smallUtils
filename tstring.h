
namespace su
{

struct TString
{
    uint32_t id;
    std::string text;
}

struct Language
{
    std::string name;
    UDINT setup;
    std::vector<TString> texts;
}

class Translator
{
public:
	Translator();
	virtual ~Translator();

public:
	bool load(const std::string& filename);
	bool setCurLang(const std::string& lang);
	std::string getCurLang() const;

	bool get(STRID id, const std::string& lang, std::string& text) const;
	bool        get(STRID id, std::string& text) const;
	const std::string* getPtr(STRID id, const std::string& lang) const;
	const std::string* getPtr(STRID id) const;

	void getListLang(std::vector<std::string>& list) const;
	//void getListSID(const std::string& lang, std::vector<rTextItem>& list) const;

private:
	UDINT         loadLang(tinyxml2::XMLElement* root, UDINT create, rError& err);
	UDINT         deleteUnused();
	rTextLang* getLangPtr(const std::string& name) const;
	const std::string* getPtr(STRID id, rTextLang* lang) const;

private:
	std::list<Language>  m_langs;
	Language* m_curLang;
};

}