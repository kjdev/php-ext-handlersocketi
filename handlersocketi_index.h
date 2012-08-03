#ifndef HANDLERSOCKETI_INDEX_H
#define HANDLERSOCKETI_INDEX_H

PHP_HANDLERSOCKETI_API int handlersocketi_register_index(TSRMLS_D);
PHP_HANDLERSOCKETI_API zend_class_entry *handlersocketi_get_ce_index(void);
PHP_HANDLERSOCKETI_API void handlersocketi_create_index(
    zval *return_value, zval *link, char *db, long db_len,
    char *table, long table_len, zval *fields, zval *options TSRMLS_DC);

#endif /* HANDLERSOCKETI_INDEX_H */
