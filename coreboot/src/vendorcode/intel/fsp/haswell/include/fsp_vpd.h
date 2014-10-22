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

**/

#ifndef __VPDHEADER_H__
#define __VPDHEADER_H__

#pragma pack(1)


#define MAX_CH 2            /* Maximum Number of Memory Channels */
#define MAX_DIMM 2          /* Maximum Number of DIMMs PER Memory Channel */

typedef enum {
  MEMORY_ABSENT     = 0,    /* No memory down and no physical memory slot. */
  MEMORY_DOWN_ONLY  = 1,    /* Memory down and not a physical memory slot. */
  MEMORY_SLOT_ONLY  = 2,    /* No memory down and a physical memory slot.  */
} MemorySlotStatus;

typedef struct {
  MemorySlotStatus  SlotStatus[MAX_CH][MAX_DIMM]; /* Memory Down, Absent or Slot status of each DIMM in each Channel */
  UINT16            SpdDataLen;                   /* Length in Bytes of a single DIMM's SPD Data */
  UINT8            *SpdDataPtr[MAX_CH][MAX_DIMM]; /* Array of Pointers to SPD Data for each DIMM in each Channel */
} MEMORY_DOWN_DATA;

typedef struct {
  UINT32  VendorDeviceId;         /* Vendor ID/Device ID */
  UINT16  SubSystemId;            /* SubSystem ID */
  UINT8   RevisionId;             /* Revision ID. 0xFF applies to all steppings */
  UINT8   FrontPanelSupport;      /* Front panel support (1=yes, 2=no) */
  UINT16  NumberOfRearJacks;      /* Number of Rear Jacks */
  UINT16  NumberOfFrontJacks;     /* Number of Front Jacks */
} SA_HDA_HEADER;

typedef struct {
  UINT16  VendorId;               /* Codec Vendor ID */
  UINT16  DeviceId;               /* Codec Device ID */
  UINT8   RevisionId;             /* Revision ID of the codec. 0xFF matches any revision. */
  UINT8   SdiNo;                  /* SDI number, 0xFF matches any SDI. */
  UINT16  DataDwords;             /* Number of data DWORDs pointed by the codec data buffer. */
  UINT32  Reserved;               /* Reserved for future use. Must be set to 0. */
} AZALIA_HEADER;

typedef struct {
  SA_HDA_HEADER        Header;    /* SA HDA header */
  UINT32              *Data;      /* Pointer to the data buffer. Its length is determined by the header */
} AUDIO_SA_VERB_TABLE;

typedef struct {
  AZALIA_HEADER        Header;    /* AZALIA PCH header */
  UINT32              *Data;      /* Pointer to the data buffer. Its length is specified in the header */
} AUDIO_AZALIA_VERB_TABLE;

typedef struct _UPD_DATA_REGION {
  UINT64                      Signature;                     /* Offset 0x0000 */
  UINT64                      Reserved;                      /* Offset 0x0008 */
  UINT32                      SerialDebugPortAddress;        /* Offset 0x0010 */
  UINT8                       SerialDebugPortType;           /* Offset 0x0014 */
  UINT8                       PcdMrcDebugMsg;                /* Offset 0x0015 */
  UINT8                       UnusedUpdSpace0[10];           /* Offset 0x0016 */
  UINT8                       PcdSpdBaseAddress_0_0;         /* Offset 0x0020 */
  UINT8                       PcdSpdBaseAddress_0_1;         /* Offset 0x0021 */
  UINT8                       PcdSpdBaseAddress_1_0;         /* Offset 0x0022 */
  UINT8                       PcdSpdBaseAddress_1_1;         /* Offset 0x0023 */
  UINT8                       PcdTsegSize;                   /* Offset 0x0024 */
  UINT32                      UnusedUpdSpace1;               /* Offset 0x0025 */
  UINT8                       PcdIgdDvmt50PreAlloc;          /* Offset 0x0029 */
  UINT8                       PcdPrimaryDisplay;             /* Offset 0x002A */
  UINT8                       PcdInternalGfx;                /* Offset 0x002B */
  UINT8                       PcdMmioSize;                   /* Offset 0x002C */
  UINT8                       UnusedUpdSpace2;               /* Offset 0x002D */
  UINT8                       PcdApertureSize;               /* Offset 0x002E */
  UINT8                       PcdEnableAudio;                /* Offset 0x002F */
  UINT8                       PcdEnableRmt;                  /* Offset 0x0030 */
  UINT8                       PcdDDRVoltageSelectionWithCustomizedGpio; /* Offset 0x0031 */
  UINT8                       PcdDDRVoltageSelectionGpio0;   /* Offset 0x0032 */
  UINT8                       PcdDDRVoltageSelectionGpio0Active; /* Offset 0x0033 */
  UINT8                       PcdDDRVoltageSelectionGpio1;   /* Offset 0x0034 */
  UINT8                       PcdDDRVoltageSelectionGpio1Active; /* Offset 0x0035 */
  UINT8                       PcdDDRVoltageSelectionGpio2;   /* Offset 0x0036 */
  UINT8                       PcdDDRVoltageSelectionGpio2Active; /* Offset 0x0037 */
  UINT8                       PcdPegResetGpio;               /* Offset 0x0038 */
  UINT8                       PcdPegResetGpioActive;         /* Offset 0x0039 */
  UINT8                       UnusedUpdSpace3[70];           /* Offset 0x003A */
  UINT8                       PcdEnableLan;                  /* Offset 0x0080 */
  UINT8                       PcdEnableSata;                 /* Offset 0x0081 */
  UINT8                       PcdSataMode;                   /* Offset 0x0082 */
  UINT8                       PcdEnableAzalia;               /* Offset 0x0083 */
  UINT8                       PcdEnableXhci;                 /* Offset 0x0084 */
  UINT8                       PcdEnableEhci1;                /* Offset 0x0085 */
  UINT8                       PcdEnableEhci2;                /* Offset 0x0086 */
  UINT8                       PcdEnableSmbus;                /* Offset 0x0087 */
  UINT8                       UnusedUpdSpace4;               /* Offset 0x0088 */
  UINT8                       ReservedUpdSpace0[119];        /* Offset 0x0089 */
  UINT8                       PcdFastBoot;                   /* Offset 0x0100 */
  UINT8                       PcdUserCrbBoardType;           /* Offset 0x0101 */
  UINT8                       UnusedUpdSpace6[126];          /* Offset 0x0102 */
  AUDIO_SA_VERB_TABLE*        SaHdaVerbTablePtr;             /* Offset 0x0180 */
  AUDIO_AZALIA_VERB_TABLE*    AzaliaVerbTablePtr;            /* Offset 0x0184 */
  MEMORY_DOWN_DATA*           PcdMemoryDownSpdPtr;           /* Offset 0x0188 */
  UINT8                       UnusedUpdSpace7[388];          /* Offset 0x018C */
  UINT16                      PcdRegionTerminator;           /* Offset 0x0310 */
} UPD_DATA_REGION;

#define VPD_IMAGE_ID    0x562D54504C575348        /* 'HSWLPT-V' */
#define VPD_IMAGE_REV   0x00000302

typedef struct _VPD_DATA_REGION {
  UINT64                      PcdVpdRegionSign;              /* Offset 0x0000 */
  UINT32                      PcdImageRevision;              /* Offset 0x0008 */
  UINT32                      PcdUpdRegionOffset;            /* Offset 0x000C */
  UINT8                       UnusedVpdSpace0[16];           /* Offset 0x0010 */
  UINT32                      PcdFspReservedMemoryLength;    /* Offset 0x0020 */
  UINT8                       PcdPort80Route;                /* Offset 0x0024 */
} VPD_DATA_REGION;

#pragma pack()

#endif
