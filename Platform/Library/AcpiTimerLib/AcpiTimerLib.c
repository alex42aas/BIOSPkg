/** @file
  ACPI Timer implements one instance of Timer Library.

  Copyright (c) 2008, Intel Corporation<BR> All rights
  reserved. This program and the accompanying materials are
  licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php
  
  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/ 

#include <Base.h>
#include <Library/TimerLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>

#define ACPI_TIMER_FREQUENCY   3579545
#define ACPI_TIMER_COUNT_SIZE  0x1000000ULL
#define ACPI_TIMER_WIDTH       24

/**
  The constructor function enables ACPI IO space.

  If ACPI I/O space not enabled, this function will enable it.
  It will always return RETURN_SUCCESS.

  @retval EFI_SUCCESS   The constructor always returns RETURN_SUCCESS.

**/
RETURN_STATUS
EFIAPI
AcpiTimerLibConstructor (
  VOID
  )
{
  return RETURN_SUCCESS; 
}

/**
  Internal function to read the current tick counter of ACPI.

  Internal function to read the current tick counter of ACPI.

  @return The tick counter read.

**/
STATIC
UINT32
InternalAcpiGetTimerTick (
  VOID
  )
{
  UINT32 PmBase; 
  PmBase = PciRead32(PCI_LIB_ADDRESS(0,31,0,0x40)) & 0xFFFE;
  return IoRead32 ( PmBase + 8 );
}

/**
  Stalls the CPU for at least the given number of ticks.

  Stalls the CPU for at least the given number of ticks. It's invoked by
  MicroSecondDelay() and NanoSecondDelay().

  @param  Delay     A period of time to delay in ticks.

**/
STATIC
VOID
InternalAcpiDelay (
  IN      UINT32                    Delay
  )
{
  UINT32                            Ticks;
  UINT32                            Times;

  Times    = Delay >> (ACPI_TIMER_WIDTH  - 2);
  Delay   &= (1 << (ACPI_TIMER_WIDTH  - 2)) - 1;
  do {
    //
    // The target timer count is calculated here
    //
    Ticks    = InternalAcpiGetTimerTick () + Delay;
    Delay    = (1 << (ACPI_TIMER_WIDTH  - 2));
    //
    // Wait until time out
    // Delay >= 2^23 (if ACPI provide 24-bit timer) or Delay >= 2^31 (if ACPI
    // provide 32-bit timer) could not be handled by this function
    // Timer wrap-arounds are handled correctly by this function
    //
    while (((Ticks - InternalAcpiGetTimerTick ()) & (1 << (ACPI_TIMER_WIDTH - 1))) == 0) {
//      IoWrite8 (0xEB, 0x00);
      CpuPause ();
    }
  } while (Times-- > 0);
}

/**
  Stalls the CPU for at least the given number of microseconds.

  Stalls the CPU for the number of microseconds specified by MicroSeconds.

  @param  MicroSeconds  The minimum number of microseconds to delay.

  @return MicroSeconds

**/
UINTN
EFIAPI
MicroSecondDelay (
  IN      UINTN                     MicroSeconds
  )
{
  InternalAcpiDelay (
    (UINT32)DivU64x32 (
              MultU64x32 (
                MicroSeconds,
                ACPI_TIMER_FREQUENCY
                ),
              1000000u
              )
    );
  return MicroSeconds;
}

/**
  Stalls the CPU for at least the given number of nanoseconds.

  Stalls the CPU for the number of nanoseconds specified by NanoSeconds.

  @param  NanoSeconds The minimum number of nanoseconds to delay.

  @return NanoSeconds

**/
UINTN
EFIAPI
NanoSecondDelay (
  IN      UINTN                     NanoSeconds
  )
{
  InternalAcpiDelay (
    (UINT32)DivU64x32 (
              MultU64x32 (
                NanoSeconds,
                ACPI_TIMER_FREQUENCY
                ),
              1000000000u
              )
    );
  return NanoSeconds;
}

/**
  Retrieves the current value of a 64-bit free running performance counter.

  Retrieves the current value of a 64-bit free running performance counter. The
  counter can either count up by 1 or count down by 1. If the physical
  performance counter counts by a larger increment, then the counter values
  must be translated. The properties of the counter can be retrieved from
  GetPerformanceCounterProperties().

  @return The current value of the free running performance counter.

**/
UINT64
EFIAPI
GetPerformanceCounter (
  VOID
  )
{
  return (UINT64)InternalAcpiGetTimerTick ();
}

/**
  Retrieves the 64-bit frequency in Hz and the range of performance counter
  values.

  If StartValue is not NULL, then the value that the performance counter starts
  with immediately after is it rolls over is returned in StartValue. If
  EndValue is not NULL, then the value that the performance counter end with
  immediately before it rolls over is returned in EndValue. The 64-bit
  frequency of the performance counter in Hz is always returned. If StartValue
  is less than EndValue, then the performance counter counts up. If StartValue
  is greater than EndValue, then the performance counter counts down. For
  example, a 64-bit free running counter that counts up would have a StartValue
  of 0 and an EndValue of 0xFFFFFFFFFFFFFFFF. A 24-bit free running counter
  that counts down would have a StartValue of 0xFFFFFF and an EndValue of 0.

  @param  StartValue  The value the performance counter starts with when it
                      rolls over.
  @param  EndValue    The value that the performance counter ends with before
                      it rolls over.

  @return The frequency in Hz.

**/
UINT64
EFIAPI
GetPerformanceCounterProperties (
  OUT      UINT64                    *StartValue,  OPTIONAL
  OUT      UINT64                    *EndValue     OPTIONAL
  )
{
  if (StartValue != NULL) {
    *StartValue = 0;
  }

  if (EndValue != NULL) {
    *EndValue = ACPI_TIMER_COUNT_SIZE - 1;
  }

  return ACPI_TIMER_FREQUENCY;
}


/**
  Converts elapsed ticks of performance counter to time in nanoseconds.

  This function converts the elapsed ticks of running performance counter to
  time value in unit of nanoseconds.

  @param  Ticks     The number of elapsed ticks of running performance counter.

  @return The elapsed time in nanoseconds.

**/
UINT64
EFIAPI
GetTimeInNanoSecond (
  IN      UINT64                     Ticks
  )
{
  UINT64  NanoSeconds;
  UINT32  Remainder;

  //
  //          Ticks
  // Time = --------- x 1,000,000,000
  //        Frequency
  //
  NanoSeconds = MultU64x32 (DivU64x32Remainder (Ticks, ACPI_TIMER_FREQUENCY, &Remainder), 1000000000u);

  //
  // Frequency < 0x100000000, so Remainder < 0x100000000, then (Remainder * 1,000,000,000)
  // will not overflow 64-bit.
  //
  NanoSeconds += DivU64x32 (MultU64x32 ((UINT64) Remainder, 1000000000u), ACPI_TIMER_FREQUENCY);

  return NanoSeconds;
}
