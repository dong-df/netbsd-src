/*	$NetBSD: sdmmcreg.h,v 1.35 2024/10/18 11:03:52 jmcneill Exp $	*/
/*	$OpenBSD: sdmmcreg.h,v 1.4 2009/01/09 10:55:22 jsg Exp $	*/

/*
 * Copyright (c) 2006 Uwe Stuehler <uwe@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef	_SDMMCREG_H_
#define	_SDMMCREG_H_

/* MMC commands */				/* response type */
#define MMC_GO_IDLE_STATE		0	/* R0 */
#define MMC_SEND_OP_COND		1	/* R3 */
#define MMC_ALL_SEND_CID		2	/* R2 */
#define MMC_SET_RELATIVE_ADDR 	  	3	/* R1 */
#define MMC_SWITCH			6	/* R1b */
#define MMC_SELECT_CARD			7	/* R1 */
#define MMC_SEND_EXT_CSD		8	/* R1 */
#define MMC_SEND_CSD			9	/* R2 */
#define MMC_SEND_CID			10	/* R2 */
#define MMC_STOP_TRANSMISSION		12	/* R1b */
#define MMC_SEND_STATUS			13	/* R1 */
#define MMC_INACTIVE_STATE		15	/* R0 */
#define MMC_SET_BLOCKLEN		16	/* R1 */
#define MMC_READ_BLOCK_SINGLE		17	/* R1 */
#define MMC_READ_BLOCK_MULTIPLE		18	/* R1 */
#define MMC_SEND_TUNING_BLOCK		19	/* R1 */
#define MMC_SEND_TUNING_BLOCK_HS200	21	/* R1 */
#define MMC_SET_BLOCK_COUNT		23	/* R1 */
#define MMC_WRITE_BLOCK_SINGLE		24	/* R1 */
#define MMC_WRITE_BLOCK_MULTIPLE	25	/* R1 */
#define MMC_PROGRAM_CSD			27	/* R1 */
#define MMC_SET_WRITE_PROT		28	/* R1b */
#define MMC_SET_CLR_WRITE_PROT		29	/* R1b */
#define MMC_SET_SEND_WRITE_PROT		30	/* R1 */
#define MMC_TAG_SECTOR_START		32	/* R1 */
#define MMC_TAG_SECTOR_END		33	/* R1 */
#define MMC_UNTAG_SECTOR		34	/* R1 */
#define MMC_TAG_ERASE_GROUP_START	35	/* R1 */
#define MMC_TAG_ERASE_GROUP_END		36	/* R1 */
#define MMC_UNTAG_ERASE_GROUP		37	/* R1 */
#define MMC_ERASE			38	/* R1b */
#define MMC_LOCK_UNLOCK			42	/* R1b */
#define MMC_APP_CMD			55	/* R1 */
#define MMC_READ_OCR			58	/* R3 */

/* SD commands */			/* response type */
#define SD_SEND_RELATIVE_ADDR 	  	3	/* R6 */
#define SD_SEND_SWITCH_FUNC		6	/* R1 */
#define SD_SEND_IF_COND			8	/* R7 */
#define SD_VOLTAGE_SWITCH		11	/* R1 */
#define SD_ERASE_WR_BLK_START		32	/* R1 */
#define SD_ERASE_WR_BLK_END		33	/* R1 */
#define SD_READ_EXTR_SINGLE		48	/* R1 */
#define SD_WRITE_EXTR_SINGLE		49	/* R1 */

/* SD application commands */			/* response type */
#define SD_APP_SET_BUS_WIDTH		6	/* R1 */
#define SD_APP_SD_STATUS		13	/* R1 */
#define SD_APP_SET_WR_BLK_ERASE_COUNT	23	/* R1 */
#define SD_APP_OP_COND			41	/* R3 */
#define SD_APP_SEND_SCR			51	/* R1 */

/* SD extended register argument fields */
#define SD_EXTR_MIO			(0x1U << 31)
#define SD_EXTR_MIO_MEM			0
#define SD_EXTR_MIO_IO			1
#define SD_EXTR_FNO			(0xfU << 27)
#define SD_EXTR_MW			(1U << 26)	/* write only */
#define SD_EXTR_ADDR			(0x1ffff << 9)
#define SD_EXTR_LEN			0x1ff

/* SD erase arguments */
#define SD_ERASE_DISCARD		0x00000001
#define SD_ERASE_FULE			0x00000002

/* OCR bits */
#define MMC_OCR_MEM_READY		(1U<<31)/* memory power-up status bit */
#define MMC_OCR_HCS			(1<<30)	/* SD only */
#define MMC_OCR_ACCESS_MODE_MASK	(3<<29)	/* MMC only */
#define MMC_OCR_ACCESS_MODE_BYTE	(0<<29)	/* MMC only */
#define MMC_OCR_ACCESS_MODE_SECTOR	(2<<29)	/* MMC only */
#define MMC_OCR_S18A			(1<<24)
#define MMC_OCR_3_5V_3_6V		(1<<23)
#define MMC_OCR_3_4V_3_5V		(1<<22)
#define MMC_OCR_3_3V_3_4V		(1<<21)
#define MMC_OCR_3_2V_3_3V		(1<<20)
#define MMC_OCR_3_1V_3_2V		(1<<19)
#define MMC_OCR_3_0V_3_1V		(1<<18)
#define MMC_OCR_2_9V_3_0V		(1<<17)
#define MMC_OCR_2_8V_2_9V		(1<<16)
#define MMC_OCR_2_7V_2_8V		(1<<15)
#define MMC_OCR_2_6V_2_7V		(1<<14)
#define MMC_OCR_2_5V_2_6V		(1<<13)
#define MMC_OCR_2_4V_2_5V		(1<<12)
#define MMC_OCR_2_3V_2_4V		(1<<11)
#define MMC_OCR_2_2V_2_3V		(1<<10)
#define MMC_OCR_2_1V_2_2V		(1<<9)
#define MMC_OCR_2_0V_2_1V		(1<<8)
#define MMC_OCR_1_65V_1_95V		(1<<7)

/* R1 response type bits */
#define MMC_R1_READY_FOR_DATA		(1<<8)	/* ready for next transfer */
#define MMC_R1_SWITCH_ERROR		(1<<7)	/* switch command failed */
#define MMC_R1_APP_CMD			(1<<5)	/* app. commands supported */

/* 48-bit response decoding (32 bits w/o CRC) */
#define MMC_R1(resp)			((resp)[0])
#define MMC_R3(resp)			((resp)[0])
#define SD_R6(resp)			((resp)[0])
#define MMC_R7(resp)			((resp)[0])
#define MMC_SPI_R1(resp)		((resp)[0])
#define MMC_SPI_R7(resp)		((resp)[1])

/* RCA argument and response */
#define MMC_ARG_RCA(rca)		((rca) << 16)
#define SD_R6_RCA(resp)			(SD_R6((resp)) >> 16)

/* bus width argument */
#define SD_ARG_BUS_WIDTH_1		0
#define SD_ARG_BUS_WIDTH_4		2

/* EXT_CSD fields */
#define EXT_CSD_FLUSH_CACHE		32	/* W/E_P */
#define EXT_CSD_CACHE_CTRL		33	/* R/W/E_P */
#define EXT_CSD_RST_N_FUNCTION		162	/* R/W */
#define EXT_CSD_BUS_WIDTH		183	/* W/E_P */
#define EXT_CSD_HS_TIMING		185	/* R/W/E_P */
#define EXT_CSD_REV			192	/* R */
#define EXT_CSD_STRUCTURE		194	/* R */
#define EXT_CSD_CARD_TYPE		196	/* R */
#define EXT_CSD_SEC_COUNT		212	/* R */
#define EXT_CSD_CACHE_SIZE		249	/* R (4 bytes) */

/* EXT_CSD field definitions */
#define EXT_CSD_CMD_SET_NORMAL		(1U << 0)
#define EXT_CSD_CMD_SET_SECURE		(1U << 1)
#define EXT_CSD_CMD_SET_CPSECURE	(1U << 2)

/* EXT_CSD_FLUSH_CACHE */
#define EXT_CSD_FLUSH_CACHE_FLUSH	(1U << 0)
#define EXT_CSD_FLUSH_CACHE_BARRIER	(1U << 1)

/* EXT_CSD_CACHE_CTRL */
#define EXT_CSD_CACHE_CTRL_CACHE_EN	(1U << 0)

/* EXT_CSD_BUS_WIDTH */
#define EXT_CSD_BUS_WIDTH_1		0	/* 1 bit mode */
#define EXT_CSD_BUS_WIDTH_4		1	/* 4 bit mode */
#define EXT_CSD_BUS_WIDTH_8		2	/* 8 bit mode */
#define EXT_CSD_BUS_WIDTH_4_DDR		5	/* 4 bit mode (DDR) */
#define EXT_CSD_BUS_WIDTH_8_DDR		6	/* 8 bit mode (DDR) */

/* EXT_CSD_HS_TIMING */
#define EXT_CSD_HS_TIMING_LEGACY	0
#define EXT_CSD_HS_TIMING_HIGHSPEED	1
#define EXT_CSD_HS_TIMING_HS200		2
#define EXT_CSD_HS_TIMING_HS400		3

/* EXT_CSD_STRUCTURE */
#define EXT_CSD_STRUCTURE_VER_1_0	0	/* CSD Version No.1.0 */
#define EXT_CSD_STRUCTURE_VER_1_1	1	/* CSD Version No.1.1 */
#define EXT_CSD_STRUCTURE_VER_1_2	2	/* Version 4.1-4.2-4.3 */

/* EXT_CSD_CARD_TYPE */
#define EXT_CSD_CARD_TYPE_F_26M		(1 << 0) /* HS 26 MHz */
#define EXT_CSD_CARD_TYPE_F_52M		(1 << 1) /* HS 52 MHz */
#define EXT_CSD_CARD_TYPE_F_DDR52_1_8V	(1 << 2) /* HS DDR 52 MHz 1.8V or 3V */
#define EXT_CSD_CARD_TYPE_F_DDR52_1_2V	(1 << 3) /* HS DDR 52 MHz 1.2V */
#define EXT_CSD_CARD_TYPE_F_HS200_1_8V	(1 << 4) /* HS200 SDR 200 MHz 1.8V */
#define EXT_CSD_CARD_TYPE_F_HS200_1_2V	(1 << 5) /* HS200 SDR 200 MHz 1.2V */
#define EXT_CSD_CARD_TYPE_F_HS400_1_8V	(1 << 6) /* HS400 DDR 200 MHz 1.8V */
#define EXT_CSD_CARD_TYPE_F_HS400_1_2V	(1 << 7) /* HS400 DDR 200 MHz 1.2V */

/* EXT_CSD_RST_N_FUNCTION */
#define	EXT_CSD_RST_N_TMP_DISABLED	0x00
#define	EXT_CSD_RST_N_PERM_ENABLED	0x01
#define	EXT_CSD_RST_N_PERM_DISABLED	0x02
#define	EXT_CSD_RST_N_MASK		0x03

/* MMC_SWITCH access mode */
#define MMC_SWITCH_MODE_CMD_SET		0x00	/* Change the command set */
#define MMC_SWITCH_MODE_SET_BITS	0x01	/* Set bits in value */
#define MMC_SWITCH_MODE_CLEAR_BITS	0x02	/* Clear bits in value */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03	/* Set target to value */

/* SPI mode reports R1/R2(SEND_STATUS) status. */
#define R1_SPI_IDLE		(1 << 0)
#define R1_SPI_ERASE_RESET	(1 << 1)
#define R1_SPI_ILLEGAL_COMMAND	(1 << 2)
#define R1_SPI_COM_CRC		(1 << 3)
#define R1_SPI_ERASE_SEQ	(1 << 4)
#define R1_SPI_ADDRESS		(1 << 5)
#define R1_SPI_PARAMETER	(1 << 6)
/* R1 bit 7 is always zero */
#define R2_SPI_CARD_LOCKED	(1 << 8)
#define R2_SPI_WP_ERASE_SKIP	(1 << 9)	/* or lock/unlock fail */
#define R2_SPI_LOCK_UNLOCK_FAIL	R2_SPI_WP_ERASE_SKIP
#define R2_SPI_ERROR		(1 << 10)
#define R2_SPI_CC_ERROR		(1 << 11)
#define R2_SPI_CARD_ECC_ERROR	(1 << 12)
#define R2_SPI_WP_VIOLATION	(1 << 13)
#define R2_SPI_ERASE_PARAM	(1 << 14)
#define R2_SPI_OUT_OF_RANGE	(1 << 15)	/* or CSD overwrite */
#define R2_SPI_CSD_OVERWRITE	R2_SPI_OUT_OF_RANGE

/* MMC R2 response (CSD) */
#define MMC_CSD_CSDVER(resp)		MMC_RSP_BITS((resp), 126, 2)
#define  MMC_CSD_CSDVER_1_0		0
#define  MMC_CSD_CSDVER_1_1		1
#define  MMC_CSD_CSDVER_1_2		2 /* MMC 4.1 - 4.2 - 4.3 */
#define  MMC_CSD_CSDVER_EXT_CSD		3 /* Version is coded in CSD_STRUCTURE in EXT_CSD */
#define MMC_CSD_MMCVER(resp)		MMC_RSP_BITS((resp), 122, 4)
#define  MMC_CSD_MMCVER_1_0		0 /* MMC 1.0 - 1.2 */
#define  MMC_CSD_MMCVER_1_4		1 /* MMC 1.4 */
#define  MMC_CSD_MMCVER_2_0		2 /* MMC 2.0 - 2.2 */
#define  MMC_CSD_MMCVER_3_1		3 /* MMC 3.1 - 3.3 */
#define  MMC_CSD_MMCVER_4_0		4 /* MMC 4.1 - 4.2 - 4.3 */
#define MMC_CSD_TAAC(resp)		MMC_RSP_BITS((resp), 112, 8)
#define MMC_CSD_TAAC_MANT(resp)		MMC_RSP_BITS((resp), 115, 4)
#define MMC_CSD_TAAC_EXP(resp)		MMC_RSP_BITS((resp), 112, 3)
#define MMC_CSD_NSAC(resp)		MMC_RSP_BITS((resp), 104, 8)
#define MMC_CSD_TRAN_SPEED(resp)	MMC_RSP_BITS((resp), 96, 8)
#define MMC_CSD_TRAN_SPEED_MANT(resp)	MMC_RSP_BITS((resp), 99, 4)
#define MMC_CSD_TRAN_SPEED_EXP(resp)	MMC_RSP_BITS((resp), 96, 3)
#define MMC_CSD_READ_BL_LEN(resp)	MMC_RSP_BITS((resp), 80, 4)
#define MMC_CSD_C_SIZE(resp)		MMC_RSP_BITS((resp), 62, 12)
#define MMC_CSD_CAPACITY(resp)		((MMC_CSD_C_SIZE((resp))+1) << \
					 (MMC_CSD_C_SIZE_MULT((resp))+2))
#define MMC_CSD_C_SIZE_MULT(resp)	MMC_RSP_BITS((resp), 47, 3)
#define MMC_CSD_R2W_FACTOR(resp)	MMC_RSP_BITS((resp), 26, 3)
#define MMC_CSD_WRITE_BL_LEN(resp)	MMC_RSP_BITS((resp), 22, 4)

/* MMC v1 R2 response (CID) */
#define MMC_CID_MID_V1(resp)		MMC_RSP_BITS((resp), 104, 24)
#define MMC_CID_PNM_V1_CPY(resp, pnm)					\
	do {								\
		(pnm)[0] = MMC_RSP_BITS((resp), 96, 8);			\
		(pnm)[1] = MMC_RSP_BITS((resp), 88, 8);			\
		(pnm)[2] = MMC_RSP_BITS((resp), 80, 8);			\
		(pnm)[3] = MMC_RSP_BITS((resp), 72, 8);			\
		(pnm)[4] = MMC_RSP_BITS((resp), 64, 8);			\
		(pnm)[5] = MMC_RSP_BITS((resp), 56, 8);			\
		(pnm)[6] = MMC_RSP_BITS((resp), 48, 8);			\
		(pnm)[7] = '\0';					\
	} while (/*CONSTCOND*/0)
#define MMC_CID_REV_V1(resp)		MMC_RSP_BITS((resp), 40, 8)
#define MMC_CID_PSN_V1(resp)		MMC_RSP_BITS((resp), 16, 24)
#define MMC_CID_MDT_V1(resp)		MMC_RSP_BITS((resp), 8, 8)

/* MMC v2 R2 response (CID) */
#define MMC_CID_MID_V2(resp)		MMC_RSP_BITS((resp), 120, 8)
#define MMC_CID_OID_V2(resp)		MMC_RSP_BITS((resp), 104, 16)
#define MMC_CID_PNM_V2_CPY(resp, pnm)					\
	do {								\
		(pnm)[0] = MMC_RSP_BITS((resp), 96, 8);			\
		(pnm)[1] = MMC_RSP_BITS((resp), 88, 8);			\
		(pnm)[2] = MMC_RSP_BITS((resp), 80, 8);			\
		(pnm)[3] = MMC_RSP_BITS((resp), 72, 8);			\
		(pnm)[4] = MMC_RSP_BITS((resp), 64, 8);			\
		(pnm)[5] = MMC_RSP_BITS((resp), 56, 8);			\
		(pnm)[6] = '\0';					\
	} while (/*CONSTCOND*/0)
#define MMC_CID_PSN_V2(resp)		MMC_RSP_BITS((resp), 16, 32)

/* SD R2 response (CSD) */
#define SD_CSD_CSDVER(resp)		MMC_RSP_BITS((resp), 126, 2)
#define  SD_CSD_CSDVER_1_0		0
#define  SD_CSD_CSDVER_2_0		1
#define SD_CSD_MMCVER(resp)		MMC_RSP_BITS((resp), 122, 4)
#define SD_CSD_TAAC(resp)		MMC_RSP_BITS((resp), 112, 8)
#define SD_CSD_TAAC_EXP(resp)		MMC_RSP_BITS((resp), 115, 4)
#define SD_CSD_TAAC_MANT(resp)		MMC_RSP_BITS((resp), 112, 3)
#define  SD_CSD_TAAC_1_5_MSEC		0x26
#define SD_CSD_NSAC(resp)		MMC_RSP_BITS((resp), 104, 8)
#define SD_CSD_SPEED(resp)		MMC_RSP_BITS((resp), 96, 8)
#define SD_CSD_SPEED_MANT(resp)		MMC_RSP_BITS((resp), 99, 4)
#define SD_CSD_SPEED_EXP(resp)		MMC_RSP_BITS((resp), 96, 3)
#define  SD_CSD_SPEED_25_MHZ		0x32
#define  SD_CSD_SPEED_50_MHZ		0x5a
#define SD_CSD_CCC(resp)		MMC_RSP_BITS((resp), 84, 12)
#define  SD_CSD_CCC_BASIC		(1 << 0)	/* basic */
#define  SD_CSD_CCC_BR			(1 << 2)	/* block read */
#define  SD_CSD_CCC_BW			(1 << 4)	/* block write */
#define  SD_CSD_CCC_ERACE		(1 << 5)	/* erase */
#define  SD_CSD_CCC_WP			(1 << 6)	/* write protection */
#define  SD_CSD_CCC_LC			(1 << 7)	/* lock card */
#define  SD_CSD_CCC_AS			(1 << 8)	/*application specific*/
#define  SD_CSD_CCC_IOM			(1 << 9)	/* I/O mode */
#define  SD_CSD_CCC_SWITCH		(1 << 10)	/* switch */
#define SD_CSD_READ_BL_LEN(resp)	MMC_RSP_BITS((resp), 80, 4)
#define SD_CSD_READ_BL_PARTIAL(resp)	MMC_RSP_BITS((resp), 79, 1)
#define SD_CSD_WRITE_BLK_MISALIGN(resp)	MMC_RSP_BITS((resp), 78, 1)
#define SD_CSD_READ_BLK_MISALIGN(resp)	MMC_RSP_BITS((resp), 77, 1)
#define SD_CSD_DSR_IMP(resp)		MMC_RSP_BITS((resp), 76, 1)
#define SD_CSD_C_SIZE(resp)		MMC_RSP_BITS((resp), 62, 12)
#define SD_CSD_CAPACITY(resp)		((SD_CSD_C_SIZE((resp))+1) << \
					 (SD_CSD_C_SIZE_MULT((resp))+2))
#define SD_CSD_VDD_R_CURR_MIN(resp)	MMC_RSP_BITS((resp), 59, 3)
#define SD_CSD_VDD_R_CURR_MAX(resp)	MMC_RSP_BITS((resp), 56, 3)
#define SD_CSD_VDD_W_CURR_MIN(resp)	MMC_RSP_BITS((resp), 53, 3)
#define SD_CSD_VDD_W_CURR_MAX(resp)	MMC_RSP_BITS((resp), 50, 3)
#define  SD_CSD_VDD_RW_CURR_100mA	0x7
#define  SD_CSD_VDD_RW_CURR_80mA	0x6
#define SD_CSD_V2_C_SIZE(resp)		MMC_RSP_BITS((resp), 48, 22)
#define SD_CSD_V2_CAPACITY(resp)	((SD_CSD_V2_C_SIZE((resp))+1) << 10)
#define SD_CSD_V2_BL_LEN		0x9 /* 512 */
#define SD_CSD_C_SIZE_MULT(resp)	MMC_RSP_BITS((resp), 47, 3)
#define SD_CSD_ERASE_BLK_EN(resp)	MMC_RSP_BITS((resp), 46, 1)
#define SD_CSD_SECTOR_SIZE(resp)	MMC_RSP_BITS((resp), 39, 7) /* +1 */
#define SD_CSD_WP_GRP_SIZE(resp)	MMC_RSP_BITS((resp), 32, 7) /* +1 */
#define SD_CSD_WP_GRP_ENABLE(resp)	MMC_RSP_BITS((resp), 31, 1)
#define SD_CSD_R2W_FACTOR(resp)		MMC_RSP_BITS((resp), 26, 3)
#define SD_CSD_WRITE_BL_LEN(resp)	MMC_RSP_BITS((resp), 22, 4)
#define  SD_CSD_RW_BL_LEN_2G		0xa
#define  SD_CSD_RW_BL_LEN_1G		0x9
#define SD_CSD_WRITE_BL_PARTIAL(resp)	MMC_RSP_BITS((resp), 21, 1)
#define SD_CSD_FILE_FORMAT_GRP(resp)	MMC_RSP_BITS((resp), 15, 1)
#define SD_CSD_COPY(resp)		MMC_RSP_BITS((resp), 14, 1)
#define SD_CSD_PERM_WRITE_PROTECT(resp)	MMC_RSP_BITS((resp), 13, 1)
#define SD_CSD_TMP_WRITE_PROTECT(resp)	MMC_RSP_BITS((resp), 12, 1)
#define SD_CSD_FILE_FORMAT(resp)	MMC_RSP_BITS((resp), 10, 2)

/* SD R2 response (CID) */
#define SD_CID_MID(resp)		MMC_RSP_BITS((resp), 120, 8)
#define SD_CID_OID(resp)		MMC_RSP_BITS((resp), 104, 16)
#define SD_CID_PNM_CPY(resp, pnm)					\
	do {								\
		(pnm)[0] = MMC_RSP_BITS((resp), 96, 8);			\
		(pnm)[1] = MMC_RSP_BITS((resp), 88, 8);			\
		(pnm)[2] = MMC_RSP_BITS((resp), 80, 8);			\
		(pnm)[3] = MMC_RSP_BITS((resp), 72, 8);			\
		(pnm)[4] = MMC_RSP_BITS((resp), 64, 8);			\
		(pnm)[5] = '\0';					\
	} while (/*CONSTCOND*/0)
#define SD_CID_REV(resp)		MMC_RSP_BITS((resp), 56, 8)
#define SD_CID_PSN(resp)		MMC_RSP_BITS((resp), 24, 32)
#define SD_CID_MDT(resp)		MMC_RSP_BITS((resp), 8, 12)

/* SCR (SD Configuration Register) */
#define SCR_STRUCTURE(scr)		MMC_RSP_BITS((scr), 60, 4)
#define  SCR_STRUCTURE_VER_1_0		0 /* Version 1.0 */
#define SCR_SD_SPEC(scr)		MMC_RSP_BITS((scr), 56, 4)
#define  SCR_SD_SPEC_VER_1_0		0 /* Version 1.0 and 1.01 */
#define  SCR_SD_SPEC_VER_1_10		1 /* Version 1.10 */
#define  SCR_SD_SPEC_VER_2		2 /* Version 2.00 or Version 3.0X */
#define SCR_DATA_STAT_AFTER_ERASE(scr)	MMC_RSP_BITS((scr), 55, 1)
#define SCR_SD_SECURITY(scr)		MMC_RSP_BITS((scr), 52, 3)
#define  SCR_SD_SECURITY_NONE		0 /* no security */
#define  SCR_SD_SECURITY_1_0		1 /* security protocol 1.0 */
#define  SCR_SD_SECURITY_1_0_2		2 /* security protocol 1.0 */
#define SCR_SD_BUS_WIDTHS(scr)		MMC_RSP_BITS((scr), 48, 4)
#define  SCR_SD_BUS_WIDTHS_1BIT		(1 << 0) /* 1bit (DAT0) */
#define  SCR_SD_BUS_WIDTHS_4BIT		(1 << 2) /* 4bit (DAT0-3) */
#define SCR_SD_SPEC3(scr)		MMC_RSP_BITS((scr), 47, 1)
#define SCR_EX_SECURITY(scr)		MMC_RSP_BITS((scr), 43, 4)
#define SCR_SD_SPEC4(scr)		MMC_RSP_BITS((scr), 42, 1)
#define SCR_RESERVED(scr)		MMC_RSP_BITS((scr), 34, 9)
#define SCR_CMD_SUPPORT_CMD48(scr)	MMC_RSP_BITS((scr), 34, 1)
#define SCR_CMD_SUPPORT_CMD23(scr)	MMC_RSP_BITS((scr), 33, 1)
#define SCR_CMD_SUPPORT_CMD20(scr)	MMC_RSP_BITS((scr), 32, 1)
#define SCR_RESERVED2(scr)		MMC_RSP_BITS((scr), 0, 32)

/* SSR (SD Status Register) */
#define SSR_DAT_BUS_WIDTH(resp)		__bitfield((resp), 510, 2)
#define  SSR_DAT_BUS_WIDTH_1		0
#define  SSR_DAT_BUS_WIDTH_4		2
#define SSR_SECURED_MODE(resp)		__bitfield((resp), 509, 1)
#define SSR_SD_CARD_TYPE(resp)		__bitfield((resp), 480, 16)
#define  SSR_SD_CARD_TYPE_RDWR		0
#define  SSR_SD_CARD_TYPE_ROM		1
#define  SSR_SD_CARD_TYPE_OTP		2
#define SSR_SIZE_OF_PROTECTED_AREA(resp) __bitfield((resp), 448, 32)
#define SSR_SPEED_CLASS(resp)		__bitfield((resp), 440, 8)
#define  SSR_SPEED_CLASS_0		0
#define  SSR_SPEED_CLASS_2		1
#define  SSR_SPEED_CLASS_4		2
#define  SSR_SPEED_CLASS_6		3
#define  SSR_SPEED_CLASS_10		4
#define SSR_PERFORMANCE_MOVE(resp)	__bitfield((resp), 432, 8)
#define SSR_AU_SIZE(resp)		__bitfield((resp), 428, 4)
#define SSR_ERASE_SIZE(resp)		__bitfield((resp), 408, 16)
#define SSR_ERASE_TIMEOUT(resp)		__bitfield((resp), 402, 6)
#define SSR_ERASE_OFFSET(resp)		__bitfield((resp), 400, 2)
#define SSR_UHS_SPEED_GRADE(resp)	__bitfield((resp), 396, 4)
#define  SSR_UHS_SPEED_GRADE_10MB	1
#define  SSR_UHS_SPEED_GRADE_30MB	3
#define SSR_UHS_AU_SIZE(resp)		__bitfield((resp), 392, 4)
#define SSR_VIDEO_SPEED_CLASS(resp)	__bitfield((resp), 384, 8)
#define SSR_VSC_AU_SIZE(resp)		__bitfield((resp), 368, 10)
#define SSR_SUS_ADDR(resp)		__bitfield((resp), 346, 22)
#define SSR_APP_PERF_CLASS(resp)	__bitfield((resp), 336, 4)
#define  SSR_APP_PERF_CLASS_UNSUPPORTED	0
#define  SSR_APP_PERF_CLASS_A1		1
#define  SSR_APP_PERF_CLASS_A2		2
#define SSR_PERFORMANCE_ENHANCE(resp)	__bitfield((resp), 328, 8)
#define  SSR_PERFORMANCE_ENHANCE_CACHE		__BIT(0)
#define  SSR_PERFORMANCE_ENHANCE_HOST_MAINT	__BIT(1)
#define  SSR_PERFORMANCE_ENHANCE_CARD_MAINT	__BIT(2)
#define  SSR_PERFORMANCE_ENHANCE_COMMAND_QUEUE	__BITS(7,3)
#define SSR_DISCARD_SUPPORT(resp)	__bitfield((resp), 313, 1)
#define SSR_FULE_SUPPORT(resp)		__bitfield((resp), 312, 1)

/* Status of Switch Function */
#define SFUNC_STATUS_GROUP(status, group) \
	(__bitfield((uint32_t *)(status), 400 + (group - 1) * 16, 16))

#define SD_ACCESS_MODE_SDR12	0
#define SD_ACCESS_MODE_SDR25	1
#define SD_ACCESS_MODE_SDR50	2
#define SD_ACCESS_MODE_SDR104	3
#define SD_ACCESS_MODE_DDR50	4

/* SD extension data */
#define SD_GENERAL_INFO_HDR_REVISION(data)	le16dec(&(data)[0])
#define SD_GENERAL_INFO_HDR_LENGTH(data)	le16dec(&(data)[2])
#define SD_GENERAL_INFO_HDR_NUM_EXT(data)	((data)[4])
#define SD_GENERAL_INFO_EXT_FIRST		16
#define SD_EXTENSION_INFO_SFC(data, index)	le16dec(&(data)[(index) + 0])
#define SD_EXTENSION_INFO_NEXT(data, index)	le16dec(&(data)[(index) + 40])
#define	SD_EXTENSION_INFO_NUM_REG(data, index)	((data)[(index) + 42])
#define SD_EXTENSION_INFO_REG(data, index, num)	le32dec(&(data)[(index) + 44 + (num) * 4])

#define SD_EXTENSION_INFO_REG_FNO(addr)		(((addr) >> 18) & 0xf)
#define SD_EXTENSION_INFO_REG_START_ADDR(addr)	((addr) & 0x1ffff)

/* SD extension standard function codes */
#define SD_SFC_PEF	0x0002

/* Performance Enhancement Function */
#define SD_PEF_CACHE_SUPPORT(data)		((data)[4] & 0x01)
#define SD_PEF_CACHE_ENABLE(data)		((data)[260] & 0x01)
#define SD_PEF_CACHE_FLUSH(data)		((data)[261] & 0x01)
#define SD_PEF_CACHE_ENABLE_OFFSET		260
#define SD_PEF_CACHE_FLUSH_OFFSET		261

/* This assumes the response fields are in host byte order in 32-bit units.  */
#define MMC_RSP_BITS(resp, start, len)	__bitfield((resp), (start)-8, (len))
static __inline uint32_t
__bitfield(const uint32_t *src, size_t start, size_t len)
{
	if (start + len > 512 || len == 0 || len > 32)
		return 0;

	src += start / 32;
	start %= 32;

	uint32_t dst = src[0] >> start;

	if (__predict_false((start + len - 1) / 32 != start / 32)) {
		dst |= src[1] << (32 - start);
	}

	return dst & (__BIT(len) - 1);
}

#endif	/* _SDMMCREG_H_ */
