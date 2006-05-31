#include <setjmp.h>
/* This is a C header used by the output of the Cyclone to
   C translator.  Corresponding definitions are in file lib/runtime_*.c */
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

/* Need one of these per thread (see runtime_stack.c). The runtime maintains 
   a stack that contains either _handler_cons structs or _RegionHandle structs.
   The tag is 0 for a handler_cons and 1 for a region handle.  */
struct _RuntimeStack {
  int tag; 
  struct _RuntimeStack *next;
  void (*cleanup)(struct _RuntimeStack *frame);
};

#ifndef offsetof
/* should be size_t but int is fine */
#define offsetof(t,n) ((int)(&(((t*)0)->n)))
#endif

/* Fat pointers */
struct _fat_ptr {
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};  

/* Regions */
struct _RegionPage
{ 
#ifdef CYC_REGION_PROFILE
  unsigned total_bytes;
  unsigned free_bytes;
#endif
  struct _RegionPage *next;
  char data[1];
};

struct _pool;
struct bget_region_key;
struct _RegionAllocFunctions;

struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
#if(defined(__linux__) && defined(__KERNEL__))
  struct _RegionPage *vpage;
#endif 
  struct _RegionAllocFunctions *fcns;
  char               *offset;
  char               *last_plus_one;
  struct _pool *released_ptrs;
  struct bget_region_key *key;
#ifdef CYC_REGION_PROFILE
  const char *name;
#endif
  unsigned used_bytes;
  unsigned wasted_bytes;
};


// A dynamic region is just a region handle.  The wrapper struct is for type
// abstraction.
struct Cyc_Core_DynamicRegion {
  struct _RegionHandle h;
};

/* Alias qualifier stuff */
typedef unsigned int _AliasQualHandle_t; // must match aqualt_type() in toc.cyc

struct _RegionHandle _new_region(unsigned int, const char*);
void* _region_malloc(struct _RegionHandle*, _AliasQualHandle_t, unsigned);
void* _region_calloc(struct _RegionHandle*, _AliasQualHandle_t, unsigned t, unsigned n);
void* _region_vmalloc(struct _RegionHandle*, unsigned);
void * _aqual_malloc(_AliasQualHandle_t aq, unsigned int s);
void * _aqual_calloc(_AliasQualHandle_t aq, unsigned int n, unsigned int t);
void _free_region(struct _RegionHandle*);

/* Exceptions */
struct _handler_cons {
  struct _RuntimeStack s;
  jmp_buf handler;
};
void _push_handler(struct _handler_cons*);
void _push_region(struct _RegionHandle*);
void _npop_handler(int);
void _pop_handler();
void _pop_region();


#ifndef _throw
void* _throw_null_fn(const char*,unsigned);
void* _throw_arraybounds_fn(const char*,unsigned);
void* _throw_badalloc_fn(const char*,unsigned);
void* _throw_match_fn(const char*,unsigned);
void* _throw_assert_fn(const char *,unsigned);
void* _throw_fn(void*,const char*,unsigned);
void* _rethrow(void*);
#define _throw_null() (_throw_null_fn(__FILE__,__LINE__))
#define _throw_arraybounds() (_throw_arraybounds_fn(__FILE__,__LINE__))
#define _throw_badalloc() (_throw_badalloc_fn(__FILE__,__LINE__))
#define _throw_match() (_throw_match_fn(__FILE__,__LINE__))
#define _throw_assert() (_throw_assert_fn(__FILE__,__LINE__))
#define _throw(e) (_throw_fn((e),__FILE__,__LINE__))
#endif

void* Cyc_Core_get_exn_thrown();
/* Built-in Exceptions */
struct Cyc_Null_Exception_exn_struct { char *tag; };
struct Cyc_Array_bounds_exn_struct { char *tag; };
struct Cyc_Match_Exception_exn_struct { char *tag; };
struct Cyc_Bad_alloc_exn_struct { char *tag; };
struct Cyc_Assert_exn_struct { char *tag; };
extern char Cyc_Null_Exception[];
extern char Cyc_Array_bounds[];
extern char Cyc_Match_Exception[];
extern char Cyc_Bad_alloc[];
extern char Cyc_Assert[];

/* Built-in Run-time Checks and company */
#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#define _check_null(ptr) \
  ({ typeof(ptr) _cks_null = (ptr); \
     if (!_cks_null) _throw_null(); \
     _cks_null; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_notnull(ptr,bound,elt_sz,index)\
   (((char*)ptr) + (elt_sz)*(index))
#ifdef NO_CYC_NULL_CHECKS
#define _check_known_subscript_null _check_known_subscript_notnull
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr);\
  int _index = (index);\
  if (!_cks_ptr) _throw_null(); \
  _cks_ptr + (elt_sz)*_index; })
#endif
#define _zero_arr_plus_char_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_other_fn(t_sz,orig_x,orig_sz,orig_i,f,l)((orig_x)+(orig_i))
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (_cks_index >= (bound)) _throw_arraybounds(); \
  _cks_ptr + (elt_sz)*_cks_index; })
#define _check_known_subscript_notnull(ptr,bound,elt_sz,index) ({ \
  char*_cks_ptr = (char*)(ptr); \
  unsigned _cks_index = (index); \
  if (_cks_index >= (bound)) _throw_arraybounds(); \
  _cks_ptr + (elt_sz)*_cks_index; })

/* _zero_arr_plus_*_fn(x,sz,i,filename,lineno) adds i to zero-terminated ptr
   x that has at least sz elements */
char* _zero_arr_plus_char_fn(char*,unsigned,int,const char*,unsigned);
void* _zero_arr_plus_other_fn(unsigned,void*,unsigned,int,const char*,unsigned);
#endif

/* _get_zero_arr_size_*(x,sz) returns the number of elements in a
   zero-terminated array that is NULL or has at least sz elements */
unsigned _get_zero_arr_size_char(const char*,unsigned);
unsigned _get_zero_arr_size_other(unsigned,const void*,unsigned);

/* _zero_arr_inplace_plus_*_fn(x,i,filename,lineno) sets
   zero-terminated pointer *x to *x + i */
char* _zero_arr_inplace_plus_char_fn(char**,int,const char*,unsigned);
char* _zero_arr_inplace_plus_post_char_fn(char**,int,const char*,unsigned);
// note: must cast result in toc.cyc
void* _zero_arr_inplace_plus_other_fn(unsigned,void**,int,const char*,unsigned);
void* _zero_arr_inplace_plus_post_other_fn(unsigned,void**,int,const char*,unsigned);
#define _zero_arr_plus_char(x,s,i) \
  (_zero_arr_plus_char_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_inplace_plus_char(x,i) \
  _zero_arr_inplace_plus_char_fn((char**)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_char(x,i) \
  _zero_arr_inplace_plus_post_char_fn((char**)(x),(i),__FILE__,__LINE__)
#define _zero_arr_plus_other(t,x,s,i) \
  (_zero_arr_plus_other_fn(t,x,s,i,__FILE__,__LINE__))
#define _zero_arr_inplace_plus_other(t,x,i) \
  _zero_arr_inplace_plus_other_fn(t,(void**)(x),i,__FILE__,__LINE__)
#define _zero_arr_inplace_plus_post_other(t,x,i) \
  _zero_arr_inplace_plus_post_other_fn(t,(void**)(x),(i),__FILE__,__LINE__)

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_fat_subscript(arr,elt_sz,index) ((arr).curr + (elt_sz) * (index))
#define _untag_fat_ptr(arr,elt_sz,num_elts) ((arr).curr)
#define _untag_fat_ptr_check_bound(arr,elt_sz,num_elts) ((arr).curr)
#define _check_fat_at_base(arr) (arr)
#else
#define _check_fat_subscript(arr,elt_sz,index) ({ \
  struct _fat_ptr _cus_arr = (arr); \
  unsigned char *_cus_ans = _cus_arr.curr + (elt_sz) * (index); \
  /* JGM: not needed! if (!_cus_arr.base) _throw_null();*/ \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#define _untag_fat_ptr(arr,elt_sz,num_elts) ((arr).curr)
#define _untag_fat_ptr_check_bound(arr,elt_sz,num_elts) ({ \
  struct _fat_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if ((_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one) &&\
      _curr != (unsigned char*)0) \
    _throw_arraybounds(); \
  _curr; })
#define _check_fat_at_base(arr) ({ \
  struct _fat_ptr _arr = (arr); \
  if (_arr.base != _arr.curr) _throw_arraybounds(); \
  _arr; })
#endif

#define _tag_fat(tcurr,elt_sz,num_elts) ({ \
  struct _fat_ptr _ans; \
  unsigned _num_elts = (num_elts);\
  _ans.base = _ans.curr = (void*)(tcurr); \
  /* JGM: if we're tagging NULL, ignore num_elts */ \
  _ans.last_plus_one = _ans.base ? (_ans.base + (elt_sz) * _num_elts) : 0; \
  _ans; })

#define _get_fat_size(arr,elt_sz) \
  ({struct _fat_ptr _arr = (arr); \
    unsigned char *_arr_curr=_arr.curr; \
    unsigned char *_arr_last=_arr.last_plus_one; \
    (_arr_curr < _arr.base || _arr_curr >= _arr_last) ? 0 : \
    ((_arr_last - _arr_curr) / (elt_sz));})

#define _fat_ptr_plus(arr,elt_sz,change) ({ \
  struct _fat_ptr _ans = (arr); \
  int _change = (change);\
  _ans.curr += (elt_sz) * _change;\
  _ans; })
#define _fat_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _fat_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += (elt_sz) * (change);\
  *_arr_ptr; })
#define _fat_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _fat_ptr * _arr_ptr = (arr_ptr); \
  struct _fat_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += (elt_sz) * (change);\
  _ans; })

//Not a macro since initialization order matters. Defined in runtime_zeroterm.c.
struct _fat_ptr _fat_ptr_decrease_size(struct _fat_ptr,unsigned sz,unsigned numelts);

#ifdef CYC_GC_PTHREAD_REDIRECTS
# define pthread_create GC_pthread_create
# define pthread_sigmask GC_pthread_sigmask
# define pthread_join GC_pthread_join
# define pthread_detach GC_pthread_detach
# define dlopen GC_dlopen
#endif
/* Allocation */
void* GC_malloc(int);
void* GC_malloc_atomic(int);
void* GC_calloc(unsigned,unsigned);
void* GC_calloc_atomic(unsigned,unsigned);

#if(defined(__linux__) && defined(__KERNEL__))
void *cyc_vmalloc(unsigned);
void cyc_vfree(void*);
#endif
// bound the allocation size to be < MAX_ALLOC_SIZE. See macros below for usage.
#define MAX_MALLOC_SIZE (1 << 28)
void* _bounded_GC_malloc(int,const char*,int);
void* _bounded_GC_malloc_atomic(int,const char*,int);
void* _bounded_GC_calloc(unsigned,unsigned,const char*,int);
void* _bounded_GC_calloc_atomic(unsigned,unsigned,const char*,int);
/* these macros are overridden below ifdef CYC_REGION_PROFILE */
#ifndef CYC_REGION_PROFILE
#define _cycalloc(n) _bounded_GC_malloc(n,__FILE__,__LINE__)
#define _cycalloc_atomic(n) _bounded_GC_malloc_atomic(n,__FILE__,__LINE__)
#define _cyccalloc(n,s) _bounded_GC_calloc(n,s,__FILE__,__LINE__)
#define _cyccalloc_atomic(n,s) _bounded_GC_calloc_atomic(n,s,__FILE__,__LINE__)
#endif

static inline unsigned int _check_times(unsigned x, unsigned y) {
  unsigned long long whole_ans = 
    ((unsigned long long) x)*((unsigned long long)y);
  unsigned word_ans = (unsigned)whole_ans;
  if(word_ans < whole_ans || word_ans > MAX_MALLOC_SIZE)
    _throw_badalloc();
  return word_ans;
}

#define _CYC_MAX_REGION_CONST 0
#define _CYC_MIN_ALIGNMENT (sizeof(double))

#ifdef CYC_REGION_PROFILE
extern int rgn_total_bytes;
#endif

static inline void*_fast_region_malloc(struct _RegionHandle*r, _AliasQualHandle_t aq, unsigned orig_s) {  
  if (r > (struct _RegionHandle*)_CYC_MAX_REGION_CONST && r->curr != 0) { 
#ifdef CYC_NOALIGN
    unsigned s =  orig_s;
#else
    unsigned s =  (orig_s + _CYC_MIN_ALIGNMENT - 1) & (~(_CYC_MIN_ALIGNMENT -1)); 
#endif
    char *result; 
    result = r->offset; 
    if (s <= (r->last_plus_one - result)) {
      r->offset = result + s; 
#ifdef CYC_REGION_PROFILE
    r->curr->free_bytes = r->curr->free_bytes - s;
    rgn_total_bytes += s;
#endif
      return result;
    }
  } 
  return _region_malloc(r,aq,orig_s); 
}

//doesn't make sense to fast malloc with reaps
#ifndef DISABLE_REAPS
#define _fast_region_malloc _region_malloc
#endif

#ifdef CYC_REGION_PROFILE
/* see macros below for usage. defined in runtime_memory.c */
void* _profile_GC_malloc(int,const char*,const char*,int);
void* _profile_GC_malloc_atomic(int,const char*,const char*,int);
void* _profile_GC_calloc(unsigned,unsigned,const char*,const char*,int);
void* _profile_GC_calloc_atomic(unsigned,unsigned,const char*,const char*,int);
void* _profile_region_malloc(struct _RegionHandle*,_AliasQualHandle_t,unsigned,const char*,const char*,int);
void* _profile_region_calloc(struct _RegionHandle*,_AliasQualHandle_t,unsigned,unsigned,const char *,const char*,int);
void * _profile_aqual_malloc(_AliasQualHandle_t aq, unsigned int s,const char *file, const char *func, int lineno);
void * _profile_aqual_calloc(_AliasQualHandle_t aq, unsigned int t1,unsigned int t2,const char *file, const char *func, int lineno);
struct _RegionHandle _profile_new_region(unsigned int i, const char*,const char*,const char*,int);
void _profile_free_region(struct _RegionHandle*,const char*,const char*,int);
#ifndef RUNTIME_CYC
#define _new_region(i,n) _profile_new_region(i,n,__FILE__,__FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__,__FUNCTION__,__LINE__)
#define _region_malloc(rh,aq,n) _profile_region_malloc(rh,aq,n,__FILE__,__FUNCTION__,__LINE__)
#define _region_calloc(rh,aq,n,t) _profile_region_calloc(rh,aq,n,t,__FILE__,__FUNCTION__,__LINE__)
#define _aqual_malloc(aq,n) _profile_aqual_malloc(aq,n,__FILE__,__FUNCTION__,__LINE__)
#define _aqual_calloc(aq,n,t) _profile_aqual_calloc(aq,n,t,__FILE__,__FUNCTION__,__LINE__)
#endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__,__FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__,__FUNCTION__,__LINE__)
#define _cyccalloc(n,s) _profile_GC_calloc(n,s,__FILE__,__FUNCTION__,__LINE__)
#define _cyccalloc_atomic(n,s) _profile_GC_calloc_atomic(n,s,__FILE__,__FUNCTION__,__LINE__)
#endif //CYC_REGION_PROFILE
#endif //_CYC_INCLUDE_H
 struct Cyc_Core_Opt{void*v;};extern char Cyc_Core_Invalid_argument[17U];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _fat_ptr f1;};extern char Cyc_Core_Failure[8U];struct Cyc_Core_Failure_exn_struct{char*tag;struct _fat_ptr f1;};extern char Cyc_Core_Impossible[11U];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _fat_ptr f1;};extern char Cyc_Core_Not_found[10U];struct Cyc_Core_Not_found_exn_struct{char*tag;};
# 173 "core.h"
extern struct _RegionHandle*Cyc_Core_heap_region;
# 321 "core.h"
void Cyc_Core_rethrow(void*);
# 38 "cycboot.h"
extern int Cyc_open(const char*,int,struct _fat_ptr);struct Cyc___cycFILE;
# 51
extern struct Cyc___cycFILE*Cyc_stdout;
# 53
extern struct Cyc___cycFILE*Cyc_stderr;struct Cyc_String_pa_PrintArg_struct{int tag;struct _fat_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};
# 73
extern struct _fat_ptr Cyc_aprintf(struct _fat_ptr,struct _fat_ptr);
# 79
extern int Cyc_fclose(struct Cyc___cycFILE*);
# 88
extern int Cyc_fflush(struct Cyc___cycFILE*);
# 98
extern struct Cyc___cycFILE*Cyc_fopen(const char*,const char*);
# 100
extern int Cyc_fprintf(struct Cyc___cycFILE*,struct _fat_ptr,struct _fat_ptr);
# 104
extern int Cyc_fputc(int,struct Cyc___cycFILE*);
# 106
extern int Cyc_fputs(const char*,struct Cyc___cycFILE*);
# 224 "cycboot.h"
extern int Cyc_vfprintf(struct Cyc___cycFILE*,struct _fat_ptr,struct _fat_ptr);
# 300 "cycboot.h"
extern int isspace(int);
# 310
extern int toupper(int);
# 318
extern int system(const char*);
extern void exit(int);
# 331
extern int mkdir(const char*,unsigned short);
# 334
extern int close(int);
extern int chdir(const char*);
extern struct _fat_ptr Cyc_getcwd(struct _fat_ptr,unsigned long);extern char Cyc_Lexing_Error[6U];struct Cyc_Lexing_Error_exn_struct{char*tag;struct _fat_ptr f1;};struct Cyc_Lexing_lexbuf{void(*refill_buff)(struct Cyc_Lexing_lexbuf*);void*refill_state;struct _fat_ptr lex_buffer;int lex_buffer_len;int lex_abs_pos;int lex_start_pos;int lex_curr_pos;int lex_last_pos;int lex_last_action;int lex_eof_reached;};
# 78 "lexing.h"
extern struct Cyc_Lexing_lexbuf*Cyc_Lexing_from_file(struct Cyc___cycFILE*);
# 82
extern struct _fat_ptr Cyc_Lexing_lexeme(struct Cyc_Lexing_lexbuf*);
extern char Cyc_Lexing_lexeme_char(struct Cyc_Lexing_lexbuf*,int);
extern int Cyc_Lexing_lexeme_start(struct Cyc_Lexing_lexbuf*);
extern int Cyc_Lexing_lexeme_end(struct Cyc_Lexing_lexbuf*);struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 54 "list.h"
extern struct Cyc_List_List*Cyc_List_list(struct _fat_ptr);
# 76
extern struct Cyc_List_List*Cyc_List_map(void*(*)(void*),struct Cyc_List_List*);
# 172
extern struct Cyc_List_List*Cyc_List_rev(struct Cyc_List_List*);
# 178
extern struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*);
# 184
extern struct Cyc_List_List*Cyc_List_append(struct Cyc_List_List*,struct Cyc_List_List*);struct _tuple0{struct Cyc_List_List*f0;struct Cyc_List_List*f1;};
# 294
extern struct _tuple0 Cyc_List_split(struct Cyc_List_List*);
# 322
extern int Cyc_List_mem(int(*)(void*,void*),struct Cyc_List_List*,void*);struct Cyc_Iter_Iter{void*env;int(*next)(void*,void*);};
# 37 "iter.h"
int Cyc_Iter_next(struct Cyc_Iter_Iter,void*);struct Cyc_Set_Set;
# 51 "set.h"
extern struct Cyc_Set_Set*Cyc_Set_empty(int(*)(void*,void*));
# 65
extern struct Cyc_Set_Set*Cyc_Set_insert(struct Cyc_Set_Set*,void*);
# 77
extern struct Cyc_Set_Set*Cyc_Set_union_two(struct Cyc_Set_Set*,struct Cyc_Set_Set*);
# 84
extern struct Cyc_Set_Set*Cyc_Set_diff(struct Cyc_Set_Set*,struct Cyc_Set_Set*);
# 87
extern struct Cyc_Set_Set*Cyc_Set_delete(struct Cyc_Set_Set*,void*);
# 96
extern int Cyc_Set_cardinality(struct Cyc_Set_Set*);
# 99
extern int Cyc_Set_is_empty(struct Cyc_Set_Set*);
# 102
extern int Cyc_Set_member(struct Cyc_Set_Set*,void*);
# 139
extern void*Cyc_Set_choose(struct Cyc_Set_Set*);
# 143
extern struct Cyc_Iter_Iter Cyc_Set_make_iter(struct _RegionHandle*,struct Cyc_Set_Set*);
# 38 "string.h"
extern unsigned long Cyc_strlen(struct _fat_ptr);
# 49 "string.h"
extern int Cyc_strcmp(struct _fat_ptr,struct _fat_ptr);
extern int Cyc_strptrcmp(struct _fat_ptr*,struct _fat_ptr*);
extern int Cyc_strncmp(struct _fat_ptr,struct _fat_ptr,unsigned long);
# 62
extern struct _fat_ptr Cyc_strconcat(struct _fat_ptr,struct _fat_ptr);
# 64
extern struct _fat_ptr Cyc_strconcat_l(struct Cyc_List_List*);
# 66
extern struct _fat_ptr Cyc_str_sepstr(struct Cyc_List_List*,struct _fat_ptr);
# 105 "string.h"
extern struct _fat_ptr Cyc_strdup(struct _fat_ptr);
# 110
extern struct _fat_ptr Cyc_substring(struct _fat_ptr,int,unsigned long);struct Cyc_Hashtable_Table;
# 39 "hashtable.h"
extern struct Cyc_Hashtable_Table*Cyc_Hashtable_create(int,int(*)(void*,void*),int(*)(void*));
# 50
extern void Cyc_Hashtable_insert(struct Cyc_Hashtable_Table*,void*,void*);
# 52
extern void*Cyc_Hashtable_lookup(struct Cyc_Hashtable_Table*,void*);
# 86
extern int Cyc_Hashtable_hash_stringptr(struct _fat_ptr*);
# 30 "filename.h"
extern struct _fat_ptr Cyc_Filename_concat(struct _fat_ptr,struct _fat_ptr);
# 34
extern struct _fat_ptr Cyc_Filename_chop_extension(struct _fat_ptr);
# 40
extern struct _fat_ptr Cyc_Filename_dirname(struct _fat_ptr);
# 43
extern struct _fat_ptr Cyc_Filename_basename(struct _fat_ptr);struct Cyc_Arg_Unit_spec_Arg_Spec_struct{int tag;void(*f1)(void);};struct Cyc_Arg_Flag_spec_Arg_Spec_struct{int tag;void(*f1)(struct _fat_ptr);};struct Cyc_Arg_Set_spec_Arg_Spec_struct{int tag;int*f1;};struct Cyc_Arg_String_spec_Arg_Spec_struct{int tag;void(*f1)(struct _fat_ptr);};
# 66 "arg.h"
extern void Cyc_Arg_usage(struct Cyc_List_List*,struct _fat_ptr);
# 69
extern int Cyc_Arg_current;
# 71
extern void Cyc_Arg_parse(struct Cyc_List_List*,void(*)(struct _fat_ptr),int(*)(struct _fat_ptr),struct _fat_ptr,struct _fat_ptr);struct Cyc_Buffer_t;
# 50 "buffer.h"
extern struct Cyc_Buffer_t*Cyc_Buffer_create(unsigned);
# 58
extern struct _fat_ptr Cyc_Buffer_contents(struct Cyc_Buffer_t*);
# 81
extern void Cyc_Buffer_add_char(struct Cyc_Buffer_t*,char);
# 92 "buffer.h"
extern void Cyc_Buffer_add_string(struct Cyc_Buffer_t*,struct _fat_ptr);struct Cyc_AssnDef_ExistAssnFn;struct _union_Nmspace_Abs_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Rel_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_C_n{int tag;struct Cyc_List_List*val;};struct _union_Nmspace_Loc_n{int tag;int val;};union Cyc_Absyn_Nmspace{struct _union_Nmspace_Abs_n Abs_n;struct _union_Nmspace_Rel_n Rel_n;struct _union_Nmspace_C_n C_n;struct _union_Nmspace_Loc_n Loc_n;};struct _tuple1{union Cyc_Absyn_Nmspace f0;struct _fat_ptr*f1;};
# 140 "absyn.h"
enum Cyc_Absyn_Scope{Cyc_Absyn_Static =0U,Cyc_Absyn_Abstract =1U,Cyc_Absyn_Public =2U,Cyc_Absyn_Extern =3U,Cyc_Absyn_ExternC =4U,Cyc_Absyn_Register =5U};struct Cyc_Absyn_Tqual{int print_const: 1;int q_volatile: 1;int q_restrict: 1;int real_const: 1;unsigned loc;};
# 163
enum Cyc_Absyn_AggrKind{Cyc_Absyn_StructA =0U,Cyc_Absyn_UnionA =1U};struct Cyc_Absyn_PtrLoc{unsigned ptr_loc;unsigned rgn_loc;unsigned zt_loc;};struct Cyc_Absyn_PtrAtts{void*eff;void*nullable;void*bounds;void*zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;void*autoreleased;void*aqual;};struct Cyc_Absyn_PtrInfo{void*elt_type;struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{struct _fat_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{struct Cyc_List_List*tvars;void*effect;struct Cyc_Absyn_Tqual ret_tqual;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*qual_bnd;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*checks_clause;struct Cyc_AssnDef_ExistAssnFn*checks_assn;struct Cyc_Absyn_Exp*requires_clause;struct Cyc_AssnDef_ExistAssnFn*requires_assn;struct Cyc_Absyn_Exp*ensures_clause;struct Cyc_AssnDef_ExistAssnFn*ensures_assn;struct Cyc_Absyn_Exp*throws_clause;struct Cyc_AssnDef_ExistAssnFn*throws_assn;struct Cyc_Absyn_Vardecl*return_value;struct Cyc_List_List*arg_vardecls;struct Cyc_List_List*effconstr;};struct _tuple3{enum Cyc_Absyn_AggrKind f0;struct _tuple1*f1;struct Cyc_Core_Opt*f2;};struct _union_AggrInfo_UnknownAggr{int tag;struct _tuple3 val;};struct _union_AggrInfo_KnownAggr{int tag;struct Cyc_Absyn_Aggrdecl**val;};union Cyc_Absyn_AggrInfo{struct _union_AggrInfo_UnknownAggr UnknownAggr;struct _union_AggrInfo_KnownAggr KnownAggr;};struct Cyc_Absyn_ArrayInfo{void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;void*zero_term;unsigned zt_loc;};struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct{int tag;struct Cyc_Absyn_Datatypedecl*f1;};struct Cyc_Absyn_TypeDecl{void*r;unsigned loc;};struct Cyc_Absyn_EnumCon_Absyn_TyCon_struct{int tag;struct _tuple1*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct{int tag;union Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AppType_Absyn_Type_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_PointerType_Absyn_Type_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_ArrayType_Absyn_Type_struct{int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_Absyn_Type_struct{int tag;struct Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct{int tag;enum Cyc_Absyn_AggrKind f1;int f2;struct Cyc_List_List*f3;};struct Cyc_Absyn_TypedefType_Absyn_Type_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;void*f4;};struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct{int tag;struct Cyc_Absyn_TypeDecl*f1;void**f2;};struct Cyc_Absyn_TypeofType_Absyn_Type_struct{int tag;struct Cyc_Absyn_Exp*f1;};
# 524 "absyn.h"
enum Cyc_Absyn_Primop{Cyc_Absyn_Plus =0U,Cyc_Absyn_Times =1U,Cyc_Absyn_Minus =2U,Cyc_Absyn_Div =3U,Cyc_Absyn_Mod =4U,Cyc_Absyn_Eq =5U,Cyc_Absyn_Neq =6U,Cyc_Absyn_Gt =7U,Cyc_Absyn_Lt =8U,Cyc_Absyn_Gte =9U,Cyc_Absyn_Lte =10U,Cyc_Absyn_Not =11U,Cyc_Absyn_Bitnot =12U,Cyc_Absyn_Bitand =13U,Cyc_Absyn_Bitor =14U,Cyc_Absyn_Bitxor =15U,Cyc_Absyn_Bitlshift =16U,Cyc_Absyn_Bitlrshift =17U,Cyc_Absyn_Numelts =18U,Cyc_Absyn_Tagof =19U,Cyc_Absyn_UDiv =20U,Cyc_Absyn_UMod =21U,Cyc_Absyn_UGt =22U,Cyc_Absyn_ULt =23U,Cyc_Absyn_UGte =24U,Cyc_Absyn_ULte =25U};
# 531
enum Cyc_Absyn_Incrementor{Cyc_Absyn_PreInc =0U,Cyc_Absyn_PostInc =1U,Cyc_Absyn_PreDec =2U,Cyc_Absyn_PostDec =3U};struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct{int tag;struct _fat_ptr*f1;};
# 549
enum Cyc_Absyn_Coercion{Cyc_Absyn_Unknown_coercion =0U,Cyc_Absyn_No_coercion =1U,Cyc_Absyn_Null_to_NonNull =2U,Cyc_Absyn_Other_coercion =3U};
# 563
enum Cyc_Absyn_MallocKind{Cyc_Absyn_Malloc =0U,Cyc_Absyn_Calloc =1U,Cyc_Absyn_Vmalloc =2U};struct Cyc_Absyn_MallocInfo{enum Cyc_Absyn_MallocKind mknd;struct Cyc_Absyn_Exp*rgn;struct Cyc_Absyn_Exp*aqual;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;int inline_call;};struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct{int tag;enum Cyc_Absyn_Primop f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;enum Cyc_Absyn_Incrementor f2;};struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;int f4;};struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;enum Cyc_Absyn_Coercion f4;};struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _fat_ptr*f2;int f3;int f4;};struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _fat_ptr*f2;int f3;int f4;};struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct _tuple9{struct _fat_ptr*f0;struct Cyc_Absyn_Tqual f1;void*f2;};struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _fat_ptr*f2;};struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct{int tag;void*f1;};struct Cyc_Absyn_Extension_e_Absyn_Raw_exp_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Exp{void*topt;void*r;unsigned loc;void*annot;};struct Cyc_Absyn_Stmt{void*r;unsigned loc;void*annot;};struct Cyc_Absyn_Vardecl{enum Cyc_Absyn_Scope sc;struct _tuple1*name;unsigned varloc;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;void*rgn;struct Cyc_List_List*attributes;int escapes;int is_proto;struct Cyc_Absyn_Exp*rename;};struct Cyc_Absyn_Fndecl{enum Cyc_Absyn_Scope sc;int is_inline;struct _tuple1*name;struct Cyc_Absyn_Stmt*body;struct Cyc_Absyn_FnInfo i;void*cached_type;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;enum Cyc_Absyn_Scope orig_scope;int escapes;};struct Cyc_Absyn_Aggrfield{struct _fat_ptr*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;struct Cyc_Absyn_Exp*requires_clause;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct Cyc_List_List*qual_bnd;struct Cyc_List_List*fields;int tagged;struct Cyc_List_List*effconstr;};struct Cyc_Absyn_Aggrdecl{enum Cyc_Absyn_AggrKind kind;enum Cyc_Absyn_Scope sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*impl;struct Cyc_List_List*attributes;int expected_mem_kind;};struct Cyc_Absyn_Datatypedecl{enum Cyc_Absyn_Scope sc;struct _tuple1*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int is_extensible;};struct Cyc_Absyn_Enumfield{struct _tuple1*name;struct Cyc_Absyn_Exp*tag;unsigned loc;};struct Cyc_Absyn_Enumdecl{enum Cyc_Absyn_Scope sc;struct _tuple1*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct _tuple1*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*kind;void*defn;struct Cyc_List_List*atts;int extern_c;};struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Decl{void*r;unsigned loc;};
# 1163 "absyn.h"
struct Cyc_Absyn_Decl*Cyc_Absyn_new_decl(void*,unsigned);
# 1221
struct Cyc_Absyn_Decl*Cyc_Absyn_lookup_decl(struct Cyc_List_List*,struct _fat_ptr*);struct _tuple12{enum Cyc_Absyn_AggrKind f0;struct _tuple1*f1;};
# 1227
struct _tuple12 Cyc_Absyn_aggr_kinded_name(union Cyc_Absyn_AggrInfo);
# 1235
struct _tuple1*Cyc_Absyn_binding2qvar(void*);struct Cyc_Absynpp_Params{int expand_typedefs;int qvar_to_Cids;int add_cyc_prefix;int to_VC;int decls_first;int rewrite_temp_tvars;int print_all_tvars;int print_all_kinds;int print_all_effects;int print_using_stmts;int print_externC_stmts;int print_full_evars;int print_zeroterm;int generate_line_directives;int use_curr_namespace;struct Cyc_List_List*curr_namespace;int gen_clean_cyclone;};
# 54 "absynpp.h"
void Cyc_Absynpp_set_params(struct Cyc_Absynpp_Params*);
# 56
extern struct Cyc_Absynpp_Params Cyc_Absynpp_cyc_params_r;
# 58
void Cyc_Absynpp_decllist2file(struct Cyc_List_List*,struct Cyc___cycFILE*);
# 63
struct _fat_ptr Cyc_Absynpp_typ2string(void*);
# 27 "warn.h"
extern void Cyc_Warn_reset(struct _fat_ptr);
extern int Cyc_Warn_print_warnings;
# 25 "parse.h"
struct Cyc_List_List*Cyc_Parse_parse_file(struct Cyc___cycFILE*);struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,void*);struct _RegionHandle*r;const struct Cyc_Dict_T*t;};extern char Cyc_Dict_Absent[7U];struct Cyc_Dict_Absent_exn_struct{char*tag;};struct Cyc_Tcenv_Genv{struct Cyc_Dict_Dict aggrdecls;struct Cyc_Dict_Dict datatypedecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict typedefs;struct Cyc_Dict_Dict ordinaries;};struct Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Tcenv_Genv*ae;struct Cyc_Tcenv_Fenv*le;int allow_valueof: 1;int in_extern_c_include: 1;int in_tempest: 1;int tempest_generalize: 1;int in_extern_c_inc_repeat: 1;};
# 68 "tcenv.h"
struct Cyc_Tcenv_Tenv*Cyc_Tcenv_tc_init (void);
# 29 "tc.h"
void Cyc_Tc_tc(struct Cyc_Tcenv_Tenv*,int,struct Cyc_List_List*);
# 29 "binding.h"
void Cyc_Binding_resolve_all(struct Cyc_List_List*);
# 30 "specsfile.h"
extern void Cyc_Specsfile_set_target_arch(struct _fat_ptr);
# 32
extern void Cyc_Specsfile_add_cyclone_exec_path(struct _fat_ptr);
# 34
extern struct Cyc_List_List*Cyc_Specsfile_read_specs(struct _fat_ptr);
# 36
extern struct _fat_ptr Cyc_Specsfile_get_spec(struct Cyc_List_List*,struct _fat_ptr);
# 39
extern struct _fat_ptr Cyc_Specsfile_parse_b(struct Cyc_List_List*,void(*)(struct _fat_ptr),int(*)(struct _fat_ptr),struct _fat_ptr,struct _fat_ptr);
# 44
extern struct _fat_ptr Cyc_Specsfile_find_in_arch_path(struct _fat_ptr);
# 78 "buildlib.cyl"
extern void Cyc_Lex_lex_init(int);static char _TmpG0[4U]="gcc";
# 88
static struct _fat_ptr Cyc_cyclone_cc={_TmpG0,_TmpG0,_TmpG0 + 4U};static char _TmpG1[1U]="";
static struct _fat_ptr Cyc_target_cflags={_TmpG1,_TmpG1,_TmpG1 + 1U};
# 91
static int Cyc_do_setjmp=0;
static int Cyc_verbose=0;
# 94
struct Cyc___cycFILE*Cyc_log_file=0;
struct Cyc___cycFILE*Cyc_cstubs_file=0;
struct Cyc___cycFILE*Cyc_cycstubs_file=0;
# 98
int Cyc_log(struct _fat_ptr fmt,struct _fat_ptr ap){struct Cyc___cycFILE*_T0;struct _fat_ptr _T1;struct _fat_ptr _T2;struct Cyc___cycFILE*_T3;int _T4;
# 101
if(Cyc_log_file!=0)goto _TL0;_T0=Cyc_stderr;_T1=
_tag_fat("Internal error: log file is NULL\n",sizeof(char),34U);_T2=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T0,_T1,_T2);
exit(1);goto _TL1;_TL0: _TL1: {
# 105
int x=Cyc_vfprintf(Cyc_log_file,fmt,ap);_T3=
_check_null(Cyc_log_file);Cyc_fflush(_T3);_T4=x;
return _T4;}}
# 110
static struct _fat_ptr*Cyc_current_source=0;
static struct Cyc_List_List*Cyc_current_args=0;
static struct Cyc_Set_Set**Cyc_current_targets=0;
static void Cyc_add_target(struct _fat_ptr*sptr){struct Cyc_Set_Set**_T0;struct Cyc_Set_Set*(*_T1)(struct Cyc_Set_Set*,struct _fat_ptr*);struct Cyc_Set_Set*(*_T2)(struct Cyc_Set_Set*,void*);struct Cyc_Set_Set**_T3;struct Cyc_Set_Set*_T4;struct _fat_ptr*_T5;{struct Cyc_Set_Set**_T6=_cycalloc(sizeof(struct Cyc_Set_Set*));_T2=Cyc_Set_insert;{
struct Cyc_Set_Set*(*_T7)(struct Cyc_Set_Set*,struct _fat_ptr*)=(struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T2;_T1=_T7;}_T3=_check_null(Cyc_current_targets);_T4=*_T3;_T5=sptr;*_T6=_T1(_T4,_T5);_T0=(struct Cyc_Set_Set**)_T6;}Cyc_current_targets=_T0;}
# 116
static struct Cyc_Set_Set**Cyc_headers_to_do=0;struct _tuple13{struct _fat_ptr*f0;struct Cyc_Set_Set*f1;};
# 120
struct _tuple13*Cyc_line(struct Cyc_Lexing_lexbuf*);
int Cyc_macroname(struct Cyc_Lexing_lexbuf*);
int Cyc_args(struct Cyc_Lexing_lexbuf*);
int Cyc_token(struct Cyc_Lexing_lexbuf*);
int Cyc_string(struct Cyc_Lexing_lexbuf*);
# 126
struct Cyc___cycFILE*Cyc_slurp_out=0;
# 128
int Cyc_slurp_string(struct Cyc_Lexing_lexbuf*);
# 130
int Cyc_asm_string(struct Cyc_Lexing_lexbuf*);
int Cyc_asm_comment(struct Cyc_Lexing_lexbuf*);struct _tuple14{struct _fat_ptr f0;struct _fat_ptr*f1;};
# 134
struct _tuple14*Cyc_suck_line(struct Cyc_Lexing_lexbuf*);
int Cyc_suck_macroname(struct Cyc_Lexing_lexbuf*);
int Cyc_suck_restofline(struct Cyc_Lexing_lexbuf*);
struct _fat_ptr Cyc_current_line={(void*)0,(void*)0,(void*)(0 + 0)};struct _tuple15{struct _fat_ptr f0;struct _fat_ptr f1;};struct _tuple16{struct _fat_ptr*f0;struct _fat_ptr*f1;};struct _tuple17{struct _fat_ptr f0;struct Cyc_List_List*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_List_List*f4;struct Cyc_List_List*f5;struct Cyc_List_List*f6;struct Cyc_List_List*f7;};
# 150
struct _tuple17*Cyc_spec(struct Cyc_Lexing_lexbuf*);
int Cyc_commands(struct Cyc_Lexing_lexbuf*);
int Cyc_snarfsymbols(struct Cyc_Lexing_lexbuf*);
int Cyc_block(struct Cyc_Lexing_lexbuf*);
int Cyc_block_string(struct Cyc_Lexing_lexbuf*);
int Cyc_block_comment(struct Cyc_Lexing_lexbuf*);
struct _fat_ptr Cyc_current_headerfile={(void*)0,(void*)0,(void*)(0 + 0)};
struct Cyc_List_List*Cyc_snarfed_symbols=0;
struct Cyc_List_List*Cyc_current_symbols=0;
struct Cyc_List_List*Cyc_current_user_defs=0;
struct Cyc_List_List*Cyc_current_cstubs=0;
struct Cyc_List_List*Cyc_current_cycstubs=0;
struct Cyc_List_List*Cyc_current_hstubs=0;
struct Cyc_List_List*Cyc_current_omit_symbols=0;
struct Cyc_List_List*Cyc_current_cpp=0;
struct Cyc_Buffer_t*Cyc_specbuf=0;
struct _fat_ptr Cyc_current_symbol={(void*)0,(void*)0,(void*)(0 + 0)};
int Cyc_rename_current_symbol=0;
int Cyc_braces_to_match=0;
int Cyc_parens_to_match=0;
# 171
int Cyc_numdef=0;
# 173
static struct Cyc_List_List*Cyc_cppargs=0;static char _TmpG2[14U]="BUILDLIB_sym_";
# 175
struct _fat_ptr Cyc_user_prefix={_TmpG2,_TmpG2,_TmpG2 + 14U};
static struct _fat_ptr*Cyc_add_user_prefix(struct _fat_ptr*symbol){struct _fat_ptr _T0;struct _fat_ptr*_T1;struct _fat_ptr _T2;struct _fat_ptr _T3;struct _fat_ptr*_T4;_T0=Cyc_user_prefix;_T1=symbol;_T2=*_T1;_T3=
Cyc_strconcat(_T0,_T2);{struct _fat_ptr s=_T3;{struct _fat_ptr*_T5=_cycalloc(sizeof(struct _fat_ptr));
*_T5=s;_T4=(struct _fat_ptr*)_T5;}return _T4;}}
# 180
static struct _fat_ptr Cyc_remove_user_prefix(struct _fat_ptr symbol){int _T0;struct Cyc_String_pa_PrintArg_struct _T1;void**_T2;struct Cyc___cycFILE*_T3;struct _fat_ptr _T4;struct _fat_ptr _T5;struct _fat_ptr _T6;struct _fat_ptr _T7;unsigned _T8;int _T9;unsigned long _TA;unsigned _TB;unsigned long _TC;struct _fat_ptr _TD;struct _fat_ptr _TE;
unsigned prefix_len=Cyc_strlen(Cyc_user_prefix);_T0=
Cyc_strncmp(symbol,Cyc_user_prefix,prefix_len);if(_T0==0)goto _TL2;{struct Cyc_String_pa_PrintArg_struct _TF;_TF.tag=0;
_TF.f1=symbol;_T1=_TF;}{struct Cyc_String_pa_PrintArg_struct _TF=_T1;void*_T10[1];_T2=_T10 + 0;*_T2=& _TF;_T3=Cyc_stderr;_T4=_tag_fat("Internal error: bad user type name %s\n",sizeof(char),39U);_T5=_tag_fat(_T10,sizeof(void*),1);Cyc_fprintf(_T3,_T4,_T5);}_T6=symbol;
return _T6;_TL2: _T7=symbol;_T8=prefix_len;_T9=(int)_T8;_TA=
# 186
Cyc_strlen(symbol);_TB=prefix_len;_TC=_TA - _TB;_TD=Cyc_substring(_T7,_T9,_TC);_TE=_TD;return _TE;}
# 189
static void Cyc_rename_decl(struct Cyc_Absyn_Decl*d){struct Cyc_Absyn_Decl*_T0;int*_T1;unsigned _T2;struct Cyc_Absyn_Aggrdecl*_T3;struct _tuple1*_T4;struct _fat_ptr*_T5;struct Cyc_Absyn_Aggrdecl*_T6;struct _tuple1*_T7;struct _tuple1 _T8;struct _fat_ptr*_T9;struct _fat_ptr _TA;struct Cyc_Absyn_Enumdecl*_TB;struct _tuple1*_TC;struct _fat_ptr*_TD;struct Cyc_Absyn_Enumdecl*_TE;struct _tuple1*_TF;struct _tuple1 _T10;struct _fat_ptr*_T11;struct _fat_ptr _T12;struct Cyc_Absyn_Typedefdecl*_T13;struct _tuple1*_T14;struct _fat_ptr*_T15;struct Cyc_Absyn_Typedefdecl*_T16;struct _tuple1*_T17;struct _tuple1 _T18;struct _fat_ptr*_T19;struct _fat_ptr _T1A;struct Cyc___cycFILE*_T1B;struct _fat_ptr _T1C;struct _fat_ptr _T1D;_T0=d;{
void*_T1E=_T0->r;struct Cyc_Absyn_Typedefdecl*_T1F;struct Cyc_Absyn_Enumdecl*_T20;struct Cyc_Absyn_Aggrdecl*_T21;_T1=(int*)_T1E;_T2=*_T1;switch(_T2){case 5:{struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*_T22=(struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_T1E;_T21=_T22->f1;}{struct Cyc_Absyn_Aggrdecl*ad=_T21;_T3=ad;_T4=_T3->name;{struct _fat_ptr*_T22=_cycalloc(sizeof(struct _fat_ptr));_T6=ad;_T7=_T6->name;_T8=*_T7;_T9=_T8.f1;_TA=*_T9;
# 192
*_T22=Cyc_remove_user_prefix(_TA);_T5=(struct _fat_ptr*)_T22;}(*_T4).f1=_T5;goto _LL0;}case 7:{struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*_T22=(struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_T1E;_T20=_T22->f1;}{struct Cyc_Absyn_Enumdecl*ed=_T20;_TB=ed;_TC=_TB->name;{struct _fat_ptr*_T22=_cycalloc(sizeof(struct _fat_ptr));_TE=ed;_TF=_TE->name;_T10=*_TF;_T11=_T10.f1;_T12=*_T11;
# 194
*_T22=Cyc_remove_user_prefix(_T12);_TD=(struct _fat_ptr*)_T22;}(*_TC).f1=_TD;goto _LL0;}case 8:{struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*_T22=(struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*)_T1E;_T1F=_T22->f1;}{struct Cyc_Absyn_Typedefdecl*td=_T1F;_T13=td;_T14=_T13->name;{struct _fat_ptr*_T22=_cycalloc(sizeof(struct _fat_ptr));_T16=td;_T17=_T16->name;_T18=*_T17;_T19=_T18.f1;_T1A=*_T19;
# 196
*_T22=Cyc_remove_user_prefix(_T1A);_T15=(struct _fat_ptr*)_T22;}(*_T14).f1=_T15;goto _LL0;}default: _T1B=Cyc_stderr;_T1C=
# 198
_tag_fat("Error in .cys file: bad user-defined type definition\n",sizeof(char),54U);_T1D=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T1B,_T1C,_T1D);
exit(1);}_LL0:;}}
# 205
const int Cyc_lex_base[477U]={0,0,75,192,305,310,311,166,312,91,27,384,28,523,637,715,829,325,92,- 3,0,- 1,- 2,- 8,- 3,1,- 2,323,- 4,2,166,- 5,605,907,312,- 6,- 8,- 4,16,945,- 3,1021,29,11,1135,- 4,13,1249,223,- 14,317,12,- 2,216,20,27,29,34,50,49,71,55,66,101,101,100,108,95,370,386,112,104,105,123,126,375,399,112,134,190,381,1363,1478,414,205,213,233,219,224,224,245,498,1592,1707,- 9,525,- 10,233,253,507,1821,1936,- 7,654,684,- 11,432,514,516,2013,2090,2167,2248,434,465,531,2323,253,253,253,251,247,257,0,13,4,2404,5,628,2412,2477,660,- 4,- 3,49,467,6,2438,7,701,2500,2538,793,- 25,1161,1166,273,752,253,253,270,262,276,286,284,289,283,297,300,- 22,286,290,296,306,315,299,- 24,308,314,311,324,331,333,- 18,313,329,366,395,407,402,404,420,423,409,406,412,436,447,- 20,438,437,431,444,440,456,434,456,459,448,453,444,444,- 23,446,450,460,484,466,468,484,491,492,4,6,21,519,520,507,506,519,548,556,557,24,570,571,24,20,573,590,53,646,648,651,597,600,617,597,588,589,587,621,609,605,610,632,616,630,622,618,640,620,628,634,629,633,649,650,705,716,- 17,648,649,643,653,654,678,679,1,735,735,737,765,708,708,716,743,2,799,757,759,1078,782,783,785,786,749,747,762,769,770,827,828,829,- 15,776,777,832,833,834,792,793,848,849,850,- 12,797,825,880,881,882,830,858,924,925,926,- 13,856,855,869,879,882,864,880,882,888,889,944,945,- 16,1275,890,891,888,887,887,892,926,929,930,928,942,1254,960,971,969,982,1368,1280,985,986,976,1004,1002,1015,1480,1007,1009,1036,1050,1485,- 7,- 8,8,1373,2570,9,1103,2594,2632,1451,1389,- 49,1260,- 2,1099,- 4,1101,1134,1365,1102,1121,1216,1558,1104,2659,2702,1108,1166,1107,1116,2772,1108,1218,- 36,- 42,- 37,2847,- 28,1112,- 40,1119,- 27,- 45,- 39,- 48,2922,2951,1577,1114,1200,1673,2961,2991,1692,1787,3024,3055,3093,1224,1235,3163,3201,1228,1314,1307,1317,1309,1351,- 6,- 34,1135,3133,- 47,- 30,- 32,- 46,- 29,- 31,- 33,1155,3241,1157,1190,1806,1227,1229,1234,1235,1243,1244,1248,1252,1256,3314,3398,- 21,1908,1257,- 19,- 41,- 38,- 35,- 26,1504,3480,1278,3563,1299,15,1280,1284,1285,1283,1281,1295,1365};
const int Cyc_lex_backtrk[477U]={- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,2,- 1,- 1,- 1,- 1,2,- 1,8,- 1,3,5,- 1,- 1,6,5,- 1,- 1,- 1,7,6,- 1,6,5,2,0,- 1,- 1,0,2,- 1,12,13,- 1,13,13,13,13,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,11,12,2,4,4,- 1,0,0,0,2,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,2,2,8,3,5,- 1,6,5,- 1,- 1,6,5,2,8,3,5,- 1,6,5,- 1,24,24,24,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,18,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,20,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,6,1,9,2,4,- 1,5,4,- 1,- 1,2,- 1,48,- 1,48,48,48,48,48,48,48,48,5,7,48,48,48,48,0,48,48,- 1,- 1,- 1,0,- 1,43,- 1,42,- 1,- 1,- 1,- 1,9,7,- 1,7,7,- 1,8,9,- 1,- 1,9,5,6,5,5,- 1,4,4,4,6,6,5,5,- 1,- 1,- 1,9,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,2,- 1,1,2,1,1,- 1,- 1,- 1,- 1,- 1,- 1,- 1};
const int Cyc_lex_default[477U]={- 1,- 1,- 1,372,361,143,23,102,23,19,- 1,- 1,12,31,49,35,36,23,19,0,- 1,0,0,0,0,- 1,0,- 1,0,- 1,- 1,0,- 1,- 1,- 1,0,0,0,- 1,- 1,0,- 1,42,- 1,- 1,0,- 1,- 1,- 1,0,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,0,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,0,106,- 1,- 1,- 1,- 1,- 1,113,113,113,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,0,- 1,135,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,270,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,0,- 1,0,- 1,- 1,440,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,0,0,- 1,0,- 1,0,- 1,0,0,0,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,0,- 1,- 1,0,0,0,0,0,0,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,- 1,- 1,0,0,0,0,0,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1};
const int Cyc_lex_trans[3820U]={0,0,0,0,0,0,0,0,0,0,22,19,28,469,19,28,19,28,36,19,48,48,46,46,48,22,46,0,0,0,0,0,21,269,278,470,213,22,- 1,- 1,22,- 1,- 1,48,214,46,233,22,467,467,467,467,467,467,467,467,467,467,31,106,22,215,117,42,224,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,31,227,228,231,467,135,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,465,465,465,465,465,465,465,465,465,465,124,20,77,22,70,57,58,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,59,60,61,62,465,63,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,64,65,132,373,374,373,373,374,133,22,66,67,68,71,72,134,34,34,34,34,34,34,34,34,73,74,373,375,376,75,78,377,378,379,48,48,380,381,48,382,383,384,385,386,386,386,386,386,386,386,386,386,387,79,388,389,390,48,19,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,19,- 1,- 1,392,391,80,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,362,393,104,363,144,144,24,24,144,136,125,107,107,84,97,107,85,87,28,88,24,29,86,25,364,89,90,144,91,22,26,26,21,21,107,98,99,145,118,119,120,121,122,123,26,35,35,35,35,35,35,35,35,147,203,189,30,30,30,30,30,30,30,30,69,69,183,174,69,76,76,167,160,76,155,81,81,156,157,81,69,69,365,158,69,159,161,69,137,126,146,162,76,76,76,163,164,76,81,165,166,168,27,69,169,31,170,21,83,83,171,172,83,173,175,176,76,116,116,116,116,116,116,116,116,116,116,- 1,32,- 1,- 1,83,- 1,22,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,- 1,177,- 1,- 1,116,- 1,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,92,92,28,178,92,179,180,181,182,100,100,36,184,100,185,19,107,107,115,115,107,186,115,92,187,108,108,95,95,108,35,95,100,115,115,188,190,115,191,107,192,115,193,194,195,196,197,198,108,199,95,200,201,202,94,204,115,205,206,21,21,21,109,110,109,109,109,109,109,109,109,109,109,109,21,207,208,209,210,211,212,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,216,217,218,219,109,220,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,50,50,96,221,50,222,223,33,33,33,33,33,33,33,33,33,33,103,103,225,226,103,229,50,33,33,33,33,33,33,131,131,131,131,131,131,131,131,51,230,103,232,- 1,49,- 1,234,235,104,104,236,52,104,317,273,261,241,33,33,33,33,33,33,35,35,35,35,35,35,35,35,104,237,242,243,244,- 1,245,- 1,43,43,238,239,43,246,247,248,249,240,250,251,53,252,253,254,255,54,55,256,257,258,259,43,56,142,142,142,142,142,142,142,142,260,262,263,264,265,266,44,44,44,44,44,44,44,44,44,44,267,268,270,271,36,272,28,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,94,105,274,275,44,276,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,277,279,45,35,35,35,35,35,35,35,35,148,149,150,280,151,281,311,306,152,300,295,287,288,289,37,290,291,153,154,292,293,294,296,297,298,299,105,38,39,39,39,39,39,39,39,39,39,39,301,302,303,304,305,307,21,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,308,309,310,96,39,312,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,40,313,22,33,33,33,33,33,33,33,33,33,33,314,315,316,318,319,320,31,33,33,33,33,33,33,321,322,323,324,325,326,327,328,329,343,338,334,335,- 1,336,41,41,41,41,41,41,41,41,41,41,337,33,33,33,33,33,33,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,102,339,340,341,41,342,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,344,41,41,41,41,41,41,41,41,41,41,345,346,347,356,351,352,21,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,353,354,355,357,41,358,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,46,46,282,359,46,360,283,370,370,370,370,370,370,370,370,284,463,285,462,439,437,430,405,46,143,396,348,348,460,400,348,330,330,398,399,330,404,433,438,47,47,47,47,47,47,47,47,47,47,348,166,461,188,286,330,36,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,401,402,403,459,47,36,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,46,46,349,434,46,342,342,331,456,342,173,464,350,464,464,105,305,332,36,435,436,394,333,46,316,49,330,330,342,294,330,348,348,329,464,348,19,260,159,47,47,47,47,47,47,47,47,47,47,330,36,35,21,35,348,31,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,21,35,395,35,47,31,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,81,81,331,- 1,81,347,347,349,471,347,371,371,332,472,371,473,474,350,31,475,102,102,429,81,476,21,371,371,347,0,371,0,- 1,371,0,21,31,0,0,82,82,82,82,82,82,82,82,82,82,371,31,21,102,102,429,429,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,0,0,0,441,82,429,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,31,83,83,355,355,83,0,355,360,360,0,0,360,31,31,31,31,31,31,31,31,0,0,0,83,0,355,464,0,464,464,360,0,0,28,0,0,0,0,35,82,82,82,82,82,82,82,82,82,82,464,0,0,0,0,0,0,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,0,0,0,0,82,0,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,92,92,0,431,92,432,432,432,432,432,432,432,432,432,432,0,0,0,0,411,- 1,411,0,92,412,412,412,412,412,412,412,412,412,412,0,0,0,0,0,93,93,93,93,93,93,93,93,93,93,0,0,0,0,0,0,0,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,0,0,0,0,93,0,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,94,95,95,0,0,95,412,412,412,412,412,412,412,412,412,412,0,0,0,0,415,0,415,0,95,416,416,416,416,416,416,416,416,416,416,0,0,0,0,0,93,93,93,93,93,93,93,93,93,93,0,0,0,0,0,0,0,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,0,0,0,0,93,0,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,100,100,0,0,100,416,416,416,416,416,416,416,416,416,416,159,0,0,0,0,0,0,0,100,457,457,457,457,457,457,457,457,0,0,0,0,0,0,0,101,101,101,101,101,101,101,101,101,101,0,0,0,0,0,0,0,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,0,0,0,0,101,0,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,102,103,103,159,0,103,0,0,0,0,0,0,458,458,458,458,458,458,458,458,0,0,0,0,103,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,101,101,101,101,101,101,101,101,101,101,0,0,0,0,0,0,0,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,0,0,0,0,101,0,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,111,111,111,111,111,111,111,111,111,111,111,111,22,0,0,0,0,0,0,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,0,0,0,0,111,0,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,112,111,111,111,111,111,111,111,111,111,111,22,0,0,0,0,0,0,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,0,0,0,0,111,0,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,22,0,0,0,0,0,0,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,- 1,0,0,- 1,111,0,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,0,0,0,0,112,112,112,112,112,112,112,112,112,112,112,112,114,0,0,0,0,0,0,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,0,0,0,0,112,0,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,116,116,116,116,116,116,116,116,116,116,0,0,0,0,0,0,0,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,28,0,0,127,116,0,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,0,0,28,0,0,138,128,128,128,128,128,128,128,128,130,130,130,130,130,130,130,130,130,130,0,0,0,0,0,0,0,130,130,130,130,130,130,0,0,0,139,139,139,139,139,139,139,139,0,0,0,0,0,0,0,31,0,0,- 1,0,0,0,0,130,130,130,130,130,130,0,0,0,0,0,0,0,0,0,129,130,130,130,130,130,130,130,130,130,130,31,0,0,0,0,0,0,130,130,130,130,130,130,141,141,141,141,141,141,141,141,141,141,140,0,0,0,0,0,0,141,141,141,141,141,141,0,0,0,130,130,130,130,130,130,19,0,0,366,0,0,141,141,141,141,141,141,141,141,141,141,0,141,141,141,141,141,141,141,141,141,141,141,141,0,0,0,0,0,0,0,0,0,367,367,367,367,367,367,367,367,0,0,0,0,0,0,0,0,0,141,141,141,141,141,141,0,369,369,369,369,369,369,369,369,369,369,0,0,0,0,0,0,0,369,369,369,369,369,369,0,0,28,0,0,0,0,0,0,0,0,0,0,0,0,369,369,369,369,369,369,369,369,369,369,368,369,369,369,369,369,369,369,369,369,369,369,369,0,0,406,0,417,417,417,417,417,417,417,417,418,418,0,0,0,0,0,0,0,0,0,0,0,408,369,369,369,369,369,369,419,0,0,0,0,0,0,0,0,420,0,0,421,406,0,407,407,407,407,407,407,407,407,407,407,408,0,0,0,0,0,0,419,0,0,0,408,0,0,0,0,420,0,409,421,0,0,0,0,0,0,0,410,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,408,0,0,0,0,0,0,409,0,0,0,0,0,0,0,0,410,397,397,397,397,397,397,397,397,397,397,0,0,0,0,0,0,0,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,0,0,0,0,397,0,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,0,0,0,0,0,0,0,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,0,0,0,0,397,0,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,413,413,413,413,413,413,413,413,413,413,0,0,0,0,0,0,0,0,0,0,0,414,96,0,0,0,0,406,96,407,407,407,407,407,407,407,407,407,407,412,412,412,412,412,412,412,412,412,412,0,408,0,0,414,96,0,0,409,0,0,96,94,0,0,0,0,410,94,0,413,413,413,413,413,413,413,413,413,413,0,0,0,408,0,0,0,0,0,0,409,414,96,0,94,0,0,0,96,410,94,0,0,416,416,416,416,416,416,416,416,416,416,0,0,0,0,0,0,0,0,0,0,414,96,96,0,0,0,0,96,96,406,0,417,417,417,417,417,417,417,417,418,418,0,0,0,0,0,0,0,0,0,0,0,408,0,96,0,0,0,0,427,96,0,0,0,0,0,0,406,428,418,418,418,418,418,418,418,418,418,418,0,0,0,0,0,408,0,0,0,0,0,408,427,0,0,0,0,0,425,0,0,428,0,0,0,0,0,426,0,0,432,432,432,432,432,432,432,432,432,432,0,0,0,408,0,0,0,0,0,0,425,414,96,0,0,0,0,0,96,426,422,422,422,422,422,422,422,422,422,422,0,0,0,0,0,0,0,422,422,422,422,422,422,414,96,0,0,0,0,0,96,0,0,0,0,0,0,0,422,422,422,422,422,422,422,422,422,422,0,422,422,422,422,422,422,422,422,422,422,422,422,0,0,0,442,0,423,0,0,443,0,0,0,0,0,424,0,0,444,444,444,444,444,444,444,444,0,422,422,422,422,422,422,445,0,0,0,0,423,0,0,0,0,0,0,0,0,424,0,0,0,0,0,0,0,0,0,0,0,0,0,0,446,0,0,0,0,447,448,0,0,0,449,0,0,0,0,0,0,0,450,0,0,0,451,0,452,0,453,0,454,455,455,455,455,455,455,455,455,455,455,0,0,0,0,0,0,0,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,0,0,0,0,0,0,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,202,0,0,0,0,0,0,0,0,455,455,455,455,455,455,455,455,455,455,0,0,0,0,0,0,0,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,0,0,0,0,0,0,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,19,0,0,466,0,0,0,465,465,465,465,465,465,465,465,465,465,0,0,0,0,0,0,0,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,0,0,0,0,465,0,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,468,0,0,0,0,0,0,0,467,467,467,467,467,467,467,467,467,467,0,0,0,0,0,0,0,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,0,0,0,0,467,0,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const int Cyc_lex_check[3820U]={- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,0,25,29,0,125,127,136,138,363,366,43,43,46,46,43,469,46,- 1,- 1,- 1,- 1,- 1,123,268,277,0,212,10,12,42,10,12,42,43,212,46,213,20,1,1,1,1,1,1,1,1,1,1,38,51,124,214,10,38,223,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,134,226,227,230,1,134,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,9,18,54,46,55,56,57,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,58,59,60,61,2,62,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,63,64,7,3,3,3,3,3,7,7,65,66,67,70,71,7,30,30,30,30,30,30,30,30,72,73,3,3,3,74,77,3,3,3,48,48,3,3,48,3,3,3,3,3,3,3,3,3,3,3,3,3,3,78,3,3,3,48,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,10,12,42,3,3,79,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,3,84,4,5,5,6,8,5,6,8,50,50,53,85,50,53,86,27,87,17,27,53,17,4,88,89,5,90,5,6,8,9,18,50,97,98,5,117,118,119,120,121,122,17,34,34,34,34,34,34,34,34,146,148,149,27,27,27,27,27,27,27,27,68,68,150,151,68,75,75,152,153,75,154,80,80,155,156,80,69,69,4,157,69,158,160,68,6,8,5,161,75,76,76,162,163,76,80,164,165,167,17,69,168,27,169,7,83,83,170,171,83,172,174,175,76,11,11,11,11,11,11,11,11,11,11,106,27,113,106,83,113,3,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,114,176,135,114,11,135,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,91,91,69,177,91,178,179,180,181,99,99,182,183,99,184,76,107,107,108,108,107,185,108,91,186,13,13,95,95,13,83,95,99,115,115,187,189,115,190,107,191,108,192,193,194,195,196,197,13,198,95,199,200,201,4,203,115,204,205,5,6,8,13,13,13,13,13,13,13,13,13,13,13,13,17,206,207,208,209,210,211,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,215,216,217,218,13,219,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,95,220,14,221,222,32,32,32,32,32,32,32,32,32,32,103,103,224,225,103,228,14,32,32,32,32,32,32,128,128,128,128,128,128,128,128,14,229,103,231,106,232,113,233,234,104,104,235,14,104,237,238,239,240,32,32,32,32,32,32,131,131,131,131,131,131,131,131,104,236,241,242,243,114,244,135,15,15,236,236,15,245,246,247,248,236,249,250,14,251,252,253,254,14,14,255,256,257,258,15,14,139,139,139,139,139,139,139,139,259,261,262,263,264,265,15,15,15,15,15,15,15,15,15,15,266,267,269,270,103,271,13,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,272,104,273,274,15,275,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,276,278,15,142,142,142,142,142,142,142,142,147,147,147,279,147,280,282,283,147,284,285,286,287,288,16,289,290,147,147,291,292,293,295,296,297,298,299,16,16,16,16,16,16,16,16,16,16,16,300,301,302,303,304,306,14,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,307,308,309,310,16,311,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,312,16,33,33,33,33,33,33,33,33,33,33,313,314,315,317,318,319,15,33,33,33,33,33,33,320,321,322,323,324,325,326,327,328,331,332,333,334,270,335,39,39,39,39,39,39,39,39,39,39,336,33,33,33,33,33,33,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,337,338,339,340,39,341,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,39,343,41,41,41,41,41,41,41,41,41,41,344,345,346,349,350,351,16,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,352,353,354,356,41,357,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,41,44,44,281,358,44,359,281,367,367,367,367,367,367,367,367,281,375,281,377,380,381,384,387,44,389,392,144,144,378,399,144,145,145,390,390,145,401,431,381,44,44,44,44,44,44,44,44,44,44,144,440,378,442,281,145,409,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,388,388,388,443,44,409,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,44,47,47,144,382,47,342,342,145,445,342,446,373,144,373,373,447,448,145,410,382,382,393,145,47,449,450,330,330,342,451,330,348,348,452,373,348,342,453,458,47,47,47,47,47,47,47,47,47,47,330,410,419,466,420,348,423,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,468,419,393,420,47,423,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,47,81,81,330,379,81,347,347,348,470,347,364,364,330,471,364,472,473,348,424,474,425,426,427,81,475,476,371,371,347,- 1,371,- 1,379,364,- 1,364,347,- 1,- 1,81,81,81,81,81,81,81,81,81,81,371,424,371,425,426,427,428,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,- 1,- 1,- 1,379,81,428,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,82,82,355,355,82,- 1,355,360,360,- 1,- 1,360,370,370,370,370,370,370,370,370,- 1,- 1,- 1,82,- 1,355,464,- 1,464,464,360,- 1,- 1,355,- 1,- 1,- 1,- 1,360,82,82,82,82,82,82,82,82,82,82,464,- 1,- 1,- 1,- 1,- 1,- 1,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,- 1,- 1,- 1,- 1,82,- 1,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,82,92,92,- 1,383,92,383,383,383,383,383,383,383,383,383,383,- 1,- 1,- 1,- 1,408,379,408,- 1,92,408,408,408,408,408,408,408,408,408,408,- 1,- 1,- 1,- 1,- 1,92,92,92,92,92,92,92,92,92,92,- 1,- 1,- 1,- 1,- 1,- 1,- 1,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,- 1,- 1,- 1,- 1,92,- 1,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,92,93,93,- 1,- 1,93,411,411,411,411,411,411,411,411,411,411,- 1,- 1,- 1,- 1,414,- 1,414,- 1,93,414,414,414,414,414,414,414,414,414,414,- 1,- 1,- 1,- 1,- 1,93,93,93,93,93,93,93,93,93,93,- 1,- 1,- 1,- 1,- 1,- 1,- 1,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,- 1,- 1,- 1,- 1,93,- 1,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,93,100,100,- 1,- 1,100,415,415,415,415,415,415,415,415,415,415,444,- 1,- 1,- 1,- 1,- 1,- 1,- 1,100,444,444,444,444,444,444,444,444,- 1,- 1,- 1,- 1,- 1,- 1,- 1,100,100,100,100,100,100,100,100,100,100,- 1,- 1,- 1,- 1,- 1,- 1,- 1,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,- 1,- 1,- 1,- 1,100,- 1,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,101,101,457,- 1,101,- 1,- 1,- 1,- 1,- 1,- 1,457,457,457,457,457,457,457,457,- 1,- 1,- 1,- 1,101,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,101,101,101,101,101,101,101,101,101,101,- 1,- 1,- 1,- 1,- 1,- 1,- 1,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,- 1,- 1,- 1,- 1,101,- 1,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,101,109,109,109,109,109,109,109,109,109,109,109,109,109,- 1,- 1,- 1,- 1,- 1,- 1,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,- 1,- 1,- 1,- 1,109,- 1,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,109,110,110,110,110,110,110,110,110,110,110,110,110,110,- 1,- 1,- 1,- 1,- 1,- 1,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,- 1,- 1,- 1,- 1,110,- 1,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,110,111,111,111,111,111,111,111,111,111,111,111,111,111,- 1,- 1,- 1,- 1,- 1,- 1,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,112,- 1,- 1,112,111,- 1,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,111,- 1,- 1,- 1,- 1,112,112,112,112,112,112,112,112,112,112,112,112,112,- 1,- 1,- 1,- 1,- 1,- 1,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,- 1,- 1,- 1,- 1,112,- 1,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,112,116,116,116,116,116,116,116,116,116,116,- 1,- 1,- 1,- 1,- 1,- 1,- 1,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,126,- 1,- 1,126,116,- 1,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,116,- 1,- 1,137,- 1,- 1,137,126,126,126,126,126,126,126,126,129,129,129,129,129,129,129,129,129,129,- 1,- 1,- 1,- 1,- 1,- 1,- 1,129,129,129,129,129,129,- 1,- 1,- 1,137,137,137,137,137,137,137,137,- 1,- 1,- 1,- 1,- 1,- 1,- 1,126,- 1,- 1,112,- 1,- 1,- 1,- 1,129,129,129,129,129,129,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,126,130,130,130,130,130,130,130,130,130,130,137,- 1,- 1,- 1,- 1,- 1,- 1,130,130,130,130,130,130,140,140,140,140,140,140,140,140,140,140,137,- 1,- 1,- 1,- 1,- 1,- 1,140,140,140,140,140,140,- 1,- 1,- 1,130,130,130,130,130,130,365,- 1,- 1,365,- 1,- 1,141,141,141,141,141,141,141,141,141,141,- 1,140,140,140,140,140,140,141,141,141,141,141,141,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,365,365,365,365,365,365,365,365,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,141,141,141,141,141,141,- 1,368,368,368,368,368,368,368,368,368,368,- 1,- 1,- 1,- 1,- 1,- 1,- 1,368,368,368,368,368,368,- 1,- 1,365,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,369,369,369,369,369,369,369,369,369,369,365,368,368,368,368,368,368,369,369,369,369,369,369,- 1,- 1,385,- 1,385,385,385,385,385,385,385,385,385,385,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,385,369,369,369,369,369,369,385,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,385,- 1,- 1,385,386,- 1,386,386,386,386,386,386,386,386,386,386,385,- 1,- 1,- 1,- 1,- 1,- 1,385,- 1,- 1,- 1,386,- 1,- 1,- 1,- 1,385,- 1,386,385,- 1,- 1,- 1,- 1,- 1,- 1,- 1,386,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,386,- 1,- 1,- 1,- 1,- 1,- 1,386,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,386,391,391,391,391,391,391,391,391,391,391,- 1,- 1,- 1,- 1,- 1,- 1,- 1,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,- 1,- 1,- 1,- 1,391,- 1,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,391,397,397,397,397,397,397,397,397,397,397,- 1,- 1,- 1,- 1,- 1,- 1,- 1,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,- 1,- 1,- 1,- 1,397,- 1,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,397,406,406,406,406,406,406,406,406,406,406,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,406,406,- 1,- 1,- 1,- 1,407,406,407,407,407,407,407,407,407,407,407,407,412,412,412,412,412,412,412,412,412,412,- 1,407,- 1,- 1,406,406,- 1,- 1,407,- 1,- 1,406,412,- 1,- 1,- 1,- 1,407,412,- 1,413,413,413,413,413,413,413,413,413,413,- 1,- 1,- 1,407,- 1,- 1,- 1,- 1,- 1,- 1,407,413,413,- 1,412,- 1,- 1,- 1,413,407,412,- 1,- 1,416,416,416,416,416,416,416,416,416,416,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,413,413,416,- 1,- 1,- 1,- 1,413,416,417,- 1,417,417,417,417,417,417,417,417,417,417,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,417,- 1,416,- 1,- 1,- 1,- 1,417,416,- 1,- 1,- 1,- 1,- 1,- 1,418,417,418,418,418,418,418,418,418,418,418,418,- 1,- 1,- 1,- 1,- 1,417,- 1,- 1,- 1,- 1,- 1,418,417,- 1,- 1,- 1,- 1,- 1,418,- 1,- 1,417,- 1,- 1,- 1,- 1,- 1,418,- 1,- 1,432,432,432,432,432,432,432,432,432,432,- 1,- 1,- 1,418,- 1,- 1,- 1,- 1,- 1,- 1,418,432,432,- 1,- 1,- 1,- 1,- 1,432,418,421,421,421,421,421,421,421,421,421,421,- 1,- 1,- 1,- 1,- 1,- 1,- 1,421,421,421,421,421,421,432,432,- 1,- 1,- 1,- 1,- 1,432,- 1,- 1,- 1,- 1,- 1,- 1,- 1,422,422,422,422,422,422,422,422,422,422,- 1,421,421,421,421,421,421,422,422,422,422,422,422,- 1,- 1,- 1,441,- 1,422,- 1,- 1,441,- 1,- 1,- 1,- 1,- 1,422,- 1,- 1,441,441,441,441,441,441,441,441,- 1,422,422,422,422,422,422,441,- 1,- 1,- 1,- 1,422,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,422,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,441,- 1,- 1,- 1,- 1,441,441,- 1,- 1,- 1,441,- 1,- 1,- 1,- 1,- 1,- 1,- 1,441,- 1,- 1,- 1,441,- 1,441,- 1,441,- 1,441,454,454,454,454,454,454,454,454,454,454,- 1,- 1,- 1,- 1,- 1,- 1,- 1,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,- 1,- 1,- 1,- 1,- 1,- 1,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,454,455,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,455,455,455,455,455,455,455,455,455,455,- 1,- 1,- 1,- 1,- 1,- 1,- 1,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,- 1,- 1,- 1,- 1,- 1,- 1,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,455,465,- 1,- 1,465,- 1,- 1,- 1,465,465,465,465,465,465,465,465,465,465,- 1,- 1,- 1,- 1,- 1,- 1,- 1,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,- 1,- 1,- 1,- 1,465,- 1,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,465,467,- 1,- 1,- 1,- 1,- 1,- 1,- 1,467,467,467,467,467,467,467,467,467,467,- 1,- 1,- 1,- 1,- 1,- 1,- 1,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,- 1,- 1,- 1,- 1,467,- 1,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,467,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1,- 1};
int Cyc_lex_engine(int start_state,struct Cyc_Lexing_lexbuf*lbuf){struct Cyc_Lexing_lexbuf*_T0;struct Cyc_Lexing_lexbuf*_T1;struct Cyc_Lexing_lexbuf*_T2;struct Cyc_Lexing_lexbuf*_T3;int _T4;const int*_T5;int _T6;const char*_T7;const int*_T8;int _T9;int _TA;const int*_TB;int _TC;const char*_TD;const int*_TE;struct Cyc_Lexing_lexbuf*_TF;struct Cyc_Lexing_lexbuf*_T10;struct Cyc_Lexing_lexbuf*_T11;struct Cyc_Lexing_lexbuf*_T12;int _T13;struct Cyc_Lexing_lexbuf*_T14;int _T15;struct Cyc_Lexing_lexbuf*_T16;int _T17;int _T18;int _T19;struct Cyc_Lexing_lexbuf*_T1A;struct _fat_ptr _T1B;struct Cyc_Lexing_lexbuf*_T1C;int _T1D;int _T1E;char*_T1F;char*_T20;char _T21;const int*_T22;int _T23;const char*_T24;const int*_T25;int _T26;int _T27;const int*_T28;int _T29;const char*_T2A;const int*_T2B;const int*_T2C;int _T2D;const char*_T2E;const int*_T2F;struct Cyc_Lexing_lexbuf*_T30;struct Cyc_Lexing_lexbuf*_T31;struct Cyc_Lexing_lexbuf*_T32;int _T33;struct Cyc_Lexing_Error_exn_struct*_T34;void*_T35;struct Cyc_Lexing_lexbuf*_T36;int _T37;struct Cyc_Lexing_lexbuf*_T38;
# 212
int state;int base;int backtrk;
int c;
state=start_state;
# 216
if(state < 0)goto _TL5;_T0=lbuf;_T1=lbuf;_T2=lbuf;
_T1->lex_start_pos=_T2->lex_curr_pos;_T0->lex_last_pos=_T1->lex_start_pos;_T3=lbuf;
_T3->lex_last_action=-1;goto _TL6;
# 220
_TL5: _T4=- state;state=_T4 - 1;_TL6:
# 222
 _TL7: if(1)goto _TL8;else{goto _TL9;}
_TL8: _T5=Cyc_lex_base;_T6=state;_T7=_check_known_subscript_notnull(_T5,477U,sizeof(int),_T6);_T8=(const int*)_T7;base=*_T8;
if(base >= 0)goto _TLA;_T9=- base;_TA=_T9 - 1;return _TA;_TLA: _TB=Cyc_lex_backtrk;_TC=state;_TD=_check_known_subscript_notnull(_TB,477U,sizeof(int),_TC);_TE=(const int*)_TD;
backtrk=*_TE;
if(backtrk < 0)goto _TLC;_TF=lbuf;_T10=lbuf;
_TF->lex_last_pos=_T10->lex_curr_pos;_T11=lbuf;
_T11->lex_last_action=backtrk;goto _TLD;_TLC: _TLD: _T12=lbuf;_T13=_T12->lex_curr_pos;_T14=lbuf;_T15=_T14->lex_buffer_len;
# 230
if(_T13 < _T15)goto _TLE;_T16=lbuf;_T17=_T16->lex_eof_reached;
if(_T17)goto _TL10;else{goto _TL12;}
_TL12: _T18=- state;_T19=_T18 - 1;return _T19;
# 234
_TL10: c=256;goto _TLF;
# 236
_TLE: _T1A=lbuf;_T1B=_T1A->lex_buffer;_T1C=lbuf;_T1D=_T1C->lex_curr_pos;_T1C->lex_curr_pos=_T1D + 1;_T1E=_T1D;_T1F=_check_fat_subscript(_T1B,sizeof(char),_T1E);_T20=(char*)_T1F;_T21=*_T20;c=(int)_T21;
if(c!=-1)goto _TL13;c=256;goto _TL14;
_TL13: if(c >= 0)goto _TL15;c=256 + c;goto _TL16;_TL15: _TL16: _TL14: _TLF: _T22=Cyc_lex_check;_T23=base + c;_T24=_check_known_subscript_notnull(_T22,3820U,sizeof(int),_T23);_T25=(const int*)_T24;_T26=*_T25;_T27=state;
# 240
if(_T26!=_T27)goto _TL17;_T28=Cyc_lex_trans;_T29=base + c;_T2A=_check_known_subscript_notnull(_T28,3820U,sizeof(int),_T29);_T2B=(const int*)_T2A;
state=*_T2B;goto _TL18;
# 243
_TL17: _T2C=Cyc_lex_default;_T2D=state;_T2E=_check_known_subscript_notnull(_T2C,477U,sizeof(int),_T2D);_T2F=(const int*)_T2E;state=*_T2F;_TL18:
 if(state >= 0)goto _TL19;_T30=lbuf;_T31=lbuf;
_T30->lex_curr_pos=_T31->lex_last_pos;_T32=lbuf;_T33=_T32->lex_last_action;
if(_T33!=-1)goto _TL1B;{struct Cyc_Lexing_Error_exn_struct*_T39=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T39->tag=Cyc_Lexing_Error;
_T39->f1=_tag_fat("empty token",sizeof(char),12U);_T34=(struct Cyc_Lexing_Error_exn_struct*)_T39;}_T35=(void*)_T34;_throw(_T35);goto _TL1C;
# 249
_TL1B: _T36=lbuf;_T37=_T36->lex_last_action;return _T37;_TL1C: goto _TL1A;
# 252
_TL19: if(c!=256)goto _TL1D;_T38=lbuf;_T38->lex_eof_reached=0;goto _TL1E;_TL1D: _TL1E: _TL1A: goto _TL7;_TL9:;}
# 256
struct _tuple13*Cyc_line_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct Cyc_Set_Set**_T2;struct Cyc_Set_Set*(*_T3)(struct Cyc_Set_Set*,struct _fat_ptr*);struct Cyc_Set_Set*(*_T4)(struct Cyc_Set_Set*,void*);struct Cyc_Set_Set**_T5;struct Cyc_Set_Set*_T6;struct Cyc_List_List*_T7;void*_T8;struct _fat_ptr*_T9;struct Cyc_List_List*_TA;struct _tuple13*_TB;struct Cyc_Set_Set**_TC;struct _tuple13*_TD;struct Cyc_Lexing_lexbuf*_TE;void(*_TF)(struct Cyc_Lexing_lexbuf*);struct _tuple13*_T10;struct Cyc_Lexing_Error_exn_struct*_T11;void*_T12;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:
# 214 "buildlib.cyl"
 Cyc_macroname(lexbuf);
_TL23: if(Cyc_current_args!=0)goto _TL21;else{goto _TL22;}
_TL21:{struct Cyc_Set_Set**_T13=_cycalloc(sizeof(struct Cyc_Set_Set*));_T4=Cyc_Set_delete;{struct Cyc_Set_Set*(*_T14)(struct Cyc_Set_Set*,struct _fat_ptr*)=(struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T4;_T3=_T14;}_T5=_check_null(Cyc_current_targets);_T6=*_T5;_T7=Cyc_current_args;_T8=_T7->hd;_T9=(struct _fat_ptr*)_T8;*_T13=_T3(_T6,_T9);_T2=(struct Cyc_Set_Set**)_T13;}Cyc_current_targets=_T2;_TA=
# 215
_check_null(Cyc_current_args);Cyc_current_args=_TA->tl;goto _TL23;_TL22:{struct _tuple13*_T13=_cycalloc(sizeof(struct _tuple13));
# 219
_T13->f0=_check_null(Cyc_current_source);_TC=_check_null(Cyc_current_targets);_T13->f1=*_TC;_TB=(struct _tuple13*)_T13;}return _TB;case 1: _TD=
# 222 "buildlib.cyl"
Cyc_line(lexbuf);return _TD;case 2:
# 224
 return 0;default: _TE=lexbuf;_TF=_TE->refill_buff;
_TF(lexbuf);_T10=
Cyc_line_rec(lexbuf,lexstate);return _T10;}{struct Cyc_Lexing_Error_exn_struct*_T13=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T13->tag=Cyc_Lexing_Error;
# 228
_T13->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T11=(struct Cyc_Lexing_Error_exn_struct*)_T13;}_T12=(void*)_T11;_throw(_T12);}
# 230
struct _tuple13*Cyc_line(struct Cyc_Lexing_lexbuf*lexbuf){struct _tuple13*_T0;_T0=Cyc_line_rec(lexbuf,0);return _T0;}
int Cyc_macroname_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _fat_ptr*_T2;struct _fat_ptr _T3;int _T4;int _T5;int _T6;int _T7;unsigned long _T8;struct _fat_ptr _T9;struct Cyc_Set_Set**_TA;struct Cyc_Set_Set*(*_TB)(int(*)(struct _fat_ptr*,struct _fat_ptr*));struct Cyc_Set_Set*(*_TC)(int(*)(void*,void*));int(*_TD)(struct _fat_ptr*,struct _fat_ptr*);struct _fat_ptr*_TE;struct _fat_ptr _TF;int _T10;int _T11;int _T12;int _T13;unsigned long _T14;struct _fat_ptr _T15;struct Cyc_Set_Set**_T16;struct Cyc_Set_Set*(*_T17)(int(*)(struct _fat_ptr*,struct _fat_ptr*));struct Cyc_Set_Set*(*_T18)(int(*)(void*,void*));int(*_T19)(struct _fat_ptr*,struct _fat_ptr*);struct _fat_ptr*_T1A;struct _fat_ptr _T1B;struct Cyc_Set_Set**_T1C;struct Cyc_Set_Set*(*_T1D)(int(*)(struct _fat_ptr*,struct _fat_ptr*));struct Cyc_Set_Set*(*_T1E)(int(*)(void*,void*));int(*_T1F)(struct _fat_ptr*,struct _fat_ptr*);struct Cyc_Lexing_lexbuf*_T20;void(*_T21)(struct Cyc_Lexing_lexbuf*);int _T22;struct Cyc_Lexing_Error_exn_struct*_T23;void*_T24;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:{struct _fat_ptr*_T25=_cycalloc(sizeof(struct _fat_ptr));_T3=
# 228 "buildlib.cyl"
Cyc_Lexing_lexeme(lexbuf);_T4=
Cyc_Lexing_lexeme_end(lexbuf);_T5=Cyc_Lexing_lexeme_start(lexbuf);_T6=_T4 - _T5;_T7=_T6 - 2;_T8=(unsigned long)_T7;_T9=
# 228
Cyc_substring(_T3,0,_T8);*_T25=_T9;_T2=(struct _fat_ptr*)_T25;}Cyc_current_source=_T2;
# 230
Cyc_current_args=0;{struct Cyc_Set_Set**_T25=_cycalloc(sizeof(struct Cyc_Set_Set*));_TC=Cyc_Set_empty;{
struct Cyc_Set_Set*(*_T26)(int(*)(struct _fat_ptr*,struct _fat_ptr*))=(struct Cyc_Set_Set*(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*)))_TC;_TB=_T26;}_TD=Cyc_strptrcmp;*_T25=_TB(_TD);_TA=(struct Cyc_Set_Set**)_T25;}Cyc_current_targets=_TA;
Cyc_token(lexbuf);
return 0;case 1:{struct _fat_ptr*_T25=_cycalloc(sizeof(struct _fat_ptr));_TF=
# 236
Cyc_Lexing_lexeme(lexbuf);_T10=
Cyc_Lexing_lexeme_end(lexbuf);_T11=Cyc_Lexing_lexeme_start(lexbuf);_T12=_T10 - _T11;_T13=_T12 - 1;_T14=(unsigned long)_T13;_T15=
# 236
Cyc_substring(_TF,0,_T14);*_T25=_T15;_TE=(struct _fat_ptr*)_T25;}Cyc_current_source=_TE;
# 238
Cyc_current_args=0;{struct Cyc_Set_Set**_T25=_cycalloc(sizeof(struct Cyc_Set_Set*));_T18=Cyc_Set_empty;{
struct Cyc_Set_Set*(*_T26)(int(*)(struct _fat_ptr*,struct _fat_ptr*))=(struct Cyc_Set_Set*(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*)))_T18;_T17=_T26;}_T19=Cyc_strptrcmp;*_T25=_T17(_T19);_T16=(struct Cyc_Set_Set**)_T25;}Cyc_current_targets=_T16;
Cyc_args(lexbuf);
return 0;case 2:{struct _fat_ptr*_T25=_cycalloc(sizeof(struct _fat_ptr));_T1B=
# 244
Cyc_Lexing_lexeme(lexbuf);*_T25=_T1B;_T1A=(struct _fat_ptr*)_T25;}Cyc_current_source=_T1A;
Cyc_current_args=0;{struct Cyc_Set_Set**_T25=_cycalloc(sizeof(struct Cyc_Set_Set*));_T1E=Cyc_Set_empty;{
struct Cyc_Set_Set*(*_T26)(int(*)(struct _fat_ptr*,struct _fat_ptr*))=(struct Cyc_Set_Set*(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*)))_T1E;_T1D=_T26;}_T1F=Cyc_strptrcmp;*_T25=_T1D(_T1F);_T1C=(struct Cyc_Set_Set**)_T25;}Cyc_current_targets=_T1C;
Cyc_token(lexbuf);
return 0;default: _T20=lexbuf;_T21=_T20->refill_buff;
# 250
_T21(lexbuf);_T22=
Cyc_macroname_rec(lexbuf,lexstate);return _T22;}{struct Cyc_Lexing_Error_exn_struct*_T25=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T25->tag=Cyc_Lexing_Error;
# 253
_T25->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T23=(struct Cyc_Lexing_Error_exn_struct*)_T25;}_T24=(void*)_T23;_throw(_T24);}
# 255
int Cyc_macroname(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_macroname_rec(lexbuf,1);return _T0;}
int Cyc_args_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _fat_ptr*_T2;struct _fat_ptr _T3;int _T4;int _T5;int _T6;int _T7;unsigned long _T8;struct _fat_ptr _T9;struct Cyc_List_List*_TA;int _TB;struct _fat_ptr*_TC;struct _fat_ptr _TD;int _TE;int _TF;int _T10;int _T11;unsigned long _T12;struct _fat_ptr _T13;struct Cyc_List_List*_T14;int _T15;struct _fat_ptr*_T16;struct _fat_ptr _T17;int _T18;int _T19;int _T1A;int _T1B;unsigned long _T1C;struct _fat_ptr _T1D;struct Cyc_List_List*_T1E;int _T1F;struct Cyc_Lexing_lexbuf*_T20;void(*_T21)(struct Cyc_Lexing_lexbuf*);int _T22;struct Cyc_Lexing_Error_exn_struct*_T23;void*_T24;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:  {
# 253 "buildlib.cyl"
struct _fat_ptr*a;a=_cycalloc(sizeof(struct _fat_ptr));_T2=a;_T3=Cyc_Lexing_lexeme(lexbuf);_T4=
Cyc_Lexing_lexeme_end(lexbuf);_T5=Cyc_Lexing_lexeme_start(lexbuf);_T6=_T4 - _T5;_T7=_T6 - 2;_T8=(unsigned long)_T7;_T9=
# 253
Cyc_substring(_T3,0,_T8);*_T2=_T9;{struct Cyc_List_List*_T25=_cycalloc(sizeof(struct Cyc_List_List));
# 255
_T25->hd=a;_T25->tl=Cyc_current_args;_TA=(struct Cyc_List_List*)_T25;}Cyc_current_args=_TA;_TB=
Cyc_args(lexbuf);return _TB;}case 1:  {
# 259
struct _fat_ptr*a;a=_cycalloc(sizeof(struct _fat_ptr));_TC=a;_TD=Cyc_Lexing_lexeme(lexbuf);_TE=
Cyc_Lexing_lexeme_end(lexbuf);_TF=Cyc_Lexing_lexeme_start(lexbuf);_T10=_TE - _TF;_T11=_T10 - 1;_T12=(unsigned long)_T11;_T13=
# 259
Cyc_substring(_TD,0,_T12);*_TC=_T13;{struct Cyc_List_List*_T25=_cycalloc(sizeof(struct Cyc_List_List));
# 261
_T25->hd=a;_T25->tl=Cyc_current_args;_T14=(struct Cyc_List_List*)_T25;}Cyc_current_args=_T14;_T15=
Cyc_args(lexbuf);return _T15;}case 2:  {
# 265
struct _fat_ptr*a;a=_cycalloc(sizeof(struct _fat_ptr));_T16=a;_T17=Cyc_Lexing_lexeme(lexbuf);_T18=
Cyc_Lexing_lexeme_end(lexbuf);_T19=Cyc_Lexing_lexeme_start(lexbuf);_T1A=_T18 - _T19;_T1B=_T1A - 1;_T1C=(unsigned long)_T1B;_T1D=
# 265
Cyc_substring(_T17,0,_T1C);*_T16=_T1D;{struct Cyc_List_List*_T25=_cycalloc(sizeof(struct Cyc_List_List));
# 267
_T25->hd=a;_T25->tl=Cyc_current_args;_T1E=(struct Cyc_List_List*)_T25;}Cyc_current_args=_T1E;_T1F=
Cyc_token(lexbuf);return _T1F;}default: _T20=lexbuf;_T21=_T20->refill_buff;
# 270
_T21(lexbuf);_T22=
Cyc_args_rec(lexbuf,lexstate);return _T22;}{struct Cyc_Lexing_Error_exn_struct*_T25=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T25->tag=Cyc_Lexing_Error;
# 273
_T25->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T23=(struct Cyc_Lexing_Error_exn_struct*)_T25;}_T24=(void*)_T23;_throw(_T24);}
# 275
int Cyc_args(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_args_rec(lexbuf,2);return _T0;}
int Cyc_token_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _fat_ptr*_T2;struct _fat_ptr _T3;int _T4;int _T5;int _T6;int _T7;int _T8;int _T9;int _TA;int _TB;int _TC;int _TD;int _TE;int _TF;int _T10;int _T11;int _T12;int _T13;int _T14;int _T15;int _T16;int _T17;int _T18;int _T19;int _T1A;int _T1B;int _T1C;int _T1D;int _T1E;int _T1F;int _T20;int _T21;int _T22;int _T23;int _T24;int _T25;int _T26;int _T27;int _T28;int _T29;int _T2A;int _T2B;int _T2C;int _T2D;int _T2E;int _T2F;int _T30;int _T31;int _T32;int _T33;struct Cyc_Lexing_lexbuf*_T34;void(*_T35)(struct Cyc_Lexing_lexbuf*);int _T36;struct Cyc_Lexing_Error_exn_struct*_T37;void*_T38;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:{struct _fat_ptr*_T39=_cycalloc(sizeof(struct _fat_ptr));_T3=
# 274 "buildlib.cyl"
Cyc_Lexing_lexeme(lexbuf);*_T39=_T3;_T2=(struct _fat_ptr*)_T39;}Cyc_add_target(_T2);_T4=Cyc_token(lexbuf);return _T4;case 1:
# 277 "buildlib.cyl"
 return 0;case 2: _T5=
# 280 "buildlib.cyl"
Cyc_token(lexbuf);return _T5;case 3:
# 283 "buildlib.cyl"
 Cyc_string(lexbuf);_T6=Cyc_token(lexbuf);return _T6;case 4: _T7=
# 286 "buildlib.cyl"
Cyc_token(lexbuf);return _T7;case 5: _T8=
# 288
Cyc_token(lexbuf);return _T8;case 6: _T9=
# 290
Cyc_token(lexbuf);return _T9;case 7: _TA=
# 292
Cyc_token(lexbuf);return _TA;case 8: _TB=
# 295 "buildlib.cyl"
Cyc_token(lexbuf);return _TB;case 9: _TC=
# 298 "buildlib.cyl"
Cyc_token(lexbuf);return _TC;case 10: _TD=
# 301 "buildlib.cyl"
Cyc_token(lexbuf);return _TD;case 11: _TE=
# 303
Cyc_token(lexbuf);return _TE;case 12: _TF=
# 305
Cyc_token(lexbuf);return _TF;case 13: _T10=
# 307
Cyc_token(lexbuf);return _T10;case 14: _T11=
# 309
Cyc_token(lexbuf);return _T11;case 15: _T12=
# 311
Cyc_token(lexbuf);return _T12;case 16: _T13=
# 313
Cyc_token(lexbuf);return _T13;case 17: _T14=
# 315
Cyc_token(lexbuf);return _T14;case 18: _T15=
# 317
Cyc_token(lexbuf);return _T15;case 19: _T16=
# 319
Cyc_token(lexbuf);return _T16;case 20: _T17=
# 321
Cyc_token(lexbuf);return _T17;case 21: _T18=
# 323
Cyc_token(lexbuf);return _T18;case 22: _T19=
# 325
Cyc_token(lexbuf);return _T19;case 23: _T1A=
# 327
Cyc_token(lexbuf);return _T1A;case 24: _T1B=
# 330 "buildlib.cyl"
Cyc_token(lexbuf);return _T1B;case 25: _T1C=
# 332
Cyc_token(lexbuf);return _T1C;case 26: _T1D=
# 334
Cyc_token(lexbuf);return _T1D;case 27: _T1E=
# 336
Cyc_token(lexbuf);return _T1E;case 28: _T1F=
# 338
Cyc_token(lexbuf);return _T1F;case 29: _T20=
# 340
Cyc_token(lexbuf);return _T20;case 30: _T21=
# 342
Cyc_token(lexbuf);return _T21;case 31: _T22=
# 344
Cyc_token(lexbuf);return _T22;case 32: _T23=
# 346
Cyc_token(lexbuf);return _T23;case 33: _T24=
# 348
Cyc_token(lexbuf);return _T24;case 34: _T25=
# 350
Cyc_token(lexbuf);return _T25;case 35: _T26=
# 352
Cyc_token(lexbuf);return _T26;case 36: _T27=
# 354
Cyc_token(lexbuf);return _T27;case 37: _T28=
# 356
Cyc_token(lexbuf);return _T28;case 38: _T29=
# 358
Cyc_token(lexbuf);return _T29;case 39: _T2A=
# 360
Cyc_token(lexbuf);return _T2A;case 40: _T2B=
# 362
Cyc_token(lexbuf);return _T2B;case 41: _T2C=
# 364
Cyc_token(lexbuf);return _T2C;case 42: _T2D=
# 366
Cyc_token(lexbuf);return _T2D;case 43: _T2E=
# 368
Cyc_token(lexbuf);return _T2E;case 44: _T2F=
# 370
Cyc_token(lexbuf);return _T2F;case 45: _T30=
# 372
Cyc_token(lexbuf);return _T30;case 46: _T31=
# 374
Cyc_token(lexbuf);return _T31;case 47: _T32=
# 376
Cyc_token(lexbuf);return _T32;case 48: _T33=
# 379 "buildlib.cyl"
Cyc_token(lexbuf);return _T33;default: _T34=lexbuf;_T35=_T34->refill_buff;
_T35(lexbuf);_T36=
Cyc_token_rec(lexbuf,lexstate);return _T36;}{struct Cyc_Lexing_Error_exn_struct*_T39=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T39->tag=Cyc_Lexing_Error;
# 383
_T39->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T37=(struct Cyc_Lexing_Error_exn_struct*)_T39;}_T38=(void*)_T37;_throw(_T38);}
# 385
int Cyc_token(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_token_rec(lexbuf,3);return _T0;}
int Cyc_string_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;int _T2;int _T3;int _T4;int _T5;int _T6;int _T7;int _T8;struct Cyc_Lexing_lexbuf*_T9;void(*_TA)(struct Cyc_Lexing_lexbuf*);int _TB;struct Cyc_Lexing_Error_exn_struct*_TC;void*_TD;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0: _T2=
# 384 "buildlib.cyl"
Cyc_string(lexbuf);return _T2;case 1:
# 385 "buildlib.cyl"
 return 0;case 2: _T3=
# 386 "buildlib.cyl"
Cyc_string(lexbuf);return _T3;case 3: _T4=
# 387 "buildlib.cyl"
Cyc_string(lexbuf);return _T4;case 4: _T5=
# 390 "buildlib.cyl"
Cyc_string(lexbuf);return _T5;case 5: _T6=
# 393 "buildlib.cyl"
Cyc_string(lexbuf);return _T6;case 6: _T7=
# 395
Cyc_string(lexbuf);return _T7;case 7:
# 396 "buildlib.cyl"
 return 0;case 8:
# 397 "buildlib.cyl"
 return 0;case 9: _T8=
# 398 "buildlib.cyl"
Cyc_string(lexbuf);return _T8;default: _T9=lexbuf;_TA=_T9->refill_buff;
_TA(lexbuf);_TB=
Cyc_string_rec(lexbuf,lexstate);return _TB;}{struct Cyc_Lexing_Error_exn_struct*_TE=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_TE->tag=Cyc_Lexing_Error;
# 402
_TE->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_TC=(struct Cyc_Lexing_Error_exn_struct*)_TE;}_TD=(void*)_TC;_throw(_TD);}
# 404
int Cyc_string(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_string_rec(lexbuf,4);return _T0;}
int Cyc_slurp_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct Cyc___cycFILE*_T2;int _T3;struct Cyc___cycFILE*_T4;struct _fat_ptr _T5;struct _fat_ptr _T6;struct Cyc___cycFILE*_T7;struct _fat_ptr _T8;struct _fat_ptr _T9;struct Cyc___cycFILE*_TA;struct _fat_ptr _TB;struct _fat_ptr _TC;struct Cyc___cycFILE*_TD;struct _fat_ptr _TE;struct _fat_ptr _TF;struct Cyc___cycFILE*_T10;struct _fat_ptr _T11;struct _fat_ptr _T12;struct _fat_ptr _T13;struct _fat_ptr _T14;struct _fat_ptr _T15;struct _fat_ptr _T16;struct _fat_ptr _T17;struct _fat_ptr _T18;struct _fat_ptr _T19;struct _fat_ptr _T1A;struct _fat_ptr _T1B;struct _fat_ptr _T1C;struct _fat_ptr _T1D;struct _fat_ptr _T1E;struct _fat_ptr _T1F;struct _fat_ptr _T20;struct _fat_ptr _T21;struct _fat_ptr _T22;struct _fat_ptr _T23;struct _fat_ptr _T24;struct _fat_ptr _T25;struct _fat_ptr _T26;struct Cyc___cycFILE*_T27;struct Cyc___cycFILE*_T28;struct Cyc___cycFILE*_T29;struct Cyc___cycFILE*_T2A;struct Cyc___cycFILE*_T2B;struct Cyc___cycFILE*_T2C;char _T2D;int _T2E;struct Cyc___cycFILE*_T2F;struct Cyc_Lexing_lexbuf*_T30;void(*_T31)(struct Cyc_Lexing_lexbuf*);int _T32;struct Cyc_Lexing_Error_exn_struct*_T33;void*_T34;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:
# 407 "buildlib.cyl"
 return 0;case 1: _T2=
# 409
_check_null(Cyc_slurp_out);Cyc_fputc(34,_T2);
_TL29: _T3=Cyc_slurp_string(lexbuf);if(_T3)goto _TL2A;else{goto _TL2B;}_TL2A: goto _TL29;_TL2B:
 return 1;case 2: _T4=
# 416 "buildlib.cyl"
_check_null(Cyc_slurp_out);Cyc_fputs("*__IGNORE_FOR_CYCLONE_MALLOC(",_T4);_T5=
_tag_fat("Warning: declaration of malloc sidestepped\n",sizeof(char),44U);_T6=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T5,_T6);
return 1;case 3: _T7=
# 422 "buildlib.cyl"
_check_null(Cyc_slurp_out);Cyc_fputs(" __IGNORE_FOR_CYCLONE_MALLOC(",_T7);_T8=
_tag_fat("Warning: declaration of malloc sidestepped\n",sizeof(char),44U);_T9=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T8,_T9);
return 1;case 4: _TA=
# 428 "buildlib.cyl"
_check_null(Cyc_slurp_out);Cyc_fputs("*__IGNORE_FOR_CYCLONE_CALLOC(",_TA);_TB=
_tag_fat("Warning: declaration of calloc sidestepped\n",sizeof(char),44U);_TC=_tag_fat(0U,sizeof(void*),0);Cyc_log(_TB,_TC);
return 1;case 5: _TD=
# 434 "buildlib.cyl"
_check_null(Cyc_slurp_out);Cyc_fputs(" __IGNORE_FOR_CYCLONE_CALLOC(",_TD);_TE=
_tag_fat("Warning: declaration of calloc sidestepped\n",sizeof(char),44U);_TF=_tag_fat(0U,sizeof(void*),0);Cyc_log(_TE,_TF);
return 1;case 6: _T10=
# 438
_check_null(Cyc_slurp_out);Cyc_fputs("__region",_T10);_T11=
_tag_fat("Warning: use of region sidestepped\n",sizeof(char),36U);_T12=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T11,_T12);
return 1;case 7: _T13=
# 442
_tag_fat("Warning: use of __extension__ deleted\n",sizeof(char),39U);_T14=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T13,_T14);
return 1;case 8: _T15=
# 446 "buildlib.cyl"
_tag_fat("Warning: use of nonnull attribute deleted\n",sizeof(char),43U);_T16=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T15,_T16);
return 1;case 9: _T17=
# 451 "buildlib.cyl"
_tag_fat("Warning: use of mode HI deleted\n",sizeof(char),33U);_T18=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T17,_T18);
return 1;case 10: _T19=
# 454
_tag_fat("Warning: use of mode SI deleted\n",sizeof(char),33U);_T1A=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T19,_T1A);
return 1;case 11: _T1B=
# 457
_tag_fat("Warning: use of mode QI deleted\n",sizeof(char),33U);_T1C=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T1B,_T1C);
return 1;case 12: _T1D=
# 460
_tag_fat("Warning: use of mode DI deleted\n",sizeof(char),33U);_T1E=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T1D,_T1E);
return 1;case 13: _T1F=
# 463
_tag_fat("Warning: use of mode DI deleted\n",sizeof(char),33U);_T20=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T1F,_T20);
return 1;case 14: _T21=
# 466
_tag_fat("Warning: use of mode word deleted\n",sizeof(char),35U);_T22=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T21,_T22);
return 1;case 15: _T23=
# 469
_tag_fat("Warning: use of __attribute__ ((__deprecated__)) deleted\n",sizeof(char),58U);_T24=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T23,_T24);
return 1;case 16: _T25=
# 472
_tag_fat("Warning: use of __attribute__ ((__transparent_union__)) deleted\n",sizeof(char),65U);_T26=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T25,_T26);
return 1;case 17: _T27=
# 475
_check_null(Cyc_slurp_out);Cyc_fputs("inline",_T27);return 1;case 18: _T28=
# 477
_check_null(Cyc_slurp_out);Cyc_fputs("inline",_T28);return 1;case 19: _T29=
# 479
_check_null(Cyc_slurp_out);Cyc_fputs("const",_T29);return 1;case 20: _T2A=
# 481
_check_null(Cyc_slurp_out);Cyc_fputs("const",_T2A);return 1;case 21: _T2B=
# 483
_check_null(Cyc_slurp_out);Cyc_fputs("signed",_T2B);return 1;case 22: _T2C=
# 488 "buildlib.cyl"
_check_null(Cyc_slurp_out);Cyc_fputs("int",_T2C);return 1;case 23:
# 490
 return 1;case 24: _T2D=
# 492
Cyc_Lexing_lexeme_char(lexbuf,0);_T2E=(int)_T2D;_T2F=_check_null(Cyc_slurp_out);Cyc_fputc(_T2E,_T2F);return 1;default: _T30=lexbuf;_T31=_T30->refill_buff;
_T31(lexbuf);_T32=
Cyc_slurp_rec(lexbuf,lexstate);return _T32;}{struct Cyc_Lexing_Error_exn_struct*_T35=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T35->tag=Cyc_Lexing_Error;
# 496
_T35->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T33=(struct Cyc_Lexing_Error_exn_struct*)_T35;}_T34=(void*)_T33;_throw(_T34);}
# 498
int Cyc_slurp(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_slurp_rec(lexbuf,5);return _T0;}
int Cyc_slurp_string_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct Cyc___cycFILE*_T2;struct _fat_ptr _T3;struct _fat_ptr _T4;struct Cyc_String_pa_PrintArg_struct _T5;void**_T6;struct Cyc___cycFILE*_T7;struct _fat_ptr _T8;struct _fat_ptr _T9;struct Cyc_String_pa_PrintArg_struct _TA;void**_TB;struct Cyc___cycFILE*_TC;struct _fat_ptr _TD;struct _fat_ptr _TE;struct Cyc_String_pa_PrintArg_struct _TF;void**_T10;struct Cyc___cycFILE*_T11;struct _fat_ptr _T12;struct _fat_ptr _T13;struct Cyc_String_pa_PrintArg_struct _T14;void**_T15;struct Cyc___cycFILE*_T16;struct _fat_ptr _T17;struct _fat_ptr _T18;struct Cyc_String_pa_PrintArg_struct _T19;void**_T1A;struct Cyc___cycFILE*_T1B;struct _fat_ptr _T1C;struct _fat_ptr _T1D;struct Cyc_String_pa_PrintArg_struct _T1E;void**_T1F;struct Cyc___cycFILE*_T20;struct _fat_ptr _T21;struct _fat_ptr _T22;struct Cyc_String_pa_PrintArg_struct _T23;void**_T24;struct Cyc___cycFILE*_T25;struct _fat_ptr _T26;struct _fat_ptr _T27;struct Cyc_Lexing_lexbuf*_T28;void(*_T29)(struct Cyc_Lexing_lexbuf*);int _T2A;struct Cyc_Lexing_Error_exn_struct*_T2B;void*_T2C;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:
# 496 "buildlib.cyl"
 return 0;case 1: _T2=
# 498
_check_null(Cyc_slurp_out);Cyc_fputc(34,_T2);return 0;case 2: _T3=
# 500
_tag_fat("Warning: unclosed string\n",sizeof(char),26U);_T4=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T3,_T4);{struct Cyc_String_pa_PrintArg_struct _T2D;_T2D.tag=0;
_T2D.f1=Cyc_Lexing_lexeme(lexbuf);_T5=_T2D;}{struct Cyc_String_pa_PrintArg_struct _T2D=_T5;void*_T2E[1];_T6=_T2E + 0;*_T6=& _T2D;_T7=_check_null(Cyc_slurp_out);_T8=_tag_fat("%s",sizeof(char),3U);_T9=_tag_fat(_T2E,sizeof(void*),1);Cyc_fprintf(_T7,_T8,_T9);}return 1;case 3:{struct Cyc_String_pa_PrintArg_struct _T2D;_T2D.tag=0;
# 503
_T2D.f1=Cyc_Lexing_lexeme(lexbuf);_TA=_T2D;}{struct Cyc_String_pa_PrintArg_struct _T2D=_TA;void*_T2E[1];_TB=_T2E + 0;*_TB=& _T2D;_TC=_check_null(Cyc_slurp_out);_TD=_tag_fat("%s",sizeof(char),3U);_TE=_tag_fat(_T2E,sizeof(void*),1);Cyc_fprintf(_TC,_TD,_TE);}return 1;case 4:{struct Cyc_String_pa_PrintArg_struct _T2D;_T2D.tag=0;
# 505
_T2D.f1=Cyc_Lexing_lexeme(lexbuf);_TF=_T2D;}{struct Cyc_String_pa_PrintArg_struct _T2D=_TF;void*_T2E[1];_T10=_T2E + 0;*_T10=& _T2D;_T11=_check_null(Cyc_slurp_out);_T12=_tag_fat("%s",sizeof(char),3U);_T13=_tag_fat(_T2E,sizeof(void*),1);Cyc_fprintf(_T11,_T12,_T13);}return 1;case 5:{struct Cyc_String_pa_PrintArg_struct _T2D;_T2D.tag=0;
# 507
_T2D.f1=Cyc_Lexing_lexeme(lexbuf);_T14=_T2D;}{struct Cyc_String_pa_PrintArg_struct _T2D=_T14;void*_T2E[1];_T15=_T2E + 0;*_T15=& _T2D;_T16=_check_null(Cyc_slurp_out);_T17=_tag_fat("%s",sizeof(char),3U);_T18=_tag_fat(_T2E,sizeof(void*),1);Cyc_fprintf(_T16,_T17,_T18);}return 1;case 6:{struct Cyc_String_pa_PrintArg_struct _T2D;_T2D.tag=0;
# 509
_T2D.f1=Cyc_Lexing_lexeme(lexbuf);_T19=_T2D;}{struct Cyc_String_pa_PrintArg_struct _T2D=_T19;void*_T2E[1];_T1A=_T2E + 0;*_T1A=& _T2D;_T1B=_check_null(Cyc_slurp_out);_T1C=_tag_fat("%s",sizeof(char),3U);_T1D=_tag_fat(_T2E,sizeof(void*),1);Cyc_fprintf(_T1B,_T1C,_T1D);}return 1;case 7:{struct Cyc_String_pa_PrintArg_struct _T2D;_T2D.tag=0;
# 511
_T2D.f1=Cyc_Lexing_lexeme(lexbuf);_T1E=_T2D;}{struct Cyc_String_pa_PrintArg_struct _T2D=_T1E;void*_T2E[1];_T1F=_T2E + 0;*_T1F=& _T2D;_T20=_check_null(Cyc_slurp_out);_T21=_tag_fat("%s",sizeof(char),3U);_T22=_tag_fat(_T2E,sizeof(void*),1);Cyc_fprintf(_T20,_T21,_T22);}return 1;case 8:{struct Cyc_String_pa_PrintArg_struct _T2D;_T2D.tag=0;
# 513
_T2D.f1=Cyc_Lexing_lexeme(lexbuf);_T23=_T2D;}{struct Cyc_String_pa_PrintArg_struct _T2D=_T23;void*_T2E[1];_T24=_T2E + 0;*_T24=& _T2D;_T25=_check_null(Cyc_slurp_out);_T26=_tag_fat("%s",sizeof(char),3U);_T27=_tag_fat(_T2E,sizeof(void*),1);Cyc_fprintf(_T25,_T26,_T27);}return 1;default: _T28=lexbuf;_T29=_T28->refill_buff;
_T29(lexbuf);_T2A=
Cyc_slurp_string_rec(lexbuf,lexstate);return _T2A;}{struct Cyc_Lexing_Error_exn_struct*_T2D=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T2D->tag=Cyc_Lexing_Error;
# 517
_T2D->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T2B=(struct Cyc_Lexing_Error_exn_struct*)_T2D;}_T2C=(void*)_T2B;_throw(_T2C);}
# 519
int Cyc_slurp_string(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_slurp_string_rec(lexbuf,6);return _T0;}
int Cyc_asmtok_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;int _T2;int _T3;struct Cyc_Lexing_lexbuf*_T4;void(*_T5)(struct Cyc_Lexing_lexbuf*);int _T6;struct Cyc_Lexing_Error_exn_struct*_T7;void*_T8;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:
# 523 "buildlib.cyl"
 return 0;case 1:
# 525
 if(Cyc_parens_to_match!=1)goto _TL2E;return 0;_TL2E:
 Cyc_parens_to_match=Cyc_parens_to_match + -1;
return 1;case 2:
# 529
 Cyc_parens_to_match=Cyc_parens_to_match + 1;
return 1;case 3:
# 532
 _TL30: _T2=Cyc_asm_string(lexbuf);if(_T2)goto _TL31;else{goto _TL32;}_TL31: goto _TL30;_TL32:
 return 1;case 4:
# 535
 _TL33: _T3=Cyc_asm_comment(lexbuf);if(_T3)goto _TL34;else{goto _TL35;}_TL34: goto _TL33;_TL35:
 return 1;case 5:
# 538
 return 1;case 6:
# 540
 return 1;default: _T4=lexbuf;_T5=_T4->refill_buff;
_T5(lexbuf);_T6=
Cyc_asmtok_rec(lexbuf,lexstate);return _T6;}{struct Cyc_Lexing_Error_exn_struct*_T9=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T9->tag=Cyc_Lexing_Error;
# 544
_T9->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T7=(struct Cyc_Lexing_Error_exn_struct*)_T9;}_T8=(void*)_T7;_throw(_T8);}
# 546
int Cyc_asmtok(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_asmtok_rec(lexbuf,7);return _T0;}
int Cyc_asm_string_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _fat_ptr _T2;struct _fat_ptr _T3;struct _fat_ptr _T4;struct _fat_ptr _T5;struct Cyc_Lexing_lexbuf*_T6;void(*_T7)(struct Cyc_Lexing_lexbuf*);int _T8;struct Cyc_Lexing_Error_exn_struct*_T9;void*_TA;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0: _T2=
# 544 "buildlib.cyl"
_tag_fat("Warning: unclosed string\n",sizeof(char),26U);_T3=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T2,_T3);return 0;case 1:
# 546
 return 0;case 2: _T4=
# 548
_tag_fat("Warning: unclosed string\n",sizeof(char),26U);_T5=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T4,_T5);return 1;case 3:
# 550
 return 1;case 4:
# 552
 return 1;case 5:
# 554
 return 1;case 6:
# 556
 return 1;case 7:
# 558
 return 1;case 8:
# 560
 return 1;default: _T6=lexbuf;_T7=_T6->refill_buff;
_T7(lexbuf);_T8=
Cyc_asm_string_rec(lexbuf,lexstate);return _T8;}{struct Cyc_Lexing_Error_exn_struct*_TB=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_TB->tag=Cyc_Lexing_Error;
# 564
_TB->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T9=(struct Cyc_Lexing_Error_exn_struct*)_TB;}_TA=(void*)_T9;_throw(_TA);}
# 566
int Cyc_asm_string(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_asm_string_rec(lexbuf,8);return _T0;}
int Cyc_asm_comment_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _fat_ptr _T2;struct _fat_ptr _T3;struct Cyc_Lexing_lexbuf*_T4;void(*_T5)(struct Cyc_Lexing_lexbuf*);int _T6;struct Cyc_Lexing_Error_exn_struct*_T7;void*_T8;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0: _T2=
# 564 "buildlib.cyl"
_tag_fat("Warning: unclosed comment\n",sizeof(char),27U);_T3=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T2,_T3);return 0;case 1:
# 566
 return 0;case 2:
# 568
 return 1;default: _T4=lexbuf;_T5=_T4->refill_buff;
_T5(lexbuf);_T6=
Cyc_asm_comment_rec(lexbuf,lexstate);return _T6;}{struct Cyc_Lexing_Error_exn_struct*_T9=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T9->tag=Cyc_Lexing_Error;
# 572
_T9->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T7=(struct Cyc_Lexing_Error_exn_struct*)_T9;}_T8=(void*)_T7;_throw(_T8);}
# 574
int Cyc_asm_comment(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_asm_comment_rec(lexbuf,9);return _T0;}
struct _tuple14*Cyc_suck_line_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _tuple14*_T2;struct _tuple14*_T3;struct Cyc_Lexing_lexbuf*_T4;void(*_T5)(struct Cyc_Lexing_lexbuf*);struct _tuple14*_T6;struct Cyc_Lexing_Error_exn_struct*_T7;void*_T8;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:
# 576 "buildlib.cyl"
 Cyc_current_line=_tag_fat("#define ",sizeof(char),9U);
Cyc_suck_macroname(lexbuf);{struct _tuple14*_T9=_cycalloc(sizeof(struct _tuple14));
_T9->f0=Cyc_current_line;_T9->f1=_check_null(Cyc_current_source);_T2=(struct _tuple14*)_T9;}return _T2;case 1: _T3=
# 580
Cyc_suck_line(lexbuf);return _T3;case 2:
# 582
 return 0;default: _T4=lexbuf;_T5=_T4->refill_buff;
_T5(lexbuf);_T6=
Cyc_suck_line_rec(lexbuf,lexstate);return _T6;}{struct Cyc_Lexing_Error_exn_struct*_T9=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T9->tag=Cyc_Lexing_Error;
# 586
_T9->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T7=(struct Cyc_Lexing_Error_exn_struct*)_T9;}_T8=(void*)_T7;_throw(_T8);}
# 588
struct _tuple14*Cyc_suck_line(struct Cyc_Lexing_lexbuf*lexbuf){struct _tuple14*_T0;_T0=Cyc_suck_line_rec(lexbuf,10);return _T0;}
int Cyc_suck_macroname_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){struct _fat_ptr*_T0;struct _fat_ptr _T1;struct _fat_ptr _T2;struct _fat_ptr*_T3;struct _fat_ptr _T4;int _T5;struct Cyc_Lexing_lexbuf*_T6;void(*_T7)(struct Cyc_Lexing_lexbuf*);int _T8;struct Cyc_Lexing_Error_exn_struct*_T9;void*_TA;
lexstate=Cyc_lex_engine(lexstate,lexbuf);if(lexstate!=0)goto _TL39;{struct _fat_ptr*_TB=_cycalloc(sizeof(struct _fat_ptr));_T1=
# 586 "buildlib.cyl"
Cyc_Lexing_lexeme(lexbuf);*_TB=_T1;_T0=(struct _fat_ptr*)_TB;}Cyc_current_source=_T0;_T2=Cyc_current_line;_T3=Cyc_current_source;_T4=*_T3;
Cyc_current_line=Cyc_strconcat(_T2,_T4);_T5=
Cyc_suck_restofline(lexbuf);return _T5;_TL39: _T6=lexbuf;_T7=_T6->refill_buff;
# 590
_T7(lexbuf);_T8=
Cyc_suck_macroname_rec(lexbuf,lexstate);return _T8;{struct Cyc_Lexing_Error_exn_struct*_TB=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_TB->tag=Cyc_Lexing_Error;
# 593
_TB->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T9=(struct Cyc_Lexing_Error_exn_struct*)_TB;}_TA=(void*)_T9;_throw(_TA);}
# 595
int Cyc_suck_macroname(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_suck_macroname_rec(lexbuf,11);return _T0;}
int Cyc_suck_restofline_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){struct _fat_ptr _T0;struct _fat_ptr _T1;struct Cyc_Lexing_lexbuf*_T2;void(*_T3)(struct Cyc_Lexing_lexbuf*);int _T4;struct Cyc_Lexing_Error_exn_struct*_T5;void*_T6;
lexstate=Cyc_lex_engine(lexstate,lexbuf);if(lexstate!=0)goto _TL3B;_T0=Cyc_current_line;_T1=
# 593 "buildlib.cyl"
Cyc_Lexing_lexeme(lexbuf);Cyc_current_line=Cyc_strconcat(_T0,_T1);return 0;_TL3B: _T2=lexbuf;_T3=_T2->refill_buff;
_T3(lexbuf);_T4=
Cyc_suck_restofline_rec(lexbuf,lexstate);return _T4;{struct Cyc_Lexing_Error_exn_struct*_T7=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T7->tag=Cyc_Lexing_Error;
# 597
_T7->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T5=(struct Cyc_Lexing_Error_exn_struct*)_T7;}_T6=(void*)_T5;_throw(_T6);}
# 599
int Cyc_suck_restofline(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_suck_restofline_rec(lexbuf,12);return _T0;}
struct _tuple17*Cyc_spec_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _tuple17*_T2;struct _fat_ptr _T3;int _T4;int _T5;int _T6;int _T7;unsigned long _T8;struct _fat_ptr _T9;int _TA;struct _tuple17*_TB;struct _tuple17*_TC;struct Cyc_Int_pa_PrintArg_struct _TD;char _TE;int _TF;void**_T10;struct Cyc___cycFILE*_T11;struct _fat_ptr _T12;struct _fat_ptr _T13;struct Cyc_Lexing_lexbuf*_T14;void(*_T15)(struct Cyc_Lexing_lexbuf*);struct _tuple17*_T16;struct Cyc_Lexing_Error_exn_struct*_T17;void*_T18;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0: _T2=
# 600 "buildlib.cyl"
Cyc_spec(lexbuf);return _T2;case 1: _T3=
# 603
Cyc_Lexing_lexeme(lexbuf);_T4=
Cyc_Lexing_lexeme_end(lexbuf);_T5=Cyc_Lexing_lexeme_start(lexbuf);_T6=_T4 - _T5;_T7=_T6 - 1;_T8=(unsigned long)_T7;_T9=
# 603
Cyc_substring(_T3,0,_T8);
# 602
Cyc_current_headerfile=_T9;
# 605
Cyc_current_symbols=0;
Cyc_current_user_defs=0;
Cyc_current_omit_symbols=0;
Cyc_current_cstubs=0;
Cyc_current_cycstubs=0;
Cyc_current_hstubs=0;
Cyc_current_cpp=0;
_TL3E: _TA=Cyc_commands(lexbuf);if(_TA)goto _TL3F;else{goto _TL40;}_TL3F: goto _TL3E;_TL40:
 Cyc_current_hstubs=Cyc_List_imp_rev(Cyc_current_hstubs);
Cyc_current_cstubs=Cyc_List_imp_rev(Cyc_current_cstubs);
Cyc_current_cycstubs=Cyc_List_imp_rev(Cyc_current_cycstubs);
Cyc_current_cpp=Cyc_List_imp_rev(Cyc_current_cpp);{struct _tuple17*_T19=_cycalloc(sizeof(struct _tuple17));
_T19->f0=Cyc_current_headerfile;
_T19->f1=Cyc_current_symbols;
_T19->f2=Cyc_current_user_defs;
_T19->f3=Cyc_current_omit_symbols;
_T19->f4=Cyc_current_hstubs;
_T19->f5=Cyc_current_cstubs;
_T19->f6=Cyc_current_cycstubs;
_T19->f7=Cyc_current_cpp;_TB=(struct _tuple17*)_T19;}
# 617
return _TB;case 2: _TC=
# 627
Cyc_spec(lexbuf);return _TC;case 3:
# 629
 return 0;case 4:{struct Cyc_Int_pa_PrintArg_struct _T19;_T19.tag=1;_TE=
# 633
Cyc_Lexing_lexeme_char(lexbuf,0);_TF=(int)_TE;_T19.f1=(unsigned long)_TF;_TD=_T19;}{struct Cyc_Int_pa_PrintArg_struct _T19=_TD;void*_T1A[1];_T10=_T1A + 0;*_T10=& _T19;_T11=Cyc_stderr;_T12=
# 632
_tag_fat("Error in .cys file: expected header file name, found '%c' instead\n",sizeof(char),67U);_T13=_tag_fat(_T1A,sizeof(void*),1);Cyc_fprintf(_T11,_T12,_T13);}
# 634
return 0;default: _T14=lexbuf;_T15=_T14->refill_buff;
_T15(lexbuf);_T16=
Cyc_spec_rec(lexbuf,lexstate);return _T16;}{struct Cyc_Lexing_Error_exn_struct*_T19=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T19->tag=Cyc_Lexing_Error;
# 638
_T19->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T17=(struct Cyc_Lexing_Error_exn_struct*)_T19;}_T18=(void*)_T17;_throw(_T18);}
# 640
struct _tuple17*Cyc_spec(struct Cyc_Lexing_lexbuf*lexbuf){struct _tuple17*_T0;_T0=Cyc_spec_rec(lexbuf,13);return _T0;}
int Cyc_commands_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;int _T2;int _T3;struct Cyc___cycFILE*_T4;struct _fat_ptr _T5;struct _fat_ptr _T6;int _T7;struct _tuple15*_T8;struct _fat_ptr _T9;struct _tuple15*_TA;struct Cyc_Buffer_t*_TB;struct _fat_ptr _TC;struct Cyc_List_List*_TD;struct _fat_ptr*_TE;struct _fat_ptr _TF;unsigned long _T10;int _T11;struct _fat_ptr _T12;char*_T13;char*_T14;char _T15;int _T16;int _T17;struct _fat_ptr*_T18;struct _fat_ptr _T19;char*_T1A;char*_T1B;char _T1C;int _T1D;int _T1E;struct _fat_ptr*_T1F;struct _fat_ptr _T20;struct _fat_ptr _T21;unsigned char*_T22;struct _fat_ptr _T23;unsigned char*_T24;int _T25;int _T26;unsigned long _T27;int _T28;struct _tuple15*_T29;struct _fat_ptr _T2A;struct _tuple15*_T2B;struct Cyc_Buffer_t*_T2C;struct _fat_ptr _T2D;struct Cyc_List_List*_T2E;int _T2F;struct _tuple15*_T30;struct _fat_ptr _T31;struct _tuple15*_T32;struct Cyc_Buffer_t*_T33;struct _fat_ptr _T34;struct Cyc_List_List*_T35;struct _fat_ptr*_T36;struct _fat_ptr _T37;unsigned long _T38;int _T39;struct _fat_ptr _T3A;char*_T3B;char*_T3C;char _T3D;int _T3E;int _T3F;struct _fat_ptr*_T40;struct _fat_ptr _T41;char*_T42;char*_T43;char _T44;int _T45;int _T46;struct _fat_ptr*_T47;struct _fat_ptr _T48;struct _fat_ptr _T49;unsigned char*_T4A;struct _fat_ptr _T4B;unsigned char*_T4C;int _T4D;int _T4E;unsigned long _T4F;int _T50;struct _tuple15*_T51;struct _fat_ptr _T52;struct _tuple15*_T53;struct Cyc_Buffer_t*_T54;struct _fat_ptr _T55;struct Cyc_List_List*_T56;int _T57;struct _tuple15*_T58;struct _fat_ptr _T59;struct _tuple15*_T5A;struct Cyc_Buffer_t*_T5B;struct _fat_ptr _T5C;struct Cyc_List_List*_T5D;struct _fat_ptr*_T5E;struct _fat_ptr _T5F;unsigned long _T60;int _T61;struct _fat_ptr _T62;char*_T63;char*_T64;char _T65;int _T66;int _T67;struct _fat_ptr*_T68;struct _fat_ptr _T69;char*_T6A;char*_T6B;char _T6C;int _T6D;int _T6E;struct _fat_ptr*_T6F;struct _fat_ptr _T70;struct _fat_ptr _T71;unsigned char*_T72;struct _fat_ptr _T73;unsigned char*_T74;int _T75;int _T76;unsigned long _T77;int _T78;struct _tuple15*_T79;struct _fat_ptr _T7A;struct _tuple15*_T7B;struct Cyc_Buffer_t*_T7C;struct _fat_ptr _T7D;struct Cyc_List_List*_T7E;int _T7F;struct _fat_ptr*_T80;struct Cyc_Buffer_t*_T81;struct _fat_ptr _T82;struct Cyc_List_List*_T83;struct Cyc_Int_pa_PrintArg_struct _T84;char _T85;int _T86;void**_T87;struct Cyc___cycFILE*_T88;struct _fat_ptr _T89;struct _fat_ptr _T8A;struct Cyc_Lexing_lexbuf*_T8B;void(*_T8C)(struct Cyc_Lexing_lexbuf*);int _T8D;struct Cyc_Lexing_Error_exn_struct*_T8E;void*_T8F;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:
# 638 "buildlib.cyl"
 return 0;case 1:
# 640
 return 0;case 2:
# 642
 Cyc_snarfed_symbols=0;
_TL42: _T2=Cyc_snarfsymbols(lexbuf);if(_T2)goto _TL43;else{goto _TL44;}_TL43: goto _TL42;_TL44:
 Cyc_current_symbols=Cyc_List_append(Cyc_snarfed_symbols,Cyc_current_symbols);
return 1;case 3:
# 647
 Cyc_snarfed_symbols=0;{
struct Cyc_List_List*tmp_user_defs=Cyc_current_user_defs;
_TL45: _T3=Cyc_snarfsymbols(lexbuf);if(_T3)goto _TL46;else{goto _TL47;}_TL46: goto _TL45;_TL47:
 if(tmp_user_defs==Cyc_current_user_defs)goto _TL48;_T4=Cyc_stderr;_T5=
# 652
_tag_fat("Error in .cys file: got optional definition in omitsymbols\n",sizeof(char),60U);_T6=_tag_fat(0U,sizeof(void*),0);
# 651
Cyc_fprintf(_T4,_T5,_T6);
# 653
return 0;_TL48:
# 655
 Cyc_current_omit_symbols=Cyc_List_append(Cyc_snarfed_symbols,Cyc_current_omit_symbols);
return 1;}case 4:
# 658
 Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255U);
_TL4A: _T7=Cyc_block(lexbuf);if(_T7)goto _TL4B;else{goto _TL4C;}_TL4B: goto _TL4A;_TL4C: {
struct _tuple15*x;x=_cycalloc(sizeof(struct _tuple15));_T8=x;_T9=_tag_fat(0,0,0);_T8->f0=_T9;_TA=x;_TB=
_check_null(Cyc_specbuf);_TC=Cyc_Buffer_contents(_TB);_TA->f1=_TC;{struct Cyc_List_List*_T90=_cycalloc(sizeof(struct Cyc_List_List));
_T90->hd=x;_T90->tl=Cyc_current_hstubs;_TD=(struct Cyc_List_List*)_T90;}Cyc_current_hstubs=_TD;
return 1;}case 5:  {
# 666
struct _fat_ptr s=Cyc_Lexing_lexeme(lexbuf);_TE=& s;_TF=
_tag_fat("hstub",sizeof(char),6U);_T10=Cyc_strlen(_TF);_T11=(int)_T10;_fat_ptr_inplace_plus(_TE,sizeof(char),_T11);
_TL4D: _T12=s;_T13=_check_fat_subscript(_T12,sizeof(char),0U);_T14=(char*)_T13;_T15=*_T14;_T16=(int)_T15;_T17=isspace(_T16);if(_T17)goto _TL4E;else{goto _TL4F;}_TL4E: _T18=& s;_fat_ptr_inplace_plus(_T18,sizeof(char),1);goto _TL4D;_TL4F: {
struct _fat_ptr t=s;
_TL50: _T19=t;_T1A=_check_fat_subscript(_T19,sizeof(char),0U);_T1B=(char*)_T1A;_T1C=*_T1B;_T1D=(int)_T1C;_T1E=isspace(_T1D);if(_T1E)goto _TL52;else{goto _TL51;}_TL51: _T1F=& t;_fat_ptr_inplace_plus(_T1F,sizeof(char),1);goto _TL50;_TL52: _T20=s;_T21=t;_T22=_T21.curr;_T23=s;_T24=_T23.curr;_T25=_T22 - _T24;_T26=_T25 / sizeof(char);_T27=(unsigned long)_T26;{
struct _fat_ptr symbol=Cyc_substring(_T20,0,_T27);
Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255U);
_TL53: _T28=Cyc_block(lexbuf);if(_T28)goto _TL54;else{goto _TL55;}_TL54: goto _TL53;_TL55: {
struct _tuple15*x;x=_cycalloc(sizeof(struct _tuple15));_T29=x;_T2A=symbol;_T29->f0=_T2A;_T2B=x;_T2C=
_check_null(Cyc_specbuf);_T2D=Cyc_Buffer_contents(_T2C);_T2B->f1=_T2D;{struct Cyc_List_List*_T90=_cycalloc(sizeof(struct Cyc_List_List));
_T90->hd=x;_T90->tl=Cyc_current_hstubs;_T2E=(struct Cyc_List_List*)_T90;}Cyc_current_hstubs=_T2E;
return 1;}}}}case 6:
# 680
 Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255U);
_TL56: _T2F=Cyc_block(lexbuf);if(_T2F)goto _TL57;else{goto _TL58;}_TL57: goto _TL56;_TL58: {
struct _tuple15*x;x=_cycalloc(sizeof(struct _tuple15));_T30=x;_T31=_tag_fat(0,0,0);_T30->f0=_T31;_T32=x;_T33=
_check_null(Cyc_specbuf);_T34=Cyc_Buffer_contents(_T33);_T32->f1=_T34;{struct Cyc_List_List*_T90=_cycalloc(sizeof(struct Cyc_List_List));
_T90->hd=x;_T90->tl=Cyc_current_cstubs;_T35=(struct Cyc_List_List*)_T90;}Cyc_current_cstubs=_T35;
return 1;}case 7:  {
# 688
struct _fat_ptr s=Cyc_Lexing_lexeme(lexbuf);_T36=& s;_T37=
_tag_fat("cstub",sizeof(char),6U);_T38=Cyc_strlen(_T37);_T39=(int)_T38;_fat_ptr_inplace_plus(_T36,sizeof(char),_T39);
_TL59: _T3A=s;_T3B=_check_fat_subscript(_T3A,sizeof(char),0U);_T3C=(char*)_T3B;_T3D=*_T3C;_T3E=(int)_T3D;_T3F=isspace(_T3E);if(_T3F)goto _TL5A;else{goto _TL5B;}_TL5A: _T40=& s;_fat_ptr_inplace_plus(_T40,sizeof(char),1);goto _TL59;_TL5B: {
struct _fat_ptr t=s;
_TL5C: _T41=t;_T42=_check_fat_subscript(_T41,sizeof(char),0U);_T43=(char*)_T42;_T44=*_T43;_T45=(int)_T44;_T46=isspace(_T45);if(_T46)goto _TL5E;else{goto _TL5D;}_TL5D: _T47=& t;_fat_ptr_inplace_plus(_T47,sizeof(char),1);goto _TL5C;_TL5E: _T48=s;_T49=t;_T4A=_T49.curr;_T4B=s;_T4C=_T4B.curr;_T4D=_T4A - _T4C;_T4E=_T4D / sizeof(char);_T4F=(unsigned long)_T4E;{
struct _fat_ptr symbol=Cyc_substring(_T48,0,_T4F);
Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255U);
_TL5F: _T50=Cyc_block(lexbuf);if(_T50)goto _TL60;else{goto _TL61;}_TL60: goto _TL5F;_TL61: {
struct _tuple15*x;x=_cycalloc(sizeof(struct _tuple15));_T51=x;_T52=symbol;_T51->f0=_T52;_T53=x;_T54=
_check_null(Cyc_specbuf);_T55=Cyc_Buffer_contents(_T54);_T53->f1=_T55;{struct Cyc_List_List*_T90=_cycalloc(sizeof(struct Cyc_List_List));
_T90->hd=x;_T90->tl=Cyc_current_cstubs;_T56=(struct Cyc_List_List*)_T90;}Cyc_current_cstubs=_T56;
return 1;}}}}case 8:
# 702
 Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255U);
_TL62: _T57=Cyc_block(lexbuf);if(_T57)goto _TL63;else{goto _TL64;}_TL63: goto _TL62;_TL64: {
struct _tuple15*x;x=_cycalloc(sizeof(struct _tuple15));_T58=x;_T59=_tag_fat(0,0,0);_T58->f0=_T59;_T5A=x;_T5B=
_check_null(Cyc_specbuf);_T5C=Cyc_Buffer_contents(_T5B);_T5A->f1=_T5C;{struct Cyc_List_List*_T90=_cycalloc(sizeof(struct Cyc_List_List));
_T90->hd=x;_T90->tl=Cyc_current_cycstubs;_T5D=(struct Cyc_List_List*)_T90;}Cyc_current_cycstubs=_T5D;
return 1;}case 9:  {
# 710
struct _fat_ptr s=Cyc_Lexing_lexeme(lexbuf);_T5E=& s;_T5F=
_tag_fat("cycstub",sizeof(char),8U);_T60=Cyc_strlen(_T5F);_T61=(int)_T60;_fat_ptr_inplace_plus(_T5E,sizeof(char),_T61);
_TL65: _T62=s;_T63=_check_fat_subscript(_T62,sizeof(char),0U);_T64=(char*)_T63;_T65=*_T64;_T66=(int)_T65;_T67=isspace(_T66);if(_T67)goto _TL66;else{goto _TL67;}_TL66: _T68=& s;_fat_ptr_inplace_plus(_T68,sizeof(char),1);goto _TL65;_TL67: {
struct _fat_ptr t=s;
_TL68: _T69=t;_T6A=_check_fat_subscript(_T69,sizeof(char),0U);_T6B=(char*)_T6A;_T6C=*_T6B;_T6D=(int)_T6C;_T6E=isspace(_T6D);if(_T6E)goto _TL6A;else{goto _TL69;}_TL69: _T6F=& t;_fat_ptr_inplace_plus(_T6F,sizeof(char),1);goto _TL68;_TL6A: _T70=s;_T71=t;_T72=_T71.curr;_T73=s;_T74=_T73.curr;_T75=_T72 - _T74;_T76=_T75 / sizeof(char);_T77=(unsigned long)_T76;{
struct _fat_ptr symbol=Cyc_substring(_T70,0,_T77);
Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255U);
_TL6B: _T78=Cyc_block(lexbuf);if(_T78)goto _TL6C;else{goto _TL6D;}_TL6C: goto _TL6B;_TL6D: {
struct _tuple15*x;x=_cycalloc(sizeof(struct _tuple15));_T79=x;_T7A=symbol;_T79->f0=_T7A;_T7B=x;_T7C=
_check_null(Cyc_specbuf);_T7D=Cyc_Buffer_contents(_T7C);_T7B->f1=_T7D;{struct Cyc_List_List*_T90=_cycalloc(sizeof(struct Cyc_List_List));
_T90->hd=x;_T90->tl=Cyc_current_cycstubs;_T7E=(struct Cyc_List_List*)_T90;}Cyc_current_cycstubs=_T7E;
return 1;}}}}case 10:
# 724
 Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255U);
_TL6E: _T7F=Cyc_block(lexbuf);if(_T7F)goto _TL6F;else{goto _TL70;}_TL6F: goto _TL6E;_TL70: {
struct _fat_ptr*x;x=_cycalloc(sizeof(struct _fat_ptr));_T80=x;_T81=_check_null(Cyc_specbuf);_T82=Cyc_Buffer_contents(_T81);*_T80=_T82;{struct Cyc_List_List*_T90=_cycalloc(sizeof(struct Cyc_List_List));
_T90->hd=x;_T90->tl=Cyc_current_cpp;_T83=(struct Cyc_List_List*)_T90;}Cyc_current_cpp=_T83;
return 1;}case 11:
# 731
 return 1;case 12:
# 733
 return 1;case 13:{struct Cyc_Int_pa_PrintArg_struct _T90;_T90.tag=1;_T85=
# 737
Cyc_Lexing_lexeme_char(lexbuf,0);_T86=(int)_T85;_T90.f1=(unsigned long)_T86;_T84=_T90;}{struct Cyc_Int_pa_PrintArg_struct _T90=_T84;void*_T91[1];_T87=_T91 + 0;*_T87=& _T90;_T88=Cyc_stderr;_T89=
# 736
_tag_fat("Error in .cys file: expected command, found '%c' instead\n",sizeof(char),58U);_T8A=_tag_fat(_T91,sizeof(void*),1);Cyc_fprintf(_T88,_T89,_T8A);}
# 738
return 0;default: _T8B=lexbuf;_T8C=_T8B->refill_buff;
_T8C(lexbuf);_T8D=
Cyc_commands_rec(lexbuf,lexstate);return _T8D;}{struct Cyc_Lexing_Error_exn_struct*_T90=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T90->tag=Cyc_Lexing_Error;
# 742
_T90->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T8E=(struct Cyc_Lexing_Error_exn_struct*)_T90;}_T8F=(void*)_T8E;_throw(_T8F);}
# 744
int Cyc_commands(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_commands_rec(lexbuf,14);return _T0;}
int Cyc_snarfsymbols_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct Cyc_List_List*_T2;struct _fat_ptr*_T3;struct _fat_ptr _T4;struct _fat_ptr _T5;char*_T6;char*_T7;char _T8;int _T9;int _TA;struct _fat_ptr*_TB;struct _fat_ptr _TC;struct _fat_ptr _TD;unsigned char*_TE;struct _fat_ptr _TF;unsigned char*_T10;int _T11;int _T12;unsigned long _T13;int _T14;struct _tuple16*_T15;struct _fat_ptr*_T16;struct _tuple16*_T17;struct _fat_ptr*_T18;struct Cyc_Buffer_t*_T19;struct _fat_ptr _T1A;struct Cyc_List_List*_T1B;struct _fat_ptr*_T1C;struct _fat_ptr _T1D;struct Cyc_List_List*_T1E;struct Cyc___cycFILE*_T1F;struct _fat_ptr _T20;struct _fat_ptr _T21;struct Cyc_Int_pa_PrintArg_struct _T22;char _T23;int _T24;void**_T25;struct Cyc___cycFILE*_T26;struct _fat_ptr _T27;struct _fat_ptr _T28;struct Cyc_Lexing_lexbuf*_T29;void(*_T2A)(struct Cyc_Lexing_lexbuf*);int _T2B;struct Cyc_Lexing_Error_exn_struct*_T2C;void*_T2D;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0:{struct Cyc_List_List*_T2E=_cycalloc(sizeof(struct Cyc_List_List));{struct _fat_ptr*_T2F=_cycalloc(sizeof(struct _fat_ptr));_T4=
# 748 "buildlib.cyl"
Cyc_Lexing_lexeme(lexbuf);*_T2F=_T4;_T3=(struct _fat_ptr*)_T2F;}_T2E->hd=_T3;_T2E->tl=Cyc_snarfed_symbols;_T2=(struct Cyc_List_List*)_T2E;}Cyc_snarfed_symbols=_T2;
return 1;case 1:  {
# 751
struct _fat_ptr s=Cyc_Lexing_lexeme(lexbuf);
struct _fat_ptr t=s;
_TL72: _T5=t;_T6=_check_fat_subscript(_T5,sizeof(char),0U);_T7=(char*)_T6;_T8=*_T7;_T9=(int)_T8;_TA=isspace(_T9);if(_TA)goto _TL74;else{goto _TL73;}_TL73: _TB=& t;_fat_ptr_inplace_plus(_TB,sizeof(char),1);goto _TL72;_TL74: _TC=s;_TD=t;_TE=_TD.curr;_TF=s;_T10=_TF.curr;_T11=_TE - _T10;_T12=_T11 / sizeof(char);_T13=(unsigned long)_T12;
Cyc_current_symbol=Cyc_substring(_TC,0,_T13);
Cyc_rename_current_symbol=1;
Cyc_braces_to_match=1;
Cyc_specbuf=Cyc_Buffer_create(255U);
_TL75: _T14=Cyc_block(lexbuf);if(_T14)goto _TL76;else{goto _TL77;}_TL76: goto _TL75;_TL77:
# 760
 Cyc_rename_current_symbol=0;{
struct _tuple16*user_def;user_def=_cycalloc(sizeof(struct _tuple16));_T15=user_def;{struct _fat_ptr*_T2E=_cycalloc(sizeof(struct _fat_ptr));*_T2E=Cyc_current_symbol;_T16=(struct _fat_ptr*)_T2E;}_T15->f0=_T16;_T17=user_def;{struct _fat_ptr*_T2E=_cycalloc(sizeof(struct _fat_ptr));_T19=
_check_null(Cyc_specbuf);_T1A=Cyc_Buffer_contents(_T19);*_T2E=_T1A;_T18=(struct _fat_ptr*)_T2E;}_T17->f1=_T18;{struct Cyc_List_List*_T2E=_cycalloc(sizeof(struct Cyc_List_List));{struct _fat_ptr*_T2F=_cycalloc(sizeof(struct _fat_ptr));_T1D=Cyc_current_symbol;
*_T2F=_T1D;_T1C=(struct _fat_ptr*)_T2F;}_T2E->hd=_T1C;_T2E->tl=Cyc_snarfed_symbols;_T1B=(struct Cyc_List_List*)_T2E;}Cyc_snarfed_symbols=_T1B;{struct Cyc_List_List*_T2E=_cycalloc(sizeof(struct Cyc_List_List));
_T2E->hd=user_def;_T2E->tl=Cyc_current_user_defs;_T1E=(struct Cyc_List_List*)_T2E;}Cyc_current_user_defs=_T1E;
return 1;}}case 2:
# 767
 return 1;case 3:
# 769
 return 0;case 4: _T1F=Cyc_stderr;_T20=
# 772
_tag_fat("Error in .cys file: unexpected end-of-file\n",sizeof(char),44U);_T21=_tag_fat(0U,sizeof(void*),0);
# 771
Cyc_fprintf(_T1F,_T20,_T21);
# 773
return 0;case 5:{struct Cyc_Int_pa_PrintArg_struct _T2E;_T2E.tag=1;_T23=
# 777
Cyc_Lexing_lexeme_char(lexbuf,0);_T24=(int)_T23;_T2E.f1=(unsigned long)_T24;_T22=_T2E;}{struct Cyc_Int_pa_PrintArg_struct _T2E=_T22;void*_T2F[1];_T25=_T2F + 0;*_T25=& _T2E;_T26=Cyc_stderr;_T27=
# 776
_tag_fat("Error in .cys file: expected symbol, found '%c' instead\n",sizeof(char),57U);_T28=_tag_fat(_T2F,sizeof(void*),1);Cyc_fprintf(_T26,_T27,_T28);}
# 778
return 0;default: _T29=lexbuf;_T2A=_T29->refill_buff;
_T2A(lexbuf);_T2B=
Cyc_snarfsymbols_rec(lexbuf,lexstate);return _T2B;}{struct Cyc_Lexing_Error_exn_struct*_T2E=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T2E->tag=Cyc_Lexing_Error;
# 782
_T2E->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T2C=(struct Cyc_Lexing_Error_exn_struct*)_T2E;}_T2D=(void*)_T2C;_throw(_T2D);}
# 784
int Cyc_snarfsymbols(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_snarfsymbols_rec(lexbuf,15);return _T0;}
int Cyc_block_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _fat_ptr _T2;struct _fat_ptr _T3;struct Cyc_Buffer_t*_T4;struct Cyc_Buffer_t*_T5;struct Cyc_Buffer_t*_T6;int _T7;struct Cyc_Buffer_t*_T8;struct _fat_ptr _T9;int _TA;struct Cyc_Buffer_t*_TB;struct _fat_ptr _TC;struct _fat_ptr _TD;int _TE;int _TF;struct Cyc_Buffer_t*_T10;struct _fat_ptr*_T11;struct _fat_ptr*_T12;struct _fat_ptr _T13;struct Cyc_Buffer_t*_T14;struct _fat_ptr _T15;struct Cyc_Buffer_t*_T16;char _T17;struct Cyc_Lexing_lexbuf*_T18;void(*_T19)(struct Cyc_Lexing_lexbuf*);int _T1A;struct Cyc_Lexing_Error_exn_struct*_T1B;void*_T1C;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0: _T2=
# 788 "buildlib.cyl"
_tag_fat("Warning: unclosed brace\n",sizeof(char),25U);_T3=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T2,_T3);return 0;case 1:
# 790
 if(Cyc_braces_to_match!=1)goto _TL79;return 0;_TL79:
 Cyc_braces_to_match=Cyc_braces_to_match + -1;_T4=
_check_null(Cyc_specbuf);Cyc_Buffer_add_char(_T4,'}');
return 1;case 2:
# 795
 Cyc_braces_to_match=Cyc_braces_to_match + 1;_T5=
_check_null(Cyc_specbuf);Cyc_Buffer_add_char(_T5,'{');
return 1;case 3: _T6=
# 799
_check_null(Cyc_specbuf);Cyc_Buffer_add_char(_T6,'"');
_TL7B: _T7=Cyc_block_string(lexbuf);if(_T7)goto _TL7C;else{goto _TL7D;}_TL7C: goto _TL7B;_TL7D:
 return 1;case 4: _T8=
# 803
_check_null(Cyc_specbuf);_T9=_tag_fat("/*",sizeof(char),3U);Cyc_Buffer_add_string(_T8,_T9);
_TL7E: _TA=Cyc_block_comment(lexbuf);if(_TA)goto _TL7F;else{goto _TL80;}_TL7F: goto _TL7E;_TL80:
 return 1;case 5: _TB=
# 807
_check_null(Cyc_specbuf);_TC=Cyc_Lexing_lexeme(lexbuf);Cyc_Buffer_add_string(_TB,_TC);
return 1;case 6: _TD=
# 810
Cyc_Lexing_lexeme(lexbuf);{struct _fat_ptr symbol=_TD;_TE=Cyc_rename_current_symbol;
if(!_TE)goto _TL81;_TF=Cyc_strcmp(symbol,Cyc_current_symbol);if(_TF)goto _TL81;else{goto _TL83;}
_TL83: _T10=_check_null(Cyc_specbuf);{struct _fat_ptr*_T1D=_cycalloc(sizeof(struct _fat_ptr));*_T1D=symbol;_T11=(struct _fat_ptr*)_T1D;}_T12=Cyc_add_user_prefix(_T11);_T13=*_T12;Cyc_Buffer_add_string(_T10,_T13);goto _TL82;
# 814
_TL81: _T14=_check_null(Cyc_specbuf);_T15=symbol;Cyc_Buffer_add_string(_T14,_T15);_TL82:
 return 1;}case 7: _T16=
# 817
_check_null(Cyc_specbuf);_T17=Cyc_Lexing_lexeme_char(lexbuf,0);Cyc_Buffer_add_char(_T16,_T17);
return 1;default: _T18=lexbuf;_T19=_T18->refill_buff;
_T19(lexbuf);_T1A=
Cyc_block_rec(lexbuf,lexstate);return _T1A;}{struct Cyc_Lexing_Error_exn_struct*_T1D=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T1D->tag=Cyc_Lexing_Error;
# 822
_T1D->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T1B=(struct Cyc_Lexing_Error_exn_struct*)_T1D;}_T1C=(void*)_T1B;_throw(_T1C);}
# 824
int Cyc_block(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_block_rec(lexbuf,16);return _T0;}
int Cyc_block_string_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _fat_ptr _T2;struct _fat_ptr _T3;struct Cyc_Buffer_t*_T4;struct _fat_ptr _T5;struct _fat_ptr _T6;struct Cyc_Buffer_t*_T7;struct _fat_ptr _T8;struct Cyc_Buffer_t*_T9;struct _fat_ptr _TA;struct Cyc_Buffer_t*_TB;struct _fat_ptr _TC;struct Cyc_Buffer_t*_TD;struct _fat_ptr _TE;struct Cyc_Buffer_t*_TF;struct _fat_ptr _T10;struct Cyc_Buffer_t*_T11;struct _fat_ptr _T12;struct Cyc_Buffer_t*_T13;struct _fat_ptr _T14;struct Cyc_Lexing_lexbuf*_T15;void(*_T16)(struct Cyc_Lexing_lexbuf*);int _T17;struct Cyc_Lexing_Error_exn_struct*_T18;void*_T19;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0: _T2=
# 822 "buildlib.cyl"
_tag_fat("Warning: unclosed string\n",sizeof(char),26U);_T3=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T2,_T3);return 0;case 1: _T4=
# 824
_check_null(Cyc_specbuf);Cyc_Buffer_add_char(_T4,'"');return 0;case 2: _T5=
# 826
_tag_fat("Warning: unclosed string\n",sizeof(char),26U);_T6=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T5,_T6);_T7=
_check_null(Cyc_specbuf);_T8=Cyc_Lexing_lexeme(lexbuf);Cyc_Buffer_add_string(_T7,_T8);
return 1;case 3: _T9=
# 830
_check_null(Cyc_specbuf);_TA=Cyc_Lexing_lexeme(lexbuf);Cyc_Buffer_add_string(_T9,_TA);
return 1;case 4: _TB=
# 833
_check_null(Cyc_specbuf);_TC=Cyc_Lexing_lexeme(lexbuf);Cyc_Buffer_add_string(_TB,_TC);
return 1;case 5: _TD=
# 836
_check_null(Cyc_specbuf);_TE=Cyc_Lexing_lexeme(lexbuf);Cyc_Buffer_add_string(_TD,_TE);
return 1;case 6: _TF=
# 839
_check_null(Cyc_specbuf);_T10=Cyc_Lexing_lexeme(lexbuf);Cyc_Buffer_add_string(_TF,_T10);
return 1;case 7: _T11=
# 842
_check_null(Cyc_specbuf);_T12=Cyc_Lexing_lexeme(lexbuf);Cyc_Buffer_add_string(_T11,_T12);
return 1;case 8: _T13=
# 845
_check_null(Cyc_specbuf);_T14=Cyc_Lexing_lexeme(lexbuf);Cyc_Buffer_add_string(_T13,_T14);
return 1;default: _T15=lexbuf;_T16=_T15->refill_buff;
_T16(lexbuf);_T17=
Cyc_block_string_rec(lexbuf,lexstate);return _T17;}{struct Cyc_Lexing_Error_exn_struct*_T1A=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_T1A->tag=Cyc_Lexing_Error;
# 850
_T1A->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_T18=(struct Cyc_Lexing_Error_exn_struct*)_T1A;}_T19=(void*)_T18;_throw(_T19);}
# 852
int Cyc_block_string(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_block_string_rec(lexbuf,17);return _T0;}
int Cyc_block_comment_rec(struct Cyc_Lexing_lexbuf*lexbuf,int lexstate){int _T0;int _T1;struct _fat_ptr _T2;struct _fat_ptr _T3;struct Cyc_Buffer_t*_T4;struct _fat_ptr _T5;struct Cyc_Buffer_t*_T6;struct _fat_ptr _T7;struct Cyc_Lexing_lexbuf*_T8;void(*_T9)(struct Cyc_Lexing_lexbuf*);int _TA;struct Cyc_Lexing_Error_exn_struct*_TB;void*_TC;
lexstate=Cyc_lex_engine(lexstate,lexbuf);_T0=lexstate;_T1=(int)_T0;switch(_T1){case 0: _T2=
# 850 "buildlib.cyl"
_tag_fat("Warning: unclosed comment\n",sizeof(char),27U);_T3=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T2,_T3);return 0;case 1: _T4=
# 852
_check_null(Cyc_specbuf);_T5=_tag_fat("*/",sizeof(char),3U);Cyc_Buffer_add_string(_T4,_T5);return 0;case 2: _T6=
# 854
_check_null(Cyc_specbuf);_T7=Cyc_Lexing_lexeme(lexbuf);Cyc_Buffer_add_string(_T6,_T7);
return 1;default: _T8=lexbuf;_T9=_T8->refill_buff;
_T9(lexbuf);_TA=
Cyc_block_comment_rec(lexbuf,lexstate);return _TA;}{struct Cyc_Lexing_Error_exn_struct*_TD=_cycalloc(sizeof(struct Cyc_Lexing_Error_exn_struct));_TD->tag=Cyc_Lexing_Error;
# 859
_TD->f1=_tag_fat("some action didn't return!",sizeof(char),27U);_TB=(struct Cyc_Lexing_Error_exn_struct*)_TD;}_TC=(void*)_TB;_throw(_TC);}
# 861
int Cyc_block_comment(struct Cyc_Lexing_lexbuf*lexbuf){int _T0;_T0=Cyc_block_comment_rec(lexbuf,18);return _T0;}
# 863 "buildlib.cyl"
void Cyc_scan_type(void*,struct Cyc_Hashtable_Table*);struct _tuple18{struct Cyc_List_List*f0;struct Cyc_Absyn_Exp*f1;};
void Cyc_scan_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Hashtable_Table*dep){struct Cyc_Absyn_Exp*_T0;int*_T1;unsigned _T2;void*_T3;struct _tuple1*_T4;struct _tuple1 _T5;struct Cyc_List_List*_T6;void*_T7;struct Cyc_Absyn_Exp*_T8;struct Cyc_Hashtable_Table*_T9;struct Cyc_List_List*_TA;struct Cyc_List_List*_TB;void*_TC;struct Cyc_Absyn_Exp*_TD;struct Cyc_Hashtable_Table*_TE;struct Cyc_List_List*_TF;void*_T10;struct Cyc_Absyn_MallocInfo _T11;struct Cyc_Absyn_MallocInfo _T12;struct Cyc_Absyn_MallocInfo _T13;struct Cyc_Absyn_MallocInfo _T14;struct Cyc_Absyn_MallocInfo _T15;void**_T16;void*_T17;struct Cyc_Hashtable_Table*_T18;void*_T19;void*_T1A;void*_T1B;struct Cyc_List_List*_T1C;struct Cyc_List_List*_T1D;void*_T1E;struct Cyc_List_List*_T1F;struct Cyc___cycFILE*_T20;struct _fat_ptr _T21;struct _fat_ptr _T22;struct Cyc___cycFILE*_T23;struct _fat_ptr _T24;struct _fat_ptr _T25;struct Cyc___cycFILE*_T26;struct _fat_ptr _T27;struct _fat_ptr _T28;struct Cyc___cycFILE*_T29;struct _fat_ptr _T2A;struct _fat_ptr _T2B;struct Cyc___cycFILE*_T2C;struct _fat_ptr _T2D;struct _fat_ptr _T2E;struct Cyc___cycFILE*_T2F;struct _fat_ptr _T30;struct _fat_ptr _T31;struct Cyc___cycFILE*_T32;struct _fat_ptr _T33;struct _fat_ptr _T34;struct Cyc___cycFILE*_T35;struct _fat_ptr _T36;struct _fat_ptr _T37;struct Cyc___cycFILE*_T38;struct _fat_ptr _T39;struct _fat_ptr _T3A;struct Cyc___cycFILE*_T3B;struct _fat_ptr _T3C;struct _fat_ptr _T3D;struct Cyc___cycFILE*_T3E;struct _fat_ptr _T3F;struct _fat_ptr _T40;struct Cyc___cycFILE*_T41;struct _fat_ptr _T42;struct _fat_ptr _T43;struct Cyc___cycFILE*_T44;struct _fat_ptr _T45;struct _fat_ptr _T46;struct Cyc___cycFILE*_T47;struct _fat_ptr _T48;struct _fat_ptr _T49;struct Cyc___cycFILE*_T4A;struct _fat_ptr _T4B;struct _fat_ptr _T4C;struct Cyc___cycFILE*_T4D;struct _fat_ptr _T4E;struct _fat_ptr _T4F;struct Cyc___cycFILE*_T50;struct _fat_ptr _T51;struct _fat_ptr _T52;struct Cyc___cycFILE*_T53;struct _fat_ptr _T54;struct _fat_ptr _T55;_T0=
_check_null(e);{void*_T56=_T0->r;struct _fat_ptr*_T57;void**_T58;enum Cyc_Absyn_MallocKind _T59;struct Cyc_List_List*_T5A;struct Cyc_Absyn_Exp*_T5B;struct Cyc_Absyn_Exp*_T5C;void*_T5D;_T1=(int*)_T56;_T2=*_T1;switch(_T2){case 1:{struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Var_e_Absyn_Raw_exp_struct*)_T56;_T3=_T5E->f1;_T5D=(void*)_T3;}{void*b=_T5D;_T4=
# 867
Cyc_Absyn_binding2qvar(b);_T5=*_T4;{struct _fat_ptr*v=_T5.f1;
Cyc_add_target(v);
return;}}case 3:{struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Primop_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f2;}{struct Cyc_List_List*x=_T5D;
# 871
_TL8A: if(x!=0)goto _TL88;else{goto _TL89;}
_TL88: _T6=x;_T7=_T6->hd;_T8=(struct Cyc_Absyn_Exp*)_T7;_T9=dep;Cyc_scan_exp(_T8,_T9);_TA=x;
# 871
x=_TA->tl;goto _TL8A;_TL89:
# 874
 return;}case 23:{struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Subscript_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;_T5C=_T5E->f2;}{struct Cyc_Absyn_Exp*e1=_T5D;struct Cyc_Absyn_Exp*e2=_T5C;_T5D=e1;_T5C=e2;goto _LL8;}case 9:{struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_SeqExp_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;_T5C=_T5E->f2;}_LL8: {struct Cyc_Absyn_Exp*e1=_T5D;struct Cyc_Absyn_Exp*e2=_T5C;_T5D=e1;_T5C=e2;goto _LLA;}case 4:{struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_AssignOp_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;_T5C=_T5E->f3;}_LLA: {struct Cyc_Absyn_Exp*e1=_T5D;struct Cyc_Absyn_Exp*e2=_T5C;
# 880
Cyc_scan_exp(e1,dep);
Cyc_scan_exp(e2,dep);
return;}case 40:{struct Cyc_Absyn_Extension_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Extension_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;}{struct Cyc_Absyn_Exp*e1=_T5D;_T5D=e1;goto _LLE;}case 20:{struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Deref_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;}_LLE: {struct Cyc_Absyn_Exp*e1=_T5D;_T5D=e1;goto _LL10;}case 18:{struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Sizeofexp_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;}_LL10: {struct Cyc_Absyn_Exp*e1=_T5D;_T5D=e1;goto _LL12;}case 15:{struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Address_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;}_LL12: {struct Cyc_Absyn_Exp*e1=_T5D;_T5D=e1;goto _LL14;}case 5:{struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Increment_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;}_LL14: {struct Cyc_Absyn_Exp*e1=_T5D;
# 891
Cyc_scan_exp(e1,dep);
return;}case 6:{struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Conditional_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;_T5C=_T5E->f2;_T5B=_T5E->f3;}{struct Cyc_Absyn_Exp*e1=_T5D;struct Cyc_Absyn_Exp*e2=_T5C;struct Cyc_Absyn_Exp*e3=_T5B;
# 894
Cyc_scan_exp(e1,dep);
Cyc_scan_exp(e2,dep);
Cyc_scan_exp(e3,dep);
return;}case 7:{struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_And_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;_T5C=_T5E->f2;}{struct Cyc_Absyn_Exp*e1=_T5D;struct Cyc_Absyn_Exp*e2=_T5C;_T5D=e1;_T5C=e2;goto _LL1A;}case 8:{struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Or_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;_T5C=_T5E->f2;}_LL1A: {struct Cyc_Absyn_Exp*e1=_T5D;struct Cyc_Absyn_Exp*e2=_T5C;
# 900
Cyc_scan_exp(e1,dep);
Cyc_scan_exp(e2,dep);
return;}case 10:{struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_FnCall_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;_T5A=_T5E->f2;}{struct Cyc_Absyn_Exp*e1=_T5D;struct Cyc_List_List*x=_T5A;
# 904
Cyc_scan_exp(e1,dep);
_TL8E: if(x!=0)goto _TL8C;else{goto _TL8D;}
_TL8C: _TB=x;_TC=_TB->hd;_TD=(struct Cyc_Absyn_Exp*)_TC;_TE=dep;Cyc_scan_exp(_TD,_TE);_TF=x;
# 905
x=_TF->tl;goto _TL8E;_TL8D:
# 908
 return;}case 14:{struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Cast_e_Absyn_Raw_exp_struct*)_T56;_T10=_T5E->f1;_T5D=(void*)_T10;_T5C=_T5E->f2;}{void*t1=_T5D;struct Cyc_Absyn_Exp*e1=_T5C;
# 910
Cyc_scan_type(t1,dep);
Cyc_scan_exp(e1,dep);
return;}case 33:{struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Malloc_e_Absyn_Raw_exp_struct*)_T56;_T11=_T5E->f1;_T59=_T11.mknd;_T12=_T5E->f1;_T5D=_T12.rgn;_T13=_T5E->f1;_T5C=_T13.aqual;_T14=_T5E->f1;_T58=_T14.elt_type;_T15=_T5E->f1;_T5B=_T15.num_elts;}{enum Cyc_Absyn_MallocKind mknd=_T59;struct Cyc_Absyn_Exp*ropt=_T5D;struct Cyc_Absyn_Exp*aqopt=_T5C;void**topt=_T58;struct Cyc_Absyn_Exp*e=_T5B;
# 914
if(ropt==0)goto _TL8F;Cyc_scan_exp(ropt,dep);goto _TL90;_TL8F: _TL90:
 if(aqopt==0)goto _TL91;Cyc_scan_exp(aqopt,dep);goto _TL92;_TL91: _TL92:
 if(topt==0)goto _TL93;_T16=topt;_T17=*_T16;_T18=dep;Cyc_scan_type(_T17,_T18);goto _TL94;_TL93: _TL94:
 Cyc_scan_exp(e,dep);
return;}case 37:{struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Tagcheck_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;}{struct Cyc_Absyn_Exp*e=_T5D;
# 920
Cyc_scan_exp(e,dep);return;}case 38:{struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Valueof_e_Absyn_Raw_exp_struct*)_T56;_T19=_T5E->f1;_T5D=(void*)_T19;}{void*t1=_T5D;_T5D=t1;goto _LL26;}case 17:{struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Sizeoftype_e_Absyn_Raw_exp_struct*)_T56;_T1A=_T5E->f1;_T5D=(void*)_T1A;}_LL26: {void*t1=_T5D;
# 923
Cyc_scan_type(t1,dep);
return;}case 21:{struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_AggrMember_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;_T57=_T5E->f2;}{struct Cyc_Absyn_Exp*e1=_T5D;struct _fat_ptr*fn=_T57;_T5D=e1;_T57=fn;goto _LL2A;}case 22:{struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_AggrArrow_e_Absyn_Raw_exp_struct*)_T56;_T5D=_T5E->f1;_T57=_T5E->f2;}_LL2A: {struct Cyc_Absyn_Exp*e1=_T5D;struct _fat_ptr*fn=_T57;
# 928
Cyc_scan_exp(e1,dep);
Cyc_add_target(fn);
return;}case 19:{struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_Offsetof_e_Absyn_Raw_exp_struct*)_T56;_T1B=_T5E->f1;_T5D=(void*)_T1B;_T5A=_T5E->f2;}{void*t1=_T5D;struct Cyc_List_List*f=_T5A;
# 932
Cyc_scan_type(t1,dep);_T1C=
# 934
_check_null(f);{void*_T5E=_T1C->hd;struct _fat_ptr*_T5F;{struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct*_T60=(struct Cyc_Absyn_StructField_Absyn_OffsetofField_struct*)_T5E;_T5F=_T60->f1;}{struct _fat_ptr*fn=_T5F;
Cyc_add_target(fn);goto _LL57;}_LL57:;}
# 937
return;}case 0:
# 939
 return;case 35:{struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*_T5E=(struct Cyc_Absyn_UnresolvedMem_e_Absyn_Raw_exp_struct*)_T56;_T5A=_T5E->f2;}{struct Cyc_List_List*x=_T5A;
# 941
_TL98: if(x!=0)goto _TL96;else{goto _TL97;}
_TL96: _T1D=x;_T1E=_T1D->hd;{struct _tuple18*_T5E=(struct _tuple18*)_T1E;struct Cyc_Absyn_Exp*_T5F;{struct _tuple18 _T60=*_T5E;_T5F=_T60.f1;}{struct Cyc_Absyn_Exp*e1=_T5F;
Cyc_scan_exp(e1,dep);}}_T1F=x;
# 941
x=_T1F->tl;goto _TL98;_TL97:
# 945
 return;}case 39:
 return;case 2: _T20=Cyc_stderr;_T21=
# 948
_tag_fat("Error: unexpected Pragma_e\n",sizeof(char),28U);_T22=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T20,_T21,_T22);
exit(1);return;case 34: _T23=Cyc_stderr;_T24=
# 951
_tag_fat("Error: unexpected Swap_e\n",sizeof(char),26U);_T25=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T23,_T24,_T25);
exit(1);return;case 36: _T26=Cyc_stderr;_T27=
# 954
_tag_fat("Error: unexpected Stmt_e\n",sizeof(char),26U);_T28=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T26,_T27,_T28);
exit(1);return;case 41: _T29=Cyc_stderr;_T2A=
# 957
_tag_fat("Error: unexpected Assert_e\n",sizeof(char),28U);_T2B=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T29,_T2A,_T2B);
exit(1);return;case 42: _T2C=Cyc_stderr;_T2D=
# 960
_tag_fat("Error: unexpected Assert_false_e\n",sizeof(char),34U);_T2E=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T2C,_T2D,_T2E);
exit(1);return;case 11: _T2F=Cyc_stderr;_T30=
# 963
_tag_fat("Error: unexpected Throw_e\n",sizeof(char),27U);_T31=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T2F,_T30,_T31);
exit(1);return;case 12: _T32=Cyc_stderr;_T33=
# 966
_tag_fat("Error: unexpected NoInstantiate_e\n",sizeof(char),35U);_T34=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T32,_T33,_T34);
exit(1);return;case 13: _T35=Cyc_stderr;_T36=
# 969
_tag_fat("Error: unexpected Instantiate_e\n",sizeof(char),33U);_T37=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T35,_T36,_T37);
exit(1);return;case 16: _T38=Cyc_stderr;_T39=
# 972
_tag_fat("Error: unexpected New_e\n",sizeof(char),25U);_T3A=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T38,_T39,_T3A);
exit(1);return;case 24: _T3B=Cyc_stderr;_T3C=
# 975
_tag_fat("Error: unexpected CompoundLit_e\n",sizeof(char),33U);_T3D=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T3B,_T3C,_T3D);
exit(1);return;case 25: _T3E=Cyc_stderr;_T3F=
# 978
_tag_fat("Error: unexpected Array_e\n",sizeof(char),27U);_T40=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T3E,_T3F,_T40);
exit(1);return;case 26: _T41=Cyc_stderr;_T42=
# 981
_tag_fat("Error: unexpected Comprehension_e\n",sizeof(char),35U);_T43=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T41,_T42,_T43);
exit(1);return;case 27: _T44=Cyc_stderr;_T45=
# 984
_tag_fat("Error: unexpected ComprehensionNoinit_e\n",sizeof(char),41U);_T46=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T44,_T45,_T46);
exit(1);return;case 28: _T47=Cyc_stderr;_T48=
# 987
_tag_fat("Error: unexpected Aggregate_e\n",sizeof(char),31U);_T49=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T47,_T48,_T49);
exit(1);return;case 29: _T4A=Cyc_stderr;_T4B=
# 990
_tag_fat("Error: unexpected AnonStruct_e\n",sizeof(char),32U);_T4C=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T4A,_T4B,_T4C);
exit(1);return;case 30: _T4D=Cyc_stderr;_T4E=
# 993
_tag_fat("Error: unexpected Datatype_e\n",sizeof(char),30U);_T4F=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T4D,_T4E,_T4F);
exit(1);return;case 31: _T50=Cyc_stderr;_T51=
# 996
_tag_fat("Error: unexpected Enum_e\n",sizeof(char),26U);_T52=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T50,_T51,_T52);
exit(1);return;default: _T53=Cyc_stderr;_T54=
# 999
_tag_fat("Error: unexpected AnonEnum_e\n",sizeof(char),30U);_T55=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T53,_T54,_T55);
exit(1);return;};}}
# 1004
void Cyc_scan_exp_opt(struct Cyc_Absyn_Exp*eo,struct Cyc_Hashtable_Table*dep){struct Cyc_Absyn_Exp*_T0;unsigned _T1;_T0=eo;_T1=(unsigned)_T0;
if(!_T1)goto _TL99;Cyc_scan_exp(eo,dep);goto _TL9A;_TL99: _TL9A:
 return;}
# 1009
void Cyc_scan_decl(struct Cyc_Absyn_Decl*,struct Cyc_Hashtable_Table*);
void Cyc_scan_type(void*t,struct Cyc_Hashtable_Table*dep){void*_T0;int*_T1;unsigned _T2;void*_T3;void*_T4;void*_T5;int*_T6;unsigned _T7;void*_T8;struct _tuple1*_T9;void*_TA;struct _tuple1*_TB;struct Cyc_String_pa_PrintArg_struct _TC;void**_TD;struct Cyc___cycFILE*_TE;struct _fat_ptr _TF;struct _fat_ptr _T10;void*_T11;struct Cyc_Absyn_PtrInfo _T12;void*_T13;struct Cyc_Hashtable_Table*_T14;void*_T15;struct Cyc_Absyn_ArrayInfo _T16;struct Cyc_Absyn_ArrayInfo _T17;struct Cyc_Absyn_ArrayInfo _T18;void*_T19;void*_T1A;struct Cyc_Absyn_FnInfo _T1B;void*_T1C;struct Cyc_Hashtable_Table*_T1D;struct Cyc_Absyn_FnInfo _T1E;struct Cyc_List_List*_T1F;void*_T20;struct Cyc_List_List*_T21;struct Cyc_Absyn_FnInfo _T22;struct Cyc_Absyn_VarargInfo*_T23;struct Cyc_Absyn_FnInfo _T24;struct Cyc_Absyn_VarargInfo*_T25;void*_T26;struct Cyc_Hashtable_Table*_T27;void*_T28;struct Cyc_List_List*_T29;void*_T2A;struct Cyc_Absyn_Aggrfield*_T2B;void*_T2C;struct Cyc_Hashtable_Table*_T2D;struct Cyc_List_List*_T2E;void*_T2F;struct Cyc_Absyn_Aggrfield*_T30;struct Cyc_Absyn_Exp*_T31;struct Cyc_Hashtable_Table*_T32;struct Cyc_List_List*_T33;void*_T34;struct _tuple1*_T35;void*_T36;struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*_T37;struct Cyc_Absyn_TypeDecl*_T38;struct Cyc_Absyn_TypeDecl*_T39;void*_T3A;int*_T3B;unsigned _T3C;void*_T3D;struct Cyc_Absyn_TypeDecl*_T3E;void*_T3F;struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*_T40;void*_T41;struct Cyc_Absyn_Decl*_T42;struct Cyc_Hashtable_Table*_T43;struct Cyc_Absyn_Aggrdecl*_T44;void*_T45;struct Cyc_Absyn_TypeDecl*_T46;void*_T47;struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*_T48;void*_T49;struct Cyc_Absyn_Decl*_T4A;struct Cyc_Hashtable_Table*_T4B;struct Cyc_Absyn_Enumdecl*_T4C;void*_T4D;struct Cyc_Absyn_TypeDecl*_T4E;void*_T4F;struct Cyc___cycFILE*_T50;struct _fat_ptr _T51;struct _fat_ptr _T52;struct Cyc___cycFILE*_T53;struct _fat_ptr _T54;struct _fat_ptr _T55;struct Cyc___cycFILE*_T56;struct _fat_ptr _T57;struct _fat_ptr _T58;struct Cyc___cycFILE*_T59;struct _fat_ptr _T5A;struct _fat_ptr _T5B;struct Cyc___cycFILE*_T5C;struct _fat_ptr _T5D;struct _fat_ptr _T5E;struct Cyc_Absyn_Datatypedecl*_T5F;struct Cyc_Absyn_Enumdecl*_T60;struct Cyc_Absyn_Aggrdecl*_T61;struct _fat_ptr*_T62;struct Cyc_List_List*_T63;struct Cyc_Absyn_FnInfo _T64;struct Cyc_Absyn_Exp*_T65;struct Cyc_Absyn_PtrInfo _T66;void*_T67;void*_T68;_T0=t;_T1=(int*)_T0;_T2=*_T1;switch(_T2){case 0: _T3=t;{struct Cyc_Absyn_AppType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_AppType_Absyn_Type_struct*)_T3;_T4=_T69->f1;_T68=(void*)_T4;_T67=_T69->f2;}{void*c=_T68;struct Cyc_List_List*ts=_T67;struct _fat_ptr*_T69;union Cyc_Absyn_AggrInfo _T6A;_T5=c;_T6=(int*)_T5;_T7=*_T6;switch(_T7){case 0: goto _LL21;case 1: _LL21: goto _LL23;case 21: _LL23: goto _LL25;case 2: _LL25: goto _LL27;case 3: _LL27: goto _LL29;case 20: _LL29:
# 1020
 return;case 24: _T8=c;{struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*_T6B=(struct Cyc_Absyn_AggrCon_Absyn_TyCon_struct*)_T8;_T6A=_T6B->f1;}{union Cyc_Absyn_AggrInfo info=_T6A;
# 1022
struct _tuple12 _T6B=Cyc_Absyn_aggr_kinded_name(info);struct _fat_ptr*_T6C;_T9=_T6B.f1;{struct _tuple1 _T6D=*_T9;_T6C=_T6D.f1;}{struct _fat_ptr*v=_T6C;_T69=v;goto _LL2D;}}case 19: _TA=c;{struct Cyc_Absyn_EnumCon_Absyn_TyCon_struct*_T6B=(struct Cyc_Absyn_EnumCon_Absyn_TyCon_struct*)_TA;_TB=_T6B->f1;{struct _tuple1 _T6C=*_TB;_T69=_T6C.f1;}}_LL2D: {struct _fat_ptr*v=_T69;
# 1024
Cyc_add_target(v);return;}case 22: goto _LL31;case 23: _LL31: goto _LL33;case 4: _LL33: goto _LL35;case 6: _LL35: goto _LL37;case 7: _LL37: goto _LL39;case 8: _LL39: goto _LL3B;case 9: _LL3B: goto _LL3D;case 10: _LL3D: goto _LL3F;case 5: _LL3F: goto _LL41;case 11: _LL41: goto _LL43;case 12: _LL43: goto _LL45;case 13: _LL45: goto _LL47;case 14: _LL47: goto _LL49;case 15: _LL49: goto _LL4B;case 16: _LL4B: goto _LL4D;case 18: _LL4D: goto _LL4F;default: _LL4F:{struct Cyc_String_pa_PrintArg_struct _T6B;_T6B.tag=0;
# 1034
_T6B.f1=Cyc_Absynpp_typ2string(t);_TC=_T6B;}{struct Cyc_String_pa_PrintArg_struct _T6B=_TC;void*_T6C[1];_TD=_T6C + 0;*_TD=& _T6B;_TE=Cyc_stderr;_TF=_tag_fat("Error: unexpected %s\n",sizeof(char),22U);_T10=_tag_fat(_T6C,sizeof(void*),1);Cyc_fprintf(_TE,_TF,_T10);}
exit(1);return;};}case 4: _T11=t;{struct Cyc_Absyn_PointerType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_PointerType_Absyn_Type_struct*)_T11;_T66=_T69->f1;}{struct Cyc_Absyn_PtrInfo x=_T66;_T12=x;_T13=_T12.elt_type;_T14=dep;
# 1039
Cyc_scan_type(_T13,_T14);
return;}case 5: _T15=t;{struct Cyc_Absyn_ArrayType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_ArrayType_Absyn_Type_struct*)_T15;_T16=_T69->f1;_T68=_T16.elt_type;_T17=_T69->f1;_T65=_T17.num_elts;_T18=_T69->f1;_T67=_T18.zero_term;}{void*t=_T68;struct Cyc_Absyn_Exp*sz=_T65;void*zt=_T67;
# 1042
Cyc_scan_type(t,dep);
Cyc_scan_exp_opt(sz,dep);
return;}case 11: _T19=t;{struct Cyc_Absyn_TypeofType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_TypeofType_Absyn_Type_struct*)_T19;_T65=_T69->f1;}{struct Cyc_Absyn_Exp*e=_T65;
# 1046
Cyc_scan_exp(e,dep);
return;}case 6: _T1A=t;{struct Cyc_Absyn_FnType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_FnType_Absyn_Type_struct*)_T1A;_T64=_T69->f1;}{struct Cyc_Absyn_FnInfo x=_T64;_T1B=x;_T1C=_T1B.ret_type;_T1D=dep;
# 1049
Cyc_scan_type(_T1C,_T1D);_T1E=x;{
struct Cyc_List_List*a=_T1E.args;_TLA0: if(a!=0)goto _TL9E;else{goto _TL9F;}
_TL9E: _T1F=a;_T20=_T1F->hd;{struct _tuple9*_T69=(struct _tuple9*)_T20;void*_T6A;{struct _tuple9 _T6B=*_T69;_T6A=_T6B.f2;}{void*argt=_T6A;
Cyc_scan_type(argt,dep);}}_T21=a;
# 1050
a=_T21->tl;goto _TLA0;_TL9F:;}_T22=x;_T23=_T22.cyc_varargs;
# 1054
if(_T23==0)goto _TLA1;_T24=x;_T25=_T24.cyc_varargs;_T26=_T25->type;_T27=dep;
Cyc_scan_type(_T26,_T27);goto _TLA2;_TLA1: _TLA2:
 return;}case 7: _T28=t;{struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_AnonAggrType_Absyn_Type_struct*)_T28;_T63=_T69->f3;}{struct Cyc_List_List*sfs=_T63;
# 1058
_TLA6: if(sfs!=0)goto _TLA4;else{goto _TLA5;}
_TLA4: _T29=sfs;_T2A=_T29->hd;_T2B=(struct Cyc_Absyn_Aggrfield*)_T2A;_T2C=_T2B->type;_T2D=dep;Cyc_scan_type(_T2C,_T2D);_T2E=sfs;_T2F=_T2E->hd;_T30=(struct Cyc_Absyn_Aggrfield*)_T2F;_T31=_T30->width;_T32=dep;
Cyc_scan_exp_opt(_T31,_T32);_T33=sfs;
# 1058
sfs=_T33->tl;goto _TLA6;_TLA5:
# 1062
 return;}case 8: _T34=t;{struct Cyc_Absyn_TypedefType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_TypedefType_Absyn_Type_struct*)_T34;_T35=_T69->f1;{struct _tuple1 _T6A=*_T35;_T62=_T6A.f1;}}{struct _fat_ptr*v=_T62;
# 1064
Cyc_add_target(v);
return;}case 10: _T36=t;_T37=(struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*)_T36;_T38=_T37->f1;_T39=(struct Cyc_Absyn_TypeDecl*)_T38;_T3A=_T39->r;_T3B=(int*)_T3A;_T3C=*_T3B;switch(_T3C){case 0: _T3D=t;{struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*)_T3D;_T3E=_T69->f1;{struct Cyc_Absyn_TypeDecl _T6A=*_T3E;_T3F=_T6A.r;{struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct*_T6B=(struct Cyc_Absyn_Aggr_td_Absyn_Raw_typedecl_struct*)_T3F;_T61=_T6B->f1;}}}{struct Cyc_Absyn_Aggrdecl*x=_T61;{struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*_T69=_cycalloc(sizeof(struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct));_T69->tag=5;
# 1068
_T69->f1=x;_T40=(struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_T69;}_T41=(void*)_T40;_T42=Cyc_Absyn_new_decl(_T41,0U);_T43=dep;Cyc_scan_decl(_T42,_T43);_T44=x;{
struct _tuple1*_T69=_T44->name;struct _fat_ptr*_T6A;{struct _tuple1 _T6B=*_T69;_T6A=_T6B.f1;}{struct _fat_ptr*n=_T6A;
Cyc_add_target(n);
return;}}}case 1: _T45=t;{struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*)_T45;_T46=_T69->f1;{struct Cyc_Absyn_TypeDecl _T6A=*_T46;_T47=_T6A.r;{struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct*_T6B=(struct Cyc_Absyn_Enum_td_Absyn_Raw_typedecl_struct*)_T47;_T60=_T6B->f1;}}}{struct Cyc_Absyn_Enumdecl*x=_T60;{struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*_T69=_cycalloc(sizeof(struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct));_T69->tag=7;
# 1074
_T69->f1=x;_T48=(struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_T69;}_T49=(void*)_T48;_T4A=Cyc_Absyn_new_decl(_T49,0U);_T4B=dep;Cyc_scan_decl(_T4A,_T4B);_T4C=x;{
struct _tuple1*_T69=_T4C->name;struct _fat_ptr*_T6A;{struct _tuple1 _T6B=*_T69;_T6A=_T6B.f1;}{struct _fat_ptr*n=_T6A;
Cyc_add_target(n);
return;}}}default: _T4D=t;{struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*_T69=(struct Cyc_Absyn_TypeDeclType_Absyn_Type_struct*)_T4D;_T4E=_T69->f1;{struct Cyc_Absyn_TypeDecl _T6A=*_T4E;_T4F=_T6A.r;{struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct*_T6B=(struct Cyc_Absyn_Datatype_td_Absyn_Raw_typedecl_struct*)_T4F;_T5F=_T6B->f1;}}}{struct Cyc_Absyn_Datatypedecl*dd=_T5F;_T50=Cyc_stderr;_T51=
# 1080
_tag_fat("Error: unexpected Datatype declaration\n",sizeof(char),40U);_T52=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T50,_T51,_T52);
exit(1);return;}};case 3: _T53=Cyc_stderr;_T54=
# 1083
_tag_fat("Error: unexpected Cvar\n",sizeof(char),24U);_T55=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T53,_T54,_T55);
exit(1);return;case 1: _T56=Cyc_stderr;_T57=
# 1086
_tag_fat("Error: unexpected Evar\n",sizeof(char),24U);_T58=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T56,_T57,_T58);
exit(1);return;case 2: _T59=Cyc_stderr;_T5A=
# 1089
_tag_fat("Error: unexpected VarType\n",sizeof(char),27U);_T5B=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T59,_T5A,_T5B);
exit(1);return;default: _T5C=Cyc_stderr;_T5D=
# 1092
_tag_fat("Error: unexpected valueof_t\n",sizeof(char),29U);_T5E=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T5C,_T5D,_T5E);
exit(1);return;};}
# 1097
void Cyc_scan_decl(struct Cyc_Absyn_Decl*d,struct Cyc_Hashtable_Table*dep){struct Cyc_Set_Set**_T0;struct Cyc_Set_Set*(*_T1)(int(*)(struct _fat_ptr*,struct _fat_ptr*));struct Cyc_Set_Set*(*_T2)(int(*)(void*,void*));int(*_T3)(struct _fat_ptr*,struct _fat_ptr*);struct Cyc_Absyn_Decl*_T4;int*_T5;unsigned _T6;struct Cyc_Absyn_Vardecl*_T7;struct Cyc_Absyn_Vardecl*_T8;void*_T9;struct Cyc_Hashtable_Table*_TA;struct Cyc_Absyn_Vardecl*_TB;struct Cyc_Absyn_Exp*_TC;struct Cyc_Hashtable_Table*_TD;struct Cyc_Absyn_Fndecl*_TE;struct Cyc_Absyn_Fndecl*_TF;struct Cyc_Absyn_FnInfo _T10;void*_T11;struct Cyc_Hashtable_Table*_T12;struct Cyc_Absyn_Fndecl*_T13;struct Cyc_Absyn_FnInfo _T14;struct Cyc_List_List*_T15;void*_T16;struct Cyc_List_List*_T17;struct Cyc_Absyn_Fndecl*_T18;struct Cyc_Absyn_FnInfo _T19;struct Cyc_Absyn_VarargInfo*_T1A;struct Cyc_Absyn_Fndecl*_T1B;struct Cyc_Absyn_FnInfo _T1C;struct Cyc_Absyn_VarargInfo*_T1D;void*_T1E;struct Cyc_Hashtable_Table*_T1F;struct Cyc_Absyn_Fndecl*_T20;int _T21;struct Cyc_String_pa_PrintArg_struct _T22;struct _fat_ptr*_T23;void**_T24;struct _fat_ptr _T25;struct _fat_ptr _T26;struct Cyc_Absyn_Aggrdecl*_T27;struct Cyc_Absyn_Aggrdecl*_T28;struct Cyc_Absyn_AggrdeclImpl*_T29;unsigned _T2A;struct Cyc_Absyn_Aggrdecl*_T2B;struct Cyc_Absyn_AggrdeclImpl*_T2C;struct Cyc_List_List*_T2D;void*_T2E;struct Cyc_Absyn_Aggrfield*_T2F;void*_T30;struct Cyc_Hashtable_Table*_T31;struct Cyc_Absyn_Aggrfield*_T32;struct Cyc_Absyn_Exp*_T33;struct Cyc_Hashtable_Table*_T34;struct Cyc_List_List*_T35;struct Cyc_Absyn_Aggrdecl*_T36;struct Cyc_Absyn_AggrdeclImpl*_T37;struct Cyc_Absyn_AggrdeclImpl*_T38;struct Cyc_List_List*_T39;struct Cyc_Absyn_Enumdecl*_T3A;struct Cyc_Absyn_Enumdecl*_T3B;struct Cyc_Core_Opt*_T3C;unsigned _T3D;struct Cyc_Absyn_Enumdecl*_T3E;struct Cyc_Core_Opt*_T3F;void*_T40;struct Cyc_List_List*_T41;void*_T42;struct Cyc_Absyn_Enumfield*_T43;struct Cyc_Absyn_Exp*_T44;struct Cyc_Hashtable_Table*_T45;struct Cyc_List_List*_T46;struct Cyc_Absyn_Enumdecl*_T47;struct Cyc_Core_Opt*_T48;struct Cyc_Core_Opt*_T49;void*_T4A;struct Cyc_List_List*_T4B;struct Cyc_Absyn_Typedefdecl*_T4C;struct Cyc_Absyn_Typedefdecl*_T4D;void*_T4E;unsigned _T4F;struct Cyc_Absyn_Typedefdecl*_T50;void*_T51;struct Cyc_Hashtable_Table*_T52;struct Cyc___cycFILE*_T53;struct _fat_ptr _T54;struct _fat_ptr _T55;struct Cyc___cycFILE*_T56;struct _fat_ptr _T57;struct _fat_ptr _T58;struct Cyc___cycFILE*_T59;struct _fat_ptr _T5A;struct _fat_ptr _T5B;struct Cyc___cycFILE*_T5C;struct _fat_ptr _T5D;struct _fat_ptr _T5E;struct Cyc___cycFILE*_T5F;struct _fat_ptr _T60;struct _fat_ptr _T61;struct Cyc___cycFILE*_T62;struct _fat_ptr _T63;struct _fat_ptr _T64;struct Cyc___cycFILE*_T65;struct _fat_ptr _T66;struct _fat_ptr _T67;struct Cyc___cycFILE*_T68;struct _fat_ptr _T69;struct _fat_ptr _T6A;struct Cyc___cycFILE*_T6B;struct _fat_ptr _T6C;struct _fat_ptr _T6D;struct Cyc___cycFILE*_T6E;struct _fat_ptr _T6F;struct _fat_ptr _T70;struct Cyc___cycFILE*_T71;struct _fat_ptr _T72;struct _fat_ptr _T73;struct Cyc___cycFILE*_T74;struct _fat_ptr _T75;struct _fat_ptr _T76;struct _handler_cons*_T77;int*_T78;int _T79;struct Cyc_Set_Set*(*_T7A)(struct Cyc_Hashtable_Table*,struct _fat_ptr*);void*(*_T7B)(struct Cyc_Hashtable_Table*,void*);void*_T7C;struct Cyc_Core_Not_found_exn_struct*_T7D;char*_T7E;char*_T7F;struct Cyc_Set_Set*(*_T80)(int(*)(struct _fat_ptr*,struct _fat_ptr*));struct Cyc_Set_Set*(*_T81)(int(*)(void*,void*));int(*_T82)(struct _fat_ptr*,struct _fat_ptr*);struct Cyc_Set_Set**_T83;struct Cyc_Set_Set*_T84;struct Cyc_Set_Set*_T85;void(*_T86)(struct Cyc_Hashtable_Table*,struct _fat_ptr*,struct Cyc_Set_Set*);void(*_T87)(struct Cyc_Hashtable_Table*,void*,void*);
struct Cyc_Set_Set**saved_targets=Cyc_current_targets;
struct _fat_ptr*saved_source=Cyc_current_source;{struct Cyc_Set_Set**_T88=_cycalloc(sizeof(struct Cyc_Set_Set*));_T2=Cyc_Set_empty;{
struct Cyc_Set_Set*(*_T89)(int(*)(struct _fat_ptr*,struct _fat_ptr*))=(struct Cyc_Set_Set*(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*)))_T2;_T1=_T89;}_T3=Cyc_strptrcmp;*_T88=_T1(_T3);_T0=(struct Cyc_Set_Set**)_T88;}Cyc_current_targets=_T0;_T4=d;{
void*_T88=_T4->r;struct Cyc_Absyn_Typedefdecl*_T89;struct Cyc_Absyn_Enumdecl*_T8A;struct Cyc_Absyn_Aggrdecl*_T8B;struct Cyc_Absyn_Fndecl*_T8C;struct Cyc_Absyn_Vardecl*_T8D;_T5=(int*)_T88;_T6=*_T5;switch(_T6){case 0:{struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*_T8E=(struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_T88;_T8D=_T8E->f1;}{struct Cyc_Absyn_Vardecl*x=_T8D;_T7=x;{
# 1103
struct _tuple1*_T8E=_T7->name;struct _fat_ptr*_T8F;{struct _tuple1 _T90=*_T8E;_T8F=_T90.f1;}{struct _fat_ptr*v=_T8F;
Cyc_current_source=v;_T8=x;_T9=_T8->type;_TA=dep;
Cyc_scan_type(_T9,_TA);_TB=x;_TC=_TB->initializer;_TD=dep;
Cyc_scan_exp_opt(_TC,_TD);goto _LL0;}}}case 1:{struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*_T8E=(struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_T88;_T8C=_T8E->f1;}{struct Cyc_Absyn_Fndecl*x=_T8C;_TE=x;{
# 1109
struct _tuple1*_T8E=_TE->name;struct _fat_ptr*_T8F;{struct _tuple1 _T90=*_T8E;_T8F=_T90.f1;}{struct _fat_ptr*v=_T8F;
Cyc_current_source=v;_TF=x;_T10=_TF->i;_T11=_T10.ret_type;_T12=dep;
Cyc_scan_type(_T11,_T12);_T13=x;_T14=_T13->i;{
struct Cyc_List_List*a=_T14.args;_TLAC: if(a!=0)goto _TLAA;else{goto _TLAB;}
_TLAA: _T15=a;_T16=_T15->hd;{struct _tuple9*_T90=(struct _tuple9*)_T16;void*_T91;{struct _tuple9 _T92=*_T90;_T91=_T92.f2;}{void*t1=_T91;
Cyc_scan_type(t1,dep);}}_T17=a;
# 1112
a=_T17->tl;goto _TLAC;_TLAB:;}_T18=x;_T19=_T18->i;_T1A=_T19.cyc_varargs;
# 1116
if(_T1A==0)goto _TLAD;_T1B=x;_T1C=_T1B->i;_T1D=_T1C.cyc_varargs;_T1E=_T1D->type;_T1F=dep;
Cyc_scan_type(_T1E,_T1F);goto _TLAE;_TLAD: _TLAE: _T20=x;_T21=_T20->is_inline;
if(!_T21)goto _TLAF;{struct Cyc_String_pa_PrintArg_struct _T90;_T90.tag=0;_T23=v;
_T90.f1=*_T23;_T22=_T90;}{struct Cyc_String_pa_PrintArg_struct _T90=_T22;void*_T91[1];_T24=_T91 + 0;*_T24=& _T90;_T25=_tag_fat("Warning: ignoring inline function %s\n",sizeof(char),38U);_T26=_tag_fat(_T91,sizeof(void*),1);Cyc_log(_T25,_T26);}goto _TLB0;_TLAF: _TLB0: goto _LL0;}}}case 5:{struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*_T8E=(struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_T88;_T8B=_T8E->f1;}{struct Cyc_Absyn_Aggrdecl*x=_T8B;_T27=x;{
# 1122
struct _tuple1*_T8E=_T27->name;struct _fat_ptr*_T8F;{struct _tuple1 _T90=*_T8E;_T8F=_T90.f1;}{struct _fat_ptr*v=_T8F;
Cyc_current_source=v;_T28=x;_T29=_T28->impl;_T2A=(unsigned)_T29;
if(!_T2A)goto _TLB1;_T2B=x;_T2C=_T2B->impl;{
struct Cyc_List_List*fs=_T2C->fields;_TLB6: if(fs!=0)goto _TLB4;else{goto _TLB5;}
_TLB4: _T2D=fs;_T2E=_T2D->hd;{struct Cyc_Absyn_Aggrfield*f=(struct Cyc_Absyn_Aggrfield*)_T2E;_T2F=f;_T30=_T2F->type;_T31=dep;
Cyc_scan_type(_T30,_T31);_T32=f;_T33=_T32->width;_T34=dep;
Cyc_scan_exp_opt(_T33,_T34);}_T35=fs;
# 1125
fs=_T35->tl;goto _TLB6;_TLB5:;}_T36=x;_T37=_T36->impl;_T38=
# 1132
_check_null(_T37);{struct Cyc_List_List*fs=_T38->fields;_TLBA: if(fs!=0)goto _TLB8;else{goto _TLB9;}_TLB8: _T39=fs;fs=_T39->tl;goto _TLBA;_TLB9:;}goto _TLB2;_TLB1: _TLB2: goto _LL0;}}}case 7:{struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*_T8E=(struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_T88;_T8A=_T8E->f1;}{struct Cyc_Absyn_Enumdecl*x=_T8A;_T3A=x;{
# 1138
struct _tuple1*_T8E=_T3A->name;struct _fat_ptr*_T8F;{struct _tuple1 _T90=*_T8E;_T8F=_T90.f1;}{struct _fat_ptr*v=_T8F;
Cyc_current_source=v;_T3B=x;_T3C=_T3B->fields;_T3D=(unsigned)_T3C;
if(!_T3D)goto _TLBB;_T3E=x;_T3F=_T3E->fields;_T40=_T3F->v;{
struct Cyc_List_List*fs=(struct Cyc_List_List*)_T40;_TLC0: if(fs!=0)goto _TLBE;else{goto _TLBF;}
_TLBE: _T41=fs;_T42=_T41->hd;{struct Cyc_Absyn_Enumfield*f=(struct Cyc_Absyn_Enumfield*)_T42;_T43=f;_T44=_T43->tag;_T45=dep;
Cyc_scan_exp_opt(_T44,_T45);}_T46=fs;
# 1141
fs=_T46->tl;goto _TLC0;_TLBF:;}_T47=x;_T48=_T47->fields;_T49=
# 1147
_check_null(_T48);_T4A=_T49->v;{struct Cyc_List_List*fs=(struct Cyc_List_List*)_T4A;_TLC4: if(fs!=0)goto _TLC2;else{goto _TLC3;}_TLC2: _T4B=fs;fs=_T4B->tl;goto _TLC4;_TLC3:;}goto _TLBC;_TLBB: _TLBC: goto _LL0;}}}case 8:{struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*_T8E=(struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*)_T88;_T89=_T8E->f1;}{struct Cyc_Absyn_Typedefdecl*x=_T89;_T4C=x;{
# 1153
struct _tuple1*_T8E=_T4C->name;struct _fat_ptr*_T8F;{struct _tuple1 _T90=*_T8E;_T8F=_T90.f1;}{struct _fat_ptr*v=_T8F;
Cyc_current_source=v;_T4D=x;_T4E=_T4D->defn;_T4F=(unsigned)_T4E;
if(!_T4F)goto _TLC5;_T50=x;_T51=_T50->defn;_T52=dep;
Cyc_scan_type(_T51,_T52);goto _TLC6;_TLC5: _TLC6: goto _LL0;}}}case 4: _T53=Cyc_stderr;_T54=
# 1159
_tag_fat("Error: unexpected region declaration",sizeof(char),37U);_T55=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T53,_T54,_T55);
exit(1);case 13: _T56=Cyc_stderr;_T57=
# 1162
_tag_fat("Error: unexpected __cyclone_port_on__",sizeof(char),38U);_T58=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T56,_T57,_T58);
exit(1);case 14: _T59=Cyc_stderr;_T5A=
# 1165
_tag_fat("Error: unexpected __cyclone_port_off__",sizeof(char),39U);_T5B=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T59,_T5A,_T5B);
exit(1);case 15: _T5C=Cyc_stderr;_T5D=
# 1168
_tag_fat("Error: unexpected __tempest_on__",sizeof(char),33U);_T5E=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T5C,_T5D,_T5E);
exit(1);case 16: _T5F=Cyc_stderr;_T60=
# 1171
_tag_fat("Error: unexpected __tempest_off__",sizeof(char),34U);_T61=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T5F,_T60,_T61);
exit(1);case 2: _T62=Cyc_stderr;_T63=
# 1174
_tag_fat("Error: unexpected let declaration\n",sizeof(char),35U);_T64=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T62,_T63,_T64);
exit(1);case 6: _T65=Cyc_stderr;_T66=
# 1177
_tag_fat("Error: unexpected datatype declaration\n",sizeof(char),40U);_T67=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T65,_T66,_T67);
exit(1);case 3: _T68=Cyc_stderr;_T69=
# 1180
_tag_fat("Error: unexpected let declaration\n",sizeof(char),35U);_T6A=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T68,_T69,_T6A);
exit(1);case 9: _T6B=Cyc_stderr;_T6C=
# 1183
_tag_fat("Error: unexpected namespace declaration\n",sizeof(char),41U);_T6D=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T6B,_T6C,_T6D);
exit(1);case 10: _T6E=Cyc_stderr;_T6F=
# 1186
_tag_fat("Error: unexpected using declaration\n",sizeof(char),37U);_T70=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T6E,_T6F,_T70);
exit(1);case 11: _T71=Cyc_stderr;_T72=
# 1189
_tag_fat("Error: unexpected extern \"C\" declaration\n",sizeof(char),42U);_T73=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T71,_T72,_T73);
exit(1);default: _T74=Cyc_stderr;_T75=
# 1192
_tag_fat("Error: unexpected extern \"C include\" declaration\n",sizeof(char),50U);_T76=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T74,_T75,_T76);
exit(1);}_LL0:;}{
# 1200
struct Cyc_Set_Set*old;
struct _fat_ptr*name=_check_null(Cyc_current_source);{struct _handler_cons _T88;_T77=& _T88;_push_handler(_T77);{int _T89=0;_T78=_T88.handler;_T79=setjmp(_T78);if(!_T79)goto _TLC7;_T89=1;goto _TLC8;_TLC7: _TLC8: if(_T89)goto _TLC9;else{goto _TLCB;}_TLCB: _T7B=Cyc_Hashtable_lookup;{
# 1203
struct Cyc_Set_Set*(*_T8A)(struct Cyc_Hashtable_Table*,struct _fat_ptr*)=(struct Cyc_Set_Set*(*)(struct Cyc_Hashtable_Table*,struct _fat_ptr*))_T7B;_T7A=_T8A;}old=_T7A(dep,name);_pop_handler();goto _TLCA;_TLC9: _T7C=Cyc_Core_get_exn_thrown();{void*_T8A=(void*)_T7C;void*_T8B;_T7D=(struct Cyc_Core_Not_found_exn_struct*)_T8A;_T7E=_T7D->tag;_T7F=Cyc_Core_Not_found;if(_T7E!=_T7F)goto _TLCC;_T81=Cyc_Set_empty;{
# 1205
struct Cyc_Set_Set*(*_T8C)(int(*)(struct _fat_ptr*,struct _fat_ptr*))=(struct Cyc_Set_Set*(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*)))_T81;_T80=_T8C;}_T82=Cyc_strptrcmp;old=_T80(_T82);goto _LL35;_TLCC: _T8B=_T8A;{void*exn=_T8B;_rethrow(exn);}_LL35:;}_TLCA:;}}_T83=
# 1207
_check_null(Cyc_current_targets);_T84=*_T83;_T85=old;{struct Cyc_Set_Set*targets=Cyc_Set_union_two(_T84,_T85);_T87=Cyc_Hashtable_insert;{
void(*_T88)(struct Cyc_Hashtable_Table*,struct _fat_ptr*,struct Cyc_Set_Set*)=(void(*)(struct Cyc_Hashtable_Table*,struct _fat_ptr*,struct Cyc_Set_Set*))_T87;_T86=_T88;}_T86(dep,name,targets);
# 1210
Cyc_current_targets=saved_targets;
Cyc_current_source=saved_source;}}}
# 1214
struct Cyc_Hashtable_Table*Cyc_new_deps (void){struct Cyc_Hashtable_Table*(*_T0)(int,int(*)(struct _fat_ptr*,struct _fat_ptr*),int(*)(struct _fat_ptr*));struct Cyc_Hashtable_Table*(*_T1)(int,int(*)(void*,void*),int(*)(void*));int(*_T2)(struct _fat_ptr*,struct _fat_ptr*);int(*_T3)(struct _fat_ptr*);struct Cyc_Hashtable_Table*_T4;_T1=Cyc_Hashtable_create;{
struct Cyc_Hashtable_Table*(*_T5)(int,int(*)(struct _fat_ptr*,struct _fat_ptr*),int(*)(struct _fat_ptr*))=(struct Cyc_Hashtable_Table*(*)(int,int(*)(struct _fat_ptr*,struct _fat_ptr*),int(*)(struct _fat_ptr*)))_T1;_T0=_T5;}_T2=Cyc_strptrcmp;_T3=Cyc_Hashtable_hash_stringptr;_T4=_T0(107,_T2,_T3);return _T4;}
# 1218
struct Cyc_Set_Set*Cyc_find(struct Cyc_Hashtable_Table*t,struct _fat_ptr*x){struct _handler_cons*_T0;int*_T1;int _T2;struct Cyc_Set_Set*(*_T3)(struct Cyc_Hashtable_Table*,struct _fat_ptr*);void*(*_T4)(struct Cyc_Hashtable_Table*,void*);void*_T5;struct Cyc_Core_Not_found_exn_struct*_T6;char*_T7;char*_T8;struct Cyc_Set_Set*(*_T9)(int(*)(struct _fat_ptr*,struct _fat_ptr*));struct Cyc_Set_Set*(*_TA)(int(*)(void*,void*));int(*_TB)(struct _fat_ptr*,struct _fat_ptr*);struct Cyc_Set_Set*_TC;struct _handler_cons _TD;_T0=& _TD;_push_handler(_T0);{int _TE=0;_T1=_TD.handler;_T2=setjmp(_T1);if(!_T2)goto _TLCE;_TE=1;goto _TLCF;_TLCE: _TLCF: if(_TE)goto _TLD0;else{goto _TLD2;}_TLD2: _T4=Cyc_Hashtable_lookup;{
struct Cyc_Set_Set*(*_TF)(struct Cyc_Hashtable_Table*,struct _fat_ptr*)=(struct Cyc_Set_Set*(*)(struct Cyc_Hashtable_Table*,struct _fat_ptr*))_T4;_T3=_TF;}{struct Cyc_Set_Set*_TF=_T3(t,x);_npop_handler(0);return _TF;}_pop_handler();goto _TLD1;_TLD0: _T5=Cyc_Core_get_exn_thrown();{void*_TF=(void*)_T5;void*_T10;_T6=(struct Cyc_Core_Not_found_exn_struct*)_TF;_T7=_T6->tag;_T8=Cyc_Core_Not_found;if(_T7!=_T8)goto _TLD3;_TA=Cyc_Set_empty;{
# 1221
struct Cyc_Set_Set*(*_T11)(int(*)(struct _fat_ptr*,struct _fat_ptr*))=(struct Cyc_Set_Set*(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*)))_TA;_T9=_T11;}_TB=Cyc_strptrcmp;_TC=_T9(_TB);return _TC;_TLD3: _T10=_TF;{void*exn=_T10;_rethrow(exn);};}_TLD1:;}}
# 1225
struct Cyc_Set_Set*Cyc_reachable(struct Cyc_List_List*init,struct Cyc_Hashtable_Table*t){struct Cyc_Set_Set*(*_T0)(int(*)(struct _fat_ptr*,struct _fat_ptr*));struct Cyc_Set_Set*(*_T1)(int(*)(void*,void*));int(*_T2)(struct _fat_ptr*,struct _fat_ptr*);struct Cyc_Set_Set*(*_T3)(struct Cyc_Set_Set*,struct _fat_ptr*);struct Cyc_Set_Set*(*_T4)(struct Cyc_Set_Set*,void*);struct Cyc_Set_Set*_T5;struct Cyc_List_List*_T6;void*_T7;struct _fat_ptr*_T8;struct Cyc_List_List*_T9;struct _fat_ptr*_TA;int _TB;int(*_TC)(struct Cyc_Iter_Iter,struct _fat_ptr**);int(*_TD)(struct Cyc_Iter_Iter,void*);struct Cyc_Iter_Iter _TE;struct _fat_ptr**_TF;int _T10;struct Cyc_Set_Set*_T11;struct Cyc_Set_Set*_T12;struct Cyc_Set_Set*_T13;_T1=Cyc_Set_empty;{
# 1235 "buildlib.cyl"
struct Cyc_Set_Set*(*_T14)(int(*)(struct _fat_ptr*,struct _fat_ptr*))=(struct Cyc_Set_Set*(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*)))_T1;_T0=_T14;}_T2=Cyc_strptrcmp;{struct Cyc_Set_Set*emptyset=_T0(_T2);
struct Cyc_Set_Set*curr;
curr=emptyset;_TLD8: if(init!=0)goto _TLD6;else{goto _TLD7;}
_TLD6: _T4=Cyc_Set_insert;{struct Cyc_Set_Set*(*_T14)(struct Cyc_Set_Set*,struct _fat_ptr*)=(struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T4;_T3=_T14;}_T5=curr;_T6=init;_T7=_T6->hd;_T8=(struct _fat_ptr*)_T7;curr=_T3(_T5,_T8);_T9=init;
# 1237
init=_T9->tl;goto _TLD8;_TLD7: {
# 1240
struct Cyc_Set_Set*delta=curr;
# 1242
struct _fat_ptr*sptr;sptr=_cycalloc(sizeof(struct _fat_ptr));_TA=sptr;*_TA=_tag_fat("",sizeof(char),1U);
_TLD9: _TB=Cyc_Set_cardinality(delta);if(_TB > 0)goto _TLDA;else{goto _TLDB;}
_TLDA:{struct Cyc_Set_Set*next=emptyset;
struct Cyc_Iter_Iter iter=Cyc_Set_make_iter(Cyc_Core_heap_region,delta);
_TLDC: _TD=Cyc_Iter_next;{int(*_T14)(struct Cyc_Iter_Iter,struct _fat_ptr**)=(int(*)(struct Cyc_Iter_Iter,struct _fat_ptr**))_TD;_TC=_T14;}_TE=iter;_TF=& sptr;_T10=_TC(_TE,_TF);if(_T10)goto _TLDD;else{goto _TLDE;}
_TLDD: _T11=next;_T12=Cyc_find(t,sptr);next=Cyc_Set_union_two(_T11,_T12);goto _TLDC;_TLDE:
 delta=Cyc_Set_diff(next,curr);
curr=Cyc_Set_union_two(curr,delta);}goto _TLD9;_TLDB: _T13=curr;
# 1251
return _T13;}}}
# 1254
enum Cyc_buildlib_mode{Cyc_NORMAL =0U,Cyc_GATHER =1U,Cyc_GATHERSCRIPT =2U,Cyc_FINISH =3U};
static enum Cyc_buildlib_mode Cyc_mode=Cyc_NORMAL;
static int Cyc_gathering (void){int _T0;enum Cyc_buildlib_mode _T1;int _T2;enum Cyc_buildlib_mode _T3;int _T4;_T1=Cyc_mode;_T2=(int)_T1;
if(_T2!=1)goto _TLDF;_T0=1;goto _TLE0;_TLDF: _T3=Cyc_mode;_T4=(int)_T3;_T0=_T4==2;_TLE0: return _T0;}
# 1260
static struct Cyc___cycFILE*Cyc_script_file=0;
int Cyc_prscript(struct _fat_ptr fmt,struct _fat_ptr ap){struct Cyc___cycFILE*_T0;struct _fat_ptr _T1;struct _fat_ptr _T2;int _T3;
# 1264
if(Cyc_script_file!=0)goto _TLE1;_T0=Cyc_stderr;_T1=
_tag_fat("Internal error: script file is NULL\n",sizeof(char),37U);_T2=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T0,_T1,_T2);
exit(1);goto _TLE2;_TLE1: _TLE2: _T3=
# 1268
Cyc_vfprintf(Cyc_script_file,fmt,ap);return _T3;}
# 1271
int Cyc_force_directory(struct _fat_ptr d){enum Cyc_buildlib_mode _T0;int _T1;struct Cyc_String_pa_PrintArg_struct _T2;struct Cyc_String_pa_PrintArg_struct _T3;void**_T4;void**_T5;struct _fat_ptr _T6;struct _fat_ptr _T7;struct _fat_ptr _T8;char*_T9;const char*_TA;struct _fat_ptr _TB;struct _fat_ptr _TC;char*_TD;char*_TE;const char*_TF;int _T10;struct Cyc_String_pa_PrintArg_struct _T11;void**_T12;struct Cyc___cycFILE*_T13;struct _fat_ptr _T14;struct _fat_ptr _T15;_T0=Cyc_mode;_T1=(int)_T0;
if(_T1!=2)goto _TLE3;{struct Cyc_String_pa_PrintArg_struct _T16;_T16.tag=0;
_T16.f1=d;_T2=_T16;}{struct Cyc_String_pa_PrintArg_struct _T16=_T2;{struct Cyc_String_pa_PrintArg_struct _T17;_T17.tag=0;_T17.f1=d;_T3=_T17;}{struct Cyc_String_pa_PrintArg_struct _T17=_T3;void*_T18[2];_T4=_T18 + 0;*_T4=& _T16;_T5=_T18 + 1;*_T5=& _T17;_T6=_tag_fat("if ! test -e %s; then mkdir %s; fi\n",sizeof(char),36U);_T7=_tag_fat(_T18,sizeof(void*),2);Cyc_prscript(_T6,_T7);}}goto _TLE4;
# 1278
_TLE3: _T8=d;_T9=_untag_fat_ptr_check_bound(_T8,sizeof(char),1U);_TA=(const char*)_T9;_TB=_tag_fat(0U,sizeof(unsigned short),0);{int fd=Cyc_open(_TA,0,_TB);
if(fd!=-1)goto _TLE5;_TC=d;_TD=_untag_fat_ptr(_TC,sizeof(char),1U);_TE=_check_null(_TD);_TF=(const char*)_TE;_T10=
mkdir(_TF,448);if(_T10!=-1)goto _TLE7;{struct Cyc_String_pa_PrintArg_struct _T16;_T16.tag=0;
_T16.f1=d;_T11=_T16;}{struct Cyc_String_pa_PrintArg_struct _T16=_T11;void*_T17[1];_T12=_T17 + 0;*_T12=& _T16;_T13=Cyc_stderr;_T14=_tag_fat("Error: could not create directory %s\n",sizeof(char),38U);_T15=_tag_fat(_T17,sizeof(void*),1);Cyc_fprintf(_T13,_T14,_T15);}
return 1;_TLE7: goto _TLE6;
# 1285
_TLE5: close(fd);_TLE6:;}_TLE4:
# 1287
 return 0;}
# 1290
int Cyc_force_directory_prefixes(struct _fat_ptr file){unsigned long _T0;struct Cyc_List_List*_T1;struct _fat_ptr*_T2;struct _fat_ptr _T3;struct Cyc_List_List*_T4;void*_T5;struct _fat_ptr*_T6;struct _fat_ptr _T7;int _T8;struct Cyc_List_List*_T9;
# 1294
struct _fat_ptr curr=Cyc_strdup(file);
# 1296
struct Cyc_List_List*x=0;
_TLE9: if(1)goto _TLEA;else{goto _TLEB;}
_TLEA: curr=Cyc_Filename_dirname(curr);_T0=
Cyc_strlen(curr);if(_T0!=0U)goto _TLEC;goto _TLEB;_TLEC:{struct Cyc_List_List*_TA=_cycalloc(sizeof(struct Cyc_List_List));{struct _fat_ptr*_TB=_cycalloc(sizeof(struct _fat_ptr));_T3=curr;
*_TB=_T3;_T2=(struct _fat_ptr*)_TB;}_TA->hd=_T2;_TA->tl=x;_T1=(struct Cyc_List_List*)_TA;}x=_T1;goto _TLE9;_TLEB:
# 1303
 _TLF1: if(x!=0)goto _TLEF;else{goto _TLF0;}
_TLEF: _T4=x;_T5=_T4->hd;_T6=(struct _fat_ptr*)_T5;_T7=*_T6;_T8=Cyc_force_directory(_T7);if(!_T8)goto _TLF2;return 1;_TLF2: _T9=x;
# 1303
x=_T9->tl;goto _TLF1;_TLF0:
# 1306
 return 0;}char Cyc_NO_SUPPORT[11U]="NO_SUPPORT";struct Cyc_NO_SUPPORT_exn_struct{char*tag;struct _fat_ptr f1;};
# 1313
static int Cyc_is_other_special(char c){char _T0;int _T1;_T0=c;_T1=(int)_T0;switch(_T1){case 92: goto _LL4;case 34: _LL4: goto _LL6;case 59: _LL6: goto _LL8;case 38: _LL8: goto _LLA;case 40: _LLA: goto _LLC;case 41: _LLC: goto _LLE;case 124: _LLE: goto _LL10;case 94: _LL10: goto _LL12;case 60: _LL12: goto _LL14;case 62: _LL14: goto _LL16;case 10: _LL16: goto _LL18;case 9: _LL18:
# 1329
 return 1;default:
 return 0;};}
# 1334
static struct _fat_ptr Cyc_sh_escape_string(struct _fat_ptr s){int _T0;unsigned long _T1;unsigned long _T2;struct _fat_ptr _T3;unsigned char*_T4;const char*_T5;int _T6;char _T7;int _T8;int _T9;struct _fat_ptr _TA;struct Cyc_List_List*_TB;struct _fat_ptr**_TC;struct _fat_ptr*_TD;struct _fat_ptr**_TE;struct _fat_ptr*_TF;struct _fat_ptr _T10;struct _fat_ptr**_T11;struct _fat_ptr*_T12;struct _fat_ptr _T13;struct _fat_ptr _T14;unsigned long _T15;int _T16;unsigned long _T17;unsigned long _T18;int _T19;unsigned long _T1A;struct _fat_ptr _T1B;unsigned long _T1C;char*_T1D;unsigned _T1E;char*_T1F;unsigned _T20;char*_T21;int _T22;unsigned long _T23;unsigned long _T24;struct _fat_ptr _T25;unsigned char*_T26;const char*_T27;int _T28;char _T29;int _T2A;int _T2B;struct _fat_ptr _T2C;int _T2D;int _T2E;char*_T2F;char*_T30;unsigned _T31;unsigned char*_T32;char*_T33;struct _fat_ptr _T34;int _T35;int _T36;char*_T37;char*_T38;unsigned _T39;unsigned char*_T3A;char*_T3B;struct _fat_ptr _T3C;
unsigned long len=Cyc_strlen(s);
# 1338
int single_quotes=0;
int other_special=0;{
int i=0;_TLF8: _T0=i;_T1=(unsigned long)_T0;_T2=len;if(_T1 < _T2)goto _TLF6;else{goto _TLF7;}
_TLF6: _T3=s;_T4=_T3.curr;_T5=(const char*)_T4;_T6=i;{char c=_T5[_T6];_T7=c;_T8=(int)_T7;
if(_T8!=39)goto _TLF9;single_quotes=single_quotes + 1;goto _TLFA;
_TLF9: _T9=Cyc_is_other_special(c);if(!_T9)goto _TLFB;other_special=other_special + 1;goto _TLFC;_TLFB: _TLFC: _TLFA:;}
# 1340
i=i + 1;goto _TLF8;_TLF7:;}
# 1347
if(single_quotes!=0)goto _TLFD;if(other_special!=0)goto _TLFD;_TA=s;
return _TA;_TLFD:
# 1351
 if(single_quotes!=0)goto _TLFF;{struct _fat_ptr*_T3D[3];_TC=_T3D + 0;{struct _fat_ptr*_T3E=_cycalloc(sizeof(struct _fat_ptr));
*_T3E=_tag_fat("'",sizeof(char),2U);_TD=(struct _fat_ptr*)_T3E;}*_TC=_TD;_TE=_T3D + 1;{struct _fat_ptr*_T3E=_cycalloc(sizeof(struct _fat_ptr));_T10=s;*_T3E=_T10;_TF=(struct _fat_ptr*)_T3E;}*_TE=_TF;_T11=_T3D + 2;{struct _fat_ptr*_T3E=_cycalloc(sizeof(struct _fat_ptr));*_T3E=_tag_fat("'",sizeof(char),2U);_T12=(struct _fat_ptr*)_T3E;}*_T11=_T12;_T13=_tag_fat(_T3D,sizeof(struct _fat_ptr*),3);_TB=Cyc_List_list(_T13);}_T14=Cyc_strconcat_l(_TB);return _T14;_TLFF: _T15=len;_T16=single_quotes;_T17=(unsigned long)_T16;_T18=_T15 + _T17;_T19=other_special;_T1A=(unsigned long)_T19;{
# 1355
unsigned long len2=_T18 + _T1A;_T1C=len2 + 1U;{unsigned _T3D=_T1C + 1U;_T1E=_check_times(_T3D,sizeof(char));{char*_T3E=_cycalloc_atomic(_T1E);{unsigned _T3F=len2 + 1U;unsigned i;i=0;_TL104: if(i < _T3F)goto _TL102;else{goto _TL103;}_TL102: _T20=i;_T1F=_T3E + _T20;
*_T1F='\000';i=i + 1;goto _TL104;_TL103: _T21=_T3E + _T3F;*_T21=0;}_T1D=(char*)_T3E;}_T1B=_tag_fat(_T1D,sizeof(char),_T3D);}{struct _fat_ptr s2=_T1B;
int i=0;
int j=0;
_TL108: _T22=i;_T23=(unsigned long)_T22;_T24=len;if(_T23 < _T24)goto _TL106;else{goto _TL107;}
_TL106: _T25=s;_T26=_T25.curr;_T27=(const char*)_T26;_T28=i;{char c=_T27[_T28];_T29=c;_T2A=(int)_T29;
if(_T2A==39)goto _TL10B;else{goto _TL10C;}_TL10C: _T2B=Cyc_is_other_special(c);if(_T2B)goto _TL10B;else{goto _TL109;}
_TL10B: _T2C=s2;_T2D=j;j=_T2D + 1;_T2E=_T2D;{struct _fat_ptr _T3D=_fat_ptr_plus(_T2C,sizeof(char),_T2E);_T2F=_check_fat_subscript(_T3D,sizeof(char),0U);_T30=(char*)_T2F;{char _T3E=*_T30;char _T3F='\\';_T31=_get_fat_size(_T3D,sizeof(char));if(_T31!=1U)goto _TL10D;if(_T3E!=0)goto _TL10D;if(_T3F==0)goto _TL10D;_throw_arraybounds();goto _TL10E;_TL10D: _TL10E: _T32=_T3D.curr;_T33=(char*)_T32;*_T33=_T3F;}}goto _TL10A;_TL109: _TL10A: _T34=s2;_T35=j;
j=_T35 + 1;_T36=_T35;{struct _fat_ptr _T3D=_fat_ptr_plus(_T34,sizeof(char),_T36);_T37=_check_fat_subscript(_T3D,sizeof(char),0U);_T38=(char*)_T37;{char _T3E=*_T38;char _T3F=c;_T39=_get_fat_size(_T3D,sizeof(char));if(_T39!=1U)goto _TL10F;if(_T3E!=0)goto _TL10F;if(_T3F==0)goto _TL10F;_throw_arraybounds();goto _TL110;_TL10F: _TL110: _T3A=_T3D.curr;_T3B=(char*)_T3A;*_T3B=_T3F;}}}
# 1359
i=i + 1;goto _TL108;_TL107: _T3C=s2;
# 1365
return _T3C;}}}
# 1367
static struct _fat_ptr*Cyc_sh_escape_stringptr(struct _fat_ptr*sp){struct _fat_ptr*_T0;struct _fat_ptr*_T1;struct _fat_ptr _T2;{struct _fat_ptr*_T3=_cycalloc(sizeof(struct _fat_ptr));_T1=sp;_T2=*_T1;
*_T3=Cyc_sh_escape_string(_T2);_T0=(struct _fat_ptr*)_T3;}return _T0;}
# 1372
int Cyc_process_file(const char*filename,struct Cyc_List_List*start_symbols,struct Cyc_List_List*user_defs,struct Cyc_List_List*omit_symbols,struct Cyc_List_List*hstubs,struct Cyc_List_List*cstubs,struct Cyc_List_List*cycstubs,struct Cyc_List_List*cpp_insert){struct Cyc_Set_Set**_T0;unsigned _T1;int(*_T2)(struct Cyc_Set_Set*,struct _fat_ptr*);int(*_T3)(struct Cyc_Set_Set*,void*);struct Cyc_Set_Set**_T4;struct Cyc_Set_Set*_T5;struct _fat_ptr*_T6;struct _fat_ptr _T7;void*_T8;void*_T9;unsigned _TA;int _TB;struct Cyc_String_pa_PrintArg_struct _TC;struct _fat_ptr _TD;void*_TE;void*_TF;unsigned _T10;void**_T11;struct Cyc___cycFILE*_T12;struct _fat_ptr _T13;struct _fat_ptr _T14;int _T15;struct Cyc_String_pa_PrintArg_struct _T16;struct _fat_ptr _T17;void*_T18;void*_T19;unsigned _T1A;void**_T1B;struct _fat_ptr _T1C;struct _fat_ptr _T1D;struct _fat_ptr _T1E;void*_T1F;void*_T20;unsigned _T21;struct _fat_ptr _T22;void*_T23;void*_T24;unsigned _T25;struct _fat_ptr _T26;struct _fat_ptr _T27;struct _fat_ptr _T28;char*_T29;char*_T2A;struct _fat_ptr _T2B;struct _fat_ptr _T2C;unsigned _T2D;struct _fat_ptr _T2E;struct Cyc_String_pa_PrintArg_struct _T2F;void**_T30;struct _fat_ptr _T31;struct _fat_ptr _T32;struct _fat_ptr _T33;struct _fat_ptr _T34;struct Cyc_String_pa_PrintArg_struct _T35;void**_T36;struct _fat_ptr _T37;struct _fat_ptr _T38;char*_T39;char*_T3A;struct _fat_ptr _T3B;struct _fat_ptr _T3C;unsigned _T3D;struct _fat_ptr _T3E;struct Cyc_String_pa_PrintArg_struct _T3F;void**_T40;struct _fat_ptr _T41;struct _fat_ptr _T42;struct _fat_ptr _T43;struct _fat_ptr _T44;struct Cyc_String_pa_PrintArg_struct _T45;void**_T46;struct _fat_ptr _T47;struct _fat_ptr _T48;char*_T49;char*_T4A;struct _fat_ptr _T4B;struct _fat_ptr _T4C;unsigned _T4D;struct _fat_ptr _T4E;struct Cyc_String_pa_PrintArg_struct _T4F;void**_T50;struct _fat_ptr _T51;struct _fat_ptr _T52;struct _fat_ptr _T53;struct _fat_ptr _T54;struct Cyc_String_pa_PrintArg_struct _T55;void**_T56;struct _fat_ptr _T57;struct _fat_ptr _T58;char*_T59;char*_T5A;struct _handler_cons*_T5B;int*_T5C;int _T5D;struct _fat_ptr _T5E;void*_T5F;void*_T60;unsigned _T61;int _T62;enum Cyc_buildlib_mode _T63;int _T64;enum Cyc_buildlib_mode _T65;int _T66;struct Cyc_String_pa_PrintArg_struct _T67;struct _fat_ptr _T68;void*_T69;void*_T6A;unsigned _T6B;void**_T6C;struct _fat_ptr _T6D;struct _fat_ptr _T6E;struct Cyc_String_pa_PrintArg_struct _T6F;struct Cyc_List_List*_T70;void*_T71;struct _fat_ptr*_T72;void**_T73;struct _fat_ptr _T74;struct _fat_ptr _T75;struct Cyc_List_List*_T76;struct Cyc_String_pa_PrintArg_struct _T77;struct _fat_ptr _T78;void*_T79;void*_T7A;unsigned _T7B;void**_T7C;struct _fat_ptr _T7D;struct _fat_ptr _T7E;struct _fat_ptr _T7F;struct _fat_ptr _T80;struct Cyc_String_pa_PrintArg_struct _T81;struct Cyc_String_pa_PrintArg_struct _T82;struct _fat_ptr _T83;void*_T84;void*_T85;unsigned _T86;struct Cyc_String_pa_PrintArg_struct _T87;struct _fat_ptr _T88;void*_T89;void*_T8A;unsigned _T8B;void**_T8C;void**_T8D;void**_T8E;struct _fat_ptr _T8F;struct _fat_ptr _T90;struct Cyc_String_pa_PrintArg_struct _T91;struct Cyc_String_pa_PrintArg_struct _T92;struct _fat_ptr _T93;void*_T94;void*_T95;unsigned _T96;struct Cyc_String_pa_PrintArg_struct _T97;struct _fat_ptr _T98;void*_T99;void*_T9A;unsigned _T9B;void**_T9C;void**_T9D;void**_T9E;struct _fat_ptr _T9F;struct _fat_ptr _TA0;struct Cyc_String_pa_PrintArg_struct _TA1;struct _fat_ptr _TA2;void*_TA3;void*_TA4;unsigned _TA5;void**_TA6;struct _fat_ptr _TA7;struct _fat_ptr _TA8;struct Cyc___cycFILE*_TA9;unsigned _TAA;struct Cyc_String_pa_PrintArg_struct _TAB;struct _fat_ptr _TAC;void*_TAD;void*_TAE;unsigned _TAF;void**_TB0;struct Cyc___cycFILE*_TB1;struct _fat_ptr _TB2;struct _fat_ptr _TB3;int _TB4;struct Cyc_String_pa_PrintArg_struct _TB5;struct _fat_ptr _TB6;void*_TB7;void*_TB8;unsigned _TB9;void**_TBA;struct Cyc___cycFILE*_TBB;struct _fat_ptr _TBC;struct _fat_ptr _TBD;struct Cyc_List_List*_TBE;void*_TBF;struct _fat_ptr*_TC0;struct _fat_ptr _TC1;char*_TC2;char*_TC3;const char*_TC4;struct Cyc___cycFILE*_TC5;struct Cyc_List_List*_TC6;struct Cyc_String_pa_PrintArg_struct _TC7;struct _fat_ptr _TC8;void*_TC9;void*_TCA;unsigned _TCB;void**_TCC;struct Cyc___cycFILE*_TCD;struct _fat_ptr _TCE;struct _fat_ptr _TCF;struct Cyc_List_List*_TD0;struct _fat_ptr*_TD1;struct _fat_ptr _TD2;struct Cyc_List_List*(*_TD3)(struct _fat_ptr*(*)(struct _fat_ptr*),struct Cyc_List_List*);struct Cyc_List_List*(*_TD4)(void*(*)(void*),struct Cyc_List_List*);struct Cyc_List_List*_TD5;struct _fat_ptr _TD6;struct _fat_ptr _TD7;struct Cyc_String_pa_PrintArg_struct _TD8;struct Cyc_String_pa_PrintArg_struct _TD9;struct Cyc_String_pa_PrintArg_struct _TDA;struct Cyc_String_pa_PrintArg_struct _TDB;struct _fat_ptr _TDC;void*_TDD;void*_TDE;unsigned _TDF;struct Cyc_String_pa_PrintArg_struct _TE0;struct _fat_ptr _TE1;void*_TE2;void*_TE3;unsigned _TE4;struct Cyc_String_pa_PrintArg_struct _TE5;int _TE6;void**_TE7;void**_TE8;void**_TE9;void**_TEA;void**_TEB;void**_TEC;struct _fat_ptr _TED;struct _fat_ptr _TEE;char*_TEF;char*_TF0;int _TF1;struct Cyc_String_pa_PrintArg_struct _TF2;struct _fat_ptr _TF3;void*_TF4;void*_TF5;unsigned _TF6;void**_TF7;struct Cyc___cycFILE*_TF8;struct _fat_ptr _TF9;struct _fat_ptr _TFA;int _TFB;struct _fat_ptr _TFC;struct Cyc_String_pa_PrintArg_struct _TFD;struct Cyc_String_pa_PrintArg_struct _TFE;struct Cyc_String_pa_PrintArg_struct _TFF;struct Cyc_String_pa_PrintArg_struct _T100;struct _fat_ptr _T101;void*_T102;void*_T103;unsigned _T104;struct Cyc_String_pa_PrintArg_struct _T105;struct _fat_ptr _T106;void*_T107;void*_T108;unsigned _T109;struct Cyc_String_pa_PrintArg_struct _T10A;int _T10B;void**_T10C;void**_T10D;void**_T10E;void**_T10F;void**_T110;void**_T111;struct _fat_ptr _T112;struct _fat_ptr _T113;char*_T114;char*_T115;int _T116;struct Cyc_String_pa_PrintArg_struct _T117;struct _fat_ptr _T118;void*_T119;void*_T11A;unsigned _T11B;void**_T11C;struct Cyc___cycFILE*_T11D;struct _fat_ptr _T11E;struct _fat_ptr _T11F;int _T120;struct Cyc___cycFILE*_T121;unsigned _T122;struct Cyc_NO_SUPPORT_exn_struct*_T123;struct _fat_ptr _T124;struct Cyc_String_pa_PrintArg_struct _T125;struct _fat_ptr _T126;void*_T127;void*_T128;unsigned _T129;void**_T12A;struct _fat_ptr _T12B;struct _fat_ptr _T12C;void*_T12D;int _T12E;struct Cyc___cycFILE*_T12F;struct _fat_ptr _T130;struct _fat_ptr _T131;struct _tuple13*_T132;void(*_T133)(struct Cyc_Hashtable_Table*,struct _fat_ptr*,struct Cyc_Set_Set*);void(*_T134)(struct Cyc_Hashtable_Table*,void*,void*);int _T135;struct Cyc___cycFILE*_T136;struct _fat_ptr _T137;struct _fat_ptr _T138;struct Cyc___cycFILE*_T139;unsigned _T13A;struct Cyc_NO_SUPPORT_exn_struct*_T13B;struct _fat_ptr _T13C;struct Cyc_String_pa_PrintArg_struct _T13D;struct _fat_ptr _T13E;void*_T13F;void*_T140;unsigned _T141;void**_T142;struct _fat_ptr _T143;struct _fat_ptr _T144;void*_T145;int _T146;struct Cyc___cycFILE*_T147;struct _fat_ptr _T148;struct _fat_ptr _T149;struct Cyc___cycFILE*_T14A;unsigned _T14B;int _T14C;int _T14D;struct Cyc___cycFILE*_T14E;struct _fat_ptr _T14F;struct _fat_ptr _T150;struct Cyc_List_List*_T151;void*_T152;struct Cyc_String_pa_PrintArg_struct _T153;struct _fat_ptr*_T154;void**_T155;struct Cyc___cycFILE*_T156;struct _fat_ptr _T157;struct _fat_ptr _T158;struct Cyc_List_List*_T159;struct Cyc___cycFILE*_T15A;enum Cyc_buildlib_mode _T15B;int _T15C;struct Cyc___cycFILE*_T15D;unsigned _T15E;int _T15F;struct Cyc___cycFILE*_T160;struct _fat_ptr _T161;struct _fat_ptr _T162;struct _fat_ptr _T163;void*_T164;void*_T165;unsigned _T166;struct _handler_cons*_T167;int*_T168;int _T169;int _T16A;struct Cyc___cycFILE*_T16B;struct _fat_ptr _T16C;struct _fat_ptr _T16D;void*_T16E;int _T16F;struct Cyc___cycFILE*_T170;struct _fat_ptr _T171;struct _fat_ptr _T172;struct Cyc_List_List*_T173;void*_T174;struct Cyc_Absyn_Decl*_T175;struct Cyc_Hashtable_Table*_T176;struct Cyc_List_List*_T177;struct Cyc_List_List*(*_T178)(struct _fat_ptr*(*)(struct _fat_ptr*),struct Cyc_List_List*);struct Cyc_List_List*(*_T179)(void*(*)(void*),struct Cyc_List_List*);struct _tuple0 _T17A;struct Cyc_List_List*_T17B;struct Cyc_List_List*_T17C;struct Cyc_Hashtable_Table*_T17D;struct Cyc_Set_Set*(*_T17E)(int(*)(struct _fat_ptr*,struct _fat_ptr*));struct Cyc_Set_Set*(*_T17F)(int(*)(void*,void*));int(*_T180)(struct _fat_ptr*,struct _fat_ptr*);struct Cyc_List_List*_T181;void*_T182;struct Cyc_Absyn_Decl*_T183;int*_T184;unsigned _T185;struct Cyc_Absyn_Vardecl*_T186;struct Cyc_Set_Set*(*_T187)(struct Cyc_Set_Set*,struct _fat_ptr*);struct Cyc_Set_Set*(*_T188)(struct Cyc_Set_Set*,void*);int(*_T189)(int(*)(struct _fat_ptr*,struct _fat_ptr*),struct Cyc_List_List*,struct _fat_ptr*);int(*_T18A)(int(*)(void*,void*),struct Cyc_List_List*,void*);int(*_T18B)(struct _fat_ptr*,struct _fat_ptr*);struct Cyc_List_List*_T18C;struct _fat_ptr*_T18D;int _T18E;struct Cyc_Absyn_Fndecl*_T18F;struct Cyc_Set_Set*(*_T190)(struct Cyc_Set_Set*,struct _fat_ptr*);struct Cyc_Set_Set*(*_T191)(struct Cyc_Set_Set*,void*);int(*_T192)(int(*)(struct _fat_ptr*,struct _fat_ptr*),struct Cyc_List_List*,struct _fat_ptr*);int(*_T193)(int(*)(void*,void*),struct Cyc_List_List*,void*);int(*_T194)(struct _fat_ptr*,struct _fat_ptr*);struct Cyc_List_List*_T195;struct _fat_ptr*_T196;int _T197;struct Cyc_Absyn_Aggrdecl*_T198;struct Cyc_Absyn_Enumdecl*_T199;int(*_T19A)(struct Cyc_Set_Set*,struct _fat_ptr*);int(*_T19B)(struct Cyc_Set_Set*,void*);int _T19C;struct Cyc_List_List*_T19D;struct Cyc_Absyn_Enumdecl*_T19E;struct Cyc_Core_Opt*_T19F;unsigned _T1A0;struct Cyc_Absyn_Enumdecl*_T1A1;struct Cyc_Core_Opt*_T1A2;void*_T1A3;struct Cyc_List_List*_T1A4;void*_T1A5;struct Cyc_Absyn_Enumfield*_T1A6;int(*_T1A7)(struct Cyc_Set_Set*,struct _fat_ptr*);int(*_T1A8)(struct Cyc_Set_Set*,void*);int _T1A9;struct Cyc_List_List*_T1AA;struct Cyc_List_List*_T1AB;struct Cyc_Absyn_Typedefdecl*_T1AC;int(*_T1AD)(struct Cyc_Set_Set*,struct _fat_ptr*);int(*_T1AE)(struct Cyc_Set_Set*,void*);int _T1AF;struct _fat_ptr*_T1B0;struct _fat_ptr _T1B1;struct _fat_ptr _T1B2;unsigned long _T1B3;int _T1B4;struct Cyc_List_List*_T1B5;struct Cyc_List_List*_T1B6;struct Cyc_List_List*_T1B7;int _T1B8;struct Cyc___cycFILE*_T1B9;unsigned _T1BA;struct _fat_ptr _T1BB;struct Cyc_String_pa_PrintArg_struct _T1BC;struct _fat_ptr _T1BD;void*_T1BE;void*_T1BF;unsigned _T1C0;void**_T1C1;struct _fat_ptr _T1C2;struct _fat_ptr _T1C3;int _T1C4;unsigned _T1C5;struct _fat_ptr _T1C6;unsigned _T1C7;struct _fat_ptr _T1C8;unsigned char*_T1C9;char*_T1CA;int _T1CB;char _T1CC;int _T1CD;struct _fat_ptr _T1CE;unsigned char*_T1CF;char*_T1D0;int _T1D1;char _T1D2;int _T1D3;struct _fat_ptr _T1D4;int _T1D5;unsigned char*_T1D6;char*_T1D7;unsigned _T1D8;unsigned char*_T1D9;char*_T1DA;struct _fat_ptr _T1DB;unsigned char*_T1DC;char*_T1DD;int _T1DE;char _T1DF;int _T1E0;struct _fat_ptr _T1E1;unsigned char*_T1E2;char*_T1E3;int _T1E4;char _T1E5;int _T1E6;struct _fat_ptr _T1E7;int _T1E8;unsigned char*_T1E9;char*_T1EA;struct _fat_ptr _T1EB;unsigned char*_T1EC;char*_T1ED;int _T1EE;char _T1EF;int _T1F0;int _T1F1;unsigned _T1F2;unsigned char*_T1F3;char*_T1F4;int _T1F5;struct Cyc___cycFILE*_T1F6;struct _fat_ptr _T1F7;struct _fat_ptr _T1F8;struct Cyc_String_pa_PrintArg_struct _T1F9;struct Cyc_String_pa_PrintArg_struct _T1FA;void**_T1FB;void**_T1FC;struct Cyc___cycFILE*_T1FD;struct _fat_ptr _T1FE;struct _fat_ptr _T1FF;struct Cyc_List_List*_T200;void*_T201;struct Cyc_Absyn_Decl*_T202;int*_T203;unsigned _T204;struct Cyc_Absyn_Vardecl*_T205;struct Cyc_Absyn_Fndecl*_T206;int _T207;struct Cyc_Absyn_Fndecl*_T208;struct Cyc_Absyn_Aggrdecl*_T209;struct Cyc_Absyn_Enumdecl*_T20A;struct Cyc_Absyn_Typedefdecl*_T20B;struct _fat_ptr*_T20C;unsigned _T20D;int _T20E;int(*_T20F)(struct Cyc_Set_Set*,struct _fat_ptr*);int(*_T210)(struct Cyc_Set_Set*,void*);struct Cyc_Set_Set*_T211;struct _fat_ptr*_T212;int _T213;struct Cyc_Core_Impossible_exn_struct*_T214;void*_T215;struct Cyc_Absyn_Decl*_T216;int*_T217;unsigned _T218;struct Cyc_NO_SUPPORT_exn_struct*_T219;void*_T21A;struct Cyc_List_List*_T21B;struct Cyc_List_List*_T21C;struct Cyc_List_List*_T21D;struct Cyc_List_List*_T21E;struct _handler_cons*_T21F;int*_T220;int _T221;struct Cyc_Tcenv_Tenv*_T222;struct Cyc_List_List*_T223;void*_T224;struct Cyc_NO_SUPPORT_exn_struct*_T225;void*_T226;struct _tuple0 _T227;struct Cyc_List_List*_T228;void*_T229;struct Cyc_List_List*_T22A;void*_T22B;struct _fat_ptr*_T22C;unsigned _T22D;struct Cyc_Absynpp_Params*_T22E;struct Cyc_Absynpp_Params*_T22F;struct _fat_ptr*_T230;unsigned _T231;struct _fat_ptr _T232;struct Cyc_String_pa_PrintArg_struct _T233;struct _fat_ptr*_T234;void**_T235;struct _fat_ptr _T236;struct _fat_ptr _T237;struct Cyc_String_pa_PrintArg_struct _T238;void**_T239;struct Cyc___cycFILE*_T23A;struct _fat_ptr _T23B;struct _fat_ptr _T23C;struct Cyc_String_pa_PrintArg_struct _T23D;void**_T23E;struct Cyc___cycFILE*_T23F;struct _fat_ptr _T240;struct _fat_ptr _T241;struct Cyc_List_List*_T242;struct Cyc_Absyn_Decl**_T243;struct _fat_ptr _T244;struct Cyc___cycFILE*_T245;struct Cyc___cycFILE*_T246;struct _fat_ptr _T247;struct _fat_ptr _T248;struct Cyc_List_List*_T249;struct Cyc_Absyn_Decl**_T24A;struct _fat_ptr _T24B;struct Cyc___cycFILE*_T24C;struct Cyc_List_List*_T24D;struct Cyc_List_List*_T24E;struct Cyc___cycFILE*_T24F;unsigned _T250;struct Cyc_NO_SUPPORT_exn_struct*_T251;struct _fat_ptr _T252;struct Cyc_String_pa_PrintArg_struct _T253;struct _fat_ptr _T254;void*_T255;void*_T256;unsigned _T257;void**_T258;struct _fat_ptr _T259;struct _fat_ptr _T25A;void*_T25B;struct _tuple14*_T25C;int(*_T25D)(struct Cyc_Set_Set*,struct _fat_ptr*);int(*_T25E)(struct Cyc_Set_Set*,void*);int _T25F;struct Cyc_String_pa_PrintArg_struct _T260;struct _fat_ptr*_T261;void**_T262;struct Cyc___cycFILE*_T263;struct _fat_ptr _T264;struct _fat_ptr _T265;struct Cyc_String_pa_PrintArg_struct _T266;void**_T267;struct Cyc___cycFILE*_T268;struct _fat_ptr _T269;struct _fat_ptr _T26A;struct Cyc___cycFILE*_T26B;struct _fat_ptr _T26C;struct _fat_ptr _T26D;enum Cyc_buildlib_mode _T26E;int _T26F;struct Cyc_List_List*_T270;void*_T271;struct _fat_ptr _T272;unsigned char*_T273;char*_T274;struct _fat_ptr _T275;unsigned char*_T276;char*_T277;struct _fat_ptr _T278;char*_T279;const char*_T27A;struct Cyc___cycFILE*_T27B;int(*_T27C)(struct Cyc_Set_Set*,struct _fat_ptr*);int(*_T27D)(struct Cyc_Set_Set*,void*);struct Cyc_Set_Set*_T27E;struct _fat_ptr*_T27F;int _T280;struct _fat_ptr _T281;char*_T282;const char*_T283;struct Cyc___cycFILE*_T284;struct Cyc_String_pa_PrintArg_struct _T285;void**_T286;struct _fat_ptr _T287;struct _fat_ptr _T288;int _T289;struct _fat_ptr*(*_T28A)(struct Cyc_Set_Set*);void*(*_T28B)(struct Cyc_Set_Set*);struct Cyc_Set_Set*(*_T28C)(struct Cyc_Set_Set*,struct _fat_ptr*);struct Cyc_Set_Set*(*_T28D)(struct Cyc_Set_Set*,void*);struct Cyc_String_pa_PrintArg_struct _T28E;struct _fat_ptr*_T28F;void**_T290;struct _fat_ptr _T291;struct _fat_ptr _T292;struct _fat_ptr _T293;struct _fat_ptr _T294;struct Cyc_String_pa_PrintArg_struct _T295;void**_T296;struct _fat_ptr _T297;struct _fat_ptr _T298;struct Cyc_List_List*_T299;struct Cyc___cycFILE*_T29A;struct _fat_ptr _T29B;struct _fat_ptr _T29C;int _T29D;struct Cyc___cycFILE*_T29E;struct _fat_ptr _T29F;struct _fat_ptr _T2A0;int _T2A1;struct Cyc_List_List*_T2A2;void*_T2A3;struct _fat_ptr _T2A4;unsigned char*_T2A5;char*_T2A6;struct _fat_ptr _T2A7;unsigned char*_T2A8;char*_T2A9;int(*_T2AA)(struct Cyc_Set_Set*,struct _fat_ptr*);int(*_T2AB)(struct Cyc_Set_Set*,void*);struct Cyc_Set_Set*_T2AC;struct _fat_ptr*_T2AD;int _T2AE;struct _fat_ptr _T2AF;char*_T2B0;const char*_T2B1;struct Cyc___cycFILE*_T2B2;struct Cyc_List_List*_T2B3;struct Cyc_String_pa_PrintArg_struct _T2B4;struct _fat_ptr _T2B5;void*_T2B6;void*_T2B7;unsigned _T2B8;void**_T2B9;struct Cyc___cycFILE*_T2BA;struct _fat_ptr _T2BB;struct _fat_ptr _T2BC;struct Cyc_List_List*_T2BD;void*_T2BE;struct _fat_ptr _T2BF;unsigned char*_T2C0;char*_T2C1;struct _fat_ptr _T2C2;unsigned char*_T2C3;char*_T2C4;int(*_T2C5)(struct Cyc_Set_Set*,struct _fat_ptr*);int(*_T2C6)(struct Cyc_Set_Set*,void*);struct Cyc_Set_Set*_T2C7;struct _fat_ptr*_T2C8;int _T2C9;struct _fat_ptr _T2CA;char*_T2CB;const char*_T2CC;struct Cyc___cycFILE*_T2CD;struct Cyc_List_List*_T2CE;struct Cyc___cycFILE*_T2CF;struct _fat_ptr _T2D0;struct _fat_ptr _T2D1;void*_T2D2;struct Cyc_Core_Impossible_exn_struct*_T2D3;char*_T2D4;char*_T2D5;struct Cyc_String_pa_PrintArg_struct _T2D6;void**_T2D7;struct _fat_ptr _T2D8;struct _fat_ptr _T2D9;struct Cyc_Dict_Absent_exn_struct*_T2DA;char*_T2DB;char*_T2DC;struct _fat_ptr _T2DD;struct _fat_ptr _T2DE;struct Cyc_Core_Failure_exn_struct*_T2DF;char*_T2E0;char*_T2E1;struct Cyc_String_pa_PrintArg_struct _T2E2;void**_T2E3;struct _fat_ptr _T2E4;struct _fat_ptr _T2E5;struct Cyc_Core_Invalid_argument_exn_struct*_T2E6;char*_T2E7;char*_T2E8;struct Cyc_String_pa_PrintArg_struct _T2E9;void**_T2EA;struct _fat_ptr _T2EB;struct _fat_ptr _T2EC;struct Cyc_Core_Not_found_exn_struct*_T2ED;char*_T2EE;char*_T2EF;struct _fat_ptr _T2F0;struct _fat_ptr _T2F1;struct Cyc_NO_SUPPORT_exn_struct*_T2F2;char*_T2F3;char*_T2F4;struct Cyc_String_pa_PrintArg_struct _T2F5;void**_T2F6;struct _fat_ptr _T2F7;struct _fat_ptr _T2F8;struct Cyc_Lexing_Error_exn_struct*_T2F9;char*_T2FA;char*_T2FB;struct Cyc_String_pa_PrintArg_struct _T2FC;void**_T2FD;struct _fat_ptr _T2FE;struct _fat_ptr _T2FF;struct _fat_ptr _T300;struct _fat_ptr _T301;struct Cyc___cycFILE*_T302;unsigned _T303;struct Cyc_String_pa_PrintArg_struct _T304;struct _fat_ptr _T305;void*_T306;void*_T307;unsigned _T308;void**_T309;struct Cyc___cycFILE*_T30A;struct _fat_ptr _T30B;struct _fat_ptr _T30C;struct Cyc_String_pa_PrintArg_struct _T30D;struct _fat_ptr _T30E;void*_T30F;void*_T310;unsigned _T311;void**_T312;struct Cyc___cycFILE*_T313;struct _fat_ptr _T314;struct _fat_ptr _T315;struct _fat_ptr _T316;struct _fat_ptr _T317;
# 1380
struct Cyc___cycFILE*maybe;
struct Cyc___cycFILE*in_file;
struct Cyc___cycFILE*out_file;
int errorcode=0;_T0=Cyc_headers_to_do;_T1=(unsigned)_T0;
# 1385
if(!_T1)goto _TL111;_T3=Cyc_Set_member;{int(*_T318)(struct Cyc_Set_Set*,struct _fat_ptr*)=(int(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T3;_T2=_T318;}_T4=Cyc_headers_to_do;_T5=*_T4;{struct _fat_ptr*_T318=_cycalloc(sizeof(struct _fat_ptr));{const char*_T319=filename;_T8=(void*)_T319;_T9=(void*)_T319;_TA=_get_zero_arr_size_char(_T9,1U);_T7=_tag_fat(_T8,sizeof(char),_TA);}*_T318=_T7;_T6=(struct _fat_ptr*)_T318;}_TB=_T2(_T5,_T6);if(_TB)goto _TL111;else{goto _TL113;}
_TL113: return 0;_TL111:{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;{const char*_T319=filename;_TE=(void*)_T319;_TF=(void*)_T319;_T10=_get_zero_arr_size_char(_TF,1U);_TD=_tag_fat(_TE,sizeof(char),_T10);}
# 1389
_T318.f1=_TD;_TC=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_TC;void*_T319[1];_T11=_T319 + 0;*_T11=& _T318;_T12=Cyc_stderr;_T13=_tag_fat("********************************* %s...\n",sizeof(char),41U);_T14=_tag_fat(_T319,sizeof(void*),1);Cyc_fprintf(_T12,_T13,_T14);}_T15=
# 1391
Cyc_gathering();if(_T15)goto _TL114;else{goto _TL116;}_TL116:{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;{const char*_T319=filename;_T18=(void*)_T319;_T19=(void*)_T319;_T1A=_get_zero_arr_size_char(_T19,1U);_T17=_tag_fat(_T18,sizeof(char),_T1A);}_T318.f1=_T17;_T16=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_T16;void*_T319[1];_T1B=_T319 + 0;*_T1B=& _T318;_T1C=_tag_fat("\n%s:\n",sizeof(char),6U);_T1D=_tag_fat(_T319,sizeof(void*),1);Cyc_log(_T1C,_T1D);}goto _TL115;_TL114: _TL115:{const char*_T318=filename;_T1F=(void*)_T318;_T20=(void*)_T318;_T21=_get_zero_arr_size_char(_T20,1U);_T1E=_tag_fat(_T1F,sizeof(char),_T21);}{
# 1403 "buildlib.cyl"
struct _fat_ptr basename=Cyc_Filename_basename(_T1E);{const char*_T318=filename;_T23=(void*)_T318;_T24=(void*)_T318;_T25=_get_zero_arr_size_char(_T24,1U);_T22=_tag_fat(_T23,sizeof(char),_T25);}{
struct _fat_ptr dirname=Cyc_Filename_dirname(_T22);
struct _fat_ptr choppedname=Cyc_Filename_chop_extension(basename);_T26=choppedname;_T27=
_tag_fat(".iA",sizeof(char),4U);_T28=Cyc_strconcat(_T26,_T27);_T29=_untag_fat_ptr_check_bound(_T28,sizeof(char),1U);_T2A=_check_null(_T29);{const char*cppinfile=(const char*)_T2A;_T2C=dirname;_T2D=
# 1409
_get_fat_size(_T2C,sizeof(char));if(_T2D!=0U)goto _TL117;{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;
_T318.f1=choppedname;_T2F=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_T2F;void*_T319[1];_T30=_T319 + 0;*_T30=& _T318;_T31=_tag_fat("%s.iB",sizeof(char),6U);_T32=_tag_fat(_T319,sizeof(void*),1);_T2E=Cyc_aprintf(_T31,_T32);}_T2B=_T2E;goto _TL118;_TL117: _T33=dirname;{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;
_T318.f1=choppedname;_T35=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_T35;void*_T319[1];_T36=_T319 + 0;*_T36=& _T318;_T37=_tag_fat("%s.iB",sizeof(char),6U);_T38=_tag_fat(_T319,sizeof(void*),1);_T34=Cyc_aprintf(_T37,_T38);}_T2B=Cyc_Filename_concat(_T33,_T34);_TL118: _T39=_untag_fat_ptr_check_bound(_T2B,sizeof(char),1U);_T3A=_check_null(_T39);{
# 1408
const char*macrosfile=(const char*)_T3A;_T3C=dirname;_T3D=
# 1414
_get_fat_size(_T3C,sizeof(char));if(_T3D!=0U)goto _TL119;{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;
_T318.f1=choppedname;_T3F=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_T3F;void*_T319[1];_T40=_T319 + 0;*_T40=& _T318;_T41=_tag_fat("%s.iC",sizeof(char),6U);_T42=_tag_fat(_T319,sizeof(void*),1);_T3E=Cyc_aprintf(_T41,_T42);}_T3B=_T3E;goto _TL11A;_TL119: _T43=dirname;{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;
_T318.f1=choppedname;_T45=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_T45;void*_T319[1];_T46=_T319 + 0;*_T46=& _T318;_T47=_tag_fat("%s.iC",sizeof(char),6U);_T48=_tag_fat(_T319,sizeof(void*),1);_T44=Cyc_aprintf(_T47,_T48);}_T3B=Cyc_Filename_concat(_T43,_T44);_TL11A: _T49=_untag_fat_ptr_check_bound(_T3B,sizeof(char),1U);_T4A=_check_null(_T49);{
# 1413
const char*declsfile=(const char*)_T4A;_T4C=dirname;_T4D=
# 1419
_get_fat_size(_T4C,sizeof(char));if(_T4D!=0U)goto _TL11B;{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;
_T318.f1=choppedname;_T4F=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_T4F;void*_T319[1];_T50=_T319 + 0;*_T50=& _T318;_T51=_tag_fat("%s.iD",sizeof(char),6U);_T52=_tag_fat(_T319,sizeof(void*),1);_T4E=Cyc_aprintf(_T51,_T52);}_T4B=_T4E;goto _TL11C;_TL11B: _T53=dirname;{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;
_T318.f1=choppedname;_T55=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_T55;void*_T319[1];_T56=_T319 + 0;*_T56=& _T318;_T57=_tag_fat("%s.iD",sizeof(char),6U);_T58=_tag_fat(_T319,sizeof(void*),1);_T54=Cyc_aprintf(_T57,_T58);}_T4B=Cyc_Filename_concat(_T53,_T54);_TL11C: _T59=_untag_fat_ptr_check_bound(_T4B,sizeof(char),1U);_T5A=_check_null(_T59);{
# 1418
const char*filtereddeclsfile=(const char*)_T5A;{struct _handler_cons _T318;_T5B=& _T318;_push_handler(_T5B);{int _T319=0;_T5C=_T318.handler;_T5D=setjmp(_T5C);if(!_T5D)goto _TL11D;_T319=1;goto _TL11E;_TL11D: _TL11E: if(_T319)goto _TL11F;else{goto _TL121;}_TL121:{const char*_T31A=filename;_T5F=(void*)_T31A;_T60=(void*)_T31A;_T61=_get_zero_arr_size_char(_T60,1U);_T5E=_tag_fat(_T5F,sizeof(char),_T61);}_T62=
# 1426
Cyc_force_directory_prefixes(_T5E);if(!_T62)goto _TL122;{int _T31A=1;_npop_handler(0);return _T31A;}_TL122: _T63=Cyc_mode;_T64=(int)_T63;
# 1431
if(_T64==3)goto _TL124;_T65=Cyc_mode;_T66=(int)_T65;
if(_T66!=2)goto _TL126;{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{const char*_T31B=cppinfile;_T69=(void*)_T31B;_T6A=(void*)_T31B;_T6B=_get_zero_arr_size_char(_T6A,1U);_T68=_tag_fat(_T69,sizeof(char),_T6B);}
_T31A.f1=_T68;_T67=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_T67;void*_T31B[1];_T6C=_T31B + 0;*_T6C=& _T31A;_T6D=_tag_fat("cat >%s <<XXX\n",sizeof(char),15U);_T6E=_tag_fat(_T31B,sizeof(void*),1);Cyc_prscript(_T6D,_T6E);}{
struct Cyc_List_List*l=cpp_insert;_TL12B: if(l!=0)goto _TL129;else{goto _TL12A;}
_TL129:{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;_T70=l;_T71=_T70->hd;_T72=(struct _fat_ptr*)_T71;_T31A.f1=*_T72;_T6F=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_T6F;void*_T31B[1];_T73=_T31B + 0;*_T73=& _T31A;_T74=_tag_fat("%s",sizeof(char),3U);_T75=_tag_fat(_T31B,sizeof(void*),1);Cyc_prscript(_T74,_T75);}_T76=l;
# 1434
l=_T76->tl;goto _TL12B;_TL12A:;}{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{const char*_T31B=filename;_T79=(void*)_T31B;_T7A=(void*)_T31B;_T7B=_get_zero_arr_size_char(_T7A,1U);_T78=_tag_fat(_T79,sizeof(char),_T7B);}
# 1436
_T31A.f1=_T78;_T77=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_T77;void*_T31B[1];_T7C=_T31B + 0;*_T7C=& _T31A;_T7D=_tag_fat("#include <%s>\n",sizeof(char),15U);_T7E=_tag_fat(_T31B,sizeof(void*),1);Cyc_prscript(_T7D,_T7E);}_T7F=
_tag_fat("XXX\n",sizeof(char),5U);_T80=_tag_fat(0U,sizeof(void*),0);Cyc_prscript(_T7F,_T80);{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;
_T31A.f1=Cyc_target_cflags;_T81=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_T81;{struct Cyc_String_pa_PrintArg_struct _T31B;_T31B.tag=0;{const char*_T31C=macrosfile;_T84=(void*)_T31C;_T85=(void*)_T31C;_T86=_get_zero_arr_size_char(_T85,1U);_T83=_tag_fat(_T84,sizeof(char),_T86);}_T31B.f1=_T83;_T82=_T31B;}{struct Cyc_String_pa_PrintArg_struct _T31B=_T82;{struct Cyc_String_pa_PrintArg_struct _T31C;_T31C.tag=0;{const char*_T31D=cppinfile;_T89=(void*)_T31D;_T8A=(void*)_T31D;_T8B=_get_zero_arr_size_char(_T8A,1U);_T88=_tag_fat(_T89,sizeof(char),_T8B);}_T31C.f1=_T88;_T87=_T31C;}{struct Cyc_String_pa_PrintArg_struct _T31C=_T87;void*_T31D[3];_T8C=_T31D + 0;*_T8C=& _T31A;_T8D=_T31D + 1;*_T8D=& _T31B;_T8E=_T31D + 2;*_T8E=& _T31C;_T8F=_tag_fat("$GCC %s -E -dM -o %s -x c %s && \\\n",sizeof(char),35U);_T90=_tag_fat(_T31D,sizeof(void*),3);Cyc_prscript(_T8F,_T90);}}}{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;
_T31A.f1=Cyc_target_cflags;_T91=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_T91;{struct Cyc_String_pa_PrintArg_struct _T31B;_T31B.tag=0;{const char*_T31C=declsfile;_T94=(void*)_T31C;_T95=(void*)_T31C;_T96=_get_zero_arr_size_char(_T95,1U);_T93=_tag_fat(_T94,sizeof(char),_T96);}_T31B.f1=_T93;_T92=_T31B;}{struct Cyc_String_pa_PrintArg_struct _T31B=_T92;{struct Cyc_String_pa_PrintArg_struct _T31C;_T31C.tag=0;{const char*_T31D=cppinfile;_T99=(void*)_T31D;_T9A=(void*)_T31D;_T9B=_get_zero_arr_size_char(_T9A,1U);_T98=_tag_fat(_T99,sizeof(char),_T9B);}_T31C.f1=_T98;_T97=_T31C;}{struct Cyc_String_pa_PrintArg_struct _T31C=_T97;void*_T31D[3];_T9C=_T31D + 0;*_T9C=& _T31A;_T9D=_T31D + 1;*_T9D=& _T31B;_T9E=_T31D + 2;*_T9E=& _T31C;_T9F=_tag_fat("$GCC %s -E     -o %s -x c %s;\n",sizeof(char),31U);_TA0=_tag_fat(_T31D,sizeof(void*),3);Cyc_prscript(_T9F,_TA0);}}}{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{const char*_T31B=cppinfile;_TA3=(void*)_T31B;_TA4=(void*)_T31B;_TA5=_get_zero_arr_size_char(_TA4,1U);_TA2=_tag_fat(_TA3,sizeof(char),_TA5);}
_T31A.f1=_TA2;_TA1=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_TA1;void*_T31B[1];_TA6=_T31B + 0;*_TA6=& _T31A;_TA7=_tag_fat("rm %s\n",sizeof(char),7U);_TA8=_tag_fat(_T31B,sizeof(void*),1);Cyc_prscript(_TA7,_TA8);}goto _TL127;
# 1443
_TL126: maybe=Cyc_fopen(cppinfile,"w");_TA9=maybe;_TAA=(unsigned)_TA9;
if(_TAA)goto _TL12C;else{goto _TL12E;}
_TL12E:{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{const char*_T31B=cppinfile;_TAD=(void*)_T31B;_TAE=(void*)_T31B;_TAF=_get_zero_arr_size_char(_TAE,1U);_TAC=_tag_fat(_TAD,sizeof(char),_TAF);}_T31A.f1=_TAC;_TAB=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_TAB;void*_T31B[1];_TB0=_T31B + 0;*_TB0=& _T31A;_TB1=Cyc_stderr;_TB2=_tag_fat("Error: could not create file %s\n",sizeof(char),33U);_TB3=_tag_fat(_T31B,sizeof(void*),1);Cyc_fprintf(_TB1,_TB2,_TB3);}{int _T31A=1;_npop_handler(0);return _T31A;}_TL12C: _TB4=Cyc_verbose;
# 1448
if(!_TB4)goto _TL12F;{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{const char*_T31B=cppinfile;_TB7=(void*)_T31B;_TB8=(void*)_T31B;_TB9=_get_zero_arr_size_char(_TB8,1U);_TB6=_tag_fat(_TB7,sizeof(char),_TB9);}
_T31A.f1=_TB6;_TB5=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_TB5;void*_T31B[1];_TBA=_T31B + 0;*_TBA=& _T31A;_TBB=Cyc_stderr;_TBC=_tag_fat("Creating %s\n",sizeof(char),13U);_TBD=_tag_fat(_T31B,sizeof(void*),1);Cyc_fprintf(_TBB,_TBC,_TBD);}goto _TL130;_TL12F: _TL130:
 out_file=maybe;{
struct Cyc_List_List*l=cpp_insert;_TL134: if(l!=0)goto _TL132;else{goto _TL133;}
_TL132: _TBE=l;_TBF=_TBE->hd;_TC0=(struct _fat_ptr*)_TBF;_TC1=*_TC0;_TC2=_untag_fat_ptr_check_bound(_TC1,sizeof(char),1U);_TC3=_check_null(_TC2);_TC4=(const char*)_TC3;_TC5=out_file;Cyc_fputs(_TC4,_TC5);_TC6=l;
# 1451
l=_TC6->tl;goto _TL134;_TL133:;}{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{const char*_T31B=filename;_TC9=(void*)_T31B;_TCA=(void*)_T31B;_TCB=_get_zero_arr_size_char(_TCA,1U);_TC8=_tag_fat(_TC9,sizeof(char),_TCB);}
# 1454
_T31A.f1=_TC8;_TC7=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_TC7;void*_T31B[1];_TCC=_T31B + 0;*_TCC=& _T31A;_TCD=out_file;_TCE=_tag_fat("#include <%s>\n",sizeof(char),15U);_TCF=_tag_fat(_T31B,sizeof(void*),1);Cyc_fprintf(_TCD,_TCE,_TCF);}
Cyc_fclose(out_file);{struct Cyc_List_List*_T31A=_cycalloc(sizeof(struct Cyc_List_List));{struct _fat_ptr*_T31B=_cycalloc(sizeof(struct _fat_ptr));_TD2=
# 1457
_tag_fat("",sizeof(char),1U);*_T31B=_TD2;_TD1=(struct _fat_ptr*)_T31B;}_T31A->hd=_TD1;_TD4=Cyc_List_map;{
struct Cyc_List_List*(*_T31B)(struct _fat_ptr*(*)(struct _fat_ptr*),struct Cyc_List_List*)=(struct Cyc_List_List*(*)(struct _fat_ptr*(*)(struct _fat_ptr*),struct Cyc_List_List*))_TD4;_TD3=_T31B;}_TD5=Cyc_List_rev(Cyc_cppargs);_T31A->tl=_TD3(Cyc_sh_escape_stringptr,_TD5);_TD0=(struct Cyc_List_List*)_T31A;}_TD6=
_tag_fat(" ",sizeof(char),2U);{
# 1456
struct _fat_ptr cppargs_string=
Cyc_str_sepstr(_TD0,_TD6);{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;
# 1462
_T31A.f1=Cyc_cyclone_cc;_TD8=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_TD8;{struct Cyc_String_pa_PrintArg_struct _T31B;_T31B.tag=0;_T31B.f1=Cyc_target_cflags;_TD9=_T31B;}{struct Cyc_String_pa_PrintArg_struct _T31B=_TD9;{struct Cyc_String_pa_PrintArg_struct _T31C;_T31C.tag=0;
_T31C.f1=cppargs_string;_TDA=_T31C;}{struct Cyc_String_pa_PrintArg_struct _T31C=_TDA;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;{const char*_T31E=macrosfile;_TDD=(void*)_T31E;_TDE=(void*)_T31E;_TDF=_get_zero_arr_size_char(_TDE,1U);_TDC=_tag_fat(_TDD,sizeof(char),_TDF);}_T31D.f1=_TDC;_TDB=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_TDB;{struct Cyc_String_pa_PrintArg_struct _T31E;_T31E.tag=0;{const char*_T31F=cppinfile;_TE2=(void*)_T31F;_TE3=(void*)_T31F;_TE4=_get_zero_arr_size_char(_TE3,1U);_TE1=_tag_fat(_TE2,sizeof(char),_TE4);}_T31E.f1=_TE1;_TE0=_T31E;}{struct Cyc_String_pa_PrintArg_struct _T31E=_TE0;{struct Cyc_String_pa_PrintArg_struct _T31F;_T31F.tag=0;_TE6=Cyc_verbose;
if(!_TE6)goto _TL135;_T31F.f1=_tag_fat("",sizeof(char),1U);goto _TL136;_TL135: _T31F.f1=_tag_fat("-w",sizeof(char),3U);_TL136: _TE5=_T31F;}{struct Cyc_String_pa_PrintArg_struct _T31F=_TE5;void*_T320[6];_TE7=_T320 + 0;*_TE7=& _T31A;_TE8=_T320 + 1;*_TE8=& _T31B;_TE9=_T320 + 2;*_TE9=& _T31C;_TEA=_T320 + 3;*_TEA=& _T31D;_TEB=_T320 + 4;*_TEB=& _T31E;_TEC=_T320 + 5;*_TEC=& _T31F;_TED=
# 1461
_tag_fat("%s %s %s -E -dM -o %s -x c %s %s",sizeof(char),33U);_TEE=_tag_fat(_T320,sizeof(void*),6);_TD7=Cyc_aprintf(_TED,_TEE);}}}}}}_TEF=_untag_fat_ptr_check_bound(_TD7,sizeof(char),1U);_TF0=_check_null(_TEF);{char*cmd=(char*)_TF0;_TF1=Cyc_verbose;
# 1465
if(!_TF1)goto _TL137;{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{char*_T31B=cmd;_TF4=(void*)_T31B;_TF5=(void*)_T31B;_TF6=_get_zero_arr_size_char(_TF5,1U);_TF3=_tag_fat(_TF4,sizeof(char),_TF6);}
_T31A.f1=_TF3;_TF2=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_TF2;void*_T31B[1];_TF7=_T31B + 0;*_TF7=& _T31A;_TF8=Cyc_stderr;_TF9=_tag_fat("%s\n",sizeof(char),4U);_TFA=_tag_fat(_T31B,sizeof(void*),1);Cyc_fprintf(_TF8,_TF9,_TFA);}goto _TL138;_TL137: _TL138: _TFB=
system(cmd);if(_TFB)goto _TL139;else{goto _TL13B;}
# 1470
_TL13B:{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;
_T31A.f1=Cyc_cyclone_cc;_TFD=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_TFD;{struct Cyc_String_pa_PrintArg_struct _T31B;_T31B.tag=0;_T31B.f1=Cyc_target_cflags;_TFE=_T31B;}{struct Cyc_String_pa_PrintArg_struct _T31B=_TFE;{struct Cyc_String_pa_PrintArg_struct _T31C;_T31C.tag=0;
_T31C.f1=cppargs_string;_TFF=_T31C;}{struct Cyc_String_pa_PrintArg_struct _T31C=_TFF;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;{const char*_T31E=declsfile;_T102=(void*)_T31E;_T103=(void*)_T31E;_T104=_get_zero_arr_size_char(_T103,1U);_T101=_tag_fat(_T102,sizeof(char),_T104);}_T31D.f1=_T101;_T100=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T100;{struct Cyc_String_pa_PrintArg_struct _T31E;_T31E.tag=0;{const char*_T31F=cppinfile;_T107=(void*)_T31F;_T108=(void*)_T31F;_T109=_get_zero_arr_size_char(_T108,1U);_T106=_tag_fat(_T107,sizeof(char),_T109);}_T31E.f1=_T106;_T105=_T31E;}{struct Cyc_String_pa_PrintArg_struct _T31E=_T105;{struct Cyc_String_pa_PrintArg_struct _T31F;_T31F.tag=0;_T10B=Cyc_verbose;
if(!_T10B)goto _TL13C;_T31F.f1=_tag_fat("",sizeof(char),1U);goto _TL13D;_TL13C: _T31F.f1=_tag_fat("-w",sizeof(char),3U);_TL13D: _T10A=_T31F;}{struct Cyc_String_pa_PrintArg_struct _T31F=_T10A;void*_T320[6];_T10C=_T320 + 0;*_T10C=& _T31A;_T10D=_T320 + 1;*_T10D=& _T31B;_T10E=_T320 + 2;*_T10E=& _T31C;_T10F=_T320 + 3;*_T10F=& _T31D;_T110=_T320 + 4;*_T110=& _T31E;_T111=_T320 + 5;*_T111=& _T31F;_T112=
# 1470
_tag_fat("%s %s %s -E -o %s -x c %s %s",sizeof(char),29U);_T113=_tag_fat(_T320,sizeof(void*),6);_TFC=Cyc_aprintf(_T112,_T113);}}}}}}_T114=_untag_fat_ptr_check_bound(_TFC,sizeof(char),1U);_T115=_check_null(_T114);cmd=(char*)_T115;_T116=Cyc_verbose;
# 1474
if(!_T116)goto _TL13E;{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{char*_T31B=cmd;_T119=(void*)_T31B;_T11A=(void*)_T31B;_T11B=_get_zero_arr_size_char(_T11A,1U);_T118=_tag_fat(_T119,sizeof(char),_T11B);}
_T31A.f1=_T118;_T117=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_T117;void*_T31B[1];_T11C=_T31B + 0;*_T11C=& _T31A;_T11D=Cyc_stderr;_T11E=_tag_fat("%s\n",sizeof(char),4U);_T11F=_tag_fat(_T31B,sizeof(void*),1);Cyc_fprintf(_T11D,_T11E,_T11F);}goto _TL13F;_TL13E: _TL13F:
 system(cmd);goto _TL13A;_TL139: _TL13A:;}}_TL127: goto _TL125;_TL124: _TL125: _T120=
# 1481
Cyc_gathering();if(!_T120)goto _TL140;{int _T31A=0;_npop_handler(0);return _T31A;}_TL140:{
# 1484
struct Cyc_Hashtable_Table*t=Cyc_new_deps();
maybe=Cyc_fopen(macrosfile,"r");_T121=maybe;_T122=(unsigned)_T121;
if(_T122)goto _TL142;else{goto _TL144;}_TL144:{struct Cyc_NO_SUPPORT_exn_struct*_T31A=_cycalloc(sizeof(struct Cyc_NO_SUPPORT_exn_struct));_T31A->tag=Cyc_NO_SUPPORT;{struct Cyc_String_pa_PrintArg_struct _T31B;_T31B.tag=0;{const char*_T31C=macrosfile;_T127=(void*)_T31C;_T128=(void*)_T31C;_T129=_get_zero_arr_size_char(_T128,1U);_T126=_tag_fat(_T127,sizeof(char),_T129);}
_T31B.f1=_T126;_T125=_T31B;}{struct Cyc_String_pa_PrintArg_struct _T31B=_T125;void*_T31C[1];_T12A=_T31C + 0;*_T12A=& _T31B;_T12B=
# 1486
_tag_fat("can't open macrosfile %s",sizeof(char),25U);_T12C=_tag_fat(_T31C,sizeof(void*),1);_T124=Cyc_aprintf(_T12B,_T12C);}_T31A->f1=_T124;_T123=(struct Cyc_NO_SUPPORT_exn_struct*)_T31A;}_T12D=(void*)_T123;_throw(_T12D);goto _TL143;_TL142: _TL143: _T12E=Cyc_verbose;
# 1489
if(!_T12E)goto _TL145;_T12F=Cyc_stderr;_T130=_tag_fat("Getting macros...",sizeof(char),18U);_T131=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T12F,_T130,_T131);goto _TL146;_TL145: _TL146:
 in_file=maybe;{
struct Cyc_Lexing_lexbuf*l=Cyc_Lexing_from_file(in_file);
struct _tuple13*entry;
_TL147: entry=Cyc_line(l);_T132=entry;if(_T132!=0)goto _TL148;else{goto _TL149;}
_TL148:{struct _tuple13*_T31A=entry;struct Cyc_Set_Set*_T31B;struct _fat_ptr*_T31C;{struct _tuple13 _T31D=*_T31A;_T31C=_T31D.f0;_T31B=_T31D.f1;}{struct _fat_ptr*name=_T31C;struct Cyc_Set_Set*uses=_T31B;_T134=Cyc_Hashtable_insert;{
void(*_T31D)(struct Cyc_Hashtable_Table*,struct _fat_ptr*,struct Cyc_Set_Set*)=(void(*)(struct Cyc_Hashtable_Table*,struct _fat_ptr*,struct Cyc_Set_Set*))_T134;_T133=_T31D;}_T133(t,name,uses);}}goto _TL147;_TL149:
# 1499
 Cyc_fclose(in_file);_T135=Cyc_verbose;
if(!_T135)goto _TL14A;_T136=Cyc_stderr;_T137=_tag_fat("done.\n",sizeof(char),7U);_T138=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T136,_T137,_T138);goto _TL14B;_TL14A: _TL14B:
# 1503
 maybe=Cyc_fopen(declsfile,"r");_T139=maybe;_T13A=(unsigned)_T139;
if(_T13A)goto _TL14C;else{goto _TL14E;}_TL14E:{struct Cyc_NO_SUPPORT_exn_struct*_T31A=_cycalloc(sizeof(struct Cyc_NO_SUPPORT_exn_struct));_T31A->tag=Cyc_NO_SUPPORT;{struct Cyc_String_pa_PrintArg_struct _T31B;_T31B.tag=0;{const char*_T31C=declsfile;_T13F=(void*)_T31C;_T140=(void*)_T31C;_T141=_get_zero_arr_size_char(_T140,1U);_T13E=_tag_fat(_T13F,sizeof(char),_T141);}
_T31B.f1=_T13E;_T13D=_T31B;}{struct Cyc_String_pa_PrintArg_struct _T31B=_T13D;void*_T31C[1];_T142=_T31C + 0;*_T142=& _T31B;_T143=
# 1504
_tag_fat("can't open declsfile %s",sizeof(char),24U);_T144=_tag_fat(_T31C,sizeof(void*),1);_T13C=Cyc_aprintf(_T143,_T144);}_T31A->f1=_T13C;_T13B=(struct Cyc_NO_SUPPORT_exn_struct*)_T31A;}_T145=(void*)_T13B;_throw(_T145);goto _TL14D;_TL14C: _TL14D: _T146=Cyc_verbose;
# 1507
if(!_T146)goto _TL14F;_T147=Cyc_stderr;_T148=_tag_fat("Extracting declarations...",sizeof(char),27U);_T149=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T147,_T148,_T149);goto _TL150;_TL14F: _TL150:
 in_file=maybe;
l=Cyc_Lexing_from_file(in_file);
Cyc_slurp_out=Cyc_fopen(filtereddeclsfile,"w");_T14A=Cyc_slurp_out;_T14B=(unsigned)_T14A;
if(_T14B)goto _TL151;else{goto _TL153;}_TL153:{int _T31A=1;_npop_handler(0);return _T31A;}_TL151:
 _TL154: _T14C=Cyc_slurp(l);if(_T14C)goto _TL155;else{goto _TL156;}_TL155: goto _TL154;_TL156: _T14D=Cyc_verbose;
if(!_T14D)goto _TL157;_T14E=Cyc_stderr;_T14F=_tag_fat("done.\n",sizeof(char),7U);_T150=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T14E,_T14F,_T150);goto _TL158;_TL157: _TL158: {
# 1515
struct Cyc_List_List*x=user_defs;
_TL159: if(x!=0)goto _TL15A;else{goto _TL15B;}
_TL15A: _T151=x;_T152=_T151->hd;{struct _tuple16*_T31A=(struct _tuple16*)_T152;struct _fat_ptr*_T31B;{struct _tuple16 _T31C=*_T31A;_T31B=_T31C.f1;}{struct _fat_ptr*s=_T31B;{struct Cyc_String_pa_PrintArg_struct _T31C;_T31C.tag=0;_T154=s;
_T31C.f1=*_T154;_T153=_T31C;}{struct Cyc_String_pa_PrintArg_struct _T31C=_T153;void*_T31D[1];_T155=_T31D + 0;*_T155=& _T31C;_T156=_check_null(Cyc_slurp_out);_T157=_tag_fat("%s",sizeof(char),3U);_T158=_tag_fat(_T31D,sizeof(void*),1);Cyc_fprintf(_T156,_T157,_T158);}_T159=x;
x=_T159->tl;}}goto _TL159;_TL15B:
# 1521
 Cyc_fclose(in_file);_T15A=
_check_null(Cyc_slurp_out);Cyc_fclose(_T15A);_T15B=Cyc_mode;_T15C=(int)_T15B;
if(_T15C==3)goto _TL15C;goto _TL15D;_TL15C: _TL15D:
# 1527
 maybe=Cyc_fopen(filtereddeclsfile,"r");_T15D=maybe;_T15E=(unsigned)_T15D;
if(_T15E)goto _TL15E;else{goto _TL160;}_TL160:{int _T31A=1;_npop_handler(0);return _T31A;}_TL15E: _T15F=Cyc_verbose;
if(!_T15F)goto _TL161;_T160=Cyc_stderr;_T161=_tag_fat("Parsing declarations...",sizeof(char),24U);_T162=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T160,_T161,_T162);goto _TL162;_TL161: _TL162:
 in_file=maybe;{const char*_T31A=filtereddeclsfile;_T164=(void*)_T31A;_T165=(void*)_T31A;_T166=_get_zero_arr_size_char(_T165,1U);_T163=_tag_fat(_T164,sizeof(char),_T166);}
Cyc_Warn_reset(_T163);
Cyc_Lex_lex_init(0);
Cyc_Warn_print_warnings=Cyc_verbose;{
struct Cyc_List_List*decls=0;{struct _handler_cons _T31A;_T167=& _T31A;_push_handler(_T167);{int _T31B=0;_T168=_T31A.handler;_T169=setjmp(_T168);if(!_T169)goto _TL163;_T31B=1;goto _TL164;_TL163: _TL164: if(_T31B)goto _TL165;else{goto _TL167;}_TL167:
# 1536
 decls=Cyc_Parse_parse_file(in_file);
Cyc_Warn_print_warnings=1;
Cyc_Lex_lex_init(0);
Cyc_fclose(in_file);_T16A=Cyc_verbose;
if(!_T16A)goto _TL168;_T16B=Cyc_stderr;_T16C=_tag_fat("done.\n",sizeof(char),7U);_T16D=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T16B,_T16C,_T16D);goto _TL169;_TL168: _TL169: _pop_handler();goto _TL166;_TL165: _T16E=Cyc_Core_get_exn_thrown();{void*_T31C=(void*)_T16E;void*_T31D;_T31D=_T31C;{void*x=_T31D;
# 1544
Cyc_Warn_print_warnings=1;
Cyc_Lex_lex_init(0);
Cyc_fclose(in_file);_T16F=Cyc_verbose;
if(!_T16F)goto _TL16A;_T170=Cyc_stderr;_T171=_tag_fat("exception thrown.\n",sizeof(char),19U);_T172=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T170,_T171,_T172);goto _TL16B;_TL16A: _TL16B:
 Cyc_Core_rethrow(x);goto _LL6;}_LL6:;}_TL166:;}}{
# 1552
struct Cyc_List_List*d=decls;_TL16F: if(d!=0)goto _TL16D;else{goto _TL16E;}
_TL16D: _T173=d;_T174=_T173->hd;_T175=(struct Cyc_Absyn_Decl*)_T174;_T176=t;Cyc_scan_decl(_T175,_T176);_T177=d;
# 1552
d=_T177->tl;goto _TL16F;_TL16E:;}_T179=Cyc_List_map;{
# 1556
struct Cyc_List_List*(*_T31A)(struct _fat_ptr*(*)(struct _fat_ptr*),struct Cyc_List_List*)=(struct Cyc_List_List*(*)(struct _fat_ptr*(*)(struct _fat_ptr*),struct Cyc_List_List*))_T179;_T178=_T31A;}_T17A=Cyc_List_split(user_defs);_T17B=_T17A.f0;{struct Cyc_List_List*user_symbols=_T178(Cyc_add_user_prefix,_T17B);_T17C=
Cyc_List_append(start_symbols,user_symbols);_T17D=t;{struct Cyc_Set_Set*reachable_set=Cyc_reachable(_T17C,_T17D);
# 1560
struct Cyc_List_List*reachable_decls=0;
struct Cyc_List_List*user_decls=0;_T17F=Cyc_Set_empty;{
struct Cyc_Set_Set*(*_T31A)(int(*)(struct _fat_ptr*,struct _fat_ptr*))=(struct Cyc_Set_Set*(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*)))_T17F;_T17E=_T31A;}_T180=Cyc_strptrcmp;{struct Cyc_Set_Set*defined_symbols=_T17E(_T180);{
struct Cyc_List_List*d=decls;_TL173: if(d!=0)goto _TL171;else{goto _TL172;}
_TL171: _T181=d;_T182=_T181->hd;{struct Cyc_Absyn_Decl*decl=(struct Cyc_Absyn_Decl*)_T182;
struct _fat_ptr*name;_T183=decl;{
void*_T31A=_T183->r;struct Cyc_Absyn_Typedefdecl*_T31B;struct Cyc_Absyn_Enumdecl*_T31C;struct Cyc_Absyn_Aggrdecl*_T31D;struct Cyc_Absyn_Fndecl*_T31E;struct Cyc_Absyn_Vardecl*_T31F;_T184=(int*)_T31A;_T185=*_T184;switch(_T185){case 0:{struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_T31A;_T31F=_T320->f1;}{struct Cyc_Absyn_Vardecl*x=_T31F;_T186=x;{
# 1568
struct _tuple1*_T320=_T186->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;_T188=Cyc_Set_insert;{
struct Cyc_Set_Set*(*_T322)(struct Cyc_Set_Set*,struct _fat_ptr*)=(struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T188;_T187=_T322;}defined_symbols=_T187(defined_symbols,v);_T18A=Cyc_List_mem;{
int(*_T322)(int(*)(struct _fat_ptr*,struct _fat_ptr*),struct Cyc_List_List*,struct _fat_ptr*)=(int(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*),struct Cyc_List_List*,struct _fat_ptr*))_T18A;_T189=_T322;}_T18B=Cyc_strptrcmp;_T18C=omit_symbols;_T18D=v;_T18E=_T189(_T18B,_T18C,_T18D);if(!_T18E)goto _TL175;name=0;goto _TL176;
_TL175: name=v;_TL176: goto _LL9;}}}case 1:{struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_T31A;_T31E=_T320->f1;}{struct Cyc_Absyn_Fndecl*x=_T31E;_T18F=x;{
# 1574
struct _tuple1*_T320=_T18F->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;_T191=Cyc_Set_insert;{
struct Cyc_Set_Set*(*_T322)(struct Cyc_Set_Set*,struct _fat_ptr*)=(struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T191;_T190=_T322;}defined_symbols=_T190(defined_symbols,v);_T193=Cyc_List_mem;{
int(*_T322)(int(*)(struct _fat_ptr*,struct _fat_ptr*),struct Cyc_List_List*,struct _fat_ptr*)=(int(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*),struct Cyc_List_List*,struct _fat_ptr*))_T193;_T192=_T322;}_T194=Cyc_strptrcmp;_T195=omit_symbols;_T196=v;_T197=_T192(_T194,_T195,_T196);if(!_T197)goto _TL177;name=0;goto _TL178;
_TL177: name=v;_TL178: goto _LL9;}}}case 5:{struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_T31A;_T31D=_T320->f1;}{struct Cyc_Absyn_Aggrdecl*x=_T31D;_T198=x;{
# 1580
struct _tuple1*_T320=_T198->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;
name=v;goto _LL9;}}}case 7:{struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_T31A;_T31C=_T320->f1;}{struct Cyc_Absyn_Enumdecl*x=_T31C;_T199=x;{
# 1584
struct _tuple1*_T320=_T199->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;
name=v;
# 1588
if(name==0)goto _TL179;_T19B=Cyc_Set_member;{int(*_T322)(struct Cyc_Set_Set*,struct _fat_ptr*)=(int(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T19B;_T19A=_T322;}_T19C=_T19A(reachable_set,name);if(!_T19C)goto _TL179;{struct Cyc_List_List*_T322=_cycalloc(sizeof(struct Cyc_List_List));
_T322->hd=decl;_T322->tl=reachable_decls;_T19D=(struct Cyc_List_List*)_T322;}reachable_decls=_T19D;goto _TL17A;
# 1591
_TL179: _T19E=x;_T19F=_T19E->fields;_T1A0=(unsigned)_T19F;if(!_T1A0)goto _TL17B;_T1A1=x;_T1A2=_T1A1->fields;_T1A3=_T1A2->v;{
struct Cyc_List_List*fs=(struct Cyc_List_List*)_T1A3;_TL180: if(fs!=0)goto _TL17E;else{goto _TL17F;}
_TL17E: _T1A4=fs;_T1A5=_T1A4->hd;{struct Cyc_Absyn_Enumfield*f=(struct Cyc_Absyn_Enumfield*)_T1A5;_T1A6=f;{
struct _tuple1*_T322=_T1A6->name;struct _fat_ptr*_T323;{struct _tuple1 _T324=*_T322;_T323=_T324.f1;}{struct _fat_ptr*v=_T323;_T1A8=Cyc_Set_member;{
int(*_T324)(struct Cyc_Set_Set*,struct _fat_ptr*)=(int(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T1A8;_T1A7=_T324;}_T1A9=_T1A7(reachable_set,v);if(!_T1A9)goto _TL181;{struct Cyc_List_List*_T324=_cycalloc(sizeof(struct Cyc_List_List));
_T324->hd=decl;_T324->tl=reachable_decls;_T1AA=(struct Cyc_List_List*)_T324;}reachable_decls=_T1AA;goto _TL17F;_TL181:;}}}_T1AB=fs;
# 1592
fs=_T1AB->tl;goto _TL180;_TL17F:;}goto _TL17C;_TL17B: _TL17C: _TL17A:
# 1601
 name=0;goto _LL9;}}}case 8:{struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*)_T31A;_T31B=_T320->f1;}{struct Cyc_Absyn_Typedefdecl*x=_T31B;_T1AC=x;{
# 1604
struct _tuple1*_T320=_T1AC->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;
name=v;goto _LL9;}}}case 13: goto _LL17;case 14: _LL17: goto _LL19;case 15: _LL19: goto _LL1B;case 16: _LL1B: goto _LL1D;case 2: _LL1D: goto _LL1F;case 6: _LL1F: goto _LL21;case 3: _LL21: goto _LL23;case 9: _LL23: goto _LL25;case 10: _LL25: goto _LL27;case 11: _LL27: goto _LL29;case 12: _LL29: goto _LL2B;default: _LL2B:
# 1619
 name=0;goto _LL9;}_LL9:;}
# 1623
if(name==0)goto _TL183;_T1AE=Cyc_Set_member;{int(*_T31A)(struct Cyc_Set_Set*,struct _fat_ptr*)=(int(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T1AE;_T1AD=_T31A;}_T1AF=_T1AD(reachable_set,name);if(!_T1AF)goto _TL183;_T1B0=name;_T1B1=*_T1B0;_T1B2=Cyc_user_prefix;_T1B3=
Cyc_strlen(Cyc_user_prefix);_T1B4=Cyc_strncmp(_T1B1,_T1B2,_T1B3);if(_T1B4==0)goto _TL185;{struct Cyc_List_List*_T31A=_cycalloc(sizeof(struct Cyc_List_List));
_T31A->hd=decl;_T31A->tl=reachable_decls;_T1B5=(struct Cyc_List_List*)_T31A;}reachable_decls=_T1B5;goto _TL186;
# 1628
_TL185: Cyc_rename_decl(decl);{struct Cyc_List_List*_T31A=_cycalloc(sizeof(struct Cyc_List_List));
_T31A->hd=decl;_T31A->tl=user_decls;_T1B6=(struct Cyc_List_List*)_T31A;}user_decls=_T1B6;_TL186: goto _TL184;_TL183: _TL184:;}_T1B7=d;
# 1563
d=_T1B7->tl;goto _TL173;_TL172:;}_T1B8=Cyc_do_setjmp;
# 1635
if(_T1B8)goto _TL187;else{goto _TL189;}
_TL189: maybe=Cyc_fopen(filename,"w");_T1B9=maybe;_T1BA=(unsigned)_T1B9;
if(_T1BA)goto _TL18A;else{goto _TL18C;}_TL18C:{int _T31A=1;_npop_handler(0);return _T31A;}_TL18A:
 out_file=maybe;goto _TL188;
_TL187: out_file=Cyc_stdout;_TL188:{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{const char*_T31B=filename;_T1BE=(void*)_T31B;_T1BF=(void*)_T31B;_T1C0=_get_zero_arr_size_char(_T1BF,1U);_T1BD=_tag_fat(_T1BE,sizeof(char),_T1C0);}
_T31A.f1=_T1BD;_T1BC=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_T1BC;void*_T31B[1];_T1C1=_T31B + 0;*_T1C1=& _T31A;_T1C2=_tag_fat("_%s_",sizeof(char),5U);_T1C3=_tag_fat(_T31B,sizeof(void*),1);_T1BB=Cyc_aprintf(_T1C2,_T1C3);}{struct _fat_ptr ifdefmacro=_T1BB;{
int j=0;_TL190: _T1C4=j;_T1C5=(unsigned)_T1C4;_T1C6=ifdefmacro;_T1C7=_get_fat_size(_T1C6,sizeof(char));if(_T1C5 < _T1C7)goto _TL18E;else{goto _TL18F;}
_TL18E: _T1C8=ifdefmacro;_T1C9=_T1C8.curr;_T1CA=(char*)_T1C9;_T1CB=j;_T1CC=_T1CA[_T1CB];_T1CD=(int)_T1CC;if(_T1CD==46)goto _TL193;else{goto _TL194;}_TL194: _T1CE=ifdefmacro;_T1CF=_T1CE.curr;_T1D0=(char*)_T1CF;_T1D1=j;_T1D2=_T1D0[_T1D1];_T1D3=(int)_T1D2;if(_T1D3==47)goto _TL193;else{goto _TL191;}
_TL193: _T1D4=ifdefmacro;_T1D5=j;{struct _fat_ptr _T31A=_fat_ptr_plus(_T1D4,sizeof(char),_T1D5);_T1D6=_T31A.curr;_T1D7=(char*)_T1D6;{char _T31B=*_T1D7;char _T31C='_';_T1D8=_get_fat_size(_T31A,sizeof(char));if(_T1D8!=1U)goto _TL195;if(_T31B!=0)goto _TL195;if(_T31C==0)goto _TL195;_throw_arraybounds();goto _TL196;_TL195: _TL196: _T1D9=_T31A.curr;_T1DA=(char*)_T1D9;*_T1DA=_T31C;}}goto _TL192;
_TL191: _T1DB=ifdefmacro;_T1DC=_T1DB.curr;_T1DD=(char*)_T1DC;_T1DE=j;_T1DF=_T1DD[_T1DE];_T1E0=(int)_T1DF;if(_T1E0==95)goto _TL197;_T1E1=ifdefmacro;_T1E2=_T1E1.curr;_T1E3=(char*)_T1E2;_T1E4=j;_T1E5=_T1E3[_T1E4];_T1E6=(int)_T1E5;if(_T1E6==47)goto _TL197;_T1E7=ifdefmacro;_T1E8=j;{struct _fat_ptr _T31A=_fat_ptr_plus(_T1E7,sizeof(char),_T1E8);_T1E9=_T31A.curr;_T1EA=(char*)_T1E9;{char _T31B=*_T1EA;_T1EB=ifdefmacro;_T1EC=_T1EB.curr;_T1ED=(char*)_T1EC;_T1EE=j;_T1EF=_T1ED[_T1EE];_T1F0=(int)_T1EF;_T1F1=
toupper(_T1F0);{char _T31C=(char)_T1F1;_T1F2=_get_fat_size(_T31A,sizeof(char));if(_T1F2!=1U)goto _TL199;if(_T31B!=0)goto _TL199;if(_T31C==0)goto _TL199;_throw_arraybounds();goto _TL19A;_TL199: _TL19A: _T1F3=_T31A.curr;_T1F4=(char*)_T1F3;*_T1F4=_T31C;}}}goto _TL198;_TL197: _TL198: _TL192:
# 1641
 j=j + 1;goto _TL190;_TL18F:;}_T1F5=Cyc_do_setjmp;
# 1647
if(_T1F5)goto _TL19B;else{goto _TL19D;}
_TL19D: _T1F6=out_file;_T1F7=_tag_fat("__noinference__{\n",sizeof(char),18U);_T1F8=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T1F6,_T1F7,_T1F8);goto _TL19C;_TL19B: _TL19C:{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;
# 1652
_T31A.f1=ifdefmacro;_T1F9=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_T1F9;{struct Cyc_String_pa_PrintArg_struct _T31B;_T31B.tag=0;
_T31B.f1=ifdefmacro;_T1FA=_T31B;}{struct Cyc_String_pa_PrintArg_struct _T31B=_T1FA;void*_T31C[2];_T1FB=_T31C + 0;*_T1FB=& _T31A;_T1FC=_T31C + 1;*_T1FC=& _T31B;_T1FD=out_file;_T1FE=
# 1651
_tag_fat("#ifndef %s\n#define %s\n",sizeof(char),23U);_T1FF=_tag_fat(_T31C,sizeof(void*),2);Cyc_fprintf(_T1FD,_T1FE,_T1FF);}}{
# 1656
struct Cyc_List_List*print_decls=0;
struct Cyc_List_List*names=0;{
struct Cyc_List_List*d=reachable_decls;_TL1A1: if(d!=0)goto _TL19F;else{goto _TL1A0;}
_TL19F: _T200=d;_T201=_T200->hd;{struct Cyc_Absyn_Decl*decl=(struct Cyc_Absyn_Decl*)_T201;
int anon_enum=0;
struct _fat_ptr*name;_T202=decl;{
void*_T31A=_T202->r;struct Cyc_Absyn_Typedefdecl*_T31B;struct Cyc_Absyn_Enumdecl*_T31C;struct Cyc_Absyn_Aggrdecl*_T31D;struct Cyc_Absyn_Fndecl*_T31E;struct Cyc_Absyn_Vardecl*_T31F;_T203=(int*)_T31A;_T204=*_T203;switch(_T204){case 0:{struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Var_d_Absyn_Raw_decl_struct*)_T31A;_T31F=_T320->f1;}{struct Cyc_Absyn_Vardecl*x=_T31F;_T205=x;{
# 1664
struct _tuple1*_T320=_T205->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;
name=v;goto _LL3E;}}}case 1:{struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Fn_d_Absyn_Raw_decl_struct*)_T31A;_T31E=_T320->f1;}{struct Cyc_Absyn_Fndecl*x=_T31E;_T206=x;_T207=_T206->is_inline;
# 1668
if(!_T207)goto _TL1A3;name=0;goto _LL3E;_TL1A3: _T208=x;{
struct _tuple1*_T320=_T208->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;
name=v;goto _LL3E;}}}case 5:{struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Aggr_d_Absyn_Raw_decl_struct*)_T31A;_T31D=_T320->f1;}{struct Cyc_Absyn_Aggrdecl*x=_T31D;_T209=x;{
# 1673
struct _tuple1*_T320=_T209->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;
name=v;goto _LL3E;}}}case 7:{struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Enum_d_Absyn_Raw_decl_struct*)_T31A;_T31C=_T320->f1;}{struct Cyc_Absyn_Enumdecl*x=_T31C;_T20A=x;{
# 1677
struct _tuple1*_T320=_T20A->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;
name=v;goto _LL3E;}}}case 8:{struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*_T320=(struct Cyc_Absyn_Typedef_d_Absyn_Raw_decl_struct*)_T31A;_T31B=_T320->f1;}{struct Cyc_Absyn_Typedefdecl*x=_T31B;_T20B=x;{
# 1681
struct _tuple1*_T320=_T20B->name;struct _fat_ptr*_T321;{struct _tuple1 _T322=*_T320;_T321=_T322.f1;}{struct _fat_ptr*v=_T321;
name=v;goto _LL3E;}}}case 4: goto _LL4C;case 13: _LL4C: goto _LL4E;case 14: _LL4E: goto _LL50;case 15: _LL50: goto _LL52;case 16: _LL52: goto _LL54;case 2: _LL54: goto _LL56;case 6: _LL56: goto _LL58;case 3: _LL58: goto _LL5A;case 9: _LL5A: goto _LL5C;case 10: _LL5C: goto _LL5E;case 11: _LL5E: goto _LL60;default: _LL60:
# 1696
 name=0;goto _LL3E;}_LL3E:;}_T20C=name;_T20D=(unsigned)_T20C;
# 1699
if(_T20D)goto _TL1A5;else{goto _TL1A7;}_TL1A7: _T20E=anon_enum;if(_T20E)goto _TL1A5;else{goto _TL1A8;}_TL1A8: goto _TL19E;_TL1A5: _T210=Cyc_Set_member;{
# 1704
int(*_T31A)(struct Cyc_Set_Set*,struct _fat_ptr*)=(int(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T210;_T20F=_T31A;}_T211=reachable_set;_T212=Cyc_add_user_prefix(name);_T213=_T20F(_T211,_T212);if(!_T213)goto _TL1A9;{
struct Cyc_Absyn_Decl*user_decl=Cyc_Absyn_lookup_decl(user_decls,name);
if(user_decl!=0)goto _TL1AB;{struct Cyc_Core_Impossible_exn_struct*_T31A=_cycalloc(sizeof(struct Cyc_Core_Impossible_exn_struct));_T31A->tag=Cyc_Core_Impossible;
_T31A->f1=_tag_fat("Internal Error: bad user-def name",sizeof(char),34U);_T214=(struct Cyc_Core_Impossible_exn_struct*)_T31A;}_T215=(void*)_T214;_throw(_T215);goto _TL1AC;
# 1710
_TL1AB: _T216=user_decl;{void*_T31A=_T216->r;_T217=(int*)_T31A;_T218=*_T217;switch(_T218){case 0: goto _LL74;case 1: _LL74:{struct Cyc_NO_SUPPORT_exn_struct*_T31B=_cycalloc(sizeof(struct Cyc_NO_SUPPORT_exn_struct));_T31B->tag=Cyc_NO_SUPPORT;
# 1713
_T31B->f1=_tag_fat("user defintions for function or variable decls",sizeof(char),47U);_T219=(struct Cyc_NO_SUPPORT_exn_struct*)_T31B;}_T21A=(void*)_T219;_throw(_T21A);default: goto _LL70;}_LL70:;}_TL1AC:{struct Cyc_List_List*_T31A=_cycalloc(sizeof(struct Cyc_List_List));
# 1719
_T31A->hd=decl;_T31A->tl=print_decls;_T21B=(struct Cyc_List_List*)_T31A;}print_decls=_T21B;}goto _TL1AA;
# 1722
_TL1A9:{struct Cyc_List_List*_T31A=_cycalloc(sizeof(struct Cyc_List_List));_T31A->hd=decl;_T31A->tl=print_decls;_T21C=(struct Cyc_List_List*)_T31A;}print_decls=_T21C;_TL1AA:{struct Cyc_List_List*_T31A=_cycalloc(sizeof(struct Cyc_List_List));
_T31A->hd=name;_T31A->tl=names;_T21D=(struct Cyc_List_List*)_T31A;}names=_T21D;}_TL19E: _T21E=d;
# 1658
d=_T21E->tl;goto _TL1A1;_TL1A0:;}{struct _handler_cons _T31A;_T21F=& _T31A;_push_handler(_T21F);{int _T31B=0;_T220=_T31A.handler;_T221=setjmp(_T220);if(!_T221)goto _TL1AE;_T31B=1;goto _TL1AF;_TL1AE: _TL1AF: if(_T31B)goto _TL1B0;else{goto _TL1B2;}_TL1B2:
# 1728
 Cyc_Binding_resolve_all(print_decls);_T222=
Cyc_Tcenv_tc_init();_T223=print_decls;Cyc_Tc_tc(_T222,1,_T223);_pop_handler();goto _TL1B1;_TL1B0: _T224=Cyc_Core_get_exn_thrown();{void*_T31C=(void*)_T224;{struct Cyc_NO_SUPPORT_exn_struct*_T31D=_cycalloc(sizeof(struct Cyc_NO_SUPPORT_exn_struct));_T31D->tag=Cyc_NO_SUPPORT;
# 1731
_T31D->f1=_tag_fat("can't typecheck acquired declarations",sizeof(char),38U);_T225=(struct Cyc_NO_SUPPORT_exn_struct*)_T31D;}_T226=(void*)_T225;_throw(_T226);;}_TL1B1:;}}{struct _tuple0 _T31A;
# 1735
_T31A.f0=print_decls;_T31A.f1=names;_T227=_T31A;}{struct _tuple0 _T31A=_T227;struct Cyc_List_List*_T31B;struct Cyc_List_List*_T31C;_T31C=_T31A.f0;_T31B=_T31A.f1;{struct Cyc_List_List*d=_T31C;struct Cyc_List_List*n=_T31B;
_TL1B6:
# 1735
 if(d!=0)goto _TL1B7;else{goto _TL1B5;}_TL1B7: if(n!=0)goto _TL1B4;else{goto _TL1B5;}
# 1737
_TL1B4: _T228=d;_T229=_T228->hd;{struct Cyc_Absyn_Decl*decl=(struct Cyc_Absyn_Decl*)_T229;_T22A=n;_T22B=_T22A->hd;{
struct _fat_ptr*name=(struct _fat_ptr*)_T22B;
int anon_enum=0;_T22C=name;_T22D=(unsigned)_T22C;
if(_T22D)goto _TL1B8;else{goto _TL1BA;}
_TL1BA: anon_enum=1;goto _TL1B9;_TL1B8: _TL1B9: _T22E=& Cyc_Absynpp_cyc_params_r;_T22F=(struct Cyc_Absynpp_Params*)_T22E;
# 1744
Cyc_Absynpp_set_params(_T22F);_T230=name;_T231=(unsigned)_T230;
if(!_T231)goto _TL1BB;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;_T234=name;
_T31D.f1=*_T234;_T233=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T233;void*_T31E[1];_T235=_T31E + 0;*_T235=& _T31D;_T236=_tag_fat("_%s_def_",sizeof(char),9U);_T237=_tag_fat(_T31E,sizeof(void*),1);_T232=Cyc_aprintf(_T236,_T237);}ifdefmacro=_T232;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;
_T31D.f1=ifdefmacro;_T238=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T238;void*_T31E[1];_T239=_T31E + 0;*_T239=& _T31D;_T23A=out_file;_T23B=_tag_fat("#ifndef %s\n",sizeof(char),12U);_T23C=_tag_fat(_T31E,sizeof(void*),1);Cyc_fprintf(_T23A,_T23B,_T23C);}{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;
_T31D.f1=ifdefmacro;_T23D=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T23D;void*_T31E[1];_T23E=_T31E + 0;*_T23E=& _T31D;_T23F=out_file;_T240=_tag_fat("#define %s\n",sizeof(char),12U);_T241=_tag_fat(_T31E,sizeof(void*),1);Cyc_fprintf(_T23F,_T240,_T241);}{struct Cyc_Absyn_Decl*_T31D[1];_T243=_T31D + 0;*_T243=decl;_T244=_tag_fat(_T31D,sizeof(struct Cyc_Absyn_Decl*),1);_T242=Cyc_List_list(_T244);}_T245=out_file;
# 1750
Cyc_Absynpp_decllist2file(_T242,_T245);_T246=out_file;_T247=
_tag_fat("#endif\n",sizeof(char),8U);_T248=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T246,_T247,_T248);goto _TL1BC;
# 1755
_TL1BB:{struct Cyc_Absyn_Decl*_T31D[1];_T24A=_T31D + 0;*_T24A=decl;_T24B=_tag_fat(_T31D,sizeof(struct Cyc_Absyn_Decl*),1);_T249=Cyc_List_list(_T24B);}_T24C=out_file;Cyc_Absynpp_decllist2file(_T249,_T24C);_TL1BC:;}}_T24D=d;
# 1736
d=_T24D->tl;_T24E=n;n=_T24E->tl;goto _TL1B6;_TL1B5:;}}
# 1760
maybe=Cyc_fopen(macrosfile,"r");_T24F=maybe;_T250=(unsigned)_T24F;
if(_T250)goto _TL1BD;else{goto _TL1BF;}_TL1BF:{struct Cyc_NO_SUPPORT_exn_struct*_T31A=_cycalloc(sizeof(struct Cyc_NO_SUPPORT_exn_struct));_T31A->tag=Cyc_NO_SUPPORT;{struct Cyc_String_pa_PrintArg_struct _T31B;_T31B.tag=0;{const char*_T31C=macrosfile;_T255=(void*)_T31C;_T256=(void*)_T31C;_T257=_get_zero_arr_size_char(_T256,1U);_T254=_tag_fat(_T255,sizeof(char),_T257);}
_T31B.f1=_T254;_T253=_T31B;}{struct Cyc_String_pa_PrintArg_struct _T31B=_T253;void*_T31C[1];_T258=_T31C + 0;*_T258=& _T31B;_T259=
# 1761
_tag_fat("can't open macrosfile %s",sizeof(char),25U);_T25A=_tag_fat(_T31C,sizeof(void*),1);_T252=Cyc_aprintf(_T259,_T25A);}_T31A->f1=_T252;_T251=(struct Cyc_NO_SUPPORT_exn_struct*)_T31A;}_T25B=(void*)_T251;_throw(_T25B);goto _TL1BE;_TL1BD: _TL1BE:
# 1763
 in_file=maybe;
l=Cyc_Lexing_from_file(in_file);{
struct _tuple14*entry2;
_TL1C0: entry2=Cyc_suck_line(l);_T25C=entry2;if(_T25C!=0)goto _TL1C1;else{goto _TL1C2;}
_TL1C1:{struct _tuple14*_T31A=entry2;struct _fat_ptr*_T31B;struct _fat_ptr _T31C;{struct _tuple14 _T31D=*_T31A;_T31C=_T31D.f0;_T31B=_T31D.f1;}{struct _fat_ptr line=_T31C;struct _fat_ptr*name=_T31B;_T25E=Cyc_Set_member;{
int(*_T31D)(struct Cyc_Set_Set*,struct _fat_ptr*)=(int(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T25E;_T25D=_T31D;}_T25F=_T25D(reachable_set,name);if(!_T25F)goto _TL1C3;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;_T261=name;
_T31D.f1=*_T261;_T260=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T260;void*_T31E[1];_T262=_T31E + 0;*_T262=& _T31D;_T263=out_file;_T264=_tag_fat("#ifndef %s\n",sizeof(char),12U);_T265=_tag_fat(_T31E,sizeof(void*),1);Cyc_fprintf(_T263,_T264,_T265);}{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;
_T31D.f1=line;_T266=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T266;void*_T31E[1];_T267=_T31E + 0;*_T267=& _T31D;_T268=out_file;_T269=_tag_fat("%s\n",sizeof(char),4U);_T26A=_tag_fat(_T31E,sizeof(void*),1);Cyc_fprintf(_T268,_T269,_T26A);}_T26B=out_file;_T26C=
_tag_fat("#endif\n",sizeof(char),8U);_T26D=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T26B,_T26C,_T26D);goto _TL1C4;_TL1C3: _TL1C4:;}}goto _TL1C0;_TL1C2:
# 1774
 Cyc_fclose(in_file);_T26E=Cyc_mode;_T26F=(int)_T26E;
if(_T26F==3)goto _TL1C5;goto _TL1C6;_TL1C5: _TL1C6:
# 1777
 if(hstubs==0)goto _TL1C7;{
struct Cyc_List_List*x=hstubs;_TL1CC: if(x!=0)goto _TL1CA;else{goto _TL1CB;}
_TL1CA: _T270=x;_T271=_T270->hd;{struct _tuple15*_T31A=(struct _tuple15*)_T271;struct _fat_ptr _T31B;struct _fat_ptr _T31C;{struct _tuple15 _T31D=*_T31A;_T31C=_T31D.f0;_T31B=_T31D.f1;}{struct _fat_ptr symbol=_T31C;struct _fat_ptr text=_T31B;_T272=text;_T273=_T272.curr;_T274=(char*)_T273;
if(_T274==0)goto _TL1CD;_T275=symbol;_T276=_T275.curr;_T277=(char*)_T276;
if(_T277!=0)goto _TL1CF;_T278=text;_T279=_untag_fat_ptr_check_bound(_T278,sizeof(char),1U);_T27A=(const char*)_T279;_T27B=out_file;
# 1783
Cyc_fputs(_T27A,_T27B);goto _TL1D0;
_TL1CF: _T27D=Cyc_Set_member;{int(*_T31D)(struct Cyc_Set_Set*,struct _fat_ptr*)=(int(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T27D;_T27C=_T31D;}_T27E=defined_symbols;{struct _fat_ptr*_T31D=_cycalloc(sizeof(struct _fat_ptr));*_T31D=symbol;_T27F=(struct _fat_ptr*)_T31D;}_T280=_T27C(_T27E,_T27F);if(!_T280)goto _TL1D1;_T281=text;_T282=_untag_fat_ptr_check_bound(_T281,sizeof(char),1U);_T283=(const char*)_T282;_T284=out_file;
Cyc_fputs(_T283,_T284);goto _TL1D2;
# 1787
_TL1D1:{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;_T31D.f1=symbol;_T285=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T285;void*_T31E[1];_T286=_T31E + 0;*_T286=& _T31D;_T287=_tag_fat("%s is not supported on this platform\n",sizeof(char),38U);_T288=_tag_fat(_T31E,sizeof(void*),1);Cyc_log(_T287,_T288);}{
struct Cyc_Set_Set*x=defined_symbols;_TL1D6: _T289=Cyc_Set_is_empty(x);if(_T289)goto _TL1D5;else{goto _TL1D4;}
_TL1D4: _T28B=Cyc_Set_choose;{struct _fat_ptr*(*_T31D)(struct Cyc_Set_Set*)=(struct _fat_ptr*(*)(struct Cyc_Set_Set*))_T28B;_T28A=_T31D;}{struct _fat_ptr*y=_T28A(x);_T28D=Cyc_Set_delete;{struct Cyc_Set_Set*(*_T31D)(struct Cyc_Set_Set*,struct _fat_ptr*)=(struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T28D;_T28C=_T31D;}x=_T28C(x,y);{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;_T28F=y;
_T31D.f1=*_T28F;_T28E=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T28E;void*_T31E[1];_T290=_T31E + 0;*_T290=& _T31D;_T291=_tag_fat("+%s",sizeof(char),4U);_T292=_tag_fat(_T31E,sizeof(void*),1);Cyc_log(_T291,_T292);}_T293=
_tag_fat("\n",sizeof(char),2U);_T294=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T293,_T294);}goto _TL1D6;_TL1D5:;}_TL1D2: _TL1D0: goto _TL1CE;
# 1796
_TL1CD:{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;
_T31D.f1=symbol;_T295=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T295;void*_T31E[1];_T296=_T31E + 0;*_T296=& _T31D;_T297=
# 1796
_tag_fat("Null text for %s, will not be supported on this platform\n",sizeof(char),58U);_T298=_tag_fat(_T31E,sizeof(void*),1);Cyc_log(_T297,_T298);}_TL1CE:;}}_T299=x;
# 1778
x=_T299->tl;goto _TL1CC;_TL1CB:;}goto _TL1C8;_TL1C7: _TL1C8: _T29A=out_file;_T29B=
# 1800
_tag_fat("#endif\n",sizeof(char),8U);_T29C=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T29A,_T29B,_T29C);_T29D=Cyc_do_setjmp;
if(_T29D)goto _TL1D7;else{goto _TL1D9;}
_TL1D9: _T29E=out_file;_T29F=_tag_fat("}\n",sizeof(char),3U);_T2A0=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T29E,_T29F,_T2A0);goto _TL1D8;_TL1D7: _TL1D8: _T2A1=Cyc_do_setjmp;
if(!_T2A1)goto _TL1DA;{int _T31A=0;_npop_handler(0);return _T31A;}
_TL1DA: Cyc_fclose(out_file);
# 1807
if(cstubs==0)goto _TL1DC;
out_file=_check_null(Cyc_cstubs_file);{
struct Cyc_List_List*x=cstubs;_TL1E1: if(x!=0)goto _TL1DF;else{goto _TL1E0;}
_TL1DF: _T2A2=x;_T2A3=_T2A2->hd;{struct _tuple15*_T31A=(struct _tuple15*)_T2A3;struct _fat_ptr _T31B;struct _fat_ptr _T31C;{struct _tuple15 _T31D=*_T31A;_T31C=_T31D.f0;_T31B=_T31D.f1;}{struct _fat_ptr symbol=_T31C;struct _fat_ptr text=_T31B;_T2A4=text;_T2A5=_T2A4.curr;_T2A6=(char*)_T2A5;
if(_T2A6==0)goto _TL1E2;_T2A7=symbol;_T2A8=_T2A7.curr;_T2A9=(char*)_T2A8;if(_T2A9==0)goto _TL1E4;else{goto _TL1E5;}_TL1E5: _T2AB=Cyc_Set_member;{
int(*_T31D)(struct Cyc_Set_Set*,struct _fat_ptr*)=(int(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T2AB;_T2AA=_T31D;}_T2AC=defined_symbols;{struct _fat_ptr*_T31D=_cycalloc(sizeof(struct _fat_ptr));*_T31D=symbol;_T2AD=(struct _fat_ptr*)_T31D;}_T2AE=_T2AA(_T2AC,_T2AD);
# 1811
if(_T2AE)goto _TL1E4;else{goto _TL1E2;}
# 1813
_TL1E4: _T2AF=text;_T2B0=_untag_fat_ptr_check_bound(_T2AF,sizeof(char),1U);_T2B1=(const char*)_T2B0;_T2B2=out_file;Cyc_fputs(_T2B1,_T2B2);goto _TL1E3;_TL1E2: _TL1E3:;}}_T2B3=x;
# 1809
x=_T2B3->tl;goto _TL1E1;_TL1E0:;}goto _TL1DD;_TL1DC: _TL1DD:
# 1818
 out_file=_check_null(Cyc_cycstubs_file);
if(cycstubs==0)goto _TL1E6;{struct Cyc_String_pa_PrintArg_struct _T31A;_T31A.tag=0;{const char*_T31B=filename;_T2B6=(void*)_T31B;_T2B7=(void*)_T31B;_T2B8=_get_zero_arr_size_char(_T2B7,1U);_T2B5=_tag_fat(_T2B6,sizeof(char),_T2B8);}
# 1823
_T31A.f1=_T2B5;_T2B4=_T31A;}{struct Cyc_String_pa_PrintArg_struct _T31A=_T2B4;void*_T31B[1];_T2B9=_T31B + 0;*_T2B9=& _T31A;_T2BA=out_file;_T2BB=_tag_fat("#include <%s>\n\n",sizeof(char),16U);_T2BC=_tag_fat(_T31B,sizeof(void*),1);Cyc_fprintf(_T2BA,_T2BB,_T2BC);}
out_file=_check_null(Cyc_cycstubs_file);{
struct Cyc_List_List*x=cycstubs;_TL1EB: if(x!=0)goto _TL1E9;else{goto _TL1EA;}
_TL1E9: _T2BD=x;_T2BE=_T2BD->hd;{struct _tuple15*_T31A=(struct _tuple15*)_T2BE;struct _fat_ptr _T31B;struct _fat_ptr _T31C;{struct _tuple15 _T31D=*_T31A;_T31C=_T31D.f0;_T31B=_T31D.f1;}{struct _fat_ptr symbol=_T31C;struct _fat_ptr text=_T31B;_T2BF=text;_T2C0=_T2BF.curr;_T2C1=(char*)_T2C0;
if(_T2C1==0)goto _TL1EC;_T2C2=symbol;_T2C3=_T2C2.curr;_T2C4=(char*)_T2C3;if(_T2C4==0)goto _TL1EE;else{goto _TL1EF;}_TL1EF: _T2C6=Cyc_Set_member;{
int(*_T31D)(struct Cyc_Set_Set*,struct _fat_ptr*)=(int(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T2C6;_T2C5=_T31D;}_T2C7=defined_symbols;{struct _fat_ptr*_T31D=_cycalloc(sizeof(struct _fat_ptr));*_T31D=symbol;_T2C8=(struct _fat_ptr*)_T31D;}_T2C9=_T2C5(_T2C7,_T2C8);
# 1827
if(_T2C9)goto _TL1EE;else{goto _TL1EC;}
# 1829
_TL1EE: _T2CA=text;_T2CB=_untag_fat_ptr_check_bound(_T2CA,sizeof(char),1U);_T2CC=(const char*)_T2CB;_T2CD=out_file;Cyc_fputs(_T2CC,_T2CD);goto _TL1ED;_TL1EC: _TL1ED:;}}_T2CE=x;
# 1825
x=_T2CE->tl;goto _TL1EB;_TL1EA:;}_T2CF=out_file;_T2D0=
# 1831
_tag_fat("\n",sizeof(char),2U);_T2D1=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T2CF,_T2D0,_T2D1);goto _TL1E7;_TL1E6: _TL1E7: {int _T31A=0;_npop_handler(0);return _T31A;}}}}}}}}}}}_pop_handler();goto _TL120;_TL11F: _T2D2=Cyc_Core_get_exn_thrown();{void*_T31A=(void*)_T2D2;void*_T31B;struct _fat_ptr _T31C;_T2D3=(struct Cyc_Core_Impossible_exn_struct*)_T31A;_T2D4=_T2D3->tag;_T2D5=Cyc_Core_Impossible;if(_T2D4!=_T2D5)goto _TL1F0;{struct Cyc_Core_Impossible_exn_struct*_T31D=(struct Cyc_Core_Impossible_exn_struct*)_T31A;_T31C=_T31D->f1;}{struct _fat_ptr s=_T31C;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;
# 1838
_T31D.f1=s;_T2D6=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T2D6;void*_T31E[1];_T2D7=_T31E + 0;*_T2D7=& _T31D;_T2D8=_tag_fat("Got Core::Impossible(%s)\n",sizeof(char),26U);_T2D9=_tag_fat(_T31E,sizeof(void*),1);Cyc_log(_T2D8,_T2D9);}goto _LL89;}_TL1F0: _T2DA=(struct Cyc_Dict_Absent_exn_struct*)_T31A;_T2DB=_T2DA->tag;_T2DC=Cyc_Dict_Absent;if(_T2DB!=_T2DC)goto _TL1F2;_T2DD=
# 1840
_tag_fat("Got Dict::Absent\n",sizeof(char),18U);_T2DE=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T2DD,_T2DE);goto _LL89;_TL1F2: _T2DF=(struct Cyc_Core_Failure_exn_struct*)_T31A;_T2E0=_T2DF->tag;_T2E1=Cyc_Core_Failure;if(_T2E0!=_T2E1)goto _TL1F4;{struct Cyc_Core_Failure_exn_struct*_T31D=(struct Cyc_Core_Failure_exn_struct*)_T31A;_T31C=_T31D->f1;}{struct _fat_ptr s=_T31C;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;
# 1842
_T31D.f1=s;_T2E2=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T2E2;void*_T31E[1];_T2E3=_T31E + 0;*_T2E3=& _T31D;_T2E4=_tag_fat("Got Core::Failure(%s)\n",sizeof(char),23U);_T2E5=_tag_fat(_T31E,sizeof(void*),1);Cyc_log(_T2E4,_T2E5);}goto _LL89;}_TL1F4: _T2E6=(struct Cyc_Core_Invalid_argument_exn_struct*)_T31A;_T2E7=_T2E6->tag;_T2E8=Cyc_Core_Invalid_argument;if(_T2E7!=_T2E8)goto _TL1F6;{struct Cyc_Core_Invalid_argument_exn_struct*_T31D=(struct Cyc_Core_Invalid_argument_exn_struct*)_T31A;_T31C=_T31D->f1;}{struct _fat_ptr s=_T31C;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;
# 1844
_T31D.f1=s;_T2E9=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T2E9;void*_T31E[1];_T2EA=_T31E + 0;*_T2EA=& _T31D;_T2EB=_tag_fat("Got Invalid_argument(%s)\n",sizeof(char),26U);_T2EC=_tag_fat(_T31E,sizeof(void*),1);Cyc_log(_T2EB,_T2EC);}goto _LL89;}_TL1F6: _T2ED=(struct Cyc_Core_Not_found_exn_struct*)_T31A;_T2EE=_T2ED->tag;_T2EF=Cyc_Core_Not_found;if(_T2EE!=_T2EF)goto _TL1F8;_T2F0=
# 1846
_tag_fat("Got Not_found\n",sizeof(char),15U);_T2F1=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T2F0,_T2F1);goto _LL89;_TL1F8: _T2F2=(struct Cyc_NO_SUPPORT_exn_struct*)_T31A;_T2F3=_T2F2->tag;_T2F4=Cyc_NO_SUPPORT;if(_T2F3!=_T2F4)goto _TL1FA;{struct Cyc_NO_SUPPORT_exn_struct*_T31D=(struct Cyc_NO_SUPPORT_exn_struct*)_T31A;_T31C=_T31D->f1;}{struct _fat_ptr s=_T31C;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;
# 1848
_T31D.f1=s;_T2F5=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T2F5;void*_T31E[1];_T2F6=_T31E + 0;*_T2F6=& _T31D;_T2F7=_tag_fat("No support because %s\n",sizeof(char),23U);_T2F8=_tag_fat(_T31E,sizeof(void*),1);Cyc_log(_T2F7,_T2F8);}goto _LL89;}_TL1FA: _T2F9=(struct Cyc_Lexing_Error_exn_struct*)_T31A;_T2FA=_T2F9->tag;_T2FB=Cyc_Lexing_Error;if(_T2FA!=_T2FB)goto _TL1FC;{struct Cyc_Lexing_Error_exn_struct*_T31D=(struct Cyc_Lexing_Error_exn_struct*)_T31A;_T31C=_T31D->f1;}{struct _fat_ptr s=_T31C;{struct Cyc_String_pa_PrintArg_struct _T31D;_T31D.tag=0;
# 1850
_T31D.f1=s;_T2FC=_T31D;}{struct Cyc_String_pa_PrintArg_struct _T31D=_T2FC;void*_T31E[1];_T2FD=_T31E + 0;*_T2FD=& _T31D;_T2FE=_tag_fat("Got a lexing error %s\n",sizeof(char),23U);_T2FF=_tag_fat(_T31E,sizeof(void*),1);Cyc_log(_T2FE,_T2FF);}goto _LL89;}_TL1FC: _T31B=_T31A;{void*x=_T31B;_T300=
# 1852
_tag_fat("Got unknown exception\n",sizeof(char),23U);_T301=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T300,_T301);
Cyc_Core_rethrow(x);}_LL89:;}_TL120:;}}
# 1858
maybe=Cyc_fopen(filename,"w");_T302=maybe;_T303=(unsigned)_T302;
if(_T303)goto _TL1FE;else{goto _TL200;}
_TL200:{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;{const char*_T319=filename;_T306=(void*)_T319;_T307=(void*)_T319;_T308=_get_zero_arr_size_char(_T307,1U);_T305=_tag_fat(_T306,sizeof(char),_T308);}_T318.f1=_T305;_T304=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_T304;void*_T319[1];_T309=_T319 + 0;*_T309=& _T318;_T30A=Cyc_stderr;_T30B=_tag_fat("Error: could not create file %s\n",sizeof(char),33U);_T30C=_tag_fat(_T319,sizeof(void*),1);Cyc_fprintf(_T30A,_T30B,_T30C);}
return 1;_TL1FE:
# 1863
 out_file=maybe;{struct Cyc_String_pa_PrintArg_struct _T318;_T318.tag=0;{const char*_T319=filename;_T30F=(void*)_T319;_T310=(void*)_T319;_T311=_get_zero_arr_size_char(_T310,1U);_T30E=_tag_fat(_T30F,sizeof(char),_T311);}
# 1866
_T318.f1=_T30E;_T30D=_T318;}{struct Cyc_String_pa_PrintArg_struct _T318=_T30D;void*_T319[1];_T312=_T319 + 0;*_T312=& _T318;_T313=out_file;_T314=
# 1865
_tag_fat("#error -- %s is not supported on this platform\n",sizeof(char),48U);_T315=_tag_fat(_T319,sizeof(void*),1);Cyc_fprintf(_T313,_T314,_T315);}
# 1867
Cyc_fclose(out_file);_T316=
# 1870
_tag_fat("Not supported on this platform\n",sizeof(char),32U);_T317=_tag_fat(0U,sizeof(void*),0);Cyc_log(_T316,_T317);
# 1877
return 0;}}}}}}}
# 1881
int Cyc_process_specfile(const char*file,const char*dir){int _T0;struct Cyc_String_pa_PrintArg_struct _T1;struct _fat_ptr _T2;void*_T3;void*_T4;unsigned _T5;void**_T6;struct Cyc___cycFILE*_T7;struct _fat_ptr _T8;struct _fat_ptr _T9;struct Cyc___cycFILE*_TA;unsigned _TB;struct Cyc_String_pa_PrintArg_struct _TC;struct _fat_ptr _TD;void*_TE;void*_TF;unsigned _T10;void**_T11;struct Cyc___cycFILE*_T12;struct _fat_ptr _T13;struct _fat_ptr _T14;char*_T15;char*_T16;unsigned _T17;char*_T18;unsigned _T19;char*_T1A;struct _fat_ptr _T1B;struct _fat_ptr _T1C;unsigned _T1D;enum Cyc_buildlib_mode _T1E;int _T1F;int _T20;struct Cyc_String_pa_PrintArg_struct _T21;struct _fat_ptr _T22;void*_T23;void*_T24;unsigned _T25;void**_T26;struct Cyc___cycFILE*_T27;struct _fat_ptr _T28;struct _fat_ptr _T29;enum Cyc_buildlib_mode _T2A;int _T2B;struct _fat_ptr _T2C;struct Cyc_String_pa_PrintArg_struct _T2D;struct Cyc_String_pa_PrintArg_struct _T2E;void**_T2F;void**_T30;struct _fat_ptr _T31;struct _fat_ptr _T32;int _T33;struct Cyc_String_pa_PrintArg_struct _T34;void**_T35;struct Cyc___cycFILE*_T36;struct _fat_ptr _T37;struct _fat_ptr _T38;struct _fat_ptr _T39;char*_T3A;char*_T3B;const char*_T3C;struct _tuple17*_T3D;struct _fat_ptr _T3E;char*_T3F;char*_T40;const char*_T41;struct Cyc_List_List*_T42;struct Cyc_List_List*_T43;struct Cyc_List_List*_T44;struct Cyc_List_List*_T45;struct Cyc_List_List*_T46;struct Cyc_List_List*_T47;struct Cyc_List_List*_T48;int _T49;enum Cyc_buildlib_mode _T4A;int _T4B;struct _fat_ptr _T4C;char*_T4D;char*_T4E;char*_T4F;int _T50;struct Cyc_String_pa_PrintArg_struct _T51;void**_T52;struct Cyc___cycFILE*_T53;struct _fat_ptr _T54;struct _fat_ptr _T55;_T0=Cyc_verbose;
if(!_T0)goto _TL201;{struct Cyc_String_pa_PrintArg_struct _T56;_T56.tag=0;{const char*_T57=file;_T3=(void*)_T57;_T4=(void*)_T57;_T5=_get_zero_arr_size_char(_T4,1U);_T2=_tag_fat(_T3,sizeof(char),_T5);}
_T56.f1=_T2;_T1=_T56;}{struct Cyc_String_pa_PrintArg_struct _T56=_T1;void*_T57[1];_T6=_T57 + 0;*_T6=& _T56;_T7=Cyc_stderr;_T8=_tag_fat("Processing %s\n",sizeof(char),15U);_T9=_tag_fat(_T57,sizeof(void*),1);Cyc_fprintf(_T7,_T8,_T9);}goto _TL202;_TL201: _TL202: {
struct Cyc___cycFILE*maybe=Cyc_fopen(file,"r");_TA=maybe;_TB=(unsigned)_TA;
if(_TB)goto _TL203;else{goto _TL205;}
_TL205:{struct Cyc_String_pa_PrintArg_struct _T56;_T56.tag=0;{const char*_T57=file;_TE=(void*)_T57;_TF=(void*)_T57;_T10=_get_zero_arr_size_char(_TF,1U);_TD=_tag_fat(_TE,sizeof(char),_T10);}_T56.f1=_TD;_TC=_T56;}{struct Cyc_String_pa_PrintArg_struct _T56=_TC;void*_T57[1];_T11=_T57 + 0;*_T11=& _T56;_T12=Cyc_stderr;_T13=_tag_fat("Error: could not open %s\n",sizeof(char),26U);_T14=_tag_fat(_T57,sizeof(void*),1);Cyc_fprintf(_T12,_T13,_T14);}
return 1;_TL203: {
# 1889
struct Cyc___cycFILE*in_file=maybe;{unsigned _T56=1024U + 1U;_T17=_check_times(_T56,sizeof(char));{char*_T57=_cycalloc_atomic(_T17);{unsigned _T58=1024U;unsigned i;i=0;_TL209: if(i < _T58)goto _TL207;else{goto _TL208;}_TL207: _T19=i;_T18=_T57 + _T19;
# 1893
*_T18='\000';i=i + 1;goto _TL209;_TL208: _T1A=_T57 + _T58;*_T1A=0;}_T16=(char*)_T57;}_T15=_T16;}{struct _fat_ptr buf=_tag_fat(_T15,sizeof(char),1025U);_T1B=buf;_T1C=buf;_T1D=
_get_fat_size(_T1C,sizeof(char));{struct _fat_ptr cwd=Cyc_getcwd(_T1B,_T1D);_T1E=Cyc_mode;_T1F=(int)_T1E;
if(_T1F==2)goto _TL20A;_T20=
chdir(dir);if(!_T20)goto _TL20C;{struct Cyc_String_pa_PrintArg_struct _T56;_T56.tag=0;{const char*_T57=dir;_T23=(void*)_T57;_T24=(void*)_T57;_T25=_get_zero_arr_size_char(_T24,1U);_T22=_tag_fat(_T23,sizeof(char),_T25);}
_T56.f1=_T22;_T21=_T56;}{struct Cyc_String_pa_PrintArg_struct _T56=_T21;void*_T57[1];_T26=_T57 + 0;*_T26=& _T56;_T27=Cyc_stderr;_T28=_tag_fat("Error: can't change directory to %s\n",sizeof(char),37U);_T29=_tag_fat(_T57,sizeof(void*),1);Cyc_fprintf(_T27,_T28,_T29);}
return 1;_TL20C: goto _TL20B;_TL20A: _TL20B: _T2A=Cyc_mode;_T2B=(int)_T2A;
# 1901
if(_T2B!=1)goto _TL20E;{struct Cyc_String_pa_PrintArg_struct _T56;_T56.tag=0;
# 1904
_T56.f1=Cyc_cyclone_cc;_T2D=_T56;}{struct Cyc_String_pa_PrintArg_struct _T56=_T2D;{struct Cyc_String_pa_PrintArg_struct _T57;_T57.tag=0;_T57.f1=Cyc_target_cflags;_T2E=_T57;}{struct Cyc_String_pa_PrintArg_struct _T57=_T2E;void*_T58[2];_T2F=_T58 + 0;*_T2F=& _T56;_T30=_T58 + 1;*_T30=& _T57;_T31=
# 1903
_tag_fat("echo | %s %s -E -dM - -o INITMACROS.h\n",sizeof(char),39U);_T32=_tag_fat(_T58,sizeof(void*),2);_T2C=Cyc_aprintf(_T31,_T32);}}{struct _fat_ptr cmd=_T2C;_T33=Cyc_verbose;
# 1905
if(!_T33)goto _TL210;{struct Cyc_String_pa_PrintArg_struct _T56;_T56.tag=0;
_T56.f1=cmd;_T34=_T56;}{struct Cyc_String_pa_PrintArg_struct _T56=_T34;void*_T57[1];_T35=_T57 + 0;*_T35=& _T56;_T36=Cyc_stderr;_T37=_tag_fat("%s\n",sizeof(char),4U);_T38=_tag_fat(_T57,sizeof(void*),1);Cyc_fprintf(_T36,_T37,_T38);}goto _TL211;_TL210: _TL211: _T39=cmd;_T3A=_untag_fat_ptr(_T39,sizeof(char),1U);_T3B=_check_null(_T3A);_T3C=(const char*)_T3B;
system(_T3C);}goto _TL20F;_TL20E: _TL20F: {
# 1910
struct Cyc_Lexing_lexbuf*l=Cyc_Lexing_from_file(in_file);
struct _tuple17*entry;
_TL212: entry=Cyc_spec(l);_T3D=entry;if(_T3D!=0)goto _TL213;else{goto _TL214;}
_TL213:{struct _tuple17*_T56=entry;struct Cyc_List_List*_T57;struct Cyc_List_List*_T58;struct Cyc_List_List*_T59;struct Cyc_List_List*_T5A;struct Cyc_List_List*_T5B;struct Cyc_List_List*_T5C;struct Cyc_List_List*_T5D;struct _fat_ptr _T5E;{struct _tuple17 _T5F=*_T56;_T5E=_T5F.f0;_T5D=_T5F.f1;_T5C=_T5F.f2;_T5B=_T5F.f3;_T5A=_T5F.f4;_T59=_T5F.f5;_T58=_T5F.f6;_T57=_T5F.f7;}{struct _fat_ptr headerfile=_T5E;struct Cyc_List_List*start_symbols=_T5D;struct Cyc_List_List*user_defs=_T5C;struct Cyc_List_List*omit_symbols=_T5B;struct Cyc_List_List*hstubs=_T5A;struct Cyc_List_List*cstubs=_T59;struct Cyc_List_List*cycstubs=_T58;struct Cyc_List_List*cpp_insert=_T57;_T3E=headerfile;_T3F=_untag_fat_ptr(_T3E,sizeof(char),1U);_T40=_check_null(_T3F);_T41=(const char*)_T40;_T42=start_symbols;_T43=user_defs;_T44=omit_symbols;_T45=hstubs;_T46=cstubs;_T47=cycstubs;_T48=cpp_insert;_T49=
# 1915
Cyc_process_file(_T41,_T42,_T43,_T44,_T45,_T46,_T47,_T48);if(!_T49)goto _TL215;
# 1917
return 1;_TL215:;}}goto _TL212;_TL214:
# 1919
 Cyc_fclose(in_file);_T4A=Cyc_mode;_T4B=(int)_T4A;
# 1921
if(_T4B==2)goto _TL217;_T4C=cwd;_T4D=_untag_fat_ptr(_T4C,sizeof(char),1U);_T4E=_check_null(_T4D);_T4F=(char*)_T4E;_T50=
chdir(_T4F);if(!_T50)goto _TL219;{struct Cyc_String_pa_PrintArg_struct _T56;_T56.tag=0;
_T56.f1=cwd;_T51=_T56;}{struct Cyc_String_pa_PrintArg_struct _T56=_T51;void*_T57[1];_T52=_T57 + 0;*_T52=& _T56;_T53=Cyc_stderr;_T54=_tag_fat("Error: could not change directory to %s\n",sizeof(char),41U);_T55=_tag_fat(_T57,sizeof(void*),1);Cyc_fprintf(_T53,_T54,_T55);}
return 1;_TL219: goto _TL218;_TL217: _TL218:
# 1927
 return 0;}}}}}}
# 1931
int Cyc_process_setjmp(const char*dir){char*_T0;char*_T1;unsigned _T2;char*_T3;unsigned _T4;char*_T5;struct _fat_ptr _T6;struct _fat_ptr _T7;unsigned _T8;int _T9;struct Cyc_String_pa_PrintArg_struct _TA;struct _fat_ptr _TB;void*_TC;void*_TD;unsigned _TE;void**_TF;struct Cyc___cycFILE*_T10;struct _fat_ptr _T11;struct _fat_ptr _T12;struct Cyc_List_List*_T13;struct _fat_ptr**_T14;struct _fat_ptr*_T15;struct _fat_ptr _T16;struct Cyc_List_List*_T17;struct _tuple15**_T18;struct _tuple15*_T19;struct _fat_ptr _T1A;int _T1B;struct _fat_ptr _T1C;char*_T1D;char*_T1E;char*_T1F;int _T20;struct Cyc_String_pa_PrintArg_struct _T21;void**_T22;struct Cyc___cycFILE*_T23;struct _fat_ptr _T24;struct _fat_ptr _T25;{unsigned _T26=1024U + 1U;_T2=_check_times(_T26,sizeof(char));{char*_T27=_cycalloc_atomic(_T2);{unsigned _T28=1024U;unsigned i;i=0;_TL21E: if(i < _T28)goto _TL21C;else{goto _TL21D;}_TL21C: _T4=i;_T3=_T27 + _T4;
# 1934
*_T3='\000';i=i + 1;goto _TL21E;_TL21D: _T5=_T27 + _T28;*_T5=0;}_T1=(char*)_T27;}_T0=_T1;}{struct _fat_ptr buf=_tag_fat(_T0,sizeof(char),1025U);_T6=buf;_T7=buf;_T8=
_get_fat_size(_T7,sizeof(char));{struct _fat_ptr cwd=Cyc_getcwd(_T6,_T8);_T9=
chdir(dir);if(!_T9)goto _TL21F;{struct Cyc_String_pa_PrintArg_struct _T26;_T26.tag=0;{const char*_T27=dir;_TC=(void*)_T27;_TD=(void*)_T27;_TE=_get_zero_arr_size_char(_TD,1U);_TB=_tag_fat(_TC,sizeof(char),_TE);}
_T26.f1=_TB;_TA=_T26;}{struct Cyc_String_pa_PrintArg_struct _T26=_TA;void*_T27[1];_TF=_T27 + 0;*_TF=& _T26;_T10=Cyc_stderr;_T11=_tag_fat("Error: can't change directory to %s\n",sizeof(char),37U);_T12=_tag_fat(_T27,sizeof(void*),1);Cyc_fprintf(_T10,_T11,_T12);}
return 1;_TL21F:{struct _fat_ptr*_T26[1];_T14=_T26 + 0;{struct _fat_ptr*_T27=_cycalloc(sizeof(struct _fat_ptr));
# 1940
*_T27=_tag_fat("jmp_buf",sizeof(char),8U);_T15=(struct _fat_ptr*)_T27;}*_T14=_T15;_T16=_tag_fat(_T26,sizeof(struct _fat_ptr*),1);_T13=Cyc_List_list(_T16);}{struct _tuple15*_T26[1];_T18=_T26 + 0;{struct _tuple15*_T27=_cycalloc(sizeof(struct _tuple15));
_T27->f0=_tag_fat("setjmp",sizeof(char),7U);_T27->f1=_tag_fat("extern int setjmp(jmp_buf);\n",sizeof(char),29U);_T19=(struct _tuple15*)_T27;}*_T18=_T19;_T1A=_tag_fat(_T26,sizeof(struct _tuple15*),1);_T17=Cyc_List_list(_T1A);}_T1B=
# 1940
Cyc_process_file("setjmp.h",_T13,0,0,_T17,0,0,0);if(!_T1B)goto _TL221;
# 1943
return 1;_TL221: _T1C=cwd;_T1D=_untag_fat_ptr(_T1C,sizeof(char),1U);_T1E=_check_null(_T1D);_T1F=(char*)_T1E;_T20=
chdir(_T1F);if(!_T20)goto _TL223;{struct Cyc_String_pa_PrintArg_struct _T26;_T26.tag=0;
_T26.f1=cwd;_T21=_T26;}{struct Cyc_String_pa_PrintArg_struct _T26=_T21;void*_T27[1];_T22=_T27 + 0;*_T22=& _T26;_T23=Cyc_stderr;_T24=_tag_fat("Error: could not change directory to %s\n",sizeof(char),41U);_T25=_tag_fat(_T27,sizeof(void*),1);Cyc_fprintf(_T23,_T24,_T25);}
return 1;_TL223:
# 1948
 return 0;}}}static char _TmpG3[13U]="BUILDLIB.OUT";
# 1952
static struct _fat_ptr Cyc_output_dir={_TmpG3,_TmpG3,_TmpG3 + 13U};
static void Cyc_set_output_dir(struct _fat_ptr s){
Cyc_output_dir=s;}
# 1956
static struct Cyc_List_List*Cyc_spec_files=0;
static void Cyc_add_spec_file(struct _fat_ptr s){struct Cyc_List_List*_T0;struct _fat_ptr _T1;char*_T2;char*_T3;{struct Cyc_List_List*_T4=_cycalloc(sizeof(struct Cyc_List_List));_T1=s;_T2=_untag_fat_ptr_check_bound(_T1,sizeof(char),1U);_T3=_check_null(_T2);
_T4->hd=(const char*)_T3;_T4->tl=Cyc_spec_files;_T0=(struct Cyc_List_List*)_T4;}Cyc_spec_files=_T0;}
# 1960
static int Cyc_no_other(struct _fat_ptr s){return 0;}
static void Cyc_set_GATHER (void){
Cyc_mode=1U;}
# 1964
static void Cyc_set_GATHERSCRIPT (void){
Cyc_mode=2U;}
# 1967
static void Cyc_set_FINISH (void){
Cyc_mode=3U;}
# 1970
static void Cyc_add_cpparg(struct _fat_ptr s){struct Cyc_List_List*_T0;struct _fat_ptr*_T1;{struct Cyc_List_List*_T2=_cycalloc(sizeof(struct Cyc_List_List));{struct _fat_ptr*_T3=_cycalloc(sizeof(struct _fat_ptr));
*_T3=s;_T1=(struct _fat_ptr*)_T3;}_T2->hd=_T1;_T2->tl=Cyc_cppargs;_T0=(struct Cyc_List_List*)_T2;}Cyc_cppargs=_T0;}
# 1973
static int Cyc_badparse=0;
static void Cyc_unsupported_option(struct _fat_ptr s){struct Cyc_String_pa_PrintArg_struct _T0;void**_T1;struct Cyc___cycFILE*_T2;struct _fat_ptr _T3;struct _fat_ptr _T4;{struct Cyc_String_pa_PrintArg_struct _T5;_T5.tag=0;
_T5.f1=s;_T0=_T5;}{struct Cyc_String_pa_PrintArg_struct _T5=_T0;void*_T6[1];_T1=_T6 + 0;*_T1=& _T5;_T2=Cyc_stderr;_T3=_tag_fat("Unsupported option %s\n",sizeof(char),23U);_T4=_tag_fat(_T6,sizeof(void*),1);Cyc_fprintf(_T2,_T3,_T4);}
Cyc_badparse=1;}
# 1978
static void Cyc_set_header(struct _fat_ptr s){struct Cyc_Set_Set**_T0;unsigned _T1;struct Cyc_Set_Set**_T2;struct Cyc_Set_Set*(*_T3)(int(*)(struct _fat_ptr*,struct _fat_ptr*));struct Cyc_Set_Set*(*_T4)(int(*)(void*,void*));int(*_T5)(struct _fat_ptr*,struct _fat_ptr*);struct Cyc_Set_Set**_T6;struct Cyc_Set_Set*(*_T7)(struct Cyc_Set_Set*,struct _fat_ptr*);struct Cyc_Set_Set*(*_T8)(struct Cyc_Set_Set*,void*);struct Cyc_Set_Set**_T9;struct Cyc_Set_Set*_TA;struct _fat_ptr*_TB;_T0=Cyc_headers_to_do;_T1=(unsigned)_T0;
if(_T1)goto _TL225;else{goto _TL227;}
_TL227:{struct Cyc_Set_Set**_TC=_cycalloc(sizeof(struct Cyc_Set_Set*));_T4=Cyc_Set_empty;{struct Cyc_Set_Set*(*_TD)(int(*)(struct _fat_ptr*,struct _fat_ptr*))=(struct Cyc_Set_Set*(*)(int(*)(struct _fat_ptr*,struct _fat_ptr*)))_T4;_T3=_TD;}_T5=Cyc_strptrcmp;*_TC=_T3(_T5);_T2=(struct Cyc_Set_Set**)_TC;}Cyc_headers_to_do=_T2;goto _TL226;_TL225: _TL226: _T6=
_check_null(Cyc_headers_to_do);_T8=Cyc_Set_insert;{struct Cyc_Set_Set*(*_TC)(struct Cyc_Set_Set*,struct _fat_ptr*)=(struct Cyc_Set_Set*(*)(struct Cyc_Set_Set*,struct _fat_ptr*))_T8;_T7=_TC;}_T9=Cyc_headers_to_do;_TA=*_T9;{struct _fat_ptr*_TC=_cycalloc(sizeof(struct _fat_ptr));*_TC=s;_TB=(struct _fat_ptr*)_TC;}*_T6=_T7(_TA,_TB);}
# 1987
extern void GC_blacklist_warn_clear (void);struct _tuple19{struct _fat_ptr f0;int f1;struct _fat_ptr f2;void*f3;struct _fat_ptr f4;};
int Cyc_main(int argc,struct _fat_ptr argv){struct Cyc_List_List*_T0;struct _tuple19**_T1;struct _tuple19*_T2;struct Cyc_Arg_String_spec_Arg_Spec_struct*_T3;struct _tuple19**_T4;struct _tuple19*_T5;struct Cyc_Arg_Unit_spec_Arg_Spec_struct*_T6;struct _tuple19**_T7;struct _tuple19*_T8;struct Cyc_Arg_Unit_spec_Arg_Spec_struct*_T9;struct _tuple19**_TA;struct _tuple19*_TB;struct Cyc_Arg_Unit_spec_Arg_Spec_struct*_TC;struct _tuple19**_TD;struct _tuple19*_TE;struct Cyc_Arg_Set_spec_Arg_Spec_struct*_TF;struct _tuple19**_T10;struct _tuple19*_T11;struct Cyc_Arg_String_spec_Arg_Spec_struct*_T12;struct _tuple19**_T13;struct _tuple19*_T14;struct Cyc_Arg_Flag_spec_Arg_Spec_struct*_T15;struct _tuple19**_T16;struct _tuple19*_T17;struct Cyc_Arg_String_spec_Arg_Spec_struct*_T18;struct _tuple19**_T19;struct _tuple19*_T1A;struct Cyc_Arg_Set_spec_Arg_Spec_struct*_T1B;struct _tuple19**_T1C;struct _tuple19*_T1D;struct Cyc_Arg_Flag_spec_Arg_Spec_struct*_T1E;struct _fat_ptr _T1F;struct Cyc_List_List*_T20;struct _fat_ptr _T21;struct _fat_ptr _T22;struct Cyc_List_List*_T23;struct _fat_ptr _T24;struct _fat_ptr _T25;int _T26;int _T27;int _T28;int _T29;enum Cyc_buildlib_mode _T2A;int _T2B;int _T2C;enum Cyc_buildlib_mode _T2D;int _T2E;struct Cyc_List_List*_T2F;struct _fat_ptr _T30;struct _fat_ptr _T31;int _T32;struct Cyc_String_pa_PrintArg_struct _T33;void**_T34;struct Cyc___cycFILE*_T35;struct _fat_ptr _T36;struct _fat_ptr _T37;struct Cyc_List_List*_T38;struct _fat_ptr _T39;int _T3A;struct Cyc_String_pa_PrintArg_struct _T3B;void**_T3C;struct Cyc___cycFILE*_T3D;struct _fat_ptr _T3E;struct _fat_ptr _T3F;struct Cyc_List_List*_T40;struct _fat_ptr _T41;struct _fat_ptr _T42;char*_T43;unsigned _T44;int _T45;struct Cyc_String_pa_PrintArg_struct _T46;void**_T47;struct Cyc___cycFILE*_T48;struct _fat_ptr _T49;struct _fat_ptr _T4A;enum Cyc_buildlib_mode _T4B;int _T4C;int _T4D;struct Cyc___cycFILE*_T4E;struct _fat_ptr _T4F;struct _fat_ptr _T50;struct Cyc___cycFILE*_T51;unsigned _T52;struct Cyc___cycFILE*_T53;struct _fat_ptr _T54;struct _fat_ptr _T55;struct _fat_ptr _T56;struct _fat_ptr _T57;struct _fat_ptr _T58;struct _fat_ptr _T59;int _T5A;int _T5B;struct Cyc_String_pa_PrintArg_struct _T5C;void**_T5D;struct Cyc___cycFILE*_T5E;struct _fat_ptr _T5F;struct _fat_ptr _T60;int _T61;struct Cyc_String_pa_PrintArg_struct _T62;void**_T63;struct Cyc___cycFILE*_T64;struct _fat_ptr _T65;struct _fat_ptr _T66;enum Cyc_buildlib_mode _T67;int _T68;struct Cyc_String_pa_PrintArg_struct _T69;void**_T6A;struct _fat_ptr _T6B;struct _fat_ptr _T6C;struct Cyc_String_pa_PrintArg_struct _T6D;void**_T6E;struct _fat_ptr _T6F;struct _fat_ptr _T70;int _T71;struct _fat_ptr _T72;struct _fat_ptr _T73;struct _fat_ptr _T74;char*_T75;char*_T76;const char*_T77;struct Cyc___cycFILE*_T78;unsigned _T79;struct Cyc_String_pa_PrintArg_struct _T7A;void**_T7B;struct Cyc___cycFILE*_T7C;struct _fat_ptr _T7D;struct _fat_ptr _T7E;int _T7F;struct _fat_ptr _T80;struct _fat_ptr _T81;struct _fat_ptr _T82;char*_T83;char*_T84;const char*_T85;struct Cyc___cycFILE*_T86;unsigned _T87;struct Cyc_String_pa_PrintArg_struct _T88;void**_T89;struct Cyc___cycFILE*_T8A;struct _fat_ptr _T8B;struct _fat_ptr _T8C;struct _fat_ptr _T8D;struct _fat_ptr _T8E;struct _fat_ptr _T8F;char*_T90;char*_T91;const char*_T92;struct Cyc___cycFILE*_T93;unsigned _T94;struct Cyc_String_pa_PrintArg_struct _T95;void**_T96;struct Cyc___cycFILE*_T97;struct _fat_ptr _T98;struct _fat_ptr _T99;struct Cyc___cycFILE*_T9A;struct _fat_ptr _T9B;struct _fat_ptr _T9C;struct _fat_ptr _T9D;char*_T9E;char*_T9F;int _TA0;int _TA1;struct Cyc_List_List*_TA2;void*_TA3;const char*_TA4;const char*_TA5;int _TA6;struct Cyc___cycFILE*_TA7;struct _fat_ptr _TA8;struct _fat_ptr _TA9;struct Cyc_List_List*_TAA;enum Cyc_buildlib_mode _TAB;int _TAC;struct Cyc___cycFILE*_TAD;int _TAE;struct Cyc___cycFILE*_TAF;int _TB0;struct Cyc___cycFILE*_TB1;struct Cyc___cycFILE*_TB2;
GC_blacklist_warn_clear();{struct _tuple19*_TB3[10];_T1=_TB3 + 0;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 1992
_TB4->f0=_tag_fat("-d",sizeof(char),3U);_TB4->f1=0;_TB4->f2=_tag_fat(" <file>",sizeof(char),8U);{struct Cyc_Arg_String_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_String_spec_Arg_Spec_struct));_TB5->tag=5;
_TB5->f1=Cyc_set_output_dir;_T3=(struct Cyc_Arg_String_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_T3;
_TB4->f4=_tag_fat("Set the output directory to <file>",sizeof(char),35U);_T2=(struct _tuple19*)_TB4;}
# 1992
*_T1=_T2;_T4=_TB3 + 1;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 1995
_TB4->f0=_tag_fat("-gather",sizeof(char),8U);_TB4->f1=0;_TB4->f2=_tag_fat("",sizeof(char),1U);{struct Cyc_Arg_Unit_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_Unit_spec_Arg_Spec_struct));_TB5->tag=0;
_TB5->f1=Cyc_set_GATHER;_T6=(struct Cyc_Arg_Unit_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_T6;
_TB4->f4=_tag_fat("Gather C library info but don't produce Cyclone headers",sizeof(char),56U);_T5=(struct _tuple19*)_TB4;}
# 1995
*_T4=_T5;_T7=_TB3 + 2;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 1998
_TB4->f0=_tag_fat("-gatherscript",sizeof(char),14U);_TB4->f1=0;_TB4->f2=_tag_fat("",sizeof(char),1U);{struct Cyc_Arg_Unit_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_Unit_spec_Arg_Spec_struct));_TB5->tag=0;
_TB5->f1=Cyc_set_GATHERSCRIPT;_T9=(struct Cyc_Arg_Unit_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_T9;
_TB4->f4=_tag_fat("Produce a script to gather C library info",sizeof(char),42U);_T8=(struct _tuple19*)_TB4;}
# 1998
*_T7=_T8;_TA=_TB3 + 3;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 2001
_TB4->f0=_tag_fat("-finish",sizeof(char),8U);_TB4->f1=0;_TB4->f2=_tag_fat("",sizeof(char),1U);{struct Cyc_Arg_Unit_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_Unit_spec_Arg_Spec_struct));_TB5->tag=0;
_TB5->f1=Cyc_set_FINISH;_TC=(struct Cyc_Arg_Unit_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_TC;
_TB4->f4=_tag_fat("Produce Cyclone headers from pre-gathered C library info",sizeof(char),57U);_TB=(struct _tuple19*)_TB4;}
# 2001
*_TA=_TB;_TD=_TB3 + 4;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 2004
_TB4->f0=_tag_fat("-setjmp",sizeof(char),8U);_TB4->f1=0;_TB4->f2=_tag_fat("",sizeof(char),1U);{struct Cyc_Arg_Set_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_Set_spec_Arg_Spec_struct));_TB5->tag=3;
_TB5->f1=& Cyc_do_setjmp;_TF=(struct Cyc_Arg_Set_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_TF;
# 2009
_TB4->f4=_tag_fat("Produce the jmp_buf and setjmp declarations on the standard output, for use by the Cyclone compiler special file cyc_setjmp.h.  Cannot be used with -gather, -gatherscript, or specfiles.",sizeof(char),186U);_TE=(struct _tuple19*)_TB4;}
# 2004
*_TD=_TE;_T10=_TB3 + 5;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 2010
_TB4->f0=_tag_fat("-b",sizeof(char),3U);_TB4->f1=0;_TB4->f2=_tag_fat(" <machine>",sizeof(char),11U);{struct Cyc_Arg_String_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_String_spec_Arg_Spec_struct));_TB5->tag=5;
_TB5->f1=Cyc_Specsfile_set_target_arch;_T12=(struct Cyc_Arg_String_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_T12;
_TB4->f4=_tag_fat("Set the target machine for compilation to <machine>",sizeof(char),52U);_T11=(struct _tuple19*)_TB4;}
# 2010
*_T10=_T11;_T13=_TB3 + 6;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 2013
_TB4->f0=_tag_fat("-B",sizeof(char),3U);_TB4->f1=1;_TB4->f2=_tag_fat("<file>",sizeof(char),7U);{struct Cyc_Arg_Flag_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_Flag_spec_Arg_Spec_struct));_TB5->tag=1;
_TB5->f1=Cyc_Specsfile_add_cyclone_exec_path;_T15=(struct Cyc_Arg_Flag_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_T15;
_TB4->f4=_tag_fat("Add to the list of directories to search for compiler files",sizeof(char),60U);_T14=(struct _tuple19*)_TB4;}
# 2013
*_T13=_T14;_T16=_TB3 + 7;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 2016
_TB4->f0=_tag_fat("-h",sizeof(char),3U);_TB4->f1=0;_TB4->f2=_tag_fat(" <header>",sizeof(char),10U);{struct Cyc_Arg_String_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_String_spec_Arg_Spec_struct));_TB5->tag=5;
_TB5->f1=Cyc_set_header;_T18=(struct Cyc_Arg_String_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_T18;
_TB4->f4=_tag_fat("Produce this header, and other -h headers only",sizeof(char),47U);_T17=(struct _tuple19*)_TB4;}
# 2016
*_T16=_T17;_T19=_TB3 + 8;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 2019
_TB4->f0=_tag_fat("-v",sizeof(char),3U);_TB4->f1=0;_TB4->f2=_tag_fat("",sizeof(char),1U);{struct Cyc_Arg_Set_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_Set_spec_Arg_Spec_struct));_TB5->tag=3;
_TB5->f1=& Cyc_verbose;_T1B=(struct Cyc_Arg_Set_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_T1B;
_TB4->f4=_tag_fat("Verbose operation",sizeof(char),18U);_T1A=(struct _tuple19*)_TB4;}
# 2019
*_T19=_T1A;_T1C=_TB3 + 9;{struct _tuple19*_TB4=_cycalloc(sizeof(struct _tuple19));
# 2022
_TB4->f0=_tag_fat("-",sizeof(char),2U);_TB4->f1=1;_TB4->f2=_tag_fat("",sizeof(char),1U);{struct Cyc_Arg_Flag_spec_Arg_Spec_struct*_TB5=_cycalloc(sizeof(struct Cyc_Arg_Flag_spec_Arg_Spec_struct));_TB5->tag=1;
_TB5->f1=Cyc_add_cpparg;_T1E=(struct Cyc_Arg_Flag_spec_Arg_Spec_struct*)_TB5;}_TB4->f3=(void*)_T1E;
_TB4->f4=_tag_fat("",sizeof(char),1U);_T1D=(struct _tuple19*)_TB4;}
# 2022
*_T1C=_T1D;_T1F=_tag_fat(_TB3,sizeof(struct _tuple19*),10);_T0=Cyc_List_list(_T1F);}{
# 1991
struct Cyc_List_List*options=_T0;_T20=options;_T21=
# 2027
_tag_fat("Options:",sizeof(char),9U);_T22=argv;{struct _fat_ptr otherargs=Cyc_Specsfile_parse_b(_T20,Cyc_add_spec_file,Cyc_no_other,_T21,_T22);
# 2029
Cyc_Arg_current=0;_T23=options;_T24=
_tag_fat("Options:",sizeof(char),9U);_T25=otherargs;Cyc_Arg_parse(_T23,Cyc_add_spec_file,Cyc_no_other,_T24,_T25);_T26=Cyc_badparse;
if(_T26)goto _TL22A;else{goto _TL22E;}_TL22E: _T27=Cyc_do_setjmp;if(_T27)goto _TL22D;else{goto _TL22F;}_TL22F: if(Cyc_spec_files==0)goto _TL22A;else{goto _TL22D;}_TL22D: _T28=Cyc_do_setjmp;if(_T28)goto _TL230;else{goto _TL22C;}_TL230: if(Cyc_spec_files!=0)goto _TL22A;else{goto _TL22C;}_TL22C: _T29=Cyc_do_setjmp;if(_T29)goto _TL231;else{goto _TL22B;}_TL231: _T2A=Cyc_mode;_T2B=(int)_T2A;if(_T2B==1)goto _TL22A;else{goto _TL22B;}_TL22B: _T2C=Cyc_do_setjmp;if(_T2C)goto _TL232;else{goto _TL228;}_TL232: _T2D=Cyc_mode;_T2E=(int)_T2D;if(_T2E==2)goto _TL22A;else{goto _TL228;}
# 2036
_TL22A: _T2F=options;_T30=
# 2038
_tag_fat("Usage: buildlib [options] specfile1 specfile2 ...\nOptions:",sizeof(char),59U);
# 2036
Cyc_Arg_usage(_T2F,_T30);
# 2039
return 1;_TL228: _T31=
# 2045
_tag_fat("cycspecs",sizeof(char),9U);{struct _fat_ptr specs_file=Cyc_Specsfile_find_in_arch_path(_T31);_T32=Cyc_verbose;
if(!_T32)goto _TL233;{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;_TB3.f1=specs_file;_T33=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T33;void*_TB4[1];_T34=_TB4 + 0;*_T34=& _TB3;_T35=Cyc_stderr;_T36=_tag_fat("Reading from specs file %s\n",sizeof(char),28U);_T37=_tag_fat(_TB4,sizeof(void*),1);Cyc_fprintf(_T35,_T36,_T37);}goto _TL234;_TL233: _TL234: {
struct Cyc_List_List*specs=Cyc_Specsfile_read_specs(specs_file);_T38=specs;_T39=
_tag_fat("cyclone_target_cflags",sizeof(char),22U);Cyc_target_cflags=Cyc_Specsfile_get_spec(_T38,_T39);_T3A=Cyc_verbose;
if(!_T3A)goto _TL235;{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;_TB3.f1=Cyc_target_cflags;_T3B=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T3B;void*_TB4[1];_T3C=_TB4 + 0;*_T3C=& _TB3;_T3D=Cyc_stderr;_T3E=_tag_fat("Target cflags are %s\n",sizeof(char),22U);_T3F=_tag_fat(_TB4,sizeof(void*),1);Cyc_fprintf(_T3D,_T3E,_T3F);}goto _TL236;_TL235: _TL236: _T40=specs;_T41=
_tag_fat("cyclone_cc",sizeof(char),11U);Cyc_cyclone_cc=Cyc_Specsfile_get_spec(_T40,_T41);_T42=Cyc_cyclone_cc;_T43=_T42.curr;_T44=(unsigned)_T43;
if(_T44)goto _TL237;else{goto _TL239;}_TL239: Cyc_cyclone_cc=_tag_fat("gcc",sizeof(char),4U);goto _TL238;_TL237: _TL238: _T45=Cyc_verbose;
if(!_T45)goto _TL23A;{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;_TB3.f1=Cyc_cyclone_cc;_T46=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T46;void*_TB4[1];_T47=_TB4 + 0;*_T47=& _TB3;_T48=Cyc_stderr;_T49=_tag_fat("C compiler is %s\n",sizeof(char),18U);_T4A=_tag_fat(_TB4,sizeof(void*),1);Cyc_fprintf(_T48,_T49,_T4A);}goto _TL23B;_TL23A: _TL23B: _T4B=Cyc_mode;_T4C=(int)_T4B;
# 2054
if(_T4C!=2)goto _TL23C;_T4D=Cyc_verbose;
if(!_T4D)goto _TL23E;_T4E=Cyc_stderr;_T4F=
_tag_fat("Creating BUILDLIB.sh\n",sizeof(char),22U);_T50=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T4E,_T4F,_T50);goto _TL23F;_TL23E: _TL23F:
 Cyc_script_file=Cyc_fopen("BUILDLIB.sh","w");_T51=Cyc_script_file;_T52=(unsigned)_T51;
if(_T52)goto _TL240;else{goto _TL242;}
_TL242: _T53=Cyc_stderr;_T54=_tag_fat("Could not create file BUILDLIB.sh\n",sizeof(char),35U);_T55=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_T53,_T54,_T55);
exit(1);goto _TL241;_TL240: _TL241: _T56=
# 2062
_tag_fat("#!/bin/sh\n",sizeof(char),11U);_T57=_tag_fat(0U,sizeof(void*),0);Cyc_prscript(_T56,_T57);_T58=
_tag_fat("GCC=\"gcc\"\n",sizeof(char),11U);_T59=_tag_fat(0U,sizeof(void*),0);Cyc_prscript(_T58,_T59);goto _TL23D;_TL23C: _TL23D: _T5A=
# 2067
Cyc_force_directory_prefixes(Cyc_output_dir);if(_T5A)goto _TL245;else{goto _TL246;}_TL246: _T5B=Cyc_force_directory(Cyc_output_dir);if(_T5B)goto _TL245;else{goto _TL243;}
_TL245:{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;_TB3.f1=Cyc_output_dir;_T5C=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T5C;void*_TB4[1];_T5D=_TB4 + 0;*_T5D=& _TB3;_T5E=Cyc_stderr;_T5F=_tag_fat("Error: could not create directory %s\n",sizeof(char),38U);_T60=_tag_fat(_TB4,sizeof(void*),1);Cyc_fprintf(_T5E,_T5F,_T60);}
return 1;_TL243: _T61=Cyc_verbose;
# 2071
if(!_T61)goto _TL247;{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;
_TB3.f1=Cyc_output_dir;_T62=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T62;void*_TB4[1];_T63=_TB4 + 0;*_T63=& _TB3;_T64=Cyc_stderr;_T65=_tag_fat("Output directory is %s\n",sizeof(char),24U);_T66=_tag_fat(_TB4,sizeof(void*),1);Cyc_fprintf(_T64,_T65,_T66);}goto _TL248;_TL247: _TL248: _T67=Cyc_mode;_T68=(int)_T67;
# 2074
if(_T68!=2)goto _TL249;{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;
_TB3.f1=Cyc_output_dir;_T69=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T69;void*_TB4[1];_T6A=_TB4 + 0;*_T6A=& _TB3;_T6B=_tag_fat("cd %s\n",sizeof(char),7U);_T6C=_tag_fat(_TB4,sizeof(void*),1);Cyc_prscript(_T6B,_T6C);}{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;
# 2077
_TB3.f1=Cyc_target_cflags;_T6D=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T6D;void*_TB4[1];_T6E=_TB4 + 0;*_T6E=& _TB3;_T6F=
# 2076
_tag_fat("echo | $GCC %s -E -dM - -o INITMACROS.h\n",sizeof(char),41U);_T70=_tag_fat(_TB4,sizeof(void*),1);Cyc_prscript(_T6F,_T70);}goto _TL24A;_TL249: _TL24A: _T71=
# 2080
Cyc_gathering();if(_T71)goto _TL24B;else{goto _TL24D;}
# 2083
_TL24D: _T72=Cyc_output_dir;_T73=_tag_fat("BUILDLIB.LOG",sizeof(char),13U);_T74=Cyc_Filename_concat(_T72,_T73);_T75=_untag_fat_ptr_check_bound(_T74,sizeof(char),1U);_T76=_check_null(_T75);_T77=(const char*)_T76;Cyc_log_file=Cyc_fopen(_T77,"w");_T78=Cyc_log_file;_T79=(unsigned)_T78;
if(_T79)goto _TL24E;else{goto _TL250;}
_TL250:{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;_TB3.f1=Cyc_output_dir;_T7A=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T7A;void*_TB4[1];_T7B=_TB4 + 0;*_T7B=& _TB3;_T7C=Cyc_stderr;_T7D=_tag_fat("Error: could not create log file in directory %s\n",sizeof(char),50U);_T7E=_tag_fat(_TB4,sizeof(void*),1);Cyc_fprintf(_T7C,_T7D,_T7E);}
return 1;_TL24E: _T7F=Cyc_do_setjmp;
# 2089
if(_T7F)goto _TL251;else{goto _TL253;}
# 2091
_TL253: _T80=Cyc_output_dir;_T81=_tag_fat("cstubs.c",sizeof(char),9U);_T82=Cyc_Filename_concat(_T80,_T81);_T83=_untag_fat_ptr_check_bound(_T82,sizeof(char),1U);_T84=_check_null(_T83);_T85=(const char*)_T84;Cyc_cstubs_file=Cyc_fopen(_T85,"w");_T86=Cyc_cstubs_file;_T87=(unsigned)_T86;
if(_T87)goto _TL254;else{goto _TL256;}
_TL256:{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;_TB3.f1=Cyc_output_dir;_T88=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T88;void*_TB4[1];_T89=_TB4 + 0;*_T89=& _TB3;_T8A=Cyc_stderr;_T8B=_tag_fat("Error: could not create cstubs.c in directory %s\n",sizeof(char),50U);_T8C=_tag_fat(_TB4,sizeof(void*),1);Cyc_fprintf(_T8A,_T8B,_T8C);}
return 1;_TL254: _T8D=Cyc_output_dir;_T8E=
# 2098
_tag_fat("cycstubs.cyc",sizeof(char),13U);_T8F=Cyc_Filename_concat(_T8D,_T8E);_T90=_untag_fat_ptr_check_bound(_T8F,sizeof(char),1U);_T91=_check_null(_T90);_T92=(const char*)_T91;Cyc_cycstubs_file=Cyc_fopen(_T92,"w");_T93=Cyc_cycstubs_file;_T94=(unsigned)_T93;
if(_T94)goto _TL257;else{goto _TL259;}
_TL259:{struct Cyc_String_pa_PrintArg_struct _TB3;_TB3.tag=0;
# 2102
_TB3.f1=Cyc_output_dir;_T95=_TB3;}{struct Cyc_String_pa_PrintArg_struct _TB3=_T95;void*_TB4[1];_T96=_TB4 + 0;*_T96=& _TB3;_T97=Cyc_stderr;_T98=
# 2101
_tag_fat("Error: could not create cycstubs.c in directory %s\n",sizeof(char),52U);_T99=_tag_fat(_TB4,sizeof(void*),1);Cyc_fprintf(_T97,_T98,_T99);}
# 2103
return 1;_TL257: _T9A=Cyc_cycstubs_file;_T9B=
# 2107
_tag_fat("#include <core.h>\nusing Core;\n\n",sizeof(char),32U);_T9C=_tag_fat(0U,sizeof(void*),0);
# 2105
Cyc_fprintf(_T9A,_T9B,_T9C);goto _TL252;_TL251: _TL252: goto _TL24C;_TL24B: _TL24C: _T9D=Cyc_output_dir;_T9E=_untag_fat_ptr_check_bound(_T9D,sizeof(char),1U);_T9F=_check_null(_T9E);{
# 2112
const char*outdir=(const char*)_T9F;_TA0=Cyc_do_setjmp;
if(!_TA0)goto _TL25A;_TA1=Cyc_process_setjmp(outdir);if(!_TA1)goto _TL25A;
return 1;
# 2118
_TL25A: _TL25F: if(Cyc_spec_files!=0)goto _TL25D;else{goto _TL25E;}
_TL25D: _TA2=Cyc_spec_files;_TA3=_TA2->hd;_TA4=(const char*)_TA3;_TA5=outdir;_TA6=Cyc_process_specfile(_TA4,_TA5);if(!_TA6)goto _TL260;_TA7=Cyc_stderr;_TA8=
_tag_fat("FATAL ERROR -- QUIT!\n",sizeof(char),22U);_TA9=_tag_fat(0U,sizeof(void*),0);Cyc_fprintf(_TA7,_TA8,_TA9);
exit(1);goto _TL261;_TL260: _TL261: _TAA=Cyc_spec_files;
# 2118
Cyc_spec_files=_TAA->tl;goto _TL25F;_TL25E: _TAB=Cyc_mode;_TAC=(int)_TAB;
# 2126
if(_TAC!=2)goto _TL262;_TAD=
_check_null(Cyc_script_file);Cyc_fclose(_TAD);goto _TL263;
# 2129
_TL262: _TAE=Cyc_gathering();if(_TAE)goto _TL264;else{goto _TL266;}
_TL266: _TAF=_check_null(Cyc_log_file);Cyc_fclose(_TAF);_TB0=Cyc_do_setjmp;
if(_TB0)goto _TL267;else{goto _TL269;}
_TL269: _TB1=_check_null(Cyc_cstubs_file);Cyc_fclose(_TB1);_TB2=
_check_null(Cyc_cycstubs_file);Cyc_fclose(_TB2);goto _TL268;_TL267: _TL268: goto _TL265;_TL264: _TL265: _TL263:
# 2137
 return 0;}}}}}}
