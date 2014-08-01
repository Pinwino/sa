#ifndef __FMC_H__
#define __FMC_H__

#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>

#define FMC_MAX_CARDS 32

extern int usleep(useconds_t usec);

struct device {}; /* Trick or treat! */

/*
 * Yeah! I know it looks bad, and 0f course it is not ellegant but if any other 
 * of the structure fields needs to be used this monstes makes the work faster
 */
struct fmc_device {
	//unsigned long version;
	//unsigned long flags;
	//struct module *owner;		/* char device must pin it */
//	struct fmc_fru_id id;		/* for EEPROM-based match */
//	struct fmc_operations *op;	/* carrier-provided */
//	int irq;			/* according to host bus. 0 == none */
	int eeprom_len;			/* Usually 8kB, may be less */
	int eeprom_addr;		/* 0x50, 0x52 etc */
	uint8_t *eeprom;		/* Full contents or leading part */
//	char *carrier_name;		/* "SPEC" or similar, for special use */
//	void *carrier_data;		/* "struct spec *" or equivalent */
//	__iomem void *fpga_base;	/* May be NULL (Etherbone) */
//	__iomem void *slot_base;	/* Set by the driver */
//	struct fmc_device **devarray;	/* Allocated by the bus */
//	int slot_id;			/* Index in the slot array */
//	int nr_slots;			/* Number of slots in this carrier */
//	unsigned long memlen;		/* Used for the char device */
	struct device dev;		/* For Linux use */
	struct device *hwdev;		/* The underlying hardware device */
//	unsigned long sdbfs_entry;
//	struct sdb_array *sdb;
//	uint32_t device_id;		/* Filled by the device */
//	char *mezzanine_name;		/* Defaults to ``fmc'' */
//	void *mezzanine_data;
};

static inline uint32_t fmc_readl(struct fmc_device *fmc, int offset)
{
	uint32_t *p= (uint32_t *) offset;
	/* Looks unncessary? Yes, it does. But for any reason it works better than
	 * usleep(x). So you'd better don't remove it 
	 */
	if (offset >= 0x480500 && offset < 0x490000)
		mprintf("[READ ]: fmc_readl -> Dir %08X val %08X\n", p, *p); 
	return *p;

}

static inline void fmc_writel(struct fmc_device *fmc, uint32_t val, int off)
{
	uint32_t *p = (uint32_t *) off;
	*p = val;
}

#endif __FMC_H__
