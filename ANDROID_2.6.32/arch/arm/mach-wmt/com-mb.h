/* PVCS version log
** $Log:  $
 * 
 */
#ifndef COM_MB_H
/* To assert that only one occurrence is included */
#define COM_MB_H

/*--- com-mb.h---------------------------------------------------------------
*   Copyright (C) 2008 WonderMedia Tech. Inc.
*
* MODULE       : com-mb.h
* AUTHOR       : Jason Lin
* DATE         : 2008/12/08
* DESCRIPTION  : 
*------------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Jason Lin, 2008/12/08
*   First version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/

/*------------------------------------------------------------------------------
   Macros below are used for driver in IOCTL
------------------------------------------------------------------------------*/
#define MB_IOC_MAGIC			'M'

#define MBIO_MALLOC				_IOWR(MB_IOC_MAGIC, 0, unsigned long) // O: physical address	I: size
#define MBIO_FREE				_IOWR(MB_IOC_MAGIC, 1, unsigned long) // O: ummap size		I: user address if map, physical address if not map
#define MBIO_UNMAP				_IOWR(MB_IOC_MAGIC, 2, unsigned long) // O: ummap size		I: user address
#define MBIO_MBSIZE				_IOWR(MB_IOC_MAGIC, 3, unsigned long) // O: mb size			I: phys address
#define MBIO_MAX_AVAILABLE_SIZE	_IOR (MB_IOC_MAGIC, 4, unsigned long) // O: max free mba size
/* advance use only */
#define MBIO_GET				_IOW (MB_IOC_MAGIC, 5, unsigned long) // I: user address
#define MBIO_PUT				_IOW (MB_IOC_MAGIC, 6, unsigned long) // I: user address
#define MBIO_USER_TO_VIRT		_IOWR(MB_IOC_MAGIC, 7, unsigned long) // O: virt address		I: user address
#define MBIO_USER_TO_PHYS		_IOWR(MB_IOC_MAGIC, 8, unsigned long) // O: phys address		I: user address
#define MBIO_PREFETCH			_IOW (MB_IOC_MAGIC, 9, unsigned long) // I: size
#define MBIO_STATIC_SIZE		_IOR (MB_IOC_MAGIC,10, unsigned long) // O: static mba size
#define MBIO_MB_USER_COUNT		_IOWR(MB_IOC_MAGIC,11, unsigned long) // O: use counter		I: physical address
#define MBIO_FORCE_RESET		_IO  (MB_IOC_MAGIC,12)

#define MBIO_MAXiMUM			11
#endif /* ifndef COM_MB_H */

/*=== END com-mb.h ==========================================================*/

