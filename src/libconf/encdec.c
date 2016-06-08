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

static int void_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%p", var->p);
	return 1;
}

static int void_set(conf_entry_t *var, const char *s)
{
	sscanf(s, "%p", &(var->p));
	return 1;
}

static int int_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%i", *(int32_t *) (var->p));
	return 1;
}

static int int_set(conf_entry_t *var, const char *s)
{
	int32_t *p;

	if (var->p == NULL) {
		var->p = malloc(sizeof(int32_t));
	}

	p = (int32_t *) var->p;

	*p = strtol(s, NULL, 0);

	return 1;
}

static int uint_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%u", *(uint32_t *) (var->p));
	return 1;
}

static int uint_set(conf_entry_t *var, const char *s)
{
	uint32_t *p;

	if (var->p == NULL) {
		var->p = malloc(sizeof(uint32_t));
	}

	p = (uint32_t *) var->p;

	*p = strtoul(s, NULL, 0);

	return 1;
}

static int float_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%f", *(double *) (var->p));
	return 1;
}

static int float_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(double));
	}
	sscanf(s, "%lf", (double *) (var->p));
	return 1;
}

static int string_get(conf_entry_t *var, char *s)
{
	char * cp;			/* source */

	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	cp = (char *)(var->p);
	sprintf(s, "\"%s\"", cp);

	return 1;
}

static int string_set(conf_entry_t *var, const char *s)
{
	char *cp;			/* source */
	int len;
	char quote;
	char *ep;

	if ((!s) || (!var))
		return 0;

	cp = (char *) s;
	LTRIM(cp);
	if ((*cp == '"') || (*cp == '\'')) {
		quote = *cp;
		cp++;
		if ((ep = strchr(cp, quote)) != NULL)
			len = ep - cp;
		else {
#ifdef DEBUG
			fprintf(stderr, "Unterminated string.\n");
#endif
			len = 0;
		}

	} else {
		len = strlen(cp);
	}

	if (var->p == NULL) {
		var->p = malloc(len + 1);
	}

	if (var->len > 0)
		len = MIN(len, var->len);

	strncpy((char *) (var->p), cp, len);
	cp = (char *)(var->p);
	cp[len] = '\0';

	return 1;
}

static int bool_get(conf_entry_t *var, char *s)
{
	if ((*(bool *) (var->p)) == 0)
		sprintf(s, "False");
	else
		sprintf(s, "True");
	return 1;
}

static int bool_set(conf_entry_t *var, const char *s)
{
	char *cp;			/* source */

	if (var->p == NULL) {
		var->p = malloc(sizeof(int));
	}

	cp = (char *) s;
	LTRIM(cp);

	if (!strncasecmp(cp, "TRUE", 4) ||
		!strncasecmp(cp, "YES", 3) ||
		!strncasecmp(cp, "ON", 2) || 
		!strncasecmp(cp, "1", 1)) {
		*((bool *) var->p) = true;
		return 1;
	}

	if (!strncasecmp(cp, "FALSE", 5) ||
		!strncasecmp(cp, "NO", 2) ||
		!strncasecmp(cp, "OFF", 3) || 
		!strncasecmp(cp, "0", 1)) {
		*((bool *) var->p) = false;
		return 1;
	}

	return 0;
}

static int char_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	//printf("*** %s: var '%s' = '%c'\n", __FUNCTION__, var->name, *(char *)(var->p));

	sprintf(s, "%c", *(char *) (var->p));
	return 1;
}

static int char_set(conf_entry_t *var, const char *s)
{
	char *cp;			/* source */

	if (var->p == NULL) {
		var->p = malloc(sizeof(char));
	}

	cp = (char *) s;
	LTRIM(cp);

	//printf("*** %s: var '%s' source='%s'\n", __FUNCTION__, var->name, cp);

	sscanf(cp, "%c", (char *) (var->p));
	return 1;
}

/*
 * 8 bits integers
 */
static int int8_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%i", *(int8_t *) (var->p));
	return 1;
}

static int int8_set(conf_entry_t *var, const char *s)
{
	int8_t *p;

	if (var->p == NULL) {
		var->p = malloc(sizeof(int8_t));
	}

	p = (int8_t *) var->p;

	*p = strtol(s, NULL, 0);

	return 1;
}

static int uint8_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%u", *(uint8_t *) (var->p));
	return 1;
}

static int uint8_set(conf_entry_t *var, const char *s)
{
	uint8_t *p;

	if (var->p == NULL) {
		var->p = malloc(sizeof(uint8_t));
	}

	p = (uint8_t *) var->p;

	*p = strtoul(s, NULL, 0);

	return 1;
}

static int hex8_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "0x%02X", *(uint8_t*) (var->p));
	return 1;
}

static int hex8_set(conf_entry_t *var, const char *s)
{
	uint32_t val;

	if (var->p == NULL) {
		var->p = malloc(sizeof(uint8_t));
	}
	sscanf(s, "%X", &val);

	*(uint8_t *)(var->p) = val;
	return 1;
}

static int bin8_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}
	s = "";

	return 0;
}

static int bin8_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(unsigned char));
	}

	return 0;
}

static int oct8_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}
	s = "";

	return 0;
}

static int oct8_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(unsigned char));
	}

	return 0;
}

/*
 * 16 bits integers
 */
static int int16_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%i", *(int16_t *) (var->p));
	return 1;
}

static int int16_set(conf_entry_t *var, const char *s)
{
	int16_t *p;

	if (var->p == NULL) {
		var->p = malloc(sizeof(int16_t));
	}

	p = (int16_t *) var->p;

	*p = strtol(s, NULL, 0);

	return 1;
}

static int uint16_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%u", *(uint16_t *) (var->p));
	return 1;
}

static int uint16_set(conf_entry_t *var, const char *s)
{
	uint16_t *p;

	if (var->p == NULL) {
		var->p = malloc(sizeof(uint16_t));
	}

	p = (uint16_t *) var->p;

	*p = strtoul(s, NULL, 0);

	return 1;
}

static int hex16_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "0x%04X", *(uint16_t *) (var->p));
	return 1;
}

static int hex16_set(conf_entry_t *var, const char *s)
{
	uint32_t val;

	if (var->p == NULL) {
		var->p = malloc(sizeof(uint16_t));
	}
	sscanf(s, "%X", &val);
	*(uint16_t *)(var->p) = val;
	return 1;
}

static int bin16_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}
	s = "";

	return 0;
}

static int bin16_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(short int));
	}

	return 0;
}

static int oct16_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}
	s = "";

	return 0;
}

static int oct16_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(short int));
	}

	return 0;
}

/*
 * 32 bits integers
 */

static int hex32_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		sprintf(s, "NULL");
		return 0;
	}

	sprintf(s, "0x%08X", *(uint32_t *) (var->p));
	return 1;
}

static int hex32_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(uint32_t));
	}

	sscanf(s, "%X", (uint32_t *) (var->p));
	return 1;
}

static int bin32_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}
	s = "";

	return 0;
}

static int bin32_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(unsigned long));
	}
	return 0;
}

static int oct32_get(conf_entry_t *var, char *s)
{
	s = "";

	return 0;
}

static int oct32_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(uint32_t));
	}

	return 0;
}

/*
 * 32 bits integers
 */
static int int64_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%lli", *(int64_t *) (var->p));
	return 1;
}

static int int64_set(conf_entry_t *var, const char *s)
{
	int64_t *p;

	if (var->p == NULL) {
		var->p = malloc(sizeof(int64_t));
	}

	p = (int64_t *) var->p;

	*p = strtoll(s, NULL, 0);

	return 1;
}

static int uint64_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "%llu", *(uint64_t *) (var->p));
	return 1;
}

static int uint64_set(conf_entry_t *var, const char *s)
{
	uint64_t *p;

	if (var->p == NULL) {
		var->p = malloc(sizeof(uint64_t));
	}

	p = (uint64_t *) var->p;

	*p = strtoull(s, NULL, 0);

	return 1;
}

static int hex64_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}

	sprintf(s, "0x%016llX", *(uint64_t *) (var->p));

	return 1;
}

static int hex64_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(uint64_t));
	}

	sscanf(s, "%llX", (uint64_t *) (var->p));

	return 1;
}

static int bin64_get(conf_entry_t *var, char *s)
{
	s = "";

	return 0;
}

static int bin64_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(long long int));
	}

	return 0;
}
static int oct64_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		sprintf(s, "NULL");
		return 0;
	}
	s = "";

	return 0;
}

static int oct64_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(long long int));
	}

	return 0;
}

/*
 * network stuff
 */
static int ipv4addr_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		DBG(DBG_WARNING, "var->p == NULL!");
		sprintf(s, "NULL");
		return 0;
	}

	strcpy(s, inet_ntoa(*(struct in_addr *)var->p));
	return 1;
}

static int ipv4addr_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		DBG(DBG_WARNING, "var->p == NULL!");
		var->p = malloc(sizeof(struct in_addr));
	}

	inet_aton(s, (struct in_addr *)var->p);

	return 1;
}

static int rgb_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		sprintf(s, "NULL");
		return 0;
	}
	s = "";

	return 0;
}

static int rgb_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(uint32_t));
	}

	return 0;
}

static int rgbi_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}
	s = "";

	return 0;
}

static int rgbi_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(uint32_t));
	}

	return 0;
}

static int cymk_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}
	s = "";

	return 0;
}

static int cymk_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(sizeof(uint32_t));
	}

	return 0;
}

static int ratio_get(conf_entry_t *var, char *s)
{
	if (var->p == NULL) {
		strcpy(s, NULL_STRING);
		return 0;
	}
	s = "";

	return 0;
}

static int ratio_set(conf_entry_t *var, const char *s)
{
	if (var->p == NULL) {
		var->p = malloc(2 * sizeof(int));
	}

	return 0;
}
