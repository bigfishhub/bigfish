#ifndef __JPEGMANAGE_H__
#define __JPEGMANAGE_H__

#include <stdio.h>
#include "include/_define.h"
#include "utils/Logger.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "jpeglib.h"  
#include "jerror.h" 

#ifdef __cplusplus
}
#endif

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