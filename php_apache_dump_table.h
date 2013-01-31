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

#ifndef __HAS_APACHE_DUMP_TABLE__
#define __HAS_APACHE_DUMP_TABLE__

/* real HTML tags */
#define TD_NAME "<td class=\"directive\">"
#define TD_ARG "<td class=\"args\">"
#define TD_SUBDATA "<td class=\"subdata\" collspan=\"2\">"

/*	abstract tags to use in function */
#define TAG_START_ROW "<tr>"
#define TAG_START_ARG TD_ARG
#define TAG_START_SUBDATA TD_SUBDATA
#define TAG_END_ITEM "</td>"
#define TAG_END_ROW "</tr>"
#define TAG_START "<table>"
#define TAG_END "</table>"

/* tag based templates */
#define TAG_NAME_FORMAT TD_NAME "%s" TAG_END_ITEM
#define TAG_FILENAME_FORMAT TAG_START_ARG "%s" TAG_END_ITEM 
#define TAG_LINENUM_FORMAT TAG_START_ARG "%d" TAG_END_ITEM
#define TAG_FILE_DATA_FORMAT TAG_FILENAME_FORMAT TAG_LINENUM_FORMAT

#include "main/php_streams.h"

inline void print_name(php_stream *stream, const char *name)
{
	php_stream_printf(stream,TAG_START_ROW TAG_NAME_FORMAT,name);
}

inline void print_args(php_stream *stream, const char *args, int args_len)
{
	php_stream_puts(stream,TAG_START_ARG);
	php_stream_write(stream,args,args_len);
	php_stream_puts(stream,TAG_END_ITEM);
}

inline void print_file_data(php_stream *stream, const char *filename, int line)
{
	php_stream_printf(stream,TAG_FILE_DATA_FORMAT,filename,line);
}

inline void print_start_subtree(php_stream *stream)
{
	php_stream_puts(stream,TAG_START_ROW TAG_START_SUBDATA);
}

inline void print_end_subtree(php_stream *stream)
{
	php_stream_puts(stream,TAG_END_ITEM TAG_END_ROW);
}

inline void print_start_row(php_stream *stream)
{
	php_stream_puts(stream,TAG_START_ROW);
}

inline void print_end_row(php_stream *stream)
{
	php_stream_puts(stream,TAG_END_ROW);
}

inline void print_start_block(php_stream *stream)
{
	php_stream_puts(stream,TAG_START);
}

inline void print_end_block(php_stream *stream)
{
	php_stream_puts(stream,TAG_END);
}

#endif
