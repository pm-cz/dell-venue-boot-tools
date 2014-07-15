/*
 * unpack.c
 *
 * Copyright 2012 Emilio López <turl@tuxfamily.org>
 * Modified for Dell Venue by Pavel Moravec, 2014 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <endian.h>
#include <string.h>


#include "bootheader.h"


int main(int argc, char *argv[])
{
	char *origin;
	char *bzImage;
	char *ramdisk;
	char *bootloader=NULL;
	char *cmdline=NULL;
	FILE *forigin;
	FILE *fbzImage;
	FILE *framdisk;
	FILE *fother;
	uint32_t bzImageLen;
	uint32_t ramdiskLen;
	uint32_t missing;
	int32_t offset=0; //Bootloader end offset in header

	char buf[BUFSIZ];
	struct bootheader tmphdr;
	size_t size;

	if (argc < 4 || argc > 6)
		ERROR("Usage: %s <image to unpack> <bzImage out> <ramdisk out> [<bootloader out> [<cmdline out>]]\n", argv[0]);

	origin = argv[1];
	bzImage = argv[2];
	ramdisk = argv[3];
	if (argc > 4) { bootloader=argv[4]; }
	if (argc > 5) { cmdline=argv[5]; }

	forigin = fopen(origin, "rb");
	if (!forigin)
		ERROR("ERROR: failed to open input image\n");

	if (fread(&tmphdr, sizeof(tmphdr), 1, forigin) != 1)
		ERROR("ERROR: failed to read file header\n");
	checkBootHeader(&tmphdr);
	
	/* Read bzImage length from the image to unpack */
	if (fseek(forigin, CMDLINE_END, SEEK_SET) == -1)
		ERROR("ERROR: failed to seek on image\n");

	if (fread(&bzImageLen, sizeof(uint32_t), 1, forigin) != 1)
		ERROR("ERROR: failed to read bzImage length\n");
	else
		bzImageLen = le32toh(bzImageLen);

	/* Read ramdisk length from the image to unpack */
	if (fseek(forigin, (CMDLINE_END+sizeof(uint32_t)), SEEK_SET) == -1)
		ERROR("ERROR: failed to seek on image\n");

	if (fread(&ramdiskLen, sizeof(uint32_t), 1, forigin) != 1)
		ERROR("ERROR: failed to read ramdisk length\n");
	else
		ramdiskLen = le32toh(ramdiskLen);

	/* Fix different bootloader sizes */
	offset = bootloader_end(&tmphdr);
	if (bootloader) {
		fother=fopen(bootloader, "wb");
		if (!fother)
			ERROR("ERROR: failed to open bootloader image\n");
		fwrite(tmphdr.bootstubStack, 1, BOOTSTUBSTACK_SIZE-offset, fother);
		fclose(fother);
	}

	if (cmdline) {
		fother=fopen(cmdline, "w");
		if (!fother)
			ERROR("ERROR: failed to open command line info\n");
		fprintf(fother, "%s", tmphdr.cmdline);
		fclose(fother);
	}

	/* Copy bzImage */
	if (fseek(forigin, sizeof(struct bootheader)-offset, SEEK_SET) == -1)
		ERROR("ERROR: failed to seek when copying bzImage\n");

	fbzImage = fopen(bzImage, "wb");
	framdisk = fopen(ramdisk, "wb");
	if (!fbzImage || !framdisk)
		ERROR("ERROR: failed to open ramdisk or kernel image\n");

	missing = bzImageLen;
	while (missing > 0) {
		size = fread(buf, 1, ((missing > BUFSIZ) ? BUFSIZ : missing), forigin);
		if (size != 0) {
			fwrite(buf, 1, size, fbzImage);
			missing -= size;
		}
	}

	/* Copy ramdisk */
	if (fseek(forigin, (sizeof(struct bootheader)+bzImageLen-offset), SEEK_SET) == -1)
		ERROR("ERROR: failed to seek when copying ramdisk\n");

	missing = ramdiskLen;
	while (missing > 0) {
		size = fread(buf, 1, ((missing > BUFSIZ) ? BUFSIZ : missing), forigin);
		if (size != 0) {
			fwrite(buf, 1, size, framdisk);
			missing -= size;
		}
	}

	return 0;
}
