#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/iniFile.h"

using std::string;

//#define INITEST
#ifdef INITEST
#define LOGE printf
#define LOGD printf
#define LOGI printf
#else
#define LOGE(...) ((void)0) 
#define LOGD(...) ((void)0) 
#define LOGI(...) ((void)0)
#endif

#define INI_BUF_SIZE 1024
/**Error code*/
#define RET_OK    0
#define RET_ERR   -1

IniFile::IniFile()
{

}

IniFile::~IniFile()
{

}

int IniFile::open(const string &filename)
{
	FILE* fp;
	string line;
	//IniSection Sect;
	int IniDex = -1;
	int lineCount = 0;

	fp = fopen(filename.c_str(), "r");
	if (fp == NULL )
    {
        LOGE("[IniFile] Coun't open file(%s)\n",filename.c_str());
        return RET_ERR;
    }

    //release
    releaseData();

    while(getline(line,fp) && IniDex < MAXINISECTION)
    {
    	lineCount++;
    	if (lineCount > 2000) lineCount=0;

    	trimCharacter(line," ");
    	trimCharacter(line,"\r");
    	trimCharacter(line,"\n");
    	if (line.empty()) continue;
    	if ('#' == line[0]) continue;

    	LOGI("line.size:%lu,line:%s\n",line.size(), line.c_str());
    	if (line[0] == '[')
    	{
    		int index = line.find("]");

    		if (index < 0)
    		{
    			fclose(fp);
    			LOGE("[IniFile] Cann't find ']'\n");
    			return RET_ERR;
    		}

    		int len = index;

    		if (len <= 0)
            {
                LOGI("[IniFile] Section is null\n");
                continue;
            }

            trimCharacter(line,"[");
    		trimCharacter(line,"]");
    		IniDex++;
            Section[IniDex].title = line;
    	}
    	else
    	{
    		IniItem_t its;
    		string key, keyvalue;

    		if (parse(line,key,keyvalue) == RET_OK)
    		{
    			its.key = key;
    			its.value = keyvalue;
    			Section[IniDex].addItems(its);
    		}
    		else
    		{
    			printf("[IniFile] Parse fail! [%s]\n", line.c_str());
    			printf("please check \"%s\" %d lines,thanks!\n",filename.c_str(),lineCount);
                fclose(fp);
                return RET_ERR;
    		}
    	}
    }

    if (IniDex >= MAXINISECTION)
    {
    	LOGE("[IniFile] open,file too long than %d line!fail!\n",MAXINISECTION);
    	return RET_ERR;
    }
    return RET_OK;
}

int IniFile::close()
{
	releaseData();
	return RET_OK;
}

int IniFile::trimCharacter(string& line,const string args)
{
	int index = 0;

	while((index = line.find(args)) != -1)
    {
    	line.erase(index,args.size());
    	//LOGI("index:%d,line:%s\n",index, line.c_str());
    }

    return RET_OK;
}

int IniFile::getline(string& str, FILE* fp)
{
    int plen = 0;
    int buf_size = INI_BUF_SIZE * sizeof(char);
    char* pbuf = NULL;
    char* buf = (char*) malloc(buf_size);
    char* p = buf;

    if (buf == NULL || p == NULL) /*solve pclint e449*/
    {
        LOGE("[IniFile]No enough memory!exit!");
        return RET_ERR;
    }

    memset(buf, 0, buf_size);
    int total_size = buf_size;

    while (fgets(p, buf_size, fp) != NULL)
    {
        plen = strlen(p);

        if ( plen > 0 && p[plen - 1] != '\n' && !feof(fp))
        {
            total_size = strlen(buf) + buf_size;
            pbuf = (char*)realloc(buf, total_size);

            if (!pbuf)
            {
                LOGE("[IniFile] No enough memory!");
                return RET_ERR;
            }

            buf = pbuf;
            p = buf + strlen(buf);

            continue;
        }
        else
        {
            break;
        }
    }

    str = buf;

    if (buf)
    {
        free(buf);
        buf = NULL;
    }

    return str.length();
}

int IniFile::parse(const string& content, string& key, 
			string& value, char divChar/* = '='*/, char endChar/* = ';'*/)
{
	int index = 0,index2 = 0;

	if ((index = content.find(divChar)) <= 0)
	{
		LOGE("[IniFile] parse not find '%c'!\n",divChar);
		return RET_ERR;
	}
	key = string(content,0,index);
	if ((index2 = content.find(endChar)) <= 0)
	{
		LOGE("[IniFile] parse not find '%c'!\n",endChar);
		index2 = content.length();
		value = string(content,(index+1),(index2-index-1));
		return RET_OK;
	}
	value = string(content,(index+1),(index2-index-1));
	return RET_OK;
}

int IniFile::getStringValue(const string& title, const string& key, string& value)
{
	int i = 0;

	for (i=0;i<MAXINISECTION;i++)
	{
		if (Section[i].title == title)
		{
			return Section[i].findItems(key,value);
		}
	}
	return RET_ERR;
}

int IniFile::getStringValue(const string& title, const string& key, long& value)
{
	int ret = RET_ERR;
	string keyValue;
	long index = -1;
	long intValue = 0;

	ret = getStringValue(title,key,keyValue);
	if (ret == RET_OK)
	{
		if((index = keyValue.find("0x")) < 0 && (index = keyValue.find("0X")) < 0)
		{
			value = atol(keyValue.c_str());
		}
		else
		{
			intValue = 0;
			index +=2;
			while (index < (long)keyValue.size())
			{
				printf("c:%c\n", keyValue[index]);
				if (keyValue[index] >= '0' && keyValue[index] <= '9')
				{
					intValue = (intValue<<4) + keyValue[index] - '0';
				}
				else if (keyValue[index] >= 'a' && keyValue[index] <= 'f')
				{
					intValue = (intValue<<4) + keyValue[index] - 'a' + 10;
				}
				else if (keyValue[index] >= 'A' && keyValue[index] <= 'F')
				{
					intValue = (intValue<<4) + keyValue[index] - 'A' + 10;
				}
				else
				{
					break;
				}
				index++;
			}
			value = intValue;
		}
		
	}
	return ret;
}

int IniFile::releaseData()
{
	int i = 0;
	for (i=0; i<MAXINISECTION; i++)
	{
		Section[i].title = "";
		Section[i].clearAllTtems();
	}
	return RET_OK;
}

void IniFile::outputValue()
{
	int i = 0;

	for (i=0; i<MAXINISECTION; i++)
	{
		if (Section[i].title != "")
		{
			printf("\n[%d]title:[%s]\n",i, Section[i].title.c_str());
			Section[i].outputValue();
		}
	}
}

IniSection::IniSection()
{
	iniItems = NULL;
}

IniSection::~IniSection()
{
	clearAllTtems();
}

int IniSection::addItems(const IniItem_t& items)
{
	IniItem_t* it = new IniItem_t();
	if (NULL == it)
	{
		LOGE("[IniSection] malloc fail!\n");
		return RET_ERR;
	}
	it->key   = items.key;
	it->value = items.value;
	it->next  = NULL;
	it->prev  = NULL;
	if (!iniItems)
	{
		iniItems = it;
		it->prev = NULL;
		LOGD("[IniSection] addItems success! it:%p\n",it);
		return RET_OK;
	}

	IniItem_t *head = iniItems;

	while (head)
	{
		if (!head->next)
		{
			head->next = it;
			it->prev = head;
			LOGD("[IniSection] addItems success! it:%p\n",it);
			return RET_OK;
		}
		else
		{
			head = head->next;
		}
	}	
	return RET_ERR;
}

int IniSection::delItems(const string key)
{

	return RET_OK;
}

int IniSection::findItems(const string key, string& value)
{
	IniItem_t *head = iniItems;

	while (head)
	{
		if (!head->next)
		{
			LOGE("[IniSection] findItems fail!\n");
			break;
		}
		else
		{
			if (key == head->key)
			{
				value = head->value;
				LOGE("[IniSection] findItems success!\n");
				return RET_OK;
			}
			head = head->next;
		}
	}

	return RET_ERR;
}

int IniSection::clearAllTtems()
{
	IniItem_t *curritems = NULL;
	IniItem_t *head = iniItems;

	while(head)
	{
		curritems = head;
		head = head->next;
		delete (curritems);
		LOGD("[IniSection] clear items !p:%p\n",curritems);
	}
	iniItems = NULL;
	return RET_OK;
}

IniSection& IniSection::operator= (const IniSection &rhs)
{
	IniItem_t *items = rhs.iniItems;
	title = rhs.title;

	while (items)
	{
		addItems(*items);
		items = items->next;
	}
	return *this;
}

void IniSection::outputValue()
{
	IniItem_s* items = NULL;
	items = iniItems;
	while (items)
	{
		printf("key:%s,keyvalue:%s\n", 
			items->key.c_str(),items->value.c_str());
		items = items->next;
	}
}