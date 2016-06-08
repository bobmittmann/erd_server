/* 
 * File:	ctrl.c
 * Author:	Robinson Mittmann (bobmittmann@gmail.com)
 * Comment:
 * Copyright(c) 2012 i-LAX Electronics Inc. All Rights Reserved.
 */

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdbool.h>

#include "debug.h"
#include "syscfg.h"
#include "conf.h"

struct syscfg syscfg = {
	.debug_enabled = false,
	.quiet = true,
	.session = {
		.name = "Default",
		.port = "COM1",
		.tmo_ms = 500
	}
};

BEGIN_SECTION(conf_session)
	DEFINE_STRINGCNT("name", &syscfg.session.name, SESSION_NAME_MAX)
	DEFINE_STRINGCNT("port", &syscfg.session.port, SESSION_PORT_MAX)
	DEFINE_UINT32("out_tmo", &syscfg.session.tmo_ms)
END_SECTION

BEGIN_SECTION(conf_root)
	DEFINE_BOOLEAN("debug", &syscfg.debug_enabled)
	DEFINE_BOOLEAN("quiet", &syscfg.quiet)
	DEFINE_SECTION("session", &conf_session)
END_SECTION

static struct {
	bool started;
	uint32_t save_req;
	uint32_t save_ack;
	char path[128];
	pthread_mutex_t mutex;
} confsvc;

static int syscfg_commit(void)
{
	int ret;

	if (confsvc.save_ack == confsvc.save_req) {
		return 0;
	}

	DBG(DBG_TRACE, "commiting changes ...");

	if ((ret = conf_save(confsvc.path, conf_root)) < 0) {
		DBG(DBG_WARNING, "conf_save() failed!");
	}

	confsvc.save_ack = confsvc.save_req;

	return ret;
}

int syscfg_load(void)
{
	int ret;

	if (!confsvc.started) {
		DBG(DBG_WARNING, "Config service not started.");
		return -1;
	}

	pthread_mutex_lock(&confsvc.mutex);

	if ((ret = conf_load(confsvc.path, conf_root)) < 0) {
		DBG(DBG_WARNING, "conf_load() failed!");
	}

	pthread_mutex_unlock(&confsvc.mutex);
	return 0;
}

int syscfg_save(void)
{
	int ret;

	if (!confsvc.started) {
		DBG(DBG_WARNING, "Config service not started.");
		return -1;
	}

	pthread_mutex_lock(&confsvc.mutex);

	confsvc.save_req++;
	ret = syscfg_commit();

	pthread_mutex_unlock(&confsvc.mutex);
	return ret;
}

int syscfg_delete(void)
{
	if (confsvc.started) {
		DBG(DBG_WARNING, "Config service is running.");
		pthread_mutex_lock(&confsvc.mutex);
	}

	/* remove existing file */
	if (unlink(confsvc.path) < 0) {
		DBG(DBG_WARNING, "unlink() failed: %s!", strerror(errno));
	} else {
		DBG(DBG_TRACE, "Configuration file deleted.");
	}

	if (confsvc.started) {
		pthread_mutex_unlock(&confsvc.mutex);
	}

	return 0;
}

int syscfg_start(const char * path) 
{
	if (confsvc.started)
		return 0;

	if (path == NULL)
		return -1;

	strcpy(confsvc.path, path);

	pthread_mutex_init(&confsvc.mutex, NULL);

	confsvc.save_req = 0;
	confsvc.save_ack = 0;

 /* Create the timer */

	DBG(DBG_TRACE, "Config service started (%s).", path);

	confsvc.started = true;

	return 0;
}


int syscfg_stop(void)
{
	if (!confsvc.started)
		return -1;

	pthread_mutex_lock(&confsvc.mutex);

	/* commit any pending save request */
	syscfg_commit();

	pthread_mutex_unlock(&confsvc.mutex);

	pthread_mutex_destroy(&confsvc.mutex);

	DBG(DBG_TRACE, "Config service stopped.");

	confsvc.started = false;
	return 0;
}

