#include "../php_esl.h"

extern "C" {
    #include "php.h"
}

zend_object_handlers esl_event_handlers;

zend_object *esl_event_new(zend_class_entry *ce) {
    esl_event_object* obj;
    obj = (esl_event_object *)emalloc(sizeof *obj);
    zend_object_std_init((zend_object *) obj, ce);
    obj->esl_event = nullptr;
    obj->std.handlers = &esl_event_handlers;
    return &obj->std;
}

void esl_event_dtor(zend_object *object) {
    //reinterpret_cast<esl_event_object *>(object);
    esl_event_object *eslevent = (esl_event_object *)((char *)object - XtOffsetOf(esl_event_object, std));
    delete eslevent->esl_event;
    zend_object_std_dtor(&eslevent->std);
}

void esl_event_handlers_init() {
    memcpy(&esl_event_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    esl_event_handlers.offset = XtOffsetOf(esl_event_object, std);
    //esl_event_handlers.free_obj = esl_event_free;
    esl_event_handlers.clone_obj = nullptr;
    esl_event_handlers.compare = nullptr;
    esl_event_handlers.dtor_obj = esl_event_dtor;
}

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_event_getHeader, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, headerName, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_event_getBody, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_event_getType, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_construct, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, name, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, subclass, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_event_serialize, 0)
        ZEND_ARG_TYPE_INFO(0, type, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_addHeader, 0, 0, 1)
        ZEND_ARG_TYPE_INFO(0, header_name, IS_STRING, 0)
        ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_addBody, 0, 0, 1)
        ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_event_firstHeader, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_event_nextHeader, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_delHeader, 0, 0, 1)
                ZEND_ARG_TYPE_INFO(0, header_name, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_event_setPriority, 0, 0, 1)
                ZEND_ARG_TYPE_INFO(0, priority, IS_STRING, 0)
ZEND_END_ARG_INFO()
/* }}} */

PHP_METHOD(ESLevent, __construct)
{
    zend_string* name;
    zend_string* subclass=nullptr;

   ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,2)
        Z_PARAM_STR(name)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(subclass)
    ZEND_PARSE_PARAMETERS_END();
    //ZEND_PARSE_PARAMETERS_END_EX(RETURN_FALSE);

    if (ZSTR_LEN(name) == 0) {
        zend_throw_exception(zend_ce_exception, "Event name cannot be empty", 0);
        return;
    }

    std::string type_str(ZSTR_VAL(name), ZSTR_LEN(name));
    std::string subclass_str = subclass ? std::string(ZSTR_VAL(subclass), ZSTR_LEN(subclass)) : "";

    ESLevent* event = new ESLevent(type_str, subclass_str);
    esl_event_object *event_obj = nullptr;
    event_obj = (esl_event_object *)Z_OBJ_P(getThis());
    event_obj->esl_event = event;
}

PHP_METHOD(ESLevent, addHeader)
{
    zend_string *header_name;
    zend_string *z_value = nullptr;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,2)
            Z_PARAM_STR(header_name)
            Z_PARAM_OPTIONAL
            Z_PARAM_STR(z_value)
    ZEND_PARSE_PARAMETERS_END();

    std::string header = std::string(ZSTR_VAL(header_name), ZSTR_LEN(header_name));
    std::string value = std::string(ZSTR_VAL(z_value), ZSTR_LEN(z_value));
    auto *event_obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));
    bool status = event_obj->esl_event->addHeader(header, value);
    RETURN_BOOL(status);
}

PHP_METHOD(ESLevent, addBody)
{
    zend_string *z_value;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,1)
            Z_PARAM_STR(z_value)
    ZEND_PARSE_PARAMETERS_END();

    std::string value = std::string(ZSTR_VAL(z_value), ZSTR_LEN(z_value));
    auto *event_obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));
    event_obj->esl_event->addBody( value);
    RETURN_BOOL(true);
}

PHP_METHOD(ESLevent, getHeader)
{
	std::string event;
	esl_event_object *obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));
    
    char* headerName;
    size_t headerNameLen;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,1)
        Z_PARAM_STRING(headerName, headerNameLen)
    ZEND_PARSE_PARAMETERS_END();

    std::string header = obj->esl_event->getHeader(headerName);

    if(!header.empty()) {
        RETURN_STRINGL(header.c_str(), header.length());
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLevent, serialize)
{
	std::string event;
    std::string typeSer;
    zend_string* type=nullptr;

    ZEND_PARSE_PARAMETERS_START(0, 1)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(type)
    ZEND_PARSE_PARAMETERS_END();

    typeSer = type ? std::string(ZSTR_VAL(type), ZSTR_LEN(type)) : "plain";

	esl_event_object *obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));

    event = obj->esl_event->serialize(typeSer);

	RETURN_STRINGL(event.c_str(), event.length());
}

PHP_METHOD(ESLevent, getBody)
{
    std::string body;
    esl_event_object *obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));
    body = obj->esl_event->getBody();
    RETURN_STRINGL(body.c_str(), body.length());
}

PHP_METHOD(ESLevent, getType)
{
    std::string type;
    esl_event_object *obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));
    type = obj->esl_event->getType();
    RETURN_STRINGL(type.c_str(), type.length());
}

PHP_METHOD(ESLevent, firstHeader)
{
    esl_event_object *obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));
    std::string header = obj->esl_event->firstHeader();

    if(!header.empty()) {
        RETURN_STRINGL(header.c_str(), header.length());
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLevent, nextHeader)
{
    esl_event_object *obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));
    std::string header = obj->esl_event->nextHeader();

    if(!header.empty()) {
        RETURN_STRINGL(header.c_str(), header.length());
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLevent, delHeader)
{
    zend_string *header_name;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,1)
            Z_PARAM_STR(header_name)
    ZEND_PARSE_PARAMETERS_END();

    std::string header = std::string(ZSTR_VAL(header_name), ZSTR_LEN(header_name));
    auto *event_obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));
    bool status = event_obj->esl_event->delHeader(header);
    RETURN_BOOL(status);
}

PHP_METHOD(ESLevent, setPriority) {
    char *priority_str;
    size_t priority_str_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &priority_str, &priority_str_len) == FAILURE) {
        RETURN_NULL();
    }

    if (strcmp(priority_str, "LOW") == 0 || strcmp(priority_str, "NORMAL") == 0 || strcmp(priority_str, "HIGH") == 0) {
        auto *event_obj = reinterpret_cast<esl_event_object *>(Z_OBJ_P(getThis()));
        bool status = event_obj->esl_event->setPriority(priority_str);
        RETURN_BOOL(status);
    } else {
        // Invalid string
        RETURN_BOOL(false);
    }
}

zend_function_entry esl_event_functions[] = {
    PHP_ME(ESLevent, __construct, arginfo_event_construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(ESLevent, getHeader, arginfo_event_getHeader, ZEND_ACC_PUBLIC)
    PHP_ME(ESLevent, getBody, arginfo_event_getBody, ZEND_ACC_PUBLIC)
    PHP_ME(ESLevent, addBody, arginfo_event_addBody, ZEND_ACC_PUBLIC)
    PHP_ME(ESLevent, serialize, arginfo_event_serialize, ZEND_ACC_PUBLIC)
    PHP_ME(ESLevent, addHeader, arginfo_event_addHeader, ZEND_ACC_PUBLIC)
    PHP_ME(ESLevent, firstHeader, arginfo_event_firstHeader, ZEND_ACC_PUBLIC)
    PHP_ME(ESLevent, nextHeader, arginfo_event_nextHeader, ZEND_ACC_PUBLIC)
    PHP_ME(ESLevent, delHeader, arginfo_event_delHeader, ZEND_ACC_PUBLIC)
    PHP_ME(ESLevent, setPriority, arginfo_event_setPriority, ZEND_ACC_PUBLIC)
    PHP_FE_END
};