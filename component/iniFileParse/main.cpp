#include "utils/iniFile.h"

int main(int argc, char const *argv[])
{
	IniFile file;
	string value;
	long value2;
	int ret = 0;
	printf("ceshi main\n");
	file.open("./config.ini");

	ret = file.getStringValue("channel1","frequency",value2);
	printf("ret=%d,channel1-frequency:%ld\n",ret, value2);
	ret = file.getStringValue("channel2","audioPid",value2);
	printf("ret=%d,channel2-audioPid:%ld\n",ret, value2);

	ret = file.getStringValue("channel1","videoPid",value2);
	printf("ret=%d,channel1-videoPid:%ld\n",ret, value2);
	ret = file.getStringValue("channel2","videoPid",value2);
	printf("ret=%d,channel2-videoPid:%ld\n",ret, value2);

	file.outputValue();
	file.close();

	ret = file.open("./appcfg.ini");

	ret = file.getStringValue("Launcher","path",value);
	printf("ret=%d,Launcher-path:%s\n",ret, value.c_str());
	ret = file.getStringValue("Launcher","argv",value);
	printf("ret=%d,Launcher-argv:%s\n",ret, value.c_str());

	ret = file.getStringValue("Browser_Server","exclusive",value);
	printf("ret=%d,Browser_Server-exclusive:%s\n",ret, value.c_str());
	ret = file.getStringValue("Browser_Server","iconpath",value);
	printf("ret=%d,Browser_Server-iconpath:%s\n",ret, value.c_str());

	file.outputValue();
	file.close();
	return 0;
}