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

#include "cmd.h"

#include "debug.h"
#include "sysconf.h"
#include "conf.h"

#define Q15(VAL) (((int32_t)((VAL) * (1 << 16)) + 1) >> 1)

struct sysconf syscfg = {
	.debug_enabled = false,
	.quiet = true,
	.root_dir = "/app",
	.stderr_fname = "/dev/null",

	.av = {
		.enabled = true,
		.capture = true,
#ifdef DEBUG
		.cap_tmo = 5,
		.quiet = false,
#else
		.cap_tmo = 4,
		.quiet = true,
#endif

		.server = "/app/dav_srv",
		.audio = {
			.enabled = false,
			.codec = AV_AUDIO_CODEC_G711,
			.sps = AV_AUDIO_SPS_8K,
			.bitrate = AV_AUDIO_BITRATE_64K
		},
		.video = {
			.chan[0] = {
				.enabled = true,
				.resolution = AV_VIDEO_RES_VGA,
				.framerate = Q15(30),
				.encoder = AV_VIDEO_ENCODER_H264,
				.quality = AV_VIDEO_QUALITY_MEDIUM,
				.ratectrl = AV_VIDEO_RATECTRL_VBR
			}, 
			.chan[1] = {
				.enabled = true,
				.resolution = AV_VIDEO_RES_360P,
				.framerate = Q15(30),
				.encoder = AV_VIDEO_ENCODER_MJPEG,
				.quality = AV_VIDEO_QUALITY_MEDIUM,
				.ratectrl = AV_VIDEO_RATECTRL_VBR
			} 
		},
		.md =  {
			.enabled = false,
			.timeout = 500,
			.level = AV_MD_LEVEL_HIGH,
			.size = AV_MD_SIZE_SMALL
		},
		.exp =  {
			.meter_mode = AV_EXP_METER_CENTER,
			.lens_type = AV_EXP_LENS_DETECT,
			.gain_priority = false,
			.shut_range = AV_EXP_SHUT_RANGE_X1,
			.gain_range = AV_EXP_GAIN_RANGE_X16,
			.flickerless_mode = AV_EXP_FLICKERLESS_OFF,
			.low_lux_bw = false
		},
		.pict =  {
			.brightness = 128,
			.contrast = 128,
			.saturation = 128
		},
		.opt =  {
			.osd_clk = false,
		},
		.rtsp =  {
			.basic_auth = false, 
			.max_clients = 2
		}
	},
	.web = {
		.port = 80,
		.root_dir = "/app/web",
		.passwd = "/cfg/passwd"
	},
	.net = {
		.ip_conf = "/cfg/ip.conf",
		.hostname = "coockapoo",
		.upnp_enabled = true,
		.websrv_enabled = true
	},
	.time = {
		.ntp = {
			.enabled = false,
			.server = "time.nrc.ca"
		},
		.tz_name = "America/Toronto",
		.localtime = "/cfg/localtime"
	},
	.alarms = {
		.io = {
			.in[0] = {
				.mode = INPUT_ON_OFF,
				.inv = false
			},
			.in[1] = {
				.mode = INPUT_ON_OFF,
				.inv = false
			},
			.out[0] = {
				.mode = OUTPUT_NO
			},
		},
		.sink[ALRM_SINK_OUT] = {
			.ev_set = 0,
			.ev_rst = 0,
			.tmo_ms = 500
		},
		.sink[ALRM_SINK_OSD] = {
			.ev_set = 0,
			.ev_rst = 0,
			.tmo_ms = 500
		}
	}
};

BEGIN_SECTION(conf_web)
	DEFINE_UINT16("port", &syscfg.web.port)
	DEFINE_STRINGCNT("root", &syscfg.web.root_dir, WEBSRV_DIR_LEN_MAX)
	DEFINE_STRINGCNT("passwd", &syscfg.web.passwd, 
					 PASSWD_PATH_LEN_MAX)
END_SECTION

BEGIN_SECTION(conf_net)
	DEFINE_STRINGCNT("ip.conf", &syscfg.net.ip_conf, IP_CONF_LEN_MAX)
	DEFINE_STRINGCNT("hostname", &syscfg.net.hostname, HOSTNAME_LEN_MAX)
	DEFINE_BOOLEAN("upnp", &syscfg.net.upnp_enabled)
	DEFINE_BOOLEAN("websrv", &syscfg.net.websrv_enabled)
END_SECTION

BEGIN_SECTION(conf_time)
	DEFINE_BOOLEAN("ntp_enabled", &syscfg.time.ntp.enabled)
	DEFINE_STRINGCNT("ntp_server", &syscfg.time.ntp.server, 
					 NTP_SERVER_LEN_MAX )
	DEFINE_STRINGCNT("tz", &syscfg.time.tz_name, TZ_NAME_LEN_MAX)
	DEFINE_STRINGCNT("localtime", &syscfg.time.localtime, LOCALTIME_LEN_MAX)
END_SECTION

BEGIN_SECTION(conf_alarms_io)
	DEFINE_UINT8("in1_mode", &syscfg.alarms.io.in[0].mode)
	DEFINE_BOOLEAN("in1_inv", &syscfg.alarms.io.in[0].inv)
	DEFINE_UINT8("in2_mode", &syscfg.alarms.io.in[1].mode)
	DEFINE_BOOLEAN("in2_inv", &syscfg.alarms.io.in[1].inv)
	DEFINE_UINT8("out1_mode", &syscfg.alarms.io.out[0].mode)
END_SECTION

BEGIN_SECTION(conf_alarms_sink)
	DEFINE_UINT16("out_set", &syscfg.alarms.sink[ALRM_SINK_OUT].ev_set)
	DEFINE_UINT16("out_reset", &syscfg.alarms.sink[ALRM_SINK_OUT].ev_rst)
	DEFINE_UINT16("out_tmo", &syscfg.alarms.sink[ALRM_SINK_OUT].tmo_ms)
	DEFINE_UINT16("osd_set", &syscfg.alarms.sink[ALRM_SINK_OSD].ev_set)
	DEFINE_UINT16("osd_reset", &syscfg.alarms.sink[ALRM_SINK_OSD].ev_rst)
	DEFINE_UINT16("osd_tmo", &syscfg.alarms.sink[ALRM_SINK_OSD].tmo_ms)
END_SECTION

BEGIN_SECTION(conf_alarms)
	DEFINE_SECTION("io", &conf_alarms_io)
	DEFINE_SECTION("sink", &conf_alarms_sink)
END_SECTION

#define EXTERN_SECTION(X) extern conf_entry_t X[];

BEGIN_SECTION(conf_audio)
	DEFINE_BOOLEAN("enabled", &syscfg.av.audio.enabled)
	DEFINE_UINT8("codec", &syscfg.av.audio.codec)
	DEFINE_UINT8("framerate", &syscfg.av.audio.sps)
	DEFINE_UINT8("bitrate", &syscfg.av.audio.bitrate)
END_SECTION

BEGIN_SECTION(conf_video_chan0)
	DEFINE_BOOLEAN("enabled", &syscfg.av.video.chan[0].enabled)
	DEFINE_UINT8("resolution", &syscfg.av.video.chan[0].resolution)
	DEFINE_UINT32("framerate", &syscfg.av.video.chan[0].framerate)
	DEFINE_UINT8("encoder", &syscfg.av.video.chan[0].encoder)
	DEFINE_UINT8("quality", &syscfg.av.video.chan[0].quality)
	DEFINE_UINT8("ratectrl", &syscfg.av.video.chan[0].ratectrl)
END_SECTION

BEGIN_SECTION(conf_video_chan1)
	DEFINE_BOOLEAN("enabled", &syscfg.av.video.chan[1].enabled)
	DEFINE_UINT8("resolution", &syscfg.av.video.chan[1].resolution)
	DEFINE_UINT32("framerate", &syscfg.av.video.chan[1].framerate)
	DEFINE_UINT8("encoder", &syscfg.av.video.chan[1].encoder)
	DEFINE_UINT8("quality", &syscfg.av.video.chan[1].quality)
	DEFINE_UINT8("ratectrl", &syscfg.av.video.chan[1].ratectrl)
END_SECTION

BEGIN_SECTION(conf_video)
	DEFINE_SECTION("chan0", &conf_video_chan0)
	DEFINE_SECTION("chan1", &conf_video_chan1)
END_SECTION

BEGIN_SECTION(conf_motion_detect)
	DEFINE_BOOLEAN("enabled", &syscfg.av.md.enabled)
	DEFINE_UINT16("timeout", &syscfg.av.md.timeout)
	DEFINE_UINT8("level", &syscfg.av.md.level)
	DEFINE_UINT8("size", &syscfg.av.md.size)
END_SECTION

BEGIN_SECTION(conf_exposure)
	DEFINE_UINT8("meter", &syscfg.av.exp.meter_mode)
	DEFINE_UINT8("lens", &syscfg.av.exp.lens_type)
	DEFINE_UINT8("shut_range", &syscfg.av.exp.shut_range)
	DEFINE_UINT8("gain_range", &syscfg.av.exp.gain_range)
	DEFINE_BOOLEAN("gain_priority", &syscfg.av.exp.gain_priority)
	DEFINE_BOOLEAN("flickerless", &syscfg.av.exp.flickerless_mode)
	DEFINE_BOOLEAN("low_lux_bw", &syscfg.av.exp.low_lux_bw)
END_SECTION

BEGIN_SECTION(conf_picture)
	DEFINE_UINT8("bright", &syscfg.av.pict.brightness)
	DEFINE_UINT8("contrast", &syscfg.av.pict.contrast)
	DEFINE_UINT8("saturation", &syscfg.av.pict.saturation)
END_SECTION

BEGIN_SECTION(conf_opt)
	DEFINE_BOOLEAN("osd_clk", &syscfg.av.opt.osd_clk)
END_SECTION

BEGIN_SECTION(conf_rtsp)
	DEFINE_BOOLEAN("basic_auth", &syscfg.av.rtsp.basic_auth)
	DEFINE_UINT8("max_clients", &syscfg.av.rtsp.max_clients)
END_SECTION

BEGIN_SECTION(conf_media)
	DEFINE_BOOLEAN("enabled", &syscfg.av.enabled)
	DEFINE_BOOLEAN("capture", &syscfg.av.capture)
	DEFINE_UINT8("cap_tmo", &syscfg.av.cap_tmo)
	DEFINE_BOOLEAN("quiet", &syscfg.av.quiet)
	DEFINE_STRING("server", syscfg.av.server)
	DEFINE_SECTION("audio", &conf_audio)
	DEFINE_SECTION("video", &conf_video)
	DEFINE_SECTION("exposure", &conf_exposure)
	DEFINE_SECTION("motion_detect", &conf_motion_detect)
	DEFINE_SECTION("picture", &conf_picture)
	DEFINE_SECTION("opt", &conf_opt)
	DEFINE_SECTION("rtsp", &conf_rtsp)
END_SECTION

BEGIN_SECTION(conf_root)
	DEFINE_BOOLEAN("debug", &syscfg.debug_enabled)
	DEFINE_BOOLEAN("quiet", &syscfg.quiet)
	DEFINE_STRINGCNT("stderr", &syscfg.stderr_fname, STDERR_FNAME_LEN_MAX)
	DEFINE_STRINGCNT("root_dir", &syscfg.root_dir, ROOT_DIR_LEN_MAX)
	DEFINE_SECTION("network", &conf_net)
	DEFINE_SECTION("web", &conf_web)
	DEFINE_SECTION("media", &conf_media)
	DEFINE_SECTION("time", &conf_time)
	DEFINE_SECTION("alarms", &conf_alarms)
END_SECTION

static struct {
	bool started;
	uint32_t save_req;
	uint32_t save_ack;
	char path[128];
//	struct timer * tmr;
	timer_t tmr;
	pthread_mutex_t mutex;
} confsvc;

static int sysconf_commit(void)
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

int sysconf_load(void)
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

int sysconf_save(void)
{
	int ret;

	if (!confsvc.started) {
		DBG(DBG_WARNING, "Config service not started.");
		return -1;
	}

	pthread_mutex_lock(&confsvc.mutex);

	confsvc.save_req++;
	ret = sysconf_commit();

	pthread_mutex_unlock(&confsvc.mutex);
	return ret;
}

static void sysconf_save_req_tmo(union sigval arg)
{
	DBG(DBG_INFO, "saving global configuration...");

	pthread_mutex_lock(&confsvc.mutex);
	sysconf_commit();
	pthread_mutex_unlock(&confsvc.mutex);
}

int sysconf_save_req(void)
{
	struct itimerspec its;

	if (!confsvc.started) {
		DBG(DBG_WARNING, "Config service not started.");
		return -1;
	}

	DBG(DBG_INFO, "scheduling save ...");

	confsvc.save_req++;

	/* restart timer ... */
	its.it_value.tv_sec = 15;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (timer_settime(confsvc.tmr, 0, &its, NULL) < 0) {
		DBG(DBG_WARNING, "timer_settime() failed: %s!", strerror(errno));
	}

	return 0;
}

int sysconf_delete(void)
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

int sysconf_start(const char * path) 
{
	struct sigevent sev;
	pthread_attr_t attr;

	if (confsvc.started)
		return 0;

	if (path == NULL)
		return -1;

	strcpy(confsvc.path, path);

	pthread_mutex_init(&confsvc.mutex, NULL);

	confsvc.save_req = 0;
	confsvc.save_ack = 0;

 /* Create the timer */
	pthread_attr_init(&attr);

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = sysconf_save_req_tmo;
	sev.sigev_notify_attributes = &attr;
	sev.sigev_value.sival_ptr = &confsvc.tmr;

	if (timer_create(CLOCK_MONOTONIC, &sev, &confsvc.tmr) < 0) {
		DBG(DBG_ERROR, "timer_create() failed: %s.", strerror(errno));
		return -1;
	}

	pthread_attr_destroy(&attr);

	DBG(DBG_TRACE, "Config service started (%s).", path);

	confsvc.started = true;

	return 0;
}


int sysconf_stop(void)
{
	if (!confsvc.started)
		return -1;

	pthread_mutex_lock(&confsvc.mutex);

	/* commit any pending save request */
	sysconf_commit();

	timer_delete(confsvc.tmr);

	pthread_mutex_unlock(&confsvc.mutex);

	pthread_mutex_destroy(&confsvc.mutex);

	DBG(DBG_TRACE, "Config service stopped.");

	confsvc.started = false;
	return 0;
}

