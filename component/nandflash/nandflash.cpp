#define LOG_NDEBUG 0
#define LOG_TAG "NANDFLASH"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <mtd/mtd-user.h>
#include "nandflash.h"


static NANDflash* nandflashInstance = _NULL;

NANDflash* NANDflash::getInstance()
{
	if (_NULL == nandflashInstance)
	{
		nandflashInstance = new NANDflash();
	}
	return nandflashInstance;
}

NANDflash::NANDflash()
{

}

_S32 NANDflash::nand_flash_init(_CHAR* devname)
{
	_S32 flash_fd = -1,ret = -1;
	struct mtd_info_user mtdinfo;

	if ((flash_fd = open(devname,O_RDWR)) == -1)
	{
		LOGE("nand_flash_init open failed!\n");
		return -1;
	}

	if (ioctl(flash_fd,MEMGETINFO,&mtdinfo) == -1)
	{
		LOGE("nand_flash_init MEMGETINFO error:%s\n",strerror(errno));
		close(flash_fd);
		return -1;
	}
	LOGI("erasesize:%#x,size:%#x\n",mtdinfo.erasesize,mtdinfo.size);
	
	nandMtdInfo.size = mtdinfo.size;
	nandMtdInfo.erasesize = mtdinfo.erasesize;
    nandMtdInfo.writesize = mtdinfo.writesize;
	nandMtdInfo.readbuf = (_U8* )malloc(mtdinfo.erasesize);
	if (nandMtdInfo.readbuf == _NULL)
	{
		LOGE("nand_flash_init malloc fail!");
		close(flash_fd);
		return -1;
	}
	ret = nand_set_block_status(flash_fd);
	if (ret < 0)
	{
		LOGE("nand_flash_init nand_set_block_status fail!\n");
		close(flash_fd);
		return -1;
	}
	return flash_fd;
}

_S32 NANDflash::nand_flash_write(_S32 flash_fd,_U64 offset, _U8 *src, _S32 writelength)
{
    _U8* data;
    _U32 buffer_len;
    _U32 buffer_offset;
    _U32 range_base;
    _U32 data_offset= 0;
    _U32 copy_len;
    _S32 block_attr;
    _U32 i;
    _U32 data_incr=0;
    _U32 block_address;
    _U32 temp_block_address;
    _U32 block_size;
    _U8* read_buffer;
    _U32 range_len;
    _U32 return_val;
    range_descriptor_t range_descrptr;

    read_buffer = nand_get_read_buffer();
    block_size = nand_get_block_size();
    buffer_len = writelength;
    buffer_offset = (_U64)offset;
    data = src;

    LOGI("nand_flash_write start offset = 0x%08x,writelength = 0x%08x\n",(_U32)offset,writelength);
    if (buffer_len == 0)
    {
        return  0;
    }
    
    range_base = buffer_offset;
    range_len = buffer_len;

    nand_get_range_intersection(range_base, buffer_len, block_size, &range_descrptr);

    LOGI("range_descrptr.new_range_base 0x%x\n", range_descrptr.new_range_base);
    LOGI("range_descrptr.first_block_data_offset %d\n", range_descrptr.first_block_data_offset);
    LOGI("range_descrptr.last_block_data_offset %d\n", range_descrptr.last_block_data_offset);
    LOGI("range_descrptr.num_block_access %d\n", range_descrptr.num_block_access);

    block_address = range_descrptr.new_range_base;
    for ( i = 0; i < (range_descrptr.num_block_access); i++ )
    {
        if (  i == 0 && range_descrptr.first_block_data_offset > 0 )
        {
            data_offset = range_descrptr.first_block_data_offset;
            data_incr = block_size - range_descrptr.first_block_data_offset;
            
            if ( range_len <= (block_size - range_descrptr.first_block_data_offset) )
            {
                block_attr = NAND_ACCESS_LESS_THAN_BLOCK;
                copy_len = range_len;
            }
            else
            {
                block_attr = NAND_ACCESS_FIRST_BLOCK;
                copy_len = 0;
            }
        }
        else if ( (i == range_descrptr.num_block_access - 1) && ( range_descrptr.last_block_data_offset != 0 ) )
        {
            data_offset = range_descrptr.last_block_data_offset;
            block_attr = NAND_ACCESS_LAST_BLOCK;
            copy_len = 0;
        }
        else
        {
            data_offset = 0;
            block_attr = NAND_ACCESS_MIDDLE_BLOCK;
            copy_len = 0;
            data_incr = block_size;
        }

        temp_block_address = nand_get_good_block(flash_fd,block_address,block_size);
        
        /* check whether the block is in partition or not */
        if ( nand_flash_check_block_address (temp_block_address) != 0 )
        {
            LOGE("Failure during writing to nand partition, can not find good blocks in partition\n");
            return -1;
        }

        return_val = nand_block_write ( flash_fd,temp_block_address, data, read_buffer, data_offset, block_size, block_attr, copy_len );

        LOGI("    temp_block_address = %08x\n", temp_block_address);
        
        if ( return_val  != 0 )
        {
            LOGE("nand_block_write failed at address 0x%x\n", temp_block_address );
            /* call write error handler to perform skip-block algorithm on the partition.   */
            /* note: read_buffer contains the block of data which still needs to be written */
            return -1;
        }

        data += data_incr;
        block_address += block_size;
    }
    return 0;
}

_S32 NANDflash::nand_flash_read(_S32 flash_fd, _U64 offset, _U8 *dest, _U32 readlength, _U32 *retlength)
{
    _U8* data_buf;
    _U32 buffer_len;
    _U32 buffer_offset;
    _U32 range_base;
    //_U32 nand_read_disturb_address=0;
    _U32 data_offset= 0;
    _U32 copy_len;
    _S32 block_attr;
    _U32 data_incr=0;
    _U32 block_address;
    _U32 temp_block_address;
    _U32 range_len;
    _U32 block_size;
    _S32 return_val;
    _U8 *read_buffer;
    range_descriptor_t range_descrptr;
    _U32 i;


    read_buffer = nand_get_read_buffer();
    block_size = nand_get_block_size();
    buffer_len = readlength;
    buffer_offset = offset;
    data_buf = dest;

    range_base = buffer_offset;
    range_len = buffer_len;


    LOGI("nand_flash_read   buffer_offset = %08x,buffer_len = %08x\n", buffer_offset,buffer_len);

    nand_get_range_intersection(range_base, buffer_len, block_size, &range_descrptr);

    LOGI("range_descrptr.new_range_base 0x%x\n", range_descrptr.new_range_base);
    LOGI("range_descrptr.first_block_data_offset %d\n", range_descrptr.first_block_data_offset);
    LOGI("range_descrptr.last_block_data_offset %d\n", range_descrptr.last_block_data_offset);
    LOGI("range_descrptr.num_block_access %d\n", range_descrptr.num_block_access);

    block_address = range_descrptr.new_range_base;
    for ( i = 0; i < (range_descrptr.num_block_access); i++ )
    {
        if (  i == 0 && range_descrptr.first_block_data_offset > 0 )
        {
            data_offset = range_descrptr.first_block_data_offset;
            data_incr = block_size - range_descrptr.first_block_data_offset;
            
            if ( range_len <= (block_size - range_descrptr.first_block_data_offset) )
            {
                block_attr = NAND_ACCESS_LESS_THAN_BLOCK;
                copy_len = range_len;
            }
            else
            {
                block_attr = NAND_ACCESS_FIRST_BLOCK;
                copy_len = 0;
            }
        }
        else if ( (i == range_descrptr.num_block_access - 1) && ( range_descrptr.last_block_data_offset != 0 ) )
        {
            data_offset = range_descrptr.last_block_data_offset;
            block_attr = NAND_ACCESS_LAST_BLOCK;
            copy_len = 0;
        }
        else
        {
            data_offset = 0;
            block_attr = NAND_ACCESS_MIDDLE_BLOCK;
            copy_len = 0;
            data_incr = block_size;
        }

        temp_block_address = nand_get_good_block(flash_fd,block_address,block_size);
        if ( nand_flash_check_block_address ( temp_block_address) != 0 )
        {
            LOGE("Failure during reading from nand partition, can not find good blocks in partition\n");
            return -1;
        }
        LOGI("    temp_block_address = %08x\n", temp_block_address);
        LOGI("    copy_len           = %08x\n", copy_len);

        return_val = nand_block_read ( flash_fd,temp_block_address, data_buf, read_buffer, data_offset, block_size, block_attr, copy_len );

        if ( return_val  != 0 )
        {
            if (return_val == -1 )
            {
                //nand_read_disturb = 1;
                //nand_read_disturb_address = block_address;
            }
            else
            {
                printf("!!!!! UNC read error occurred, while reading from NAND flash\n");
            }
        }
        LOGI("nand_block_read return, data_incr = %d\n", data_incr);
        data_buf += data_incr;
        block_address += block_size;
    }
    return 0;
}

/*******************************************
 *
 * erase sectors between offset and offset+eraselength
 * operation is sector based, not bytes based
 * flash inner address should be sector aligned, like 64K or 128K aligned
 *
 ******************************************/
_S32 NANDflash::nand_flash_erase(_S32 flash_fd, _U64 offset, _S32 eraselength)
{
    range_descriptor_t range_descrptr;
    _U32 buffer_len;
    _U32 block_size;
    _U32 range_base;
    _U32 block_address;
    _U32 i;
    _S32 ret = -1;

    block_size = nand_get_block_size();
    range_base = offset;
    buffer_len = eraselength;
    nand_get_range_intersection(range_base, buffer_len, block_size, &range_descrptr);

    block_address = range_descrptr.new_range_base;
    
    for ( i = 0; i < (range_descrptr.num_block_access); i++ )
    {
        if ((ret =nand_erase_block ( flash_fd,block_address, block_size )) != 0)
        {
            LOGE("nand_flash_erase address : %#x blocksize: %#x fail!\n",block_address,block_size);
            //break;
        }
        block_address += block_size;
    }

    return ret;
}

/* nand_block_write:
 * This will write the data from the data buffer to the block specified by the address.
 * The address always need to be block alligned.
 * There can be four types of block writes:
 *      - Program the whole block
 *      - Program the bottom part of the block with the new data in the buffer,
 *        but preserve the old data in the top part.
 *      - Program the top part of the block but preserve the bottop part.
        - Program only a portion of a block.
 */

_S32 NANDflash::nand_block_write (  _S32 flash_fd, _U32 address, _U8* data, _U8* read_buffer,
                                    _U32 offset, _U32 block_size, _S32 block_attr, _U32 copy_len)
{
    _U32 i;
    _S32 return_val = -1;
    _U8 * temp_data;

    /*
     * NOTE!!
     * temp_data always refers to read_buffer, since it's 4 bytes aligned
     * 4 bytes alignment is not guaranted for data pointer
     */
    temp_data = read_buffer;

    LOGI("nand_block_write: address = 0x%x, offset = 0x%x, block_attr = 0x%x, copy_len=%d block_size %d \n", address, offset, block_attr, copy_len, block_size );

    if ( block_attr == NAND_WRITE_FIRST_BLOCK || block_attr == NAND_WRITE_LAST_BLOCK || block_attr == NAND_WRITE_LESS_THAN_BLOCK )
    {
        /* as we are not checking for read disturbance here no need to pass a separate pointer for read disturbance buffer */
        return_val = nand_block_read ( flash_fd,address, read_buffer, read_buffer, 0, block_size, NAND_READ_MIDDLE_BLOCK, 0 );
        /* now modify the data with the new data */
        if ( block_attr == NAND_WRITE_FIRST_BLOCK )
        {
            /* here we need copy the data from offset to the end of the block to the write buffer */
            
            for ( i = 0; i < block_size - offset; i++ )
            {
                read_buffer[ i + offset ] = data[ i ];
            }
        }
        else if ( block_attr == NAND_WRITE_LAST_BLOCK )
        {
            for ( i = 0; i < offset; i++)
            {
                read_buffer[ i ] = data[ i ];
            }
        }
        else
        {
            for ( i = 0; i < copy_len; i++ )
            {
                read_buffer[ i + offset]  = data[ i ];
            }
        }
    }
    else
    {
        /* copy data to read_buffer[] */
        for (i = 0; i < block_size; i++)
            read_buffer[i] = data[i];
    }

    return_val = nand_erase_block ( flash_fd,address, block_size );
    if ( return_val != 0 )
    {
        goto handle_error;
    }

    if (lseek(flash_fd, address,SEEK_SET) == -1)
    {
        LOGE("flash lseek error\n");
        return 1;
    }
    if (write(flash_fd, temp_data, block_size) == -1)
    {
        LOGE("flash write failed\n");
        return 1;
    }
    LOGI("nand_block_write end, address =0x%08X,%02x,%02x,%02x,%02x \n", address,data[0],data[1],data[2],data[3]);

handle_error:

    return return_val;
}

_S32 NANDflash::nand_flash_block_erase(_S32 fd,_U32 index)
{
    _U32 block_size;
    _U32 block_address;
    _S32 ret;

    block_size = nand_get_block_size();
    block_address = index * block_size;;
    
    if ((ret =nand_erase_block ( fd,block_address, block_size )) != 0)
    {
        LOGE("nand_flash_block_erase fail!\n");
    }

    return ret;
}

/*  *********************************************************************
    *  nand_erase_block(softc,range)
    *
    *  Erase a range of sectors
    *
    *  Input parameters:
    *      softc - our flash
    *      range - range structure
    *
    *  Return value:
    *      0 if ok
    *      else error
    *
    ********************************************************************* */
_S32 NANDflash::nand_erase_block ( _S32 flash_fd, _U32 blk_addr, _U32 blk_size )
{
    _S32 ret;
    struct erase_info_user erase_info;

    LOGI("nand_erase_block: blk_addr = 0x%x, blk_size = 0x%x\n", blk_addr, blk_size );

    erase_info.start = blk_addr;
    erase_info.length = blk_size;

    ret = ioctl(flash_fd,MEMERASE,&erase_info);
    if(ret)
    {
        LOGE("nand_erase_block ioctl error: addr=0x%08X ,res : %s\n",blk_addr,strerror(errno));
    }
    else
    {
        LOGI("nand_erase_block ok ret %d\n",ret);
    }
    return ret;
}

_S32 NANDflash::nand_block_read (_S32 flash_fd, _U32 address,_U8* data, _U8* read_buffer,
                               _U32 offset, _U32 block_size, _S32 block_attr, _S32 copy_len)
{
    _U32 i;
    _S32 return_val=0;
    _U32 size=0;
    _U32 start_address=0;
    range_descriptor_t pdt;
    _U32 page_address;
    _U32 read_distrb_detect = 0;

    LOGI("nand_block_read: address = 0x%x, offset = 0x%x, block_attr = 0x%x\n", address, offset, block_attr );

    if (  block_attr == NAND_READ_FIRST_BLOCK )
    {
        start_address = address + offset;
        size = block_size - offset;
    }
    else if ( block_attr == NAND_READ_LESS_THAN_BLOCK )
    {
        start_address = address + offset;
        size = copy_len;
    }
    else if ( block_attr == NAND_READ_MIDDLE_BLOCK )
    {
        start_address = address;
        size = block_size;
    }
    else if ( block_attr == NAND_READ_LAST_BLOCK )
    {
        start_address = address;
        size = offset;
    }

    LOGI("Reading from nand flash, address = 0x%x, size = %d\n", start_address, size);

    nand_get_range_intersection(start_address, size, nand_get_page_size(), &pdt);

    LOGI("pdt.new_range_base 0x%x\n", pdt.new_range_base);
    LOGI("pdt.first_block_data_offset %d\n", pdt.first_block_data_offset);
    LOGI("pdt.last_block_data_offset %d\n", pdt.last_block_data_offset);
    LOGI("pdt.num_block_access %d\n", pdt.num_block_access);

    page_address = pdt.new_range_base;
    for ( i = 0; i < pdt.num_block_access; i++ )
    {
        /* copy data from internal page buffer to data buffer */
        
        if (  i == 0 && pdt.first_block_data_offset > 0 )
        {
            offset = pdt.first_block_data_offset;
            
            if ( size <= (nand_get_page_size() - pdt.first_block_data_offset) )
            {
                copy_len = size;
            }
            else
            {
                copy_len = nand_get_page_size() - offset;
            }
        
        }
        else if ( ( i == pdt.num_block_access - 1 ) && ( pdt.last_block_data_offset != 0 ) )
        {
            offset = 0;
            copy_len = pdt.last_block_data_offset;
        }
        else
        {
            offset = 0;
            copy_len = nand_get_page_size();
        }
        
        return_val = nand_read_page ( flash_fd,page_address, data, offset, copy_len );
        
        if ( return_val != 0 )
        {
            /* we detected a read distrb*/
            read_distrb_detect = 1;
        }
        data += copy_len;
        page_address += nand_get_page_size();
    }

    if ( read_distrb_detect == 1 )
    {
        /* we got a read disturbance we need to reprogram the read data,
        * so we first read the data again in the read buffer, the higher level
        * code should use this data to reprogram this block.
        */
        LOGI("Read Disturbance detected\n");
        return_val = -1;
    }
    /* in case of a uncorr error the return val would be -1 */
    return return_val;
}

_S32 NANDflash::nand_read_page (_S32 flash_fd, _U32 page_address, _U8* data, _U32 offset, _U32 size )
{
    _U8 *page_buf = _NULL;
    ssize_t ret_value;

    LOGI("nand_read_page called, page_address = 0x%x, size = %d, offset = %d\n", page_address, size, offset);

    page_buf = (_U8*)malloc(nand_get_page_size());
    if (page_buf == _NULL)
    {
        LOGE("nand_read_page malloc fail! size: %#x ,res:%s\n",nand_get_page_size(),strerror(errno));
        return -1;
    }

    if (lseek(flash_fd, page_address, SEEK_SET) == -1)
    {
        LOGE("flash lseek fail! res:%s\n",strerror(errno));
        if (page_buf != _NULL) free(page_buf);
        return -1;
    }
    if ((ret_value = read(flash_fd, page_buf, nand_get_page_size())) == -1)
    {
        LOGE("flash read failed! res:%s\n",strerror(errno));
        if (page_buf != _NULL) free(page_buf);
        return -1;
    }
    memcpy((_VOID *)data,(_VOID *)&page_buf[offset],size);
    if (page_buf != _NULL) free(page_buf);

    LOGI("nand_read_page called, read num = %d\n", ret_value);

    return 0;
}

_S32 NANDflash::nand_set_block_status(_S32 flash_fd)
{
    _S32 ret;
    _U32 i;
    _U32 block_num;
    _U32 block_size;
    _U32 mtd_size;
    _U8 *block_status;
    _U64 temp_addr;

    if (flash_fd < 0) 
    {
    	LOGE("do not open nand flash!");
    	return -1;
    }

    block_status = nand_get_block_status();
    block_size = nand_get_block_size();
    mtd_size = nand_get_mtd_size();
    block_num = mtd_size/block_size;
    for ( i = 0; i < block_num; i++ )
    {
        temp_addr = i * block_size;
	  	ret = ioctl(flash_fd,MEMGETBADBLOCK,&temp_addr);
        if(ret > 0)
        {
	    	LOGI("nand_get_good_block bad block addr 0x%08x,ret %d\n", i * block_size,ret);
        	block_status[i] = NAND_BLOCK_STATUS_BAD;
        }
        else if (ret < 0)
        {
            LOGE("nand_get_good_block error:%s\n",strerror(errno));
            return -1;;
        }
        else
        {
            block_status[i] = NAND_BLOCK_STATUS_GOOD;
        }
    }
    return 0;
}

_S32 NANDflash::nand_flash_close(_S32 flash_fd)
{
    if (nandMtdInfo.readbuf != NULL)
    {
        free((_VOID *)nandMtdInfo.readbuf);
        nandMtdInfo.readbuf = NULL;
    }
    close(flash_fd);	
    return 0;
}

_S32 NANDflash::nand_get_range_intersection ( _U32 range_base, _U32 range_len, _U32 blk_size, range_descriptor_t* rdt)
{
    _U32 size;
    
    rdt->first_block_data_offset = 0;
    rdt->last_block_data_offset = 0;
    rdt->new_range_base = range_base;
    rdt->num_block_access = 0;
    
    size = range_len;

    LOGI("nand_get_range_intersection	called with range_base = 0x%x, range_len = %d\n", range_base, range_len );

    /* first align the address to blk boundary */
    rdt->new_range_base = range_base & (~(blk_size-1));
    rdt->first_block_data_offset = range_base - rdt->new_range_base;

    if ( rdt->first_block_data_offset > 0 )
    {
        rdt->num_block_access = 1;
        
        if ( range_len <= blk_size - rdt->first_block_data_offset )
        {
           /* the data is less than 1 blk */
           goto end_fn;
        }
        size -= (blk_size - rdt->first_block_data_offset);
    }

    /* the last blk may not cover a full block */
    if ( size % blk_size != 0 )
    {
        rdt->last_block_data_offset = size % blk_size;
        size -= rdt->last_block_data_offset;
        rdt->num_block_access++;
    }
    
    /* now the data should be blk aligned */
    rdt->num_block_access += size / blk_size;

end_fn:

return 0;
}

_U32 NANDflash::nand_get_bad_blk_cnt (_S32 flash_fd, _U32 index1, _U32 index2 )
{
    _U32 i;
    _U32 bad_blk_cnt = 0;
    _U8 *block_status;

    block_status = nand_get_block_status();
    for ( i = index1; i <= index2; i++ )
    {
        LOGI("fp_blk_status[%04x].block_status = %08x\n", i, block_status[i]);
        if ( block_status[i] == NAND_BLOCK_STATUS_BAD )
        {
            bad_blk_cnt++;
        }
    }
    return bad_blk_cnt;

}

_U32 NANDflash::nand_get_good_block (_S32 flash_fd,_U32 address,_U32 block_size)
{
    _U32 total_bad_blk_num = 0;
    _U32 bad_blk_num = 0;
    _U32 index1, index2;

    LOGI("nand_get_good_block called with address=0x%x\n", address );

    index1 = 0;
    index2 = address/block_size;
    
    while(1)
    {
        bad_blk_num = nand_get_bad_blk_cnt(flash_fd, index1, index2);
        
        if ( bad_blk_num == 0 )
        {
            break;
        }
        else if ( bad_blk_num > 0 )
        {
            total_bad_blk_num += bad_blk_num;
            index1 = index2 + 1;
            index2 = index2 + bad_blk_num;
        }
        else
        {
            /* we are out of blocks in partition */
            return -1;
        }
    }

    LOGI("nand_get_good_block returned with address = 0x%x\n", (address + total_bad_blk_num * block_size ) );

    return  (address + total_bad_blk_num * block_size );

}

_S32 NANDflash::nand_flash_check_block_address (_U32 address)
{
    /*if ( address == -1 )
    {
        return 1;
    }
    else
    {
        return 0;
    }*/
    return 0;
}

_U32 NANDflash::nand_get_block_size()
{
    return nandMtdInfo.erasesize;
}

_U32 NANDflash::nand_get_mtd_size()
{
    return nandMtdInfo.size;
}

_U8 *NANDflash::nand_get_read_buffer()
{
    return nandMtdInfo.readbuf;
}

_U8 *NANDflash::nand_get_block_status()
{
    return (_U8 *)&nandMtdInfo.blockstatus[0];
}

_U32 NANDflash::nand_get_page_size()
{
    if (nandMtdInfo.writesize == 0)
    {
        return NAND_DEFAULT_PAGE_BUFFER_SIZE;
    }
    else
    {
        return nandMtdInfo.writesize;
    }
}
