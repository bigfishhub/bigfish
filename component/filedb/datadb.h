#ifndef _DATADB_H
#define _DATADB_H

#include <common/_define.h>

class DATADB
{
public:
    
    virtual ~DATADB();
		
    static _S32 save_file(_CHAR *rel_path, _CHAR *data, _S64 length);

    static _S32 get_file_length(_CHAR *rel_path, _S64 *length);

    static _S32 get_file(_CHAR *rel_path, _CHAR *data, _S64 length);

    static _S32 delete_file(const _CHAR *rel_path);

    static _S32 delete_dir(const _CHAR* dirname);

    static _S32 IsDir(const _CHAR *path);

    static _S32 CreateDir(const _CHAR *path);

    static _S32 FileCopy(const _CHAR *srcpath, const _CHAR *dstpath);

    static _S32 DirCopy(const _CHAR *srcpath, const _CHAR *dstpath);

    static _S32 compareFileAmendTime(const _CHAR* oldFile ,const _CHAR *newFile);

    static _S32 compareFileSize(const _CHAR* bigSizeFile ,const _CHAR *smallSizeFile);

private:
    DATADB();

    static _S32 create_dir(const char *path);
    static _S32 _delete_dir(const _CHAR* dirname);
    
};

#endif 
