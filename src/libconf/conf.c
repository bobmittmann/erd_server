/* $Id: conf.c,v 1.11 2005/05/17 20:54:25 carlos Exp $ 
 *
 * File:	conf.c
 * Module:
 * Project:	libconf
 * Author:	Robinson Mittmann (bob@boreste.com, bob@methafora.com.br)
 * Target:
 * Comment:
 * Copyright(C) 2005 BORESTE LTDA. All Rights Reserved.
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
 * Revisions
 * 
 * 0.1 Bob
 *     Release inicial.
 * 0.2 Jonas Set/10 (jonasmuriloantunes@gmail.com)
 *     +feature: Adicionado suporte a strings com tamanho máximo através das macros
 *     DEFINE_STRINGCNT e EXPORT_STRINGCNT.
 * 0.3 Jonas Set/10 (jonasmuriloantunes@gmail.com)
 *     +fix: corrigido bug na rotina char_set().  Não fazia a leitura do char
 *     corretamente porque faltava um LTRIM na string source.
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "conf.h"
#include "private.h"
#include "debug.h"

struct conf_type conf_type_tab[] = {
	{CONF_VOID, "void", void_get, void_set},
	{CONF_SECTION, "section", void_get, void_set},
	{CONF_INT, "integer", int_get, int_set},
	{CONF_UINT, "unsigned", uint_get, uint_set},
	{CONF_FLOAT, "float", float_get, float_set},
	{CONF_STRING, "string", string_get, string_set},
	{CONF_BOOLEAN, "boolean", bool_get, bool_set},
	{CONF_CHAR, "char", char_get, char_set},

	{CONF_INT8, "int_8", int8_get, int8_set},
	{CONF_INT8, "uint_8", uint8_get, uint8_set},
	{CONF_HEX8, "hex_8", hex8_get, hex8_set},
	{CONF_BIN8, "binary_8", bin8_get, bin8_set},
	{CONF_OCT8, "octal_8", oct8_get, oct8_set},

	{CONF_INT16, "int_16", int16_get, int16_set},
	{CONF_INT16, "uint_16", uint16_get, uint16_set},
	{CONF_HEX16, "hex_16", hex16_get, hex16_set},
	{CONF_BIN16, "binary_16", bin16_get, bin16_set},
	{CONF_OCT16, "octal_16", oct16_get, oct16_set},

/*	{CONF_INT32, "int_32", int_get, int_set},
	{CONF_INT32, "uint_32", uint_get, uint_set}, */
	{CONF_HEX32, "hex_32", hex32_get, hex32_set},
	{CONF_BIN32, "binary_32", bin32_get, bin32_set},
	{CONF_OCT32, "octal_32", oct32_get, oct32_set},

	{CONF_INT64, "int_64", int64_get, int64_set},
	{CONF_INT64, "uint_64", uint64_get, uint64_set},
	{CONF_HEX64, "hex_64", hex64_get, hex64_set},
	{CONF_BIN64, "binary_64", bin64_get, bin64_set},
	{CONF_OCT64, "octal_64", oct64_get, oct64_set},

	{CONF_IPV4ADDR, "ipv4_address", ipv4addr_get, ipv4addr_set},

	{CONF_RGB, "rgb", rgb_get, rgb_set},
	{CONF_RGBI, "rgbi", rgbi_get, rgbi_set},
	{CONF_CYMK, "cymk", cymk_get, cymk_set},
	{CONF_RATIO, "ratio", ratio_get, ratio_set}
};

int strtokcmp(const char * haystack, const char * needle, 
				 int delim, char ** saveptr)
{
	int hc;
	int nc;

	if ((haystack == NULL) || (needle == NULL))
		return 0;
	
	do {
		hc = *haystack++;
		nc = *needle++;

		if (hc != nc) {

			if ((hc == delim) && (nc == '\0')) {
				if (saveptr != NULL) {
					*saveptr = (char *)haystack;

					return 1;
				}
			}

			return -1;
		}

	} while (nc != '\0');

	return 0;
}

static conf_entry_t * section_lookup(conf_entry_t * section, 
									 const char *name);

static conf_entry_t * entry_lookup(conf_entry_t * section, const char *name)
{
	conf_entry_t *entry;
	int ret;
	char * rem;

	if ((name == NULL) || (*name == '\0')) {
		DBG(DBG_WARNING, "name invalid");
		return NULL;
	}

	if ((entry = section) == NULL) {
		DBG(DBG_WARNING, "section invalid");
		return NULL;
	}

	while (entry->name != NULL) {

		if ((ret = strtokcmp(name, entry->name, '/', &rem)) >= 0) {

			if (ret == 0)
				return entry;

			if (entry->type != CONF_TYPE(CONF_SECTION)) {
				return NULL;
			}

			return entry_lookup((conf_entry_t *)entry->p, rem);
		}
		entry++;
	}

	DBG(DBG_WARNING, "name not found");
	return NULL;
}

static conf_entry_t * section_lookup(conf_entry_t * section, const char *name)
{
	conf_entry_t * entry;

	DBG(DBG_INFO, "loking for \"%s\" at section 0x%p", name, section);

	if ((entry = entry_lookup(section, name)) == NULL) {
		return entry;
	}

	if (entry->type != CONF_TYPE(CONF_SECTION)) {
		return NULL;
	}

	return (conf_entry_t *) entry->p;
}

int conf_entry_set(conf_entry_t * section, const char *name, const char *value)
{
	conf_entry_t *entry;

	entry = entry_lookup(section, name);
	if (entry == NULL)
		return -1;

	/* Assuming NULL set as a stub configuration. */
	if (value == NULL)
		value = NULL_STRING;
	
	if (entry->type->t_set(entry, value))
		return 0;

	return -1;
}

int conf_entry_get(conf_entry_t * section, const char *name, char *value)
{
	conf_entry_t *entry;

	if (value == NULL)
		return -1;

	entry = entry_lookup(section, name);
	if (entry == NULL)
		return -1;

	if (entry->type->t_get(entry, value))
		return 0;
	else {
		strcpy(value, NULL_STRING);
		return 0;
	}

	return -1;
}

int conf_parse(uint8_t * buf, conf_entry_t * root)
{
	conf_entry_t * section;
	conf_entry_t * entry;
	uint8_t * cp = buf;
	char token[512];
	int skip_section = 0;
	int count = 0;;
	int line = 0;
	int pos;
	int c;

	section = root;

	for (;;) {
		/* ---------------------------------------------------------
		   locate the next section, ignoring empty and comment lines */
		line++;
		DBG(DBG_MSG, "line %3d", line);

		/* skip spaces */
		do { c = *cp++; } while ((c == ' ') || (c == '\t'));

		if (c == '\0')
			break;

		if (c == '\n') {
			DBG(DBG_MSG, "empty line");
			continue;
		}

		if (c == '#') {
			DBG(DBG_MSG, "comment");
			do {
				c = *cp++;
				if (c == '\0')
					goto end;
			} while (c != '\n');
			continue;	
		}

		if (c == '[') {
			skip_section = 0;

			/* skip spaces */
			do { c = *cp++; } while ((c == ' ') || (c == '\t'));

			/* section name must start with a letter */
			if (!isalpha(c)) {
				DBG(DBG_ERROR, "line %d: expecting alphabetic character", 
					line);
				return -1;
			}

			/* read the section name */
			pos = 0;
			do { 
				token[pos++] = c;	
				c = *cp++; 
			} while (isalnum(c) || (c == '_') || (c == '/'));
			token[pos] = '\0';	

			/* skip spaces, if any */
			while ((c == ' ') || (c == '\t')) { c = *cp++; };

			if (c != ']') {
				DBG(DBG_ERROR, "line %d: expecting ]", line);
				return -1;
			}

			/* skip spaces */
			do { c = *cp++; } while ((c == ' ') || (c == '\t'));

			if (c == '#') {
				/* ignore to the end of line */
				DBG(DBG_MSG, "comment");
				do {
					c = *cp++;
					if (c == '\0')
						goto end;
				} while (c != '\n');
			} 

			if (c != '\n') {
				DBG(DBG_ERROR, "line %d: expecting EOL", line);
				return -1;
			}

			DBG(DBG_MSG, "section_lookup: '%s'", token);

			if ((section = section_lookup(root, token)) == NULL) {
				DBG(DBG_WARNING, "invalid section: '%s'", token);
				skip_section = 1;
			}
			continue;
		}

		if (skip_section) {
			DBG(DBG_MSG, "skiping");
			do {
				c = *cp++;
				if (c == '\0')
					goto end;
			} while (c != '\n');
			continue;	
		}
			
		/* ---------------------------------------------------------
		   read the section content 
		   -------------------------------------------------------- */

		/* entry name must start with a letter */
		if (!isalpha(c)) {
			DBG(DBG_ERROR, "line %d: expecting alphabetic character", line);
			return -1;
		}

		/* read the entry name */
		pos = 0;
		do { 
			token[pos++] = c;	
			c = *cp++; 
		} while (isalnum(c) || (c == '.') || (c == '-') || (c == '_'));
		token[pos] = '\0';	

		/* skip spaces, if any */
		while ((c == ' ') || (c == '\t')) { c = *cp++; };

		if (c != '=') {
			DBG(DBG_ERROR, "line %d: expecting =", line);
			return -1;
		}

		if ((entry = entry_lookup(section, token)) == NULL) {
			DBG(DBG_WARNING, "invalid entry: '%s.%s'", section->name, token);
			/* ignore to the end of line */
			do {
				c = *cp++;
				if (c == '\0')
					goto end;
			} while (c != '\n');
			continue;
		}

		/* skip spaces */
		do { c = *cp++; } while ((c == ' ') || (c == '\t'));

		/* read the entry value */
		pos = 0;
		do { 
			token[pos++] = c;	
			c = *cp++; 
		} while ((c != '\n') && (c != '\0'));
		token[pos] = '\0';	

		/* set the variable */
		if (entry->type->t_set(entry, token))
			count++;
		else
			DBG(DBG_WARNING, "line %d: '%s': decode error.", line, entry->name);
	}
end:
	return 0;
}

int conf_load(const char * path, conf_entry_t * root)
{
	uint8_t * buf;
	FILE * f;
	int size;

	/* open the file for reading */
	if (!(f = fopen(path, "r"))) {
		DBG(DBG_ERROR, "fopen(\"%s\", \"r\") fail!", path);
		return -1;
	}

	/* get the file's size */
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	/* alloc memory for reading */
	buf = (uint8_t *)calloc(1, size + 1);

	/* read the file */
	if ((fread(buf, size, 1, f)) < 1) {
		DBG(DBG_ERROR, "fread() fail!");
		return -1;
	}

	/* close the file */
	fclose(f);
	/* null terminate the string */
	buf[size] = '\0';

	return conf_parse(buf, root);
}

/*
* Recursive writes a section into a stream
*
*/
static int write_section(FILE * f, conf_entry_t * section, char * branch)
{
	conf_entry_t *entry;
	int count = 0;
	char buf[2048];

	/* define a queue for sections */
	conf_entry_t *queue[1024];
	int head = 0;
	int tail = 0;

	if (section == NULL)
		return 0;

	DBG(DBG_INFO, "section: '%s'", section->name);

	entry = section;
	while (entry->name != NULL) {

		if (entry->type == CONF_TYPE(CONF_SECTION)) {
			queue[tail++] = (conf_entry_t *) entry;
		} else {
			if (entry->type->t_get(entry, buf)) {
				fprintf(f, "%s = %s\n", entry->name, buf);
			} else {
				/* Assuming the above failure means configuration stub. */
				fprintf(f, "%s = %s\n", entry->name, NULL_STRING);
			}
			count++;
		}
		entry++;
	}

	for (head = 0; head < tail; head++) {
		entry = queue[head];
		fprintf(f, "\n");
		if ((branch != NULL) && (*branch != '\0'))
			sprintf(buf, "%s/%s", branch, entry->name);
		else
			strcpy(buf, entry->name);
		fprintf(f, "[%s]\n", buf);
		count += write_section(f, (conf_entry_t *) entry->p, buf);
	}

	return count;
}

int conf_save(const char *path, conf_entry_t *root)
{
	FILE *f;
	int count;

	if (!(f = fopen(path, "w+"))) {
		DBG(DBG_ERROR, "fopen('%s') fail!", path);
		return -1;
	};

	count = write_section(f, root, NULL);

	fclose(f);

	return count;
}

int conf_dump(conf_entry_t *root)
{
	return write_section(stdout, root, NULL);
}



