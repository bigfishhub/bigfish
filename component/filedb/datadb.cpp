#define LOG_TAG "DATADB"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
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

//-------------------------------------------------------------------

#define BUFFER_SIZE 1024  
/* 
 * 判断是否是目录 
 * @ 是目录返回1，是普通文件返回0，出错返回-1 
 * */  
_S32 DATADB::IsDir(const _CHAR *path)  
{  
    struct stat buf;  
    if (stat(path, &buf)==-1)  
    {  
        LOGD("stat :%s",strerror(errno));  
        LOGD("path = %s\n", path);  
        return -1;  
    }  
    return S_ISDIR(buf.st_mode);  
} 

/* 
 * 创建目录 
 * @ 可以创建多级目录，失败返回-1 
 * */  
_S32 DATADB::CreateDir(const _CHAR *path)  
{  
    _CHAR pathname[256];  
    strcpy(pathname, path);  
    _S32 i, len = strlen(pathname);  
    if (pathname[len-1]!='/')  
    {  
        strcat(pathname, "/");  
        len++;  
    }  
    for (i=0; i<len; i++)  
    {  
        if (pathname[i]=='/')  
        {  
            if (i == 0)
                continue;
            pathname[i]=0;  
            if (access(pathname, F_OK)) //判断路径是否存在  
            {   //不存在则创建  
                if (mkdir(pathname, 0755)==-1)  
                {  
                    LOGD("i:%d mkdir \"%s\":%s\n",i,pathname,strerror(errno));  
                    return -1;  
                }  
            }  
            pathname[i]='/';  
        }  
    }  
    return 0;  
}

/* 
 * 拷贝文件 
 * @ @dstpath -- 可以是文件名也可以是目录名 
 * */  
_S32 DATADB::FileCopy(const _CHAR *srcpath, const _CHAR *dstpath)  
{  
    _S32 srcfd, dstfd, file_len, ret=1;  
    _CHAR buffer[BUFFER_SIZE];  
    _CHAR dstfn[256];  
    if (access(srcpath, R_OK))  
    {  
        LOGD("Cannot copy does not exist or unreadable files: %s\n", srcpath);  
        return -1;  
    }  
    strcpy(dstfn, dstpath);  
    //如果@dstpath存在且是目录则在其后加上srcpath的文件名  
    if (access(dstpath, F_OK) == 0 && IsDir(dstpath) == 1)  
    {  
        if (dstfn[strlen(dstfn)-1]!='/')  
        {  
            strcat(dstfn, "/");  
        }  
        if (strchr(srcpath, '/'))  
        {  
            strcat(dstfn, strrchr(srcpath, '/'));  
        }  
        else  
        {  
            strcat(dstfn, srcpath);  
        }  
    }  
    srcfd = open(srcpath, O_RDONLY);  
    struct stat stabuff;
    if (0 == fstat(srcfd,&stabuff))
    {
        dstfd = open(dstfn, O_WRONLY|O_CREAT,stabuff.st_mode); 
    }
    else
    {
        dstfd = open(dstfn, O_WRONLY|O_CREAT,0755);  
    }    
    if (srcfd == -1 || dstfd == -1)  
    {  
        if (srcfd!=-1)  
        {  
            close(srcfd);  
        }  
        LOGD("open \"%s\":%s!\n",dstfn,strerror(errno));  
        return -1;  
    }  
/*    {
        struct stat stabuff;
        if (0 == fstat(srcfd,&stabuff))
        {
            int flag = 0;
            if ((flag = fcntl(dstfd,F_GETFL,0)) != -1)
            {
                flag |= stabuff.st_mode;
                if (fcntl(dstfd,F_SETFL,flag) < 0)
                {
                    HI_LOGD("set fcntl error!");
                }
            }
        }
    }*/
      
    file_len = lseek(srcfd, 0L, SEEK_END);  
    lseek(srcfd, 0L, SEEK_SET);  
    while(ret)  
    {  
        ret = read(srcfd, buffer, BUFFER_SIZE);  
        if (ret==-1)  
        {  
            LOGD("read fail!:%s\n",strerror(errno));  
            close(srcfd);  
            close(dstfd);  
            return -1;  
        }  
        if(-1 == write(dstfd, buffer, ret))
        {
            LOGD("write fail!:%s\n",strerror(errno));  
            close(srcfd);  
            close(dstfd);  
            return -1; 
        }  
        file_len -= ret;  
        bzero(buffer, BUFFER_SIZE);  
    }  
    close(srcfd);  
    close(dstfd);  
    if (ret)  
    {  
        LOGD("文件: %s, 没有拷贝完!\n", srcpath);  
        return -1;  
    }  
    return 0;  
}

/* 
 * 目录拷贝 
 * */  
_S32 DATADB::DirCopy(const _CHAR *srcpath, const _CHAR *dstpath)  
{  
    _S32 ret;  
    DIR * dir;  
    struct dirent *ptr;  
    _CHAR frompath[256];
    _CHAR topath[256];  
    if (!(ret = IsDir(srcpath))) //如果@srcpath 是文件，直接进行文件拷贝  
    {  
        FileCopy(srcpath, dstpath);  
        return 0;  
    }  
    else if(ret!=1)  //目录或文件不存在  
    {  
        return -1;  
    }  
    dir = opendir(srcpath);  
    CreateDir(dstpath);  
    while((ptr=readdir(dir))!=NULL)  
    {  
        bzero(frompath, 256);  
        bzero(topath, 256);  
        strcpy(frompath, srcpath);  
        strcpy(topath, dstpath);  
        if (frompath[strlen(frompath)-1]!='/')  
        {  
            strcat(frompath, "/");  
        }  
        if (topath[strlen(topath)-1]!='/')  
        {  
            strcat(topath, "/");  
        }  
        strcat(frompath, ptr->d_name);  
        //HI_LOGD("%s\n", frompath);  
        strcat(topath, ptr->d_name);  
        if ((ret=IsDir(frompath))==1)  
        {  
            if (strcmp(strrchr(frompath, '/'), "/.")==0  
                || strcmp(strrchr(frompath, '/'), "/..")==0)  
            {  
                //HI_LOGD(". or ..目录不用复制\n");  
            }  
            else  
            {  
                DirCopy(frompath, topath);  
            }  
        }  
        else if (ret!=-1)  
        {  
            FileCopy(frompath, topath);  
        }  
    }  
    closedir(dir);  
    return 0;  
}  

/* 
 * 如果  oldFile 比 newFile 修改时间新，返回1，否则返回0
 * 打开文件错误返回 -1;
 * */ 
_S32 DATADB::compareFileAmendTime(const _CHAR* oldFile ,const _CHAR *newFile)
{
    struct timespec oldFileTime;
    struct timespec newFileTime;
    struct stat stabuff;

    if(0 != stat(oldFile,&stabuff))
    {
        LOGE("[compareFileAmendTime] stat \"%s\" fail!%s\n",oldFile,strerror(errno));
        return -1;
    }
    oldFileTime = stabuff.st_mtim;
    if(0 != stat(newFile,&stabuff))
    {
        LOGE("[compareFileAmendTime] stat \"%s\" fail!%s\n",oldFile,strerror(errno));
        return -1;
    }
    newFileTime = stabuff.st_mtim;

    LOGD("[compareFileAmendTime] \"%s\" sec %u nsec %u\n",oldFile,(int32_t)oldFileTime.tv_sec,(int32_t)oldFileTime.tv_nsec);
    LOGD("[compareFileAmendTime] \"%s\" sec %u nsec %u\n",newFile,(int32_t)newFileTime.tv_sec,(int32_t)newFileTime.tv_nsec);
    if (oldFileTime.tv_sec < newFileTime.tv_sec)
    {
        return 0;
    }
    else if (oldFileTime.tv_sec == newFileTime.tv_sec)
    {
        if (oldFileTime.tv_nsec < newFileTime.tv_sec)
        {
            return 0;
        }
    }

    return 1;
}

_S32 DATADB::compareFileSize(const _CHAR* bigSizeFile ,const _CHAR *smallSizeFile)
{
    off_t bigFileSize;
    off_t smallFileSize;
    struct stat stabuff;

    if(0 != stat(bigSizeFile,&stabuff))
    {
        LOGD("[compareFileSize] stat \"%s\" fail!%s\n",bigSizeFile,strerror(errno));
        return -1;
    }
    bigFileSize = stabuff.st_size;
    if(0 != stat(smallSizeFile,&stabuff))
    {
        LOGD("[compareFileSize] stat \"%s\" fail!%s\n",smallSizeFile,strerror(errno));
        return -1;
    }
    smallFileSize = stabuff.st_size;

    LOGD("[compareFileSize] \"%s\" off_t %u\n",bigSizeFile,(int32_t)bigFileSize);
    LOGD("[compareFileSize] \"%s\" off_t %u\n",smallSizeFile,(int32_t)smallFileSize);
    if (bigFileSize >= smallFileSize)
    {
        return 0;
    }

    return 1;
}

