#ifndef _DATADB_H
#define _DATADB_H

#include <include/_define.h>

class DATADB
{
public:
    DATADB();
    virtual ~DATADB();
		
    _S32 save_file(_CHAR *rel_path, _CHAR *data, _S64 length);

     _S32 get_file_length(_CHAR *rel_path, _S64 *length);

    _S32 get_file(_CHAR *rel_path, _CHAR *data, _S64 length);

    _S32 delete_file(const _CHAR *rel_path);

    _S32 delete_dir(const _CHAR* dirname);

private:
    static _S32 create_dir(const char *path);
    _S32 _delete_dir(const _CHAR* dirname);
    
};

#endif 
