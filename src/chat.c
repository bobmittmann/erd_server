#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <serial.h>

static struct {
	bool debug;
	unsigned int timeout;
} chat = {
	.debug = true,
	.timeout = 200
};

static int tohex(int c)
{
	return (c > 9) ? c - 9 + 'a' : c + '0';
}

#define CHAT_LOG_LINE_MAX 72

void chat_xmt_log(char * s)
{
	char buf[2 * CHAT_LOG_LINE_MAX];
	char * cp;
	int i;
	int c;

	if (!chat.debug)
		return;

	cp = s;
	do {
		for (i = 0; i < CHAT_LOG_LINE_MAX;) {
			c = *cp++;
			if (c == '\0')
				break;
			if (c == '\r') { 
				buf[i++] = '\\';
				buf[i++] = 'r';
			} else if (c == '\t') {
				buf[i++] = '\\';
				buf[i++] = 't';
			} else if (c == '\n') {
				buf[i++] = '\\';
				buf[i++] = 'n';
				break;
			} else if (c < ' ') {
				buf[i++] = '\\';
				buf[i++] = tohex(c & 0xf);
				buf[i++] = tohex((c >> 4) & 0xf);
			} else
				buf[i++] = c;
		}
		buf[i] = '\0';

		printf("--> \"%s\"\n", buf);
		fflush(stdout);
	} while (c != '\0');

}

void chat_recv_log(int c)
{
	static int i = 0;
	static char buf[2 * CHAT_LOG_LINE_MAX];

	if (!chat.debug)
		return;

	if (c == '\0') { 
		/* do nothing */
	} else if (c == '\r') { 
		buf[i++] = '\\';
		buf[i++] = 'r';
	} else if (c == '\t') {
		buf[i++] = '\\';
		buf[i++] = 't';
	} else if (c == '\n') {
		buf[i++] = '\\';
		buf[i++] = 'n';
	} else if (c < ' ') {
		buf[i++] = '\\';
		buf[i++] = tohex(c & 0xf);
		buf[i++] = tohex((c >> 4) & 0xf);
	} else
		buf[i++] = c;

	if ((i >= CHAT_LOG_LINE_MAX) || (c == '\0') || (c == '\n')) {
		if (i > 0) {
			buf[i] = '\0';
			printf("<-- \"%s\"\n", buf);
			fflush(stdout);
			i = 0;
		}
	}
}

#define CHAT_WAIT_LIST_MAX 64

int serial_chat(struct serial_dev * ser, char * req, ...)
{
	char * rval[CHAT_WAIT_LIST_MAX];
	int pos[CHAT_WAIT_LIST_MAX];
	int rcnt;
	char buf[1];
	va_list ap;
	int ret;
	int i;
	int n;

	va_start(ap, req);

	for (i = 0; i < CHAT_WAIT_LIST_MAX; ++i) {
		rval[i] = va_arg(ap, char *);
		if (rval[i] == NULL)
			break;
	}
	rcnt = i;

	va_end(ap);

	/* send the request */
	n = strlen(req);
	chat_xmt_log(req);
	if ((ret = serial_send(ser, req, n)) < 0) {
		return ret;
	}

	/* reset the string matching pointers */
	for (i = 0; i < rcnt; ++i)
		pos[i] = 0;	

	/* wait for the response */
	while (1) {
		int c;

		for (i = 0; i < rcnt; ++i) {
			if (rval[i][pos[i]] == '\0') {
				/* flush the log */
				chat_recv_log('\0');
				return i + 1;
			}
		}	

		ret = serial_recv(ser, buf, 1, chat.timeout);
		c = *buf;

		chat_recv_log(c);

		if (ret == 0) {
			break;
		}

		if (ret < 0) {
			break;
		}

		for (i = 0; i < rcnt; ++i) {
			if (c == rval[i][pos[i]])
				pos[i]++;
			else
				pos[i] = 0;
		}	
	} 

	/* flush the log */
	chat_recv_log('\0');

	return ret;
}

void chat_timeout(unsigned int tmo)
{
	chat.timeout = tmo;
}

void chat_debug(bool enable)
{
	chat.debug = enable;
}

