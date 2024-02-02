/* FreeSWITCH ESL extension for PHP
 * Author: Milan M. <amsoftswitch@gmail.com> */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
extern "C" {
	#include "php.h"
}
#include "ext/standard/info.h"
#include "php_esl.h"

#include "Zend/zend_API.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(esl)
{
#if defined(ZTS) && defined(COMPILE_DL_ESL)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(esl)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "esl support", "enabled");
	php_info_print_table_end();
}
/* }}} */

zend_class_entry *esl_event_ce = nullptr;
zend_class_entry *esl_conn_ce = nullptr;
zend_class_entry *esl_server_ce = nullptr;
/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(esl)
{
	esl_event_handlers_init();

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "ESLevent", esl_event_functions);
	esl_event_ce = zend_register_internal_class(&ce);
	esl_event_ce->create_object = esl_event_new;

	INIT_CLASS_ENTRY(ce, "ESLconnection", esl_conn_functions);
	esl_conn_ce = zend_register_internal_class(&ce);
	esl_conn_ce->create_object = esl_conn_new;
	zend_declare_property_bool(esl_conn_ce, "connected", strlen("connected"), 0, ZEND_ACC_PROTECTED);

	INIT_CLASS_ENTRY(ce, "ESLserver", esl_server_functions);
	esl_server_ce = zend_register_internal_class(&ce);

	return SUCCESS;
}
/* }}} */

/* {{{ esl_module_entry
 */
zend_module_entry esl_module_entry = {
	STANDARD_MODULE_HEADER,
	"esl",					            /* Extension name */
	NULL,					            /* zend_function_entry */
	PHP_MINIT(esl),			/* PHP_MINIT - Module initialization */
	NULL,					/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(esl),			/* PHP_RINIT - Request initialization */
	NULL,					/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(esl),			        /* PHP_MINFO - Module info */
	PHP_ESL_VERSION,		            /* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ESL
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(esl)
#endif