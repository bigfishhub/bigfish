#include <stdio.h>
#include <stdlib.h>
#include <utils/Logger.h>
#include <include/_define.h>
#include "datadb.h"

int main(int argc, char const *argv[])
{
	DATADB fileoperate;
	_S64 fileLength = 0;
	_CHAR *pfile = _NULL;
	fileoperate.get_file_length((_CHAR*)"datatest.cpp",&fileLength);
	LOGD("datatest.cpp length : %lld\n",fileLength);
	pfile = (_CHAR*)malloc(fileLength);
	if (_NULL == pfile)
	{
		LOGE("malloc fail! length : %lld\n",fileLength);
		return -1;
	}
	fileoperate.get_file((_CHAR*)"datatest.cpp", pfile, -1);
	fileoperate.save_file((_CHAR*)"osd/tre.txt",pfile,fileLength);
    scanf("n");
    fileoperate.delete_dir((_CHAR*)"osd");

	return 0;
}