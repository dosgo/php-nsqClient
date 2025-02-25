#include "php.h"
#include "php_libNsq.h"
#include "libnsq_subscriber.h"
ZEND_DECLARE_MODULE_GLOBALS(libNsq)

PHP_FUNCTION(startNsqSubscriber) {
    char *topic, *channel, *lookupd;
    size_t topic_len, channel_len, lookupd_len;
    zend_long max_attempts, auth_response;

    // 解析 PHP 参数
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sssll",
        &topic, &topic_len,
        &channel, &channel_len,
        &lookupd, &lookupd_len,
        &max_attempts,
        &auth_response) == FAILURE) {
        RETURN_NULL();
    }

    // 调用 Go 函数
    int ret = StartNSQSubscriber(
        topic,
        channel,
        lookupd,
        (uint16_t)max_attempts,
        (uint16_t)auth_response
    );

    RETURN_LONG(ret);
}

PHP_FUNCTION(getMessage) {
    char *topic, *channel;
    size_t topic_len, channel_len;
    zend_long timeout_ms;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssl", 
        &topic, &topic_len,
        &channel, &channel_len,
        &timeout_ms) == FAILURE) {
        RETURN_NULL();
    }

    char *result = GetMessage(topic, channel, (uint32_t)timeout_ms);
    
    if(result) {
        RETURN_STRING(result);
        FreeMessage(result); // PHP 字符串复制后立即释放
    } else {
        RETURN_NULL();
    }
}

PHP_FUNCTION(confirmMessage) {
    char *topic, *channel, *id;
    size_t topic_len, channel_len, id_len;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "sss", 
        &topic, &topic_len,
        &channel, &channel_len,
        &id, &id_len) == FAILURE) {
        RETURN_NULL();
    }
    // 需要复制ID字符串（因PHP可能重用内存）
    char *id_copy = estrndup(id, id_len);
    ConfirmMessage(topic, channel, id_copy);
    RETURN_TRUE;
}

PHP_FUNCTION(stopNsqSubscriber) {
    char *topic, *channel;
    int argc = ZEND_NUM_ARGS();
    size_t topic_len, channel_len, id_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss", 
        &topic, &topic_len,
        &channel, &channel_len) == FAILURE) {
        RETURN_NULL();
    }
    StopNSQSubscriber((char *)topic, (char *)channel);
    RETURN_TRUE;
}


// 模块入口
zend_module_entry libNsq_module_entry = {
    STANDARD_MODULE_HEADER,
    "libNsq",                  // 扩展名称
    NULL,                       // 函数列表（在ZEND_BEGIN_MODULE_GLOBALS中定义）
    NULL,                       // 模块初始化
    NULL,                       // 模块关闭
    NULL,                       // 请求初始化
    NULL,                       // 请求关闭
    NULL,                       // 模块信息
    "1.0",                      // 版本号
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_libNsq
ZEND_GET_MODULE(libNsq)
#endif