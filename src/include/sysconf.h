/* * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file match.h
 * @brief YARD-ICE 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __SYSCONF_H__
#define __SYSCONF_H__

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



/* 
 * File:	ctrl.c
 * Author:	Robinson Mittmann (bobmittmann@gmail.com)
 * Comment:
 * Copyright(c) 2012 i-LAX Electronics Inc. All Rights Reserved.
 */

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdbool.h>

#include "debug.h"
//#include "syscfg.h"
#include "conf.h"

#define SESSION_NAME_MAX 128
#define SESSION_PORT_MAX 64

struct syscfg {
	bool debug_enabled;
	bool quiet;
	struct {
		char name[SESSION_NAME_MAX];
		char port[SESSION_PORT_MAX];
		uint32_t tmo_ms;
	} session;
};

extern struct syscfg syscfg;

int syscfg_load(void);
int syscfg_save(void);
int syscfg_delete(void);
int syscfg_start(const char * path); 
int syscfg_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* __MATCH_H__ */
