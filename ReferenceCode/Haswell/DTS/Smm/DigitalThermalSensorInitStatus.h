/** @file
  This protocol is used to get the status of Digital Thermal Sensor driver initialization

@copyright
  Copyright (c) 1999 - 2012 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
#ifndef _DTS_INIT_STATUS_PROTOCOL_H_
#define _DTS_INIT_STATUS_PROTOCOL_H_

///
/// Define protocol GUID
///
#if !defined(EDK_RELEASE_VERSION) || (EDK_RELEASE_VERSION < 0x00020000)
#define DTS_INIT_STATUS_PROTOCOL_GUID \
  { \
    0xf687f441, 0x7dcf, 0x4f45, 0x8f, 0x64, 0xca, 0xf2, 0x88, 0xea, 0x50, 0x4e \
  }
#else
#define DTS_INIT_STATUS_PROTOCOL_GUID \
  { \
    0xf687f441, 0x7dcf, 0x4f45, \
    { \
      0x8f, 0x64, 0xca, 0xf2, 0x88, 0xea, 0x50, 0x4e \
    } \
  }
#endif

typedef struct _DTS_INIT_STATUS_PROTOCOL {
  BOOLEAN IsDtsInitComplete;
} DTS_INIT_STATUS_PROTOCOL;

#endif
