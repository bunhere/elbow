/*
 * Copyright (C) 2014 Ryuan Choi
 */

#ifndef log_h
#define log_h

#if !defined(NDEBUG)
#define BROWSER_LOGD(fmt, ...) \
    printf("[D]%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)
#define BROWSER_LOGE(fmt, ...) \
    printf("[E]%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)

#else
#define BROWSER_LOGD(fmt, ...) \
    do { (void) fmt; } while(0)

#define BROWSER_LOGE(fmt, ...) \
    do { (void) fmt; } while(0)
#endif

#if defined(DEVELOPER_MODE)
#define BROWSER_CALL_LOG(fmt, ...) \
    printf("%s:%d] " fmt "\n", __func__, __LINE__, ##__VA_ARGS__)

#else
#define BROWSER_CALL_LOG(fmt, ...) \
    do { (void) fmt; } while (0)

#endif

#endif //define log_h
