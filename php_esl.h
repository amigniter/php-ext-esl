/* esl extension for PHP */

#ifndef PHP_ESL_H
# define PHP_ESL_H

#include "Zend/zend_API.h"
#include "zend_exceptions.h"

#include "src/ESLevent.h"
#include "src/ESLconnection.h"
#include "src/ESLserver.h"

typedef struct _esl_event_object {
    zend_object std;
    ESLevent *esl_event;
} esl_event_object;

typedef struct _esl_conn_object {
    zend_object std;
    ESLconnection *esl_conn;
} esl_conn_object;

typedef struct _esl_server_object {
    zend_object std;
    ESLserver *esl_server;
} esl_server_object;

extern zend_object *esl_event_new(zend_class_entry *ce);
extern zend_object *esl_conn_new(zend_class_entry *ce);
extern void esl_event_handlers_init();
extern zend_function_entry esl_event_functions[];
extern zend_class_entry *esl_event_ce;
extern zend_class_entry *esl_conn_ce;
extern zend_class_entry *esl_server_ce;
extern zend_function_entry esl_conn_functions[];
extern zend_function_entry esl_server_functions[];

extern zend_module_entry esl_module_entry;
# define phpext_esl_ptr &esl_module_entry

# define PHP_ESL_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_ESL)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_ESL_H */
