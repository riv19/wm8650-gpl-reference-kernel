/*
 * wmt_env.c
 *
 * Maintained by:  Jeff Garzik <jgarzik@pobox.com>
 * Please ALWAYS copy linux-ide@vger.kernel.org on emails.
 *
 * Copyright 2003-2004 Red Hat, Inc.  All rights reserved.
 * Copyright 2003-2004 Jeff Garzik
 *
 * The contents of this file are subject to the Open
 * Software License version 1.1 that can be found at
 * http://www.opensource.org/licenses/osl-1.1.txt and is included herein
 * by reference.
 *
 * Some descriptions of such software. Copyright (c) 2008 WonderMedia Technologies, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation,
 * either version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * WonderMedia Technologies, Inc.
 * 10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
 */

#include <linux/mtd/mtd.h>

static const unsigned int sf_crc_table[256] = {
	0x00000000,	0x77073096,	0xee0e612c,	0x990951ba,	0x076dc419,
	0x706af48f,	0xe963a535,	0x9e6495a3,	0x0edb8832,	0x79dcb8a4,
	0xe0d5e91e,	0x97d2d988,	0x09b64c2b,	0x7eb17cbd,	0xe7b82d07,
	0x90bf1d91,	0x1db71064,	0x6ab020f2,	0xf3b97148,	0x84be41de,
	0x1adad47d,	0x6ddde4eb,	0xf4d4b551,	0x83d385c7,	0x136c9856,
	0x646ba8c0,	0xfd62f97a,	0x8a65c9ec,	0x14015c4f,	0x63066cd9,
	0xfa0f3d63,	0x8d080df5,	0x3b6e20c8,	0x4c69105e,	0xd56041e4,
	0xa2677172,	0x3c03e4d1,	0x4b04d447,	0xd20d85fd,	0xa50ab56b,
	0x35b5a8fa,	0x42b2986c,	0xdbbbc9d6,	0xacbcf940,	0x32d86ce3,
	0x45df5c75,	0xdcd60dcf,	0xabd13d59,	0x26d930ac,	0x51de003a,
	0xc8d75180,	0xbfd06116,	0x21b4f4b5,	0x56b3c423,	0xcfba9599,
	0xb8bda50f,	0x2802b89e,	0x5f058808,	0xc60cd9b2,	0xb10be924,
	0x2f6f7c87,	0x58684c11,	0xc1611dab,	0xb6662d3d,	0x76dc4190,
	0x01db7106,	0x98d220bc,	0xefd5102a,	0x71b18589,	0x06b6b51f,
	0x9fbfe4a5,	0xe8b8d433,	0x7807c9a2,	0x0f00f934,	0x9609a88e,
	0xe10e9818,	0x7f6a0dbb,	0x086d3d2d,	0x91646c97,	0xe6635c01,
	0x6b6b51f4,	0x1c6c6162,	0x856530d8,	0xf262004e,	0x6c0695ed,
	0x1b01a57b,	0x8208f4c1,	0xf50fc457,	0x65b0d9c6,	0x12b7e950,
	0x8bbeb8ea,	0xfcb9887c,	0x62dd1ddf,	0x15da2d49,	0x8cd37cf3,
	0xfbd44c65,	0x4db26158,	0x3ab551ce,	0xa3bc0074,	0xd4bb30e2,
	0x4adfa541,	0x3dd895d7,	0xa4d1c46d,	0xd3d6f4fb,	0x4369e96a,
	0x346ed9fc,	0xad678846,	0xda60b8d0,	0x44042d73,	0x33031de5,
	0xaa0a4c5f,	0xdd0d7cc9,	0x5005713c,	0x270241aa,	0xbe0b1010,
	0xc90c2086,	0x5768b525,	0x206f85b3,	0xb966d409,	0xce61e49f,
	0x5edef90e,	0x29d9c998,	0xb0d09822,	0xc7d7a8b4,	0x59b33d17,
	0x2eb40d81,	0xb7bd5c3b,	0xc0ba6cad,	0xedb88320,	0x9abfb3b6,
	0x03b6e20c,	0x74b1d29a,	0xead54739,	0x9dd277af,	0x04db2615,
	0x73dc1683,	0xe3630b12,	0x94643b84,	0x0d6d6a3e,	0x7a6a5aa8,
	0xe40ecf0b,	0x9309ff9d,	0x0a00ae27,	0x7d079eb1,	0xf00f9344,
	0x8708a3d2,	0x1e01f268,	0x6906c2fe,	0xf762575d,	0x806567cb,
	0x196c3671,	0x6e6b06e7,	0xfed41b76,	0x89d32be0,	0x10da7a5a,
	0x67dd4acc,	0xf9b9df6f,	0x8ebeeff9,	0x17b7be43,	0x60b08ed5,
	0xd6d6a3e8,	0xa1d1937e,	0x38d8c2c4,	0x4fdff252,	0xd1bb67f1,
	0xa6bc5767,	0x3fb506dd,	0x48b2364b,	0xd80d2bda,	0xaf0a1b4c,
	0x36034af6,	0x41047a60,	0xdf60efc3,	0xa867df55,	0x316e8eef,
	0x4669be79,	0xcb61b38c,	0xbc66831a,	0x256fd2a0,	0x5268e236,
	0xcc0c7795,	0xbb0b4703,	0x220216b9,	0x5505262f,	0xc5ba3bbe,
	0xb2bd0b28,	0x2bb45a92,	0x5cb36a04,	0xc2d7ffa7,	0xb5d0cf31,
	0x2cd99e8b,	0x5bdeae1d,	0x9b64c2b0,	0xec63f226,	0x756aa39c,
	0x026d930a,	0x9c0906a9,	0xeb0e363f,	0x72076785,	0x05005713,
	0x95bf4a82,	0xe2b87a14,	0x7bb12bae,	0x0cb61b38,	0x92d28e9b,
	0xe5d5be0d,	0x7cdcefb7,	0x0bdbdf21,	0x86d3d2d4,	0xf1d4e242,
	0x68ddb3f8,	0x1fda836e,	0x81be16cd,	0xf6b9265b,	0x6fb077e1,
	0x18b74777,	0x88085ae6,	0xff0f6a70,	0x66063bca,	0x11010b5c,
	0x8f659eff,	0xf862ae69,	0x616bffd3,	0x166ccf45,	0xa00ae278,
	0xd70dd2ee,	0x4e048354,	0x3903b3c2,	0xa7672661,	0xd06016f7,
	0x4969474d,	0x3e6e77db,	0xaed16a4a,	0xd9d65adc,	0x40df0b66,
	0x37d83bf0,	0xa9bcae53,	0xdebb9ec5,	0x47b2cf7f,	0x30b5ffe9,
	0xbdbdf21c,	0xcabac28a,	0x53b39330,	0x24b4a3a6,	0xbad03605,
	0xcdd70693,	0x54de5729,	0x23d967bf,	0xb3667a2e,	0xc4614ab8,
	0x5d681b02,	0x2a6f2b94,	0xb40bbe37,	0xc30c8ea1,	0x5a05df1b,
	0x2d02ef8d
};

/*
 * Table with supported baudrates
 */
static const unsigned long baudrate_table[] = {9600, 19200, 38400, 57600, 115200};

#define	N_BAUDRATES (sizeof(baudrate_table) / sizeof(baudrate_table[0]))
#define	DO1(buf) crc = sf_crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
#define	DO2(buf) do {DO1(buf);	DO1(buf); } while (0)
#define	DO4(buf) do {DO2(buf);	DO2(buf); } while (0)
#define	DO8(buf) do {DO4(buf);	DO4(buf); } while (0)

#define SF_UBOOT_ENV1 "u-boot env. cfg. 1-SF"
#define SF_UBOOT_ENV2 "u-boot env. cfg. 2-SF"
#define NAND_UBOOT_ENV1 "u-boot env. cfg. 1-NAND"
#define NAND_UBOOT_ENV2 "u-boot env. cfg. 2-NAND"
#define NOR_UBOOT_ENV1 "u-boot env. cfg. 1-NOR"
#define NOR_UBOOT_ENV2 "u-boot env. cfg. 2-NOR"

#define NOR_FLASH_TYPE  0
#define NAND_FLASH_TYPE 1
#define SPI_FLASH_TYPE  2
#define ENV_MAX_SIZE 0x10000
#define NAND_ENV_BLK_LENGTH 9
#define ENV_INVALID  0xFF

static unsigned int env_valid, env_invalid, nand_block;
struct env_t {
	unsigned int crc;    /* CRC32 over data bytes */
	unsigned char flags; /* active or obsolete */
	unsigned char data[ENV_MAX_SIZE - 5];
};
struct env_t_nand {
	unsigned int crc;    /* CRC32 over data bytes */
	unsigned char data[ENV_MAX_SIZE - 4];
};
static struct env_t *env_ptr;
static struct env_t_nand *env_ptr_nand;
extern struct mtd_info *mtd_table[];

static unsigned int env_crc32(unsigned char	*buf)
{
	unsigned int crc = 0;
	unsigned int len = ENV_MAX_SIZE - 5, val;
	val = *((volatile unsigned int *)(0xd8110100));
	if (val&2)
		len = ENV_MAX_SIZE - 4;

	crc	= crc ^ 0xffffffff;
	while (len >= 8) {
		DO8(buf);
		len	-= 8;
	}

	if (len) {
		do {
			DO1(buf);
		} while (--len);
	}
	return crc ^ 0xffffffff;
}

static unsigned int env_init(void)
{
	int i;
	size_t retvarlen;
	unsigned char mtd_dev[20];
	unsigned char uenv[2];
	unsigned int chip_id, val, crc32, env, blk_cnt = 0;
	unsigned int bootflash = SPI_FLASH_TYPE;
	unsigned int env1 = ENV_INVALID, env2 = ENV_INVALID, offs = 0;

	chip_id = *(volatile unsigned long *)(0xD8120000) >> 16;
	val = *((volatile unsigned int *)(0xd8110100));

	if (chip_id == 0x3426) {
		val = (val >> 1) & 0x3;
		if (val == 0)
			bootflash = NOR_FLASH_TYPE;
		else if (val == 1)
			bootflash = NAND_FLASH_TYPE;
	} else {
		if (val & 0x2)
			bootflash = NAND_FLASH_TYPE;
	}

	for (i = 0; i < MAX_MTD_DEVICES; i++) {
		if (!mtd_table[i])
			break;
		printk(KERN_DEBUG "MTD dev%d size: 0x%8.8llx \"%s\"\n",
			i, mtd_table[i]->size, mtd_table[i]->name);
		if (bootflash == SPI_FLASH_TYPE) {
			if (strcmp(SF_UBOOT_ENV1, mtd_table[i]->name) == 0)
				env1 = i;
			else if (strcmp(SF_UBOOT_ENV2, mtd_table[i]->name) == 0)
				env2 = i;
		} else if (bootflash == NAND_FLASH_TYPE) {
			if (strcmp(NAND_UBOOT_ENV1, mtd_table[i]->name) == 0)
				env1 = i;
			else if (strcmp(NAND_UBOOT_ENV2, mtd_table[i]->name) == 0)
				env2 = i;
		} else if (bootflash == NOR_FLASH_TYPE) {
			if (strcmp(NOR_UBOOT_ENV1, mtd_table[i]->name) == 0)
				env1 = i;
			else if (strcmp(NOR_UBOOT_ENV2, mtd_table[i]->name) == 0)
				env2 = i;
		}
	}

	uenv[0] = env1;
	uenv[1] = env2;

	if (bootflash == NAND_FLASH_TYPE) {
		env = uenv[0];
		if ((!mtd_table[env]) || (env == ENV_INVALID))
			return 1;
		sprintf(mtd_dev, "/dev/mtdblock%d", env);

next_blk:
		while (offs < (NAND_ENV_BLK_LENGTH * mtd_table[env]->erasesize)) {
			/*printk(KERN_WARNING "## Notice: Read MTD nand Dev%d blk_cnt=%d,", env, blk_cnt);*/
			if (!(mtd_table[env]->block_isbad(mtd_table[env], offs)))
				break;
			else {
				blk_cnt++;
				if (blk_cnt == NAND_ENV_BLK_LENGTH) {
					goto next_blk;
					offs = 0;					
				} else {
					offs += mtd_table[env]->erasesize;
					continue;
				}
			}
		}
		if (blk_cnt >= (NAND_ENV_BLK_LENGTH*2)) {
			printk(KERN_WARNING "## Warning: Read MTD nand Dev%d fail,", env);
			return 1;
		}
		if (mtd_table[env]->read(
			mtd_table[env],
			offs,
			ENV_MAX_SIZE,
			&retvarlen,
			(unsigned char *)env_ptr_nand)) {
			printk(KERN_WARNING "## Warning: Read MTD nand Dev%d fail, retvarlen = %x\n",
				env, retvarlen);
			return 1;
		}
		if ((env_ptr_nand->crc == 0xFFFFFFFF) && (blk_cnt < NAND_ENV_BLK_LENGTH)) {
			blk_cnt++;
			offs += mtd_table[env]->erasesize;
			if (blk_cnt == NAND_ENV_BLK_LENGTH)
				offs = 0;
			goto next_blk;
		}
		crc32 = env_crc32(env_ptr_nand->data);
		if (env_ptr_nand->crc == crc32) {
			env_valid = env;
			printk(KERN_WARNING "MTD nand env searched from Dev%d blk_cnt=%d,", env, blk_cnt);
			nand_block = blk_cnt%NAND_ENV_BLK_LENGTH;
			return 0;
		} else {
			blk_cnt++;
			if (blk_cnt < (NAND_ENV_BLK_LENGTH*2)) {
				offs += mtd_table[env]->erasesize;
				if (blk_cnt == NAND_ENV_BLK_LENGTH)
					offs = 0;
				goto next_blk;
			}
			printk(KERN_DEBUG "MTD Dev%d crc32 is not correct\n",	env);
			printk(KERN_DEBUG "crc32 = 0x%x , env_ptr_nand->crc = 0x%x\n",
			crc32, env_ptr_nand->crc);
		}
	} else {
		for (i = 0; i < 2; i++) {
			env = uenv[i];
			if ((!mtd_table[env]) || (env == ENV_INVALID))
				return 1;
			sprintf(mtd_dev, "/dev/mtdblock%d", env);
			if (mtd_table[env]->read(
				mtd_table[env],
				0,
				ENV_MAX_SIZE,
				&retvarlen,
				(unsigned char *)env_ptr)) {
				printk(KERN_WARNING "## Warning: Read MTD Dev%d fail, retvarlen = %x\n",
					env, retvarlen);
				return 1;
			}
			if (env_ptr->flags == 1) {
				crc32 = env_crc32(env_ptr->data);
				if (env_ptr->crc == crc32) {
					env_valid = env;
					if (env_valid == uenv[0])
						env_invalid = uenv[1];
					else
						env_invalid = uenv[0];
					return 0;
				} else {
					printk(KERN_DEBUG "MTD Dev%d crc32 is not correct\n",
						env);
					printk(KERN_DEBUG "crc32 = 0x%x , env_ptr->crc = 0x%x\n",
						crc32, env_ptr->crc);
				}
			} else {
				printk(KERN_DEBUG "MTD Dev%d isn't valid\n",
					env);
			}
		}
	}
	return 1;
}

static unsigned char env_get_char(int index)
{
	unsigned int val;
	val = *((volatile unsigned int *)(0xd8110100));

	if (val&2)
		return env_ptr_nand->data[index];
	else
		return env_ptr->data[index];
}

/*
 * Match a name / name=value pair
 *
 * s1 is either a simple 'name', or a 'name=value' pair.
 * i2 is the environment index for a 'name2=value2' pair.
 * If the names match, return the index for the value2, else NULL.
 */

static int envmatch(unsigned char *s1, int i2)
{
	while (*s1 == env_get_char(i2++))
		if (*s1++ == '=')
			return i2;
	if (*s1 == '\0' && env_get_char(i2-1) == '=')
		return i2;
	return -1;
}

static int env_update(void)
{
	size_t retvarlen;
	unsigned char invalid_env = 0;
	struct erase_info ei;
	unsigned int val, nand_boot = 2, orig_block = nand_block, tmp;
	loff_t offs = 0;
	unsigned int i, blocks_cnt = 0, blocks_size, chk_offs = 0;
	unsigned int block_offs[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	val = *((volatile unsigned int *)(0xd8110100));

	/* Update CRC */
	if (val&nand_boot)
		env_ptr_nand->crc = env_crc32(env_ptr_nand->data);
	else
		env_ptr->crc = env_crc32(env_ptr->data);

	ei.mtd = mtd_table[env_invalid];
	ei.addr = 0;
	ei.len = mtd_table[env_invalid]->size;
	ei.fail_addr = 0xffffffff;
	ei.callback = NULL;

	/* check if nand boot */
	if (val&nand_boot) {
		blocks_size = mtd_table[env_valid]->erasesize;
		blocks_cnt = 1;
		/* env just need one block so far*/
		/*while (ENV_MAX_SIZE > blocks_size) {
			blocks_size += mtd_table[env_valid]->erasesize;
			blocks_cnt++;
		}*/
		ei.mtd = mtd_table[env_valid];
		ei.len = mtd_table[env_valid]->erasesize;
		i = 0;
		offs = (orig_block+blocks_cnt) * mtd_table[env_valid]->erasesize;
		if (offs >= mtd_table[env_valid]->size)
			offs = 0;
		while (chk_offs < mtd_table[env_valid]->size && i < blocks_cnt) {
			if (mtd_table[env_valid]->block_isbad(mtd_table[env_valid], offs)) {
				printk(KERN_WARNING "## Warning: offs =0x%x is bad block on MTD Dev%d\n",
				(unsigned int)offs, env_valid);
				chk_offs += mtd_table[env_valid]->erasesize;
				offs += mtd_table[env_valid]->erasesize;
				if (offs >= mtd_table[env_valid]->size)
					offs = 0;
			} else {
				block_offs[i] = offs;
				printk(KERN_WARNING "## Warning: offs =0x%x is good block on MTD Dev%d\n",
				(unsigned int)offs, env_valid);
				chk_offs += mtd_table[env_valid]->erasesize;
				offs += mtd_table[env_valid]->erasesize;
				if (offs >= mtd_table[env_valid]->size)
					offs = 0;
				i++;
			}
		}
		if (chk_offs >= mtd_table[env_valid]->size || i < blocks_cnt) {
			printk(KERN_WARNING "## Warning: Can't find enough good blocks(%d) on MTD Dev%d\n",
			i, env_valid);
			return 1;
		}
	}
	if (val&nand_boot) {
		for (i = 0; i < blocks_cnt; i++) {
			ei.addr = block_offs[i];
			if (mtd_table[env_valid]->erase(mtd_table[env_valid], &ei) != 0) {
				printk(KERN_WARNING "## Warning: Erase MTD nand Dev%d new block = 0x%x fail\n",
					env_invalid, block_offs[i]/mtd_table[env_valid]->erasesize);
				return 1;
			}
			if (mtd_table[env_valid]->write(
			mtd_table[env_valid],
			block_offs[i],
			(i == (blocks_cnt-1))
			? (ENV_MAX_SIZE - i*mtd_table[env_valid]->erasesize)
			: mtd_table[env_valid]->erasesize,
			&retvarlen,
			(unsigned char *)env_ptr_nand) != 0) {
				printk(KERN_WARNING "## Warning: Change MTD nand Dev%d to valid env fail, write %d bytes\n",
				env_invalid, retvarlen);
				return 1;
			}
			/* new env is written to new block */
			/*printk(KERN_WARNING "block before update block = %d env_valid = %d\n", nand_block, env_valid);*/
			nand_block = block_offs[0]/mtd_table[env_valid]->erasesize;
			/*printk(KERN_WARNING "block is update block = %d env_valid = %d\n", nand_block, env_valid);*/
		}
		/*for (i = 0; i < blocks_cnt; block_offs[i] != (block * mtd_table[env_valid]->erasesize); i++) {*/
		ei.addr = orig_block * mtd_table[env_valid]->erasesize;
		if (mtd_table[env_valid]->erase(mtd_table[env_valid], &ei) != 0) {
			printk(KERN_WARNING "## Warning: Erase MTD nand Dev%d original block = 0x%x fail\n",
				env_invalid, orig_block);
			return 1;
		}
		/*}*/
		return 0;
	} else
		if (mtd_table[env_invalid]->erase(mtd_table[env_invalid], &ei) != 0) {
				printk(KERN_WARNING "## Warning: Erase MTD Dev%d fail\n",
					env_invalid);
				return 1;
		}

	if (mtd_table[env_invalid]->write(
		mtd_table[env_invalid],
		0,
		ENV_MAX_SIZE,
		&retvarlen,
		(unsigned char *)env_ptr) != 0) {
		printk(KERN_WARNING "## Warning: Change MTD Dev%d to valid env fail, write %d bytes\n",
			env_invalid, retvarlen);
		return 1;
	}
	if (mtd_table[env_valid]->write(
		mtd_table[env_valid],
		4,
		1,
		&retvarlen,
		&invalid_env) != 0) {
		printk(KERN_WARNING "## Warning: Change MTD Dev%d to invalid env fail, write %d byte\n",
			env_valid, retvarlen);
		return 1;
	}
	tmp = env_valid;
	env_valid = env_invalid;
	env_invalid = tmp;
	/*printk(KERN_WARNING "update: env is changed env_valid = %d, env_invalid = %d\n",
			env_valid, env_invalid);*/
	return 0;
}

/*
 * This function is called by device drivers and application to get the system parameter if existed.
 * varname: [IN] The system parameter name which application program or device driver expect to retrieve.
 * varval : [OUT] Pointer to a buffer to store the system parameter value.
 * varlen : [IN/OUT] the buffer size for the varval pointer as input. The actual varname size if return failure.
 * Return 0 indicates success. Nonzero indicates failure.
 */
int wmt_getsyspara(char *varname, unsigned char *varval, int *varlen)
{
	int i, j, k, nxt;
	int rcode = 0;

	if (env_ptr == NULL || (unsigned int)env_ptr != (unsigned int)env_ptr_nand) {
		env_ptr = kmalloc(ENV_MAX_SIZE, GFP_KERNEL);
		if (!env_ptr) /* need copy in RAM */ {
			rcode++;
			goto out;
		}
		env_ptr_nand = (struct env_t_nand *)env_ptr;
		printk(KERN_WARNING "kmalloc buffer env_ptr = 0x%x, env_ptr_nand = 0x%x\n",
		(unsigned int)env_ptr, (unsigned int)env_ptr_nand);
		nand_block = 0;
		if (env_init()) {
			rcode++;
			goto out;
		}
	}

	k = -1;
	i = 0;
	for (j = 0; env_get_char(j) != '\0'; j = nxt+1) {

		for (nxt = j; env_get_char(nxt) != '\0'; ++nxt)
			;
		k = envmatch((unsigned char *)varname, j);
		if (k < 0)
			continue;
		while (k < nxt)
			varval[i++] = env_get_char(k++);
		break;
	}
	varval[i] = '\0';

	if (i > *varlen) {
		printk(KERN_WARNING "## Warning: \"%s\" size(%d) exceed buffer size(%d)\n",
			varname, i, *varlen);
		*varlen = i;
		rcode++;
		goto out;
	}

	if (k < 0) {
		printk(KERN_WARNING "## Warning: \"%s\" not defined\n",
			varname);
		rcode++;
	}
out:
	/*kfree(env_ptr);*/
	return rcode;
}
EXPORT_SYMBOL_GPL(wmt_getsyspara);

/*
 * This function is called by device drivers and application to set the system parameter.
 * varname: [IN] The system parameter name which application program or device driver expect to set.
 * varval : [IN] Pointer to a buffer to store the system parameter value for setting.
 *               If the pointer is NULL and the system parameter is existed,
 *               then the system parameter will be clear after this function is returned success.
 * Return 0 indicates success. Nonzero indicates failure.
 */
int wmt_setsyspara(char *varname, char *varval)
{
	int len, oldval;
	int rcode = 0;
	unsigned char *env, *nxt = NULL;
	unsigned char *env_data;
	unsigned int val;

	if (env_ptr == NULL || (unsigned int)env_ptr != (unsigned int)env_ptr_nand) {
		env_ptr = kmalloc(ENV_MAX_SIZE, GFP_KERNEL);
		if (!env_ptr) /* need copy in RAM */ {
			rcode++;
			goto out;
		}
		env_ptr_nand = (struct env_t_nand *)env_ptr;
		printk(KERN_WARNING "wr kmalloc buffer env_ptr = 0x%x, env_ptr_nand = 0x%x\n",
		(unsigned int)env_ptr, (unsigned int)env_ptr_nand);
		nand_block = 0;
		if (env_init()) {
			rcode++;
			goto out;
		}
	}

	val = *((volatile unsigned int *)(0xd8110100));
	if (val&2)
		env_data = env_ptr_nand->data;
	else
		env_data = env_ptr->data;

	/*
	 * search if variable with this name already exists
	 */
	oldval = -1;
	for (env = env_data; *env; env = nxt+1) {
		for (nxt = env; *nxt; ++nxt)
			;
		oldval = envmatch((unsigned char *)varname, env-env_data);
		if (oldval >= 0)
			break;
	}

	/*
	 * Delete any existing definition
	 */
	if (oldval >= 0) {
		/*
		 * Switch to new baudrate if new baudrate is supported
		 */
		if (strcmp(varname, "baudrate") == 0) {
			int baudrate = simple_strtoul(varval, NULL, 10);
			int i;
			for (i = 0; i < N_BAUDRATES; ++i) {
				if (baudrate == baudrate_table[i])
					break;
			}
			if (i == N_BAUDRATES) {
				printk(KERN_INFO "## Baudrate %d bps not supported\n",
					baudrate);
				rcode++;
				goto out;
			}
		}

		if (*++nxt == '\0') {
			if (env > env_data)
				env--;
			else
				*env = '\0';
		} else {
			for (;;) {
				*env = *nxt++;
				if ((*env == '\0') && (*nxt == '\0'))
					break;
				++env;
			}
		}
		*++env = '\0';
	}

	if ((varval == NULL) || (*varval == '\0')) {
		if (oldval < 0) {
			printk(KERN_INFO "No assigned any value for %s\n",
				varname);
			rcode++;
		} else {
			/*
			 * varname will be clear
			 */
			printk(KERN_INFO "Delete environment variable: %s\n",
				varname);
			if (env_update())
				rcode++;
			else
				rcode = 0;
		}
		goto out;
	}

	/*
	 * Append new definition at the end
	 */
	for (env = env_data; *env || *(env+1); ++env)
		;
	if (env > env_data)
		++env;
	/*
	 * Overflow when:
	 * "varname" + "=" + "val" +"\0\0"  > ENV_SIZE - (env-env_data)
	 */
	len = strlen(varname) + 2;
	/* add '=' for first arg, ' ' for all others */
	len += strlen(varval);
	if (len > (&env_data[ENV_MAX_SIZE]-env)) {
		printk(KERN_WARNING "## Warning: environment overflow, \"%s\" deleted\n",
			varname);
		rcode++;
		goto out;
	}
	while ((*env = *varname++) != '\0')
		env++;

	*env = '=';
	while ((*++env = *varval++) != '\0')
		;

	/* end is marked with double '\0' */
	*++env = '\0';

	if (env_update())
		rcode++;
	else
		rcode = 0;
out:
	/*kfree(env_ptr);*/
	return rcode;
}
EXPORT_SYMBOL_GPL(wmt_setsyspara);
