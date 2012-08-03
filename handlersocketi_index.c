
#include "php.h"
#include "php_ini.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_string.h"

#include "php_handlersocketi.h"
#include "handlersocketi_exception.h"
#include "handlersocketi_class.h"
#include "handlersocketi_index.h"
#include "util/common.h"
#include "util/request.h"
#include "util/response.h"

ZEND_EXTERN_MODULE_GLOBALS(handlersocketi);

#define HS_DEFAULT_LIMIT  1
#define HS_DEFAULT_OFFSET 0

static zend_class_entry *hs_ce_index;
static zend_object_handlers hs_index_object_handlers;

typedef struct hs_index_obj {
    zend_object std;
    long id;
    zval *name;
    zval *db;
    zval *table;
    zval *field;
    long field_num;
    zval *filter;
    zval *link;
    zval *error;
} hs_index_obj_t;

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method__construct, 0, 0, 4)
    ZEND_ARG_INFO(0, hs)
    ZEND_ARG_INFO(0, db)
    ZEND_ARG_INFO(0, table)
    ZEND_ARG_INFO(0, fields)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_find, 0, 0, 1)
    ZEND_ARG_INFO(0, query)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_insert, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_update, 0, 0, 2)
    ZEND_ARG_INFO(0, query)
    ZEND_ARG_INFO(0, update)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_remove, 0, 0, 1)
    ZEND_ARG_INFO(0, query)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_multi, 0, 0, 1)
    ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_error, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_id, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_name, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_db, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_table, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_field, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_filter, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_hs_index_method_get_operator, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_METHOD(HandlerSocketi_Index, __construct);
ZEND_METHOD(HandlerSocketi_Index, find);
ZEND_METHOD(HandlerSocketi_Index, insert);
ZEND_METHOD(HandlerSocketi_Index, update);
ZEND_METHOD(HandlerSocketi_Index, remove);
ZEND_METHOD(HandlerSocketi_Index, multi);
ZEND_METHOD(HandlerSocketi_Index, get_error);
ZEND_METHOD(HandlerSocketi_Index, get_id);
ZEND_METHOD(HandlerSocketi_Index, get_name);
ZEND_METHOD(HandlerSocketi_Index, get_db);
ZEND_METHOD(HandlerSocketi_Index, get_table);
ZEND_METHOD(HandlerSocketi_Index, get_field);
ZEND_METHOD(HandlerSocketi_Index, get_filter);
ZEND_METHOD(HandlerSocketi_Index, get_operator);

const zend_function_entry hs_index_methods[] = {
    ZEND_ME(HandlerSocketi_Index, __construct,
            arginfo_hs_index_method__construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    ZEND_ME(HandlerSocketi_Index, find,
            arginfo_hs_index_method_find, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, insert,
            arginfo_hs_index_method_insert, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, update,
            arginfo_hs_index_method_update, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, remove,
            arginfo_hs_index_method_remove, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, multi,
            arginfo_hs_index_method_multi, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, get_error,
            arginfo_hs_index_method_get_error, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, get_id,
            arginfo_hs_index_method_get_id, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, get_name,
            arginfo_hs_index_method_get_name, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, get_db,
            arginfo_hs_index_method_get_db, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, get_table,
            arginfo_hs_index_method_get_table, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, get_field,
            arginfo_hs_index_method_get_field, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, get_filter,
            arginfo_hs_index_method_get_filter, ZEND_ACC_PUBLIC)
    ZEND_ME(HandlerSocketi_Index, get_operator,
            arginfo_hs_index_method_get_operator, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi_Index, getError, get_error,
                arginfo_hs_index_method_get_error, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi_Index, getId, get_id,
                arginfo_hs_index_method_get_id, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi_Index, getName, get_name,
                arginfo_hs_index_method_get_name, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi_Index, getDb, get_db,
                arginfo_hs_index_method_get_db, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi_Index, getTable, get_table,
                arginfo_hs_index_method_get_table, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi_Index, getField, get_field,
                arginfo_hs_index_method_get_field, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi_Index, getFilter, get_filter,
                arginfo_hs_index_method_get_filter, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(HandlerSocketi_Index, getOperator, get_operator,
                arginfo_hs_index_method_get_operator, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

#define HS_EXCEPTION(...)                                       \
    zend_throw_exception_ex(handlersocketi_get_ce_exception(),   \
                            0 TSRMLS_CC,                        \
                            "HandlerSocketi_Index::" __VA_ARGS__)

#define HS_CHECK_OBJECT(object, classname)                        \
    if (!(object)) {                                              \
        HS_EXCEPTION("The " #classname " object has not been "    \
                     "correctly initialized by its constructor"); \
        RETURN_FALSE;                                             \
    }

#define HS_ERROR_RESET(error)  \
    if (error) {               \
        zval_ptr_dtor(&error); \
    }                          \
    MAKE_STD_ZVAL(error);      \
    ZVAL_NULL(error)

static inline void
hs_index_object_free_storage(void *object TSRMLS_DC)
{
    hs_index_obj_t *intern = (hs_index_obj_t *)object;
    zend_object_std_dtor(&intern->std TSRMLS_CC);

    if (intern->link) {
        zval_ptr_dtor(&intern->link);
    }

    if (intern->name) {
        zval_ptr_dtor(&intern->name);
    }

    if (intern->db) {
        zval_ptr_dtor(&intern->db);
    }

    if (intern->table) {
        zval_ptr_dtor(&intern->table);
    }

    if (intern->field) {
        zval_ptr_dtor(&intern->field);
    }

    if (intern->filter) {
        zval_ptr_dtor(&intern->filter);
    }

    if (intern->error) {
        zval_ptr_dtor(&intern->error);
    }

    efree(object);
}

static inline zend_object_value
hs_index_object_new_ex(zend_class_entry *ce, hs_index_obj_t **ptr TSRMLS_DC)
{
    hs_index_obj_t *intern;
    zend_object_value retval;
#if ZEND_MODULE_API_NO < 20100525
    zval *tmp;
#endif

    intern = emalloc(sizeof(hs_index_obj_t));
    memset(intern, 0, sizeof(hs_index_obj_t));
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
        (zend_objects_free_object_storage_t)hs_index_object_free_storage,
        NULL TSRMLS_CC);
    retval.handlers = &hs_index_object_handlers;

    intern->id = 0;
    intern->name = NULL;
    intern->db = NULL;
    intern->table = NULL;
    intern->field = NULL;
    intern->field_num = 0;
    intern->filter = NULL;
    intern->link = NULL;
    intern->error = NULL;

    return retval;
}


static inline zend_object_value
hs_index_object_new(zend_class_entry *ce TSRMLS_DC)
{
    return hs_index_object_new_ex(ce, NULL TSRMLS_CC);
}

static inline zend_object_value
hs_index_object_clone(zval *this_ptr TSRMLS_DC)
{
    hs_index_obj_t *new_obj = NULL;
    hs_index_obj_t *old_obj =
        (hs_index_obj_t *)zend_object_store_get_object(this_ptr TSRMLS_CC);
    zend_object_value new_ov = hs_index_object_new_ex(old_obj->std.ce,
                                                      &new_obj TSRMLS_CC);

    zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std,
                               Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

    new_obj->id = old_obj->id;

    MAKE_STD_ZVAL(new_obj->name);
    *new_obj->name = *old_obj->name;
    zval_copy_ctor(new_obj->name);

    MAKE_STD_ZVAL(new_obj->db);
    *new_obj->db = *old_obj->db;
    zval_copy_ctor(new_obj->db);

    MAKE_STD_ZVAL(new_obj->table);
    *new_obj->table = *old_obj->table;
    zval_copy_ctor(new_obj->table);

    MAKE_STD_ZVAL(new_obj->field);
    *new_obj->field = *old_obj->field;
    zval_copy_ctor(new_obj->field);

    new_obj->field_num = old_obj->field_num;

    if (old_obj->filter) {
        MAKE_STD_ZVAL(new_obj->filter);
        *new_obj->filter = *old_obj->filter;
        zval_copy_ctor(new_obj->filter);
    } else {
        new_obj->filter = NULL;
    }

    MAKE_STD_ZVAL(new_obj->link);
    *new_obj->link = *old_obj->link;
    zval_copy_ctor(new_obj->link);

    new_obj->error = NULL;

    return new_ov;
}

static inline zval
*hs_zval_to_comma_string(zval *val)
{
    smart_str comma = {0};
    zval *retval;

    if (Z_TYPE_P(val) == IS_ARRAY) {
        zval **tmp;
        HashTable *ht;
        HashPosition pos;

        ht = HASH_OF(val);
        if (zend_hash_num_elements(ht) >= 0) {
            zend_hash_internal_pointer_reset_ex(ht, &pos);
            while (zend_hash_get_current_data_ex(ht, (void **)&tmp,
                                                 &pos) == SUCCESS) {
                switch ((*tmp)->type) {
                    case IS_STRING:
                        if (Z_STRLEN_PP(tmp) > 0) {
                            smart_str_appendl_ex(&comma, Z_STRVAL_PP(tmp),
                                                 Z_STRLEN_PP(tmp), 1);
                            smart_str_appendl_ex(&comma, ",", strlen(","), 1);
                        }
                        break;
                    default:
                        convert_to_string(*tmp);
                        if (Z_STRLEN_PP(tmp) > 0) {
                            smart_str_appendl_ex(&comma, Z_STRVAL_PP(tmp),
                                                 Z_STRLEN_PP(tmp), 1);
                            smart_str_appendl_ex(&comma, ",", strlen(","), 1);
                        }
                        break;
                }
                zend_hash_move_forward_ex(ht, &pos);
            }
            comma.len--;
            comma.a--;
        }
    } else if (Z_TYPE_P(val) == IS_STRING) {
        if (Z_STRLEN_P(val) > 0) {
            smart_str_appendl_ex(&comma, Z_STRVAL_P(val), Z_STRLEN_P(val), 1);
        }
    } else {
        convert_to_string(val);
        if (Z_STRLEN_P(val) > 0) {
            smart_str_appendl_ex(&comma, Z_STRVAL_P(val), Z_STRLEN_P(val), 1);
        }
    }

    MAKE_STD_ZVAL(retval);
    ZVAL_STRINGL(retval, comma.c, comma.len, 1);
    smart_str_free(&comma);

    return retval;
}

static inline zval
*hs_zval_to_comma_array(zval *val)
{
    zval *retval;

    MAKE_STD_ZVAL(retval);

    if (Z_TYPE_P(val) == IS_ARRAY) {
        long n, i;
        /*
        array_init(retval);
        n = zend_hash_num_elements(HASH_OF(*tmp));
        for (i = 0; i < n; i++) {
            zval **val;
            if (zend_hash_index_find(HASH_OF(*tmp), i,
                                     (void **)&val) == SUCCESS) {
                convert_to_string(*val);
                add_next_index_stringl(retval, Z_STRVAL_PP(val),
                                       Z_STRLEN_PP(val), 1);
            }
        }
        */
        *retval = *val;
        zval_copy_ctor(retval);
    } else if (Z_TYPE_P(val) == IS_STRING) {
        zval delim;
        ZVAL_STRINGL(&delim, ",", strlen(","), 0);

        array_init(retval);
        php_explode(&delim, val, retval, LONG_MAX);
    } else {
        ZVAL_NULL(retval);
    }

    return retval;
}

static inline int
hs_get_options_long(HashTable *options, char *item, long def TSRMLS_DC)
{
    zval **tmp;

    if (zend_hash_find(options, item, strlen(item)+1,
                       (void**)&tmp) == SUCCESS &&
        Z_TYPE_PP(tmp) == IS_LONG) {
        return Z_LVAL_PP(tmp);
    }
    return def;
}

static inline int
hs_is_options_safe(HashTable *options TSRMLS_DC)
{
    zval **tmp;

    if (zend_hash_find(options, "safe", sizeof("safe"),
                       (void**)&tmp) == SUCCESS) {
        if (Z_TYPE_PP(tmp) == IS_STRING ||
            ((Z_TYPE_PP(tmp) == IS_LONG || Z_TYPE_PP(tmp) == IS_BOOL) &&
             Z_LVAL_PP(tmp) >= 1)) {
            return 1;
        }
    }
    return 0;
}

static inline int
hs_zval_to_operate_criteria(zval *query, zval *operate,
                            zval **criteria, char *defaults TSRMLS_DC)
{
    if (query == NULL) {
        return FAILURE;
    }

    if (Z_TYPE_P(query) == IS_ARRAY) {
        char *key;
        uint key_len;
        ulong index;
        HashTable *ht;
        zval **tmp;

        ht = HASH_OF(query);
        if (zend_hash_get_current_data_ex(ht, (void **)&tmp, NULL) != SUCCESS) {
            return FAILURE;
        }

        if (zend_hash_get_current_key_ex(ht, &key, &key_len, &index,
                                         0, NULL) == HASH_KEY_IS_STRING) {
            ZVAL_STRINGL(operate, key, key_len-1, 1);
            *criteria = *tmp;
        } else {
            ZVAL_STRINGL(operate, defaults, strlen(defaults), 1);
            *criteria = query;
        }
    } else {
        ZVAL_STRINGL(operate, defaults, strlen(defaults), 1);
        *criteria = query;
    }

    return SUCCESS;
}

static inline zval
*hs_zval_search_key(zval *value, zval *array TSRMLS_DC)
{
    zval *return_value, **entry, res;
    HashPosition pos;
    HashTable *ht;
    ulong index;
    uint key_len;
    char *key;
    int (*is_equal_func)(zval *, zval *, zval * TSRMLS_DC) = is_equal_function;

    MAKE_STD_ZVAL(return_value);

    ht = HASH_OF(array);
    zend_hash_internal_pointer_reset_ex(ht, &pos);
    while (zend_hash_get_current_data_ex(ht, (void **)&entry,
                                         &pos) == SUCCESS) {
        is_equal_func(&res, value, *entry TSRMLS_CC);
        if (Z_LVAL(res)) {
            switch (zend_hash_get_current_key_ex(ht, &key, &key_len,
                                                 &index, 0, &pos)) {
                case HASH_KEY_IS_STRING:
                    ZVAL_STRINGL(return_value, key, key_len - 1, 1);
                    break;
                case HASH_KEY_IS_LONG:
                    ZVAL_LONG(return_value, index);
                    break;
                default:
                    ZVAL_NULL(return_value);
                    break;
            }
            return return_value;
        }
        zend_hash_move_forward_ex(ht, &pos);
    }
    ZVAL_NULL(return_value);
    return return_value;
}

static inline void
hs_zval_to_filter(zval **return_value, zval *filter,
                  zval *value, char *type TSRMLS_DC)
{
    HashTable *ht;
    HashPosition pos;
    zval **tmp, **ftmp, **vtmp, *index, *item;
    long n;

    if (value == NULL || Z_TYPE_P(value) != IS_ARRAY) {
        return;
    }

    ht = HASH_OF(value);
    n = zend_hash_num_elements(ht);

    if (n <= 0 || zend_hash_index_find(ht, 0, (void **)&ftmp) != SUCCESS) {
        return;
    }

    zend_hash_internal_pointer_reset_ex(ht, &pos);

    if (Z_TYPE_PP(ftmp) == IS_ARRAY) {
        do {
            if (zend_hash_move_forward_ex(ht, &pos) < 0) {
                break;
            }
            hs_zval_to_filter(return_value, filter, *ftmp, type TSRMLS_CC);
        } while (zend_hash_get_current_data_ex(
                     ht, (void **)&ftmp, &pos) == SUCCESS);
        return;
    } else if (n < 3) {
        return;
    }

    if (zend_hash_index_find(ht, 1, (void **)&tmp) != SUCCESS) {
        return;
    }

    index = hs_zval_search_key(*ftmp, filter TSRMLS_CC);
    if (Z_TYPE_P(index) != IS_LONG) {
        zval_ptr_dtor(&index);
        return;
    }

    if (zend_hash_index_find(ht, 2, (void **)&vtmp) != SUCCESS) {
        zval_ptr_dtor(&index);
        return;
    }

    MAKE_STD_ZVAL(item);
    array_init(item);

    add_next_index_stringl(item, type, strlen(type), 1);

    convert_to_string(*tmp);
    add_next_index_stringl(item, Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp), 1);

    add_next_index_long(item, Z_LVAL_P(index));

    if (Z_TYPE_PP(vtmp) == IS_NULL) {
        add_next_index_null(item);
    } else if (Z_TYPE_PP(vtmp) == IS_LONG) {
        add_next_index_long(item, Z_LVAL_PP(vtmp));
    } else if (Z_TYPE_PP(vtmp) == IS_DOUBLE) {
        add_next_index_double(item, Z_DVAL_PP(vtmp));
    } else {
        convert_to_string(*tmp);
        add_next_index_stringl(item, Z_STRVAL_PP(vtmp), Z_STRLEN_PP(vtmp), 1);
    }

    if (!(*return_value)) {
        MAKE_STD_ZVAL(*return_value);
        array_init(*return_value);
    }

    add_next_index_zval(*return_value, item);

    zval_ptr_dtor(&index);
}

static inline void
hs_array_to_in_filter(HashTable *ht, zval *filter, zval **filters,
                      long *in_key, zval **in_values TSRMLS_DC)
{
    HashPosition pos;
    zval **val;
    char *key;
    ulong key_index;
    uint key_len;

    if (!filter) {
        return;
    }

    zend_hash_internal_pointer_reset_ex(ht, &pos);
    while (zend_hash_get_current_data_ex(ht, (void **)&val, &pos) == SUCCESS) {
        if (zend_hash_get_current_key_ex(ht, &key, &key_len,
                                         &key_index, 0, &pos)
            != HASH_KEY_IS_STRING) {
            zend_hash_move_forward_ex(ht, &pos);
            continue;
        }

        if (strcmp(key, "in") == 0) {
            /* in */
            if (Z_TYPE_PP(val) == IS_ARRAY) {
                HashTable *in_ht;
                HashPosition in_pos;
                zval **tmp;
                char *in_key_name;
                ulong in_key_index;
                uint in_key_len;

                in_ht = HASH_OF(*val);

                zend_hash_internal_pointer_reset_ex(in_ht, &in_pos);
                if (zend_hash_get_current_data_ex(in_ht, (void **)&tmp,
                                                  &in_pos) == SUCCESS) {
                    if (Z_TYPE_PP(tmp) == IS_ARRAY) {
                        switch (zend_hash_get_current_key_ex(
                                    in_ht, &in_key_name, &in_key_len,
                                    &in_key_index, 0, &in_pos)) {
                            case HASH_KEY_NON_EXISTANT:
                                *in_key = 0;
                                break;
                            case HASH_KEY_IS_LONG:
                                *in_key = in_key_index;
                                break;
                            default:
                            {
                                zval *key;
                                MAKE_STD_ZVAL(key);
                                ZVAL_STRINGL(key, in_key_name, in_key_len, 1);
                                convert_to_long(key);
                                *in_key = Z_LVAL_P(key);
                                zval_ptr_dtor(&key);
                                break;

                            }
                        }
                        *in_values = *tmp;
                    } else {
                        *in_key = 0;
                        *in_values = *val;
                    }
                }
            } else {
                *in_key = 0;
                *in_values = *val;
            }
        } else if (strcmp(key, "filter") == 0 && filter != NULL) {
            /* filter */
            hs_zval_to_filter(filters, filter, *val,
                              HS_PROTOCOL_FILTER TSRMLS_CC);
        } else if (strcmp(key, "while") == 0 && filter != NULL) {
            /* while */
            hs_zval_to_filter(filters, filter, *val,
                              HS_PROTOCOL_WHILE TSRMLS_CC);
        }

        zend_hash_move_forward_ex(ht, &pos);
    }
}

static inline void
hs_index_object_init(hs_index_obj_t *hsi,
                     zval *link, char *db, long db_len,
                     char *table, long table_len,
                     zval *fields, zval *options TSRMLS_DC)
{
    int id = 0;
    zval *index = NULL, *fields_str = NULL, *filter = NULL, *retval;
    php_stream *stream;
    long timeout;
    smart_str request = {0};

    if (db_len == 0) {
        HS_EXCEPTION("__construct(): invalid database %s", db);
        return;
    }

    if (table_len == 0) {
        HS_EXCEPTION("__construct(): invalid table %s", table);
        return;
    }

    fields_str = hs_zval_to_comma_string(fields);
    if (!fields_str) {
        HS_EXCEPTION("__construct(): invalid fields");
        return;
    }

    if (options && Z_TYPE_P(options) == IS_ARRAY) {
        zval **tmp;
        if (zend_hash_find(Z_ARRVAL_P(options), "id",
                           sizeof("id"), (void **)&tmp) == SUCCESS) {
            convert_to_long_ex(tmp);
            id = Z_LVAL_PP(tmp);
        }

        if (zend_hash_find(Z_ARRVAL_P(options), "index",
                           sizeof("index"), (void **)&tmp) == SUCCESS) {
            convert_to_string_ex(tmp);
            MAKE_STD_ZVAL(index);
            ZVAL_STRINGL(index, Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp), 1);
        }

        if (zend_hash_find(Z_ARRVAL_P(options), "filter",
                           sizeof("filter"), (void **)&tmp) == SUCCESS) {
            filter = hs_zval_to_comma_array(*tmp);
        }
    }

    if (!index) {
        MAKE_STD_ZVAL(index);
        ZVAL_STRINGL(index, "PRIMARY", sizeof("PRIMARY")-1, 1);
    } else if (Z_STRLEN_P(index) <= 0) {
        if (index) {
            zval_ptr_dtor(&index);
        }
        if (fields_str) {
            zval_ptr_dtor(&fields_str);
        }
        if (filter) {
            zval_ptr_dtor(&filter);
        }
        HS_EXCEPTION("__construct(): invalid index");
        return;
    }

    if (id <= 0) {
        id = HANDLERSOCKETI_G(id)++;
    }

    /* handerlsocket: object */
    //hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    //HS_CHECK_OBJECT(hsi, HandlersocketiIndex);
    if (!hsi) {
        HS_EXCEPTION("The HandlerSocketi_Index object has not been "
                     "correctly initialized by its constructor");
        return;
    }

    hsi->link = link;
    zval_add_ref(&hsi->link);

    /* id */
    hsi->id = id;

    /* name */
    MAKE_STD_ZVAL(hsi->name);
    *hsi->name = *index;
    zval_copy_ctor(hsi->name);

    /* db */
    MAKE_STD_ZVAL(hsi->db);
    ZVAL_STRINGL(hsi->db, db, db_len, 1);

    /* table */
    MAKE_STD_ZVAL(hsi->table);
    ZVAL_STRINGL(hsi->table, table, db_len, 1);

    /* field */
    MAKE_STD_ZVAL(hsi->field);
    if (Z_TYPE_P(fields) == IS_STRING) {
        zval delim, *arr;
        MAKE_STD_ZVAL(arr);
        array_init(arr);

        ZVAL_STRINGL(&delim, ",", strlen(","), 0);
        php_explode(&delim, fields, arr, LONG_MAX);

        hsi->field_num = zend_hash_num_elements(HASH_OF(arr));

        *hsi->field = *arr;
        zval_copy_ctor(hsi->field);
        zval_ptr_dtor(&arr);
    } else {
        hsi->field_num = zend_hash_num_elements(HASH_OF(fields));
        *hsi->field = *fields;
        zval_copy_ctor(hsi->field);
    }

    if (filter) {
        MAKE_STD_ZVAL(hsi->filter);
        *hsi->filter = *filter;
        zval_copy_ctor(hsi->filter);
    }

    /* stream */
    stream = handlersocketi_object_store_get_stream(hsi->link);
    timeout = handlersocketi_object_store_get_timeout(hsi->link);

    hs_request_string(&request, HS_PROTOCOL_OPEN, 1);
    hs_request_delim(&request);

    /* id */
    hs_request_long(&request, hsi->id);
    hs_request_delim(&request);

    /* db */
    hs_request_string(&request, db, db_len);
    hs_request_delim(&request);

    /* table */
    hs_request_string(&request, table, table_len);
    hs_request_delim(&request);

    /* index */
    hs_request_string(&request, Z_STRVAL_P(index), Z_STRLEN_P(index));
    hs_request_delim(&request);

    /* fields */
    hs_request_string(&request, Z_STRVAL_P(fields_str), Z_STRLEN_P(fields_str));

    /* filters */
    if (hsi->filter) {
        hs_request_filter(&request, HASH_OF(hsi->filter) TSRMLS_CC);
    }

    /* eol */
    hs_request_next(&request);

    /* request: send */
    if (hs_request_send(stream, &request TSRMLS_CC) < 0) {
        smart_str_free(&request);
        zval_ptr_dtor(&index);
        if (fields_str) {
            zval_ptr_dtor(&fields_str);
        }
        HS_EXCEPTION("__construct(): invalid request send");
        return;
    }

    smart_str_free(&request);

    /* response */
    MAKE_STD_ZVAL(retval);
    hs_response_value(stream, timeout, retval, hsi->error, 0 TSRMLS_CC);
    if (Z_TYPE_P(retval) == IS_BOOL && Z_LVAL_P(retval) == 0) {
        HS_EXCEPTION("__construct(): unable to open index: %ld: %s",
                     id,
                     hsi->error == NULL ?
                     "Unknwon error" : Z_STRVAL_P(hsi->error));
        return;
    }

    /* cleanup */
    zval_ptr_dtor(&retval);
    zval_ptr_dtor(&index);
    if (fields_str) {
        zval_ptr_dtor(&fields_str);
    }
}

PHP_HANDLERSOCKETI_API int
handlersocketi_register_index(TSRMLS_D)
{
    zend_class_entry ce;

    INIT_CLASS_ENTRY(ce, "HandlerSocketi_Index", hs_index_methods);

    ce.create_object = hs_index_object_new;

    hs_ce_index = zend_register_internal_class(&ce TSRMLS_CC);
    if (hs_ce_index == NULL) {
        return FAILURE;
    }

    memcpy(&hs_index_object_handlers, zend_get_std_object_handlers(),
           sizeof(zend_object_handlers));

    hs_index_object_handlers.clone_obj = hs_index_object_clone;

    return SUCCESS;
}

PHP_HANDLERSOCKETI_API zend_class_entry
*handlersocketi_get_ce_index(void) {
    return hs_ce_index;
}

PHP_HANDLERSOCKETI_API void
handlersocketi_create_index(zval *return_value,
                            zval *link, char *db, long db_len,
                            char *table, long table_len,
                            zval *fields, zval *options TSRMLS_DC)
{
    object_init_ex(return_value, hs_ce_index);

    hs_index_object_init(zend_object_store_get_object(return_value TSRMLS_CC),
                         link, db, db_len, table, table_len,
                         fields, options TSRMLS_CC);
}

ZEND_METHOD(HandlerSocketi_Index, __construct)
{
    zval *link;
    char *db, *table;
    int db_len, table_len, id = 0;
    zval *fields, *options = NULL;
    zval *index = NULL, *fields_str = NULL, *filter = NULL;
    hs_index_obj_t *hsi;
    php_stream *stream;
    long timeout;
    smart_str request = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ossz|z",
                              &link, handlersocketi_get_ce(),
                              &db, &db_len, &table, &table_len,
                              &fields, &options) == FAILURE) {
        HS_EXCEPTION("__construct(): expects parameters");
        zval *object = getThis();
        ZVAL_NULL(object);
        return;
    }

    if (db_len == 0) {
        HS_EXCEPTION("__construct(): invalid database %s", db);
        return;
    }

    if (table_len == 0) {
        HS_EXCEPTION("__construct(): invalid table %s", table);
        return;
    }

    fields_str = hs_zval_to_comma_string(fields);
    if (!fields_str) {
        HS_EXCEPTION("__construct(): invalid fields");
        return;
    }

    if (options && Z_TYPE_P(options) == IS_ARRAY) {
        zval **tmp;
        if (zend_hash_find(Z_ARRVAL_P(options), "id",
                           sizeof("id"), (void **)&tmp) == SUCCESS) {
            convert_to_long_ex(tmp);
            id = Z_LVAL_PP(tmp);
        }

        if (zend_hash_find(Z_ARRVAL_P(options), "index",
                           sizeof("index"), (void **)&tmp) == SUCCESS) {
            convert_to_string_ex(tmp);
            MAKE_STD_ZVAL(index);
            ZVAL_STRINGL(index, Z_STRVAL_PP(tmp), Z_STRLEN_PP(tmp), 1);
        }

        if (zend_hash_find(Z_ARRVAL_P(options), "filter",
                           sizeof("filter"), (void **)&tmp) == SUCCESS) {
            filter = hs_zval_to_comma_array(*tmp);
        }
    }

    if (!index) {
        MAKE_STD_ZVAL(index);
        ZVAL_STRINGL(index, "PRIMARY", sizeof("PRIMARY")-1, 1);
    } else if (Z_STRLEN_P(index) <= 0) {
        if (index) {
            zval_ptr_dtor(&index);
        }
        if (fields_str) {
            zval_ptr_dtor(&fields_str);
        }
        if (filter) {
            zval_ptr_dtor(&filter);
        }
        HS_EXCEPTION("__construct(): invalid index");
        return;
    }

    if (id <= 0) {
        id = HANDLERSOCKETI_G(id)++;
    }

    /* handerlsocket: object */
    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

    hsi->link = link;
    zval_add_ref(&hsi->link);

    /* id */
    hsi->id = id;

    /* name */
    MAKE_STD_ZVAL(hsi->name);
    *hsi->name = *index;
    zval_copy_ctor(hsi->name);

    /* db */
    MAKE_STD_ZVAL(hsi->db);
    ZVAL_STRINGL(hsi->db, db, db_len, 1);

    /* table */
    MAKE_STD_ZVAL(hsi->table);
    ZVAL_STRINGL(hsi->table, table, db_len, 1);

    /* field */
    MAKE_STD_ZVAL(hsi->field);
    if (Z_TYPE_P(fields) == IS_STRING) {
        zval delim, *arr;
        MAKE_STD_ZVAL(arr);
        array_init(arr);

        ZVAL_STRINGL(&delim, ",", strlen(","), 0);
        php_explode(&delim, fields, arr, LONG_MAX);

        hsi->field_num = zend_hash_num_elements(HASH_OF(arr));

        *hsi->field = *arr;
        zval_copy_ctor(hsi->field);
        zval_ptr_dtor(&arr);
    } else {
        hsi->field_num = zend_hash_num_elements(HASH_OF(fields));
        *hsi->field = *fields;
        zval_copy_ctor(hsi->field);
    }

    if (filter) {
        MAKE_STD_ZVAL(hsi->filter);
        *hsi->filter = *filter;
        zval_copy_ctor(hsi->filter);
    }

    /* stream */
    stream = handlersocketi_object_store_get_stream(hsi->link);
    timeout = handlersocketi_object_store_get_timeout(hsi->link);

    hs_request_string(&request, HS_PROTOCOL_OPEN, 1);
    hs_request_delim(&request);

    /* id */
    hs_request_long(&request, hsi->id);
    hs_request_delim(&request);

    /* db */
    hs_request_string(&request, db, db_len);
    hs_request_delim(&request);

    /* table */
    hs_request_string(&request, table, table_len);
    hs_request_delim(&request);

    /* index */
    hs_request_string(&request, Z_STRVAL_P(index), Z_STRLEN_P(index));
    hs_request_delim(&request);

    /* fields */
    hs_request_string(&request, Z_STRVAL_P(fields_str), Z_STRLEN_P(fields_str));

    /* filters */
    if (hsi->filter) {
        hs_request_filter(&request, HASH_OF(hsi->filter) TSRMLS_CC);
    }

    /* eol */
    hs_request_next(&request);

    /* request: send */
    if (hs_request_send(stream, &request TSRMLS_CC) < 0) {
        smart_str_free(&request);
        zval_ptr_dtor(&index);
        if (fields_str) {
            zval_ptr_dtor(&fields_str);
        }
        HS_EXCEPTION("__construct(): invalid request send");
        return;
    }

    smart_str_free(&request);

    /* response */
    hs_response_value(stream, timeout, return_value, hsi->error, 0 TSRMLS_CC);

    if (Z_TYPE_P(return_value) == IS_BOOL && Z_LVAL_P(return_value) == 0) {
        HS_EXCEPTION("__construct(): unable to open index: %ld: %s",
                     id,
                     hsi->error == NULL ?
                     "Unknwon error" : Z_STRVAL_P(hsi->error));
        return;
    }

    /* cleanup */
    zval_ptr_dtor(&index);
    if (fields_str) {
        zval_ptr_dtor(&fields_str);
    }
}

ZEND_METHOD(HandlerSocketi_Index, find)
{
    zval *query, *operate, *criteria;
    zval *options = NULL;
    long safe = -1, limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;
    zval *filters = NULL, *in_values = NULL;
    long in_key = -1;
    hs_index_obj_t *hsi;
    php_stream *stream;
    long timeout;
    smart_str request = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z",
                              &query, &options) == FAILURE) {
        RETURN_FALSE;
    }

    /* handerlsocket: object */
    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
    HS_ERROR_RESET(hsi->error);

    /* options */
    if (options != NULL && Z_TYPE_P(options) == IS_ARRAY) {
        /* limit */
        limit = hs_get_options_long(HASH_OF(options), "limit", limit);
        /* offset */
        offset = hs_get_options_long(HASH_OF(options), "offset", offset);
        /* safe */
        safe = hs_is_options_safe(HASH_OF(options) TSRMLS_CC);
        /* in, fiter, while */
        hs_array_to_in_filter(HASH_OF(options), hsi->filter,
                              &filters, &in_key, &in_values TSRMLS_CC);
    }

    /* stream */
    stream = handlersocketi_object_store_get_stream(hsi->link);
    timeout = handlersocketi_object_store_get_timeout(hsi->link);

    /* operate : criteria */
    MAKE_STD_ZVAL(operate);
    if (hs_zval_to_operate_criteria(query, operate, &criteria,
                                    HS_FIND_EQUAL TSRMLS_CC) != SUCCESS) {
        zval_ptr_dtor(&operate);
        RETURN_FALSE;
    }

    /* command */
    hs_request_command(&request, hsi->id, operate, criteria,
                       limit, offset, filters, in_key, in_values TSRMLS_CC);

    /* eol */
    hs_request_next(&request);

    /* request: send */
    if (hs_request_send(stream, &request TSRMLS_CC) < 0) {
        ZVAL_BOOL(return_value, 0);
    } else {
        /* response */
        hs_response_value(stream, timeout, return_value,
                          hsi->error, 0 TSRMLS_CC);
    }

    zval_ptr_dtor(&operate);
    if (filters) {
        zval_ptr_dtor(&filters);
    }
    smart_str_free(&request);

    /* exception */
    if (safe > 0 &&
        Z_TYPE_P(return_value) == IS_BOOL && Z_LVAL_P(return_value) == 0) {
        HS_EXCEPTION("find(): response error: %s",
                     hsi->error == NULL ?
                     "Unknown error" : Z_STRVAL_P(hsi->error));
        RETURN_FALSE;
    }
}

ZEND_METHOD(HandlerSocketi_Index, insert)
{
    zval ***args;
    zval *operate, *fields;
    long i, argc = ZEND_NUM_ARGS();
    hs_index_obj_t *hsi;
    php_stream *stream;
    long timeout;
    smart_str request = {0};
    long fnum = 0;

    if (argc < 1) {
        zend_wrong_param_count(TSRMLS_C);
        RETURN_FALSE;
    }

    args = safe_emalloc(argc, sizeof(zval **), 0);
    if (zend_get_parameters_array_ex(argc, args) == FAILURE) {
        efree(args);
        zend_wrong_param_count(TSRMLS_C);
        RETURN_FALSE;
    }

    if (Z_TYPE_PP(args[0]) == IS_ARRAY) {
        fields = *args[0];
    } else {
        MAKE_STD_ZVAL(fields);
        array_init(fields);

        for (i = 0; i < argc; i++) {
            switch (Z_TYPE_P(*args[i])) {
                case IS_NULL:
                    add_next_index_null(fields);
                    break;
                case IS_LONG:
                    add_next_index_long(fields, Z_LVAL_P(*args[i]));
                    break;
                case IS_DOUBLE:
                    add_next_index_double(fields, Z_DVAL_P(*args[i]));
                    break;
                default:
                    convert_to_string(*args[i]);
                    add_next_index_stringl(fields, Z_STRVAL_P(*args[i]),
                                           Z_STRLEN_P(*args[i]), 1);
                    break;
            }
        }
    }

    /* handerlsocket: object */
    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
    HS_ERROR_RESET(hsi->error);

    /* fiedls check */
    fnum = zend_hash_num_elements(HASH_OF(fields));
    if (fnum < hsi->field_num) {
        for (i = 0; i < fnum; i++) {
            add_next_index_null(fields);
        }
    } else if (fnum > hsi->field_num) {
        /*
        efree(args);
        ZVAL_STRINGL(hsi->error, "invalid field count",
                     strlen("invalid field count"), 1);
        RETURN_FALSE;
        */
        zend_error(E_WARNING,
                   "HandlerSocketi_Index::insert(): invalid field count");
    }

    stream = handlersocketi_object_store_get_stream(hsi->link);
    timeout = handlersocketi_object_store_get_timeout(hsi->link);

    /* operate */
    MAKE_STD_ZVAL(operate);
    ZVAL_STRINGL(operate, HS_PROTOCOL_INSERT, 1, 1);

    /* command */
    hs_request_command(&request, hsi->id, operate, fields,
                       0, 0, NULL, -1, NULL TSRMLS_CC);

    /* eol */
    hs_request_next(&request);

    /* request: send */
    if (hs_request_send(stream, &request TSRMLS_CC) < 0) {
        ZVAL_BOOL(return_value, 0);
    } else {
        /* response */
        hs_response_value(stream, timeout, return_value,
                          hsi->error, 1 TSRMLS_CC);
    }

   zval_ptr_dtor(&operate);
   //zval_ptr_dtor(&fields);
   smart_str_free(&request);
   efree(args);
}

ZEND_METHOD(HandlerSocketi_Index, update)
{
    zval *query, *update, *options = NULL;
    long safe = -1, modify = 1;
    long limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;
    zval *operate, *criteria, *modify_operate, *modify_criteria;
    zval *filters = NULL, *in_values = NULL;
    long in_key = -1;
    hs_index_obj_t *hsi;
    php_stream *stream;
    long timeout;
    smart_str request = {0};
    long fnum = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z",
                              &query, &update, &options) == FAILURE) {
        RETURN_FALSE;
    }

    /* handerlsocket: object */
    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
    HS_ERROR_RESET(hsi->error);

    if (options != NULL && Z_TYPE_P(options) == IS_ARRAY) {
        /* limit */
        limit = hs_get_options_long(HASH_OF(options), "limit", limit);
        /* offset */
        offset = hs_get_options_long(HASH_OF(options), "offset", offset);
        /* safe */
        safe = hs_is_options_safe(HASH_OF(options) TSRMLS_CC);
        /* in, fiter, while */
        hs_array_to_in_filter(HASH_OF(options), hsi->filter,
                              &filters, &in_key, &in_values TSRMLS_CC);
    }

    /* stream */
    stream = handlersocketi_object_store_get_stream(hsi->link);
    timeout = handlersocketi_object_store_get_timeout(hsi->link);

    /* operate : criteria */
    MAKE_STD_ZVAL(operate);
    if (hs_zval_to_operate_criteria(query, operate, &criteria,
                                    HS_FIND_EQUAL TSRMLS_CC) != SUCCESS) {
        zval_ptr_dtor(&operate);
        RETURN_FALSE;
    }

    /* modify_operete : modify_criteria */
    MAKE_STD_ZVAL(modify_operate);
    if (hs_zval_to_operate_criteria(update, modify_operate, &modify_criteria,
                                    HS_MODIFY_UPDATE TSRMLS_CC)  != SUCCESS) {
        zval_ptr_dtor(&operate);
        zval_ptr_dtor(&modify_operate);
        RETURN_FALSE;
    }

    /* fiedls check */
    if (Z_TYPE_P(modify_criteria) == IS_ARRAY) {
        fnum = zend_hash_num_elements(HASH_OF(modify_criteria));
    } else {
        fnum = 1;
    }

    if (fnum > hsi->field_num) {
        /*
        ZVAL_STRINGL(hsi->error, "invalid field count",
                     strlen("invalid field count"), 1);
        RETURN_FALSE;
        */
        zend_error(E_WARNING,
                   "HandlerSocketi_Index::update(): invalid field count");
    }

    /* command */
    hs_request_command(&request, hsi->id, operate, criteria,
                       limit, offset, filters, in_key, in_values TSRMLS_CC);

    /* command: modify */
    modify = hs_request_command_modify(&request, modify_operate,
                                       modify_criteria, -1 TSRMLS_CC);
    if (modify >= 0) {
        /* eol */
        hs_request_next(&request);

        /* request: send */
        if (hs_request_send(stream, &request TSRMLS_CC) < 0) {
            ZVAL_BOOL(return_value, 0);
        } else {
            /* response */
            hs_response_value(stream, timeout, return_value,
                              hsi->error, modify TSRMLS_CC);
        }
    } else {
        ZVAL_BOOL(return_value, 0);
        ZVAL_STRINGL(hsi->error, "unable to update parameter",
                     strlen("unable to update parameter"), 1);
    }

    zval_ptr_dtor(&operate);
    zval_ptr_dtor(&modify_operate);
    if (filters) {
        zval_ptr_dtor(&filters);
    }
    smart_str_free(&request);

    /* exception */
    if (safe > 0 &&
        Z_TYPE_P(return_value) == IS_BOOL && Z_LVAL_P(return_value) == 0) {
        HS_EXCEPTION("update(): response error: %s",
                     hsi->error == NULL ?
                     "Unknown error" : Z_STRVAL_P(hsi->error));
    }
}

ZEND_METHOD(HandlerSocketi_Index, remove)
{
    zval *query, *options = NULL;
    zval *operate, *criteria;
    long safe = -1, limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;
    zval *filters = NULL, *in_values = NULL;
    long in_key = -1;
    hs_index_obj_t *hsi;
    php_stream *stream;
    long timeout;
    smart_str request = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z",
                              &query, &options) == FAILURE) {
        RETURN_FALSE;
    }

    /* handelrsocket : object */
    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
    HS_ERROR_RESET(hsi->error);

    if (options != NULL && Z_TYPE_P(options) == IS_ARRAY) {
        /* limit */
        limit = hs_get_options_long(HASH_OF(options), "limit", limit);
        /* offset */
        offset = hs_get_options_long(HASH_OF(options), "offset", offset);
        /* safe */
        safe = hs_is_options_safe(HASH_OF(options) TSRMLS_CC);
        /* in, fiter, while */
        hs_array_to_in_filter(HASH_OF(options), hsi->filter,
                              &filters, &in_key, &in_values TSRMLS_CC);
    }

    /* stream */
    stream = handlersocketi_object_store_get_stream(hsi->link);
    timeout = handlersocketi_object_store_get_timeout(hsi->link);

    /* operate : criteria */
    MAKE_STD_ZVAL(operate);
    if (hs_zval_to_operate_criteria(query, operate, &criteria,
                                    HS_FIND_EQUAL TSRMLS_CC) != SUCCESS) {
        zval_ptr_dtor(&operate);
        RETURN_FALSE;
    }

    /* command */
    hs_request_command(&request, hsi->id, operate, criteria,
                       limit, offset, filters, in_key, in_values TSRMLS_CC);

    /* find: modify: D */
    hs_request_delim(&request);
    hs_request_string(&request, HS_MODIFY_REMOVE, 1);

    /* eol */
    hs_request_next(&request);

    /* request: send */
    if (hs_request_send(stream, &request TSRMLS_CC) < 0) {
        ZVAL_BOOL(return_value, 0);
    } else {
        /* response */
        hs_response_value(stream, timeout, return_value,
                          hsi->error, 1 TSRMLS_CC);
    }

    zval_ptr_dtor(&operate);
    if (filters) {
        zval_ptr_dtor(&filters);
    }
    smart_str_free(&request);

    /* exception */
    if (safe > 0 &&
        Z_TYPE_P(return_value) == IS_BOOL && Z_LVAL_P(return_value) == 0) {
        HS_EXCEPTION("remove(): response error: %s",
                     hsi->error == NULL ?
                     "Unknown error" : Z_STRVAL_P(hsi->error));
    }
}

ZEND_METHOD(HandlerSocketi_Index, multi)
{
    zval *args = NULL;

    zval *mreq;
    HashPosition pos;
    zval **val;
    int err = -1;

    hs_index_obj_t *hsi;
    php_stream *stream;
    long timeout;
    smart_str request = {0};

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a",
                              &args) == FAILURE) {
        RETURN_FALSE;
    }

    /* handlersocket: object */
    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);
    HS_ERROR_RESET(hsi->error);

    MAKE_STD_ZVAL(mreq);
    array_init(mreq);

    zend_hash_internal_pointer_reset_ex(HASH_OF(args), &pos);
    while (zend_hash_get_current_data_ex(HASH_OF(args),
                                         (void **)&val, &pos) == SUCCESS) {
        zval **method;
        HashTable *ht;

        if (Z_TYPE_PP(val) != IS_ARRAY) {
            err = -1;
            break;
        }

        ht = HASH_OF(*val);

        /* 0: method */
        if (zend_hash_index_find(ht, 0, (void **)&method) != SUCCESS) {
            err = -1;
            break;
        }

        convert_to_string(*method);

        if (strncmp(Z_STRVAL_PP(method), "find", strlen("find")) == 0) {
            /* method: find */
            zval **query, **options, *operate, *criteria;
            zval *filters = NULL, *in_values = NULL;
            long in_key = -1;
            long limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;

            if (zend_hash_num_elements(ht) <= 1) {
                err = -1;
                break;
            }

            /* 1: query */
            if (zend_hash_index_find(ht, 1, (void **)&query) != SUCCESS) {
                err = -1;
                break;
            }

            /* 2: options */
            if (zend_hash_index_find(ht, 2, (void **)&options) == SUCCESS &&
                options != NULL && Z_TYPE_PP(options) == IS_ARRAY) {
                limit = hs_get_options_long(HASH_OF(*options),
                                            "limit", limit);
                offset = hs_get_options_long(HASH_OF(*options),
                                             "offset", offset);
                hs_array_to_in_filter(HASH_OF(*options), hsi->filter,
                                      &filters, &in_key, &in_values TSRMLS_CC);
            }

            /* operete : criteria */
            MAKE_STD_ZVAL(operate);
            if (hs_zval_to_operate_criteria(*query, operate, &criteria,
                                            HS_FIND_EQUAL TSRMLS_CC)
                != SUCCESS) {
                err = -1;
                zval_ptr_dtor(&operate);
                break;
            }

            if (*query == NULL) {
                err = -1;
                zval_ptr_dtor(&operate);
                break;
            }

            /* command */
            hs_request_command(&request, hsi->id, operate, criteria,
                               limit, offset, filters, in_key,
                               in_values TSRMLS_CC);

            /* eol */
            hs_request_next(&request);

            add_next_index_long(mreq, 0);
            err = 0;

            zval_ptr_dtor(&operate);
            if (filters) {
                zval_ptr_dtor(&filters);
            }
        } else if (strncmp(Z_STRVAL_PP(method),
                           "insert", strlen("insert")) == 0) {
            /* method: insert */
            zval *operate, *fields;
            zval **tmp;
            long i, n, fnum = 0;

            if (zend_hash_num_elements(ht) <= 1) {
                err = -1;
                break;
            }

            if (zend_hash_index_find(ht, 1, (void **)&tmp) != SUCCESS) {
                err = -1;
                break;
            }

            MAKE_STD_ZVAL(fields);
            array_init(fields);

            if (Z_TYPE_PP(tmp) == IS_ARRAY) {
                n = zend_hash_num_elements(HASH_OF(*tmp));
                for (i = 0; i < n; i++) {
                    zval **val;
                    if (zend_hash_index_find(HASH_OF(*tmp),
                                             i, (void **)&val) == SUCCESS) {
                        if (Z_TYPE_PP(val) == IS_NULL) {
                            add_next_index_null(fields);
                        } else {
                            convert_to_string(*val);
                            add_next_index_stringl(fields, Z_STRVAL_PP(val),
                                                   Z_STRLEN_PP(val), 1);
                        }
                    }
                }
            } else {
                i = 1;
                do {
                    if (Z_TYPE_PP(tmp) == IS_NULL) {
                        add_next_index_null(fields);
                    } else {
                        convert_to_string(*tmp);
                        add_next_index_stringl(fields, Z_STRVAL_PP(tmp),
                                               Z_STRLEN_PP(tmp), 1);
                    }

                    i++;

                    if (zend_hash_index_find(ht, i, (void **)&tmp) != SUCCESS) {
                        break;
                    }
                } while (i < n);
            }

            /* fields check */
            fnum = zend_hash_num_elements(HASH_OF(fields));
            if (fnum < hsi->field_num) {
                for (i = 0; i < fnum; i++) {
                    add_next_index_null(fields);
                }
            }

            MAKE_STD_ZVAL(operate);
            ZVAL_STRINGL(operate, HS_PROTOCOL_INSERT, 1, 1);

            /* command */
            hs_request_command(&request, hsi->id, operate, fields, 0, 0,
                               NULL, -1, NULL TSRMLS_CC);

            /* eol */
            hs_request_next(&request);

            add_next_index_long(mreq, 1);
            err = 0;

            zval_ptr_dtor(&operate);
            zval_ptr_dtor(&fields);
        } else if (strncmp(Z_STRVAL_PP(method),
                           "remove", strlen("remove")) == 0) {
            /* method: remove */
            zval **query, **options, *operate, *criteria;
            zval *filters = NULL, *in_values = NULL;
            long in_key = -1;
            long limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;

            if (zend_hash_num_elements(ht) <= 1) {
                err = -1;
                break;
            }

            /* 1: query */
            if (zend_hash_index_find(ht, 1, (void **)&query) != SUCCESS) {
                if (operate) {
                    zval_ptr_dtor(&operate);
                }
                err = -1;
                break;
            }

            /* 2: options */
            if (zend_hash_index_find(ht, 2, (void **)&options) == SUCCESS &&
                options != NULL && Z_TYPE_PP(options) == IS_ARRAY) {
                limit = hs_get_options_long(HASH_OF(*options),
                                            "limit", limit);
                offset = hs_get_options_long(HASH_OF(*options),
                                             "offset", offset);
                hs_array_to_in_filter(HASH_OF(*options), hsi->filter,
                                      &filters, &in_key, &in_values TSRMLS_CC);
            }

            /* operete : criteria */
            MAKE_STD_ZVAL(operate);
            if (hs_zval_to_operate_criteria(*query, operate, &criteria,
                                            HS_FIND_EQUAL TSRMLS_CC)
                != SUCCESS) {
                zval_ptr_dtor(&operate);
                err = -1;
                break;
            }

            /* command */
            hs_request_command(&request, hsi->id, operate, criteria,
                               limit, offset, filters, in_key,
                               in_values TSRMLS_CC);

            /* find: modify: D */
            hs_request_delim(&request);

            hs_request_string(&request, HS_MODIFY_REMOVE, 1);

            /* eol */
            hs_request_next(&request);

            add_next_index_long(mreq, 1);
            err = 0;

            zval_ptr_dtor(&operate);
            if (filters) {
                zval_ptr_dtor(&filters);
            }
        } else if (strncmp(Z_STRVAL_PP(method),
                           "update", strlen("update")) == 0) {
            /* method: update */
            zval **query, **update, **options;
            zval *operate, *criteria, *modify_operate, *modify_criteria;
            zval *filters = NULL, *in_values = NULL;
            long in_key = -1;
            long limit = HS_DEFAULT_LIMIT, offset = HS_DEFAULT_OFFSET;
            int modify;

            if (zend_hash_num_elements(ht) <= 2) {
                err = -1;
                break;
            }

            /* 1: query */
            if (zend_hash_index_find(ht, 1, (void **)&query) != SUCCESS) {
                err = -1;
                break;
            }

            /* 2: update */
            if (zend_hash_index_find(ht, 2, (void **)&update) != SUCCESS) {
                err = -1;
                break;
            }

            /* 3: options */
            if (zend_hash_index_find(ht, 3, (void **)&options) == SUCCESS &&
                options != NULL && Z_TYPE_PP(options) == IS_ARRAY) {
                limit = hs_get_options_long(HASH_OF(*options),
                                            "limit", limit);
                offset = hs_get_options_long(HASH_OF(*options),
                                             "offset", offset);
                hs_array_to_in_filter(HASH_OF(*options), hsi->filter,
                                      &filters, &in_key, &in_values TSRMLS_CC);
            }

            /* operete : criteria */
            MAKE_STD_ZVAL(operate);
            if (hs_zval_to_operate_criteria(*query, operate, &criteria,
                                            HS_FIND_EQUAL TSRMLS_CC)
                != SUCCESS) {
                zval_ptr_dtor(&operate);
                err = -1;
                break;
            }

            /* modify_operete : modify_criteria */
            MAKE_STD_ZVAL(modify_operate);
            if (hs_zval_to_operate_criteria(*update, modify_operate,
                                            &modify_criteria,
                                            HS_MODIFY_UPDATE TSRMLS_CC)
                != SUCCESS) {
                zval_ptr_dtor(&operate);
                zval_ptr_dtor(&modify_operate);
                err = -1;
                break;
            }

            /* command */
            hs_request_command(&request, hsi->id, operate, criteria,
                               limit, offset, filters, in_key,
                               in_values TSRMLS_CC);

            /* command: modify */
            modify = hs_request_command_modify(&request, modify_operate,
                                               modify_criteria, -1 TSRMLS_CC);
            if (modify < 0) {
                err = -1;
                break;
            }

            /* eol */
            hs_request_next(&request);

            add_next_index_long(mreq, modify);
            err = 0;

            zval_ptr_dtor(&operate);
            zval_ptr_dtor(&modify_operate);
            if (filters) {
                zval_ptr_dtor(&filters);
            }
        } else {
            err = -1;
            break;
        }

        zend_hash_move_forward_ex(HASH_OF(args), &pos);
    }

    /* stream */
    stream = handlersocketi_object_store_get_stream(hsi->link);
    timeout = handlersocketi_object_store_get_timeout(hsi->link);

    /* request: send */
    if (err < 0  || hs_request_send(stream, &request TSRMLS_CC) < 0) {
        smart_str_free(&request);
        zval_ptr_dtor(&mreq);
        RETURN_FALSE;
    }
    smart_str_free(&request);

    /* response */
    hs_response_multi(stream, timeout, return_value, hsi->error, mreq TSRMLS_CC);

    zval_ptr_dtor(&mreq);
}

ZEND_METHOD(HandlerSocketi_Index, get_error)
{
    hs_index_obj_t *hsi;

    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

    if (hsi->error == NULL) {
        RETURN_NULL();
    } else {
        RETVAL_ZVAL(hsi->error, 1, 0);
    }
}

ZEND_METHOD(HandlerSocketi_Index, get_id)
{
    hs_index_obj_t *hsi;

    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

    RETVAL_LONG(hsi->id);
}

ZEND_METHOD(HandlerSocketi_Index, get_name)
{
    hs_index_obj_t *hsi;

    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

    RETVAL_ZVAL(hsi->name, 1, 0);
}

ZEND_METHOD(HandlerSocketi_Index, get_db)
{
    hs_index_obj_t *hsi;

    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

    RETVAL_ZVAL(hsi->db, 1, 0);
}

ZEND_METHOD(HandlerSocketi_Index, get_table)
{
    hs_index_obj_t *hsi;

    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

    RETVAL_ZVAL(hsi->table, 1, 0);
}

ZEND_METHOD(HandlerSocketi_Index, get_field)
{
    hs_index_obj_t *hsi;

    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

    RETVAL_ZVAL(hsi->field, 1, 0);
}

ZEND_METHOD(HandlerSocketi_Index, get_filter)
{
    hs_index_obj_t *hsi;

    hsi = (hs_index_obj_t *)zend_object_store_get_object(getThis() TSRMLS_CC);
    HS_CHECK_OBJECT(hsi, HandlerSocketi_Index);

    if (hsi->filter) {
        RETVAL_ZVAL(hsi->filter, 1, 0);
    } else {
        RETVAL_NULL();
    }
}

ZEND_METHOD(HandlerSocketi_Index, get_operator)
{
    zval *query, *modify;

    MAKE_STD_ZVAL(query);
    array_init(query);

    add_next_index_stringl(query, HS_FIND_EQUAL, strlen(HS_FIND_EQUAL), 1);
    add_next_index_stringl(query, HS_FIND_LESS, strlen(HS_FIND_LESS), 1);
    add_next_index_stringl(query, HS_FIND_LESS_EQUAL,
                           strlen(HS_FIND_LESS_EQUAL), 1);
    add_next_index_stringl(query, HS_FIND_GREATER, strlen(HS_FIND_GREATER), 1);
    add_next_index_stringl(query, HS_FIND_GREATER_EQUAL,
                           strlen(HS_FIND_GREATER_EQUAL), 1);

    MAKE_STD_ZVAL(modify);
    array_init(modify);

    add_next_index_stringl(modify, HS_MODIFY_UPDATE,
                           strlen(HS_MODIFY_UPDATE), 1);
    add_next_index_stringl(modify, HS_MODIFY_INCREMENT,
                           strlen(HS_MODIFY_INCREMENT), 1);
    add_next_index_stringl(modify, HS_MODIFY_DECREMENT,
                           strlen(HS_MODIFY_DECREMENT), 1);
    add_next_index_stringl(modify, HS_MODIFY_REMOVE,
                           strlen(HS_MODIFY_REMOVE), 1);
    add_next_index_stringl(modify, HS_MODIFY_GET_UPDATE,
                           strlen(HS_MODIFY_GET_UPDATE), 1);
    add_next_index_stringl(modify, HS_MODIFY_GET_INCREMENT,
                           strlen(HS_MODIFY_GET_INCREMENT), 1);
    add_next_index_stringl(modify, HS_MODIFY_GET_DECREMENT,
                           strlen(HS_MODIFY_GET_DECREMENT), 1);
    add_next_index_stringl(modify, HS_MODIFY_GET_REMOVE,
                           strlen(HS_MODIFY_GET_REMOVE), 1);

    array_init(return_value);
    add_assoc_zval(return_value, "query", query);
    add_assoc_zval(return_value, "modify", modify);
}
