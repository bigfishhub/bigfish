#include <stdio.h>
#include <stdlib.h>
#include <utils/Logger.h>
#include <common/_define.h>
#include "datadb.h"

int unused __attribute__((unused));

int main(int argc, char const *argv[])
{
	_S64 fileLength = 0;
	_CHAR *pfile = _NULL;
	DATADB::get_file_length((_CHAR*)"datatest.cpp",&fileLength);
	LOGD("datatest.cpp length : %d\n",(int32_t)fileLength);
	pfile = (_CHAR*)malloc(fileLength);
	if (_NULL == pfile)
	{
		LOGE("malloc fail! length : %d\n",(int32_t)fileLength);
		return -1;
	}
	DATADB::get_file((_CHAR*)"datatest.cpp", pfile, -1);
	DATADB::save_file((_CHAR*)"osd/tre.txt",pfile,fileLength);
    unused = scanf("n");
    DATADB::delete_dir((_CHAR*)"osd");

	return 0;
}