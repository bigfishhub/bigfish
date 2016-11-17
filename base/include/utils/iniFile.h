#ifndef __INIFILE_H__
#define __INIFILE_H__

#include <stdio.h>
#include <string>
//#include <vector>
//#include <list>

using std::string;

#define MAXINISECTION 20 //section 最大支持多少

typedef struct IniItem_s
{
    string key;
    string value;
    //string comment;
    struct IniItem_s *next;
    struct IniItem_s *prev;
}IniItem_t;

class IniSection
{
public:
    string title;
    //string comment;

    IniSection();
    ~IniSection();

    int addItems(const IniItem_t& items);
    int delItems(const string key);
    int findItems(const string key, string& value);
    int clearAllTtems();
    void outputValue();

    IniSection& operator= (const IniSection &rhs);

private:
    IniItem_t *iniItems;
};

class IniFile
{
public:
	IniFile();
	~IniFile();

	int open(const string &fname);
	int close();
	
    int getStringValue(const string& title, const string& key, string& value);
    int getStringValue(const string& title, const string& key, long& value);

    int getline(string& str, FILE* fp);
    int trimCharacter(string& line,const string args);
    int parse(const string& content, string& key, string& value, char divChar = '=', char endChar = ';');
    void outputValue();

private:
	int releaseData();

private:
	IniSection Section[MAXINISECTION];
};

#endif