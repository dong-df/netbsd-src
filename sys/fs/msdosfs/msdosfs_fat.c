/*	$NetBSD: msdosfs_fat.c,v 1.37 2024/05/13 00:24:19 msaitoh Exp $	*/

/*-
 * Copyright (C) 1994, 1995, 1997 Wolfgang Solfrank.
 * Copyright (C) 1994, 1995, 1997 TooLs GmbH.
 * All rights reserved.
 * Original code by Paul Popelka (paulp@uts.amdahl.com) (see below).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by TooLs GmbH.
 * 4. The name of TooLs GmbH may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY TOOLS GMBH ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TOOLS GMBH BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Written by Paul Popelka (paulp@uts.amdahl.com)
 *
 * You can do anything you want with this software, just don't say you wrote
 * it, and don't remove this notice.
 *
 * This software is provided "as is".
 *
 * The author supplies this software to be publicly redistributed on the
 * understanding that the author is not responsible for the correct
 * functioning of this software in any circumstances and is not liable for
 * any damages caused by this software.
 *
 * October 1992
 */

#if HAVE_NBTOOL_CONFIG_H
#include "nbtool_config.h"
#endif

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: msdosfs_fat.c,v 1.37 2024/05/13 00:24:19 msaitoh Exp $");

/*
 * kernel include files.
 */
#include <sys/param.h>
#include <sys/file.h>
#ifdef _KERNEL
#include <sys/mount.h>		/* to define statvfs structure */
#include <sys/errno.h>
#include <sys/systm.h>
#include <sys/kauth.h>
#include <sys/dirent.h>
#include <sys/namei.h>
#include <sys/buf.h>
#include <sys/vnode.h>		/* to define vattr structure */
#else
#include <strings.h>
#include <ffs/buf.h>
#endif

/*
 * msdosfs include files.
 */
#include <fs/msdosfs/bpb.h>
#include <fs/msdosfs/msdosfsmount.h>
#include <fs/msdosfs/direntry.h>
#include <fs/msdosfs/denode.h>
#include <fs/msdosfs/fat.h>

/*
 * Fat cache stats.
 */
int fc_fileextends;		/* # of file extends			 */
int fc_lfcempty;		/* # of time last file cluster cache entry
				 * was empty */
int fc_bmapcalls;		/* # of times pcbmap was called		 */

#define	LMMAX	20
int fc_lmdistance[LMMAX];	/* counters for how far off the last
				 * cluster mapped entry was. */
int fc_largedistance;		/* off by more than LMMAX		 */
int fc_wherefrom, fc_whereto, fc_lastclust;
int pm_fatblocksize;

#ifdef MSDOSFS_DEBUG
#define DPRINTF(a) printf a
#else
#define DPRINTF(a)
#endif
#ifdef MSDOSFS_DEBUG
void print_fat_stats(void);

void
print_fat_stats(void)
{
	int i;

	printf("fc_fileextends=%d fc_lfcempty=%d fc_bmapcalls=%d "
	    "fc_largedistance=%d [%d->%d=%d] fc_lastclust=%d pm_fatblocksize=%d\n",
	    fc_fileextends, fc_lfcempty, fc_bmapcalls, fc_largedistance,
	    fc_wherefrom, fc_whereto, fc_whereto-fc_wherefrom,
	    fc_lastclust, pm_fatblocksize);
	
	fc_fileextends = fc_lfcempty = fc_bmapcalls = 0;
	fc_wherefrom = fc_whereto = fc_lastclust = 0;
    
	for (i = 0; i < LMMAX; i++) {
		printf("%d:%d ", i, fc_lmdistance[i]);
	fc_lmdistance[i] = 0;
	}

	printf("\n");
}
#endif

static void fatblock(struct msdosfsmount *, u_long, u_long *, u_long *,
			  u_long *);
void updatefats(struct msdosfsmount *, struct buf *, u_long);
static inline void usemap_free(struct msdosfsmount *, u_long);
static inline void usemap_alloc(struct msdosfsmount *, u_long);
static int fatchain(struct msdosfsmount *, u_long, u_long, u_long);
int chainlength(struct msdosfsmount *, u_long, u_long);
int chainalloc(struct msdosfsmount *, u_long, u_long, u_long, u_long *,
		    u_long *);

static void
fatblock(struct msdosfsmount *pmp, u_long ofs, u_long *bnp, u_long *sizep, u_long *bop)
{
	u_long bn, size;

	bn = ofs / pmp->pm_fatblocksize * pmp->pm_fatblocksec;
	size = uimin(pmp->pm_fatblocksec, pmp->pm_FATsecs - bn)
	    * pmp->pm_BytesPerSec;
	bn += pmp->pm_fatblk + pmp->pm_curfat * pmp->pm_FATsecs;

	DPRINTF(("%s(ofs=%lu bn=%lu, size=%lu, bo=%lu)\n", __func__, ofs, bn,
	    size, ofs % pmp->pm_fatblocksize));
	if (bnp)
		*bnp = bn;
	if (sizep)
		*sizep = size;
	if (bop)
		*bop = ofs % pmp->pm_fatblocksize;

	pm_fatblocksize = pmp->pm_fatblocksize;
}

/*
 * Map the logical cluster number of a file into a physical disk sector
 * that is filesystem relative.
 *
 * dep	  - address of denode representing the file of interest
 * findcn - file relative cluster whose filesystem relative cluster number
 *	    and/or block number are/is to be found
 * bnp	  - address of where to place the file system relative block number.
 *	    If this pointer is null then don't return this quantity.
 * cnp	  - address of where to place the file system relative cluster number.
 *	    If this pointer is null then don't return this quantity.
 *
 * NOTE: Either bnp or cnp must be non-null.
 * This function has one side effect.  If the requested file relative cluster
 * is beyond the end of file, then the actual number of clusters in the file
 * is returned in *cnp.  This is useful for determining how long a directory is.
 *  If cnp is null, nothing is returned.
 */
int
msdosfs_pcbmap(struct denode *dep,
    u_long findcn,	/* file relative cluster to get */
    daddr_t *bnp,	/* returned filesys rel sector number */
    u_long *cnp,	/* returned cluster number */
    int *sp)		/* returned block size */
{
	int error;
	u_long i;
	u_long cn;
	u_long prevcn = 0; /* XXX: prevcn could be used unititialized */
	u_long byteoffset;
	u_long bn;
	u_long bo;
	struct buf *bp = NULL;
	u_long bp_bn = -1;
	struct msdosfsmount *pmp = dep->de_pmp;
	u_long bsize;

	fc_bmapcalls++;

	/*
	 * If they don't give us someplace to return a value then don't
	 * bother doing anything.
	 */
	if (bnp == NULL && cnp == NULL && sp == NULL)
		return (0);

	cn = dep->de_StartCluster;
	DPRINTF(("%s(start cluster=%lu)\n", __func__, cn));
	/*
	 * The "file" that makes up the root directory is contiguous,
	 * permanently allocated, of fixed size, and is not made up of
	 * clusters.  If the cluster number is beyond the end of the root
	 * directory, then return the number of clusters in the file.
	 */
	if (cn == MSDOSFSROOT) {
		if (dep->de_Attributes & ATTR_DIRECTORY) {
			if (de_cn2off(pmp, findcn) >= dep->de_FileSize) {
				if (cnp)
					*cnp = de_bn2cn(pmp, pmp->pm_rootdirsize);
				DPRINTF(("%s(root, %lu ETOOBIG)\n", __func__,
				    de_cn2off(pmp, findcn)));
				return (E2BIG);
			}
			if (bnp)
				*bnp = pmp->pm_rootdirblk + de_cn2bn(pmp, findcn);
			if (cnp)
				*cnp = MSDOSFSROOT;
			if (sp)
				*sp = uimin(pmp->pm_bpcluster,
				    dep->de_FileSize - de_cn2off(pmp, findcn));
			DPRINTF(("%s(root, bn=%lu, cn=%u)\n", __func__,
			    pmp->pm_rootdirblk + de_cn2bn(pmp, findcn),
			    MSDOSFSROOT));
			return (0);
		} else {		/* just an empty file */
			if (cnp)
				*cnp = 0;
			DPRINTF(("%s(root, empty ETOOBIG)\n", __func__));
			return (E2BIG);
		}
	}

	/*
	 * All other files do I/O in cluster sized blocks
	 */
	if (sp)
		*sp = pmp->pm_bpcluster;

	/*
	 * Rummage around in the FAT cache, maybe we can avoid tromping
	 * thru every FAT entry for the file. And, keep track of how far
	 * off the cache was from where we wanted to be.
	 */
	i = 0;
	msdosfs_fc_lookup(dep, findcn, &i, &cn);
	DPRINTF(("%s(bpcluster=%lu i=%lu cn=%lu\n", __func__, pmp->pm_bpcluster,
	    i, cn));
	if ((bn = findcn - i) >= LMMAX) {
		fc_largedistance++;
		fc_wherefrom = i;
		fc_whereto = findcn;
		fc_lastclust = dep->de_fc[FC_LASTFC].fc_frcn;
	} else
		fc_lmdistance[bn]++;

	/*
	 * Handle all other files or directories the normal way.
	 */
	for (; i < findcn; i++) {
		/*
		 * Stop with all reserved clusters, not just with EOF.
		 */
		if (cn >= (CLUST_RSRVD & pmp->pm_fatmask))
			goto hiteof;

		/*
		 * Also stop when cluster is not in the filesystem
		 */
		if (cn < CLUST_FIRST || cn > pmp->pm_maxcluster) {
			DPRINTF(("%s(cn, %lu not in %lu..%lu)\n", __func__,
				cn, (u_long)CLUST_FIRST, pmp->pm_maxcluster));
			if (bp)
				brelse(bp, 0);
			return (EINVAL);
		}

		byteoffset = FATOFS(pmp, cn);
		fatblock(pmp, byteoffset, &bn, &bsize, &bo);
		if (bn != bp_bn) {
			if (bp)
				brelse(bp, 0);
			error = bread(pmp->pm_devvp, de_bn2kb(pmp, bn), bsize,
			    0, &bp);
			if (error) {
				DPRINTF(("%s(bread, %d)\n", __func__, error));
				return (error);
			}
			bp_bn = bn;
		}
		prevcn = cn;
		if (bo >= bsize) {
			if (bp)
				brelse(bp, 0);
			DPRINTF(("%s(block, %lu >= %lu)\n", __func__, bo,
			    bsize));
			return (EIO);
		}
		KASSERT(bp != NULL);
		if (FAT32(pmp))
			cn = getulong((char *)bp->b_data + bo);
		else
			cn = getushort((char *)bp->b_data + bo);
		if (FAT12(pmp) && (prevcn & 1))
			cn >>= 4;
		DPRINTF(("%s(cn=%lu masked=%lu)\n", __func__, cn,
		    cn & pmp->pm_fatmask));
		cn &= pmp->pm_fatmask;
	}

	if (!MSDOSFSEOF(cn, pmp->pm_fatmask)) {
		if (bp)
			brelse(bp, 0);
		if (bnp)
			*bnp = cntobn(pmp, cn);
		if (cnp)
			*cnp = cn;
		DPRINTF(("%s(bn=%lu, cn=%lu)\n", __func__, cntobn(pmp, cn),
		    cn));
		fc_setcache(dep, FC_LASTMAP, i, cn);
		return (0);
	}

hiteof:;
	if (cnp)
		*cnp = i;
	if (bp)
		brelse(bp, 0);
	/* update last file cluster entry in the FAT cache */
	fc_setcache(dep, FC_LASTFC, i - 1, prevcn);
	DPRINTF(("%s(eof, %lu)\n", __func__, i));
	return (E2BIG);
}

/*
 * Find the closest entry in the FAT cache to the cluster we are looking
 * for.
 */
void
msdosfs_fc_lookup(struct denode *dep, u_long findcn, u_long *frcnp,
    u_long *fsrcnp)
{
	int i;
	u_long cn;
	struct fatcache *closest = 0;

	for (i = 0; i < FC_SIZE; i++) {
		cn = dep->de_fc[i].fc_frcn;
		if (cn != FCE_EMPTY && cn <= findcn) {
			if (closest == 0 || cn > closest->fc_frcn)
				closest = &dep->de_fc[i];
		}
	}
	if (closest) {
		*frcnp = closest->fc_frcn;
		*fsrcnp = closest->fc_fsrcn;
	}
}

/*
 * Purge the FAT cache in denode dep of all entries relating to file
 * relative cluster frcn and beyond.
 */
void
msdosfs_fc_purge(struct denode *dep, u_int frcn)
{
	int i;
	struct fatcache *fcp;

	fcp = dep->de_fc;
	for (i = 0; i < FC_SIZE; i++, fcp++) {
		if (fcp->fc_frcn >= frcn)
			fcp->fc_frcn = FCE_EMPTY;
	}
}

/*
 * Update the FAT.
 * If mirroring the FAT, update all copies, with the first copy as last.
 * Else update only the current FAT (ignoring the others).
 *
 * pmp	 - msdosfsmount structure for filesystem to update
 * bp	 - addr of modified FAT block
 * fatbn - block number relative to begin of filesystem of the modified FAT block.
 */
void
updatefats(struct msdosfsmount *pmp, struct buf *bp, u_long fatbn)
{
	int i, error;
	struct buf *bpn;

	DPRINTF(("%s(pmp %p, bp %p, fatbn %lu)\n", __func__, pmp, bp, fatbn));

	/*
	 * If we have an FSInfo block, update it.
	 */
	if (pmp->pm_fsinfo) {
		u_long cn = pmp->pm_nxtfree;

		if (pmp->pm_freeclustercount
		    && (pmp->pm_inusemap[cn / N_INUSEBITS]
			& (1U << (cn % N_INUSEBITS)))) {
			/*
			 * The cluster indicated in FSInfo isn't free
			 * any longer.  Got get a new free one.
			 */
			for (cn = 0; cn < pmp->pm_maxcluster; cn++)
				if (pmp->pm_inusemap[cn / N_INUSEBITS] != (u_int)-1)
					break;
			pmp->pm_nxtfree = cn
				+ ffs(pmp->pm_inusemap[cn / N_INUSEBITS]
				      ^ (u_int)-1) - 1;
		}
		/*
		 * XXX  If the fsinfo block is stored on media with
		 *      2KB or larger sectors, is the fsinfo structure
		 *      padded at the end or in the middle?
		 */
		if (bread(pmp->pm_devvp, de_bn2kb(pmp, pmp->pm_fsinfo),
		    pmp->pm_BytesPerSec, B_MODIFY, &bpn) != 0) {
			/*
			 * Ignore the error, but turn off FSInfo update for the future.
			 */
			pmp->pm_fsinfo = 0;
		} else {
			struct fsinfo *fp = (struct fsinfo *)bpn->b_data;

			putulong(fp->fsinfree, pmp->pm_freeclustercount);
			putulong(fp->fsinxtfree, pmp->pm_nxtfree);
			if (pmp->pm_flags & MSDOSFSMNT_WAITONFAT)
				bwrite(bpn);
			else
				bdwrite(bpn);
		}
	}

	if (pmp->pm_flags & MSDOSFS_FATMIRROR) {
		/*
		 * Now copy the block(s) of the modified FAT to the other copies of
		 * the FAT and write them out.  This is faster than reading in the
		 * other FATs and then writing them back out.  This could tie up
		 * the FAT for quite a while. Preventing others from accessing it.
		 * To prevent us from going after the FAT quite so much we use
		 * delayed writes, unless they specified "synchronous" when the
		 * filesystem was mounted.  If synch is asked for then use
		 * bwrite()'s and really slow things down.
		 */
		for (i = 1; i < pmp->pm_FATs; i++) {
			fatbn += pmp->pm_FATsecs;
			/* getblk() never fails */
			bpn = getblk(pmp->pm_devvp, de_bn2kb(pmp, fatbn),
			    bp->b_bcount, 0, 0);
			memcpy(bpn->b_data, bp->b_data, bp->b_bcount);
			if (pmp->pm_flags & MSDOSFSMNT_WAITONFAT) {
				error = bwrite(bpn);
				if (error)
					printf("%s: copy FAT %d (error=%d)\n",
						 __func__, i, error);
			} else
				bdwrite(bpn);
		}
	}

	/*
	 * Write out the first (or current) FAT last.
	 */
	if (pmp->pm_flags & MSDOSFSMNT_WAITONFAT) {
		error =  bwrite(bp);
		if (error)
			printf("%s: write FAT (error=%d)\n",
				__func__, error);
	} else
		bdwrite(bp);
	/*
	 * Maybe update fsinfo sector here?
	 */
}

/*
 * Updating entries in 12 bit FATs is a pain in the butt.
 *
 * The following picture shows where nibbles go when moving from a 12 bit
 * cluster number into the appropriate bytes in the FAT.
 *
 *	byte m        byte m+1      byte m+2
 *	+----+----+   +----+----+   +----+----+
 *	|  0    1 |   |  2    3 |   |  4    5 |   FAT bytes
 *	+----+----+   +----+----+   +----+----+
 *
 *	+----+----+----+   +----+----+----+
 *	|  3    0    1 |   |  4    5    2 |
 *	+----+----+----+   +----+----+----+
 *	cluster n  	   cluster n+1
 *
 * Where n is even. m = n + (n >> 2)
 *
 */
static inline void
usemap_alloc(struct msdosfsmount *pmp, u_long cn)
{

	pmp->pm_inusemap[cn / N_INUSEBITS] |= 1U << (cn % N_INUSEBITS);
	pmp->pm_freeclustercount--;
}

static inline void
usemap_free(struct msdosfsmount *pmp, u_long cn)
{

	pmp->pm_freeclustercount++;
	pmp->pm_inusemap[cn / N_INUSEBITS] &= ~(1U << (cn % N_INUSEBITS));
}

int
msdosfs_clusterfree(struct msdosfsmount *pmp, u_long cluster, u_long *oldcnp)
{
	int error;
	u_long oldcn;

	usemap_free(pmp, cluster);
	error = msdosfs_fatentry(FAT_GET_AND_SET, pmp, cluster, &oldcn,
	    MSDOSFSFREE);
	if (error) {
		usemap_alloc(pmp, cluster);
		return (error);
	}
	/*
	 * If the cluster was successfully marked free, then update
	 * the count of free clusters, and turn off the "allocated"
	 * bit in the "in use" cluster bit map.
	 */
	if (oldcnp)
		*oldcnp = oldcn;
	return (0);
}

/*
 * Get or Set or 'Get and Set' the cluster'th entry in the FAT.
 *
 * function	- whether to get or set a fat entry
 * pmp		- address of the msdosfsmount structure for the filesystem
 *		  whose FAT is to be manipulated.
 * cn		- which cluster is of interest
 * oldcontents	- address of a word that is to receive the contents of the
 *		  cluster'th entry if this is a get function
 * newcontents	- the new value to be written into the cluster'th element of
 *		  the FAT if this is a set function.
 *
 * This function can also be used to free a cluster by setting the FAT entry
 * for a cluster to 0.
 *
 * All copies of the FAT are updated if this is a set function. NOTE: If
 * fatentry() marks a cluster as free it does not update the inusemap in
 * the msdosfsmount structure. This is left to the caller.
 */
int
msdosfs_fatentry(int function, struct msdosfsmount *pmp, u_long cn,
    u_long *oldcontents, u_long newcontents)
{
	int error;
	u_long readcn;
	u_long bn, bo, bsize, byteoffset;
	struct buf *bp;

	DPRINTF(("%s(func %d, pmp %p, clust %lu, oldcon %p, newcon " "%lx)\n",
	    __func__, function, pmp, cn, oldcontents, newcontents));

#ifdef DIAGNOSTIC
	/*
	 * Be sure they asked us to do something.
	 */
	if ((function & (FAT_SET | FAT_GET)) == 0) {
		DPRINTF(("%s(): function code doesn't specify get or set\n",
		    __func__));
		return (EINVAL);
	}

	/*
	 * If they asked us to return a cluster number but didn't tell us
	 * where to put it, give them an error.
	 */
	if ((function & FAT_GET) && oldcontents == NULL) {
		DPRINTF(("%s(): get function with no place to put result\n",
			__func__));
		return (EINVAL);
	}
#endif

	/*
	 * Be sure the requested cluster is in the filesystem.
	 */
	if (cn < CLUST_FIRST || cn > pmp->pm_maxcluster)
		return (EINVAL);

	byteoffset = FATOFS(pmp, cn);
	fatblock(pmp, byteoffset, &bn, &bsize, &bo);
	if ((error = bread(pmp->pm_devvp, de_bn2kb(pmp, bn), bsize,
	    0, &bp)) != 0) {
		return (error);
	}

	if (function & FAT_GET) {
		if (FAT32(pmp))
			readcn = getulong((char *)bp->b_data + bo);
		else
			readcn = getushort((char *)bp->b_data + bo);
		if (FAT12(pmp) & (cn & 1))
			readcn >>= 4;
		readcn &= pmp->pm_fatmask;
		*oldcontents = readcn;
	}
	if (function & FAT_SET) {
		switch (pmp->pm_fatmask) {
		case FAT12_MASK:
			readcn = getushort((char *)bp->b_data + bo);
			if (cn & 1) {
				readcn &= 0x000f;
				readcn |= newcontents << 4;
			} else {
				readcn &= 0xf000;
				readcn |= newcontents & 0xfff;
			}
			putushort((char *)bp->b_data + bo, readcn);
			break;
		case FAT16_MASK:
			putushort((char *)bp->b_data + bo, newcontents);
			break;
		case FAT32_MASK:
			/*
			 * According to spec we have to retain the
			 * high order bits of the FAT entry.
			 */
			readcn = getulong((char *)bp->b_data + bo);
			readcn &= ~FAT32_MASK;
			readcn |= newcontents & FAT32_MASK;
			putulong((char *)bp->b_data + bo, readcn);
			break;
		}
		updatefats(pmp, bp, bn);
		bp = NULL;
		pmp->pm_fmod = 1;
	}
	if (bp)
		brelse(bp, 0);
	return (0);
}

/*
 * Update a contiguous cluster chain
 *
 * pmp	    - mount point
 * start    - first cluster of chain
 * count    - number of clusters in chain
 * fillwith - what to write into FAT entry of last cluster
 */
static int
fatchain(struct msdosfsmount *pmp, u_long start, u_long count, u_long fillwith)
{
	int error;
	u_long bn, bo, bsize, byteoffset, readcn, newc;
	struct buf *bp;

	DPRINTF(("%s(pmp %p, start %lu, count %lu, fillwith %lx)\n", __func__,
	    pmp, start, count, fillwith));
	/*
	 * Be sure the clusters are in the filesystem.
	 */
	if (start < CLUST_FIRST || start + count - 1 > pmp->pm_maxcluster)
		return (EINVAL);

	while (count > 0) {
		byteoffset = FATOFS(pmp, start);
		fatblock(pmp, byteoffset, &bn, &bsize, &bo);
		error = bread(pmp->pm_devvp, de_bn2kb(pmp, bn), bsize,
		    B_MODIFY, &bp);
		if (error) {
			return (error);
		}
		while (count > 0) {
			start++;
			newc = --count > 0 ? start : fillwith;
			switch (pmp->pm_fatmask) {
			case FAT12_MASK:
				readcn = getushort((char *)bp->b_data + bo);
				if (start & 1) {
					readcn &= 0xf000;
					readcn |= newc & 0xfff;
				} else {
					readcn &= 0x000f;
					readcn |= newc << 4;
				}
				putushort((char *)bp->b_data + bo, readcn);
				bo++;
				if (!(start & 1))
					bo++;
				break;
			case FAT16_MASK:
				putushort((char *)bp->b_data + bo, newc);
				bo += 2;
				break;
			case FAT32_MASK:
				readcn = getulong((char *)bp->b_data + bo);
				readcn &= ~pmp->pm_fatmask;
				readcn |= newc & pmp->pm_fatmask;
				putulong((char *)bp->b_data + bo, readcn);
				bo += 4;
				break;
			}
			if (bo >= bsize)
				break;
		}
		updatefats(pmp, bp, bn);
	}
	pmp->pm_fmod = 1;
	return (0);
}

/*
 * Check the length of a free cluster chain starting at start.
 *
 * pmp	 - mount point
 * start - start of chain
 * count - maximum interesting length
 */
int
chainlength(struct msdosfsmount *pmp, u_long start, u_long count)
{
	u_long idx, max_idx;
	u_int map;
	u_long len;

	max_idx = pmp->pm_maxcluster / N_INUSEBITS;
	idx = start / N_INUSEBITS;
	start %= N_INUSEBITS;
	map = pmp->pm_inusemap[idx];
	map &= ~((1U << start) - 1);
	if (map) {
		len = ffs(map) - 1 - start;
		return (len > count ? count : len);
	}
	len = N_INUSEBITS - start;
	if (len >= count)
		return (count);
	while (++idx <= max_idx) {
		if (len >= count)
			break;
		if ((map = pmp->pm_inusemap[idx]) != 0) {
			len +=  ffs(map) - 1;
			break;
		}
		len += N_INUSEBITS;
	}
	return (len > count ? count : len);
}

/*
 * Allocate contiguous free clusters.
 *
 * pmp	      - mount point.
 * start      - start of cluster chain.
 * count      - number of clusters to allocate.
 * fillwith   - put this value into the FAT entry for the
 *		last allocated cluster.
 * retcluster - put the first allocated cluster's number here.
 * got	      - how many clusters were actually allocated.
 */
int
chainalloc(struct msdosfsmount *pmp, u_long start, u_long count, u_long fillwith, u_long *retcluster, u_long *got)
{
	int error;
	u_long cl, n;

	for (cl = start, n = count; n-- > 0;)
		usemap_alloc(pmp, cl++);
	if ((error = fatchain(pmp, start, count, fillwith)) != 0)
		return (error);

	DPRINTF(("%s(): allocated cluster chain at %lu (%lu clusters)\n",
	    __func__, start, count));
	if (retcluster)
		*retcluster = start;
	if (got)
		*got = count;
	return (0);
}

/*
 * Allocate contiguous free clusters.
 *
 * pmp	      - mount point.
 * start      - preferred start of cluster chain.
 * count      - number of clusters requested.
 * fillwith   - put this value into the FAT entry for the
 *		last allocated cluster.
 * retcluster - put the first allocated cluster's number here.
 * got	      - how many clusters were actually allocated.
 */
int
msdosfs_clusteralloc(struct msdosfsmount *pmp, u_long start, u_long count,
    u_long *retcluster, u_long *got)
{
	u_long idx;
	u_long len, newst, foundl, cn, l;
	u_long foundcn = 0; /* XXX: foundcn could be used unititialized */
	u_long fillwith = CLUST_EOFE;
	u_int map;

	DPRINTF(("%s(): find %lu clusters\n", __func__, count));
	if (start) {
		if ((len = chainlength(pmp, start, count)) >= count)
			return (chainalloc(pmp, start, count, fillwith, retcluster, got));
	} else {
		/*
		 * This is a new file, initialize start
		 */
		struct timeval tv;

		microtime(&tv);
		start = (tv.tv_usec >> 10) | tv.tv_usec;
		len = 0;
	}

	/*
	 * Start at a (pseudo) random place to maximize cluster runs
	 * under multiple writers.
	 */
	newst = (start * 1103515245 + 12345) % (pmp->pm_maxcluster + 1);
	foundl = 0;

	for (cn = newst; cn <= pmp->pm_maxcluster;) {
		idx = cn / N_INUSEBITS;
		map = pmp->pm_inusemap[idx];
		map |= (1U << (cn % N_INUSEBITS)) - 1;
		if (map != (u_int)-1) {
			cn = idx * N_INUSEBITS + ffs(map^(u_int)-1) - 1;
			if ((l = chainlength(pmp, cn, count)) >= count)
				return (chainalloc(pmp, cn, count, fillwith, retcluster, got));
			if (l > foundl) {
				foundcn = cn;
				foundl = l;
			}
			cn += l + 1;
			continue;
		}
		cn += N_INUSEBITS - cn % N_INUSEBITS;
	}
	for (cn = 0; cn < newst;) {
		idx = cn / N_INUSEBITS;
		map = pmp->pm_inusemap[idx];
		map |= (1U << (cn % N_INUSEBITS)) - 1;
		if (map != (u_int)-1) {
			cn = idx * N_INUSEBITS + ffs(map^(u_int)-1) - 1;
			if ((l = chainlength(pmp, cn, count)) >= count)
				return (chainalloc(pmp, cn, count, fillwith, retcluster, got));
			if (l > foundl) {
				foundcn = cn;
				foundl = l;
			}
			cn += l + 1;
			continue;
		}
		cn += N_INUSEBITS - cn % N_INUSEBITS;
	}

	if (!foundl)
		return (ENOSPC);

	if (len)
		return (chainalloc(pmp, start, len, fillwith, retcluster, got));
	else
		return (chainalloc(pmp, foundcn, foundl, fillwith, retcluster, got));
}


/*
 * Free a chain of clusters.
 *
 * pmp		- address of the msdosfs mount structure for the filesystem
 *		  containing the cluster chain to be freed.
 * startcluster - number of the 1st cluster in the chain of clusters to be
 *		  freed.
 */
int
msdosfs_freeclusterchain(struct msdosfsmount *pmp, u_long cluster)
{
	int error;
	struct buf *bp = NULL;
	u_long bn, bo, bsize, byteoffset;
	u_long readcn, lbn = -1;

	bn = 0; /* XXXgcc */
	while (cluster >= CLUST_FIRST && cluster <= pmp->pm_maxcluster) {
		byteoffset = FATOFS(pmp, cluster);
		fatblock(pmp, byteoffset, &bn, &bsize, &bo);
		if (lbn != bn) {
			if (bp)
				updatefats(pmp, bp, lbn);
			error = bread(pmp->pm_devvp, de_bn2kb(pmp, bn), bsize,
			    B_MODIFY, &bp);
			if (error) {
				return (error);
			}
			lbn = bn;
		}
		usemap_free(pmp, cluster);
		KASSERT(bp != NULL);
		switch (pmp->pm_fatmask) {
		case FAT12_MASK:
			readcn = getushort((char *)bp->b_data + bo);
			if (cluster & 1) {
				cluster = readcn >> 4;
				readcn &= 0x000f;
				readcn |= MSDOSFSFREE << 4;
			} else {
				cluster = readcn;
				readcn &= 0xf000;
				readcn |= MSDOSFSFREE & 0xfff;
			}
			putushort((char *)bp->b_data + bo, readcn);
			break;
		case FAT16_MASK:
			cluster = getushort((char *)bp->b_data + bo);
			putushort((char *)bp->b_data + bo, MSDOSFSFREE);
			break;
		case FAT32_MASK:
			cluster = getulong((char *)bp->b_data + bo);
			putulong((char *)bp->b_data + bo,
				 (MSDOSFSFREE & FAT32_MASK) | (cluster & ~FAT32_MASK));
			break;
		}
		cluster &= pmp->pm_fatmask;
	}
	if (bp)
		updatefats(pmp, bp, bn);
	return (0);
}

/*
 * Read in FAT blocks looking for free clusters. For every free cluster
 * found turn off its corresponding bit in the pm_inusemap.
 */
int
msdosfs_fillinusemap(struct msdosfsmount *pmp)
{
	struct buf *bp = NULL;
	u_long cn, readcn;
	int error;
	u_long bn, bo, bsize, byteoffset;

	/*
	 * Mark all clusters in use, we mark the free ones in the FAT scan
	 * loop further down.
	 */
	for (cn = 0; cn < (pmp->pm_maxcluster + N_INUSEBITS) / N_INUSEBITS; cn++)
		pmp->pm_inusemap[cn] = (u_int)-1;

	/*
	 * Figure how many free clusters are in the filesystem by ripping
	 * through the FAT counting the number of entries whose content is
	 * zero.  These represent free clusters.
	 */
	pmp->pm_freeclustercount = 0;
	for (cn = CLUST_FIRST; cn <= pmp->pm_maxcluster; cn++) {
		byteoffset = FATOFS(pmp, cn);
		bo = byteoffset % pmp->pm_fatblocksize;
		if (!bo || !bp) {
			/* Read new FAT block */
			if (bp)
				brelse(bp, 0);
			fatblock(pmp, byteoffset, &bn, &bsize, NULL);
			error = bread(pmp->pm_devvp, de_bn2kb(pmp, bn), bsize,
			    0, &bp);
			if (error) {
				return (error);
			}
		}
		if (FAT32(pmp))
			readcn = getulong((char *)bp->b_data + bo);
		else
			readcn = getushort((char *)bp->b_data + bo);
		if (FAT12(pmp) && (cn & 1))
			readcn >>= 4;
		readcn &= pmp->pm_fatmask;

		if (readcn == 0)
			usemap_free(pmp, cn);
	}
	if (bp)
		brelse(bp, 0);
	return (0);
}

/*
 * Allocate a new cluster and chain it onto the end of the file.
 *
 * dep	 - the file to extend
 * count - number of clusters to allocate
 * bpp	 - where to return the address of the buf header for the first new
 *	   file block
 * ncp	 - where to put cluster number of the first newly allocated cluster
 *	   If this pointer is 0, do not return the cluster number.
 * flags - see fat.h
 *
 * NOTE: This function is not responsible for turning on the DE_UPDATE bit of
 * the de_flag field of the denode and it does not change the de_FileSize
 * field.  This is left for the caller to do.
 */

int
msdosfs_extendfile(struct denode *dep, u_long count, struct buf **bpp,
    u_long *ncp, int flags)
{
	int error;
	u_long frcn = 0, cn, got;
	struct msdosfsmount *pmp = dep->de_pmp;
	struct buf *bp;

	/*
	 * Don't try to extend the root directory
	 */
	if (dep->de_StartCluster == MSDOSFSROOT
	    && (dep->de_Attributes & ATTR_DIRECTORY)) {
		DPRINTF(("%s(): attempt to extend root directory\n", __func__));
		return (ENOSPC);
	}

	/*
	 * If the "file's last cluster" cache entry is empty, and the file
	 * is not empty, then fill the cache entry by calling pcbmap().
	 */
	fc_fileextends++;
	if (dep->de_fc[FC_LASTFC].fc_frcn == FCE_EMPTY &&
	    dep->de_StartCluster != 0) {
		fc_lfcempty++;
		error = msdosfs_pcbmap(dep, CLUST_END, 0, &cn, 0);
		/* we expect it to return E2BIG */
		if (error != E2BIG)
			return (error);
	}

	fc_last_to_nexttolast(dep);

	while (count > 0) {

		/*
		 * Allocate a new cluster chain and cat onto the end of the
		 * file.  If the file is empty we make de_StartCluster point
		 * to the new block.  Note that de_StartCluster being 0 is
		 * sufficient to be sure the file is empty since we exclude
		 * attempts to extend the root directory above, and the root
		 * dir is the only file with a startcluster of 0 that has
		 * blocks allocated (sort of).
		 */

		if (dep->de_StartCluster == 0)
			cn = 0;
		else
			cn = dep->de_fc[FC_LASTFC].fc_fsrcn + 1;
		error = msdosfs_clusteralloc(pmp, cn, count, &cn, &got);
		if (error)
			return (error);

		count -= got;

		/*
		 * Give them the filesystem relative cluster number if they want
		 * it.
		 */
		if (ncp) {
			*ncp = cn;
			ncp = NULL;
		}

		if (dep->de_StartCluster == 0) {
			dep->de_StartCluster = cn;
			frcn = 0;
		} else {
			error = msdosfs_fatentry(FAT_SET, pmp,
					 dep->de_fc[FC_LASTFC].fc_fsrcn,
					 0, cn);
			if (error) {
				msdosfs_clusterfree(pmp, cn, NULL);
				return (error);
			}
			frcn = dep->de_fc[FC_LASTFC].fc_frcn + 1;
		}

		/*
		 * Update the "last cluster of the file" entry in the
		 * denode's FAT cache.
		 */

		fc_setcache(dep, FC_LASTFC, frcn + got - 1, cn + got - 1);
		if ((flags & DE_CLEAR) &&
		    (dep->de_Attributes & ATTR_DIRECTORY)) {
			while (got-- > 0) {
				bp = getblk(pmp->pm_devvp,
				    de_bn2kb(pmp, cntobn(pmp, cn++)),
				    pmp->pm_bpcluster, 0, 0);
				clrbuf(bp);
				if (bpp) {
					*bpp = bp;
						bpp = NULL;
				} else {
					bdwrite(bp);
				}
			}
		}
	}

	return (0);
}
