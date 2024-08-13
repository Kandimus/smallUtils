//=================================================================================================
//===
//=== commandline.h
//===
//=== Copyright (c) 2020-2023 by RangeSoft.
//=== All rights reserved.
//===
//=== Litvinov "VeduN" Vitaly
//===
//=================================================================================================
#ifndef _SIMPLEUTILS_COMMANDLINE_H_
#define _SIMPLEUTILS_COMMANDLINE_H_
#pragma once

#include <vector>
#include <string>
#include <utility>

namespace su
{

using CommandLineOption = std::pair<std::string, unsigned char>;

class CommandLine
{
private:
    struct Item
    {
        bool          isSwitch = false;
        unsigned int  isSet = 0;
        std::string   fullname = "";
        unsigned char shortname = (unsigned char)0;
        std::string   value = "";
        std::string   description = "";
    };

#ifdef SIMPLEUTILS_COMMANDLINE_AS_SINGLETON
public:
    static CommandLine& instance() { static CommandLine Singleton; return Singleton; }
private:
    CommandLine() = default;
    CommandLine(const CommandLine&);
    CommandLine& operator=(CommandLine&);
#else
public:
    CommandLine() = default;
#endif

public:
    virtual ~CommandLine() = default;

public:
    inline CommandLine& addSwitch(const CommandLineOption& name, const std::string& description)
    {
        return addSwitch(name.first, name.second, description);
    }

    CommandLine& addSwitch(const std::string& name, unsigned char shortname, const std::string& description)
    {
        auto item = findItem(name);

        if (!item)
        {
            m_list.push_back(CommandLine::Item());
            item = &m_list.back();
        }

        updateMaxLength(name.size());

        item->isSwitch  = true;
        item->fullname  = name;
        item->shortname = shortname;
        item->description = description;

        return *this;
    }

    inline CommandLine& addOption(const CommandLineOption& name, const std::string& default_value, const std::string& description)
    {
        return addOption(name.first, name.second, default_value, description);
    }

    CommandLine& addOption(const std::string& name, const unsigned char shortname, const std::string& default_value, const std::string& description)
    {
        auto item = findItem(name);

        if (!item)
        {
            m_list.push_back(CommandLine::Item());
            item = &m_list.back();
        }

        updateMaxLength(name.size());

        item->isSwitch  = false;
        item->fullname  = name;
        item->shortname = shortname;
        item->value     = default_value;
        item->description = description;

        return *this;
    }

    CommandLine& setSwitch(const std::string& name, bool isset)
    {
        auto item = findItem(name);

        if (!item)
        {
            m_list.push_back(CommandLine::Item());
            item = &m_list.back();
            item->fullname = name;
        }

        item->isSet = isset;

        return *this;
    }

    CommandLine& setOption(const std::string& name, const std::string& value)
    {
        auto item = findItem(name);

        if (!item)
        {
            m_list.push_back(CommandLine::Item());
            item = &m_list.back();

            item->fullname = name;
        }

        item->value = value;
        item->isSet = true;

        return *this;
    }

    inline int isSet(const CommandLineOption& name) const
    {
        return isSet(name.first);
    }
    unsigned int isSet(const std::string& name) const
    {
        auto item = findConstItem(name);
        return item ? item->isSet : false;
    }

    inline const std::string& getOption(const CommandLineOption& name) const
    {
        return getOption(name.first);
    }
    const std::string& getOption(const std::string& name) const
    {
        static std::string emptyString = "";
        auto item = findConstItem(name);
        return item ? item->value : emptyString;
    }

    std::string  getArgument(unsigned int num) const
    {
        return num < m_argument.size() ? m_argument[num] : "";
    }

    size_t getCountArgument() const
    {
        return m_argument.size();
    }

    void printArguments() const
    {
        printf("arguments:\n");

        for (const auto& item : m_list)
        {
            std::string whites;
            for (size_t ii = item.fullname.size(); ii < m_maxArgLength + 4; ++ii) whites += ' ';

            std::string shortName = item.shortname ? std::string("-") + (char)item.shortname : "  ";
            printf("    --%s%s%s  %s\n",
                item.fullname.c_str(), whites.c_str(), shortName.c_str(), item.description.c_str());
        }
        printf("\n");
    }

    unsigned int parse(unsigned int argc, const char **argv)
    {
	    Item* curArg = nullptr;

	    for (unsigned int ii = 1; ii < argc; ++ii)
        {
            std::string arg = argv[ii];
            bool found = false;

            if (arg.size() > 1 && arg[0] == '-')
            {
                if (arg[1] == '-')
                {
                    found = arg.size() > 2;
                }
                else
                {
                    found = true;
                }
            }

            if (found)
            {
                Item* oldArg = curArg;
                bool assigned = false;

                curArg = (arg[1] != '-') ? parseShort(arg) : parseFull(arg, assigned);

                if (assigned)
                {
                    continue;
                }

                if (curArg)
                {
                    if (curArg->isSwitch)
                    {
                        curArg = nullptr;
                    }
                    continue;
                }

                if (oldArg)
                {
                    curArg = oldArg;
                }
            }

            if (curArg)
            {
                curArg->value = arg;
                ++curArg->isSet;
                curArg = nullptr;
                continue;
            }
            else
            {
                m_argument.push_back(arg);
            }
        }

        return 0;
    }

private:
    const Item* findConstItem(const std::string& name) const
    {
        for (auto& item : m_list)
        {
            if (item.fullname == name)
            {
                return &item;
            }
        }
        return nullptr;
    }

    Item* findItem(const std::string& name)
    {
        for (auto& item : m_list)
        {
            if (item.fullname == name)
            {
                return &item;
            }
        }
        return nullptr;
    }

    CommandLine::Item* parseShort(const std::string& arg)
    {
        std::string str = arg.substr(1);
        CommandLine::Item* lastItem = nullptr;
        
        for (auto& item : m_list)
        {
            auto pos = str.find(item.shortname);
            if(pos != std::string::npos)
            {
                lastItem = &item;
                if (item.isSwitch)
                {
                    ++item.isSet;
                }
            }
        }

        return arg.size() == 2 ? lastItem : nullptr;
    }

    Item* parseFull(const std::string& arg, bool& assigned)
    {
        std::string name = arg.substr(2);
        std::string value = "";
        auto pos = name.find('=');

        assigned = false;
        if (pos != std::string::npos)
        {
            value = name.substr(pos + 1);
            name  = name.substr(0, pos);
        }

        for (auto& item : m_list)
        {
            if (item.fullname == name)
            {
                if (item.isSwitch)
                {
                    //item.isSet += pos >= 0;
                    ++item.isSet;
                }
                else if (value.size())
                {
                    ++item.isSet;
                    item.value = value;
                    assigned = true;
                    return nullptr;
                }

                return &item;
            }
        }

        return nullptr;
    }

    void updateMaxLength(size_t len)
    {
        m_maxArgLength = m_maxArgLength > len ? m_maxArgLength : len;
    }

private:
    std::vector<CommandLine::Item> m_list;
    std::vector<std::string> m_argument;
    size_t m_maxArgLength = 12;
};

}

#endif
