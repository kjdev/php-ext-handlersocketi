#ifndef HANDLERSOCKETI_CLASS_H
#define HANDLERSOCKETI_CLASS_H

PHP_HANDLERSOCKETI_API int handlersocketi_register_class(TSRMLS_D);
PHP_HANDLERSOCKETI_API zend_class_entry *handlersocketi_get_ce(void);

PHP_HANDLERSOCKETI_API php_stream *handlersocketi_object_store_get_stream(zval *link);
PHP_HANDLERSOCKETI_API long handlersocketi_object_store_get_timeout(zval *link);

#endif /* HANDLERSOCKETI_CLASS_H */
