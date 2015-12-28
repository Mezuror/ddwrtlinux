/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/bitops.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/module.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_mtd.h>
#include <linux/delay.h>

/* NANDc reg offsets */
#define NAND_FLASH_CMD			0x00
#define NAND_ADDR0			0x04
#define NAND_ADDR1			0x08
#define NAND_FLASH_CHIP_SELECT		0x0c
#define NAND_EXEC_CMD			0x10
#define NAND_FLASH_STATUS		0x14
#define NAND_BUFFER_STATUS		0x18
#define NAND_DEV0_CFG0			0x20
#define NAND_DEV0_CFG1			0x24
#define NAND_DEV0_ECC_CFG		0x28
#define NAND_DEV1_ECC_CFG		0x2c
#define NAND_DEV1_CFG0			0x30
#define NAND_DEV1_CFG1			0x34
#define NAND_READ_ID			0x40
#define NAND_READ_STATUS		0x44
#define NAND_DEV_CMD0			0xa0
#define NAND_DEV_CMD1			0xa4
#define NAND_DEV_CMD2			0xa8
#define NAND_DEV_CMD_VLD		0xac
#define SFLASHC_BURST_CFG		0xe0
#define NAND_ERASED_CW_DETECT_CFG	0xe8
#define NAND_ERASED_CW_DETECT_STATUS	0xec
#define NAND_EBI2_ECC_BUF_CFG		0xf0
#define FLASH_BUF_ACC			0x100

#define NAND_CTRL			0xf00
#define NAND_VERSION			0xf08
#define NAND_READ_LOCATION_0		0xf20
#define NAND_READ_LOCATION_1		0xf24

/* dummy register offsets, used by write_reg_dma */
#define NAND_DEV_CMD1_RESTORE		0xdead
#define NAND_DEV_CMD_VLD_RESTORE	0xbeef

/* NAND_FLASH_CMD bits */
#define PAGE_ACC			BIT(4)
#define LAST_PAGE			BIT(5)

/* NAND_FLASH_CHIP_SELECT bits */
#define NAND_DEV_SEL			0
#define DM_EN				BIT(2)

/* NAND_FLASH_STATUS bits */
#define FS_OP_ERR			BIT(4)
#define FS_READY_BSY_N			BIT(5)
#define FS_MPU_ERR			BIT(8)
#define FS_DEVICE_STS_ERR		BIT(16)
#define FS_DEVICE_WP			BIT(23)

/* NAND_BUFFER_STATUS bits */
#define BS_UNCORRECTABLE_BIT		BIT(8)
#define BS_CORRECTABLE_ERR_MSK		0x1f

/* NAND_DEVn_CFG0 bits */
#define DISABLE_STATUS_AFTER_WRITE	4
#define CW_PER_PAGE			6
#define UD_SIZE_BYTES			9
#define ECC_PARITY_SIZE_BYTES_RS	19
#define SPARE_SIZE_BYTES		23
#define NUM_ADDR_CYCLES			27
#define STATUS_BFR_READ			30
#define SET_RD_MODE_AFTER_STATUS	31

/* NAND_DEVn_CFG0 bits */
#define DEV0_CFG1_ECC_DISABLE		0
#define WIDE_FLASH			1
#define NAND_RECOVERY_CYCLES		2
#define CS_ACTIVE_BSY			5
#define BAD_BLOCK_BYTE_NUM		6
#define BAD_BLOCK_IN_SPARE_AREA		16
#define WR_RD_BSY_GAP			17
#define ENABLE_BCH_ECC			27

/* NAND_DEV0_ECC_CFG bits */
#define ECC_CFG_ECC_DISABLE		0
#define ECC_SW_RESET			1
#define ECC_MODE			4
#define ECC_PARITY_SIZE_BYTES_BCH	8
#define ECC_NUM_DATA_BYTES		16
#define ECC_FORCE_CLK_OPEN		30

/* NAND_DEV_CMD1 bits */
#define READ_ADDR			0

/* NAND_DEV_CMD_VLD bits */
#define READ_START_VLD			0

/* NAND_EBI2_ECC_BUF_CFG bits */
#define NUM_STEPS			0

/* NAND_ERASED_CW_DETECT_CFG bits */
#define ERASED_CW_ECC_MASK		1
#define AUTO_DETECT_RES			0
#define MASK_ECC			(1 << ERASED_CW_ECC_MASK)
#define RESET_ERASED_DET		(1 << AUTO_DETECT_RES)
#define ACTIVE_ERASED_DET		(0 << AUTO_DETECT_RES)
#define CLR_ERASED_PAGE_DET		(RESET_ERASED_DET | MASK_ECC)
#define SET_ERASED_PAGE_DET		(ACTIVE_ERASED_DET | MASK_ECC)

/* NAND_ERASED_CW_DETECT_STATUS bits */
#define PAGE_ALL_ERASED			BIT(7)
#define CODEWORD_ALL_ERASED		BIT(6)
#define PAGE_ERASED			BIT(5)
#define CODEWORD_ERASED			BIT(4)
#define ERASED_PAGE			(PAGE_ALL_ERASED | PAGE_ERASED)
#define ERASED_CW			(CODEWORD_ALL_ERASED | CODEWORD_ERASED)

/* Version Mask */
#define NAND_VERSION_MAJOR_MASK		0xf0000000
#define NAND_VERSION_MAJOR_SHIFT	28
#define NAND_VERSION_MINOR_MASK		0x0fff0000
#define NAND_VERSION_MINOR_SHIFT	16

/* NAND OP_CMDs */
#define PAGE_READ			0x2
#define PAGE_READ_WITH_ECC		0x3
#define PAGE_READ_WITH_ECC_SPARE	0x4
#define PROGRAM_PAGE			0x6
#define PAGE_PROGRAM_WITH_ECC		0x7
#define PROGRAM_PAGE_SPARE		0x9
#define BLOCK_ERASE			0xa
#define FETCH_ID			0xb
#define RESET_DEVICE			0xd

/*
 * the NAND controller performs reads/writes with ECC in 516 byte chunks.
 * the driver calls the chunks 'step' or 'codeword' interchangeably
 */
#define NANDC_STEP_SIZE			512

/*
 * the largest page size we support is 8K, this will have 16 steps/codewords
 * of 512 bytes each
 */
#define	MAX_NUM_STEPS			(SZ_8K / NANDC_STEP_SIZE)

/* we read at most 3 registers per codeword scan */
#define MAX_REG_RD			(3 * MAX_NUM_STEPS)

/* ECC modes */
#define ECC_NONE	BIT(0)
#define ECC_RS_4BIT	BIT(1)
#define	ECC_BCH_4BIT	BIT(2)
#define	ECC_BCH_8BIT	BIT(3)

struct desc_info {
	struct list_head list;

	enum dma_transfer_direction dir;
	struct scatterlist sgl;
	struct dma_async_tx_descriptor *dma_desc;
};

/*
 * holds the current register values that we want to write. acts as a contiguous
 * chunk of memory which we use to write the controller registers through DMA.
 */
struct nandc_regs {
	u32 cmd;
	u32 addr0;
	u32 addr1;
	u32 chip_sel;
	u32 exec;

	u32 cfg0;
	u32 cfg1;
	u32 ecc_bch_cfg;

	u32 clrflashstatus;
	u32 clrreadstatus;

	u32 cmd1;
	u32 vld;

	u32 orig_cmd1;
	u32 orig_vld;

	u32 ecc_buf_cfg;
};

/*
 * @cmd_crci:			ADM DMA CRCI for command flow control
 * @data_crci:			ADM DMA CRCI for data flow control
 * @list:			DMA descriptor list (list of desc_infos)
 * @dma_done:			completion param to denote end of last
 *				descriptor in the list
 * @data_buffer:		our local DMA buffer for page read/writes,
 *				used when we can't use the buffer provided
 *				by upper layers directly
 * @buf_size/count/start:	markers for chip->read_buf/write_buf functions
 * @reg_read_buf:		buffer for reading register data via DMA
 * @reg_read_pos:		marker for data read in reg_read_buf
 * @cfg0, cfg1, cfg0_raw..:	NANDc register configurations needed for
 *				ecc/non-ecc mode for the current nand flash
 *				device
 * @regs:			a contiguous chunk of memory for DMA register
 *				writes
 * @ecc_strength:		4 bit or 8 bit ecc, received via DT
 * @bus_width:			8 bit or 16 bit NAND bus width, received via DT
 * @ecc_modes:			supported ECC modes by the current controller,
 *				initialized via DT match data
 * @cw_size:			the number of bytes in a single step/codeword
 *				of a page, consisting of all data, ecc, spare
 *				and reserved bytes
 * @cw_data:			the number of bytes within a codeword protected
 *				by ECC
 * @bch_enabled:		flag to tell whether BCH or RS ECC mode is used
 * @status:			value to be returned if NAND_CMD_STATUS command
 *				is executed
 */
struct qcom_nandc_data {
	struct platform_device *pdev;
	struct device *dev;

	void __iomem *base;
	struct resource *res;

	struct clk *core_clk;
	struct clk *aon_clk;

	/* DMA stuff */
	struct dma_chan *chan;
	struct dma_slave_config	slave_conf;
	unsigned int cmd_crci;
	unsigned int data_crci;
	struct list_head list;
	struct completion dma_done;

	/* MTD stuff */
	struct nand_chip chip;
	struct mtd_info mtd;

	/* local data buffer and markers */
	u8		*data_buffer;
	int		buf_size;
	int		buf_count;
	int		buf_start;

	/* local buffer to read back registers */
	u32 *reg_read_buf;
	int reg_read_pos;

	/* required configs */
	u32 cfg0, cfg1;
	u32 cfg0_raw, cfg1_raw;
	u32 ecc_buf_cfg;
	u32 ecc_bch_cfg;
	u32 clrflashstatus;
	u32 clrreadstatus;
	u32 sflashc_burst_cfg;
	u32 cmd1, vld;

	/* register state */
	struct nandc_regs *regs;

	/* things we get from DT */
	int ecc_strength;
	int bus_width;

	u32 ecc_modes;

	/* misc params */
	int cw_size;
	int cw_data;
	bool use_ecc;
	bool bch_enabled;
	u8 status;
	int last_command;
};

static inline u32 nandc_read(struct qcom_nandc_data *this, int offset)
{
	return ioread32(this->base + offset);
}

static inline void nandc_write(struct qcom_nandc_data *this, int offset,
			       u32 val)
{
	iowrite32(val, this->base + offset);
}

/* helper to configure address register values */
static void set_address(struct qcom_nandc_data *this, u16 column, int page)
{
	struct nand_chip *chip = &this->chip;
	struct nandc_regs *regs = this->regs;

	if (chip->options & NAND_BUSWIDTH_16)
		column >>= 1;

	regs->addr0 = page << 16 | column;
	regs->addr1 = page >> 16 & 0xff;
}

/*
 * update_rw_regs:	set up read/write register values, these will be
 *			written to the NAND controller registers via DMA
 *
 * @num_cw:		number of steps for the read/write operation
 * @read:		read or write operation
 */
static void update_rw_regs(struct qcom_nandc_data *this, int num_cw, bool read)
{
	struct nandc_regs *regs = this->regs;

	if (read) {
		if (this->use_ecc)
			regs->cmd = PAGE_READ_WITH_ECC | PAGE_ACC | LAST_PAGE;
		else
			regs->cmd = PAGE_READ | PAGE_ACC | LAST_PAGE;
	} else {
			regs->cmd = PROGRAM_PAGE | PAGE_ACC | LAST_PAGE;
	}

	if (this->use_ecc) {
		regs->cfg0 = (this->cfg0 & ~(7U << CW_PER_PAGE)) |
				(num_cw - 1) << CW_PER_PAGE;

		regs->cfg1 = this->cfg1;
		regs->ecc_bch_cfg = this->ecc_bch_cfg;
	} else {
		regs->cfg0 = (this->cfg0_raw & ~(7U << CW_PER_PAGE)) |
				(num_cw - 1) << CW_PER_PAGE;

		regs->cfg1 = this->cfg1_raw;
		regs->ecc_bch_cfg = 1 << ECC_CFG_ECC_DISABLE;
	}

	regs->ecc_buf_cfg = this->ecc_buf_cfg;
	regs->clrflashstatus = this->clrflashstatus;
	regs->clrreadstatus = this->clrreadstatus;
	regs->exec = 1;
}

static int prep_dma_desc(struct qcom_nandc_data *this, bool read, int reg_off,
			 const void *vaddr, int size, bool flow_control)
{
	struct desc_info *desc;
	struct dma_async_tx_descriptor *dma_desc;
	struct scatterlist *sgl;
	struct dma_slave_config slave_conf;
	int r;

	desc = kzalloc(sizeof(*desc), GFP_KERNEL);
	if (!desc)
		return -ENOMEM;

	list_add_tail(&desc->list, &this->list);

	sgl = &desc->sgl;

	sg_init_one(sgl, vaddr, size);

	desc->dir = read ? DMA_DEV_TO_MEM : DMA_MEM_TO_DEV;

	r = dma_map_sg(this->dev, sgl, 1, desc->dir);
	if (r == 0) {
		r = -ENOMEM;
		goto err;
	}

	memset(&slave_conf, 0x00, sizeof(slave_conf));

	slave_conf.device_fc = flow_control;
	if (read) {
		slave_conf.src_maxburst = 16;
		slave_conf.src_addr = this->res->start + reg_off;
		slave_conf.slave_id = this->data_crci;
	} else {
		slave_conf.dst_maxburst = 16;
		slave_conf.dst_addr = this->res->start + reg_off;
		slave_conf.slave_id = this->cmd_crci;
	}

	r = dmaengine_slave_config(this->chan, &slave_conf);
	if (r) {
		dev_err(this->dev, "failed to configure dma channel\n");
		goto err;
	}

	dma_desc = dmaengine_prep_slave_sg(this->chan, sgl, 1, desc->dir, 0);
	if (!dma_desc) {
		dev_err(this->dev, "failed to prepare desc\n");
		r = -EINVAL;
		goto err;
	}

	desc->dma_desc = dma_desc;

	return 0;
err:
	kfree(desc);

	return r;
}

/*
 * read_reg_dma:	prepares a descriptor to read a given number of
 *			contiguous registers to the reg_read_buf pointer
 *
 * @first:		offset of the first register in the contiguous block
 * @num_regs:		number of registers to read
 */
static int read_reg_dma(struct qcom_nandc_data *this, int first, int num_regs)
{
	bool flow_control = false;
	void *vaddr;
	int size;

	if (first == NAND_READ_ID || first == NAND_FLASH_STATUS)
		flow_control = true;

	size = num_regs * sizeof(u32);
	vaddr = this->reg_read_buf + this->reg_read_pos;
	this->reg_read_pos += num_regs;

	return prep_dma_desc(this, true, first, vaddr, size, flow_control);
}

/*
 * write_reg_dma:	prepares a descriptor to write a given number of
 *			contiguous registers
 *
 * @first:		offset of the first register in the contiguous block
 * @num_regs:		number of registers to write
 */
static int write_reg_dma(struct qcom_nandc_data *this, int first, int num_regs)
{
	bool flow_control = false;
	struct nandc_regs *regs = this->regs;
	void *vaddr;
	int size;

	switch (first) {
	case NAND_FLASH_CMD:
		vaddr = &regs->cmd;
		flow_control = true;
		break;
	case NAND_EXEC_CMD:
		vaddr = &regs->exec;
		break;
	case NAND_FLASH_STATUS:
		vaddr = &regs->clrflashstatus;
		break;
	case NAND_DEV0_CFG0:
		vaddr = &regs->cfg0;
		break;
	case NAND_READ_STATUS:
		vaddr = &regs->clrreadstatus;
		break;
	case NAND_DEV_CMD1:
		vaddr = &regs->cmd1;
		break;
	case NAND_DEV_CMD1_RESTORE:
		first = NAND_DEV_CMD1;
		vaddr = &regs->orig_cmd1;
		break;
	case NAND_DEV_CMD_VLD:
		vaddr = &regs->vld;
		break;
	case NAND_DEV_CMD_VLD_RESTORE:
		first = NAND_DEV_CMD_VLD;
		vaddr = &regs->orig_vld;
		break;
	case NAND_EBI2_ECC_BUF_CFG:
		vaddr = &regs->ecc_buf_cfg;
		break;
	default:
		dev_err(this->dev, "invalid starting register\n");
		return -EINVAL;
	}

	size = num_regs * sizeof(u32);

	return prep_dma_desc(this, false, first, vaddr, size, flow_control);
}

/*
 * read_data_dma:	prepares a DMA descriptor to transfer data from the
 *			controller's internal buffer to the buffer 'vaddr'
 *
 * @reg_off:		offset within the controller's data buffer
 * @vaddr:		virtual address of the buffer we want to write to
 * @size:		DMA transaction size in bytes
 */
static int read_data_dma(struct qcom_nandc_data *this, int reg_off,
			 const u8 *vaddr, int size)
{
	return prep_dma_desc(this, true, reg_off, vaddr, size, false);
}

/*
 * write_data_dma:	prepares a DMA descriptor to transfer data from
 *			'vaddr' to the controller's internal buffer
 *
 * @reg_off:		offset within the controller's data buffer
 * @vaddr:		virtual address of the buffer we want to read from
 * @size:		DMA transaction size in bytes
 */
static int write_data_dma(struct qcom_nandc_data *this, int reg_off,
			  const u8 *vaddr, int size)
{
	return prep_dma_desc(this, false, reg_off, vaddr, size, false);
}

/*
 * helper to prepare dma descriptors to configure registers needed for reading a
 * codeword/step in a page
 */
static void config_cw_read(struct qcom_nandc_data *this)
{
	write_reg_dma(this, NAND_FLASH_CMD, 3);
	write_reg_dma(this, NAND_DEV0_CFG0, 3);
	write_reg_dma(this, NAND_EBI2_ECC_BUF_CFG, 1);

	write_reg_dma(this, NAND_EXEC_CMD, 1);

	read_reg_dma(this, NAND_FLASH_STATUS, 2);
	read_reg_dma(this, NAND_ERASED_CW_DETECT_STATUS, 1);
}

/*
 * helpers to prepare dma descriptors used to configure registers needed for
 * writing a codeword/step in a page
 */
static void config_cw_write_pre(struct qcom_nandc_data *this)
{
	write_reg_dma(this, NAND_FLASH_CMD, 3);
	write_reg_dma(this, NAND_DEV0_CFG0, 3);
	write_reg_dma(this, NAND_EBI2_ECC_BUF_CFG, 1);
}

static void config_cw_write_post(struct qcom_nandc_data *this)
{
	write_reg_dma(this, NAND_EXEC_CMD, 1);

	read_reg_dma(this, NAND_FLASH_STATUS, 1);

	write_reg_dma(this, NAND_FLASH_STATUS, 1);
	write_reg_dma(this, NAND_READ_STATUS, 1);
}

/*
 * the following functions are used within chip->cmdfunc() to perform different
 * NAND_CMD_* commands
 */

/* sets up descriptors for NAND_CMD_PARAM */
static int nandc_param(struct qcom_nandc_data *this)
{
	struct nandc_regs *regs = this->regs;

	/*
	 * NAND_CMD_PARAM is called before we know much about the FLASH chip
	 * in use. we configure the controller to perform a raw read of 512
	 * bytes to read onfi params
	 */
	regs->cmd = PAGE_READ | PAGE_ACC | LAST_PAGE;
	regs->addr0 = 0;
	regs->addr1 = 0;
	regs->cfg0 =  0 << CW_PER_PAGE
			| 512 << UD_SIZE_BYTES
			| 5 << NUM_ADDR_CYCLES
			| 0 << SPARE_SIZE_BYTES;

	regs->cfg1 =  7 << NAND_RECOVERY_CYCLES
			| 0 << CS_ACTIVE_BSY
			| 17 << BAD_BLOCK_BYTE_NUM
			| 1 << BAD_BLOCK_IN_SPARE_AREA
			| 2 << WR_RD_BSY_GAP
			| 0 << WIDE_FLASH
			| 1 << DEV0_CFG1_ECC_DISABLE;

	regs->ecc_bch_cfg = 1 << ECC_CFG_ECC_DISABLE;

	/* configure CMD1 and VLD for ONFI param probing */
	regs->vld = (this->vld & ~(1 << READ_START_VLD))
			| 0 << READ_START_VLD;

	regs->cmd1 = (this->cmd1 & ~(0xFF << READ_ADDR))
			| NAND_CMD_PARAM << READ_ADDR;

	regs->exec = 1;

	regs->orig_cmd1 = this->cmd1;
	regs->orig_vld = this->vld;

	write_reg_dma(this, NAND_DEV_CMD_VLD, 1);
	write_reg_dma(this, NAND_DEV_CMD1, 1);

	this->buf_count = 512;
	memset(this->data_buffer, 0xff, this->buf_count);

	config_cw_read(this);

	read_data_dma(this, FLASH_BUF_ACC, this->data_buffer, this->buf_count);

	/* restore CMD1 and VLD regs */
	write_reg_dma(this, NAND_DEV_CMD1_RESTORE, 1);
	write_reg_dma(this, NAND_DEV_CMD_VLD_RESTORE, 1);

	return 0;
}

/* sets up descriptors for NAND_CMD_ERASE1 */
static int erase_block(struct qcom_nandc_data *this, int page_addr)
{
	struct nandc_regs *regs = this->regs;

	regs->cmd = BLOCK_ERASE | PAGE_ACC | LAST_PAGE;
	regs->addr0 = page_addr;
	regs->addr1 = 0;
	regs->cfg0 = this->cfg0_raw & ~(7 << CW_PER_PAGE);
	regs->cfg1 = this->cfg1_raw;
	regs->exec = 1;
	regs->clrflashstatus = this->clrflashstatus;
	regs->clrreadstatus = this->clrreadstatus;

	write_reg_dma(this, NAND_FLASH_CMD, 3);
	write_reg_dma(this, NAND_DEV0_CFG0, 2);
	write_reg_dma(this, NAND_EXEC_CMD, 1);

	read_reg_dma(this, NAND_FLASH_STATUS, 1);

	write_reg_dma(this, NAND_FLASH_STATUS, 1);
	write_reg_dma(this, NAND_READ_STATUS, 1);

	return 0;
}

/* sets up descriptors for NAND_CMD_READID */
static int read_id(struct qcom_nandc_data *this, int column)
{
	struct nandc_regs *regs = this->regs;

	if (column == -1)
		return 0;

	regs->cmd = FETCH_ID;
	regs->addr0 = column;
	regs->addr1 = 0;
	regs->chip_sel = DM_EN;
	regs->exec = 1;

	write_reg_dma(this, NAND_FLASH_CMD, 4);
	write_reg_dma(this, NAND_EXEC_CMD, 1);

	read_reg_dma(this, NAND_READ_ID, 1);

	return 0;
}

/* sets up descriptors for NAND_CMD_RESET */
static int reset(struct qcom_nandc_data *this)
{
	struct nandc_regs *regs = this->regs;

	regs->cmd = RESET_DEVICE;
	regs->exec = 1;

	write_reg_dma(this, NAND_FLASH_CMD, 1);
	write_reg_dma(this, NAND_EXEC_CMD, 1);

	read_reg_dma(this, NAND_FLASH_STATUS, 1);

	return 0;
}

/* helpers to submit/free our list of dma descriptors */
static void dma_callback(void *param)
{
	struct qcom_nandc_data *this = param;
	struct completion *c = &this->dma_done;

	complete(c);
}

static int submit_descs(struct qcom_nandc_data *this)
{
	struct completion *c = &this->dma_done;
	struct desc_info *desc;
	int r;

	init_completion(c);

	list_for_each_entry(desc, &this->list, list) {
		/*
		 * we add a callback to the last descriptor in our list to
		 * notify completion of command
		 */
		if (list_is_last(&desc->list, &this->list)) {
			desc->dma_desc->callback = dma_callback;
			desc->dma_desc->callback_param = this;
		}

		dmaengine_submit(desc->dma_desc);
	}

	dma_async_issue_pending(this->chan);

	r = wait_for_completion_timeout(c, msecs_to_jiffies(500));
	if (!r)
		return -ETIMEDOUT;

	return 0;
}

static void free_descs(struct qcom_nandc_data *this)
{
	struct desc_info *desc, *n;

	list_for_each_entry_safe(desc, n, &this->list, list) {
		list_del(&desc->list);
		dma_unmap_sg(this->dev, &desc->sgl, 1, desc->dir);
		kfree(desc);
	}
}

/* reset the register read buffer for next NAND operation */
static void clear_read_regs(struct qcom_nandc_data *this)
{
	this->reg_read_pos = 0;
	memset(this->reg_read_buf, 0, MAX_REG_RD * sizeof(*this->reg_read_buf));
}

static void pre_command(struct qcom_nandc_data *this, int command)
{
	this->buf_count = 0;
	this->buf_start = 0;
	this->use_ecc = false;
	this->last_command = command;

	clear_read_regs(this);
}

/*
 * this is called after NAND_CMD_PAGEPROG and NAND_CMD_ERASE1 to set our
 * privately maintained status byte, this status byte can be read after
 * NAND_CMD_STATUS is called
 */
static void parse_erase_write_errors(struct qcom_nandc_data *this, int command)
{
	struct nand_chip *chip = &this->chip;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int num_cw;
	int i;

	num_cw = command == NAND_CMD_PAGEPROG ? ecc->steps : 1;

	for (i = 0; i < num_cw; i++) {
		__le32 flash_status = le32_to_cpu(this->reg_read_buf[i]);

		if (flash_status & FS_MPU_ERR)
			this->status &= ~NAND_STATUS_WP;

		if (flash_status & FS_OP_ERR || (i == (num_cw - 1) &&
				(flash_status & FS_DEVICE_STS_ERR)))
			this->status |= NAND_STATUS_FAIL;
	}
}

static void post_command(struct qcom_nandc_data *this, int command)
{
	switch (command) {
	case NAND_CMD_READID:
		memcpy(this->data_buffer, this->reg_read_buf, this->buf_count);
		break;
	case NAND_CMD_PAGEPROG:
	case NAND_CMD_ERASE1:
		parse_erase_write_errors(this, command);
		break;
	default:
		break;
	}
}

/*
 * Implements chip->cmdfunc. It's  only used for a limited set of commands.
 * The rest of the commands wouldn't be called by upper layers. For example,
 * NAND_CMD_READOOB would never be called because we have our own versions
 * of read_oob ops for nand_ecc_ctrl.
 */
static void qcom_nandc_command(struct mtd_info *mtd, unsigned int command,
			 int column, int page_addr)
{
	struct nand_chip *chip = mtd->priv;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	struct qcom_nandc_data *this = chip->priv;
	bool wait = false;
	int r = 0;

	pre_command(this, command);

	switch (command) {
	case NAND_CMD_RESET:
		r = reset(this);
		wait = true;
		break;

	case NAND_CMD_READID:
		this->buf_count = 4;
		r = read_id(this, column);
		wait = true;
		break;

	case NAND_CMD_PARAM:
		r = nandc_param(this);
		wait = true;
		break;

	case NAND_CMD_ERASE1:
		r = erase_block(this, page_addr);
		wait = true;
		break;

	case NAND_CMD_READ0:
		/* we read the entire page for now */
		WARN_ON(column != 0);

		this->use_ecc = true;
		set_address(this, 0, page_addr);
		update_rw_regs(this, ecc->steps, true);
		break;

	case NAND_CMD_SEQIN:
		WARN_ON(column != 0);
		set_address(this, 0, page_addr);
		break;

	case NAND_CMD_PAGEPROG:
	case NAND_CMD_STATUS:
	case NAND_CMD_NONE:
	default:
		break;
	}

	if (r) {
		dev_err(this->dev, "failure executing command %d\n",
			command);
		free_descs(this);
		return;
	}

	if (wait) {
		r = submit_descs(this);
		if (r)
			dev_err(this->dev,
				"failure submitting descs for command %d\n",
				command);
	}

	free_descs(this);

	post_command(this, command);
}

/*
 * when using RS ECC, the NAND controller flags an error when reading an
 * erased page. however, there are special characters at certain offsets when
 * we read the erased page. we check here if the page is really empty. if so,
 * we replace the magic characters with 0xffs
 */
static bool empty_page_fixup(struct qcom_nandc_data *this, u8 *data_buf)
{
	struct mtd_info *mtd = &this->mtd;
	struct nand_chip *chip = &this->chip;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int cwperpage = ecc->steps;
	u8 orig1[MAX_NUM_STEPS], orig2[MAX_NUM_STEPS];
	int i, j;

	/* if BCH is enabled, HW will take care of detecting erased pages */
	if (this->bch_enabled || !this->use_ecc)
		return false;

	for (i = 0; i < cwperpage; i++) {
		u8 *empty1, *empty2;
		__le32 flash_status = le32_to_cpu(this->reg_read_buf[3 * i]);

		/*
		 * an erased page flags an error in NAND_FLASH_STATUS, check if
		 * the page is erased by looking for 0x54s at offsets 3 and 175
		 * from the beginning of each codeword
		 */
		if (!(flash_status & FS_OP_ERR))
			break;

		empty1 = &data_buf[3 + i * this->cw_data];
		empty2 = &data_buf[175 + i * this->cw_data];

		/*
		 * if the error wasn't because of an erased page, bail out and
		 * and let someone else do the error checking
		 */
		if ((*empty1 == 0x54 && *empty2 == 0xff) ||
				(*empty1 == 0xff && *empty2 == 0x54)) {
			orig1[i] = *empty1;
			orig2[i] = *empty2;

			*empty1 = 0xff;
			*empty2 = 0xff;
		} else {
			break;
		}
	}

	if (i < cwperpage || memchr_inv(data_buf, 0xff, mtd->writesize))
		goto not_empty;

	/*
	 * tell the caller that the page was empty and is fixed up, so that
	 * parse_read_errors() doesn't think it's an error
	 */
	return true;

not_empty:
	/* restore original values if not empty*/
	for (j = 0; j < i; j++) {
		data_buf[3 + j * this->cw_data] = orig1[j];
		data_buf[175 + j * this->cw_data] = orig2[j];
	}

	return false;
}

struct read_stats {
	__le32 flash;
	__le32 buffer;
	__le32 erased_cw;
};

/*
 * reads back status registers set by the controller to notify page read
 * errors. this is equivalent to what 'ecc->correct()' would do.
 */
static int parse_read_errors(struct qcom_nandc_data *this, bool erased_page)
{
	struct mtd_info *mtd = &this->mtd;
	struct nand_chip *chip = &this->chip;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int cwperpage = ecc->steps;
	unsigned int max_bitflips = 0;
	int i;

	for (i = 0; i < cwperpage; i++) {
		int stat;
		struct read_stats *buf;

		buf = (struct read_stats *) (this->reg_read_buf + 3 * i);

		buf->flash = le32_to_cpu(buf->flash);
		buf->buffer = le32_to_cpu(buf->buffer);
		buf->erased_cw = le32_to_cpu(buf->erased_cw);

		if (buf->flash & (FS_OP_ERR | FS_MPU_ERR)) {

			/* ignore erased codeword errors */
			if (this->bch_enabled) {
				if ((buf->erased_cw & ERASED_CW) == ERASED_CW)
					continue;
			} else if (erased_page) {
				continue;
			}

			if (buf->buffer & BS_UNCORRECTABLE_BIT) {
				mtd->ecc_stats.failed++;
				continue;
			}
		}

		stat = buf->buffer & BS_CORRECTABLE_ERR_MSK;
		mtd->ecc_stats.corrected += stat;

		max_bitflips = max_t(unsigned int, max_bitflips, stat);
	}

	return max_bitflips;
}

/*
 * helper to perform the actual page read operation, used by ecc->read_page()
 * and ecc->read_oob()
 */
static int read_page_low(struct qcom_nandc_data *this, u8 *data_buf,
			 u8 *oob_buf)
{
	struct nand_chip *chip = &this->chip;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int i, r;

	/* queue cmd descs for each codeword */
	for (i = 0; i < ecc->steps; i++) {
		int data_size, oob_size;

		if (i == (ecc->steps - 1)) {
			data_size = ecc->size - ((ecc->steps - 1) << 2);
			oob_size = (ecc->steps << 2) + ecc->bytes;
		} else {
			data_size = this->cw_data;
			oob_size = ecc->bytes;
		}

		config_cw_read(this);

		if (data_buf)
			read_data_dma(this, FLASH_BUF_ACC, data_buf, data_size);

		if (oob_buf)
			read_data_dma(this, FLASH_BUF_ACC + data_size, oob_buf,
					oob_size);

		if (data_buf)
			data_buf += data_size;
		if (oob_buf)
			oob_buf += oob_size;
	}

	r = submit_descs(this);
	if (r)
		dev_err(this->dev, "failure to read page/oob\n");

	free_descs(this);

	return r;
}

/*
 * a helper that copies the last step/codeword of a page (containing free oob)
 * into our local buffer
 */
static int copy_last_cw(struct qcom_nandc_data *this, bool use_ecc, int page)
{
	struct nand_chip *chip = &this->chip;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int size;
	int r;

	clear_read_regs(this);

	size = use_ecc ? this->cw_data : this->cw_size;

	/* prepare a clean read buffer */
	memset(this->data_buffer, 0xff, size);

	this->use_ecc = use_ecc;
	set_address(this, this->cw_size * (ecc->steps - 1), page);
	update_rw_regs(this, 1, true);

	config_cw_read(this);

	read_data_dma(this, FLASH_BUF_ACC, this->data_buffer, size);

	r = submit_descs(this);
	if (r)
		dev_err(this->dev, "failed to copy last codeword\n");

	free_descs(this);

	return r;
}

/* implements ecc->read_page() */
static int qcom_nandc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	struct qcom_nandc_data *this = chip->priv;
	u8 *data_buf, *oob_buf = NULL;
	bool erased_page;
	int r;

	data_buf = buf;
	oob_buf = oob_required ? chip->oob_poi : NULL;

	r = read_page_low(this, data_buf, oob_buf);
	if (r) {
		dev_err(this->dev, "failure to read page\n");
		return r;
	}

	erased_page = empty_page_fixup(this, data_buf);

	return parse_read_errors(this, erased_page);
}

/* implements ecc->read_oob() */
static int qcom_nandc_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
			       int page)
{
	struct qcom_nandc_data *this = chip->priv;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int r;

	clear_read_regs(this);

	this->use_ecc = true;
	set_address(this, 0, page);
	update_rw_regs(this, ecc->steps, true);

	r = read_page_low(this, NULL, chip->oob_poi);
	if (r)
		dev_err(this->dev, "failure to read oob\n");

	return r;
}

/* implements ecc->read_oob_raw(), used to read the bad block marker flag */
static int qcom_nandc_read_oob_raw(struct mtd_info *mtd, struct nand_chip *chip,
				   int page)
{
	struct qcom_nandc_data *this = chip->priv;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	uint8_t *oob = chip->oob_poi;
	int start, length;
	int r;

	/*
	 * configure registers for a raw page read, the address is set to the
	 * beginning of the last codeword, we don't care about reading ecc
	 * portion of oob, just the free stuff
	 */
	r = copy_last_cw(this, false, page);
	if (r)
		return r;

	/*
	 * reading raw oob has 2 parts, first the bad block byte, then the
	 * actual free oob region. perform a memcpy in two steps
	 */
	start = mtd->writesize - (this->cw_size * (ecc->steps - 1));
	length = chip->options & NAND_BUSWIDTH_16 ? 2 : 1;

	memcpy(oob, this->data_buffer + start, length);

	oob += length;

	start = this->cw_data - (ecc->steps << 2) + 1;
	length = ecc->steps << 2;

	memcpy(oob, this->data_buffer + start, length);

	return 0;
}

/* implements ecc->write_page() */
static int qcom_nandc_write_page(struct mtd_info *mtd, struct nand_chip *chip,
				 const uint8_t *buf, int oob_required)
{
	struct qcom_nandc_data *this = chip->priv;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	u8 *data_buf, *oob_buf;
	int i, r = 0;

	clear_read_regs(this);

	data_buf = (u8 *) buf;
	oob_buf = chip->oob_poi;

	this->use_ecc = true;
	update_rw_regs(this, ecc->steps, false);

	for (i = 0; i < ecc->steps; i++) {
		int data_size, oob_size;

		if (i == (ecc->steps - 1)) {
			data_size = ecc->size - ((ecc->steps - 1) << 2);
			oob_size = (ecc->steps << 2) + ecc->bytes;
		} else {
			data_size = this->cw_data;
			oob_size = ecc->bytes;
		}

		config_cw_write_pre(this);
		write_data_dma(this, FLASH_BUF_ACC, data_buf, data_size);

		/*
		 * we don't really need to write anything to oob for the
		 * first n - 1 codewords since these oob regions just
		 * contain ecc that's written by the controller itself
		 */
		if (i == (ecc->steps - 1))
			write_data_dma(this, FLASH_BUF_ACC + data_size,
					oob_buf, oob_size);
		config_cw_write_post(this);

		data_buf += data_size;
		oob_buf += oob_size;
	}

	r = submit_descs(this);
	if (r)
		dev_err(this->dev, "failure to write page\n");

	free_descs(this);

	return r;
}

/*
 * implements ecc->write_oob()
 *
 * the NAND controller cannot write only data or only oob within a codeword,
 * since ecc is calculated for the combined codeword. we first copy the
 * entire contents for the last codeword(data + oob), replace the old oob
 * with the new one in chip->oob_poi, and then write the entire codeword.
 * this read-copy-write operation results in a slight perormance loss.
 */
static int qcom_nandc_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
				int page)
{
	struct qcom_nandc_data *this = chip->priv;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	uint8_t *oob = chip->oob_poi;
	int free_boff;
	int data_size, oob_size;
	int r, status = 0;

	r = copy_last_cw(this, true, page);
	if (r)
		return r;

	clear_read_regs(this);

	/* calculate the data and oob size for the last codeword/step */
	data_size = ecc->size - ((ecc->steps - 1) << 2);
	oob_size = (ecc->steps << 2) + ecc->bytes;

	/*
	 * the location of spare data in the oob buffer, we could also use
	 * ecc->layout.oobfree here
	 */
	free_boff = ecc->bytes * (ecc->steps - 1);

	/* override new oob content to last codeword */
	memcpy(this->data_buffer + data_size, oob + free_boff, oob_size);

	this->use_ecc = true;
	set_address(this, this->cw_size * (ecc->steps - 1), page);
	update_rw_regs(this, 1, false);

	config_cw_write_pre(this);
	write_data_dma(this, FLASH_BUF_ACC, this->data_buffer,
		data_size + oob_size);
	config_cw_write_post(this);

	r = submit_descs(this);

	free_descs(this);

	if (r) {
		dev_err(this->dev, "failure to write oob\n");
		return -EIO;
	}

	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

/* implements ecc->write_oob_raw(), used to write bad block marker flag */
static int qcom_nandc_write_oob_raw(struct mtd_info *mtd,
				    struct nand_chip *chip, int page)
{
	struct qcom_nandc_data *this = chip->priv;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	uint8_t *oob = chip->oob_poi;
	int start, length;
	int r, status = 0;

	r = copy_last_cw(this, false, page);
	if (r)
		return r;

	clear_read_regs(this);

	/*
	 * writing raw oob has 2 parts, first the bad block region, then the
	 * actual free region
	 */
	start = mtd->writesize - (this->cw_size * (ecc->steps - 1));
	length = chip->options & NAND_BUSWIDTH_16 ? 2 : 1;

	memcpy(this->data_buffer + start, oob, length);

	oob += length;

	start = this->cw_data - (ecc->steps << 2) + 1;
	length = ecc->steps << 2;

	memcpy(this->data_buffer + start, oob, length);

	/* prepare write */
	this->use_ecc = false;
	set_address(this, this->cw_size * (ecc->steps - 1), page);
	update_rw_regs(this, 1, false);

	config_cw_write_pre(this);
	write_data_dma(this, FLASH_BUF_ACC, this->data_buffer, this->cw_size);
	config_cw_write_post(this);

	r = submit_descs(this);

	free_descs(this);

	if (r) {
		dev_err(this->dev, "failure to write updated oob\n");
		return -EIO;
	}

	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

/*
 * the three functions below implement chip->read_byte(), chip->read_buf()
 * and chip->write_buf() respectively. these aren't used for
 * reading/writing page data, they are used for smaller data like reading
 * id, status etc
 */
static uint8_t qcom_nandc_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct qcom_nandc_data *this = chip->priv;
	uint8_t *buf = this->data_buffer;
	uint8_t ret = 0x0;

	if (this->last_command == NAND_CMD_STATUS) {
		ret = this->status;

		this->status = NAND_STATUS_READY | NAND_STATUS_WP;

		return ret;
	}

	if (this->buf_start < this->buf_count)
		ret = buf[this->buf_start++];

	return ret;
}

static void qcom_nandc_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	struct qcom_nandc_data *this = chip->priv;
	int real_len = min_t(size_t, len, this->buf_count - this->buf_start);

	memcpy(buf, this->data_buffer + this->buf_start, real_len);
	this->buf_start += real_len;
}

static void qcom_nandc_write_buf(struct mtd_info *mtd, const uint8_t *buf,
		int len)
{
	struct nand_chip *chip = mtd->priv;
	struct qcom_nandc_data *this = chip->priv;
	int real_len = min_t(size_t, len, this->buf_count - this->buf_start);

	memcpy(this->data_buffer + this->buf_start, buf, real_len);

	this->buf_start += real_len;
}

/* we support only one external chip for now */
static void qcom_nandc_select_chip(struct mtd_info *mtd, int chipnr)
{
	struct nand_chip *chip = mtd->priv;
	struct qcom_nandc_data *this = chip->priv;

	if (chipnr <= 0)
		return;

	dev_warn(this->dev, "invalid chip select\n");
}

/*
 * NAND controller page layout info
 *
 * |-----------------------|	  |---------------------------------|
 * |		xx.......xx|	  |		*********xx.......xx|
 * |	DATA	xx..ECC..xx|	  |	DATA	**SPARE**xx..ECC..xx|
 * |   (516)	xx.......xx|	  |  (516-n*4)	**(n*4)**xx.......xx|
 * |		xx.......xx|	  |		*********xx.......xx|
 * |-----------------------|	  |---------------------------------|
 *     codeword 1,2..n-1			codeword n
 *  <---(528/532 Bytes)---->	   <-------(528/532 Bytes)---------->
 *
 * n = number of codewords in the page
 * . = ECC bytes
 * * = spare bytes
 * x = unused/reserved bytes
 *
 * 2K page: n = 4, spare = 16 bytes
 * 4K page: n = 8, spare = 32 bytes
 * 8K page: n = 16, spare = 64 bytes
 *
 * the qcom nand controller operates at a sub page/codeword level. each
 * codeword is 528 and 532 bytes for 4 bit and 8 bit ECC modes respectively.
 * the number of ECC bytes vary based on the ECC strength and the bus width.
 *
 * the first n - 1 codewords contains 516 bytes of user data, the remaining
 * 12/16 bytes consist of ECC and reserved data. The nth codeword contains
 * both user data and spare(oobavail) bytes that sum up to 516 bytes.
 *
 * the layout described above is used by the controller when the ECC block is
 * enabled. When we read a page with ECC enabled, the unused/reserved bytes are
 * skipped and not copied to our internal buffer. therefore, the nand_ecclayout
 * layouts defined below doesn't consider the positions occupied by the reserved
 * bytes
 *
 * when the ECC block is disabled, one unused byte (or two for 16 bit bus width)
 * in the last codeword is the position of bad block marker. the bad block
 * marker cannot be accessed when ECC is enabled.
 *
 */

/*
 * Layouts for different page sizes and ecc modes. We skip the eccpos field
 * since it isn't needed for this driver
 */

/* 2K page, 4 bit ECC */
static struct nand_ecclayout layout_oob_64 = {
	.eccbytes	= 40,
	.oobfree	= {
				{ 30, 16 },
			  },
};

/* 4K page, 4 bit ECC, 8/16 bit bus width */
static struct nand_ecclayout layout_oob_128 = {
	.eccbytes	= 80,
	.oobfree	= {
				{ 70, 32 },
			  },
};

/* 4K page, 8 bit ECC, 8 bit bus width */
static struct nand_ecclayout layout_oob_224_x8 = {
	.eccbytes	= 104,
	.oobfree	= {
				{ 91, 32 },
			  },
};

/* 4K page, 8 bit ECC, 16 bit bus width */
static struct nand_ecclayout layout_oob_224_x16 = {
	.eccbytes	= 112,
	.oobfree	= {
				{ 98, 32 },
			  },
};

/* 8K page, 4 bit ECC, 8/16 bit bus width */
static struct nand_ecclayout layout_oob_256 = {
	.eccbytes	= 160,
	.oobfree	= {
				{ 151, 64 },
			  },
};

/*
 * this is called before scan_ident, we do some minimal configurations so
 * that reading ID and ONFI params work
 */
static void qcom_nandc_pre_init(struct qcom_nandc_data *this)
{
	/* kill onenand */
	nandc_write(this, SFLASHC_BURST_CFG, 0);

	/* enable ADM DMA */
	nandc_write(this, NAND_FLASH_CHIP_SELECT, DM_EN);

	/* save the original values of these registers */
	this->cmd1 = nandc_read(this, NAND_DEV_CMD1);
	this->vld = nandc_read(this, NAND_DEV_CMD_VLD);

	/* initial status value */
	this->status = NAND_STATUS_READY | NAND_STATUS_WP;
}

static int qcom_nandc_ecc_init(struct qcom_nandc_data *this)
{
	struct mtd_info *mtd = &this->mtd;
	struct nand_chip *chip = &this->chip;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int cwperpage;
	bool wide_bus;

	/* the nand controller fetches codewords/chunks of 512 bytes */
	cwperpage = mtd->writesize >> 9;

	ecc->strength = this->ecc_strength;

	wide_bus = chip->options & NAND_BUSWIDTH_16 ? true : false;

	if (ecc->strength >= 8) {
		/* 8 bit ECC defaults to BCH ECC on all platforms */
		ecc->bytes = wide_bus ? 14 : 13;
	} else {
		/*
		 * if the controller supports BCH for 4 bit ECC, the controller
		 * uses lesser bytes for ECC. If RS is used, the ECC bytes is
		 * always 10 bytes
		 */
		if (this->ecc_modes & ECC_BCH_4BIT)
			ecc->bytes = wide_bus ? 8 : 7;
		else
			ecc->bytes = 10;
	}

	/* each step consists of 512 bytes of data */
	ecc->size = NANDC_STEP_SIZE;

	ecc->read_page		= qcom_nandc_read_page;
	ecc->read_oob		= qcom_nandc_read_oob;
	ecc->write_page		= qcom_nandc_write_page;
	ecc->write_oob		= qcom_nandc_write_oob;

	/*
	 * the bad block marker is readable only when we read the page with ECC
	 * disabled. all the ops above run with ECC enabled. We need raw read
	 * and write function for oob in order to access bad block marker.
	 */
	ecc->read_oob_raw	= qcom_nandc_read_oob_raw;
	ecc->write_oob_raw	= qcom_nandc_write_oob_raw;

	switch (mtd->oobsize) {
	case 64:
		ecc->layout = &layout_oob_64;
		break;
	case 128:
		ecc->layout = &layout_oob_128;
		break;
	case 224:
		if (wide_bus)
			ecc->layout = &layout_oob_224_x16;
		else
			ecc->layout = &layout_oob_224_x8;
		break;
	case 256:
		ecc->layout = &layout_oob_256;
		break;
	default:
		dev_err(this->dev, "unsupported NAND device, oobsize %d\n",
			mtd->oobsize);
		return -ENODEV;
	}

	ecc->mode = NAND_ECC_HW;

	/* enable ecc by default */
	this->use_ecc = true;

	return 0;
}

static void qcom_nandc_hw_post_init(struct qcom_nandc_data *this)
{
	struct mtd_info *mtd = &this->mtd;
	struct nand_chip *chip = &this->chip;
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int cwperpage = mtd->writesize / ecc->size;
	int spare_bytes, bad_block_byte;
	bool wide_bus;
	int ecc_mode = 0;

	wide_bus = chip->options & NAND_BUSWIDTH_16 ? true : false;

	if (ecc->strength >= 8) {
		this->cw_size = 532;

		spare_bytes = wide_bus ? 0 : 2;

		this->bch_enabled = true;
		ecc_mode = 1;
	} else {
		this->cw_size = 528;

		if (this->ecc_modes & ECC_BCH_4BIT) {
			spare_bytes = wide_bus ? 2 : 4;

			this->bch_enabled = true;
			ecc_mode = 0;
		} else {
			spare_bytes = wide_bus ? 0 : 1;
		}
	}

	/*
	 * DATA_UD_BYTES varies based on whether the read/write command protects
	 * spare data with ECC too. We protect spare data by default, so we set
	 * it to main + spare data, which are 512 and 4 bytes respectively.
	 */
	this->cw_data = 516;

	bad_block_byte = mtd->writesize - this->cw_size * (cwperpage - 1) + 1;

	this->cfg0 = (cwperpage - 1) << CW_PER_PAGE
				| this->cw_data << UD_SIZE_BYTES
				| 0 << DISABLE_STATUS_AFTER_WRITE
				| 5 << NUM_ADDR_CYCLES
				| ecc->bytes << ECC_PARITY_SIZE_BYTES_RS
				| 0 << STATUS_BFR_READ
				| 1 << SET_RD_MODE_AFTER_STATUS
				| spare_bytes << SPARE_SIZE_BYTES;

	this->cfg1 = 7 << NAND_RECOVERY_CYCLES
				| 0 <<  CS_ACTIVE_BSY
				| bad_block_byte << BAD_BLOCK_BYTE_NUM
				| 0 << BAD_BLOCK_IN_SPARE_AREA
				| 2 << WR_RD_BSY_GAP
				| wide_bus << WIDE_FLASH
				| this->bch_enabled << ENABLE_BCH_ECC;

	this->cfg0_raw = (cwperpage - 1) << CW_PER_PAGE
				| this->cw_size << UD_SIZE_BYTES
				| 5 << NUM_ADDR_CYCLES
				| 0 << SPARE_SIZE_BYTES;

	this->cfg1_raw = 7 << NAND_RECOVERY_CYCLES
				| 0 << CS_ACTIVE_BSY
				| 17 << BAD_BLOCK_BYTE_NUM
				| 1 << BAD_BLOCK_IN_SPARE_AREA
				| 2 << WR_RD_BSY_GAP
				| wide_bus << WIDE_FLASH
				| 1 << DEV0_CFG1_ECC_DISABLE;

	this->ecc_bch_cfg = this->bch_enabled << ECC_CFG_ECC_DISABLE
				| 0 << ECC_SW_RESET
				| this->cw_data << ECC_NUM_DATA_BYTES
				| 1 << ECC_FORCE_CLK_OPEN
				| ecc_mode << ECC_MODE
				| ecc->bytes << ECC_PARITY_SIZE_BYTES_BCH;

	this->ecc_buf_cfg = 0x203 << NUM_STEPS;

	this->clrflashstatus = FS_READY_BSY_N;
	this->clrreadstatus = 0xc0;

	dev_dbg(this->dev,
		"cfg0 %x cfg1 %x ecc_buf_cfg %x ecc_bch cfg %x cw_size %d cw_data %d strength %d parity_bytes %d steps %d\n",
		this->cfg0, this->cfg1, this->ecc_buf_cfg,
		this->ecc_bch_cfg, this->cw_size, this->cw_data,
		ecc->strength, ecc->bytes, cwperpage);
}

static int qcom_nandc_alloc(struct qcom_nandc_data *this)
{
	int r;

	r = dma_set_coherent_mask(this->dev, DMA_BIT_MASK(32));
	if (r) {
		dev_err(this->dev, "failed to set DMA mask\n");
		return r;
	}

	/*
	 * we use the internal buffer for reading ONFI params, reading small
	 * data like ID and status, and preforming read-copy-write operations
	 * when writing to a codeword partially. 532 is the maximum possible
	 * size of a codeword for our nand controller
	 */
	this->buf_size = 532;

	this->data_buffer = devm_kzalloc(this->dev, this->buf_size, GFP_KERNEL);
	if (!this->data_buffer)
		return -ENOMEM;

	this->regs = devm_kzalloc(this->dev, sizeof(*this->regs), GFP_KERNEL);
	if (!this->regs)
		return -ENOMEM;

	this->reg_read_buf = devm_kzalloc(this->dev,
				MAX_REG_RD * sizeof(*this->reg_read_buf),
				GFP_KERNEL);
	if (!this->reg_read_buf)
		return -ENOMEM;

	INIT_LIST_HEAD(&this->list);

	this->chan = dma_request_slave_channel(this->dev, "rxtx");
	if (!this->chan) {
		dev_err(this->dev, "failed to request slave channel\n");
		return -ENODEV;
	}

	return 0;
}

static void qcom_nandc_unalloc(struct qcom_nandc_data *this)
{
	dma_release_channel(this->chan);
}

static int qcom_nandc_init(struct qcom_nandc_data *this)
{
	struct mtd_info *mtd = &this->mtd;
	struct nand_chip *chip = &this->chip;
	struct device_node *np = this->dev->of_node;
	struct mtd_part_parser_data ppdata = { .of_node = np };
	int r;

	mtd->priv = chip;
	mtd->name = "qcom-nandc";
	mtd->owner = THIS_MODULE;

	chip->priv = this;

	chip->cmdfunc		= qcom_nandc_command;
	chip->select_chip	= qcom_nandc_select_chip;
	chip->read_byte		= qcom_nandc_read_byte;
	chip->read_buf		= qcom_nandc_read_buf;
	chip->write_buf		= qcom_nandc_write_buf;

	chip->options |= NAND_NO_SUBPAGE_WRITE | NAND_USE_BOUNCE_BUFFER;
	if (this->bus_width == 16)
		chip->options |= NAND_BUSWIDTH_16;

	chip->bbt_options = NAND_BBT_ACCESS_BBM_RAW;
	if (of_get_nand_on_flash_bbt(np))
		chip->bbt_options = NAND_BBT_USE_FLASH | NAND_BBT_NO_OOB;

	qcom_nandc_pre_init(this);

	r = nand_scan_ident(mtd, 1, NULL);
	if (r)
		return r;

	r = qcom_nandc_ecc_init(this);
	if (r)
		return r;

	qcom_nandc_hw_post_init(this);

	r = nand_scan_tail(mtd);
	if (r)
		return r;

	return mtd_device_parse_register(mtd, NULL, &ppdata, NULL, 0);
}

static int qcom_nandc_parse_dt(struct platform_device *pdev)
{
	struct qcom_nandc_data *this = platform_get_drvdata(pdev);
	struct device_node *np = this->dev->of_node;
	int r;

	this->ecc_strength = of_get_nand_ecc_strength(np);
	if (this->ecc_strength < 0) {
		dev_warn(this->dev,
			"incorrect ecc strength, setting to 4 bits/step\n");
		this->ecc_strength = 4;
	}

	this->bus_width = of_get_nand_bus_width(np);
	if (this->bus_width < 0) {
		dev_warn(this->dev, "incorrect bus width, setting to 8\n");
		this->bus_width = 8;
	}

	r = of_property_read_u32(np, "qcom,cmd-crci", &this->cmd_crci);
	if (r) {
		dev_err(this->dev, "command CRCI unspecified\n");
		return r;
	}

	r = of_property_read_u32(np, "qcom,data-crci", &this->data_crci);
	if (r) {
		dev_err(this->dev, "data CRCI unspecified\n");
		return r;
	}

	return 0;
}

static int qcom_nandc_probe(struct platform_device *pdev)
{
	struct qcom_nandc_data *this;
	const struct of_device_id *match;
	int r;

	this = devm_kzalloc(&pdev->dev, sizeof(*this), GFP_KERNEL);
	if (!this)
		return -ENOMEM;

	platform_set_drvdata(pdev, this);

	this->pdev = pdev;
	this->dev  = &pdev->dev;

	match = of_match_device(pdev->dev.driver->of_match_table, &pdev->dev);
	if (!match) {
		dev_err(&pdev->dev, "failed to match device\n");
		return -ENODEV;
	}

	if (!match->data) {
		dev_err(&pdev->dev, "failed to get device data\n");
		return -ENODEV;
	}

	this->ecc_modes = (u32) match->data;

	this->res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	this->base = devm_ioremap_resource(&pdev->dev, this->res);
	if (IS_ERR(this->base))
		return PTR_ERR(this->base);

	this->core_clk = devm_clk_get(&pdev->dev, "core");
	if (IS_ERR(this->core_clk))
		return PTR_ERR(this->core_clk);

	this->aon_clk = devm_clk_get(&pdev->dev, "aon");
	if (IS_ERR(this->aon_clk))
		return PTR_ERR(this->aon_clk);

	r = qcom_nandc_parse_dt(pdev);
	if (r)
		return r;

	r = qcom_nandc_alloc(this);
	if (r)
		return r;

	r = clk_prepare_enable(this->core_clk);
	if (r)
		goto err_core_clk;

	r = clk_prepare_enable(this->aon_clk);
	if (r)
		goto err_aon_clk;

	r = qcom_nandc_init(this);
	if (r)
		goto err_init;

	return 0;

err_init:
	clk_disable_unprepare(this->aon_clk);
err_aon_clk:
	clk_disable_unprepare(this->core_clk);
err_core_clk:
	qcom_nandc_unalloc(this);

	return r;
}

static int qcom_nandc_remove(struct platform_device *pdev)
{
	struct qcom_nandc_data *this = platform_get_drvdata(pdev);

	qcom_nandc_unalloc(this);

	clk_disable_unprepare(this->aon_clk);
	clk_disable_unprepare(this->core_clk);

	return 0;
}

#define EBI2_NANDC_ECC_MODES	(ECC_RS_4BIT | ECC_BCH_8BIT)

/*
 * data will hold a struct pointer containing more differences once we support
 * more IPs
 */
static const struct of_device_id qcom_nandc_of_match[] = {
	{	.compatible = "qcom,ebi2-nandc",
		.data = (void *) EBI2_NANDC_ECC_MODES,
	},
	{}
};
MODULE_DEVICE_TABLE(of, qcom_nandc_of_match);

static struct platform_driver qcom_nandc_driver = {
	.driver = {
		.name = "qcom-nandc",
		.of_match_table = qcom_nandc_of_match,
	},
	.probe   = qcom_nandc_probe,
	.remove  = qcom_nandc_remove,
};
module_platform_driver(qcom_nandc_driver);

MODULE_AUTHOR("Archit Taneja <architt@codeaurora.org>");
MODULE_DESCRIPTION("Qualcomm NAND Controller driver");
MODULE_LICENSE("GPL v2");
