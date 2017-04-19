/*##########################################################################

 THALES

 COPYRIGHT THALES

 NAE : 07-201
 Maximum level content in the matrix of transfer : 2

 ############################################################################*/
/**
 @application   HLS
 @file          hardware_definition.c
 @author        GAGNAT Pascal
 @date          April 22 2013

 \brief
 Exports services for log trace.

 Trace logging consists in recording log message (with log__write function)
 A log message is a text string of LOG_MSG_MAX_SIZE characters max long.
 Messages are tagged with a level, so that they are displayed only if
 their level are lower than log level.

 Log messages are either written in log files named hls_0.log, hls_1.log
 and hls_2.log or sent through syslog service.
 Log file size can't exceed LOG__C_FILE_SIZE_MAX characters
 As soon log component is initialized hls_1 log file is filled first.
 After, hls_1 and hls_2 log files are filled alternatively.

 Trace level must be set at start and can be changed after with
 log__set_level function.

 This component must be initialized and start as soon as possible in
 sofware initialization section.

 Evolutions  :
 Author      :
 Date        :
 PCR         :
 Description : */

/*############################################################################*/
/*            INCLUDE SECTION                                                 */
/*----------------------------------------------------------------------------*/
#include <time.h>
#include <sys/time.h>
#include <sys/syslog.h>
#include "log.h"
//#include "env.h"
//#include "bbid.h"

/*############################################################################*/
/*            DEFINE SECTION                                                  */
/*----------------------------------------------------------------------------*/
#define LOG_FILE_NAME           "hls"
#define LOG_FILE_EXTENSION      "log"
#define LOG_MAX_FILE            2 /* Maximum number of log file */

#define C_MONTH_OFFSET 1
#define C_YEAR_OFFSET  1900

#define PATTERN_LOG_FILE        "%s.%s"
#define PATTERN_ARCHIV_FILE     "%s-%i.%s"
#define PATTERN_TIME_STAMP      "%02d/%02d/%04d %02d:%02d:%02d.%03d-%s-%s"

#define __FILE_ID_FOR_LOG__     1

/*############################################################################*/
/*            TYPEDEF SECTION                                                 */
/*----------------------------------------------------------------------------*/

/*############################################################################*/
/*            STATIC REMANENT VARIABLES SECTION                               */
/*----------------------------------------------------------------------------*/
static char *hls_id;
static LOG__MODE_ETYPE log_mode;
static int num_archive;
static LOG_LEVEL_ETYPE log_level;
static char log_current_file[255];

/*############################################################################*/
/*            LOCAL FUNCTIONS SECTION                                         */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  Return a file size in bytes.                                              */
/*----------------------------------------------------------------------------*/
static long log_size(char *in_file_name)
{
    int retvalue = 0;
    long size = 0;
    FILE *input_file;

    /* Open log file */
    input_file = fopen(in_file_name, "rb");
    if (NULL == input_file)
    {
        retvalue = -1;
        printf("Open file %s FAILED !!!\n", in_file_name);
    }

    if (0 == retvalue)
    {
        fseek(input_file, 0, SEEK_END); /* Go to the end */
        size = ftell(input_file); /* read size */

        /* Close log file */
        retvalue = fclose(input_file);
        if (0 != retvalue)
        {
            printf("Close file %s FAILED !!!\n", in_file_name);
        }
    }

    return size;
} /* end log_size function */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  Rename current log file in approppriate swap file .                       */
/*----------------------------------------------------------------------------*/
static int archiv_log(char *in_file_name)
{
    int retvalue = 0;
    char archiv[255];

    printf("Archive log file %s.\n", in_file_name);

    // Build log file name
    if (num_archive == 0)
    {
        /* First archive is kept */
        retvalue = snprintf(archiv, 255,
        PATTERN_ARCHIV_FILE,
        LOG_FILE_NAME, num_archive,
        LOG_FILE_EXTENSION);
        num_archive = 1;
    }
    else
    {
        retvalue = snprintf(archiv, 255,
        PATTERN_ARCHIV_FILE,
        LOG_FILE_NAME, num_archive,
        LOG_FILE_EXTENSION);
        /* Swap archives */
        if (num_archive < LOG_MAX_FILE)
        {
            num_archive++;
        }
        else
        {
            num_archive = 1;
        }
    }
    if (0 > retvalue)
    {
        retvalue = 1;
        printf("Build archive log file name FAILED !!!\n");
    }
    else
    {
        retvalue = 0;
    }

    if (0 == retvalue)
    {
        // Archive log file
        retvalue = rename(in_file_name, archiv);
        if (0 != retvalue)
        {
            retvalue = 1;
            printf("Rename file FAILED !!!\n");
        }
    }

    return retvalue;

} /* end archiv_log function */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*  Timestamp a log message.                                                  */
/*----------------------------------------------------------------------------*/
static void timeStamp_log(char *in_message, char *out_trace)
{
    int retvalue = 0;
    struct timeval time_value;
    struct tm *ptr_tm;

    /* Get time information */
    retvalue = gettimeofday(&time_value, NULL);
    if (0 == retvalue)
    {
        ptr_tm = localtime(&time_value.tv_sec);
        // Build retdata (Day/Month/Year Hour:Min:Sec:Ms-Data)
        retvalue = snprintf(out_trace,
        LOG__C_MSG_MAX,
        PATTERN_TIME_STAMP, ptr_tm->tm_mday, ptr_tm->tm_mon + C_MONTH_OFFSET, ptr_tm->tm_year + C_YEAR_OFFSET,
                ptr_tm->tm_hour, ptr_tm->tm_min, ptr_tm->tm_sec, (unsigned int) time_value.tv_usec / 1000, hls_id,
                in_message);

        // printf ("%s", out_trace);

        if (0 > retvalue)
        {
            printf("Add Time stamp FAILED !!!\n");
        }
    }
} /* end timeStamp_log function */

/*############################################################################*/
/*            GLOBAL FUNCTIONS                                                */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  Return the short label corresponding to the given level value             */
/*----------------------------------------------------------------------------*/
char const* get_label_LOG_LEVEL_ETYPE_short(LOG_LEVEL_ETYPE level)
{
    return label_LOG_LEVEL_ETYPE_short[level];
}
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  Log trace initialize and start.                                           */
/*----------------------------------------------------------------------------*/
void log__initialize(int in_dongle_value, LOG_LEVEL_ETYPE in_level, LOG__MODE_ETYPE in_mode)
{
    int retvalue = 0;

    if ((in_dongle_value == 3) || (in_dongle_value == 1))
    {
        hls_id = "PDRCU";
    }
    else
    {
        hls_id = "ANTEN";
    }

    log_mode = in_mode;

    if (log_mode == LOG__REMOTE)
    {
        openlog("HLS ", LOG_PID, LOG_USER);
    }
    else if (log_mode == LOG__LOCAL)
    {
        /* Build log file name */
        retvalue = snprintf(log_current_file, 255,
        PATTERN_LOG_FILE,
        LOG_FILE_NAME,
        LOG_FILE_EXTENSION);
        if (retvalue < 0)
        {
            printf("Build log file name FAILED !!!\n");
        }
    }

    log__set_level(in_level);
    if (retvalue == 0)
    {
        log__write(LOG__INFO, "T01:0283-INFO Component log initialization ok\n");
    }

} /* end log_start */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  Set log level.                                                            */
/*----------------------------------------------------------------------------*/
void log__set_level(LOG_LEVEL_ETYPE in_level)
{
    if (in_level <= LOG__NO_TRACE)
    {
        // Log the transition trace to no trace
        log_level = LOG__WARNING;
        LOG__WRITE_WARN("TRACE LEVEL = %s\n", label_LOG_LEVEL_ETYPE[LOG__NO_TRACE]);
        log_level = LOG__NO_TRACE;
    }
    else
    {
        char mode[16];

        if (in_level > LOG__DEBUG)
            log_level = LOG__DEBUG;
        else
            log_level = in_level;

        // TODO : env__var_read("LOG_MODE", /* default value */"LOCAL", mode);
        LOG__WRITE_WARN("TRACE MODE  = %s\n", mode);
        LOG__WRITE_WARN("TRACE LEVEL = %s\n", label_LOG_LEVEL_ETYPE[log_level]);
    }
} /* end set_level function */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  Writes log message in current log file.                                   */
/*----------------------------------------------------------------------------*/
void log__write(LOG_LEVEL_ETYPE in_msg_level, char *in_log_message)
{
    char trace_msg[LOG__C_MSG_MAX];

    if (log_level != LOG__NO_TRACE && in_msg_level <= log_level) // log enabled and msg level loggable
    {
        /* Add time-stamp */
        timeStamp_log(in_log_message, trace_msg);

        if (log_mode == LOG__REMOTE)
        {
            syslog(LOG_ERR, "%s", trace_msg);
        }
        else if (log_mode == LOG__LOCAL)
        {
            /* Write data in local file */
            FILE *input_file;
            int actual_size_written = 0;
            int retvalue = 0;

            /* If current log file is full, archive it */
            actual_size_written = log_size(log_current_file);

            if (actual_size_written > LOG__C_FILE_SIZE_MAX)
            {
                retvalue = archiv_log(log_current_file);
                if (retvalue != 0)
                {
                    printf("Archive log file %s FAILED !!!\n", log_current_file);
                    return;
                }
            }

            /* Open current log file */
            input_file = fopen(log_current_file, "a");
            if (NULL == input_file)
            {
                printf("Open log file %s FAILED !!!\n", log_current_file);
                return;
            }

            /* Write data in local file */
            retvalue = fputs(trace_msg, input_file);
            if (EOF == retvalue)
            {
                printf("Write in_log_message %s FAILED !!!\n", in_log_message);
            }

            retvalue = fclose(input_file);
            if (0 != retvalue)
            {
                printf("Close log file %s FAILED !!!\n", log_current_file);
            }
        }
        else
        {
            printf("%s", trace_msg);
        }
    }
}
/*----------------------------------------------------------------------------*/
