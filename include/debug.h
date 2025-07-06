#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>
#include <string.h>
#include <esp_heap_caps.h>  // 添加这个头文件

// 调试级别定义 - 添加保护避免重定义
#ifndef DEBUG_LEVEL_NONE
	#define DEBUG_LEVEL_NONE    0
#endif
#ifndef DEBUG_LEVEL_ERROR
	#define DEBUG_LEVEL_ERROR   1
#endif
#ifndef DEBUG_LEVEL_WARNING
	#define DEBUG_LEVEL_WARNING 2
#endif
#ifndef DEBUG_LEVEL_INFO
	#define DEBUG_LEVEL_INFO    3
#endif
#ifndef DEBUG_LEVEL_DEBUG
	#define DEBUG_LEVEL_DEBUG   4
#endif
#ifndef DEBUG_LEVEL_VERBOSE
	#define DEBUG_LEVEL_VERBOSE 5
#endif

// 设置调试级别
#ifndef DEBUG_LEVEL
	#ifdef DEBUG_LEVEL_VERBOSE
		#define DEBUG_LEVEL DEBUG_LEVEL_VERBOSE
	#else
		#define DEBUG_LEVEL DEBUG_LEVEL_NONE
	#endif
#endif

// 文件名提取宏
#ifndef CUSTOM_FILENAME
	#define CUSTOM_FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

// ANSI颜色码定义
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// 核心调试宏 - 带颜色支持
#if DEBUG_LEVEL >= DEBUG_LEVEL_ERROR
	#define ERROR_PRINT(x) do { Serial.printf(ANSI_COLOR_RED "[ERROR][%s:%d] %s" ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, String(x).c_str()); Serial.flush(); } while(0)
	#define ERROR_PRINTF(fmt, ...) do { Serial.printf(ANSI_COLOR_RED "[ERROR][%s:%d] " fmt ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, ##__VA_ARGS__); Serial.flush(); } while(0)
#else
	#define ERROR_PRINT(x) ((void)0)
	#define ERROR_PRINTF(fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_WARNING
	#define WARNING_PRINT(x) do { Serial.printf(ANSI_COLOR_YELLOW "[WARNING][%s:%d] %s" ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, String(x).c_str()); Serial.flush(); } while(0)
	#define WARNING_PRINTF(fmt, ...) do { Serial.printf(ANSI_COLOR_YELLOW "[WARNING][%s:%d] " fmt ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, ##__VA_ARGS__); Serial.flush(); } while(0)
#else
	#define WARNING_PRINT(x) ((void)0)
	#define WARNING_PRINTF(fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
	#define INFO_PRINT(x) do { Serial.printf(ANSI_COLOR_GREEN "[INFO][%s:%d] %s" ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, String(x).c_str()); Serial.flush(); } while(0)
	#define INFO_PRINTF(fmt, ...) do { Serial.printf(ANSI_COLOR_GREEN "[INFO][%s:%d] " fmt ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, ##__VA_ARGS__); Serial.flush(); } while(0)
	#define printlnA(fmt, ...) do { Serial.printf(ANSI_COLOR_WHITE "[MAIN] " fmt ANSI_COLOR_RESET "\n", ##__VA_ARGS__); Serial.flush(); } while(0)
#else
	#define INFO_PRINT(x) ((void)0)
	#define INFO_PRINTF(fmt, ...) ((void)0)
	#define printlnA(fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
	#define DEBUG_PRINT(x) do { Serial.printf(ANSI_COLOR_BLUE "[DEBUG][%s:%d] %s" ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, String(x).c_str()); Serial.flush(); } while(0)
	#define DEBUG_PRINTF(fmt, ...) do { Serial.printf(ANSI_COLOR_BLUE "[DEBUG][%s:%d] " fmt ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, ##__VA_ARGS__); Serial.flush(); } while(0)
#else
	#define DEBUG_PRINT(x) ((void)0)
	#define DEBUG_PRINTF(fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_VERBOSE
	#define VERBOSE_PRINT(x) do { Serial.printf(ANSI_COLOR_CYAN "[VERBOSE][%s:%d] %s" ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, String(x).c_str()); Serial.flush(); } while(0)
	#define VERBOSE_PRINTF(fmt, ...) do { Serial.printf(ANSI_COLOR_CYAN "[VERBOSE][%s:%d] " fmt ANSI_COLOR_RESET "\n", CUSTOM_FILENAME, __LINE__, ##__VA_ARGS__); Serial.flush(); } while(0)
#else
	#define VERBOSE_PRINT(x) ((void)0)
	#define VERBOSE_PRINTF(fmt, ...) ((void)0)
#endif

// 函数级别调试宏
#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
	#define FUNC_ENTER() DEBUG_PRINTF("-> %s() ENTER", __FUNCTION__)
	#define FUNC_EXIT() DEBUG_PRINTF("<- %s() EXIT", __FUNCTION__)
	#define FUNC_EXIT_VAL(val) DEBUG_PRINTF("<- %s() EXIT, return: %s", __FUNCTION__, String(val).c_str())
#else
	#define FUNC_ENTER() ((void)0)
	#define FUNC_EXIT() ((void)0)
	#define FUNC_EXIT_VAL(val) ((void)0)
#endif

// 性能测试宏 - 修复变量名问题
#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
	#ifdef PERF_START
		#undef PERF_START
	#endif
	#ifdef PERF_END
		#undef PERF_END
	#endif
	// 使用字符串作为标识，避免变量名冲突
	#define PERF_START(name) unsigned long perf_start_time = millis(); DEBUG_PRINTF(ANSI_COLOR_MAGENTA "PERF_START: %s" ANSI_COLOR_RESET, #name)
	#define PERF_END(name) DEBUG_PRINTF(ANSI_COLOR_MAGENTA "PERF_END: %s = %lu ms" ANSI_COLOR_RESET, #name, millis() - perf_start_time)
#else
	#ifndef PERF_START
		#define PERF_START(name) ((void)0)
	#endif
	#ifndef PERF_END
		#define PERF_END(name) ((void)0)
	#endif
#endif

// 栈检查宏
#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
#define STACK_CHECK(taskName) do { \
        size_t stack_left = uxTaskGetStackHighWaterMark(nullptr) * sizeof(StackType_t); \
        if (stack_left < 512) { \
            ERROR_PRINTF("STACK CRITICAL: Task %s only %d bytes left", taskName, stack_left); \
        } else if (stack_left < 1024) { \
            WARNING_PRINTF("STACK LOW: Task %s has %d bytes left", taskName, stack_left); \
        } else { \
            VERBOSE_PRINTF("STACK OK: Task %s has %d bytes left", taskName, stack_left); \
        } \
    } while(0)
#else
#define STACK_CHECK(taskName) ((void)0)
#endif

// 内存检查宏 - 修复PSRAM函数
#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
#define MEMORY_CHECK() do { \
        static unsigned long last_check = 0; \
        if (millis() - last_check > 30000) { \
            INFO_PRINTF("MEMORY: Free=%d bytes, Min=%d bytes", esp_get_free_heap_size(), esp_get_minimum_free_heap_size()); \
            last_check = millis(); \
        } \
    } while(0)

#define MEMORY_DETAIL() do { \
        size_t free_heap = esp_get_free_heap_size(); \
        size_t min_heap = esp_get_minimum_free_heap_size(); \
        size_t psram_free = heap_caps_get_free_size(MALLOC_CAP_SPIRAM); \
        size_t psram_total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM); \
        INFO_PRINTF("MEMORY DETAIL: Heap Free=%d, Min=%d, PSRAM Free=%d, Total=%d", \
                   free_heap, min_heap, psram_free, psram_total); \
    } while(0)
#else
#define MEMORY_CHECK() ((void)0)
#define MEMORY_DETAIL() ((void)0)
#endif

// 任务信息宏
#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
	#define TASK_INFO(taskName) VERBOSE_PRINTF("TASK: %s running on core %d", taskName, xPortGetCoreID())
	#define TASK_DELAY_INFO(taskName, delayMs) VERBOSE_PRINTF("TASK: %s will delay %d ms", taskName, delayMs)
#else
	#define TASK_INFO(taskName) ((void)0)
	#define TASK_DELAY_INFO(taskName, delayMs) ((void)0)
#endif

// 条件调试宏
#if DEBUG_LEVEL >= DEBUG_LEVEL_DEBUG
	#define DEBUG_IF(condition, fmt, ...) do { if(condition) DEBUG_PRINTF("IF_TRUE: " fmt, ##__VA_ARGS__); } while(0)
	#define ERROR_IF(condition, fmt, ...) do { if(condition) ERROR_PRINTF("IF_ERROR: " fmt, ##__VA_ARGS__); } while(0)
#else
	#define DEBUG_IF(condition, fmt, ...) ((void)0)
	#define ERROR_IF(condition, fmt, ...) ((void)0)
#endif

#if DEBUG_LEVEL >= DEBUG_LEVEL_INFO
	#define INFO_IF(condition, fmt, ...) do { if(condition) INFO_PRINTF("IF_INFO: " fmt, ##__VA_ARGS__); } while(0)
#else
	#define INFO_IF(condition, fmt, ...) ((void)0)
#endif

// 兼容原有的宏定义
#define LOG(x) INFO_PRINT(x)
#define LOGF(fmt, ...) INFO_PRINTF(fmt, ##__VA_ARGS__)

#endif // DEBUG_H