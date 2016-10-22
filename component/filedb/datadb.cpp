#define LOG_TAG "DATADB"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utils/Logger.h>
#include <dirent.h>
#include <errno.h>
#include "datadb.h"

DATADB::DATADB()
{
}

DATADB::~DATADB()
{
}

_S32 DATADB::create_dir(const _CHAR *path)
{
    _CHAR str[256] = {0};
    _S32 ret = -1;

    LOGI("create dir %s \n", path);
    sprintf(str, "mkdir -p %s\n", path);
    ret = system(str);
    if (0 != ret)
    {
        LOGE("create dir fail! path : %s \n", path);
        return FAILURE;
    }
    return SUCCESS;
}

_S32 DATADB::_delete_dir(const _CHAR* dirname)
{
    _S32 ret = 0;
    _CHAR path[256] = {0};
    DIR * dir;
    struct dirent * ptr;
    dir = opendir(dirname);
    if (!dir)
    {
        LOGE("opendir \"%s\" fail!\n", dirname);
        return FAILURE;
    }

    while((ptr = readdir(dir)) != _NULL)
    {
        //LOGD("d_name : %s\n", ptr->d_name);
        //LOGD("d_type : %d\n", ptr->d_type);
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
            continue;

        if (ptr->d_type == 4)
        {
            memset(path,0x00,sizeof(path));
            strcpy(path,dirname);
            strcat(path,"/");
            strcat(path,ptr->d_name);
            LOGD("path dir:\"%s\"\n", path);
            ret = _delete_dir(path);
            if (ret != 0)
            {
                LOGE("deletedir \"%s\" fail!\n", path);
            }
        }
        else
        {
            memset(path,0x00,sizeof(path));
            strcpy(path,dirname);
            strcat(path,"/");
            strcat(path,ptr->d_name);
            LOGD("path :\"%s\"\n", path);
            ret = remove(path);
            if (ret != 0)
            {
                LOGE("remove \"%s\" fail! %s\n",ptr->d_name, strerror(errno));
            }
            LOGD("remove \"%s\"\n", path);
        }
    }
    closedir(dir);
    ret = remove(dirname);
    if (ret != 0)
    {
        LOGE("remove \"%s\" fail! %s\n",ptr->d_name, strerror(errno));
    }
    return SUCCESS;
}

_S32 DATADB::delete_dir(const _CHAR* dirname)
{
    _CHAR abs_path[512];

    memset(abs_path, 0, 512);
    //strcpy(abs_path, DATADB_PATH);
    //strcat(abs_path, "/");
    strcat(abs_path, dirname);
    
    if (access(abs_path, F_OK) < 0)
    {
        LOGE("[%s] dir does not exist!\n",abs_path);
        return FAILURE;
    }
    return _delete_dir(abs_path);
}

_S32 DATADB::delete_file(const _CHAR *rel_path)
{
    _CHAR abs_path[512];
    _S32 ret = 0;

    memset(abs_path, 0, 512);
    //strcpy(abs_path, DATADB_PATH);
    //strcat(abs_path, "/");
    strcat(abs_path, rel_path);
    
    if (access(abs_path, F_OK) < 0)
    {
        LOGE("[%s] File does not exist!\n",abs_path);
        return FAILURE;
    }

    ret = remove(abs_path);
    if (-1 == ret)
    {
        LOGE("delete [%s] failed!\n",abs_path);
        return FAILURE;
    }

    return SUCCESS;
}

_S32 DATADB::get_file_length(_CHAR *rel_path, _S64 *length)
{
    FILE* fp;
    _CHAR abs_path[512];

    memset(abs_path, 0, 512);
    //strcpy(abs_path, DATADB_PATH);
    //strcat(abs_path, "/");
    strcat(abs_path, rel_path);

    LOGE("PATH:%s", abs_path);

    if (access(abs_path, 0) != 0)
    {
        LOGE("The file does not exist!\n");
        return FAILURE;
    }
    else
    {
        fp = fopen(abs_path, "rb");
        if (_NULL == fp) 
        {
            LOGE("open file error:");
            return FAILURE;
        }
	
        fseek(fp, 0, SEEK_END);
        *length = (_S64) ftell(fp);
        fclose(fp);
        return SUCCESS;
    }
}

_S32 DATADB::get_file(_CHAR *rel_path, _CHAR *data, _S64 length)
{
    FILE* fp;
    _S64 size = 0;
    _CHAR abs_path[512];

    memset(abs_path, 0, 512);
    //strcpy(abs_path, DATADB_PATH);
    //strcat(abs_path, "/");
    strcat(abs_path, rel_path);

    LOGE("PATH:%s", abs_path);
    
    if (access(abs_path, 0) != 0)
    {
        LOGE("The file does not exist!\n");
        return FAILURE;
    }
    else
    {
        fp = fopen(abs_path, "rb");
        if (_NULL == fp) 
        {
            LOGE("open file error:");
            return FAILURE;
        }
	
        fseek(fp, 0, SEEK_END);
        size = (_S64) ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (size >= length)
        {
            if(fread(data,1,length,fp) != (_U32)length)
            {
                LOGE("read %s length less than length",abs_path);
            }
        }
        else
        {
            if(fread(data,1,size,fp) != (_U32)size)
            {
                LOGE("read %s length less than size",abs_path);
            }
        }
        fclose(fp);
		
        return SUCCESS;
    }
}

_S32 DATADB::save_file(_CHAR *rel_path, _CHAR *data, _S64 length)
{
    _CHAR abs_path[512];
    _CHAR* dir_path;
    FILE *m_pfile;

    memset(abs_path, 0, 512);
    //strcpy(abs_path, DATADB_PATH);
    //strcat(abs_path, "/");
    strcat(abs_path, rel_path);
    
    _CHAR* path = strdup(abs_path);
    if (_NULL == path) return FAILURE;
    dir_path = dirname(path);
    create_dir(dir_path);
    free(path);
	
    if (_NULL == data)
    {
        LOGE("DATADB file data is empty\n");
        return FAILURE;
    }
    m_pfile = fopen(abs_path, "wb");
    if (m_pfile == _NULL)
    {
        LOGE("Create DATADB create save file (%s) failed!\n", abs_path);
        return FAILURE;
    }

    fwrite(data, 1, length, m_pfile);
    fclose(m_pfile);
    sync();

    return SUCCESS;
}