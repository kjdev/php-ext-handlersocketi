
#include "php.h"
#include "php_network.h"
#include "ext/standard/php_smart_str.h"

#include "common.h"
#include "response.h"

#define HS_SOCKET_BLOCK_SIZE 4096

#ifndef PHP_WIN32
#define php_select(m, r, w, e, t) select(m, r, w, e, t)
#else
#include "win32/select.h"
#endif

static inline long
hs_response_select(php_stream *stream, long timeout TSRMLS_DC)
{
    php_socket_t max_fd = 0;
    int retval, max_set_count = 0;
    struct timeval tv;
    struct timeval *tv_p = NULL;
    fd_set fds;

    FD_ZERO(&fds);

    if (php_stream_cast(stream,
                        PHP_STREAM_AS_FD_FOR_SELECT|PHP_STREAM_CAST_INTERNAL,
                        (void*)&max_fd, 1) == SUCCESS && max_fd != -1) {
        PHP_SAFE_FD_SET(max_fd, &fds);
        max_set_count++;
    }

    PHP_SAFE_MAX_FD(max_fd, max_set_count);

    if (timeout > 0) {
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        tv_p = &tv;
    }

    retval = php_select(max_fd+1, &fds, NULL, NULL, tv_p);
    if (retval == -1) {
        zend_error(E_WARNING, "[HandlerSocket] unable to select");
        return -1;
    }

    if (!PHP_SAFE_FD_ISSET(max_fd, &fds)) {
        return -1;
    }

    return 0;
}

static inline long
hs_response_recv(php_stream *stream, char *recv, size_t size TSRMLS_DC)
{
    long ret;
#ifdef HS_DEBUG
    long i;
    smart_str debug = {0};
#endif

    ret  = php_stream_read(stream, recv, size);
    if (ret <= 0) {
        return -1;
    }

#ifdef HS_DEBUG
    for (i = 0; i < ret; i++) {
        if ((unsigned char)recv[i] == HS_CODE_NULL) {
            smart_str_appendl_ex(&debug, "\\0", strlen("\\0"), 1);
        } else {
            smart_str_appendc(&debug, recv[i]);
        }
    }
    smart_str_0(&debug);
    php_printf("[handlersocket] (recv) %ld : \"%s\"", ret, debug.c);
    smart_str_free(&debug);
#endif

    return ret;
}

static inline zval
*hs_response_add(zval *return_value TSRMLS_DC)
{
    zval *value;
    MAKE_STD_ZVAL(value);
    array_init(value);
    add_next_index_zval(return_value, value);
    return value;
}

static inline zval
*hs_response_zval(smart_str *buf TSRMLS_DC)
{
    zval *val;
    MAKE_STD_ZVAL(val);
    ZVAL_STRINGL(val, buf->c, buf->len, 1);
    return val;
}

void
hs_response_value(php_stream *stream, long timeout, zval *return_value,
                  zval *error, int modify TSRMLS_DC)
{
    char *recv;
    long i, j, len;
    zval *val, *item;

    smart_str response = {0};
    long n = 0, block_size = HS_SOCKET_BLOCK_SIZE;
    int escape = 0, flag = 0, null = 0;
    long ret[2] = {-1, -1};

    if (hs_response_select(stream, timeout TSRMLS_CC) < 0) {
        ZVAL_BOOL(return_value, 0);
    }

    recv = emalloc(block_size+1);
    len = hs_response_recv(stream, recv, block_size TSRMLS_CC);
    if (len <= 0) {
        efree(recv);
        ZVAL_BOOL(return_value, 0);
        return;
    }

    do {
        for (i = 0; i < len; i++) {
            if (recv[i] == HS_CODE_DELIMITER || recv[i] == HS_CODE_EOL) {
                val = hs_response_zval(&response TSRMLS_CC);
                convert_to_long(val);
                ret[flag] = Z_LVAL_P(val);
                flag++;
                zval_ptr_dtor(&val);
                smart_str_free(&response);
            } else {
                smart_str_appendc(&response, recv[i]);
            }

            if (flag > 1) {
                break;
            }
        }

        if (flag > 1) {
            break;
        } else {
            i = 0;
            len = hs_response_recv(stream, recv, block_size TSRMLS_CC);
            if (len <= 0) {
                break;
            }
        }
    } while (1);

    if (ret[0] != 0) {
        if (recv[i] != HS_CODE_EOL) {
            smart_str err = {0};

            i++;

            if (i > len) {
                i = 0;
                len = -1;
            }

            do {
                for (j = i; j < len; j++) {
                    if (recv[j] == HS_CODE_EOL) {
                        break;
                    }

                    if (recv[j] == HS_CODE_ESCAPE_PREFIX) {
                        escape = 1;
                    } else if (escape) {
                        escape = 0;
                        smart_str_appendc(
                            &err, (unsigned char)recv[j]-HS_CODE_ESCAPE_ADD);
                    } else {
                        smart_str_appendc(&err, recv[j]);
                    }
                }

                if (recv[j] == HS_CODE_EOL) {
                    break;
                }

                i = 0;
            } while ((len = hs_response_recv(
                          stream, recv, block_size TSRMLS_CC)) > 0);

            if (error) {
                ZVAL_STRINGL(error, err.c, err.len, 1);
            }

            smart_str_free(&err);
        } else if (error) {
            ZVAL_NULL(error);
        }

        efree(recv);
        ZVAL_BOOL(return_value, 0);

        return;
    }

    if (ret[1] == 1 && recv[i] == HS_CODE_EOL) {
        efree(recv);
        ZVAL_BOOL(return_value, 1);
        return;
    }

    i++;

    if (i > len) {
        i = 0;
        len = -1;
    }

    if (modify) {
        if (i > 0 && recv[i-1] == HS_CODE_EOL) {
            efree(recv);
            ZVAL_LONG(return_value, 0);
            return;
        }

        do {
            for (j = i; j < len; j++) {
                if (recv[j] == HS_CODE_EOL) {
                    ZVAL_STRINGL(return_value, response.c, response.len, 1);
                    break;
                }

                if (recv[j] == HS_CODE_ESCAPE_PREFIX) {
                    escape = 1;
                } else if (escape) {
                    escape = 0;
                    smart_str_appendc(
                        &response, (unsigned char)recv[j]-HS_CODE_ESCAPE_ADD);
                } else {
                    smart_str_appendc(&response, recv[j]);
                }
            }

            if (recv[j] == HS_CODE_EOL) {
                break;
            }
            i = 0;
        } while ((len = hs_response_recv(
                      stream, recv, block_size TSRMLS_CC)) > 0);

        convert_to_long(return_value);
    } else {
        array_init(return_value);

        if (i > 0 && recv[i-1] == HS_CODE_EOL) {
            efree(recv);
            return;
        }

        item = hs_response_add(return_value TSRMLS_CC);

        do {
            for (j = i; j < len; j++) {
                if (recv[j] == HS_CODE_DELIMITER) {
                    if (response.len == 0 && null == 1) {
                        add_next_index_null(item);
                    } else {
                        add_next_index_stringl(item, response.c,
                                               response.len, 1);
                    }

                    n++;
                    null = 0;
                    if (n == ret[1]) {
                        item = hs_response_add(return_value TSRMLS_CC);
                        n = 0;
                    }

                    smart_str_free(&response);

                    continue;
                } else if (recv[j] == HS_CODE_EOL) {
                    if (response.len == 0 && null == 1) {
                        add_next_index_null(item);
                    } else {
                        add_next_index_stringl(item, response.c,
                                               response.len, 1);
                    }
                    null = 0;
                    break;
                }

                if (recv[j] == HS_CODE_ESCAPE_PREFIX) {
                    escape = 1;
                } else if (escape) {
                    escape = 0;
                    smart_str_appendc(
                        &response, (unsigned char)recv[j]-HS_CODE_ESCAPE_ADD);
                } else if (recv[j] == HS_CODE_NULL) {
                    null = 1;
                } else {
                    smart_str_appendc(&response, recv[j]);
                }
            }

            if (recv[j] == HS_CODE_EOL) {
                break;
            }
            i = 0;
        } while ((len = hs_response_recv(
                      stream, recv, block_size TSRMLS_CC)) > 0);
    }

    efree(recv);

    smart_str_free(&response);
}

void
hs_response_multi(php_stream *stream, long timeout, zval *return_value,
                  zval *error, zval *mreq TSRMLS_DC)
{
    char *recv;
    long i, len, count;
    long current = 0;
    smart_str response = {0};
    long block_size = HS_SOCKET_BLOCK_SIZE;

    if (hs_response_select(stream, timeout TSRMLS_CC) < 0) {
        ZVAL_BOOL(return_value, 0);
    }

    recv = emalloc(block_size+1);
    len = hs_response_recv(stream, recv, block_size TSRMLS_CC);
    if (len <= 0) {
        efree(recv);
        RETVAL_BOOL(0);
    }

    count = zend_hash_num_elements(HASH_OF(mreq));

    array_init(return_value);

    array_init(error);

    for(i = 0; i < count; i++) {
        long j, k;
        zval *rval, *item, *val, **tmp;

        int flag = 0, escape = 0, null = 0;
        long n = 0, modify = 0;
        long ret[2] = {-1, -1};

        if (zend_hash_index_find(HASH_OF(mreq), i, (void **)&tmp) == SUCCESS) {
            modify = Z_LVAL_PP(tmp);
        }

        smart_str_free(&response);

        do {
            for (j = current; j < len; j++) {
                if (recv[j] == HS_CODE_DELIMITER || recv[j] == HS_CODE_EOL) {
                    rval = hs_response_zval(&response TSRMLS_CC);
                    convert_to_long(rval);
                    ret[flag] = Z_LVAL_P(rval);
                    flag++;
                    zval_ptr_dtor(&rval);
                    smart_str_free(&response);
                } else {
                    smart_str_appendc(&response, recv[j]);
                }

                if (flag > 1) {
                    break;
                }
            }

            if (flag > 1) {
                break;
            } else {
                j = 0;
                current = 0;
                len = hs_response_recv(stream, recv, block_size TSRMLS_CC);
                if (len <= 0) {
                    break;
                }
            }
        } while (1);

        if (ret[0] != 0) {
            if (recv[j] != HS_CODE_EOL) {
                smart_str err = {0};

                j++;

                if (j > len) {
                    j = 0;
                    current = 0;
                    len = -1;
                }

                do {
                    for (k = j; k < len; k++) {
                        if (recv[k] == HS_CODE_EOL) {
                            break;
                        }

                        if (recv[k] == HS_CODE_ESCAPE_PREFIX) {
                            escape = 1;
                        } else if (escape) {
                            escape = 0;
                            smart_str_appendc(
                                &err,
                                (unsigned char)recv[k] - HS_CODE_ESCAPE_ADD);
                        } else {
                            smart_str_appendc(&err, recv[k]);
                        }
                    }

                    if (recv[k] == HS_CODE_EOL) {
                        current = k;
                        break;
                    }

                    j = 0;
                    current = 0;

                } while ((len = hs_response_recv(
                              stream, recv, block_size TSRMLS_CC)) > 0);

                add_next_index_stringl(error, err.c, err.len, 1);

                smart_str_free(&err);
            } else {
                add_next_index_null(error);
            }

            add_next_index_bool(return_value, 0);

            current++;

            continue;
        }

        add_next_index_null(error);

        if (ret[1] == 1 && recv[j] == HS_CODE_EOL) {
            add_next_index_bool(return_value, 1);

            current = j + 1;

            continue;
        }

        j++;

        if (j > len) {
            j = 0;
            current = 0;
            len = -1;
        }

        if (modify) {
            zval *num_z;

            if (j > 0 && recv[j-1] == HS_CODE_EOL) {
                current = j;

                add_next_index_long(return_value, 0);

                continue;
            }

            MAKE_STD_ZVAL(num_z);

            do {
                for (k = j; k < len; k++) {
                    if (recv[k] == HS_CODE_EOL) {
                        ZVAL_STRINGL(num_z, response.c, response.len, 1);
                        break;
                    }

                    if (recv[k] == HS_CODE_ESCAPE_PREFIX) {
                        escape = 1;
                    } else if (escape) {
                        escape = 0;
                        smart_str_appendc(
                            &response,
                            (unsigned char)recv[k] - HS_CODE_ESCAPE_ADD);
                    } else {
                        smart_str_appendc(&response, recv[k]);
                    }
                }

                if (recv[k] == HS_CODE_EOL) {
                    current = k;
                    break;
                }

                j = 0;
                current = 0;

            } while ((len = hs_response_recv(
                          stream, recv, block_size TSRMLS_CC)) > 0);

            convert_to_long(num_z);

            add_next_index_long(return_value, Z_LVAL_P(num_z));

            zval_ptr_dtor(&num_z);
        } else {
            item = hs_response_add(return_value TSRMLS_CC);

            if (j > 0 && recv[j-1] == HS_CODE_EOL) {
                current = j;
                continue;
            }

            val = hs_response_add(item TSRMLS_CC);

            do {
                for (k = j; k < len; k++) {
                    if (recv[k] == HS_CODE_DELIMITER) {
                        if (response.len == 0 && null == 1) {
                            add_next_index_null(val);
                        } else {
                            add_next_index_stringl(val, response.c,
                                                   response.len, 1);
                        }

                        null = 0;
                        n++;
                        if (n == ret[1]) {
                            val = hs_response_add(item TSRMLS_CC);
                            n = 0;
                        }

                        smart_str_free(&response);

                        continue;
                    } else if (recv[k] == HS_CODE_EOL) {
                        if (response.len == 0 && null == 1) {
                            add_next_index_null(val);
                        } else {
                            add_next_index_stringl(val, response.c,
                                                   response.len, 1);
                        }
                        null = 0;
                        break;
                    }

                    if (recv[k] == HS_CODE_ESCAPE_PREFIX) {
                        escape = 1;
                    } else if (escape) {
                        escape = 0;
                        smart_str_appendc(
                            &response,
                            (unsigned char)recv[k] - HS_CODE_ESCAPE_ADD);
                    } else if (recv[k] == HS_CODE_NULL) {
                        null = 1;
                    } else {
                        smart_str_appendc(&response, recv[k]);
                    }
                }

                if (recv[k] == HS_CODE_EOL) {
                    current = k;
                    break;
                }

                j = 0;
                current = 0;

            } while ((len = hs_response_recv(
                          stream, recv, block_size TSRMLS_CC)) > 0);
        }

        current++;
    }

    efree(recv);

    smart_str_free(&response);
}
