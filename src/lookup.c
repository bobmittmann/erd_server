#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <libgen.h>
#include <dirent.h>

#include "match.h"


char * file_lookup(char * path, char * regexp)
{
	DIR * dir;
	struct dirent * ent;

	if ((dir = opendir(path)) == NULL) {
		fprintf(stderr, "ERROR: %s: opendir(): %s.\n",
				__func__, strerror(errno));
		fflush(stderr);

		return NULL;
	}

	/* print all the files and directories within directory */
	while ((ent = readdir(dir)) != NULL) {
		if (match(regexp, ent->d_name)) {
			break;
		}
	}

	closedir(dir);

	if (ent == NULL)
		return NULL;

	return ent->d_name;
}

