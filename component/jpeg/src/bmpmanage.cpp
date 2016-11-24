#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jpegmanage.h"

BMPMAP_FILE::BMPMAP_FILE()
{
    bmpBuffer = NULL;
}

BMPMAP_FILE::~BMPMAP_FILE()
{
    if (NULL != bmpBuffer)
        free(bmpBuffer);
}

_S32 BMPMAP_FILE::fillBitMapFileHeader(_U32 bfSize, _U32 bfOffBits)   //填充数据头
{
    bitmapheader.bfType      = BMPTYPE;
    bitmapheader.bfSize      = bfSize;
    bitmapheader.bfReserved1 = 0x00;
    bitmapheader.bfReserved2 = 0x00;
    bitmapheader.bfOffBits   = bfOffBits;

    return SUCCESS;
}

_S32 BMPMAP_FILE::fillDefaultBitMapInfoHeader() //填充默认数据
{
    bitmapinfoheader.biSize = 0x28; //40字节
    bitmapinfoheader.biWidth = 1280;
    bitmapinfoheader.biHeight = 720; //表明图片数据是倒立的
    bitmapinfoheader.biPlanes = 1;
    bitmapinfoheader.biBitCount = 24;
    bitmapinfoheader.biCompression = 0x00;
    bitmapinfoheader.biSizeImage = 0;
    bitmapinfoheader.biXPelsPerMeter = 0x0;
    bitmapinfoheader.biYPelsPerMeter = 0x0; //每米多少像素
    bitmapinfoheader.biClrUsed = 0x0; //每米多少像素
    bitmapinfoheader.biClrImportant = 0x00;

    return SUCCESS;
}

_S32 BMPMAP_FILE::setBitMapSize(_S32 biWidth, _S32 biHeight)  //设置生成bmp图片的大小
{
    bitmapinfoheader.biWidth = biWidth;
    bitmapinfoheader.biHeight = -biHeight;

    return SUCCESS;
}

_S32 BMPMAP_FILE::colorSpace2RGB(_U8** colorSpace, _U32* spaceLength,const MAP_COLOR_SPACE SourceColorType)
{
    _U32 dex = 0, alphaNum = 0;
    _U32 length = *spaceLength;
    _U8* toSpace = NULL,*targetSpace = NULL;
    _U8  midColorData = 0;

    if (colorSpace == NULL || spaceLength == NULL)
    {
        LOGE("[colorSpace2RGB] parameter error!\n");
        return FAILURE;
    }

    if (MAP_BGR24 == SourceColorType)
    {
        length = length/3*3;
        for (dex = 0;dex < length; dex+=3)
        {
            midColorData    = (*colorSpace)[dex];
            (*colorSpace)[dex]   = (*colorSpace)[dex+2];
            (*colorSpace)[dex+2] = midColorData;
        }
        *spaceLength = length;
    }
    else if (MAP_RGB24 == SourceColorType)
    {

    }
    else if (MAP_RGBA32 == SourceColorType)
    {
        alphaNum = 0;
        length = length/4*4;
        targetSpace = (_U8*)malloc(length/4*3);
        if (NULL == targetSpace)
        {
            LOGE("[colorSpace2RGB] malloc fail!");
            return FAILURE;
        }
        toSpace = targetSpace;
        for (dex = 0;dex < length; dex++)
        {
            if (dex%4 == 3)
            {
                alphaNum++;
                continue;
            }
            else
            {
                *toSpace++ = (*colorSpace)[dex];
            }
        }
        if (NULL != *colorSpace)
            free(*colorSpace);
        *colorSpace = targetSpace;
        *spaceLength = length - alphaNum;
    }
    else if (MAP_BGRA32 == SourceColorType)
    {
        alphaNum = 0;
        length = length/4*4;
        targetSpace = (_U8*)malloc(length/4*3);
        if (NULL == targetSpace)
        {
            LOGE("[colorSpace2RGB] malloc fail!");
            return FAILURE;
        }
        toSpace = targetSpace;
        for (dex = 0;dex < length; dex++)
        {
            if (dex%4 == 3)
            {
                alphaNum++;
                continue;
            }
            else if (dex%4 == 0)
            {
                *toSpace++ = (*colorSpace)[dex+2];
                continue;
            }
            else if (dex%4 == 2)
            {
                *toSpace++ = (*colorSpace)[dex-2];
                continue;
            }
            else if (dex%4 == 1)
            {
                *toSpace++ = (*colorSpace)[dex];
                continue;
            }
        }
        if (NULL != *colorSpace)
            free(*colorSpace);
        *colorSpace = targetSpace;
        *spaceLength = length - alphaNum;
    }
    else
    {
        LOGE("[colorSpace2RGB] %d MAP_COLOR_SPACE not support!\n",SourceColorType);
        return FAILURE;
    }
    return SUCCESS;

}

_S32 BMPMAP_FILE::fillBitMapBuffer(_U8 *rbgData, _U32 rgbDataLength, MAP_COLOR_SPACE colorType)  //填充bmp buffer
{
    _U8* pData = NULL;
    _U32 length = 0,ret = 0;

    pData = (_U8*)malloc(rgbDataLength * sizeof(_U8));
    if (NULL == pData)
    {
        LOGE("[fillBitMapBuffer] malloc fail!\n");
        return FAILURE;
    }

    memcpy((_CHAR*)pData,(_CHAR*)rbgData,rgbDataLength);

    ret = BMPMAP_FILE::colorSpace2RGB(&pData, &rgbDataLength, colorType);
    if (SUCCESS != ret)
    {
        free(pData);
        LOGE("[fillBitMapBuffer] colorSpace2RGB fail!");
        return FAILURE;
    }
    ret = BMPMAP_FILE::colorSpace2RGB(&pData, &rgbDataLength, MAP_BGR24);
    if (SUCCESS != ret)
    {
        free(pData);
        LOGE("[fillBitMapBuffer] colorSpace2RGB fail!");
        return FAILURE;
    }

    if (NULL != bmpBuffer)
        free(bmpBuffer);
    bmpBuffer = pData;
    bitmapinfoheader.biSizeImage = length;

    return SUCCESS;
}

_S32 BMPMAP_FILE::setBiSizeImage(_U32 biSizeImage)   //设置图片大小字节数
{
    bitmapinfoheader.biSizeImage = biSizeImage;
    return SUCCESS;
}

_S32 BMPMAP_FILE::createBmpMap(const _CHAR* bmpPath)             //生成bmp图片
{
    _U8* pData = NULL;
    _U8* bmpbuf = bmpBuffer;
    _U32 imageSize = 0,allSize = 0,bmpOffBits = 0;
    _U32 bmpIndex = 0,DataSizePerLine = 0;
    _S32 width = 0,height = 0;
    FILE* fp = NULL;  

    width = bitmapinfoheader.biWidth;

    if (bitmapinfoheader.biHeight < 0)
    {
        height = -bitmapinfoheader.biHeight;
        printf("height:%d\n", bitmapinfoheader.biHeight);
    }
    else
    {
        height = bitmapinfoheader.biHeight;
    }

    DataSizePerLine = (width * bitmapinfoheader.biBitCount+31)/8;
    DataSizePerLine = DataSizePerLine/4*4; // 字节数必须是4的倍数
    imageSize = DataSizePerLine * height;

    setBiSizeImage(imageSize);
    pData = (_U8*)malloc((sizeof(bitMapFileHeader) + sizeof(bitMapInfoHeader) + \
                        +  sizeof(paletteEntry) + imageSize) * sizeof(_U8));
    if (NULL == pData)
    {
        LOGE("[createBmpMap] malloc fail!\n");
        return FAILURE;
    }
    memset(pData,0x00,((sizeof(bitMapFileHeader) + sizeof(bitMapInfoHeader) + \
                        +  sizeof(paletteEntry) + imageSize) * sizeof(_U8)));

    bmpIndex = 0+sizeof(bitMapFileHeader)-sizeof(_U16) + sizeof(bitMapInfoHeader);
    //
    LOGI("[createBmpMap] bitmapbuffer size:%ld,bitMapInfoHeader:%ld\n",sizeof(bitMapFileHeader),sizeof(bitMapInfoHeader));
    LOGI("[createBmpMap] bitmapbuffer addr:%u\n",bmpIndex);
    bmpOffBits = bmpIndex;
    bmpbuf = bmpBuffer;
    LOGI("[createBmpMap] width:%#x,DataSizePerLine:%#x\n",width,DataSizePerLine);
    for (_S32 i = 0;i < height;i++)
    {
        if (DataSizePerLine == (_U32)width*3)
        {
            memcpy(&pData[bmpIndex],&bmpbuf[i*DataSizePerLine],DataSizePerLine);
        }
        else if(DataSizePerLine > (_U32)width*3)
        {
            memcpy(&pData[bmpIndex],&bmpbuf[i*width*3],width*3);
            memset(&pData[(bmpIndex + width*3)],0x00,DataSizePerLine - width*3);
        }
        else
        {
            goto outerror;
        }
        bmpIndex += DataSizePerLine;
    }

    allSize = bmpIndex;   //bmp图片总大小
    fillBitMapFileHeader(allSize, bmpOffBits);   //填充数据头

    LOGI("[createBmpMap] allSize:%#x,bmpOffBits:%#x,DataSizePerLine:%#x\n",
        allSize,bmpOffBits,DataSizePerLine);
    bmpIndex = 0;
    memcpy(&pData[bmpIndex],&bitmapheader.bfType,sizeof(_U16));
    bmpIndex = 2;
    memcpy(&pData[bmpIndex],&bitmapheader.bfSize,sizeof(_U32));
    bmpIndex = 6;
    memcpy(&pData[bmpIndex],&bitmapheader.bfReserved1,sizeof(_U16));
    bmpIndex = 8;
    memcpy(&pData[bmpIndex],&bitmapheader.bfReserved2,sizeof(_U16));
    bmpIndex = 10;
    memcpy(&pData[bmpIndex],&bitmapheader.bfOffBits,sizeof(_U32));

    bmpIndex = 0+sizeof(bitMapFileHeader)-sizeof(_U16);
    memcpy(&pData[bmpIndex],&bitmapinfoheader,sizeof(bitMapInfoHeader));
    LOGI("[createBmpMap] bitmapinfoheader addr:%u\n",bmpIndex);

    fp = fopen(bmpPath, "wb");  
    if (fp == NULL)  
    {  
        free(pData);
        LOGE("[createBmpMap] open file %s failed.\n", bmpPath);  
        return -1;  
    } 
    if (allSize != fwrite(pData, 1, allSize,fp))
    {
        free(pData);
        fclose(fp);
        LOGE("[createBmpMap] fwrite file %s failed.write size:%u\n", bmpPath,allSize);  
        return -1; 
    }
    fclose(fp);
    free(pData);
    return SUCCESS;

outerror:
    free(pData);
    return FAILURE;
}