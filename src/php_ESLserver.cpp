#include "../php_esl.h"

extern "C" {
    #include "php.h"
}

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_server_construct, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, port, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_server_accept, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_server_close, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, clientSock, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_server_destruct, 0)
ZEND_END_ARG_INFO()
/* }}} */

PHP_METHOD(ESLserver, __construct) {
    char *host;
    char *port;
    size_t host_len;
    size_t port_len;
    ESLserver *server = nullptr;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,2,2)
        Z_PARAM_STRING(host, host_len)
        Z_PARAM_STRING(port, port_len)
    ZEND_PARSE_PARAMETERS_END();

    try {
        server = new ESLserver(host, port);
        auto *obj = reinterpret_cast<esl_server_object *>(Z_OBJ_P(getThis()));
        obj->esl_server = server;
    } catch (const std::exception &e) {
        zend_throw_exception(zend_ce_exception, e.what(), 0);
        return;
    }
}

PHP_METHOD(ESLserver, __destruct) {
	auto *obj = reinterpret_cast<esl_server_object *>(Z_OBJ_P(getThis()));
    if(obj->esl_server) {
        delete obj->esl_server;
        obj->esl_server = nullptr;
    }
    zend_object_std_dtor(&obj->std);
}

PHP_METHOD(ESLserver, accept) {
    auto *obj = reinterpret_cast<esl_server_object *>(Z_OBJ_P(getThis()));
    if(obj->esl_server) {
        int sockfd = obj->esl_server->accept();
        RETURN_LONG(sockfd);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLserver, close) {
    zend_long clientSock = 0;
    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,1)
            Z_PARAM_LONG(clientSock)
    ZEND_PARSE_PARAMETERS_END();

    auto *obj = reinterpret_cast<esl_server_object *>(Z_OBJ_P(getThis()));
    if(obj->esl_server) {
        obj->esl_server->close((int)clientSock);
    }
}

zend_function_entry esl_server_functions[] = {
	PHP_ME(ESLserver, __construct, arginfo_server_construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(ESLserver, __destruct, arginfo_server_destruct, ZEND_ACC_PUBLIC)
    PHP_ME(ESLserver, accept, arginfo_server_accept, ZEND_ACC_PUBLIC)
    PHP_ME(ESLserver, close, arginfo_server_close, ZEND_ACC_PUBLIC)
    PHP_FE_END
};