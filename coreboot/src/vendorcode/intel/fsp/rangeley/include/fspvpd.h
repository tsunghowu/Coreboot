/** @file

Copyright (C) 2013, Intel Corporation

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this
  list of conditions and the following disclaimer in the documentation and/or
  other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors may
  be used to endorse or promote products derived from this software without
  specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.

  This file is automatically generated. Please do NOT modify !!!

**/

#ifndef __FSP_VPD_H__
#define __FSP_VPD_H__

#pragma pack(1)



typedef struct _UPD_DATA_REGION {
  UINT64                      Signature;                     /* Offset 0x0000 */
  UINT64                      Reserved;                      /* Offset 0x0008 */
  UINT8                       UnusedUpdSpace0[16];           /* Offset 0x0010 */
  UINT8                       PcdMrcInitTsegSize;            /* Offset 0x0020 */
  UINT8                       PcdMemoryDown;                 /* Offset 0x0021 */
  UINT8                       PcdMrcRmtSupport;              /* Offset 0x0022 */
  UINT8                       PcdMrcRmtCpgcExpLoopCntValue;  /* Offset 0x0023 */
  UINT8                       PcdMrcRmtCpgcNumBursts;        /* Offset 0x0024 */
  UINT8                       PcdSpdBaseAddress_0_0;         /* Offset 0x0025 */
  UINT8                       PcdSpdBaseAddress_0_1;         /* Offset 0x0026 */
  UINT8                       PcdSpdBaseAddress_1_0;         /* Offset 0x0027 */
  UINT8                       PcdSpdBaseAddress_1_1;         /* Offset 0x0028 */
  UINT8                       UnusedUpdSpace1[7];            /* Offset 0x0029 */
  UINT8                       PcdEnableLan;                  /* Offset 0x0030 */
  UINT8                       PcdEnableSata2;                /* Offset 0x0031 */
  UINT8                       PcdEnableSata3;                /* Offset 0x0032 */
  UINT8                       PcdEnableIQAT;                 /* Offset 0x0033 */
  UINT8                       PcdEnableUsb20;                /* Offset 0x0034 */
  UINT8                       PcdBifurcation;                /* Offset 0x0035 */
  UINT8                       UnusedUpdSpace2[10];           /* Offset 0x0036 */
  UINT8                       PcdPrintDebugMessages;         /* Offset 0x0040 */
  UINT8                       PcdFastboot;                   /* Offset 0x0041 */
  UINT8                       PcdEccSupport;                 /* Offset 0x0042 */
  UINT8                       UnusedUpdSpace3[13];           /* Offset 0x0043 */
  UINT16                      PcdRegionTerminator;           /* Offset 0x0050 */
} UPD_DATA_REGION;

#define VPD_IMAGE_ID    0x562D474E524E5641        /* 'AVNRNG-V' */
#define VPD_IMAGE_REV   0x00000101

typedef struct _VPD_DATA_REGION {
  UINT64                      PcdVpdRegionSign;              /* Offset 0x0000 */
  UINT32                      PcdImageRevision;              /* Offset 0x0008 */
  UINT32                      PcdUpdRegionOffset;            /* Offset 0x000C */
  UINT8                       UnusedVpdSpace0[16];           /* Offset 0x0010 */
  UINT32                      PcdFspReservedMemoryLength;    /* Offset 0x0020 */
  UINT8                       PcdSpdWriteProtect;            /* Offset 0x0024 */
} VPD_DATA_REGION;

#pragma pack()

#endif
