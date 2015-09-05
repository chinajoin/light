#ifndef __LOG_H_
#define __LOG_H_

enum _log_level {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR
};

int configure_log(int level, const char* file, int use_console);

void logger(int lvl, const char *file, const int line, const char *fmt, ...);

#define debug(fmt, ...) logger(DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define info(fmt, ...)  logger(INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define warn(fmt, ...)  logger(WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define error(fmt, ...) logger(ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif /* __LOG_H_ */
