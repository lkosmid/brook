/* *************************************************************************
 * Copyright (C) 2004 Jeremy Sugerman
 * All Rights Reserved
 * *************************************************************************/

/*
 * timer.h --
 *
 *      Timer exports simple functionality for reseting a timer and querying
 *      its current value.
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void Timer_Reset(void);
extern float Timer_GetMS(void);

#ifdef __cplusplus
}
#endif
#endif
