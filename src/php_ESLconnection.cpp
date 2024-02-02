#include "../php_esl.h"

extern "C" {
    #include "php.h"
}

zend_object *esl_conn_new(zend_class_entry *ce) {
    esl_conn_object *obj;
    obj = (esl_conn_object *)ecalloc(1, sizeof(esl_conn_object) + zend_object_properties_size(ce));
    zend_object_std_init((zend_object *)obj, ce);
    object_properties_init((zend_object *)obj, ce);
    obj->esl_conn = nullptr;
    obj->std.handlers = zend_get_std_object_handlers();  // Use default handlers
    return &obj->std;
}

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_construct, 0, 0, 1)
    ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_conn_destruct, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_sendrecv, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_send, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, command, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_events, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, event_type, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_filter, 0, 0, 2)
    ZEND_ARG_TYPE_INFO(0, header, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_conn_recvtimed, 0)
    ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_api, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, cmd, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, arg, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_execute, 0, 0, 3)
    ZEND_ARG_TYPE_INFO(0, app, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, arg, IS_STRING, 0)
    ZEND_ARG_TYPE_INFO(0, uuid, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_executeAsync, 0, 0, 3)
                ZEND_ARG_TYPE_INFO(0, app, IS_STRING, 0)
                ZEND_ARG_TYPE_INFO(0, arg, IS_STRING, 0)
                ZEND_ARG_TYPE_INFO(0, uuid, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_eventlock, 0, 0, 1)
                ZEND_ARG_TYPE_INFO(0, value, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_asyncexecute, 0, 0, 1)
                ZEND_ARG_TYPE_INFO(0, value, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_conn_info, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_conn_recv, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_conn_connected, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_conn_disconnect, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_conn_socketdescriptor, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_conn_sendevent, 0, 0, 1)
                ZEND_ARG_OBJ_INFO(0, event, ESLevent, 0)
ZEND_END_ARG_INFO()
/* }}} */

PHP_METHOD(ESLconnection, __construct)
{
    ESLconnection *conn = nullptr;
    zval *zhost = nullptr;
    zval *zport = nullptr;
    zval *zpass = nullptr;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|zz", &zhost, &zport, &zpass) == FAILURE) {
        // Error in parsing arguments
        zend_throw_exception(nullptr, "Failed to parse constructor arguments.", 0);
        return;
    }

    if (ZEND_NUM_ARGS() == 1 && Z_TYPE_P(zhost) == IS_LONG) {
        // Handle the case of a single integer argument
        long sockfd = Z_LVAL_P(zhost);

        conn = new ESLconnection(sockfd);
        try {
            conn->connect_outbound();
        } catch (const std::exception &e) {
            zend_throw_exception(nullptr, e.what(), 0);
            return;
        }
    } else if (ZEND_NUM_ARGS() == 3 && Z_TYPE_P(zhost) == IS_STRING && Z_TYPE_P(zport) == IS_STRING && Z_TYPE_P(zpass) == IS_STRING) {
        // Handle the case of three string arguments
        char* host = Z_STRVAL_P(zhost);
        char* port = Z_STRVAL_P(zport);
        char* pass = Z_STRVAL_P(zpass);

        conn = new ESLconnection(host, port, pass);
        try {
            conn->connect_inbound();
        } catch (const std::exception &e) {
            zend_throw_exception(nullptr, e.what(), 0);
            return;
        }

    } else {
        // Invalid argument combination or types
        zend_throw_exception(nullptr, "Invalid argument combination or types.", 0);
        return;
    }
    esl_conn_object *obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    obj->esl_conn = conn;
    zend_update_property_bool(esl_conn_ce, Z_OBJ_P(getThis()), "connected", sizeof("connected") - 1, 1);
}

PHP_METHOD(ESLconnection, __destruct)
{
	esl_conn_object *obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    if(obj->esl_conn) {
        delete obj->esl_conn;
        obj->esl_conn = nullptr;
    }
    zend_object_std_dtor(&obj->std);
}

PHP_METHOD(ESLconnection, connected)
{
	esl_conn_object *obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    bool status = obj->esl_conn->getStatus();
    zend_update_property_bool(esl_conn_ce, Z_OBJ_P(getThis()), "connected", sizeof("connected") - 1, status);
    RETURN_BOOL(status);
}

PHP_METHOD(ESLconnection, socketDescriptor)
{
    esl_conn_object *obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    int sockfd = obj->esl_conn->socketDescriptor();
    RETURN_LONG(sockfd);
}

PHP_METHOD(ESLconnection, disconnect) {
    esl_conn_object *obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    obj->esl_conn->do_disconnect();
    zend_update_property_bool(esl_conn_ce, Z_OBJ_P(getThis()), "connected", sizeof("connected") - 1, false);
}

PHP_METHOD(ESLconnection, sendRecv)
{
    zend_string* name;
    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,1)
        Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    std::string command(ZSTR_VAL(name), ZSTR_LEN(name));

    ESLevent *event= nullptr;
    esl_event_object *ev_obj = nullptr;
	esl_conn_object *conn_obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    zval obj;

    event = conn_obj->esl_conn->sendRecv(command);
    if(event) {
        object_init_ex(&obj, esl_event_ce);
        ev_obj = (esl_event_object*)Z_OBJ_P(&obj);
        ev_obj->esl_event = event;
        RETURN_OBJ(&ev_obj->std);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLconnection, send)
{
    zend_string* name;
    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,1)
            Z_PARAM_STR(name)
    ZEND_PARSE_PARAMETERS_END();

    std::string command(ZSTR_VAL(name), ZSTR_LEN(name));

    auto *conn_obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    bool status;

    status = conn_obj->esl_conn->send(command);
    RETURN_BOOL(status);
}

PHP_METHOD(ESLconnection, events)
{
    zend_string* ev_type;
    zend_string* val;
    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,2,2)
            Z_PARAM_STR(ev_type)
            Z_PARAM_STR(val)
    ZEND_PARSE_PARAMETERS_END();


    std::string event_type(ZSTR_VAL(ev_type), ZSTR_LEN(ev_type));
    std::string value(ZSTR_VAL(val), ZSTR_LEN(val));

    std::string command;

    if(event_type == "xml") {
        command = "event " + event_type + " " + value;
    } else {
        command = "event plain " + value;
    }

    ESLevent *event= nullptr;
    esl_event_object *ev_obj = nullptr;
    esl_conn_object *conn_obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    zval obj;

    event = conn_obj->esl_conn->sendRecv(command);
    if(event) {
        object_init_ex(&obj, esl_event_ce);
        ev_obj = (esl_event_object*)Z_OBJ_P(&obj);
        ev_obj->esl_event = event;
        RETURN_OBJ(&ev_obj->std);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLconnection, filter)
{
    zend_string* z_header;
    zend_string* z_value;
    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,2,2)
            Z_PARAM_STR(z_header)
            Z_PARAM_STR(z_value)
    ZEND_PARSE_PARAMETERS_END();

    std::string header(ZSTR_VAL(z_header), ZSTR_LEN(z_header));
    std::string value(ZSTR_VAL(z_value), ZSTR_LEN(z_value));

    std::string command;

    command = "filter " + header + " " + value;

    ESLevent *event= nullptr;
    esl_event_object *ev_obj = nullptr;
    esl_conn_object *conn_obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    zval obj;

    event = conn_obj->esl_conn->sendRecv(command);
    if(event) {
        object_init_ex(&obj, esl_event_ce);
        ev_obj = (esl_event_object*)Z_OBJ_P(&obj);
        ev_obj->esl_event = event;
        RETURN_OBJ(&ev_obj->std);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLconnection, recvEvent)
{
    ESLevent *event= nullptr;
    esl_event_object *ev_obj = nullptr;
	esl_conn_object *conn_obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    zval obj;

    event = conn_obj->esl_conn->recvEvent(0);
    if(event) {
        object_init_ex(&obj, esl_event_ce);
        ev_obj = (esl_event_object*)Z_OBJ_P(&obj);
        ev_obj->esl_event = event;
        RETURN_OBJ(&ev_obj->std);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLconnection, recvEventTimed)
{
    zend_long timeout = 0;
    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,0,1)
        Z_PARAM_OPTIONAL
        Z_PARAM_LONG(timeout)
    ZEND_PARSE_PARAMETERS_END();

    ESLevent *event= nullptr;
    esl_event_object *ev_obj = nullptr;
	esl_conn_object *conn_obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    zval obj;

    event = conn_obj->esl_conn->recvEvent((int)timeout);
    if(event) {
        object_init_ex(&obj, esl_event_ce);
        ev_obj = (esl_event_object*)Z_OBJ_P(&obj);
        ev_obj->esl_event = event;
        RETURN_OBJ(&ev_obj->std);
    } else {
        RETURN_NULL();
    }
}

zend_object* apiHandler(zend_string* cmd, zend_string *arg, std::string& type, zval *method) {
    std::string command;
    std::string argument;
    
    command = type + " " + std::string(ZSTR_VAL(cmd), ZSTR_LEN(cmd));
    argument = arg ? std::string(ZSTR_VAL(arg), ZSTR_LEN(arg)) : "";
    if(!argument.empty())
        command += " " + argument;
    
    ESLevent *event= nullptr;
    esl_event_object *ev_obj = nullptr;
    esl_conn_object *conn_obj = reinterpret_cast<esl_conn_object*>(Z_OBJ_P(method));
    zval obj;

    event = conn_obj->esl_conn->sendRecv(command);
    if(event) {
        object_init_ex(&obj, esl_event_ce);
        ev_obj = (esl_event_object*)Z_OBJ_P(&obj);
        ev_obj->esl_event = event;
        return &ev_obj->std;
    } else {
        return nullptr;
    }
}

PHP_METHOD(ESLconnection, api) {
    zend_string *cmd;
    zend_string *arg = nullptr;
    zend_object *event = nullptr;
    zval *method = getThis();
    std::string type ="api";

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,2)
        Z_PARAM_STR(cmd)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(arg)
    ZEND_PARSE_PARAMETERS_END();

    event = apiHandler(cmd, arg, type, method);
    if(event) {
        RETURN_OBJ(event);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLconnection, bgapi) {
    zend_string *cmd;
    zend_string *arg = nullptr;
    zend_object *event = nullptr;
    zval *method = getThis();
    std::string type ="bgapi";

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,2)
        Z_PARAM_STR(cmd)
        Z_PARAM_OPTIONAL
        Z_PARAM_STR(arg)
    ZEND_PARSE_PARAMETERS_END();

    event = apiHandler(cmd, arg, type, method);
    if(event) {
        RETURN_OBJ(event);
    } else {
        RETURN_NULL();
    }
}

zend_object *execHandler(std::string (&args)[3], std::string &type, zval *method) {
    std::string app_buf, arg_buf, cmd_buf;
    cmd_buf = "sendmsg";

    cmd_buf.append(" " + std::string(args[2]));
    app_buf.append("execute-app-name: " + std::string(args[0]) + "\n");
    arg_buf.append("execute-app-arg: " + std::string(args[1]) + "\n");
    cmd_buf.append("\ncall-command: execute\n");

    ESLevent *event= nullptr;
    esl_event_object *ev_obj = nullptr;

    esl_conn_object *conn_obj = reinterpret_cast<esl_conn_object*>(Z_OBJ_P(method));
    zval obj;

    bool eventLock= conn_obj->esl_conn->getEventLock();
    bool asyncMode;

    if(type == "sync") {
        asyncMode = conn_obj->esl_conn->getAsyncMode();
        cmd_buf.append(app_buf + arg_buf + (eventLock ? "event-lock: true\n" : "") + (asyncMode ? "async: true\n" : ""));
    } else {
        cmd_buf.append(app_buf + arg_buf + (eventLock ? "event-lock: true\n" : ""));
    }

    event = conn_obj->esl_conn->sendRecv(cmd_buf);

    if(event) {
        object_init_ex(&obj, esl_event_ce);
        ev_obj = (esl_event_object*)Z_OBJ_P(&obj);
        ev_obj->esl_event = event;
        return &ev_obj->std;
    } else {
        return nullptr;
    }
}

PHP_METHOD(ESLconnection, execute) {
    char *app = nullptr;
    char *arg = nullptr;
    char *uuid = nullptr;
    size_t app_len=0;
    size_t arg_len=0;
    size_t uuid_len=0;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,3,3)
        //Z_PARAM_OPTIONAL
        Z_PARAM_STRING(app, app_len)
        Z_PARAM_STRING(arg, arg_len)
        Z_PARAM_STRING(uuid, uuid_len)
    ZEND_PARSE_PARAMETERS_END();

    std::string args[3] = {app, arg, uuid};
    std::string type = "sync";
    zval *method = getThis();
    zend_object *event = nullptr;

    event = execHandler(args, type, method);
    if(event) {
        RETURN_OBJ(event);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLconnection, executeAsync) {
    char *app = nullptr;
    char *arg = nullptr;
    char *uuid = nullptr;
    size_t app_len=0;
    size_t arg_len=0;
    size_t uuid_len=0;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,3,3)
            Z_PARAM_STRING(app, app_len)
            Z_PARAM_STRING(arg, arg_len)
            Z_PARAM_STRING(uuid, uuid_len)
    ZEND_PARSE_PARAMETERS_END();

    std::string args[3] = {app, arg, uuid};
    std::string type = "async";
    zval *method = getThis();
    zend_object *event = nullptr;

    event = execHandler(args, type, method);
    if(event) {
        RETURN_OBJ(event);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLconnection, getInfo) {
    ESLevent *eventInfo= nullptr;
    esl_event_object *ev_obj = nullptr;
	
    esl_conn_object *conn_obj = reinterpret_cast<esl_conn_object*>(Z_OBJ_P(getThis()));
    zval obj;

    eventInfo = conn_obj->esl_conn->getInfo();
    if(eventInfo) {
        object_init_ex(&obj, esl_event_ce);
        ev_obj = (esl_event_object*)Z_OBJ_P(&obj);
        ev_obj->esl_event = eventInfo;
        RETURN_OBJ(&ev_obj->std);
    } else {
        RETURN_NULL();
    }
}

PHP_METHOD(ESLconnection, setAsyncExecute) {
    zend_bool value;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,1)
        Z_PARAM_BOOL(value)
    ZEND_PARSE_PARAMETERS_END();

    auto *conn_obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));

    conn_obj->esl_conn->setAsyncExecute(value);
    RETURN_BOOL(true);
}

PHP_METHOD(ESLconnection, setEventLock) {
    zend_bool value;

    ZEND_PARSE_PARAMETERS_START_EX(ZEND_PARSE_PARAMS_THROW,1,1)
            Z_PARAM_BOOL(value)
    ZEND_PARSE_PARAMETERS_END();

    auto *conn_obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));

    conn_obj->esl_conn->setEventLock(value);
    RETURN_BOOL(true);
}

PHP_METHOD(ESLconnection, sendEvent) {
    ESLevent *event;
    zval *event_zval;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &event_zval, esl_event_ce) == FAILURE) {
        RETURN_NULL();
    }

    esl_event_object *event_obj = (esl_event_object *)Z_OBJ_P(event_zval);
    event = event_obj->esl_event;

    auto *conn_obj = reinterpret_cast<esl_conn_object *>(Z_OBJ_P(getThis()));
    ESLevent *res_event;
    res_event = conn_obj->esl_conn->sendEvent(event);

    if(res_event) {
        zval obj;
        object_init_ex(&obj, esl_event_ce);
        esl_event_object *res_obj = (esl_event_object*)Z_OBJ_P(&obj);
        res_obj->esl_event = res_event;
        RETURN_OBJ(&res_obj->std);
    } else {
        RETURN_NULL();
    }
}

zend_function_entry esl_conn_functions[] = {
	PHP_ME(ESLconnection, __construct, arginfo_conn_construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(ESLconnection, __destruct, arginfo_conn_destruct, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, connected, arginfo_conn_connected, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, disconnect, arginfo_conn_disconnect, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, socketDescriptor, arginfo_conn_socketdescriptor, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, sendRecv, arginfo_conn_sendrecv, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, send, arginfo_conn_send, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, events, arginfo_conn_events, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, filter, arginfo_conn_filter, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, sendEvent, arginfo_conn_sendevent, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, recvEvent, arginfo_conn_recv, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, recvEventTimed, arginfo_conn_recvtimed, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, api, arginfo_conn_api, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, bgapi, arginfo_conn_api, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, execute, arginfo_conn_execute, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, executeAsync, arginfo_conn_executeAsync, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, getInfo, arginfo_conn_info, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, setEventLock, arginfo_conn_eventlock, ZEND_ACC_PUBLIC)
    PHP_ME(ESLconnection, setAsyncExecute, arginfo_conn_asyncexecute, ZEND_ACC_PUBLIC)
    PHP_FE_END
};