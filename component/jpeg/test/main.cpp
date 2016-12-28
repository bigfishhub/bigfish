#include <stdlib.h>
#include "string.h"
#include "jpegmanage.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
int unused __attribute__((unused));

void helpshow()
{
    printf("\n"
"********************************************************************************\n"
"* 1.change the size,jpg->jpg          .eg: j2js from.jpg to.jpg 1920 1080      *\n"
"* 2.change the coolor space jpg->rgb  .eg: j2rgb from.jpg to.rgb               *\n"
"* 3.change the coolor space rgba->rgb .eg: rgba2rgb from.rgba to.rgb 1920 1080 *\n"
"* 4.change the coolor space bgra->rgb .eg: bgra2rgb from.bgra to.rgb 1920 1080 *\n"
"* 5.change the coolor space rgb->jpg  .eg: rgb2j from.rgb to.jpg 1920 1080     *\n"
"* 6.change the coolor space rgb->bmp  .eg: rgb2b from.rgb to.bmp 1920 1080     *\n"
"********************************************************************************\n");
}

int main(int argc, char const *argv[])
{
	unsigned char *rgb_buffer = NULL,*torgb_buffer = NULL;
	int size = 0,width = 0,height = 0;
    int ret = 0;
	JPEGMANAGE jpegmge;
    BMPMAP_FILE bmpmge;
    if (argc < 4)
    {
        LOGE("parameter too less!\n");
        helpshow();
        return FAILURE;
    }

    if (argc == 6 && !strcmp(argv[1],"j2js"))
    {
        jpegmge.read_jpeg_file(argv[2], &rgb_buffer, &size, &width, &height);
        jpegmge.magnifyImage(rgb_buffer,width,height, &torgb_buffer, atoi(argv[4]), atoi(argv[5]));
        jpegmge.write_jpeg_file(argv[3],torgb_buffer,atoi(argv[4]),atoi(argv[5]),90);
        free(rgb_buffer);
        free(torgb_buffer);
    }
    else if (argc == 4 && !strcmp(argv[1],"j2rgb"))
    {
        jpegmge.read_jpeg_file(argv[2], &rgb_buffer, &size, &width, &height);
        FILE *fp = fopen(argv[3], "wb");  
        if (fp == NULL)  
        {  
            LOGE("open file %s failed.\n", argv[2]);  
            return -1;  
        } 
        fwrite(rgb_buffer,1,size,fp);
        fclose(fp);
        free(rgb_buffer);
    }
    else if (argc == 6 && (!strcmp(argv[1],"rgba2rgb") || !strcmp(argv[1],"bgra2rgb")))
    {
        struct stat statConf;
        _U32 length =0;
        int fp = open(argv[2],O_RDONLY);
        if (fp == -1)
        {  
            LOGE("open file %s failed.\n", argv[2]);  
            return -1;  
        } 
        ret = fstat(fp,&statConf);
        if (ret == -1)
        {  
            close(fp);
            LOGE("fstat file %s failed.\n", argv[2]);  
            return -1;  
        } 
        length = statConf.st_size;
        LOGI("stat.st_size:%ld\n",statConf.st_size);
        rgb_buffer = (_U8*)malloc(statConf.st_size);
        if (NULL == rgb_buffer)
        {
            LOGE("malloc fail!");
            close(fp);
            return -1;
        }
        memset(rgb_buffer,0x00,statConf.st_size);
        unused = read(fp,rgb_buffer,statConf.st_size);
        close(fp);
        if (!strcmp(argv[1],"bgra2rgb"))
        {
            BMPMAP_FILE::colorSpace2RGB(&rgb_buffer, &length,MAP_BGRA32);
        }
        else
        {
            BMPMAP_FILE::colorSpace2RGB(&rgb_buffer, &length,MAP_RGBA32);
        }
        fp = open(argv[3],O_WRONLY|O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
        if (fp == -1)
        {  
            LOGE("open file %s failed.\n", argv[3]);  
            return -1;  
        } 
        unused = write(fp,rgb_buffer,length);
        close(fp);
        free(rgb_buffer);
    }
    else if (argc == 6 && (!strcmp(argv[1],"rgb2j") || !strcmp(argv[1],"rgb2b")))
    {
        struct stat statConf;
        _U32 length =0;
        int fp = open(argv[2],O_RDONLY);
        if (fp == -1)
        {  
            LOGE("open file %s failed.\n", argv[2]);  
            return -1;  
        } 
        ret = fstat(fp,&statConf);
        if (ret == -1)
        {  
            close(fp);
            LOGE("fstat file %s failed.\n", argv[2]);  
            return -1;  
        } 
        length = statConf.st_size;
        LOGI("stat.st_size:%ld",statConf.st_size);
        rgb_buffer = (_U8*)malloc(statConf.st_size);
        if (NULL == rgb_buffer)
        {
            LOGE("malloc fail!");
            close(fp);
            return -1;
        }
        memset(rgb_buffer,0x00,statConf.st_size);
        unused = read(fp,rgb_buffer,statConf.st_size);
        close(fp);
        if (!strcmp(argv[1],"rgb2j"))
        {
            jpegmge.write_jpeg_file(argv[3],rgb_buffer,atoi(argv[4]),atoi(argv[5]),90);
        }
        else
        {
            bmpmge.fillDefaultBitMapInfoHeader();
            bmpmge.setBitMapSize(atoi(argv[4]),atoi(argv[5]));
            bmpmge.fillBitMapBuffer(rgb_buffer,length);
            bmpmge.createBmpMap(argv[3]);
        }
    }
    else
    {
        helpshow();
    }
	return SUCCESS;
}