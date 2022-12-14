/*
 * This file was taken directly from 
 * https://github.com/cs107e/ahconkey-JoshFrancisCodes-project/tree/master/src/lib,
 * which adapted it from a Broadcom source. See below for their references.
 * 
 * Only cosmetic changes were required to adapt it to the CA project, apart
 * from a small but costly bug where `p` in `mem_alloc()` was not 16-byte 
 * aligned. Since `mailbox.h` requires this alignment, the bug led to 
 * inconsistent performance when `p` did not happen to be 16-byte aligned
 * (and the desired values therefore were not written to the mailbox).
 * Before resolving this bug, the Pi could correctly run single GPU programs
 * but had to be fully rebooted between sending GPU programs.
 * 
 * Note that instances of `mailbox_request(MAILBOX_TAGS_ARM_TO_VC, (unsigned) p);`
 * replaced calls of `mbox_property(file_desc, p);` in the original Broadcom code.
 */

/*
Copyright (c) 2012, Broadcom Europe Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "mailbox.h" 
#include "strings.h"
#include "assert.h"

#include "../../include/mailbox_functions.h"

#define PAGE_SIZE (4*1024)

unsigned mem_alloc(int file_desc, unsigned size, unsigned align, unsigned flags)
{
   int i=0;
   unsigned p[32] __attribute__ ((aligned (16))); 
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x3000c; // (the tag id)
   p[i++] = 12; // (size of the buffer)
   p[i++] = 12; // (size of the data)
   p[i++] = size; // (num bytes? or pages?)
   p[i++] = align; // (alignment)
   p[i++] = flags; // (MEM_FLAG_L1_NONALLOCATING)

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size
       
   mailbox_request(MAILBOX_TAGS_ARM_TO_VC, (unsigned) p); //our code
   return p[5];
}

unsigned mem_free(int file_desc, unsigned handle)
{
   int i=0;
   unsigned p[32] __attribute__ ((aligned (16))); 
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x3000f; // (the tag id)
   p[i++] = 4; // (size of the buffer)
   p[i++] = 4; // (size of the data)
   p[i++] = handle;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_request(MAILBOX_TAGS_ARM_TO_VC, (unsigned) p);
   return p[5];
}

unsigned mem_lock(int file_desc, unsigned handle)
{
   int i=0;
   unsigned p[32] __attribute__ ((aligned (16)));
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x3000d; // (the tag id)
   p[i++] = 4; // (size of the buffer)
   p[i++] = 4; // (size of the data)
   p[i++] = handle;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size
   
   mailbox_request(MAILBOX_TAGS_ARM_TO_VC, (unsigned) p);
   return p[5];
}

unsigned mem_unlock(int file_desc, unsigned handle)
{
   int i=0;
   unsigned p[32] __attribute__ ((aligned (16)));
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x3000e; // (the tag id)
   p[i++] = 4; // (size of the buffer)
   p[i++] = 4; // (size of the data)
   p[i++] = handle;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_request(MAILBOX_TAGS_ARM_TO_VC, (unsigned) p);
   return p[5];
}
#define PHYS_TO_BUS(x) ((x)|0xC0000000)
unsigned execute_code(int file_desc, unsigned code, unsigned r0, unsigned r1, unsigned r2, unsigned r3, unsigned r4, unsigned r5)
{
   int i=0;
   unsigned p[32] __attribute__ ((aligned (16)));
   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x30010; // (the tag id)
   p[i++] = 28; // (size of the buffer)
   p[i++] = 28; // (size of the data)
   p[i++] = PHYS_TO_BUS(code);
   p[i++] = r0;
   p[i++] = r1;
   p[i++] = r2;
   p[i++] = r3;
   p[i++] = r4;
   p[i++] = r5;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_request(MAILBOX_TAGS_ARM_TO_VC, PHYS_TO_BUS((unsigned) p));
   return p[5];
}

unsigned qpu_enable(int file_desc, unsigned enable)
{
   int i=0;
   unsigned p[32] __attribute__ ((aligned (16)));

   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request

   p[i++] = 0x30012; // (the tag id)
   p[i++] = 4; // (size of the buffer)
   p[i++] = 4; // (size of the data)
   p[i++] = enable;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_request(MAILBOX_TAGS_ARM_TO_VC, (unsigned) p);
   return p[5];
}

unsigned execute_qpu(int file_desc, unsigned num_qpus, unsigned control, unsigned noflush, unsigned timeout) {
   int i=0;
   unsigned p[32] __attribute__ ((aligned (16)));

   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request
   p[i++] = 0x30011; // (the tag id)
   p[i++] = 16; // (size of the buffer)
   p[i++] = 16; // (size of the data)
   p[i++] = num_qpus;
   p[i++] = control;
   p[i++] = noflush;
   p[i++] = timeout; // ms

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_request(MAILBOX_TAGS_ARM_TO_VC, (unsigned) p);
   return p[5];
}

unsigned set_clock_rate(unsigned rate) {
   int i=0;
   unsigned p[32] __attribute__ ((aligned (16)));


   p[i++] = 0; // size
   p[i++] = 0x00000000; // process request
   p[i++] = 0x38002; // (the tag id)
   p[i++] = 8; // (size of the buffer)
   p[i++] = 8; // (size of the data)
   p[i++] = 5; //clock id
   p[i++] = rate;

   p[i++] = 0x00000000; // end tag
   p[0] = i*sizeof *p; // actual size

   mailbox_request(MAILBOX_TAGS_ARM_TO_VC, (unsigned) p);
   return p[5];
}

