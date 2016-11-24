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

typedef struct bitMapFileHeader_s  //14���ֽ�
{
	_U16 bfType;                 //��ʶΪbmp���ͣ�ֵ�̶�Ϊ0x4d42.
	_U32 bfSize;				 //��ʶΪ��bmpͼƬ��С
	_U16 bfReserved1;            //�����ֽ�
	_U16 bfReserved2;            //�����ֽ�
	_U32 bfOffBits;              //��ʾDIB��������bmp�ļ��е�λ��ƫ��,Ĭ��14+40=54
}bitMapFileHeader;

typedef struct bitMapInfoHeader_s
{
	_U32 biSize;                 //˵��bitMapInfoHeader_s�ṹ����Ҫ���ֽ���,40�ֽ�
	_S32 biWidth;                //˵��ͼ���ȣ�������Ϊ��λ
	_S32 biHeight;               // ˵��ͼ��߶ȣ����Ϊ��ֵ������ͼ���ǵ����ģ�
									//����Ǹ�ֵ����ͼ����������
	_U16 biPlanes;				 //����bmpͼ���ƽ�����ԣ������1
	_U16 biBitCount;            //˵��������/���أ���ֵΪ1��4��8��16��24��32
	_U32 biCompression;         //˵�����ݵ�ѹ�����ͣ�BI_RGB/BI_RLE8/BI_REL4/BI_BITFIELDS/BI_JPED
	_U32 biSizeImage;            //˵��ͼ���С���ֽ�Ϊ��λ��BI_RGB��ʽʱ��������Ϊ0��
	_U32 biXPelsPerMeter;        //˵��ˮƽ�ֱ��ʣ�����/�ױ�ʾ
	_U32 biYPelsPerMeter;        //˵����ֱ�ֱ��ʣ�����/�ױ�ʾ
	_U32 biClrUsed;              //ʵ��ʹ�õĵ�ɫ����������0��ʹ�����еĵ�ɫ������
	_U32 biClrImportant;         //˵����ͼ����ʾ����ҪӰ�����ɫ��������Ŀ�����Ϊ0����ʾ����Ҫ
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

	_S32 fillDefaultBitMapInfoHeader(); //���Ĭ������
	_S32 setBitMapSize(_S32 biWidth, _S32 biHeight);  //��������bmpͼƬ�Ĵ�С
	_S32 fillBitMapBuffer(_U8 *rbgData, _U32 rgbDataLength, MAP_COLOR_SPACE colorType = MAP_RGB24);  //���bmp buffer

	_S32 createBmpMap(const _CHAR* bmpPath);             //����bmpͼƬ

public:
	static _S32 colorSpace2RGB(_U8** colorSpace, _U32* spaceLength,const MAP_COLOR_SPACE SourceColorType);
private:
	_S32 fillBitMapFileHeader(_U32 bfSize, _U32 bfOffBits);   //�������ͷ
	_S32 setBiSizeImage(_U32 biSizeImage);   //����ͼƬ��С�ֽ���
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