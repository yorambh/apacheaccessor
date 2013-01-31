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

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif
#include "php.h"
#include "php_apache_accessor.h"
#include "zend_ini.h"
#include "zend_hash.h"
#include "php_apache_dump_table.h"
#include "main/php_streams.h"
#include "apache_conf_walk.h"
#include <dlfcn.h>
#define OPEN_TAG '<'
#define CLOSE_TAG '>'

static void *_get_symbol_from_apache(char *symbol_name);
static int prepaire_named_entry(HashTable *table,zval **pentry,char *name,int name_len);
static int add_directive_value(zval *entry,ap_directive_t *tree);
ZEND_DECLARE_MODULE_GLOBALS(apacheaccessor)

ZEND_MINFO_FUNCTION(apacheaccessor);
ZEND_MINIT_FUNCTION(apacheaccessor);
ZEND_FUNCTION(apacheaccessor_get_conf);
ZEND_FUNCTION(apacheaccessor_dump_conf);

static zend_function_entry apacheaccessor_functions[] = {
	ZEND_FE(apacheaccessor_get_conf, NULL)
	ZEND_FE(apacheaccessor_dump_conf, NULL)
	{NULL,NULL,NULL}
};

zend_module_entry apacheaccessor_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"apacheaccessor",
	apacheaccessor_functions,
	ZEND_MINIT(apacheaccessor),
	NULL,
	NULL,
	NULL,
	ZEND_MINFO(apacheaccessor),
#if ZEND_MODULE_API_NO >= 20010901
	APACHE_ACCESSOR_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
ZEND_GET_MODULE(apacheaccessor)

ZEND_INI_BEGIN()
ZEND_INI_END()

typedef struct _dump_filter {
	zval *keys;
	struct _dump_filter *parent;
} dump_filter;

typedef struct _apache_dump_options {
	php_stream *stream;
	zend_bool dump_file_data;
	dump_filter *filter_stack ;
} apache_dump_options;

static apache_accessor_walk_result dump_start(ap_directive_t *tree,apache_dump_options *options)
{
	if (tree->directive) {
		zval **filter_value;
		zval *filter = options->filter_stack->keys;
		int args_len = strlen(tree->args);
		char *name =(char *)tree->directive;
		if (tree->directive[0] == OPEN_TAG) {
			name++;
		}
		if (filter) {
			if (
				Z_TYPE_P(filter) != IS_ARRAY ||
				zend_hash_find(Z_ARRVAL_P(filter),name,strlen(name)+1,(void **)&filter_value) != SUCCESS
			) {
				return STOP;
			}
			if (tree->first_child) { 
				dump_filter *current_filter = emalloc(sizeof(dump_filter));
				current_filter->parent = options->filter_stack;
				current_filter->keys = *filter_value;
				options->filter_stack = current_filter;
			}
		}
		if (args_len && (tree->args[args_len-1] == CLOSE_TAG)) {
			args_len--;
		}
		print_name(options->stream,name);
		print_args(options->stream,tree->args,args_len);
		if (options->dump_file_data) {
			print_file_data(options->stream,tree->filename,tree->line_num);
		}
		print_end_row(options->stream);
		if (tree->first_child) {
			print_start_subtree(options->stream);
		}
	}
	if (tree->first_child) {
		print_start_block(options->stream);
	}
	return CONTINUE;
}

static apache_accessor_walk_result dump_end(ap_directive_t *tree,apache_dump_options *options)
{
	if (tree->first_child) {
		print_end_block(options->stream);
		if (tree->directive) {
			print_end_subtree(options->stream);
		}
		if (options->filter_stack->parent) {
			dump_filter *tmp = options->filter_stack;
			options->filter_stack = options->filter_stack->parent;
			efree(tmp);
		}
	}
	return CONTINUE;
}

static int prepaire_named_entry(HashTable *table,zval **pentry,char *name,int name_len)
{
	zval **p;
	if (zend_hash_find(table,name,name_len+1,(void **)&p) != SUCCESS) {
		array_init(*pentry);
		zend_hash_add(table,name,name_len+1,(void **)pentry,sizeof(zval **),NULL);
	} else {
		*pentry = *p;
	}
	return SUCCESS;
}

static int read_section(zval *section,ap_directive_t *first)
{
	int i;
	ap_directive_t *child;
	for (child = first ; child ; child = child->next) {
		zval *entry ;
		ALLOC_INIT_ZVAL(entry);
		char *name = (char *)child->directive;
		int name_len = strlen(name);
		if (child->first_child && name[0] == OPEN_TAG) { // has subtree
			name++ ;
			name_len--;
		}
		prepaire_named_entry(Z_ARRVAL_P(section),&entry,name,name_len);
		if (add_directive_value(entry,child) == FAILURE) {
			zval_dtor(entry);
			zend_error(E_ERROR,"Cannot read value(s) for apache directive %s",name);
			return FAILURE;
		}
	}
	return SUCCESS;
}

static int add_directive_value(zval *entry,ap_directive_t *tree)
{
	int retval;
	zval *data;
	zval *filename;
	zval *line_num;
	zval *args;
	MAKE_STD_ZVAL(data);
	MAKE_STD_ZVAL(filename);
	MAKE_STD_ZVAL(line_num);
	MAKE_STD_ZVAL(args);
	array_init(data);
	ZVAL_STRING(filename,tree->filename,1);
	ZVAL_STRING(args,tree->args,1);
	ZVAL_LONG(line_num,tree->line_num);
	zend_hash_add(Z_ARRVAL_P(data),"filename",sizeof("filename"),(void **)&filename,sizeof(zval **),NULL);
	zend_hash_add(Z_ARRVAL_P(data),"line_num",sizeof("line_num"),(void **)&line_num,sizeof(zval **),NULL);
	if (tree->first_child) { // has subtree
		zval *subtree;
		MAKE_STD_ZVAL(subtree);
		array_init(subtree);
		if (tree->args[Z_STRLEN_P(args) - 1] == CLOSE_TAG) {
			Z_STRLEN_P(args)--;
		}
		if (read_section(subtree,tree->first_child) == FAILURE) {
			zval_dtor(data);
			zval_dtor(subtree);
			return FAILURE;
		}
		zend_hash_add(Z_ARRVAL_P(data),"subtree",sizeof("subtree"),(void **)&subtree,sizeof(zval **),NULL); 
	}
	zend_hash_add(Z_ARRVAL_P(data),"args",sizeof("args"),(void **)&args,sizeof(zval **),NULL);
	// retval = zend_hash_update(Z_ARRVAL_P(entry),args,args_len+1,(void **)&data,sizeof(zval **),NULL);
	retval = zend_hash_next_index_insert(Z_ARRVAL_P(entry),(void **)&data,sizeof(zval **),NULL);
	if (retval == FAILURE) {
		zval_dtor(data);
	}
	return retval; 
}

/* proto array apacheaccessor_get_conf()
 * returns Apache configuration tree as PHP array */
ZEND_FUNCTION(apacheaccessor_get_conf)
{
	if (AAG(conftree) == NULL) { // Should not happen
		zend_error(E_WARNING,"Called without apache symbol\n");
		return;
	}
	array_init(return_value);
	read_section(return_value,AAG(conftree)) ;
}

/* proto array apacheaccessor_dump_conf([bool dump_file_data [,resource fh [, array wanted_keys]]])
 * Dumps Apache configuration as HTML tables
 * 		dump_file_data means dumping of filename and line-number where the directive comes from (default FALSE)
 *		fh is an opened file-handle to write into (default is php://output)
 *		wanted_keys is array with keys indicates directives to dump
 *			if value is array, then it's keys are takes as section keys, otherwize ignored */
ZEND_FUNCTION(apacheaccessor_dump_conf)
{
	int argc = ZEND_NUM_ARGS();
	handlers_set set = {dump_start,dump_end};
	dump_filter stack_head = {0,0};
	zval *zstream = NULL;
	apache_dump_options options = {0,0,&stack_head} ;
	if (zend_parse_parameters(argc TSRMLS_CC,"|bra",
		&(options.dump_file_data),
		&zstream,
		&(options.filter_stack->keys)
	) == FAILURE) {
		return;
	}
	ap_directive_t tree = {
		NULL,NULL,NULL,AAG(conftree),NULL,NULL,NULL,0
	};
	if (zstream) {
		php_stream_from_zval(options.stream,&zstream);
	} else {
		options.stream = php_stream_open_wrapper("php://output","w",0,NULL);	
	}
	walk_conf(&tree,&set,&options);
	if (!zstream) {
		php_stream_close(options.stream);
	}
}

ZEND_MINFO_FUNCTION(apacheaccessor)
{
    php_info_print_table_start();
	php_info_print_table_row(2, "apacheaccessor", AAG(conftree)?"enabled":"disabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

ZEND_MINIT_FUNCTION(apacheaccessor)
{
	ap_directive_t **p = (ap_directive_t **)_get_symbol_from_apache("ap_conftree");
	if (p == NULL) {
		zend_error(E_WARNING,"can't find symbol") ;
		AAG(conftree) = NULL ;
	} else {
		AAG(conftree) = *p;
	}
}

static void *_get_symbol_from_apache(char *symbol_name)
{
#if ZEND_WIN32
	HMODULE hModule;

	hModule = GetModuleHandle(NULL);
	if (hModule) {
		void *p=GetProcAddress(hModule, symbol_name);
		if(!p) {
#ifdef ZTS
			hModule = GetModuleHandle(TEXT("php5ts"));
#else
			hModule = GetModuleHandle(TEXT("php5"));
#endif
			if(hModule) {
				p=GetProcAddress(hModule, symbol_name);
			}
		}
		return p;
	} else {
		return NULL;
	}
#else

	void *me = NULL, *sym = NULL;
#if HAVE_MACH_O_DYLD_H
    NSSymbol symbol;
    char *sn = malloc(strlen(symbol_name)+2);
    sprintf(sn, "_%s", symbol_name);
    symbol = NSLookupAndBindSymbol(sn);
    free(sn);
    return NSAddressOfSymbol(symbol);
#endif

#if defined(_AIX)

	me = dlopen(NULL, RTLD_GLOBAL|RTLD_NOW);

    sym = dlsym(me, symbol_name);

    if(!sym) {

        fprintf(stderr, "Dynamic linker error: %s\n", dlerror());

    }

    dlclose(me);

	return sym;

#else
#if HAVE_LIBDL
#if defined(RTLD_DEFAULT)
	return dlsym(RTLD_DEFAULT, symbol_name);
#else
	return dlsym(NULL, symbol_name);
#endif	
#else
	return NULL;
#endif

#endif
#endif
	return NULL;
}
