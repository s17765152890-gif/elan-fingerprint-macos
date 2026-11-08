/* Minimal C compatibility helpers used by the unmodified elanpress matcher.
 * Local adaptation, not a replacement implementation of GLib. */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
typedef uint8_t guint8; typedef uint16_t guint16; typedef unsigned guint;
typedef int gint; typedef int64_t gint64; typedef float gfloat; typedef double gdouble;
typedef int gboolean; typedef size_t gsize; typedef const void *gconstpointer;
#define TRUE 1
#define FALSE 0
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#define CLAMP(v,l,h) MIN(MAX(v,l),h)
#define G_SQRT2 1.4142135623730950488
#define G_PI 3.14159265358979323846
#define G_MAXINT INT_MAX
static inline void *g_malloc(size_t n){void *p=malloc(n?n:1);if(!p)abort();return p;}
static inline void *g_malloc0(size_t n){void *p=calloc(1,n?n:1);if(!p)abort();return p;}
#define g_new(t,n) ((t*)g_malloc(sizeof(t)*(n)))
#define g_new0(t,n) ((t*)g_malloc0(sizeof(t)*(n)))
#define g_free free
static inline void local_cleanup(void *p){free(*(void**)p);}
#define g_autofree __attribute__((cleanup(local_cleanup)))
#define G_DEFINE_AUTOPTR_CLEANUP_FUNC(t,f)
#define g_debug(...) ((void)0)
typedef struct _GSList{void *data;struct _GSList *next;} GSList;
typedef struct{char *data;guint len;size_t elem;} GArray;
static inline GArray *g_array_new(int z,int c,size_t e){GArray*a=g_new0(GArray,1);a->elem=e;return a;}
static inline void local_array_append(GArray*a,const void*v){char*p=realloc(a->data,(a->len+1)*a->elem);if(!p)abort();a->data=p;memcpy(a->data+a->len*a->elem,v,a->elem);a->len++;}
#define g_array_append_val(a,v) local_array_append(a,&(v))
static inline void g_array_sort(GArray*a,int(*cmp)(const void*,const void*)){qsort(a->data,a->len,a->elem,cmp);}
static inline void g_array_set_size(GArray*a,guint n){if(n>a->len){char*p=realloc(a->data,n*a->elem);if(!p)abort();a->data=p;}a->len=n;}
static inline char *g_array_free(GArray*a,int segment){char*p=a->data;if(segment){free(p);p=NULL;}free(a);return p;}
