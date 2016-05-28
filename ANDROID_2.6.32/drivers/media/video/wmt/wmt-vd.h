/*--- wmt-vd.h---------------------------------------------------------------
*   Copyright (C) 2008 WonderMedia Tech. Inc.
*
* MODULE       : wmt-vd.h -- 
* AUTHOR       : Jason Lin
* DATE         : 2008/12/08
* DESCRIPTION  : 
*------------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Jason Lin, 2008/12/08
*	First version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/
#include <linux/config.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>

#include "com-vd.h"

//#define CFG_VD_PERFORM_EN   /* Flag to enable VD performance analysis */

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

/*-------------------------------------------------------------------------- 
    Since the page size in kernel is 4 KB, so we may assume the max buffer 
    size as 
       input buffer size = (prd_size/8)*4KB
    In short, 1 KB PRD size could store about 0.5 MB data.
    If we support maximun input buffer size is 50 MB, we must set prd_size
    as 100 KB.
-------------------------------------------------------------------------- */
#define MAX_INPUT_BUF_SIZE  (100*1024) /* 100 KB */

struct videodecoder_info {
	void		*prdt_virt;
	dma_addr_t	prdt_phys;
};

struct videodecoder {
	char name[32];
	int	id;	// mapping to minor number of decoder
	int	(*setup)(void);
	int	(*remove)(void);
	int	(*suspend)(pm_message_t state);
	int	(*resume)(void);
	struct file_operations	fops;
#ifdef CONFIG_PROC_FS
	int (*get_info)(char *, char **, off_t, int); // Callback for text formatting
#endif /* CONFIG_PROC_FS */
	struct cdev *device;
};

#ifdef CFG_VD_PERFORM_EN
  int wmt_vd_timer_init(int vd_id, unsigned int count, int threshold_ms);
  int wmt_vd_timer_start(int vd_id);
  int wmt_vd_timer_stop(int vd_id);
  int wmt_vd_timer_exit(int vd_id);
#else
  #define wmt_vd_timer_init(a, b, c)
  #define wmt_vd_timer_start(a)
  #define wmt_vd_timer_stop(a)
  #define wmt_vd_timer_exit(a)
#endif

int	videodecoder_register(struct videodecoder *);
int	videodecoder_unregister(struct videodecoder *);

