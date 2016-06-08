/*
 *
 * File:	libconf/private.h
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

#ifndef __CONF_PRIVATE_H__
#define __CONF_PRIVATE_H__

static const char NULL_STRING[] = "NULL";

#define LTRIM(x) { char * _cp; \
                  for (_cp = x; isspace(*_cp) && (*_cp != 0); _cp++);\
                  x = _cp; }

#define RTRIM(x) { char * _cp; \
                  for (_cp = x + (strlen(x) -1); (_cp != x) && isspace(*_cp); _cp--);\
                  _cp++; *_cp = '\0'; }

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#ifdef __cplusplus
extern "C" {
#endif

	int void_get(conf_entry_t *, char *);
	int void_set(conf_entry_t *, const char *);

	int int_get(conf_entry_t *, char *);
	int int_set(conf_entry_t *, const char *);

	int int8_get(conf_entry_t *var, char *s);
	int int8_set(conf_entry_t *var, const char *s);

	int uint_get(conf_entry_t *, char *);
	int uint_set(conf_entry_t *, const char *);

	int float_get(conf_entry_t *, char *);
	int float_set(conf_entry_t *, const char *);

	int string_get(conf_entry_t *, char *);
	int string_set(conf_entry_t *, const char *);

	int bool_get(conf_entry_t *, char *);
	int bool_set(conf_entry_t *, const char *);

	int char_get(conf_entry_t *, char *);
	int char_set(conf_entry_t *, const char *);

	int uint8_get(conf_entry_t *var, char *s);
	int uint8_set(conf_entry_t *var, const char *s);

	int hex8_get(conf_entry_t *, char *);
	int hex8_set(conf_entry_t *, const char *);

	int bin8_get(conf_entry_t *, char *);
	int bin8_set(conf_entry_t *, const char *);

	int oct8_get(conf_entry_t *, char *);
	int oct8_set(conf_entry_t *, const char *);

	int int16_get(conf_entry_t *var, char *s);
	int int16_set(conf_entry_t *var, const char *s);

	int uint16_get(conf_entry_t *var, char *s);
	int uint16_set(conf_entry_t *var, const char *s);

	int hex16_get(conf_entry_t *, char *);
	int hex16_set(conf_entry_t *, const char *);

	int oct16_get(conf_entry_t *, char *);
	int oct16_set(conf_entry_t *, const char *);

	int bin16_get(conf_entry_t *, char *);
	int bin16_set(conf_entry_t *, const char *);

	int hex32_get(conf_entry_t *, char *);
	int hex32_set(conf_entry_t *, const char *);

	int bin32_get(conf_entry_t *, char *);
	int bin32_set(conf_entry_t *, const char *);

	int oct32_get(conf_entry_t *, char *);
	int oct32_set(conf_entry_t *, const char *);

	int int64_get(conf_entry_t *, char *);
	int int64_set(conf_entry_t *, const char *);

	int uint64_get(conf_entry_t *, char *);
	int uint64_set(conf_entry_t *, const char *);

	int hex64_get(conf_entry_t *, char *);
	int hex64_set(conf_entry_t *, const char *);

	int bin64_get(conf_entry_t *, char *);
	int bin64_set(conf_entry_t *, const char *);

	int oct64_get(conf_entry_t *, char *);
	int oct64_set(conf_entry_t *, const char *);

	int ipv4addr_get(conf_entry_t *, char *);
	int ipv4addr_set(conf_entry_t *, const char *);

	int rgb_get(conf_entry_t *, char *);
	int rgb_set(conf_entry_t *, const char *);

	int rgbi_get(conf_entry_t *, char *);
	int rgbi_set(conf_entry_t *, const char *);

	int cymk_get(conf_entry_t *, char *);
	int cymk_set(conf_entry_t *, const char *);

	int ratio_get(conf_entry_t *, char *);
	int ratio_set(conf_entry_t *, const char *);
#ifdef  __cplusplus
}
#endif

#endif	/* __CONF_PRIVATE_H__ */

