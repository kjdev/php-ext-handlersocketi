
#include "php.h"
#include "php_network.h"

#include "common.h"
#include "request.h"

static inline void
hs_request_zval_scalar(smart_str *buf, zval *val, int delim)
{
    switch (Z_TYPE_P(val)) {
        case IS_LONG:
            hs_request_long(buf, Z_LVAL_P(val));
            break;
        case IS_STRING:
            hs_request_string(buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
            break;
        case IS_DOUBLE:
            convert_to_string(val);
            hs_request_string(buf, Z_STRVAL_P(val), Z_STRLEN_P(val));
            break;
        case IS_BOOL:
            convert_to_long(val);
            hs_request_long(buf, Z_LVAL_P(val));
            break;
        case IS_NULL:
            hs_request_null(buf);
            break;
        default:
            //IS_ARRAY
            //IS_OBJECT
            //IS_RESOURCE
            hs_request_long(buf, 0);
            break;
    }

    if (delim > 0) {
        hs_request_delim(buf);
    }
}

void
hs_request_string(smart_str *buf, char *str, long str_len)
{
    long i;

    if (str_len <= 0) {
        return;
    } else {
        for (i = 0; i < str_len; i++) {
            if ((unsigned char)str[i] > HS_CODE_ESCAPE) {
                smart_str_appendc(buf, str[i]);
            } else {
                smart_str_appendc(buf, HS_CODE_ESCAPE_PREFIX);
                smart_str_appendc(buf, (unsigned char)str[i]+HS_CODE_ESCAPE_ADD);
            }
        }
    }
}

void
hs_request_array(smart_str *buf, HashTable *ht, int num, int i TSRMLS_DC)
{
    long n;
    HashPosition pos;
    zval **data;

    n = zend_hash_num_elements(ht);
    if (i > 0 && i < n) {
        n = i;
    }

    if (n == 0) {
        if (num == 1) {
            hs_request_long(buf, 1);
            hs_request_delim(buf);
        }
        hs_request_null(buf);
        return;
    }

    if (num == 1) {
        hs_request_long(buf, n);
        hs_request_delim(buf);
    }

    zend_hash_internal_pointer_reset_ex(ht, &pos);
    while (zend_hash_get_current_data_ex(ht, (void **)&data, &pos) == SUCCESS) {
        if (n <= 0) {
            break;
        }
        n--;
        hs_request_zval_scalar(buf, *data, n);
        zend_hash_move_forward_ex(ht, &pos);
    }
}

void
hs_request_filter(smart_str *request, HashTable *ht TSRMLS_DC)
{
    zval **tmp;
    HashPosition pos;
    long n, i = 0;

    n = zend_hash_num_elements(ht);
    if (n >= 0) {
        hs_request_delim(request);

        zend_hash_internal_pointer_reset_ex(ht, &pos);
        while (zend_hash_get_current_data_ex(ht,
                                             (void **)&tmp, &pos) == SUCCESS) {
            switch ((*tmp)->type) {
                case IS_STRING:
                    hs_request_string(request, Z_STRVAL_PP(tmp),
                                      Z_STRLEN_PP(tmp));
                    break;
                case IS_LONG:
                    hs_request_long(request, Z_LVAL_PP(tmp));
                    break;
                default:
                    convert_to_string(*tmp);
                    hs_request_string(request, Z_STRVAL_PP(tmp),
                                      Z_STRLEN_PP(tmp));
                    break;
            }

            if (++i != n) {
                hs_request_string(request, ",", strlen(","));
            }

            zend_hash_move_forward_ex(ht, &pos);
        }
    }
}

void
hs_request_command(smart_str *buf, long id, zval * operate, zval *criteria,
                   long limit, long offset, zval *filters, long in_key,
                   zval *in_values TSRMLS_DC)
{
    hs_request_long(buf, id);
    hs_request_delim(buf);

    convert_to_string(operate);
    hs_request_string(buf, Z_STRVAL_P(operate), Z_STRLEN_P(operate));
    hs_request_delim(buf);

    if (Z_TYPE_P(criteria) == IS_ARRAY) {
        hs_request_array(buf, HASH_OF(criteria), 1, -1 TSRMLS_CC);
    } else {
        hs_request_long(buf, 1);
        hs_request_delim(buf);
        hs_request_zval_scalar(buf, criteria, 0);
    }

    hs_request_delim(buf);
    hs_request_long(buf, limit);

    hs_request_delim(buf);
    hs_request_long(buf, offset);

    if (in_key >= 0 && in_values != NULL) {
        hs_request_delim(buf);

        hs_request_string(buf, HS_PROTOCOL_IN, 1);
        hs_request_delim(buf);

        hs_request_long(buf, in_key);
        hs_request_delim(buf);

        if (Z_TYPE_P(in_values) == IS_ARRAY) {
            hs_request_array(buf, HASH_OF(in_values), 1, -1 TSRMLS_CC);
        } else {
            hs_request_zval_scalar(buf, in_values, 0);
        }
    }

    if (filters != NULL && Z_TYPE_P(filters) == IS_ARRAY) {
        HashTable *ht;
        HashPosition pos;
        zval **tmp;

        ht = HASH_OF(filters);

        zend_hash_internal_pointer_reset_ex(ht, &pos);
        while (zend_hash_get_current_data_ex(ht, (void **)&tmp,
                                             &pos) == SUCCESS) {
            if (Z_TYPE_PP(tmp) != IS_ARRAY ||
                zend_hash_num_elements(HASH_OF(*tmp)) < 4) {
                zend_hash_move_forward_ex(ht, &pos);
                continue;
            }

            hs_request_delim(buf);

            hs_request_array(buf, HASH_OF(*tmp), -1, 4 TSRMLS_CC);

            zend_hash_move_forward_ex(ht, &pos);
        }
    }
}

int
hs_request_command_modify(smart_str *buf, zval *update,
                          zval *values, long field TSRMLS_DC)
{
    int ret = -1;
    long len;

    if (update == NULL || Z_TYPE_P(update) != IS_STRING) {
        return -1;
    }

    len = Z_STRLEN_P(update);
    if (len == 1) {
        ret = 1;
    } else if (len == 2) {
        ret = 0;
    } else {
        return -1;
    }

    if (values == NULL) {
        hs_request_delim(buf);
        hs_request_string(buf, Z_STRVAL_P(update), Z_STRLEN_P(update));

        hs_request_delim(buf);
        hs_request_null(buf);
    } else if (Z_TYPE_P(values) == IS_ARRAY) {
        if (field > 0 &&
            zend_hash_num_elements(HASH_OF(values)) < field) {
            return -1;
        }

        hs_request_delim(buf);
        hs_request_string(buf, Z_STRVAL_P(update), Z_STRLEN_P(update));

        hs_request_delim(buf);
        hs_request_array(buf, HASH_OF(values), 0, -1 TSRMLS_CC);
    } else {
        if (field > 0 && field != 1) {
            return -1;
        }

        hs_request_delim(buf);
        hs_request_string(buf, Z_STRVAL_P(update), Z_STRLEN_P(update));

        hs_request_delim(buf);
        hs_request_zval_scalar(buf, values, 0);
    }

    return ret;
}

long
hs_request_send(php_stream *stream, smart_str *request TSRMLS_DC)
{
#ifdef HS_DEBUG
    long i;
    smart_str debug = {0};
    if (request->len <= 0) {
        return -1;
    }
    for (i = 0; i < request->len; i++) {
        if ((unsigned char)request->c[i] == HS_CODE_NULL) {
            smart_str_appendl_ex(&debug, "\\0", strlen("\\0"), 1);
        } else {
            smart_str_appendc(&debug, request->c[i]);
        }
    }
    smart_str_0(&debug);
    php_printf("[handlersocket] (request) %ld : \"%s\"\n",
               request->len, debug.c);
    smart_str_free(&debug);
#endif

    if (!stream) {
        return -1;
    }

    return php_stream_write(stream, request->c, request->len);
}
