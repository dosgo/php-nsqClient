PHP_ARG_ENABLE(libNsq, whether to enable libNsq support,
[  --enable-libNsq        Enable libNsq support])

if test "$PHP_LIBNSQ" != "no"; then
    # 编译Go代码为静态库
    go build -buildmode=c-archive -o libnsq_subscriber.a ../go/libnsq_subscriber.go
    
    PHP_ADD_INCLUDE(.)
    PHP_ADD_LIBRARY(stdc++, 1, LIBNSQ_SHARED_LIBADD)
    PHP_ADD_LIBRARY_WITH_PATH(nsq_subscriber, ./, LIBNSQ_SHARED_LIBADD)
    PHP_SUBST(LIBNSQ_SHARED_LIBADD)
    
    PHP_NEW_EXTENSION(libNsq, php_libNsq.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi