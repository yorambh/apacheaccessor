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

#include "apache_conf_walk.h"

void walk_conf(ap_directive_t *root,handlers_set *handlers, void *pData)
{
	ap_directive_t *child;
	if (handlers->on_start) {
		if (handlers->on_start(root,pData) != CONTINUE) {
			return;
		}
	}
    for (child = root->first_child ; child ; child = child->next) {
		walk_conf(child,handlers,pData);
	}
	if (handlers->on_end) {
		handlers->on_end(root,pData);
	}
}
