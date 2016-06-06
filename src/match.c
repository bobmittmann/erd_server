#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

static int matchhere(char *regexp, char *text);

/* matchstar: search for c*regexp at beginning of text */
static int matchstar(int c, char *regexp, char *text)
{
	do {    /* a * matches zero or more instances */
		if (matchhere(regexp, text))
			return true;

	} while (*text != '\0' && (*text++ == c || c == '.'));

	return false;
}

/* matchhere: search for regexp at beginning of text */
static int matchhere(char *regexp, char *text)
{
	if (regexp[0] == '\0')
		return true;

	if (regexp[1] == '*')
		return matchstar(regexp[0], regexp + 2, text);

	if (regexp[0] == '$' && regexp[1] == '\0')
		return *text == '\0';

	if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
		return matchhere(regexp + 1, text + 1);

	return false;
}

/* match: search for regexp anywhere in text */
bool match(char *regexp, char *text)
{
	if (regexp[0] == '^')
		return matchhere(regexp + 1, text);

	do {    /* must look even if string is empty */
		if (matchhere(regexp, text))
			return true;

	} while (*text++ != '\0');

	return false;
}

