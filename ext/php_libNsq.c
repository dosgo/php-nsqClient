/* libnsq extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "zend_exceptions.h"
#include "ext/standard/info.h"
#include "php_libnsq.h"


/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

PHP_METHOD(NsqClient,startNsqSubscriber) {
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

PHP_METHOD(NsqClient,getMessage) {
    char *topic, *channel;
    size_t topic_len, channel_len;
    zend_long timeout_ms;
	zend_long buf_len=5000;
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "ssl|l", &topic, &topic_len,&channel, &channel_len,&timeout_ms,&buf_len) == FAILURE) {
        RETURN_NULL();
    }

    char* buf = (char*)malloc(buf_len+1);
    if (buf == NULL) {
		zend_throw_exception(zend_ce_exception, "memory allocation failed", 0);
        RETURN_NULL();
    }
	memset(buf, 0,buf_len+1); 
	int resLen=GetMessageBuf(topic,channel, (uint32_t)timeout_ms, buf , buf_len);
    if(resLen > 0) {
        RETURN_STRING(buf);
		free(buf);
    } else {
		free(buf);
		if(resLen==-2){
			//php_error_docref0(NULL TSRMLS_CC, E_ERROR, "Fatal error: buf_len is not long enough");
			zend_throw_exception(zend_ce_exception, "buf_len is not long enough", 0);
		}
        RETURN_NULL();
    }
}

PHP_METHOD(NsqClient,confirmMessage) {
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

PHP_METHOD(NsqClient,stopNsqSubscriber) {
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





/* }}}*/

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(libnsq)
{
#if defined(ZTS) && defined(COMPILE_DL_LIBNSQ)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(libnsq)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "libnsq support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */



ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_startNsqSubscriber, 0, 5, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, topic, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, channel, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, lookupd, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, max_attempts, IS_LONG, 0)
    ZEND_ARG_TYPE_INFO(0, auth_response, IS_LONG, 0)
ZEND_END_ARG_INFO();

// 为 getMessage 函数定义参数信息
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_getMessage, 0, 3, IS_STRING, 1)
    ZEND_ARG_TYPE_INFO(0, topic, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, channel, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, timeout_ms, IS_LONG, 0)
ZEND_END_ARG_INFO();

// 为 confirmMessage 定义参数信息
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_confirmMessage, 0, 3, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, topic, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, channel, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, id, IS_STRING, 0)
ZEND_END_ARG_INFO();

// 为 stopNsqSubscriber 定义参数信息
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(arginfo_stopNsqSubscriber, 0, 2, _IS_BOOL, 0)
    ZEND_ARG_TYPE_INFO(0, topic, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, channel, IS_STRING, 0)
ZEND_END_ARG_INFO();

/* }}} */

/* {{{ libnsq_functions[]
 */




zend_class_entry *nsqClient_ce; // 类入口指针

static const zend_function_entry nsqClient_methods[] = {
   // PHP_ME(NsqClient, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(NsqClient, startNsqSubscriber, arginfo_startNsqSubscriber, ZEND_ACC_PUBLIC)
	PHP_ME(NsqClient, getMessage, arginfo_getMessage, ZEND_ACC_PUBLIC)
	PHP_ME(NsqClient, confirmMessage, arginfo_confirmMessage, ZEND_ACC_PUBLIC)
	PHP_ME(NsqClient, stopNsqSubscriber, arginfo_stopNsqSubscriber, ZEND_ACC_PUBLIC)
    PHP_FE_END
};


PHP_MINIT_FUNCTION(libnsq) {
    zend_class_entry ce;
    // 定义类名和构造函数（可选）
    INIT_CLASS_ENTRY(ce, "NsqClient", nsqClient_methods);
    nsqClient_ce = zend_register_internal_class(&ce);
    return SUCCESS;
}



/* }}} */

/* {{{ libnsq_module_entry
 */
zend_module_entry libnsq_module_entry = {
	STANDARD_MODULE_HEADER,
	"libnsq",					/* Extension name */
	NULL,			/* zend_function_entry */
	PHP_MINIT(libnsq),							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(libnsq),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(libnsq),			/* PHP_MINFO - Module info */
	PHP_LIBNSQ_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};


/* }}} */

#ifdef COMPILE_DL_LIBNSQ
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(libnsq)
#endif
