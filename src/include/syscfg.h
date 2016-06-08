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
 * @file syscfg.h
 * @brief YARD-ICE 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __SYSCFG_H__
#define __SYSCFG_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

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

#endif /* __SYSCFG_H__ */
