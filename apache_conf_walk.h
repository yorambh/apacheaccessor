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

#ifndef _HAVE_APACHE_CONF_WALK_
#define _HAVE_APACHE_CONF_WALK_

typedef enum {
	CONTINUE,
	STOP,
	ERROR
} apache_accessor_walk_result;

#include <sys/types.h>
/* ugly hack to bypass apr.h problem */
#ifndef __off64_t_defined
typedef __off_t off64_t;
#define __off64_t_defined
#endif
#include "httpd.h"
#include "http_config.h"

typedef apache_accessor_walk_result (* directive_handler_t)(ap_directive_t *,void *pData);
typedef struct _handlers_set {
	directive_handler_t on_start;
	directive_handler_t on_end;
} handlers_set;

void walk_conf(ap_directive_t *root,handlers_set *handlers, void *pData);

#endif
