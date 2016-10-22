#include <stdlib.h>
#include "string.h"
#include "jpegmanage.h"

int main(int argc, char const *argv[])
{
	unsigned char *rgb_buffer = NULL,*torgb_buffer = NULL;
	int size = 0,width = 0,height = 0;
	JPEGMANAGE jpegmge;
	if (argc < 5)
	{
		LOGE("parameter too less!\n");
		return FAILURE;
	}
	jpegmge.read_jpeg_file(argv[1], &rgb_buffer, &size, &width, &height);

	jpegmge.magnifyImage(rgb_buffer,width,height, &torgb_buffer, atoi(argv[3]), atoi(argv[4]));

	jpegmge.write_jpeg_file(argv[2],torgb_buffer,atoi(argv[3]),atoi(argv[4]),90);
	free(rgb_buffer);
	free(torgb_buffer);
	return SUCCESS;
}