/*
 *
 * File:	conf.h
 * Module:
 * Project:	
 * Author:	Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2000-2005 Bob Mittmann. All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#ifndef __CONF_H__
#define __CONF_H__

#include <stdint.h>
#include <stdbool.h>

struct conf_entry;

struct conf_typedesc {
	int t_code;
	char *t_name;
	int (*t_get) (struct conf_entry *, char *);
	int (*t_set) (struct conf_entry *, const char *);
};

struct conf_entry {
	const char * name;
	struct conf_typedesc * type;
	void * p; /* pointer to a variable holding the value */
	unsigned int len;
};

struct conf_root {
	const char * name;
	struct conf_entry entry[];
};

enum conf_type {
	CONF_VOID = 0,
	CONF_SECTION = 1,
	CONF_INT = 2,
	CONF_UINT = 3,
	CONF_FLOAT = 4,
	CONF_STRING = 5,
	CONF_BOOLEAN = 6,
	CONF_CHAR = 7,

	CONF_INT8 = 8,
	CONF_UINT8 = 9,
	CONF_HEX8 = 10,
	CONF_BIN8 = 11,
	CONF_OCT8 = 12,

	CONF_INT16 = 13,
	CONF_UINT16 = 14,
	CONF_HEX16 = 15,
	CONF_BIN16 = 16,
	CONF_OCT16 = 17,

	CONF_INT32 = 18,
	CONF_UINT32 = 19,
	CONF_HEX32 = 20,
	CONF_BIN32 = 21,
	CONF_OCT32 = 22,

	CONF_INT64 = 21,
	CONF_UINT64 = 22,
	CONF_HEX64 = 23,
	CONF_BIN64 = 24,
	CONF_OCT64 = 25,

	CONF_IPV4ADDR = 26,
	CONF_RGB = 27,
	CONF_RGBI = 28,
	CONF_CYMK = 29,
	CONF_RATIO = 30,
	CONF_MAX = 31
};

#define CONF_TYPE(TYPE) (&conf_type_tab[TYPE])

#define DEFINE_VOID(NM, PTR) {NM, CONF_TYPE(CONF_VOID), (void *)(PTR), 0},
#define DEFINE_SECTION(NM, PTR) {NM,  CONF_TYPE(CONF_SECTION), (void *)(PTR), 0},
#define DEFINE_INT(NM,  PTR) {NM, CONF_TYPE(CONF_INT), (void *)(PTR), 0},
#define DEFINE_UINT(NM,  PTR) {NM, CONF_TYPE(CONF_UINT), (void *)(PTR), 0},
#define DEFINE_STRING(NM, PTR) {NM, CONF_TYPE(CONF_STRING), (void *)(PTR), 0},
#define DEFINE_STRINGCNT(NM, PTR, LEN) {NM, CONF_TYPE(CONF_STRING), \
	(void *)(PTR), LEN},

#define DEFINE_FLOAT(NM, PTR) {NM, CONF_TYPE(CONF_FLOAT), (void *)(PTR), 0},
#define DEFINE_BOOLEAN(NM, PTR) {NM, CONF_TYPE(CONF_BOOLEAN), (void *)(PTR), 0},
#define DEFINE_CHAR(NM, PTR) {NM, CONF_TYPE(CONF_CHAR), (void *)(PTR), 0},

#define DEFINE_INT8(NM,  PTR) {NM, CONF_TYPE(CONF_INT8), (void *)(PTR), 0},
#define DEFINE_UINT8(NM,  PTR) {NM, CONF_TYPE(CONF_UINT8), (void *)(PTR), 0},
#define DEFINE_HEX8(NM, PTR) {NM,  CONF_TYPE(CONF_HEX8), (void *)(PTR), 0},
#define DEFINE_BIN8(NM, PTR) {NM,  CONF_TYPE(CONF_BIN8), (void *)(PTR), 0},
#define DEFINE_OCT8(NM, PTR) {NM,  CONF_TYPE(CONF_OCT8), (void *)(PTR), 0},

#define DEFINE_INT16(NM,  PTR) {NM, CONF_TYPE(CONF_INT16), (void *)(PTR), 0},
#define DEFINE_UINT16(NM,  PTR) {NM, CONF_TYPE(CONF_UINT16), (void *)(PTR), 0},
#define DEFINE_HEX16(NM, PTR) {NM,  CONF_TYPE(CONF_HEX16), (void *)(PTR), 0},
#define DEFINE_BIN16(NM, PTR) {NM,  CONF_TYPE(CONF_BIN16), (void *)(PTR), 0},
#define DEFINE_OCT16(NM, PTR) {NM,  CONF_TYPE(CONF_OCT16), (void *)(PTR), 0},

#define DEFINE_INT32(NM,  PTR) {NM, CONF_TYPE(CONF_INT32), (void *)(PTR), 0},
#define DEFINE_UINT32(NM,  PTR) {NM, CONF_TYPE(CONF_UINT32), (void *)(PTR), 0},
#define DEFINE_HEX32(NM, PTR) {NM,  CONF_TYPE(CONF_HEX32), (void *)(PTR), 0},
#define DEFINE_BIN32(NM, PTR) {NM,  CONF_TYPE(CONF_BIN32), (void *)(PTR), 0},
#define DEFINE_OCT32(NM, PTR) {NM,  CONF_TYPE(CONF_OCT32), (void *)(PTR), 0},

#define DEFINE_INT64(NM,  PTR) {NM, CONF_TYPE(CONF_INT64), (void *)(PTR), 0},
#define DEFINE_UINT64(NM,  PTR) {NM, CONF_TYPE(CONF_UINT64), (void *)(PTR), 0},
#define DEFINE_HEX64(NM, PTR) {NM,  CONF_TYPE(CONF_HEX64), (void *)(PTR), 0},
#define DEFINE_BIN64(NM, PTR) {NM,  CONF_TYPE(CONF_BIN64), (void *)(PTR), 0},
#define DEFINE_OCT64(NM, PTR) {NM,  CONF_TYPE(CONF_OCT64), (void *)(PTR), 0},

#define DEFINE_IPV4ADDR(NM, PTR) {NM,  CONF_TYPE(CONF_IPV4ADDR), \
	(void *)(PTR), 0},

#define DEFINE_RGB(NM, PTR) {NM,  CONF_TYPE(CONF_RGB), (void *)(PTR) },
#define DEFINE_RGBI(NM, PTR) {NM,  CONF_TYPE(CONF_RGBI), (void *)(PTR), 0},
#define DEFINE_CYMK(NM, PTR) {NM,  CONF_TYPE(CONF_CYMK), (void *)(PTR), 0},
#define DEFINE_RATIO(NM, PTR) {NM,  CONF_TYPE(CONF_RATIO), (void *)(PTR), 0},

#define EXTERN_SECTION(X) extern struct conf_entry X[];

#define BEGIN_SECTION(X) struct conf_entry X[] = {
#define SECTION_TERMINATOR	{ NULL, NULL, NULL }
#define END_SECTION SECTION_TERMINATOR };

#define EXPORT_VOID(VAR) DEFINE_VOID(#VAR, &(VAR))
#define EXPORT_SECTION(VAR) DEFINE_SECTION(#VAR, &(VAR))

#define EXPORT_INT(VAR) DEFINE_INT(#VAR, &(VAR))
#define EXPORT_UINT(VAR) DEFINE_UINT(#VAR, &(VAR))
#define EXPORT_FLOAT(VAR) DEFINE_FLOAT(#VAR, &(VAR))
#define EXPORT_STRING(VAR) DEFINE_STRING(#VAR, &(VAR))
#define EXPORT_BOOLEAN(VAR) DEFINE_BOOLEAN(#VAR, &(VAR))
#define EXPORT_CHAR(VAR) DEFINE_CHAR(#VAR, &(VAR))

#define EXPORT_INT8(VAR) DEFINE_INT8(#VAR, &(VAR))
#define EXPORT_UINT8(VAR) DEFINE_UINT8(#VAR, &(VAR))
#define EXPORT_HEX8(VAR) DEFINE_HEX8(#VAR, &(VAR))
#define EXPORT_BIN8(VAR) DEFINE_BIN8(#VAR, &(VAR))
#define EXPORT_OCT8(VAR) DEFINE_OCT8(#VAR, &(VAR))

#define EXPORT_INT16(VAR) DEFINE_INT16(#VAR, &(VAR))
#define EXPORT_UINT16(VAR) DEFINE_UINT16(#VAR, &(VAR))
#define EXPORT_HEX16(VAR) DEFINE_HEX16(#VAR, &(VAR))
#define EXPORT_BIN16(VAR) DEFINE_BIN16(#VAR, &(VAR))
#define EXPORT_OCT16(VAR) DEFINE_OCT16(#VAR, &(VAR))

#define EXPORT_INT32(VAR) DEFINE_INT32(#VAR, &(VAR))
#define EXPORT_UINT32(VAR) DEFINE_UINT32(#VAR, &(VAR))
#define EXPORT_HEX32(VAR) DEFINE_HEX32(#VAR, &(VAR))
#define EXPORT_BIN32(VAR) DEFINE_BIN32(#VAR, &(VAR))
#define EXPORT_OCT32(VAR) DEFINE_OCT32(#VAR, &(VAR))

#define EXPORT_INT64(VAR) DEFINE_INT64(#VAR, &(VAR))
#define EXPORT_UINT64(VAR) DEFINE_UINT64(#VAR, &(VAR))
#define EXPORT_HEX64(VAR) DEFINE_HEX64(#VAR, &(VAR))
#define EXPORT_BIN64(VAR) DEFINE_BIN64(#VAR, &(VAR))
#define EXPORT_OCT64(VAR) DEFINE_OCT64(#VAR, &(VAR))

#define EXPORT_IPV4ADDR(VAR) DEFINE_IPV4ADDR(#VAR, &(VAR))

#define EXPORT_RGB(VAR) DEFINE_RGB(#VAR, &(VAR))
#define EXPORT_RGBI(VAR) DEFINE_RGBI(#VAR, &(VAR))
#define EXPORT_CYMK(VAR) DEFINE_CYMK(#VAR, &(VAR))
#define EXPORT_RATIO(VAR) DEFINE_RATIO(#VAR, &(VAR))

#define EXPORT_STRINGCNT(VAR, LEN) DEFINE_STRINGCNT(#VAR, &(VAR), LEN)

extern struct conf_typedesc conf_type_tab[];

#ifdef __cplusplus
extern "C" {
#endif

/**
 * It cannot perform lookups for section names on the root section, due to 
 * the way it is defined. If done so, the root section address is returned
 * instead of the (expected) sibling section.
 */
int conf_lookup(char *, const char **, const char **);

int conf_dump(struct conf_entry *);
int conf_load(const char *, struct conf_entry *);
int conf_save(const char *, struct conf_entry *);

int conf_var_set(struct conf_entry *, const char *, const char *);
int conf_var_get(struct conf_entry *, const char *, char *);

#ifdef  __cplusplus
}
#endif

#endif	/* __CONF_H__ */

