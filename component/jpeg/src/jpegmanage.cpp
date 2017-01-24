#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <setjmp.h>  
#include <math.h>  
#include <sys/time.h>  
#include <time.h>  
#include "jpegmanage.h"

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}
  
JPEGMANAGE::JPEGMANAGE()
{

}

JPEGMANAGE::~JPEGMANAGE()
{

}

// 读取JPG图片数据，并解压到内存中，*rgb_buffer需要自行释放  
_S32 JPEGMANAGE::read_jpeg_file(const _CHAR* jpeg_file, _U8** rgb_buffer, _S32* size, _S32* width, _S32* height)  
{  
    struct jpeg_decompress_struct cinfo;    
    FILE* fp;  
  
    JSAMPARRAY buffer;  
    _S32 row_stride = 0;  
    _U8* tmp_buffer = NULL;  
    _S32 rgb_size;  
  
    fp = fopen(jpeg_file, "rb");  
    if (fp == NULL)  
    {  
        LOGE("open file %s failed.\n", jpeg_file);  
        return FAILURE;  
    }  
  	//struct jpeg_error_mgr jerr;
  	struct my_error_mgr jerr;

	//将错误处理结构对象绑定在JPEG对象上
	cinfo.err = jpeg_std_error(&jerr.pub);
  	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		if (*rgb_buffer != NULL)
			free(*rgb_buffer);
		LOGE("[read_jpeg_file] read jpeg file fail!\n");
		jpeg_destroy_decompress(&cinfo);
		fclose(fp);
		return -1;
	}

	*rgb_buffer = NULL;
    
    jpeg_create_decompress(&cinfo);  
  
    jpeg_stdio_src(&cinfo, fp);  
  
    jpeg_read_header(&cinfo, TRUE);  
  
    //cinfo.out_color_space = JCS_RGB; //JCS_YCbCr;  // 设置输出格式  
  
    jpeg_start_decompress(&cinfo);  
  
    row_stride = cinfo.output_width * cinfo.output_components;  
    *width = cinfo.output_width;  
    *height = cinfo.output_height;  
  
    rgb_size = row_stride * cinfo.output_height; // 总大小  
    *size = rgb_size;  
  
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);  
      
    *rgb_buffer = (unsigned char *)malloc(sizeof(char) * rgb_size);    // 分配总内存  
      
    LOGD("debug--:\nrgb_size: %d, size: %d w: %d h: %d row_stride: %d \n", rgb_size,  
                cinfo.image_width*cinfo.image_height*3,  
                cinfo.image_width,   
                cinfo.image_height,  
                row_stride);  
    tmp_buffer = *rgb_buffer;  
    while (cinfo.output_scanline < cinfo.output_height) // 解压每一行  
    {  
        jpeg_read_scanlines(&cinfo, buffer, 1);  
        // 复制到内存  
        memcpy(tmp_buffer, buffer[0], row_stride);  
        tmp_buffer += row_stride;  
    }  
  
    jpeg_finish_decompress(&cinfo);  
    jpeg_destroy_decompress(&cinfo);  
  
    fclose(fp);  
  
    return SUCCESS;  
}  
  
_S32 JPEGMANAGE::write_jpeg_file(const char* jpeg_file, unsigned char* rgb_buffer, _S32 width, _S32 height, _S32 quality)  
{  
    struct jpeg_compress_struct cinfo;  
    //struct jpeg_error_mgr jerr;  
    _S32 row_stride = 0;  
    FILE* fp = NULL;  
    JSAMPROW row_pointer[1];  
  
    //cinfo.err = jpeg_std_error(&jerr); 
    struct my_error_mgr jerr;

	fp = fopen(jpeg_file, "wb");  
    if (fp == NULL)  
    {  
        LOGE("open file %s failed.\n", jpeg_file);  
        return -1;  
    }  
	//将错误处理结构对象绑定在JPEG对象上
	cinfo.err = jpeg_std_error(&jerr.pub);
  	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		LOGE("[write_jpeg_file] write jpeg file fail!\n");
		jpeg_destroy_compress(&cinfo);
		fclose(fp);
		return FAILURE;
	} 
  
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, fp);  
    cinfo.image_width = width;  
    cinfo.image_height = height;  
    cinfo.input_components = 3;  
    cinfo.in_color_space = JCS_RGB;  
  
    jpeg_set_defaults(&cinfo);  
    jpeg_set_quality(&cinfo, quality, (boolean)1);  // todo 1 == true  
    jpeg_start_compress(&cinfo, TRUE);  
    row_stride = width * cinfo.input_components;  
  
    while (cinfo.next_scanline < cinfo.image_height)  
    {  
        row_pointer[0] = &rgb_buffer[cinfo.next_scanline * row_stride];  
        jpeg_write_scanlines(&cinfo, row_pointer, 1);  
    }  
  
    jpeg_finish_compress(&cinfo);  
    jpeg_destroy_compress(&cinfo);  
    fclose(fp);  
  
    return SUCCESS;  
}  

_S32 JPEGMANAGE::magnifyImage(_U8* rgb_buffer, _S32 width, _S32 height, 
	_U8** torgb_buffer, _S32 towidth, _S32 toheight)
{
	_U64 dex = 0,fromRowStride = 0,toRowStride = 0;
    _U64 line = 0,column = 0;
    _S32 i = 0,j = 0;
	_FP64 Hrate = 0.0,Wrate = 0.0;

    if (rgb_buffer == NULL)
    {
        LOGE("[magnifyImage] rgb_buffer input NULL!\n");
        return FAILURE;
    }
	*torgb_buffer = (_U8*)malloc(towidth*toheight*3);
	if (*torgb_buffer == NULL)
	{
		LOGE("[magnifyImage] malloc fail!");
		return FAILURE;
	}

	Hrate = (_FP64)height/(_FP64)toheight;
	Wrate = (_FP64)width/(_FP64)towidth;

	fromRowStride = width*3;
	toRowStride   = towidth*3;

	LOGD("Hrate:%lf,Wrate:%lf,fromRowStride:%ld,toRowStride:%ld\n", 
		Hrate,Wrate,(long)fromRowStride,(long)toRowStride);

	for (i = 0; i < toheight;i++)
	{
		for (j = 0;j < towidth;j++)
		{
            line = (_U64)(i*Hrate+0.5);
            column = (_U64)(j*Wrate+0.5);

            if (line >= (_U64)height)
                line = (_U64)(i*Hrate);
            if (column >= (_U64)width)
                column = (_U64)(j*Wrate);
            
			dex = (_U64)(((_U64)line)*fromRowStride + ((_U64)column)*3);
			if (dex >= fromRowStride*height)
            {
                LOGE("i:%d,j:%d,dex:%ld\n",i,j,(long)dex );
                return FAILURE;
            }
			(*torgb_buffer)[i*toRowStride+j*3+0] = rgb_buffer[dex+0];
			(*torgb_buffer)[i*toRowStride+j*3+1] = rgb_buffer[dex+1];
			(*torgb_buffer)[i*toRowStride+j*3+2] = rgb_buffer[dex+2];
		}
	}
	LOGD("convert success!\n");
	return SUCCESS;
}
