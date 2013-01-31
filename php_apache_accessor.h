/*
   +----------------------------------------------------------------------+
   | apacheaccessor                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2010 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Yoram Bar-Haim <yoram.b@zend.com>                           |
   +----------------------------------------------------------------------+
*/

#ifndef _HAVE_PHP_APACHE_
#define _HAVE_PHP_APACHE_

#include "httpd.h"
#include "http_config.h"
#include "zend.h"
#include "zend_API.h"

ZEND_BEGIN_MODULE_GLOBALS(apacheaccessor)
	ap_directive_t *conftree;
ZEND_END_MODULE_GLOBALS(apacheaccessor)

#ifdef ZTS
#define AAG(v) TSRMG(apacheaccessor_globals_id, zend_apacheaccessor_globals *, v)
#else
#define AAG(v) (apacheaccessor_globals.v)
#endif

#define APACHE_ACCESSOR_VERSION "0.1"
#define APACHE_ACCESSOR_PRODUCT_NAME "apacheaccessor"

#endif
