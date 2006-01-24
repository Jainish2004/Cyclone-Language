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
void* _throw_fn(void*,const char*,unsigned);
void* _rethrow(void*);
#define _throw_null() (_throw_null_fn(__FILE__,__LINE__))
#define _throw_arraybounds() (_throw_arraybounds_fn(__FILE__,__LINE__))
#define _throw_badalloc() (_throw_badalloc_fn(__FILE__,__LINE__))
#define _throw_match() (_throw_match_fn(__FILE__,__LINE__))
#define _throw(e) (_throw_fn((e),__FILE__,__LINE__))
#endif

void* Cyc_Core_get_exn_thrown();
/* Built-in Exceptions */
struct Cyc_Null_Exception_exn_struct { char *tag; };
struct Cyc_Array_bounds_exn_struct { char *tag; };
struct Cyc_Match_Exception_exn_struct { char *tag; };
struct Cyc_Bad_alloc_exn_struct { char *tag; };
extern char Cyc_Null_Exception[];
extern char Cyc_Array_bounds[];
extern char Cyc_Match_Exception[];
extern char Cyc_Bad_alloc[];

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

# 95 "core.h"
 struct _fat_ptr Cyc_Core_new_string(unsigned);
# 100
struct _fat_ptr Cyc_Core_rnew_string(struct _RegionHandle*,unsigned);
# 105
struct _fat_ptr Cyc_Core_rqnew_string(struct _RegionHandle*,unsigned,unsigned);extern char Cyc_Core_Invalid_argument[17U];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _fat_ptr f1;};extern char Cyc_Core_Impossible[11U];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _fat_ptr f1;};
# 173
extern struct _RegionHandle*Cyc_Core_heap_region;
# 179
extern unsigned Cyc_Core_aliasable_qual;
# 351 "core.h"
struct _fat_ptr Cyc_Core_mkfat(void*,unsigned,unsigned);struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 61 "list.h"
extern int Cyc_List_length(struct Cyc_List_List*);
# 310 "cycboot.h"
extern int toupper(int);
# 72 "string.h"
struct _fat_ptr Cyc_strncpy(struct _fat_ptr,struct _fat_ptr,unsigned long);
# 106 "string.h"
struct _fat_ptr Cyc_rstrdup(struct _RegionHandle*,struct _fat_ptr);
# 37 "string.cyc"
unsigned long Cyc_strlen(struct _fat_ptr s){
# 39
unsigned long i;
unsigned sz=_get_fat_size(s,sizeof(char));
for(i=0U;1 && i < sz;++ i){
if((int)((const char*)s.curr)[(int)i]==0)
return i;}
return i;}
# 47
inline static unsigned Cyc_umin(unsigned i,unsigned j){
# 49
if(i < j)return i;else{return j;}}
# 51
inline static unsigned Cyc_umax(unsigned i,unsigned j){
# 53
if(i > j)return i;else{return j;}}
# 59
int Cyc_strcmp(struct _fat_ptr s1,struct _fat_ptr s2){
if((char*)s1.curr==(char*)s2.curr)
return 0;{
int i=0;
unsigned sz1=_get_fat_size(s1,sizeof(char));
unsigned sz2=_get_fat_size(s2,sizeof(char));
unsigned minsz=Cyc_umin(sz1,sz2);
# 67
for(1;1 &&(unsigned)i < minsz;++ i){
char c1=((const char*)s1.curr)[i];
char c2=((const char*)s2.curr)[i];
if((int)c1==0){
if((int)c2==0)return 0;else{return -1;}}
if((int)c2==0)return 1;{
int diff=(int)c1 - (int)c2;
if(diff!=0)return diff;}}
# 76
if(sz1==sz2)return 0;
if(sz1 < sz2){if((int)((const char*)s2.curr)[i]==0)return 0;else{return -1;}}
1;
if((int)((const char*)s1.curr)[i]==0)return 0;else{return 1;}}}
# 82
int Cyc_strptrcmp(struct _fat_ptr*s1,struct _fat_ptr*s2){
return Cyc_strcmp(*s1,*s2);}
# 86
inline static int Cyc_ncmp(struct _fat_ptr s1,unsigned long len1,struct _fat_ptr s2,unsigned long len2,unsigned long n){
# 89
if(n <= 0U)return 0;{
# 91
unsigned long min_len=Cyc_umin(len1,len2);
unsigned long bound=Cyc_umin(min_len,n);
# 94
{unsigned i=0U;for(0;i < bound;++ i){
int retc;
if((retc=(int)((const char*)s1.curr)[(int)i]- (int)((const char*)s2.curr)[(int)i])!=0)
return retc;}}
# 99
if(len1 < n || len2 < n)
return(int)len1 - (int)len2;
return 0;}}
# 106
int Cyc_strncmp(struct _fat_ptr s1,struct _fat_ptr s2,unsigned long n){
unsigned long len1=Cyc_strlen(s1);
unsigned long len2=Cyc_strlen(s2);
1;
return Cyc_ncmp(s1,len1,s2,len2,n);}
# 117
int Cyc_zstrcmp(struct _fat_ptr a,struct _fat_ptr b){
if((char*)a.curr==(char*)b.curr)
return 0;{
unsigned long as=_get_fat_size(a,sizeof(char));
unsigned long bs=_get_fat_size(b,sizeof(char));
unsigned long min_length=Cyc_umin(as,bs);
int i=-1;
# 125
1;
# 127
while((++ i,(unsigned long)i < min_length)){
{int diff=(int)((const char*)a.curr)[i]- (int)((const char*)b.curr)[i];
if(diff!=0)
return diff;}
# 128
1U;}
# 132
return(int)as - (int)bs;}}
# 135
int Cyc_zstrncmp(struct _fat_ptr s1,struct _fat_ptr s2,unsigned long n){
if(n <= 0U)return 0;{
# 138
unsigned long s1size=_get_fat_size(s1,sizeof(char));
unsigned long s2size=_get_fat_size(s2,sizeof(char));
unsigned long min_size=Cyc_umin(s1size,s2size);
unsigned long bound=Cyc_umin(n,min_size);
# 143
1;
# 145
{int i=0;for(0;(unsigned long)i < bound;++ i){
if((int)((const char*)s1.curr)[i]< (int)((const char*)s2.curr)[i])
return -1;
if((int)((const char*)s2.curr)[i]< (int)((const char*)s1.curr)[i])
return 1;}}
# 151
if(min_size <= bound)
return 0;
if(s1size < s2size)
return -1;
return 1;}}
# 159
int Cyc_zstrptrcmp(struct _fat_ptr*a,struct _fat_ptr*b){
if((int)a==(int)b)return 0;
return Cyc_zstrcmp(*a,*b);}
# 169
inline static struct _fat_ptr Cyc_int_strcato(struct _fat_ptr dest,struct _fat_ptr src){
int i;
unsigned long dsize;unsigned long slen;unsigned long dlen;unsigned long dplen;
# 173
dlen=Cyc_strlen(dest);{
struct _fat_ptr dp=_fat_ptr_plus(dest,sizeof(char),(int)dlen);
dplen=_get_fat_size(dp,sizeof(char));
slen=Cyc_strlen(src);
# 178
if(slen <= dplen){
1;
for(i=0;(unsigned long)i < slen;++ i){
({struct _fat_ptr _Tmp0=_fat_ptr_plus(dp,sizeof(char),i);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2=((const char*)src.curr)[i];if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});}
# 183
if((unsigned long)i < dplen)
({struct _fat_ptr _Tmp0=_fat_ptr_plus(dp,sizeof(char),i);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2='\000';if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});}else{
# 187
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("strcat",sizeof(char),7U);_Tmp0;}));}
return dest;}}
# 193
struct _fat_ptr Cyc_strcat(struct _fat_ptr dest,struct _fat_ptr src){
return Cyc_int_strcato(dest,src);}
# 198
struct _fat_ptr Cyc_rstrconcat(struct _RegionHandle*r,struct _fat_ptr a,struct _fat_ptr b){
unsigned alen=Cyc_strlen(a);
unsigned blen=Cyc_strlen(b);
struct _fat_ptr ans=Cyc_Core_rnew_string(r,(alen + blen)+ 1U);
struct _fat_ptr ansb=_fat_ptr_plus(ans,sizeof(char),(int)alen);
unsigned i;unsigned j;
if(alen > _get_fat_size(ans,sizeof(char)))_throw((void*)({struct Cyc_Core_Impossible_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Impossible_exn_struct));_Tmp0->tag=Cyc_Core_Impossible,_Tmp0->f1=_tag_fat("rstrconcat 1",sizeof(char),13U);_Tmp0;}));
for(i=0U;i < alen;++ i){
({struct _fat_ptr _Tmp0=_fat_ptr_plus(ans,sizeof(char),(int)i);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2=((const char*)a.curr)[(int)i];if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});}
if(blen > _get_fat_size(ansb,sizeof(char)))_throw((void*)({struct Cyc_Core_Impossible_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Impossible_exn_struct));_Tmp0->tag=Cyc_Core_Impossible,_Tmp0->f1=_tag_fat("rstrconcat 1",sizeof(char),13U);_Tmp0;}));
for(j=0U;j < blen;++ j){
({struct _fat_ptr _Tmp0=_fat_ptr_plus(ansb,sizeof(char),(int)j);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2=((const char*)b.curr)[(int)j];if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});}
return ans;}
# 213
struct _fat_ptr Cyc_strconcat(struct _fat_ptr a,struct _fat_ptr b){
return Cyc_rstrconcat(Cyc_Core_heap_region,a,b);}
# 218
struct _fat_ptr Cyc_rstrconcat_l(struct _RegionHandle*r,struct Cyc_List_List*strs){
# 220
unsigned long len;
unsigned long total_len=0U;
struct _fat_ptr ans;
struct _RegionHandle _Tmp0=_new_region(0U,"temp");struct _RegionHandle*temp=& _Tmp0;_push_region(temp);{
struct Cyc_List_List*lens;
lens=_region_malloc(temp,0U,sizeof(struct Cyc_List_List)),lens->hd=(void*)0U,lens->tl=0;{
# 227
struct Cyc_List_List*end=lens;
{struct Cyc_List_List*p=strs;for(0;p!=0;p=p->tl){
len=Cyc_strlen(*((struct _fat_ptr*)p->hd));
total_len +=len;{
struct Cyc_List_List*q;q=_region_malloc(temp,0U,sizeof(struct Cyc_List_List)),q->hd=(void*)len,q->tl=0;
end->tl=q;
end=q;}}}
# 235
lens=lens->tl;
ans=Cyc_Core_rnew_string(r,total_len + 1U);{
unsigned long i=0U;
while(strs!=0){
{struct _fat_ptr next=*((struct _fat_ptr*)strs->hd);
len=(unsigned long)_check_null(lens)->hd;
({struct _fat_ptr _Tmp1=_fat_ptr_decrease_size(_fat_ptr_plus(ans,sizeof(char),(int)i),sizeof(char),1U);struct _fat_ptr _Tmp2=next;Cyc_strncpy(_Tmp1,_Tmp2,len);});
i +=len;
strs=strs->tl;
lens=lens->tl;}
# 239
1U;}}}}{
# 247
struct _fat_ptr _Tmp1=ans;_npop_handler(0);return _Tmp1;}
# 223
;_pop_region();}
# 250
struct _fat_ptr Cyc_strconcat_l(struct Cyc_List_List*strs){
return Cyc_rstrconcat_l(Cyc_Core_heap_region,strs);}
# 256
struct _fat_ptr Cyc_rstr_sepstr(struct _RegionHandle*r,struct Cyc_List_List*strs,struct _fat_ptr separator){
if(strs==0)return Cyc_Core_rnew_string(r,0U);
if(strs->tl==0)return Cyc_rstrdup(r,*((struct _fat_ptr*)strs->hd));{
struct Cyc_List_List*p=strs;
struct _RegionHandle _Tmp0=_new_region(0U,"temp");struct _RegionHandle*temp=& _Tmp0;_push_region(temp);
{struct Cyc_List_List*lens;lens=_region_malloc(temp,0U,sizeof(struct Cyc_List_List)),lens->hd=(void*)0U,lens->tl=0;{
struct Cyc_List_List*end=lens;
unsigned long len;
unsigned long total_len=0U;
unsigned long list_len=0U;
for(1;p!=0;p=p->tl){
len=Cyc_strlen(*((struct _fat_ptr*)p->hd));
total_len +=len;{
struct Cyc_List_List*q;q=_region_malloc(temp,0U,sizeof(struct Cyc_List_List)),q->hd=(void*)len,q->tl=0;
end->tl=q;
end=q;
++ list_len;}}
# 274
lens=lens->tl;{
unsigned long seplen=Cyc_strlen(separator);
total_len +=(list_len - 1U)* seplen;{
struct _fat_ptr ans=Cyc_Core_rnew_string(r,total_len + 1U);
unsigned long i=0U;
while(_check_null(strs)->tl!=0){
{struct _fat_ptr next=*((struct _fat_ptr*)strs->hd);
len=(unsigned long)_check_null(lens)->hd;
({struct _fat_ptr _Tmp1=_fat_ptr_decrease_size(_fat_ptr_plus(ans,sizeof(char),(int)i),sizeof(char),1U);struct _fat_ptr _Tmp2=next;Cyc_strncpy(_Tmp1,_Tmp2,len);});
i +=len;
({struct _fat_ptr _Tmp1=_fat_ptr_decrease_size(_fat_ptr_plus(ans,sizeof(char),(int)i),sizeof(char),1U);struct _fat_ptr _Tmp2=separator;Cyc_strncpy(_Tmp1,_Tmp2,seplen);});
i +=seplen;
strs=strs->tl;
lens=lens->tl;}
# 280
1U;}
# 289
({struct _fat_ptr _Tmp1=_fat_ptr_decrease_size(_fat_ptr_plus(ans,sizeof(char),(int)i),sizeof(char),1U);struct _fat_ptr _Tmp2=*((struct _fat_ptr*)strs->hd);Cyc_strncpy(_Tmp1,_Tmp2,(unsigned long)_check_null(lens)->hd);});{
struct _fat_ptr _Tmp1=ans;_npop_handler(0);return _Tmp1;}}}}}
# 261
;_pop_region();}}
# 294
struct _fat_ptr Cyc_str_sepstr(struct Cyc_List_List*strs,struct _fat_ptr separator){
return Cyc_rstr_sepstr(Cyc_Core_heap_region,strs,separator);}
# 299
struct _fat_ptr Cyc_strncpy(struct _fat_ptr dest,struct _fat_ptr src,unsigned long n){
unsigned i;
if(n > _get_fat_size(src,sizeof(char))|| n > _get_fat_size(dest,sizeof(char)))
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("strncpy",sizeof(char),8U);_Tmp0;}));
for(i=0U;i < n;++ i){
char srcChar=((const char*)src.curr)[(int)i];
if((int)srcChar==0)break;
((char*)dest.curr)[(int)i]=srcChar;}
# 308
for(1;i < n;++ i){
((char*)dest.curr)[(int)i]='\000';}
# 311
return dest;}
# 315
struct _fat_ptr Cyc_zstrncpy(struct _fat_ptr dest,struct _fat_ptr src,unsigned long n){
if(n > _get_fat_size(dest,sizeof(char))|| n > _get_fat_size(src,sizeof(char)))
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("zstrncpy",sizeof(char),9U);_Tmp0;}));{
unsigned i;
for(i=0U;i < n;++ i){
((char*)dest.curr)[(int)i]=((const char*)src.curr)[(int)i];}
return dest;}}
# 324
struct _fat_ptr Cyc_strcpy(struct _fat_ptr dest,struct _fat_ptr src){
unsigned ssz=_get_fat_size(src,sizeof(char));
unsigned dsz=_get_fat_size(dest,sizeof(char));
if(ssz <= dsz){
unsigned i;
for(i=0U;i < ssz;++ i){
char srcChar=((const char*)src.curr)[(int)i];
({struct _fat_ptr _Tmp0=_fat_ptr_plus(dest,sizeof(char),(int)i);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2=srcChar;if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});
if((int)srcChar==0)break;}}else{
# 336
unsigned long len=Cyc_strlen(src);
({struct _fat_ptr _Tmp0=_fat_ptr_decrease_size(dest,sizeof(char),1U);struct _fat_ptr _Tmp1=src;Cyc_strncpy(_Tmp0,_Tmp1,len);});
if(len < _get_fat_size(dest,sizeof(char)))
({struct _fat_ptr _Tmp0=_fat_ptr_plus(dest,sizeof(char),(int)len);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2='\000';if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});}
# 341
return dest;}
# 347
struct _fat_ptr Cyc_rstrdup(struct _RegionHandle*r,struct _fat_ptr src){
unsigned long len;
struct _fat_ptr temp;
# 351
len=Cyc_strlen(src);
temp=Cyc_Core_rnew_string(r,len + 1U);
{struct _fat_ptr temp2=_fat_ptr_decrease_size(temp,sizeof(char),1U);
struct _fat_ptr _Tmp0;_Tmp0=temp2;{struct _fat_ptr dst=_Tmp0;
Cyc_strncpy(dst,src,len);}}
# 357
return temp;}
# 360
struct _fat_ptr Cyc_rqstrdup(struct _RegionHandle*r,unsigned q,struct _fat_ptr src){
unsigned long len=Cyc_strlen(src);
struct _fat_ptr temp=Cyc_Core_rqnew_string(r,q,len + 1U);
{
struct _fat_ptr temp2=_fat_ptr_decrease_size(temp,sizeof(char),1U);
struct _fat_ptr _Tmp0;_Tmp0=temp2;{struct _fat_ptr dst=_Tmp0;
Cyc_strncpy(dst,src,len);}}
# 368
return temp;}
# 371
struct _fat_ptr Cyc_strdup(struct _fat_ptr src){
return Cyc_rstrdup(Cyc_Core_heap_region,src);}
# 375
struct _fat_ptr Cyc_rqrealloc(struct _RegionHandle*r,unsigned q,struct _fat_ptr s,unsigned long sz){
struct _fat_ptr temp;
unsigned long slen;
# 379
slen=_get_fat_size(s,sizeof(char));
sz=sz > slen?sz: slen;
temp=({unsigned _Tmp0=sz;_tag_fat(_region_calloc(r,q,sizeof(char),_Tmp0),sizeof(char),_Tmp0);});
# 383
{struct _fat_ptr _Tmp0;_Tmp0=temp;{struct _fat_ptr dst=_Tmp0;
Cyc_strncpy(dst,s,slen);}}
# 387
return temp;}
# 390
struct _fat_ptr Cyc_rrealloc(struct _RegionHandle*r,struct _fat_ptr s,unsigned long sz){
return Cyc_rqrealloc(r,Cyc_Core_aliasable_qual,s,sz);}
# 394
struct _fat_ptr Cyc_rexpand(struct _RegionHandle*r,struct _fat_ptr s,unsigned long sz){
struct _fat_ptr temp;
unsigned long slen;
# 398
slen=Cyc_strlen(s);
sz=sz > slen?sz: slen;
temp=Cyc_Core_rnew_string(r,sz);
# 402
{struct _fat_ptr _Tmp0;_Tmp0=temp;{struct _fat_ptr dst=_Tmp0;
Cyc_strncpy(dst,s,slen);}}
# 406
if(slen!=_get_fat_size(s,sizeof(char)))
({struct _fat_ptr _Tmp0=_fat_ptr_plus(temp,sizeof(char),(int)slen);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2='\000';if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});
# 409
return temp;}
# 412
struct _fat_ptr Cyc_expand(struct _fat_ptr s,unsigned long sz){
return Cyc_rexpand(Cyc_Core_heap_region,s,sz);}
# 416
struct _fat_ptr Cyc_cond_rrealloc_str(struct _RegionHandle*r,struct _fat_ptr str,unsigned long sz){
unsigned long maxsizeP=_get_fat_size(str,sizeof(char));
struct _fat_ptr res=_tag_fat(0,0,0);
# 420
if(maxsizeP==0U){
maxsizeP=Cyc_umax(30U,sz);
res=Cyc_Core_rnew_string(r,maxsizeP);
({struct _fat_ptr _Tmp0=_fat_ptr_plus(res,sizeof(char),0);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2='\000';if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});}else{
# 425
if(sz > maxsizeP){
maxsizeP=maxsizeP * 2U > (sz * 5U)/ 4U?maxsizeP * 2U:(sz * 5U)/ 4U;{
struct _fat_ptr _Tmp0;_Tmp0=str;{struct _fat_ptr dst=_Tmp0;
res=Cyc_rexpand(r,dst,maxsizeP);}}}}
# 432
return res;}
# 435
struct _fat_ptr Cyc_realloc_str(struct _fat_ptr str,unsigned long sz){
struct _fat_ptr res=Cyc_cond_rrealloc_str(Cyc_Core_heap_region,str,sz);
if((char*)res.curr==(char*)_tag_fat(0,0,0).curr)return str;else{
return res;}}
# 446
struct _fat_ptr Cyc_rsubstring(struct _RegionHandle*r,struct _fat_ptr s,int start,unsigned long amt){
# 450
struct _fat_ptr ans=Cyc_Core_rnew_string(r,amt + 1U);
s=_fat_ptr_plus(s,sizeof(char),start);
if(amt > _get_fat_size(ans,sizeof(char))|| amt > _get_fat_size(s,sizeof(char)))
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("rsubstring",sizeof(char),11U);_Tmp0;}));
{unsigned long i=0U;for(0;i < amt;++ i){
({struct _fat_ptr _Tmp0=_fat_ptr_plus(ans,sizeof(char),(int)i);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2=*((const char*)_check_fat_subscript(s,sizeof(char),(int)i));if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});}}
({struct _fat_ptr _Tmp0=_fat_ptr_plus(ans,sizeof(char),(int)amt);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2='\000';if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});
return ans;}
# 460
struct _fat_ptr Cyc_substring(struct _fat_ptr s,int start,unsigned long amt){
return Cyc_rsubstring(Cyc_Core_heap_region,s,start,amt);}
# 466
struct _fat_ptr Cyc_rreplace_suffix(struct _RegionHandle*r,struct _fat_ptr src,struct _fat_ptr curr_suffix,struct _fat_ptr new_suffix){
unsigned long m=_get_fat_size(src,sizeof(char));
unsigned long n=_get_fat_size(curr_suffix,sizeof(char));
struct _fat_ptr err=_tag_fat("replace_suffix",sizeof(char),15U);
if(m < n)
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=err;_Tmp0;}));
{unsigned long i=1U;for(0;i <= n;++ i){
if(({int _Tmp0=(int)*((const char*)_check_fat_subscript(src,sizeof(char),(int)(m - i)));_Tmp0!=(int)*((const char*)_check_fat_subscript(curr_suffix,sizeof(char),(int)(n - i)));}))
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=err;_Tmp0;}));}}{
struct _fat_ptr ans=Cyc_Core_rnew_string(r,((m - n)+ _get_fat_size(new_suffix,sizeof(char)))+ 1U);
({struct _fat_ptr _Tmp0=_fat_ptr_decrease_size(ans,sizeof(char),1U);struct _fat_ptr _Tmp1=src;Cyc_strncpy(_Tmp0,_Tmp1,m - n);});
({struct _fat_ptr _Tmp0=_fat_ptr_decrease_size(_fat_ptr_plus(ans,sizeof(char),(int)(m - n)),sizeof(char),1U);struct _fat_ptr _Tmp1=new_suffix;Cyc_strncpy(_Tmp0,_Tmp1,_get_fat_size(new_suffix,sizeof(char)));});
return ans;}}
# 481
struct _fat_ptr Cyc_replace_suffix(struct _fat_ptr src,struct _fat_ptr curr_suffix,struct _fat_ptr new_suffix){
return Cyc_rreplace_suffix(Cyc_Core_heap_region,src,curr_suffix,new_suffix);}
# 488
struct _fat_ptr Cyc_strpbrk(struct _fat_ptr s,struct _fat_ptr accept){
int len=(int)_get_fat_size(s,sizeof(char));
unsigned asize=_get_fat_size(accept,sizeof(char));
char c;
unsigned i;
for(i=0U;i < (unsigned)len &&(int)(c=((const char*)s.curr)[(int)i])!=0;++ i){
unsigned j=0U;for(0;j < asize;++ j){
if((int)c==(int)((const char*)accept.curr)[(int)j])
return _fat_ptr_plus(s,sizeof(char),(int)i);}}
# 498
return _tag_fat(0,0,0);}
# 501
struct _fat_ptr Cyc_mstrpbrk(struct _fat_ptr s,struct _fat_ptr accept){
int len=(int)_get_fat_size(s,sizeof(char));
unsigned asize=_get_fat_size(accept,sizeof(char));
char c;
unsigned i;
for(i=0U;i < (unsigned)len &&(int)(c=((char*)s.curr)[(int)i])!=0;++ i){
unsigned j=0U;for(0;j < asize;++ j){
if((int)c==(int)((const char*)accept.curr)[(int)j])
return _fat_ptr_plus(s,sizeof(char),(int)i);}}
# 511
return _tag_fat(0,0,0);}
# 516
struct _fat_ptr Cyc_mstrchr(struct _fat_ptr s,char c){
int len=(int)_get_fat_size(s,sizeof(char));
char c2;
unsigned i;
# 521
for(i=0U;i < (unsigned)len &&(int)(c2=((char*)s.curr)[(int)i])!=0;++ i){
if((int)c2==(int)c)return _fat_ptr_plus(s,sizeof(char),(int)i);}
# 524
return _tag_fat(0,0,0);}
# 527
struct _fat_ptr Cyc_strchr(struct _fat_ptr s,char c){
int len=(int)_get_fat_size(s,sizeof(char));
char c2;
unsigned i;
# 532
for(i=0U;i < (unsigned)len &&(int)(c2=((const char*)s.curr)[(int)i])!=0;++ i){
if((int)c2==(int)c)return _fat_ptr_plus(s,sizeof(char),(int)i);}
# 535
return _tag_fat(0,0,0);}
# 540
struct _fat_ptr Cyc_strrchr(struct _fat_ptr s0,char c){
int len=(int)Cyc_strlen(s0);
int i=len - 1;
struct _fat_ptr s=s0;
_fat_ptr_inplace_plus(& s,sizeof(char),i);
# 546
for(1;i >= 0;(i --,_fat_ptr_inplace_plus_post(& s,sizeof(char),-1))){
if((int)*((const char*)_check_fat_subscript(s,sizeof(char),0U))==(int)c)
return s;}
# 550
return _tag_fat(0,0,0);}
# 553
struct _fat_ptr Cyc_mstrrchr(struct _fat_ptr s0,char c){
int len=(int)Cyc_strlen(s0);
int i=len - 1;
struct _fat_ptr s=s0;
_fat_ptr_inplace_plus(& s,sizeof(char),i);
# 559
for(1;i >= 0;(i --,_fat_ptr_inplace_plus_post(& s,sizeof(char),-1))){
if((int)*((char*)_check_fat_subscript(s,sizeof(char),0U))==(int)c)
return s;}
# 563
return _tag_fat(0,0,0);}
# 568
struct _fat_ptr Cyc_strstr(struct _fat_ptr haystack,struct _fat_ptr needle){
if(!((unsigned)haystack.curr)|| !((unsigned)needle.curr))_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("strstr",sizeof(char),7U);_Tmp0;}));
if((int)*((const char*)_check_fat_subscript(needle,sizeof(char),0U))==0)return haystack;{
# 572
int len=(int)Cyc_strlen(needle);
{struct _fat_ptr start=haystack;for(0;({
char*_Tmp0=(char*)(start=Cyc_strchr(start,*((const char*)needle.curr))).curr;_Tmp0!=(char*)_tag_fat(0,0,0).curr;});start=({
struct _fat_ptr _Tmp0=_fat_ptr_plus(start,sizeof(char),1);Cyc_strchr(_Tmp0,*((const char*)needle.curr));})){
if(Cyc_strncmp(start,needle,(unsigned long)len)==0)
return start;}}
# 579
return _tag_fat(0,0,0);}}
# 582
struct _fat_ptr Cyc_mstrstr(struct _fat_ptr haystack,struct _fat_ptr needle){
if(!((unsigned)haystack.curr)|| !((unsigned)needle.curr))_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("mstrstr",sizeof(char),8U);_Tmp0;}));
if((int)*((const char*)_check_fat_subscript(needle,sizeof(char),0U))==0)return haystack;{
# 586
int len=(int)Cyc_strlen(needle);
{struct _fat_ptr start=haystack;for(0;({
char*_Tmp0=(char*)(start=Cyc_mstrchr(start,*((const char*)needle.curr))).curr;_Tmp0!=(char*)_tag_fat(0,0,0).curr;});start=({
struct _fat_ptr _Tmp0=_fat_ptr_plus(start,sizeof(char),1);Cyc_mstrchr(_Tmp0,*((const char*)needle.curr));})){
if(Cyc_strncmp(start,needle,(unsigned long)len)==0)
return start;}}
# 593
return _tag_fat(0,0,0);}}
# 599
unsigned long Cyc_strspn(struct _fat_ptr s,struct _fat_ptr accept){
struct _fat_ptr b=s;
unsigned long len=Cyc_strlen(b);
unsigned asize=_get_fat_size(accept,sizeof(char));
1;
{unsigned long i=0U;for(0;i < len;++ i){
int j;
for(j=0;(unsigned)j < asize;++ j){
if((int)*((const char*)_check_fat_subscript(b,sizeof(char),(int)i))==(int)((const char*)accept.curr)[j])
break;}
if((unsigned)j==asize)
return i;}}
# 613
return len;}
# 619
unsigned long Cyc_strcspn(struct _fat_ptr s,struct _fat_ptr accept){
struct _fat_ptr b=s;
unsigned long len=Cyc_strlen(b);
unsigned asize=_get_fat_size(accept,sizeof(char));
# 624
1;
{unsigned long i=0U;for(0;i < len;++ i){
int j;
for(j=0;(unsigned)j < asize;++ j){
if((int)*((const char*)_check_fat_subscript(b,sizeof(char),(int)i))==(int)((const char*)accept.curr)[j])return i;}}}
# 630
return len;}
# 637
struct _fat_ptr Cyc_strtok(struct _fat_ptr s,struct _fat_ptr delim){
# 639
static struct _fat_ptr olds={(void*)0,(void*)0,(void*)(0 + 0)};
struct _fat_ptr token;
# 642
if((char*)s.curr==(char*)_tag_fat(0,0,0).curr){
if((char*)olds.curr==(char*)_tag_fat(0,0,0).curr)
return _tag_fat(0,0,0);
s=olds;}{
# 649
unsigned long inc=Cyc_strspn(s,delim);
if(inc >= _get_fat_size(s,sizeof(char))||(int)*((char*)_check_fat_subscript(_fat_ptr_plus(s,sizeof(char),(int)inc),sizeof(char),0U))==0){
# 652
olds=_tag_fat(0,0,0);
return _tag_fat(0,0,0);}else{
# 656
_fat_ptr_inplace_plus(& s,sizeof(char),(int)inc);}
# 659
token=s;
s=Cyc_mstrpbrk(token,delim);
if((char*)s.curr==(char*)_tag_fat(0,0,0).curr)
# 663
olds=_tag_fat(0,0,0);else{
# 667
({struct _fat_ptr _Tmp0=s;char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2='\000';if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});
olds=_fat_ptr_plus(s,sizeof(char),1);}
# 670
return token;}}
# 674
struct Cyc_List_List*Cyc_rexplode(struct _RegionHandle*r,struct _fat_ptr s){
struct Cyc_List_List*result=0;
{int i=(int)(Cyc_strlen(s)- 1U);for(0;i >= 0;-- i){
result=({struct Cyc_List_List*_Tmp0=_region_malloc(r,0U,sizeof(struct Cyc_List_List));_Tmp0->hd=(void*)*((const char*)_check_fat_subscript(s,sizeof(char),i)),_Tmp0->tl=result;_Tmp0;});}}
return result;}
# 681
struct Cyc_List_List*Cyc_explode(struct _fat_ptr s){
return Cyc_rexplode(Cyc_Core_heap_region,s);}
# 685
struct _fat_ptr Cyc_implode(struct Cyc_List_List*chars){
struct _fat_ptr s=Cyc_Core_new_string((unsigned)(Cyc_List_length(chars)+ 1));
unsigned long i=0U;
while(chars!=0){
({struct _fat_ptr _Tmp0=_fat_ptr_plus(s,sizeof(char),(int)i ++);char _Tmp1=*((char*)_check_fat_subscript(_Tmp0,sizeof(char),0U));char _Tmp2=(char)((int)chars->hd);if(_get_fat_size(_Tmp0,sizeof(char))==1U &&(_Tmp1==0 && _Tmp2!=0))_throw_arraybounds();*((char*)_Tmp0.curr)=_Tmp2;});
chars=chars->tl;
# 689
1U;}
# 692
return s;}
# 696
inline static int Cyc_casecmp(struct _fat_ptr s1,unsigned long len1,struct _fat_ptr s2,unsigned long len2){
# 700
unsigned long min_length=Cyc_umin(len1,len2);
# 702
1;{
# 704
int i=-1;
while((++ i,(unsigned long)i < min_length)){
{int diff=({int _Tmp0=toupper((int)((const char*)s1.curr)[i]);_Tmp0 - toupper((int)((const char*)s2.curr)[i]);});
if(diff!=0)
return diff;}
# 706
1U;}
# 710
return(int)len1 - (int)len2;}}
# 713
int Cyc_strcasecmp(struct _fat_ptr s1,struct _fat_ptr s2){
if((char*)s1.curr==(char*)s2.curr)
return 0;{
unsigned long len1=Cyc_strlen(s1);
unsigned long len2=Cyc_strlen(s2);
return Cyc_casecmp(s1,len1,s2,len2);}}
# 721
inline static int Cyc_caseless_ncmp(struct _fat_ptr s1,unsigned long len1,struct _fat_ptr s2,unsigned long len2,unsigned long n){
# 725
if(n <= 0U)return 0;{
# 727
unsigned long min_len=Cyc_umin(len1,len2);
unsigned long bound=Cyc_umin(min_len,n);
# 730
1;
# 732
{int i=0;for(0;(unsigned long)i < bound;++ i){
int retc;
if((retc=({int _Tmp0=toupper((int)((const char*)s1.curr)[i]);_Tmp0 - toupper((int)((const char*)s2.curr)[i]);}))!=0)
return retc;}}
# 737
if(len1 < n || len2 < n)
return(int)len1 - (int)len2;
return 0;}}
# 743
int Cyc_strncasecmp(struct _fat_ptr s1,struct _fat_ptr s2,unsigned long n){
unsigned long len1=Cyc_strlen(s1);
unsigned long len2=Cyc_strlen(s2);
return Cyc_caseless_ncmp(s1,len1,s2,len2,n);}
# 752
extern void*memcpy(void*,const void*,unsigned long);
extern void*memmove(void*,const void*,unsigned long);
extern int memcmp(const void*,const void*,unsigned long);
extern char*memchr(const char*,char,unsigned long);
extern void*memset(void*,int,unsigned long);
extern void bcopy(const void*,void*,unsigned long);
extern void bzero(void*,unsigned long);
extern char*GC_realloc_hint(char*,unsigned,unsigned);
# 765
struct _fat_ptr Cyc_realloc(struct _fat_ptr s,unsigned long n){
unsigned _Tmp0=_get_fat_size(s,sizeof(char));unsigned _Tmp1;_Tmp1=_Tmp0;{unsigned oldsz=_Tmp1;
unsigned _Tmp2;_Tmp2=n;{unsigned l=_Tmp2;
char*res=({char*_Tmp3=(char*)_untag_fat_ptr_check_bound(s,sizeof(char),1U);unsigned _Tmp4=oldsz;GC_realloc_hint(_Tmp3,_Tmp4,l);});
return({struct _fat_ptr(*_Tmp3)(char*,unsigned,unsigned)=(struct _fat_ptr(*)(char*,unsigned,unsigned))Cyc_Core_mkfat;_Tmp3;})(_check_null(res),sizeof(char),l);}}}
# 772
struct _fat_ptr Cyc__memcpy(struct _fat_ptr d,struct _fat_ptr s,unsigned long n,unsigned sz){
if((((void*)d.curr==(void*)_tag_fat(0,0,0).curr || _get_fat_size(d,sizeof(void))< n)||(void*)s.curr==(void*)_tag_fat(0,0,0).curr)|| _get_fat_size(s,sizeof(void))< n)
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("memcpy",sizeof(char),7U);_Tmp0;}));
({void*_Tmp0=(void*)_untag_fat_ptr_check_bound(d,sizeof(void),1U);const void*_Tmp1=(const void*)_untag_fat_ptr_check_bound(s,sizeof(void),1U);memcpy(_Tmp0,_Tmp1,n * sz);});
return d;}
# 779
struct _fat_ptr Cyc__memmove(struct _fat_ptr d,struct _fat_ptr s,unsigned long n,unsigned sz){
if((((void*)d.curr==(void*)_tag_fat(0,0,0).curr || _get_fat_size(d,sizeof(void))< n)||(void*)s.curr==(void*)_tag_fat(0,0,0).curr)|| _get_fat_size(s,sizeof(void))< n)
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("memove",sizeof(char),7U);_Tmp0;}));
({void*_Tmp0=(void*)_untag_fat_ptr_check_bound(d,sizeof(void),1U);const void*_Tmp1=(const void*)_untag_fat_ptr_check_bound(s,sizeof(void),1U);memmove(_Tmp0,_Tmp1,n * sz);});
return d;}
# 786
int Cyc_memcmp(struct _fat_ptr s1,struct _fat_ptr s2,unsigned long n){
if((((char*)s1.curr==(char*)_tag_fat(0,0,0).curr ||(char*)s2.curr==(char*)_tag_fat(0,0,0).curr)|| _get_fat_size(s1,sizeof(char))< n)|| _get_fat_size(s2,sizeof(char))< n)
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("memcmp",sizeof(char),7U);_Tmp0;}));{
const void*_Tmp0=(const void*)_untag_fat_ptr_check_bound(s1,sizeof(char),1U);const void*_Tmp1=(const void*)_untag_fat_ptr_check_bound(s2,sizeof(char),1U);return memcmp(_Tmp0,_Tmp1,n);}}
# 792
struct _fat_ptr Cyc_memchr(struct _fat_ptr s,char c,unsigned long n){
unsigned sz=_get_fat_size(s,sizeof(char));
if((char*)s.curr==(char*)_tag_fat(0,0,0).curr || n > sz)
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("memchr",sizeof(char),7U);_Tmp0;}));{
char*p=({const char*_Tmp0=(const char*)_untag_fat_ptr_check_bound(s,sizeof(char),1U);char _Tmp1=c;memchr(_Tmp0,_Tmp1,n);});
if(p==0)return _tag_fat(0,0,0);{
unsigned sval=(unsigned)((const char*)_untag_fat_ptr(s,sizeof(char),1U));
unsigned pval=(unsigned)((const char*)p);
unsigned delta=pval - sval;
return _fat_ptr_plus(s,sizeof(char),(int)delta);}}}
# 804
struct _fat_ptr Cyc_mmemchr(struct _fat_ptr s,char c,unsigned long n){
unsigned sz=_get_fat_size(s,sizeof(char));
if((char*)s.curr==(char*)_tag_fat(0,0,0).curr || n > sz)
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("mmemchr",sizeof(char),8U);_Tmp0;}));{
char*p=({const char*_Tmp0=(const char*)_untag_fat_ptr_check_bound(s,sizeof(char),1U);char _Tmp1=c;memchr(_Tmp0,_Tmp1,n);});
if(p==0)return _tag_fat(0,0,0);{
unsigned sval=(unsigned)((const char*)_untag_fat_ptr(s,sizeof(char),1U));
unsigned pval=(unsigned)p;
unsigned delta=pval - sval;
return _fat_ptr_plus(s,sizeof(char),(int)delta);}}}
# 816
struct _fat_ptr Cyc_memset(struct _fat_ptr s,char c,unsigned long n){
if((char*)s.curr==(char*)_tag_fat(0,0,0).curr || n > _get_fat_size(s,sizeof(char)))
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("memset",sizeof(char),7U);_Tmp0;}));
({void*_Tmp0=(void*)((char*)_untag_fat_ptr_check_bound(s,sizeof(char),1U));int _Tmp1=(int)c;memset(_Tmp0,_Tmp1,n);});
return s;}
# 823
void Cyc_bzero(struct _fat_ptr s,unsigned long n){
if((char*)s.curr==(char*)_tag_fat(0,0,0).curr || _get_fat_size(s,sizeof(char))< n)
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("bzero",sizeof(char),6U);_Tmp0;}));
({void(*_Tmp0)(char*,unsigned long)=({void(*_Tmp1)(char*,unsigned long)=(void(*)(char*,unsigned long))bzero;_Tmp1;});char*_Tmp1=(char*)_untag_fat_ptr_check_bound(s,sizeof(char),1U);_Tmp0(_Tmp1,n);});}
# 829
void Cyc__bcopy(struct _fat_ptr src,struct _fat_ptr dst,unsigned long n,unsigned sz){
if((((void*)src.curr==(void*)_tag_fat(0,0,0).curr || _get_fat_size(src,sizeof(void))< n)||(void*)dst.curr==(void*)_tag_fat(0,0,0).curr)|| _get_fat_size(dst,sizeof(void))< n)
_throw((void*)({struct Cyc_Core_Invalid_argument_exn_struct*_Tmp0=_cycalloc(sizeof(struct Cyc_Core_Invalid_argument_exn_struct));_Tmp0->tag=Cyc_Core_Invalid_argument,_Tmp0->f1=_tag_fat("bcopy",sizeof(char),6U);_Tmp0;}));
({const void*_Tmp0=(const void*)_untag_fat_ptr_check_bound(src,sizeof(void),1U);void*_Tmp1=(void*)_untag_fat_ptr_check_bound(dst,sizeof(void),1U);bcopy(_Tmp0,_Tmp1,n * sz);});}
