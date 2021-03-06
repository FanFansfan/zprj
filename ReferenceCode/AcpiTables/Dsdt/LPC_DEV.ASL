/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Haswell             *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved   *;
;*    This software and associated documentation (if any) is furnished    *;
;*    under a license and may only be used or copied in accordance        *;
;*    with the terms of the license. Except as permitted by such          *;
;*    license, no part of this software or documentation may be           *;
;*    reproduced, stored in a retrieval system, or transmitted in any     *;
;*    form or by any means without the express written consent of         *;
;*    Intel Corporation.                                                  *;
;*                                                                        *;
;*                                                                        *;
;**************************************************************************/
/*++
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
--*/

Device(DMAC)  // DMA Controller
{
  Name(_HID,EISAID("PNP0200"))        

  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0x00,0x00,0x01,0x20)
    IO(Decode16,0x81,0x81,0x01,0x11)
    IO(Decode16,0x93,0x93,0x01,0x0D)
    IO(Decode16,0xC0,0xC0,0x01,0x20)
    DMA(Compatibility,NotBusMaster,Transfer8_16) {4}
  })
}
      
Device(FWHD)  // Firmware Hub Device
{
  Name(_HID,EISAID("INT0800"))        

  Name(_CRS,ResourceTemplate()
  {
    Memory32Fixed(ReadOnly,0xFF000000,0x1000000)
  })
}


Device(HPET)  // High Performance Event Timer
{
  Name(_HID,EISAID("PNP0103"))
  Name(_UID, 0)

  Name(BUF0,ResourceTemplate()
  {
    Memory32Fixed(ReadWrite,0xFED00000,0x400,FED0)
  })

  Method(_STA,0)
  {
    // Show this Device only if the OS is WINXP or beyond.

    If(LGreaterEqual(OSYS,2001))
    {
      If(HPAE)
      {
        Return(0x000F)  // Enabled, do Display.
      }
    }
    Else
    {
      // OS = WIN98, WINME, or WIN2000.

      If(HPAE)
      {
        Return(0x000B)  // Enabled, don't Display.
      }
    }

    Return(0x0000)      // Return Nothing.
  }

  Method(_CRS,0,Serialized)
  {
    If(HPAE)
    {
      // Check if HPETimer Base should be modified.

      CreateDwordField(BUF0,^FED0._BAS,HPT0)

      If(LEqual(HPAS,1))
      {
          Store(0xFED01000,HPT0)
      }

      If(LEqual(HPAS,2))
      {
          Store(0xFED02000,HPT0)
      }

      If(LEqual(HPAS,3))
      {
          Store(0xFED03000,HPT0)
      }
    }

    Return(BUF0)
  }
}

Device(IPIC)  // 8259 PIC
{
  Name(_HID,EISAID("PNP0000"))

  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0x20,0x20,0x01,0x02)
    IO(Decode16,0x24,0x24,0x01,0x02)
    IO(Decode16,0x28,0x28,0x01,0x02)
    IO(Decode16,0x2C,0x2C,0x01,0x02)
    IO(Decode16,0x30,0x30,0x01,0x02)
    IO(Decode16,0x34,0x34,0x01,0x02)
    IO(Decode16,0x38,0x38,0x01,0x02)
    IO(Decode16,0x3C,0x3C,0x01,0x02)
    IO(Decode16,0xA0,0xA0,0x01,0x02)
    IO(Decode16,0xA4,0xA4,0x01,0x02)
    IO(Decode16,0xA8,0xA8,0x01,0x02)
    IO(Decode16,0xAC,0xAC,0x01,0x02)
    IO(Decode16,0xB0,0xB0,0x01,0x02)
    IO(Decode16,0xB4,0xB4,0x01,0x02)
    IO(Decode16,0xB8,0xB8,0x01,0x02)
    IO(Decode16,0xBC,0xBC,0x01,0x02)
    IO(Decode16,0x4D0,0x4D0,0x01,0x02)
    IRQNoFlags() {2}
  })
}

#ifdef TRAD_FLAG
Device(MATH)  // Math Co-Processor
{
  Name(_HID,EISAID("PNP0C04"))

  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0xF0,0xF0,0x01,0x01)
    IRQNoFlags() {13}
  })

  //
  // Report device present for LPT-H.
  //
  Method (_STA, 0x0, NotSerialized) 
  {
    // LPT-H Chipset DID start with 0x8xxx while LPT-LP start with 0x9xxx
    If(LEqual(And(CDID,0xF000), 0x8000)) {
      Return(0x1F)
    } else {
      Return(0x0)
    }
  }
}
#endif // TRAD_FLAG

Device(LDRC)  // LPC Device Resource Consumption
{
  Name(_HID,EISAID("PNP0C02"))        

  Name(_UID,2)
  
#if 0
  Name(_CRS,ResourceTemplate()                      // This is for Cougar Point
  {
    IO(Decode16,0x2E,0x2E,0x1,0x02)             // SIO Access.
    IO(Decode16,0x4E,0x4E,0x1,0x02)             // LPC Slot Access.
    IO(Decode16,0x61,0x61,0x1,0x1)               // NMI Status.           
    IO(Decode16,0x63,0x63,0x1,0x1)               // Processor I/F.
    IO(Decode16,0x65,0x65,0x1,0x1)               // Processor I/F.
    IO(Decode16,0x67,0x67,0x1,0x1)               // Processor I/F.
    IO(Decode16,0x70,0x70,0x1,0x1)               // NMI Enable.          
    IO(Decode16,0x80,0x80,0x1,0x1)               // Port 80h.
    IO(Decode16,0x92,0x92,0x1,0x1)               // Processor I/F.
    IO(Decode16,0xB2,0xB2,0x01,0x02)           // Software SMI.
    IO(Decode16,0x680,0x680,0x1,0x20)        // 32 Byte I/O.
    IO(Decode16,0xFFFF,0xFFFF,0x1,0x1)        // ACPI IO Trap.
    IO(Decode16,0xFFFF,0xFFFF,0x1,0x1)        // DTS IO Trap. 
    IO(Decode16,0xFFFF,0xFFFF,0x1,0x1)        // HotKey IO Trap.                      

    IO(Decode16, 0x800,0x800,0x1,0xFF)       // PCH GPIO Base.
    IO(Decode16, 0x900,0x900,0x1,0xFF)       // PCH GPIO Base.
    IO(Decode16, 0xA00,0xA00,0x1,0xFF)       // PCH GPIO Base.
    IO(Decode16, 0xB00,0xB00,0x1,0xFF)       // PCH GPIO Base.
    IO(Decode16, 0x1800,0x1800,0x1,0xFF)     // PCH ACPI Base

    IO(Decode16,0x164e,0x164e,0x1,0x02)    // 16 Byte I/O.
  })
#else
  Name(BUF0,ResourceTemplate()                      // This is for Cougar Point
  {
    IO(Decode16,0x2E,0x2E,0x1,0x02)             // SIO Access.
    IO(Decode16,0x4E,0x4E,0x1,0x02)             // LPC Slot Access.
    IO(Decode16,0x61,0x61,0x1,0x1)               // NMI Status.           
    IO(Decode16,0x63,0x63,0x1,0x1)               // Processor I/F.
    IO(Decode16,0x65,0x65,0x1,0x1)               // Processor I/F.
    IO(Decode16,0x67,0x67,0x1,0x1)               // Processor I/F.
    IO(Decode16,0x70,0x70,0x1,0x1)               // NMI Enable.          
    IO(Decode16,0x80,0x80,0x1,0x1)               // Port 80h.
    IO(Decode16,0x92,0x92,0x1,0x1)               // Processor I/F.
    IO(Decode16,0xB2,0xB2,0x01,0x02)           // Software SMI.
    IO(Decode16,0x680,0x680,0x1,0x20)        // 32 Byte I/O.
    IO(Decode16,0xFFFF,0xFFFF,0x1,0x1)        // ACPI IO Trap.
    IO(Decode16,0xFFFF,0xFFFF,0x1,0x1)        // DTS IO Trap. 
    IO(Decode16,0xFFFF,0xFFFF,0x1,0x1)        // HotKey IO Trap.                      

    IO(Decode16, 0x800,0x800,0x1,0xFF, GIO0)       // PCH GPIO Base.
    IO(Decode16, 0x900,0x900,0x1,0xFF, GIO1)       // PCH GPIO Base.
    IO(Decode16, 0xA00,0xA00,0x1,0xFF, GIO2)       // PCH GPIO Base.
    IO(Decode16, 0xB00,0xB00,0x1,0xFF, GIO3)       // PCH GPIO Base.
    IO(Decode16, 0x1800,0x1800,0x1,0xFF)     // PCH ACPI Base

    IO(Decode16,0x164e,0x164e,0x1,0x02)    // 16 Byte I/O.
  })

  Method(_CRS,0)
  {
    CreateWordField(BUF0, ^GIO0._MIN, IO0M)
    CreateWordField(BUF0, ^GIO0._MAX, IO0X)
    Store(GPBS, IO0M)
    Store(GPBS, IO0X)

    CreateWordField(BUF0, ^GIO1._MIN, IO1M)
    CreateWordField(BUF0, ^GIO1._MAX, IO1X)
    Store(Add(GPBS, 0x100), IO1M)
    Store(Add(GPBS, 0x100), IO1X)

    CreateWordField(BUF0, ^GIO2._MIN, IO2M)
    CreateWordField(BUF0, ^GIO2._MAX, IO2X)
    Store(Add(GPBS, 0x200), IO2M)
    Store(Add(GPBS, 0x200), IO2X)

    CreateWordField(BUF0, ^GIO3._MIN, IO3M)
    CreateWordField(BUF0, ^GIO3._MAX, IO3X)
    Store(Add(GPBS, 0x300), IO3M)
    Store(Add(GPBS, 0x300), IO3X)

    Return(BUF0)
  }
#endif
}

Device(RTC) // RTC
{
  Name(_HID,EISAID("PNP0B00"))

  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0x70,0x70,0x01,0x08)
    IRQNoFlags() {8}
  })
}

Device(TIMR)  // 8254 Timer
{
  Name(_HID,EISAID("PNP0100"))

  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0x40,0x40,0x01,0x04)
    IO(Decode16,0x50,0x50,0x10,0x04)
    IRQNoFlags() {0}
  })
}
    
Device(CWDT)
{
  Name(_HID,EISAID("INT3F0D"))
  Name(_CID,EISAID("PNP0C02"))
  Name(BUF0,ResourceTemplate()
    {
      IO(Decode16, 0x1854, 0x1854, 0x4, 0x4)  // PMBS 0x1800 + Offset 0x54
    }
  )
  
  Method(_STA,0,Serialized)
  {
    If(LEqual(WDTE,1))
    {
      Return(0x0F)
    }
    Else
    {
      Return(0x00)
    }
  }
  
  Method(_CRS,0,Serialized)
  {
    Return(BUF0)
  }
}
