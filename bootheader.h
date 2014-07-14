/*
 * bootheader.h
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

#include <stdint.h>

#ifndef __BOOTHEADER__
#define __BOOTHEADER__

#define SECTOR						(512)

#define HEAD_LEN					(0x38) 
#define HEAD_PADDING					(0x1c8) 

#define IMAGE_HDR					("$OS$\0\0\1")
#define PLATFORM					(0x380101)
#define ID2_SIZE					(0x28) 

#define UNKNOWN_SIZE					(0x1E0)
#define CMDLINE_SIZE   					(0x400)
#define PADDING1_SIZE					(0x1000-0x410)
#define BOOTSTUBSTACK_SIZE				(0x3000)
#define BOOTSTUBSTACK_DELTA				(0x1000)
#define BOOTSTUBSTACK_EXTRA				(BOOTSTUBSTACK_SIZE-BOOTSTUBSTACK_DELTA)
#define CMDLINE_END						(HEAD_LEN+HEAD_PADDING+UNKNOWN_SIZE+CMDLINE_SIZE)

#define T_RECOVERY						(0xC)
#define T_FASTBOOT						(0xE)
#define T_BOOT							(0x0)

#ifndef le32toh
#  define le32toh(x) letoh32(x)
#endif

#define ERROR(...) do { fprintf(stderr, __VA_ARGS__); exit(1); } while(0)
#define MESSAGE(...) do { fprintf(stderr, __VA_ARGS__); } while(0)

const uint8_t boot_loader_hdr[4] = { 0xea, 0x05, 0x00, 0xc0 } ;

struct bootheader {
	uint8_t hdr[7];	    //Header: $OS$\0\0\1
	uint8_t xor56;     //XOR of first 56 bytes without this one
	uint8_t id2[ID2_SIZE]; //40 bytes: 01 01 38 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 10 01 00 10 10 01 
	uint32_t sectors_t; //Total image length in 512 sectors - 1
	uint32_t image_type;//Image type 0xd - recovery, 0x1 - boot, 0xf - fastboot
	
	uint8_t padding0[HEAD_PADDING];
	uint8_t unknown[UNKNOWN_SIZE];
	char cmdline[CMDLINE_SIZE];
	uint32_t bzImageSize;
	uint32_t initrdSize;
	uint32_t SPIUARTSuppression;
	uint32_t SPIType;
	uint8_t padding1[PADDING1_SIZE];
	uint8_t bootstubStack[BOOTSTUBSTACK_SIZE];
};

uint8_t calc_sum(struct bootheader * hdr);

/* Sanity check for struct size */
typedef char z[(sizeof(struct bootheader) == 0x21e0 + HEAD_LEN + HEAD_PADDING + BOOTSTUBSTACK_EXTRA) ? 1 : -1];

/* CRC calculation (move from header file if the code is used in different project) */
uint8_t calc_sum(struct bootheader * hdr) {
   uint8_t sum = hdr->xor56;
   uint8_t * data = (uint8_t *) hdr;
   int i;
   for (i=0; i < HEAD_LEN; i++) { sum ^= data[i]; } 
   return sum;	
}	

/* Boot header check (move from header file if the code is used in different project) */
void checkBootHeader(struct bootheader * hdr)
{
   if (memcmp(hdr->hdr,IMAGE_HDR, sizeof(hdr->hdr))) ERROR("CRITICAL: Invalid Image header - is it really Dell Venue? Stop.\n");
   if (hdr->xor56 != calc_sum(hdr)) MESSAGE("ERROR: Invalid checksum of original image - is it really Dell Venue?\n");
   if (hdr->image_type != T_BOOT && hdr->image_type != T_FASTBOOT && hdr->image_type != T_RECOVERY) MESSAGE("Warning: Unknown image type %02x of original image - is it really Dell Venue? Using anyway.\n", hdr->image_type);
}

int32_t bootloader_end(struct bootheader * hdr) {
  int32_t delta;
  uint8_t * bootstub = hdr->bootstubStack;
  for (delta=0; delta < BOOTSTUBSTACK_SIZE; delta += BOOTSTUBSTACK_DELTA) {
      if (!memcmp(bootstub+delta, boot_loader_hdr, sizeof(boot_loader_hdr))) {
        return BOOTSTUBSTACK_SIZE - delta;
//      } else {
//        fprintf(stderr, "Bootstub stack: %02x %02x %02x %02x\n", bootstub[delta], bootstub[delta+1], bootstub[delta+2], bootstub[delta+3]);
      }
  }
  ERROR("CRITICAL: Start of kernel image not found - is it really Dell Venue? Try increasing BOOTSTUBSTACK_SIZE. Stop.\n");
}

#endif
