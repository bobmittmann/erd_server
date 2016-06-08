/* $Id: lookup.c,v 1.1 2005/04/28 20:15:00 bob Exp $ 
 *
 * File:	lookup.c
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
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "conf.h"

int conf_lookup(char * pathname, const char ** searchpath, 
				const char ** filename)
{
	char * fn;
	char * sp;
	struct stat buf;
	int i;
	int j;

	if ((pathname == NULL) || (searchpath == NULL) || (filename == NULL))
		return -1;

	for (i = 0; filename[i] != NULL; i++) {
		for(fn = (char *)filename[i]; (*fn != '\0') && (*fn != '/') 
				&& (isspace(*fn)); fn++);
		if (*fn == '\0')
			continue;

		for (j = 0; searchpath[j] != NULL; j++) {

			for(sp = (char *)searchpath[j]; (*sp != '\0') && 
				(isspace(*sp)); sp++);

			/* very basic shell expansion */
			if (*sp == '~') {
				strcpy(pathname, getenv("HOME"));
			} else 
				strcpy(pathname, sp);
			strcat(pathname, "/");
			strcat(pathname, fn);

			if (stat(pathname, &buf) == 0) {

				if (S_ISREG(buf.st_mode))
					return 0;

			}
		}
	}

	return -1;
}

