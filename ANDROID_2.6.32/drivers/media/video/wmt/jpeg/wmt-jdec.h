/* PVCS version log
** $Log:  $
 * 
 */
#ifndef WM8605_JDEC_H
/* To assert that only one occurrence is included */
#define WM8605_JDEC_H

/*--- wm8605-jdec.h---------------------------------------------------------------
*   Copyright (C) 2009 WonderMedia Tech. Inc.
*
* MODULE       : wm8605-jdec.h
* AUTHOR       : Willy Chuang
* DATE         : 2009/08/03
* DESCRIPTION  : 
*------------------------------------------------------------------------------*/

/*--- History ------------------------------------------------------------------- 
*Version 0.01 , Willy Chuang, 2009/08/03
*	First version
*
*------------------------------------------------------------------------------*/
/*-------------------- MODULE DEPENDENCY -------------------------------------*/

/*-------------------- EXPORTED PRIVATE CONSTANTS ----------------------------*/
/* #define  VIAAPI_XXXX  1    *//*Example*/



/*-------------------- EXPORTED PRIVATE TYPES---------------------------------*/
/* typedef  void  viaapi_xxx_t;  *//*Example*/

/*------------------------------------------------------------------------------
    Definitions of enum
------------------------------------------------------------------------------*/



/*------------------------------------------------------------------------------
    Definitions of structures
------------------------------------------------------------------------------*/

/*-------------------- EXPORTED PRIVATE VARIABLES -----------------------------*/
#ifdef WM8605_JDEC_C 
    #define EXTERN
#else
    #define EXTERN   extern
#endif /* ifdef WM8605_JDEC_C */

/* EXTERN int      viaapi_xxxx; *//*Example*/

#undef EXTERN

/*--------------------- EXPORTED PRIVATE MACROS -------------------------------*/

/*--------------------- EXPORTED PRIVATE FUNCTIONS  ---------------------------*/
/* extern void  viaapi_xxxx(vdp_Void); *//*Example*/

    
#endif /* ifndef WM8605_JDEC_H */

/*=== END wm8605-jdec.h ==========================================================*/
