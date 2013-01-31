AC_DEFUN([PHP_ADD_EXAMPLES],[
	cat >>Makefile.global<<EOF
install-examples:
	\$(mkinstalldirs) $1/examples/
	\$(INSTALL_DATA) \$(top_srcdir)/examples/*.php $1/examples/
EOF
])

PHP_ARG_WITH(apxs, for apache support,
[  --with-apxs=PATH   apxs program])

PHP_ARG_WITH(examples, add examples to install,
[  --with-examples=TARGET_DIR])

PHP_ARG_WITH(aprconfig, for apache support,
[  --with-aprconfig=PATH    apr-config program])

  if test "$PHP_APXS" = "yes"; then
    if test `apxs -q INCLUDEDIR 2>/dev/null`; then   
	  APXS=apxs
    elif test `apxs2 -q INCLUDEDIR 2>/dev/null`; then   
	  APXS=apxs2
    else
      AC_MSG_ERROR([no apxs executable])
    fi
  elif test -x $PHP_APXS ; then
	  APXS=$PHP_APXS
  else
	  AC_MSG_ERROR([$PHP_APXS is not apxs executable])
  fi
  APACHE_INCLUDE_DIR=`$APXS -q INCLUDEDIR`
  if test -x $PHP_APRCONFIG; then
	APRCONFIG=$PHP_APRCONFIG
  else
    APRCONFIG=`$APXS -q BINDIR`/apr-config
  fi
  if test -x $APRCONFIG; then
    APR_INCLUDE_DIR=`$APRCONFIG --includedir`
  else
	AC_MSG_ERROR([no apr-config executable])
  fi

  if test "$PHP_EXAMPLES" = "yes"; then
    EXAMPLES_DIR=/var/www
  elif test -d $PHP_EXAMPLES; then
	EXAMPLES_DIR=$PHP_EXAMPLES
  fi
  if test -d "$EXAMPLES_DIR"; then
	AC_MSG_RESULT(adding examples)
	PHP_ADD_EXAMPLES($EXAMPLES_DIR)
  fi

  PHP_ADD_INCLUDE($APACHE_INCLUDE_DIR)
  PHP_ADD_INCLUDE($APR_INCLUDE_DIR)

PHP_SUBST(APACHE_ACCESSOR_SHARED_LIBADD)

PHP_NEW_EXTENSION(apacheaccessor, apache_conf_walk.c php_apache_accessor.c, $ext_shared)
