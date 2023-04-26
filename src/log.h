#ifndef _LOG_H_
#define _LOG_H_
#define LOG_TO_CHANNEL( format, ... ) printf( "[%d] " format "\n\r", board_millis(), ##__VA_ARGS__ )

#ifdef DEBUG
    #define LOGS_DEBUG( format, ... ) LOG_TO_CHANNEL( "DEBUG:\t" format, ##__VA_ARGS__ )
#else
    #define LOGS_DEBUG( format, ... ) 
#endif
#define LOGS_INFO( format, ... ) LOG_TO_CHANNEL( "INFO:\t" format, ##__VA_ARGS__ )
#define LOGS_ERROR( format, ... ) LOG_TO_CHANNEL( "ERROR:\t" format, ##__VA_ARGS__ )

#define DEBUG
#endif