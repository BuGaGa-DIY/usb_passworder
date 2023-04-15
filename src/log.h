#ifndef _LOG_H_
#define _LOG_H_
#define LOG_TO_CHANNEL( ... ) printf( __VA_ARGS__ )

#ifdef DEBUG
    #define LOGS_DEBUG( ... ) LOG_TO_CHANNEL( "DEBUG: " __VA_ARGS__ )
#else
    #define LOGS_DEBUG( ... ) 
#endif
#define LOGS_INFO( ... ) LOG_TO_CHANNEL( "INFO: " __VA_ARGS__ )
#define LOGS_ERROR( ... ) LOG_TO_CHANNEL( "ERROR: " __VA_ARGS__ )

#endif