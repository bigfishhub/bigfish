#ifndef __JPEGMANAGE_H__
#define __JPEGMANAGE_H__

#include <stdio.h>
#include "common/_define.h"
#include "utils/Logger.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "jpeglib.h"  
#include "jerror.h" 

#ifdef __cplusplus
}
#endif

#define BMPTYPE 0x4d42

typedef enum {
	MAP_UNKNOWN,		/* error/unspecified */
	MAP_RGB24,		/* red/green/blue, standard RGB (sRGB) */
	MAP_BGR24,		/* blue/green/read, standard BGR (sBGR) */
	MAP_RGBA32,		/* red/green/blue/Alpha, standard RGBA (sRGBA) */
	MAP_BGRA32		/* blue/green/read/Alpha, standard BGRA (sBGRA) */
} MAP_COLOR_SPACE;

typedef struct bitMapFileHeader_s  //14个字节
{
	_U16 bfType;                 //标识为bmp类型，值固定为0x4d42.
	_U32 bfSize;				 //标识为该bmp图片大小
	_U16 bfReserved1;            //保留字节
	_U16 bfReserved2;            //保留字节
	_U32 bfOffBits;              //表示DIB数据区在bmp文件中的位置偏移,默认14+40=54
}bitMapFileHeader;

typedef struct bitMapInfoHeader_s
{
	_U32 biSize;                 //说明bitMapInfoHeader_s结构所需要的字节数,40字节
	_S32 biWidth;                //说明图像宽度，以像素为单位
	_S32 biHeight;               // 说明图像高度，如果为正值，表明图像是倒立的，
									//如果是负值表明图像是正立的
	_U16 biPlanes;				 //表明bmp图像的平面属性，恒等于1
	_U16 biBitCount;            //说明比特数/像素，其值为1、4、8、16、24、32
	_U32 biCompression;         //说明数据的压缩类型，BI_RGB/BI_RLE8/BI_REL4/BI_BITFIELDS/BI_JPED
	_U32 biSizeImage;            //说明图像大小，字节为单位。BI_RGB格式时，可设置为0。
	_U32 biXPelsPerMeter;        //说明水平分辨率，像素/米表示
	_U32 biYPelsPerMeter;        //说明垂直分辨率，像素/米表示
	_U32 biClrUsed;              //实际使用的调色板索引数，0：使用所有的调色板索引
	_U32 biClrImportant;         //说明对图像显示有重要影响的颜色索引的数目，如果为0，表示都重要
}bitMapInfoHeader;

typedef struct paletteEntry_s
{
	_U8 peRed;
	_U8 peGreen;
	_U8 peBlue;
	_U8 peFlags;
}paletteEntry;

class BMPMAP_FILE
{
public:
	BMPMAP_FILE();
	~BMPMAP_FILE();

	_S32 fillDefaultBitMapInfoHeader(); //填充默认数据
	_S32 setBitMapSize(_S32 biWidth, _S32 biHeight);  //设置生成bmp图片的大小
	_S32 fillBitMapBuffer(_U8 *rbgData, _U32 rgbDataLength, MAP_COLOR_SPACE colorType = MAP_RGB24);  //填充bmp buffer

	_S32 createBmpMap(const _CHAR* bmpPath);             //生成bmp图片

public:
	static _S32 colorSpace2RGB(_U8** colorSpace, _U32* spaceLength,const MAP_COLOR_SPACE SourceColorType);
private:
	_S32 fillBitMapFileHeader(_U32 bfSize, _U32 bfOffBits);   //填充数据头
	_S32 setBiSizeImage(_U32 biSizeImage);   //设置图片大小字节数
private:
	bitMapFileHeader bitmapheader;
	bitMapInfoHeader bitmapinfoheader;
	paletteEntry     plaette[256];
	_U8              *bmpBuffer;
};

class JPEGMANAGE
{
public:
	JPEGMANAGE();
	~JPEGMANAGE();
	/**
	 * [read_jpeg_file description]
	 * @param  jpeg_file  [description]
	 * @param  rgb_buffer [need user release]
	 * @param  size       [description]
	 * @param  width      [description]
	 * @param  height     [description]
	 * @return            [description]
	 */
	_S32 read_jpeg_file(const _CHAR* jpeg_file, _U8** rgb_buffer, _S32* size, _S32* width, _S32* height);
	/**
	 * [write_jpeg_file description]
	 * @param  jpeg_file  [description]
	 * @param  rgb_buffer [description]
	 * @param  width      [description]
	 * @param  height     [description]
	 * @param  quality    [description]
	 * @return            [description]
	 */
	_S32 write_jpeg_file(const _CHAR* jpeg_file, _U8* rgb_buffer, _S32 width, _S32 height, _S32 quality);
	/**
	 * [magnifyImage description]
	 * @param  rgb_buffer   [description]
	 * @param  width        [description]
	 * @param  height       [description]
	 * @param  torgb_buffer [need user release]
	 * @param  towidth      [description]
	 * @param  toheight     [description]
	 * @return              [description]
	 */
	_S32 magnifyImage(_U8* rgb_buffer, _S32 width, _S32 height, _U8** torgb_buffer, _S32 towidth, _S32 toheight);

private:

};
#endif