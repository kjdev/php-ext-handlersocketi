#ifndef HANDLERSOCKET_REQUEST_H
#define HANDLERSOCKET_REQUEST_H

#include "ext/standard/php_smart_str.h"
#include "php_streams.h"

#define hs_request_long(buf, num) smart_str_append_long(buf, num)
#define hs_request_null(buf) smart_str_appendc(buf, HS_CODE_NULL)
#define hs_request_delim(buf) smart_str_appendc(buf, HS_CODE_DELIMITER)
#define hs_request_next(buf) smart_str_appendc(buf, HS_CODE_EOL)

void hs_request_string(smart_str *buf, char *str, long str_len);

void hs_request_array(smart_str *buf, HashTable *ht, int num, int i TSRMLS_DC);
void hs_request_filter(smart_str *request, HashTable *ht TSRMLS_DC);

void hs_request_command(smart_str *buf, long id, zval * operate, zval *criteria,
                        long limit, long offset, zval *filters, long in_key,
                        zval *in_values TSRMLS_DC);
int hs_request_command_modify(smart_str *buf, zval *update,
                              zval *values, long field TSRMLS_DC);

long hs_request_send(php_stream *stream, smart_str *request TSRMLS_DC);

#endif /* HANDLERSOCKET_REQUEST_H */
