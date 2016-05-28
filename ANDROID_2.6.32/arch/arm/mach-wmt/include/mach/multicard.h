/*
 *  linux/include/asm-arm/mach/multicard.h
 */
#ifndef ASMARM_MACH_Multicard_H
#define ASMARM_MACH_Multicard_H

#define  MSP_ID  0x01
#define  XDC_ID  0x02
#define  SDC_ID  0x04
#define  CFC_ID  0x08

#define  NO_ACTION   0
#define  HAVE_ACTION 1
#define  CLR_ACTION  2 // clear card-change int status

struct context {
	void *cf_cxt;
	void *ms_cxt;
	void *xd_cxt;
	void *sd_cxt;
} ;
//int common_func( void * dev_id, int id);
//int card_exist( void * dev_id, int id);

int common_func( int id,int self);
int card_exist( int id);

irqreturn_t atmsb_regular_isr(int irq, void *dev_id, struct pt_regs *regs);
irqreturn_t via_ata_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
irqreturn_t atsmb_regular_isr(int irq, void *dev_id);
irqreturn_t atxdb_regular_isr(int irq, void *dev_id, struct pt_regs *regs);
#endif
