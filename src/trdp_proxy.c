#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <libgen.h>
#include <dirent.h>

#include "debug.h"
#include "chat.h"

#ifdef _WIN32
#include <windows.h>
struct serial_dev * win_serial_open(const char * com_port);
#endif

#include "serial.h"

/* -------------------------------------------------------------------------
 * Application startup
 * ------------------------------------------------------------------------- 
 */

struct serial_dev * ser;
static char * progname;
#if defined(_WIN32)
char * serial_port = "COM1";
#else
char * serial_port = "/dev/ttyS11";
#endif
int quiet = 0;

void cleanup(int sig)
{
	printf("That was all, folks\n");
	fflush(stdout);
}

extern struct terminal * logterm;
void term_puts(struct terminal * term, char * s);
int term_printf(struct terminal * term, const char * fmt, ...);
void msleep(unsigned int msec);

void trdp_proxy_main(void * arg) 
{
	char * port = serial_port;
	struct port_entry lst[32];
	int cnt;
	int i;
	
	printf("trdp_proxy_main()\n");
	fflush(stdout);

	cnt = serial_port_list(lst, 32);
	(void)cnt;
	for (i = 0; i < cnt; ++i) {
		term_printf(logterm, "\"%s\" \"%s\"\n", lst[i].path, lst[i].desc);
		fflush(stdout);
	}

	i = 0;
	for(;;) {
		
		if (i < cnt) {
			port = lst[i].path;
			i++;
			if (i == cnt)
				i = 0;
		}

		msleep(1000);

		DBG(DBG_INFO, "win_serial_open() ...");
		if ((ser = win_serial_open(port)) == NULL) {
			term_printf(logterm, "#WARN: can't open serial port: \"%s\"\n", 
						port);
		} else {
			DBG(DBG_INFO, "win_serial_open() ...");
			serial_close(ser);
		}

	}
}


#define APP_NAME "trdp_proxy"
#define VERSION_MAJOR 0
#define VERSION_MINOR 1

static void show_usage(void)
{
	fprintf(stderr, "Usage: %s [OPTION...] ALIAS\n", progname);
	fprintf(stderr, "  -h       Show this help message\n");
	fprintf(stderr, "  -v       Show version\n");
	fprintf(stderr, "  -p PORT  Set comm PORT (default: 'COM1')\n");
	fprintf(stderr, "  -q       Quiet\n");
	fprintf(stderr, "\n");
	fflush(stderr);
}

static void show_version(void)
{
	fprintf(stderr, "%s %d.%d\n", APP_NAME, VERSION_MAJOR, VERSION_MINOR);
	fflush(stderr);
}

static void parse_err(char * opt)
{
	fprintf(stderr, "%s: invalid option %s\n", progname, opt);
	fflush(stderr);
}

int parse_cmd_line(int argc, char *argv[]) 
{
	int c;

	/* the program name start just after the last slash */
#if defined(_WIN32)
	if ((progname = (char *)strrchr(argv[0], '\\')) == NULL)
#else
	if ((progname = (char *)strrchr(argv[0], '/')) == NULL)
#endif 
		progname = argv[0];
	else
		progname++;

	/* parse the command line options */
	while ((c = getopt(argc, argv, "dvhqp:")) > 0) {
		switch (c) {
		case 'v':
			show_version();
			return 0;
		case 'h':
			show_usage();
			return 1;
		case 'p':
			serial_port = optarg;
			break;
		case 'q':
			quiet++;
			break;
		default:
			parse_err(optarg);
			return 2;
		}
	}

	chat_debug(quiet ? false : true);

	if (optind == argc) {
		fprintf(stderr, "%s: missing parameter!\n", progname);
		fflush(stderr);
		return 3;
	}

	printf("- Serial port: '%s'\n", serial_port);
	fflush(stdout);

	return 0;
}

#if defined(_WIN32)
int WINAPI AppMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
				   LPSTR lpCmdLine, int nCmdShow)
{
	LPWSTR *szArglist;
	char ** argv;
	int argc;
	int i;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (NULL == szArglist ) {
		return 0;
	}
	argv = LocalAlloc(0, argc * sizeof(char *));
	for(i = 0; i < argc; i++) {
		int len = WideCharToMultiByte(CP_UTF8, 0, szArglist[i], -1, 
									  NULL, 0, NULL, NULL);
		argv[i] = LocalAlloc(0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, szArglist[i], -1,
							argv[i], len, NULL, NULL);
	}
	// Free memory allocated for CommandLineToArgvW arguments.
	LocalFree(szArglist);

	parse_cmd_line(argc, argv); 

	return AppMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

#else

int main(int argc, char *argv[]) 
{
	parse_cmd_line(argc, *argv[]); 

	signal(SIGINT, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGBREAK, cleanup);

	return 0;
}

#endif

