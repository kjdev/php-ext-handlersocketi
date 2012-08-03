
#ifndef PHP_HANDLERSOCKETI_H
#define PHP_HANDLERSOCKETI_H

#define HANDLERSOCKETI_EXT_VERSION "0.0.1"

extern zend_module_entry handlersocketi_module_entry;
#define phpext_handlersocketi_ptr &handlersocketi_module_entry

#ifdef PHP_WIN32
#   define PHP_HANDLERSOCKETI_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#   define PHP_HANDLERSOCKETI_API __attribute__ ((visibility("default")))
#else
#   define PHP_HANDLERSOCKETI_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(handlersocketi)
    long id;
ZEND_END_MODULE_GLOBALS(handlersocketi)

#ifdef ZTS
#define HANDLERSOCKETI_G(v) TSRMG(handlersocketi_globals_id, zend_handlersocketi_globals *, v)
#else
#define HANDLERSOCKETI_G(v) (handlersocketi_globals.v)
#endif

#endif  /* PHP_HANDLERSOCKETI_H */
