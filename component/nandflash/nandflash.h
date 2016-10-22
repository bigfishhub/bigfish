#ifndef __NANDFLASH_H__
#define __NANDFLASH_H__

#include "include/_define.h"
#include "utils/Logger.h"

#define NAND_BLOCK_STATUS_GOOD				0x0
#define NAND_BLOCK_STATUS_BAD				0x1

#define NAND_DEFAULT_PAGE_BUFFER_SIZE      0x800

enum NAND_BLOCK_ACCESS_ATTR 
{
    NAND_ACCESS_FIRST_BLOCK = 0x0000,
    NAND_ACCESS_MIDDLE_BLOCK,
    NAND_ACCESS_LAST_BLOCK,
    NAND_ACCESS_LESS_THAN_BLOCK
};
enum NAND_WRITE_BLOCK_ATTR
{
    NAND_WRITE_FIRST_BLOCK = 0x0000,
    NAND_WRITE_MIDDLE_BLOCK,
    NAND_WRITE_LAST_BLOCK,
    NAND_WRITE_LESS_THAN_BLOCK
};
enum NAND_READ_BLOCK_ATTR{
    NAND_READ_FIRST_BLOCK = 0x0000,
    NAND_READ_MIDDLE_BLOCK,
    NAND_READ_LAST_BLOCK,
    NAND_READ_LESS_THAN_BLOCK
};

typedef struct 
{
    _U32 size;	/* Total size of the MTD */
    _U32 erasesize;
    _U32 writesize;
    _U8 *readbuf;
    _U8 blockstatus[256];
}nand_mtd_info_t;

typedef struct
{
	_U32 new_range_base;
	_U32 first_block_data_offset;
	_U32 last_block_data_offset;
	_U32 num_block_access;
}range_descriptor_t;

class NANDflash
{
public:
	static NANDflash* getInstance();

	_S32 nand_flash_init(_CHAR* devname);
	_S32 nand_flash_block_erase(_S32 fd,_U32 index);
	_S32 nand_flash_write(_S32 flash_fd, _U64 offset, _U8 *src, _S32 writelength);
	_S32 nand_flash_read(_S32 flash_fd, _U64 offset, _U8 *dest, _U32 readlength, _U32 *retlength);
	_S32 nand_flash_close(_S32 flash_fd);

	_U32 nand_get_block_size();
	_U32 nand_get_mtd_size();
	_U32 nand_get_page_size();

private:
	NANDflash();

	_S32 nand_set_block_status(_S32 flash_fd);
	_S32 nand_flash_erase(_S32 flash_fd, _U64 offset, _S32 eraselength);
	_S32 nand_get_range_intersection ( _U32 range_base, _U32 range_len, _U32 blk_size, range_descriptor_t* rdt);
	_U32 nand_get_bad_blk_cnt (_S32 flash_fd, _U32 index1, _U32 index2);
	_U32 nand_get_good_block (_S32 flash_fd,_U32 address,_U32 block_size);
	_S32 nand_flash_check_block_address (_U32 address);
	_S32 nand_block_write ( _S32 flash_fd, _U32 address, _U8* data, _U8* read_buffer,
    					    _U32 offset, _U32 block_size, _S32 block_attr, _U32 copy_len);
	_S32 nand_block_read (	_S32 flash_fd, _U32 address,_U8* data, _U8* read_buffer,
                          	_U32 offset, _U32 block_size, _S32 block_attr, _S32 copy_len);
	_S32 nand_read_page (_S32 flash_fd, _U32 page_address, _U8* data, _U32 offset, _U32 size);
	_S32 nand_erase_block ( _S32 flash_fd, _U32 blk_addr, _U32 blk_size);

	_U8 *nand_get_read_buffer();
	_U8 *nand_get_block_status();

	nand_mtd_info_t nandMtdInfo;
};

#endif