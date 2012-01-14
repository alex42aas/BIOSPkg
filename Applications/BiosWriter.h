/** @file

  Copyright (c) 2008 - 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _EFILOADER_H_
#define _EFILOADER_H_

#ifdef MEMORY_INIT_TYPES
typedef	U32	UINT32;
typedef	U16	UINT16;
typedef	U8	UINT8;
#endif // MEMORY_INIT_TYPES


typedef char CHAR8;


#ifndef VA_START

#define _INT_SIZE_OF(n) ((sizeof (n) + sizeof (UINTN) - 1) &~(sizeof (UINTN) - 1))

///
/// Variable used to traverse the list of arguments. This type can vary by 
/// implementation and could be an array or structure. 
///
typedef CHAR8 *VA_LIST;

#define VA_START(Marker, Parameter) (Marker = (VA_LIST) ((UINTN) & (Parameter) + _INT_SIZE_OF (Parameter)))

#define VA_ARG(Marker, TYPE)   (*(TYPE *) ((Marker += _INT_SIZE_OF (TYPE)) - _INT_SIZE_OF (TYPE)))

#define VA_END(Marker)      (Marker = (VA_LIST) 0)

#define VA_COPY(dest, src) ((void)((dest) = (src)))

#endif


void	outportb(int port_addr, int arg);
void	outport(int port_addr, int arg);
void	outportd(int port_addr, int arg);

int	inportb(int port_addr);
int	inport(int port_addr);
int	inportd(int port_addr);

/*
int	isBootCpu(void);
int	getApicId(void);
UINT32	readMsr(UINT32 msr);
#ifndef EFIX64
UINT32	retStack (void);
#else
UINT64	retStack (void);
#endif

UINT32	readFlags(void);
UINT32	sIdt(UINT16 *pnt);
void	lIdt(UINT16 *pnt);
void	sGdt(UINT16 *pnt);
void	lGdt(UINT16 *pnt);
void	saveGdtInfo(void *gdtInfo);
void	setGdtInfo(void *gdtInfo);

int	rtcRead(int index);
void	rtcWrite(int index, int value);
*/

void	setUsart(UINT32 speed);

int	tti(void);
void	tto(int arg);
int	asktti(void);

/*
enum	{
	PBM_USART = 0,
	PBM_MEM
	};
*/
void	vuprintf(int mode, UINT8 **dst, UINT8 *format, VA_LIST argptr);

void	_uprintf(UINT8 *format, ...);
void	uprintf(UINT8 *format, ...);
void	_sprintf(UINT8 *dst, UINT8 *format, ...);


int	rtcRead(int index);
void	rtcWrite(int index, int value);
//int	debugCheckPoint(char *moduleIdent);

#define	RTC_FLAG_UPRINTF	0xfe

#define	FLAG_UPRINTF		(1 << 0)
#define	FLAG_UPRINTF_ALL	(1 << 1)

#define	setFlagUprintf()	rtcWrite(RTC_FLAG_UPRINTF, rtcRead(RTC_FLAG_UPRINTF) | FLAG_UPRINTF)
#define	clrFlagUprintf()	rtcWrite(RTC_FLAG_UPRINTF, rtcRead(RTC_FLAG_UPRINTF) & ~FLAG_UPRINTF)
#define tstFlagUprintf()	(rtcRead(RTC_FLAG_UPRINTF) & FLAG_UPRINTF)

#define	setFlagUprintfAll()	rtcWrite(RTC_FLAG_UPRINTF, rtcRead(RTC_FLAG_UPRINTF) | FLAG_UPRINTF_ALL)
#define	clrFlagUprintfAll()	rtcWrite(RTC_FLAG_UPRINTF, rtcRead(RTC_FLAG_UPRINTF) & ~FLAG_UPRINTF_ALL)
#define tstFlagUprintfAll()	(rtcRead(RTC_FLAG_UPRINTF) & FLAG_UPRINTF_ALL)



void	Stall(UINTN delay);


#endif	/* EFILOADER_H	*/
