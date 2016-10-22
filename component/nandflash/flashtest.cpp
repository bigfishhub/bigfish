#define LOG_NDEBUG 0
#define LOG_TAG "FLASHTEST"

#include <stdio.h>
#include <stdlib.h>
#include "nandflash.h"

/*
** 0 擦除测试
** 1 2 3 4写入测试
 */

_S32 main(_S32 argc, char const *argv[])
{
	NANDflash *flash = NANDflash::getInstance();
	_U8 *buf;
    //_U8*rebuf;
    //_U8 *buf1;
    _U32 k;
    _U64 offset;
    _U8 *src;
    _U32 writelength;
    _U32 readlength;
    _U32 retlength;
    _S32 i;
    _S32 fd;

    fd = flash->nand_flash_init((_CHAR*)"/dev/mtd10");
    buf = (_U8 *)malloc(0x400000);
    if (buf == 0)
    {
        LOGD("malloc error\n");
        flash->nand_flash_close(fd);
        return -1;
    }
    /*rebuf = (_U8 *)malloc(0x400000);
    if (rebuf == 0)
    {
        LOGD("rebuf malloc error\n");
        flash->nand_flash_close(fd);
        return -1;
    }

    FILE *filefd = _NULL;
    _U64 rwsize = 0;

    filefd = fopen((_CHAR*)"tdi.bin","r");
    if (filefd < 0)
    {
        LOGE("filefd is null");
        return -1;
    }
    fseek(filefd,0,SEEK_END);
    rwsize = ftell(filefd);
    LOGI("tdi.bin size %d\n",rwsize);
    if (rwsize > 0x400000)
    {
        LOGE("file size too big!");
        return -1;
    }
    fseek(filefd,0,SEEK_SET);
    fread(buf,1,rwsize,filefd);

    fclose(filefd);

    flash->nand_flash_erase (fd,0, 0x400000 );
    for (k = 0; k < 0x400000; k++)
    {
        rebuf[k] = 0;
    }
    offset = 0x00000;
    readlength = 0x380000;
    flash->nand_flash_read(fd,offset,rebuf,readlength, &retlength);
    for (k = 0; k < readlength;k++)
    {
        if (rebuf[k] != 0xFF)
        {
            LOGD("error 0x%08x\n",k);
            return -1;
        }
    }
    LOGD("erace 4m data ok\n");
    offset = 0;
    src = buf;
    writelength = 0x380000;
    flash->nand_flash_write(fd,offset, src, writelength);
    offset = 0x00000;
    readlength = 0x380000;
    flash->nand_flash_read(fd,offset,rebuf,readlength, &retlength);
    LOGI("\n");
    for (_U64 dex = 0;dex < rwsize;dex++)
    {
        if(buf[dex] != rebuf[dex])
        {
            LOGE("write data error addr %d\n",dex);
            flash->nand_flash_close(fd);
            return -1;
        }
        else
        {
            if (dex%30 == 0)
                LOGI("\n");
            LOGI(" %#x",buf[dex]);
        }
    }
    LOGI("\n");
    LOGI("write ok\n");
    flash->nand_flash_close(fd);
    return 0;*/


    while (1)
    {
        scanf("%d",&i);
        if (i == 0)
        {
            for (k = 0;k <0x400000/flash->nand_get_block_size();k++)
                flash->nand_flash_block_erase (fd,k);

            LOGI("nand_flash_erase num %d ,block size %#x\n",0x400000/flash->nand_get_block_size(),flash->nand_get_block_size());
            for (k = 0; k < 0x400000; k++)
            {
                buf[k] = 0;
            }
            offset = 0x00000;
            readlength = 0x380000;
            flash->nand_flash_read(fd,offset,buf,readlength, &retlength);
            for (k = 0; k < readlength;k++)
            {
                if (buf[k] != 0xFF)
                {
                    LOGD("error 0x%08x\n",k);
                    return -1;
                }
            }
            LOGD("erace 4m data ok\n");
        }
        else if (i == 1)
        {
  
            for (k = 0; k < 0x400000; k++)
            {
                buf[k] = 0x88;
            }
            offset = 0;
            src = buf;
            writelength = 0x380000;
            flash->nand_flash_write(fd,offset, src, writelength);
            for (k = 0; k < 0x400000; k++)
            {
                buf[k] = 0;
            }
            offset = 0x00000;
            readlength = 0x380000;
            flash->nand_flash_read(fd,offset,buf,readlength, &retlength);
            for (k = 0; k < readlength;k++)
            {
                if (buf[k] != 0x88)
                {
                    LOGD("error 0x%08x\n",k);
                    return -1;
                }
            }
            LOGD("write 4M ok\n");
        }
        else if (i == 2)
        {
            for (k = 0; k < 0x400000; k++)
            {
                buf[k] = 0x55;
            }
            offset = 4;
            src = buf;
            writelength = 128*1024;
            flash->nand_flash_write(fd,offset, src, writelength);
            for (k = 0; k < 0x400000; k++)
            {
                buf[k] = 0;
            }
            offset = 0x00000;
            readlength = 128*1024+8;
            flash->nand_flash_read(fd,offset,buf,readlength, &retlength);
            for (k = 0; k < 4;k++)
            {
                if (buf[k] != 0x88)
                {
                    LOGD("error0 0x%08x\n",k);
                    break;
                }
            }
            for (k = 4; k < readlength - 4;k++)
            {
                if (buf[k] != 0x55)
                {
                    LOGD("error1 0x%08X:0x%02x 0x%02x 0x%02x 0x%02x\n",k,buf[k],buf[k+1],buf[k+2],buf[k+3]);
                    break;
                }
            }
            for (k = readlength - 4; k < readlength;k++)
            {
                if (buf[k] != 0x88)
                {
                    LOGD("error2 0x%08x\n",k);
                    break;
                }
            }
            LOGD("write 4M ok\n");
        }
        else if (i == 3)
        {
  
            for (k = 0; k < 0x20000; k++)
            {
                buf[k] = 0x11;
            }
            for (k = 0x20000; k < 0x40000; k++)
            {
                buf[k] = 0x22;
            }
	      for (k = 0x40000; k < 0x60000; k++)
            {
                buf[k] = 0x33;
            }
            offset = 0x20000;
            src = buf;
            writelength = 0x40000;
            flash->nand_flash_write(fd,offset, src, writelength);
            for (k = 0; k < 0x60000; k++)
            {
                buf[k] = 0;
            }
            offset = 0x00000;
            readlength = 0x60000;
            flash->nand_flash_read(fd,offset,buf,readlength, &retlength);

            for (k = 0x20000; k < 0x40000;k++)
            {
                if (buf[k] != 0x11)
                {
                    LOGD("error1 0x%08x\n",k);
                    return -1;
                }
            }
            for (k = 0x40000; k < 0x60000;k++)
            {
                if (buf[k] != 0x22)
                {
                    LOGD("error2 0x%08x\n",k);
                    return -1;
                }
            }
            LOGD("write 4M ok\n");
        }
        else if (i == 4)
        {
  
            for (k = 0; k < 0x20000; k++)
            {
                buf[k] = 0x11;
            }
            for (k = 0x00000; k < 0x0180; k++)
            {
                buf[k] = 0x22;
            }

            offset = 0x00000;
            src = buf;
            writelength = 0x20000;
            flash->nand_flash_write(fd,offset, src, writelength);
            for (k = 0; k < 0x20000; k++)
            {
                buf[k] = 0;
            }
            offset = 0x00000;
            readlength = 0x20000;
            flash->nand_flash_read(fd,offset,buf,readlength, &retlength);

            for (k = 0; k < 0x180;k++)
            {
                if (buf[k] != 0x22)
                {
                    LOGD("error 0x%08x\n",k);
                    return -1;
                }
            }

            for (k = 0x180; k < 0x20000;k++)
            {
                if (buf[k] != 0x11)
                {
                    LOGD("error1 0x%08x\n",k);
                    return -1;
                }
            }

            LOGD("write 4M ok\n");
        }
        else
        {
            break;
        }
    }
    flash->nand_flash_close(fd);
	return 0;
}