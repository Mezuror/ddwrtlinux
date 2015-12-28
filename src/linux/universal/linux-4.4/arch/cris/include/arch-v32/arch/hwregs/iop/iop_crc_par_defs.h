#ifndef __iop_crc_par_defs_h
#define __iop_crc_par_defs_h

/*
 * This file is autogenerated from
 *   file:           ../../inst/io_proc/rtl/iop_crc_par.r
 *     id:           <not found>
 *     last modfied: Mon Apr 11 16:08:45 2005
 *
 *   by /n/asic/design/tools/rdesc/src/rdes2c --outfile iop_crc_par_defs.h ../../inst/io_proc/rtl/iop_crc_par.r
 *      id: $Id: iop_crc_par_defs.h,v 1.5 2005/04/24 18:31:05 starvik Exp $
 * Any changes here will be lost.
 *
 * -*- buffer-read-only: t -*-
 */
/* Main access macros */
#ifndef REG_RD
#define REG_RD( scope, inst, reg ) \
  REG_READ( reg_##scope##_##reg, \
            (inst) + REG_RD_ADDR_##scope##_##reg )
#endif

#ifndef REG_WR
#define REG_WR( scope, inst, reg, val ) \
  REG_WRITE( reg_##scope##_##reg, \
             (inst) + REG_WR_ADDR_##scope##_##reg, (val) )
#endif

#ifndef REG_RD_VECT
#define REG_RD_VECT( scope, inst, reg, index ) \
  REG_READ( reg_##scope##_##reg, \
            (inst) + REG_RD_ADDR_##scope##_##reg + \
	    (index) * STRIDE_##scope##_##reg )
#endif

#ifndef REG_WR_VECT
#define REG_WR_VECT( scope, inst, reg, index, val ) \
  REG_WRITE( reg_##scope##_##reg, \
             (inst) + REG_WR_ADDR_##scope##_##reg + \
	     (index) * STRIDE_##scope##_##reg, (val) )
#endif

#ifndef REG_RD_INT
#define REG_RD_INT( scope, inst, reg ) \
  REG_READ( int, (inst) + REG_RD_ADDR_##scope##_##reg )
#endif

#ifndef REG_WR_INT
#define REG_WR_INT( scope, inst, reg, val ) \
  REG_WRITE( int, (inst) + REG_WR_ADDR_##scope##_##reg, (val) )
#endif

#ifndef REG_RD_INT_VECT
#define REG_RD_INT_VECT( scope, inst, reg, index ) \
  REG_READ( int, (inst) + REG_RD_ADDR_##scope##_##reg + \
	    (index) * STRIDE_##scope##_##reg )
#endif

#ifndef REG_WR_INT_VECT
#define REG_WR_INT_VECT( scope, inst, reg, index, val ) \
  REG_WRITE( int, (inst) + REG_WR_ADDR_##scope##_##reg + \
	     (index) * STRIDE_##scope##_##reg, (val) )
#endif

#ifndef REG_TYPE_CONV
#define REG_TYPE_CONV( type, orgtype, val ) \
  ( { union { orgtype o; type n; } r; r.o = val; r.n; } )
#endif

#ifndef reg_page_size
#define reg_page_size 8192
#endif

#ifndef REG_ADDR
#define REG_ADDR( scope, inst, reg ) \
  ( (inst) + REG_RD_ADDR_##scope##_##reg )
#endif

#ifndef REG_ADDR_VECT
#define REG_ADDR_VECT( scope, inst, reg, index ) \
  ( (inst) + REG_RD_ADDR_##scope##_##reg + \
    (index) * STRIDE_##scope##_##reg )
#endif

/* C-code for register scope iop_crc_par */

/* Register rw_cfg, scope iop_crc_par, type rw */
typedef struct {
  unsigned int mode    : 1;
  unsigned int crc_out : 1;
  unsigned int rev_out : 1;
  unsigned int inv_out : 1;
  unsigned int trig    : 2;
  unsigned int poly    : 3;
  unsigned int dummy1  : 23;
} reg_iop_crc_par_rw_cfg;
#define REG_RD_ADDR_iop_crc_par_rw_cfg 0
#define REG_WR_ADDR_iop_crc_par_rw_cfg 0

/* Register rw_init_crc, scope iop_crc_par, type rw */
typedef unsigned int reg_iop_crc_par_rw_init_crc;
#define REG_RD_ADDR_iop_crc_par_rw_init_crc 4
#define REG_WR_ADDR_iop_crc_par_rw_init_crc 4

/* Register rw_correct_crc, scope iop_crc_par, type rw */
typedef unsigned int reg_iop_crc_par_rw_correct_crc;
#define REG_RD_ADDR_iop_crc_par_rw_correct_crc 8
#define REG_WR_ADDR_iop_crc_par_rw_correct_crc 8

/* Register rw_ctrl, scope iop_crc_par, type rw */
typedef struct {
  unsigned int en : 1;
  unsigned int dummy1 : 31;
} reg_iop_crc_par_rw_ctrl;
#define REG_RD_ADDR_iop_crc_par_rw_ctrl 12
#define REG_WR_ADDR_iop_crc_par_rw_ctrl 12

/* Register rw_set_last, scope iop_crc_par, type rw */
typedef struct {
  unsigned int tr_dif : 1;
  unsigned int dummy1 : 31;
} reg_iop_crc_par_rw_set_last;
#define REG_RD_ADDR_iop_crc_par_rw_set_last 16
#define REG_WR_ADDR_iop_crc_par_rw_set_last 16

/* Register rw_wr1byte, scope iop_crc_par, type rw */
typedef struct {
  unsigned int data : 8;
  unsigned int dummy1 : 24;
} reg_iop_crc_par_rw_wr1byte;
#define REG_RD_ADDR_iop_crc_par_rw_wr1byte 20
#define REG_WR_ADDR_iop_crc_par_rw_wr1byte 20

/* Register rw_wr2byte, scope iop_crc_par, type rw */
typedef struct {
  unsigned int data : 16;
  unsigned int dummy1 : 16;
} reg_iop_crc_par_rw_wr2byte;
#define REG_RD_ADDR_iop_crc_par_rw_wr2byte 24
#define REG_WR_ADDR_iop_crc_par_rw_wr2byte 24

/* Register rw_wr3byte, scope iop_crc_par, type rw */
typedef struct {
  unsigned int data : 24;
  unsigned int dummy1 : 8;
} reg_iop_crc_par_rw_wr3byte;
#define REG_RD_ADDR_iop_crc_par_rw_wr3byte 28
#define REG_WR_ADDR_iop_crc_par_rw_wr3byte 28

/* Register rw_wr4byte, scope iop_crc_par, type rw */
typedef struct {
  unsigned int data : 32;
} reg_iop_crc_par_rw_wr4byte;
#define REG_RD_ADDR_iop_crc_par_rw_wr4byte 32
#define REG_WR_ADDR_iop_crc_par_rw_wr4byte 32

/* Register rw_wr1byte_last, scope iop_crc_par, type rw */
typedef struct {
  unsigned int data : 8;
  unsigned int dummy1 : 24;
} reg_iop_crc_par_rw_wr1byte_last;
#define REG_RD_ADDR_iop_crc_par_rw_wr1byte_last 36
#define REG_WR_ADDR_iop_crc_par_rw_wr1byte_last 36

/* Register rw_wr2byte_last, scope iop_crc_par, type rw */
typedef struct {
  unsigned int data : 16;
  unsigned int dummy1 : 16;
} reg_iop_crc_par_rw_wr2byte_last;
#define REG_RD_ADDR_iop_crc_par_rw_wr2byte_last 40
#define REG_WR_ADDR_iop_crc_par_rw_wr2byte_last 40

/* Register rw_wr3byte_last, scope iop_crc_par, type rw */
typedef struct {
  unsigned int data : 24;
  unsigned int dummy1 : 8;
} reg_iop_crc_par_rw_wr3byte_last;
#define REG_RD_ADDR_iop_crc_par_rw_wr3byte_last 44
#define REG_WR_ADDR_iop_crc_par_rw_wr3byte_last 44

/* Register rw_wr4byte_last, scope iop_crc_par, type rw */
typedef struct {
  unsigned int data : 32;
} reg_iop_crc_par_rw_wr4byte_last;
#define REG_RD_ADDR_iop_crc_par_rw_wr4byte_last 48
#define REG_WR_ADDR_iop_crc_par_rw_wr4byte_last 48

/* Register r_stat, scope iop_crc_par, type r */
typedef struct {
  unsigned int err  : 1;
  unsigned int busy : 1;
  unsigned int dummy1 : 30;
} reg_iop_crc_par_r_stat;
#define REG_RD_ADDR_iop_crc_par_r_stat 52

/* Register r_sh_reg, scope iop_crc_par, type r */
typedef unsigned int reg_iop_crc_par_r_sh_reg;
#define REG_RD_ADDR_iop_crc_par_r_sh_reg 56

/* Register r_crc, scope iop_crc_par, type r */
typedef unsigned int reg_iop_crc_par_r_crc;
#define REG_RD_ADDR_iop_crc_par_r_crc 60

/* Register rw_strb_rec_dif_in, scope iop_crc_par, type rw */
typedef struct {
  unsigned int last : 2;
  unsigned int dummy1 : 30;
} reg_iop_crc_par_rw_strb_rec_dif_in;
#define REG_RD_ADDR_iop_crc_par_rw_strb_rec_dif_in 64
#define REG_WR_ADDR_iop_crc_par_rw_strb_rec_dif_in 64


/* Constants */
enum {
  regk_iop_crc_par_calc                    = 0x00000001,
  regk_iop_crc_par_ccitt                   = 0x00000002,
  regk_iop_crc_par_check                   = 0x00000000,
  regk_iop_crc_par_crc16                   = 0x00000001,
  regk_iop_crc_par_crc32                   = 0x00000000,
  regk_iop_crc_par_crc5                    = 0x00000003,
  regk_iop_crc_par_crc5_11                 = 0x00000004,
  regk_iop_crc_par_dif_in                  = 0x00000002,
  regk_iop_crc_par_hi                      = 0x00000000,
  regk_iop_crc_par_neg                     = 0x00000002,
  regk_iop_crc_par_no                      = 0x00000000,
  regk_iop_crc_par_pos                     = 0x00000001,
  regk_iop_crc_par_pos_neg                 = 0x00000003,
  regk_iop_crc_par_rw_cfg_default          = 0x00000000,
  regk_iop_crc_par_rw_ctrl_default         = 0x00000000,
  regk_iop_crc_par_yes                     = 0x00000001
};
#endif /* __iop_crc_par_defs_h */
