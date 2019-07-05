/*
 * create 2019/07/03
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>        /* for gettimeofday() */
#include <sys/syscall.h>    /* for SYS_gettid */
#include <unistd.h>            /* for syscall() */

#include "log.h"
static log_level_e g_log_level = LOG_LEVEL_INFO;
static FILE *g_out_fp = NULL;
static DAY day_value;

static char need_check=0;
static
void log_write(log_level_e level, const char *file, int line, const char *fmt, va_list ap)
{
    char content[1024];
    char time_head[28]; /* 2019-07-03 12:37:00 */
    const char *level_text = "NONE";
    struct timeval tv;
    time_t tt;
    struct tm *tm;

    if (g_log_level < level)
    {
        return;
    }

    if (LOG_LEVEL_LOG == level)
    {
        level_text = "LOG";
    }
    else if (LOG_LEVEL_ERROR == level)
    {
        level_text = "ERROR";
    }
    else if (LOG_LEVEL_WARN == level)
    {
        level_text = "WARNING";
    }
    else if (LOG_LEVEL_INFO == level)
    {
        level_text = "INFO";
    }
    else if (LOG_LEVEL_DEBUG == level)
    {
        level_text = "DEBUG";
    }
    memset(time_head, 0, sizeof(time_head));
    gettimeofday(&tv, NULL);
    tt = tv.tv_sec;
    tm = localtime(&tt);
    snprintf(time_head, sizeof(time_head)-1, "%u-%02u-%02u %02u:%02u:%02u",
        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	if(ZX_UNLIKELY(!need_check||NULL == g_out_fp)){
    day_value.month=tm->tm_mon+1;
    day_value.day=tm->tm_mday;
    need_check=1;
    char time_head[28];
        	memset(time_head, 0, sizeof(time_head));
        	snprintf(time_head, sizeof(time_head)-1, "%u-%02u-%02u.log",
        	        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
        	log_file(time_head);
    }
	else
    {
    	if(ZX_UNLIKELY(tm->tm_mday!=day_value.day)){
    	char time_head[28];
    	memset(time_head, 0, sizeof(time_head));
    	snprintf(time_head, sizeof(time_head)-1, "%u-%02u-%02u.log",
    	        tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday);
    	log_file(time_head);
    	day_value.month=tm->tm_mon+1;
    	day_value.day=tm->tm_mday;
    	}
    }
    memset(content, 0, sizeof(content));
    vsnprintf(content, sizeof(content)-1, fmt, ap);
    fprintf(g_out_fp, "%s|%s|%s|%s|\n", time_head, "EnSSL",level_text, content);
    return;
}

static print_f g_print = log_write;

void log_init(print_f printcb)
{
    if (NULL != printcb)
    {
        g_print = printcb;
    }
}

void log_file(const char *file)
{
    FILE *fp = fopen(file, "w");
    if (NULL != fp)
    {
        setvbuf(fp, NULL, _IONBF, 0);
        g_out_fp = fp;
    }

    return;
}

void log_setlevel(log_level_e level)
{
    g_log_level = level;
}

void log_print(log_level_e level, const char *file, int line, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    g_print(level, file, line, fmt, ap);
    va_end(ap);

    return;
}
