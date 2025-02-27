/* libnsq extension for PHP */

#ifndef PHP_LIBNSQ_H
# define PHP_LIBNSQ_H

extern zend_module_entry libnsq_module_entry;
# define phpext_libnsq_ptr &libnsq_module_entry

# define PHP_LIBNSQ_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_LIBNSQ)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_LIBNSQ_H */
