//##############################################################################
//                         THALES AIR SYSTEMS - SURFACE RADAR
//                         HLS Software
//                         CSC : HLS
//                         CSU : log
//==============================================================================
/**
 Services for log trace.

 @File   : log.h
 @Author : SR-RTPE-L 2010
 @Description :
 Exports services for log trace.

 Trace logging consists in recording log message (with log__write function)
 A log message is a text string of LOG_MSG_MAX_SIZE characters max long.
 Messages are tagged with a level, so that they are displayed only if
 their level are lower than log level.

 Log messages are either written in log files named hls_0.log, hls_1.log
 and hls_2.log or sent through syslog service.
 Log file size can't exceed LOG__C_FILE_SIZE_MAX characters
 As soon log component is initialized hls_0.log file is filled first.
 After, hls_1 and hls_2 log files are filled alternatively.

 Trace level must be set at start and can be changed after with
 log__set_level function.

 This component must be initialized and start as soon as possible in
 sofware initialization section.


 @Evolutions :

 Date       : PCR-ECR            : Description

 @end_desc

 */
/*----------------------------------------------------------------------------*/

#ifndef _LOG_H
#define _LOG_H

/*############################################################################*/
/*            INCLUDE SECTION                                                 */
/*----------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

//#include "global_constants_and_types.h"

/*############################################################################*/
/*            DEFINE SECTION                                                  */
/*----------------------------------------------------------------------------*/
#define LOG__C_MSG_MAX       200             /* Max log message size in bytes */
#define LOG__C_FILE_SIZE_MAX 5000*LOG__C_MSG_MAX /* Max log file size in bytes */

/*############################################################################*/
/*            MACRO SECTION                                                   */
/*----------------------------------------------------------------------------*/
// macro VARNAME_msg_T is defined to get a "generated" variable name with a very low risk of name collision
#define CONCAT_(a, b)        a ## b
#define CONCAT2(a, b)        CONCAT_(a, b)
#define CONCAT4(a, b, c, d)  CONCAT2(CONCAT2(a, b), CONCAT2(c, d))
#define VARNAME_msg_T  CONCAT4(msg_T, __FILE_ID_FOR_LOG__, _, __LINE__)

// function-like macro LOG__WRITE to call the function log__write with a header added to the given message & format
#define LOG__WRITE(lvl, fmt, arg...) do {\
   char VARNAME_msg_T[LOG__C_MSG_MAX];/* the message to log with a header (file/line/level) */\
   snprintf(VARNAME_msg_T, LOG__C_MSG_MAX, "T%02d:%04d-%s "fmt,\
           __FILE_ID_FOR_LOG__, __LINE__,\
           get_label_LOG_LEVEL_ETYPE_short(lvl), ##arg);\
   log__write(lvl, VARNAME_msg_T);\
} while (0)

#define LOG__WRITE_CRIT(fmt, arg...)   LOG__WRITE(LOG__CRIT, fmt, ##arg)
#define LOG__WRITE_ERROR(fmt, arg...)  LOG__WRITE(LOG__ERROR, fmt, ##arg)
#define LOG__WRITE_WARN(fmt, arg...)   LOG__WRITE(LOG__WARNING, fmt, ##arg)
#define LOG__WRITE_INFO(fmt, arg...)   LOG__WRITE(LOG__INFO, fmt, ##arg)
#define LOG__WRITE_DEBUG(fmt, arg...)  LOG__WRITE(LOG__DEBUG, fmt, ##arg)

/*############################################################################*/
/*            TYPEDEF SECTION                                                 */
/*----------------------------------------------------------------------------*/

/* log output modes */
typedef enum
{
    LOG__REMOTE = 0, // by syslog service
    LOG__LOCAL = 1, // writing local file
    LOG__CONSOLE = 2  // terminal stdout
} LOG__MODE_ETYPE;

typedef enum /* Possible log trace levels based on syslog definition standard */
{
    LOG__NO_TRACE = 0, /** No trace */
    LOG__ALERT = 1, /** NOT USED */
    LOG__CRIT = 2, /** Error forcing a shutdown of the application */
    LOG__ERROR = 3, /** Fatal error for an operation but not the application */
    LOG__WARNING = 4, /** Uncommon behavior, application recover from it */
    LOG__NOTICE = 5, /** NOT USED */
    LOG__INFO = 6, /** Useful information to log in normal circumstances */
    LOG__DEBUG = 7 /** Information helpfull for development team */
} LOG_LEVEL_ETYPE;

static char const* const label_LOG_LEVEL_ETYPE[] =
{ "LOG__NO_TRACE", "LOG__ALERT", "LOG__CRIT", "LOG__ERROR", "LOG__WARNING", "LOG__NOTICE", "LOG__INFO", "LOG__DEBUG" };

static char const* const label_LOG_LEVEL_ETYPE_short[] =
{ "OFF", "ALERT", "CRIT", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG" };

/*############################################################################*/
/*            EXPORT GLOBAL FUNCTIONS                                         */
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  Return the short label corresponding to the given level value             */
extern char const* get_label_LOG_LEVEL_ETYPE_short(LOG_LEVEL_ETYPE level);
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  Log trace initialize and start.                                           */
extern void log__initialize(int in_dongle_value, LOG_LEVEL_ETYPE in_level, LOG__MODE_ETYPE in_mode);
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  Set log level.                                                            */
extern void log__set_level(LOG_LEVEL_ETYPE in_level);
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  Writes log message in current log file.                                   */
extern void log__write(LOG_LEVEL_ETYPE in_msg_level, char *in_log_message);
/*----------------------------------------------------------------------------*/

/*============================================================================*/
/*  Log trace stop.                                                           */
extern void log__stop();
/*----------------------------------------------------------------------------*/

#endif /* log.h */
