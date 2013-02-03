
#include "php.h"
#include "php_ini.h"
#include "php_streams.h"
#include "ext/standard/php_smart_str.h"

#include "php_verdep.h"
#include "php_handlersocketi.h"
#include "handlersocketi_exception.h"
#include "handlersocketi_class.h"
#include "util/common.h"
#include "util/request.h"
#include "util/response.h"

ZEND_EXTERN_MODULE_GLOBALS(handlersocketi);

#define HS_STREAM_DEFAULT_TIMEOUT 5

static zend_class_entry *hs_ce;
static zend_object_handlers hs_object_handlers;

typedef struct hs_obj {
    zend_object std;
    php_stream *stream;
    long timeout;
    zval *server;
    zval *auth;
    zval *error;
    zval *persistent;
    zend_bool is_persistent;
} hs_obj_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_method__construct, 0, 0, 2)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, port)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_method_auth, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_method_open_index, 0, 0, 3)
    ZEND_ARG_INFO(0, db)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, fields)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_METHOD(HandlerSocketi, __construct);
ZEND_METHOD(HandlerSocketi, auth);
ZEND_METHOD(HandlerSocketi, open_index);

const zend_function_entry hs_methods[] = {
    ZEND_ME(HandlerSocketi, __construct,
            arginfo_hs_method__construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    ZEND_ME(HandlerSocketi, auth,
            arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi, open_index,
            arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi, openIndex, open_index,
                arginfo_hs_method_open_index, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

static void
hs_object_free_storage(void *object TSRMLS_DC)
{
    hs_obj_t *intern = (hs_obj_t *)object;
    zend_object_std_dtor(&intern->std TSRMLS_CC);

    if (intern->stream) {
        if (!intern->persistent) {
            php_stream_close(intern->stream);
        }
    }

    if (intern->server) {
        zval_ptr_dtor(&intern->server);
    }

    if (intern->auth) {
        zval_ptr_dtor(&intern->auth);
    }

    if (intern->error) {
        zval_ptr_dtor(&intern->error);
    }

    if (intern->persistent) {
        zval_ptr_dtor(&intern->persistent);
    }

    efree(object);
}

#define HS_EXCEPTION(...)                                     \
    zend_throw_exception_ex(handlersocketi_get_ce_exception(), \
                            0 TSRMLS_CC,                      \
                            "HandlerSocketi::" __VA_ARGS__)

#define HS_CHECK_OBJECT(object, classname)                        \
    if (!(object)) {                                              \
        HS_EXCEPTION("The " #classname " object has not been "    \
                     "correctly initialized by its constructor"); \
        RETURN_FALSE;                                             \
    }

static inline zend_object_value
hs_object_new_ex(zend_class_entry *ce, hs_obj_t **ptr TSRMLS_DC)
{
    hs_obj_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = emalloc(sizeof(hs_obj_t));
    memset(intern, 0, sizeof(hs_obj_t));
    if (ptr) {
        *ptr = intern;
    }

    zend_object_std_init(&intern->std, ce TSRMLS_CC);

#if ZEND_MODULE_API_NO >= 20100525
    object_properties_init(&intern->std, ce);
#else
    zend_hash_copy(intern->std.properties, &ce->default_properties,
                   (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));
#endif

    retval.handle = zend_objects_store_put(
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,
        (zend_objects_free_object_storage_t)hs_object_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &hs_object_handlers;

    intern->timeout = 0;
    intern->stream = NULL;
    intern->server = NULL;
    intern->auth = NULL;
    intern->error = NULL;
    intern->persistent = NULL;

    return retval;
}

static inline int
hs_object_connection(hs_obj_t *obj)
{
    char *hashkey = NULL;
    char *errstr = NULL;
    int err;
    struct timeval tv;

    if (obj->timeout > 0) {
        tv.tv_sec = obj->timeout;
        tv.tv_usec = 0;
    }

    if (obj->persistent) {
        hashkey = Z_STRVAL_P(obj->persistent);
    }

    obj->stream = php_stream_xport_create(Z_STRVAL_P(obj->server),
                                          Z_STRLEN_P(obj->server),
                                          ENFORCE_SAFE_MODE|REPORT_ERRORS,
                                          STREAM_XPORT_CLIENT|
                                          STREAM_XPORT_CONNECT,
                                          hashkey, &tv, NULL, &errstr, &err);

    if (!obj->stream) {
        if (errstr) {
            efree(errstr);
        }
        return FAILURE;
    }

    if (errstr) {
        efree(errstr);
    }

    /* non-blocking */
    if (php_stream_set_option(obj->stream, PHP_STREAM_OPTION_BLOCKING,
                              0, NULL) == -1) {
        zend_error(E_WARNING,
                   "HandlerSocketi: Un set non-blocking mode on a stream");
    }

    return SUCCESS;
}

static inline zend_object_value
hs_object_new(zend_class_entry *ce TSRMLS_DC)
{
    return hs_object_new_ex(ce, NULL TSRMLS_CC);
}

static inline zend_object_value
hs_object_clone(zval *this_ptr TSRMLS_DC)
{
    hs_obj_t *new_obj = NULL;
    hs_obj_t *old_obj =
        (hs_obj_t *)zend_object_store_get_object(this_ptr TSRMLS_CC);
    zend_object_value new_ov = hs_object_new_ex(old_obj->std.ce,
                                                &new_obj TSRMLS_CC);

    zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std,
                               Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

    new_obj->timeout = old_obj->timeout;

    MAKE_STD_ZVAL(new_obj->server);
    *new_obj->server = *old_obj->server;
    zval_copy_ctor(new_obj->server);

    if (old_obj->auth) {
        MAKE_STD_ZVAL(new_obj->auth);
        *new_obj->auth = *old_obj->auth;
        zval_copy_ctor(new_obj->auth);
    } else {
        new_obj->auth = NULL;
    }

    new_obj->error = NULL;

    if (old_obj->persistent) {
        MAKE_STD_ZVAL(new_obj->persistent);
        *new_obj->persistent = *old_obj->persistent;
        zval_copy_ctor(new_obj->persistent);
    } else {
        new_obj->persistent = NULL;
    }

    hs_object_connection(new_obj);

    return new_ov;
}

PHP_HANDLERSOCKETI_API int
handlersocketi_register_class(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "HandlerSocketi", hs_methods);

    ce.create_object = hs_object_new;

    hs_ce = zend_register_internal_class(&ce TSRMLS_CC);
    if (hs_ce == NULL) {
        return FAILURE;
    }

    memcpy(&hs_object_handlers, zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));

    hs_object_handlers.clone_obj = hs_object_clone;

    return SUCCESS;
}

PHP_HANDLERSOCKETI_API zend_class_entry
*handlersocketi_get_ce(void)
{
    return hs_ce;
}

PHP_HANDLERSOCKETI_API php_stream
*handlersocketi_object_store_get_stream(zval *link)
{
    hs_obj_t *hs;

    hs = (hs_obj_t *)zend_object_store_get_object(link TSRMLS_CC);
    if (hs) {
        return hs->stream;
    } else {
        return NULL;
    }
}

PHP_HANDLERSOCKETI_API long
handlersocketi_object_store_get_timeout(zval *link)
{
    hs_obj_t *hs;

    hs = (hs_obj_t *)zend_object_store_get_object(link TSRMLS_CC);
    if (hs) {
        return hs->timeout;
    } else {
        return 0;
    }
}

ZEND_METHOD(HandlerSocketi, __construct)
{
    char *server, *host = NULL;
    int server_len, host_len = 0;
    long port = -1;
    zval *options = NULL;
    hs_obj_t *hs;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl|z",
                              &host, &host_len, &port, &options) == FAILURE) {
        zval *object = getThis();
        ZVAL_NULL(object);
        return;
    }

    if (!host || host_len <= 0) {
        HS_EXCEPTION("__construct(): no host name");
        zval *object = getThis();
        ZVAL_NULL(object);
        return;
    }

    if (port > 0) {
        server_len = spprintf(&server, 0, "%s:%ld", host, port);
    } else {
        server_len = spprintf(&server, 0, "%s", host);
    }

    hs = (hs_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hs, HandlerSocketi);

    MAKE_STD_ZVAL(hs->server);
    ZVAL_STRINGL(hs->server, server, server_len, 1);
    efree(server);

    hs->timeout = HS_STREAM_DEFAULT_TIMEOUT;

    if (options && Z_TYPE_P(options) == IS_ARRAY) {
        zval **tmp;
        if (zend_hash_find(Z_ARRVAL_P(options), "timeout",
                           sizeof("timeout"), (void **)&tmp) == SUCCESS) {
            convert_to_long_ex(tmp);
            hs->timeout = Z_LVAL_PP(tmp);
        }

        if (zend_hash_find(Z_ARRVAL_P(options), "persistent",
                           sizeof("persistent"), (void **)&tmp) == SUCCESS) {
            convert_to_string_ex(tmp);
            MAKE_STD_ZVAL(hs->persistent);
            ZVAL_STRINGL(hs->persistent, Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp), 1);
        }
    }

    if (hs_object_connection(hs) != SUCCESS) {
        HS_EXCEPTION("__construct(): unable to connect %s",
                     Z_STRVAL_P(hs->server));
        RETURN_FALSE;
    }
}

ZEND_METHOD(HandlerSocketi, auth)
{
    char *key, *type = NULL;
    long key_len, type_len = 0;
    hs_obj_t *hs;
    smart_str request = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s",
                              &key, &key_len, &type, &type_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (key_len <= 0) {
        RETURN_FALSE;
    }

    /* handerlsocket: object */
    hs = (hs_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hs, HandlerSocketi);

    MAKE_STD_ZVAL(hs->auth);
    ZVAL_STRINGL(hs->auth, key, key_len, 1);

    /* auth */
    hs_request_string(&request, HS_PROTOCOL_AUTH, 1);
    hs_request_delim(&request);
    hs_request_string(&request, "1", 1);
    hs_request_delim(&request);
    hs_request_string(&request, Z_STRVAL_P(hs->auth), Z_STRLEN_P(hs->auth));
    hs_request_next(&request);

    /* request: send */
    if (hs_request_send(hs->stream, &request TSRMLS_CC) < 0) {
        ZVAL_BOOL(return_value, 0);
    } else {
        zval *retval;
        MAKE_STD_ZVAL(retval);

        /* response */
        hs_response_value(hs->stream, hs->timeout, retval, NULL, 0 TSRMLS_CC);
        if (Z_TYPE_P(retval) == IS_BOOL && Z_LVAL_P(retval) == 1) {
            ZVAL_BOOL(return_value, 1);
        } else {
            ZVAL_BOOL(return_value, 0);
        }

        zval_ptr_dtor(&retval);
    }

    smart_str_free(&request);
}

ZEND_METHOD(HandlerSocketi, open_index)
{
    char *db, *table;
    int db_len, table_len;
    zval *fields, *options = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz|z",
                              &db, &db_len, &table, &table_len,
                              &fields, &options) == FAILURE) {
        return;
    }

    handlersocketi_create_index(return_value, getThis(), db, db_len,
                                table, table_len, fields, options TSRMLS_CC);
}
