#include <setjmp.h>
/* This is a C header file to be used by the output of the Cyclone to
   C translator.  The corresponding definitions are in file
   lib/runtime_cyc.c
*/
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

#ifdef NO_CYC_PREFIX
#define ADD_PREFIX(x) x
#else
#define ADD_PREFIX(x) Cyc_##x
#endif

#ifndef offsetof
/* should be size_t, but int is fine. */
#define offsetof(t,n) ((int)(&(((t *)0)->n)))
#endif

/* Tagged arrays */
struct _dyneither_ptr {
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};  

/* Discriminated Unions */
struct _xtunion_struct { char *tag; };

/* Need one of these per thread (we don't have threads)
   The runtime maintains a stack that contains either _handler_cons
   structs or _RegionHandle structs.  The tag is 0 for a handler_cons
   and 1 for a region handle.  */
struct _RuntimeStack {
  int tag; /* 0 for an exception handler, 1 for a region handle */
  struct _RuntimeStack *next;
};

/* Regions */
struct _RegionPage {
#ifdef CYC_REGION_PROFILE
  unsigned total_bytes;
  unsigned free_bytes;
#endif
  struct _RegionPage *next;
  char data[1];  /*FJS: used to be size 0, but that's forbidden in ansi c*/
};

struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
  char               *offset;
  char               *last_plus_one;
  struct _DynRegionHandle *sub_regions;
#ifdef CYC_REGION_PROFILE
  const char         *name;
#endif
};

struct _DynRegionFrame {
  struct _RuntimeStack s;
  struct _DynRegionHandle *x;
};

extern struct _RegionHandle _new_region(const char *);
extern void * _region_malloc(struct _RegionHandle *, unsigned);
extern void * _region_calloc(struct _RegionHandle *, unsigned t, unsigned n);
extern void   _free_region(struct _RegionHandle *);
extern void   _reset_region(struct _RegionHandle *);
extern struct _RegionHandle *_open_dynregion(struct _DynRegionFrame *f,
                                             struct _DynRegionHandle *h);
extern void   _pop_dynregion();

/* Exceptions */
struct _handler_cons {
  struct _RuntimeStack s;
  jmp_buf handler;
};
extern void _push_handler(struct _handler_cons *);
extern void _push_region(struct _RegionHandle *);
extern void _npop_handler(int);
extern void _pop_handler();
extern void _pop_region();

#ifndef _throw
extern int _throw_null_fn(const char *filename, unsigned lineno);
extern int _throw_arraybounds_fn(const char *filename, unsigned lineno);
extern int _throw_badalloc_fn(const char *filename, unsigned lineno);
extern int _throw_match_fn(const char *filename, unsigned lineno);
extern int _throw_fn(void* e, const char *filename, unsigned lineno);
#define _throw_null() (_throw_null_fn(__FILE__,__LINE__))
#define _throw_arraybounds() (_throw_arraybounds_fn(__FILE__,__LINE__))
#define _throw_badalloc() (_throw_badalloc_fn(__FILE__,__LINE__))
#define _throw_match() (_throw_match_fn(__FILE__,__LINE__))
#define _throw(e) (_throw_fn((e),__FILE__,__LINE__))
#endif

extern struct _xtunion_struct *_exn_thrown;

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
#ifdef __APPLE__
#define _INLINE_FUNCTIONS
#endif

#ifdef CYC_ANSI_OUTPUT
#define _INLINE  
#define _INLINE_FUNCTIONS
#else
#define _INLINE inline
#endif

#ifdef VC_C
#define _CYC_U_LONG_LONG_T __int64
#else
#ifdef GCC_C
#define _CYC_U_LONG_LONG_T unsigned long long
#else
#define _CYC_U_LONG_LONG_T unsigned long long
#endif
#endif

#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE void *
_check_null_fn(const void *ptr, const char *filename, unsigned lineno) {
  void*_check_null_temp = (void*)(ptr);
  if (!_check_null_temp) _throw_null_fn(filename,lineno);
  return _check_null_temp;
}
#define _check_null(p) (_check_null_fn((p),__FILE__,__LINE__))
#else
#define _check_null(ptr) \
  ({ void*_check_null_temp = (void*)(ptr); \
     if (!_check_null_temp) _throw_null(); \
     _check_null_temp; })
#endif
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  ((char *)ptr) + (elt_sz)*(index); })
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE char *
_check_known_subscript_null_fn(void *ptr, unsigned bound, unsigned elt_sz, unsigned index, const char *filename, unsigned lineno) {
  void*_cks_ptr = (void*)(ptr);
  unsigned _cks_bound = (bound);
  unsigned _cks_elt_sz = (elt_sz);
  unsigned _cks_index = (index);
  if (!_cks_ptr) _throw_null_fn(filename,lineno);
  if (_cks_index >= _cks_bound) _throw_arraybounds_fn(filename,lineno);
  return ((char *)_cks_ptr) + _cks_elt_sz*_cks_index;
}
#define _check_known_subscript_null(p,b,e) (_check_known_subscript_null_fn(p,b,e,__FILE__,__LINE__))
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  void*_cks_ptr = (void*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (_cks_index >= _cks_bound) _throw_arraybounds(); \
  ((char *)_cks_ptr) + _cks_elt_sz*_cks_index; })
#endif
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_notnull(bound,index) (index)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned
_check_known_subscript_notnull_fn(unsigned bound,unsigned index,const char *filename,unsigned lineno) { 
  unsigned _cksnn_bound = (bound); 
  unsigned _cksnn_index = (index); 
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds_fn(filename,lineno); 
  return _cksnn_index;
}
#define _check_known_subscript_notnull(b,i) (_check_known_subscript_notnull_fn(b,i,__FILE__,__LINE__))
#else
#define _check_known_subscript_notnull(bound,index) ({ \
  unsigned _cksnn_bound = (bound); \
  unsigned _cksnn_index = (index); \
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); \
  _cksnn_index; })
#endif
#endif

/* Add i to zero-terminated pointer x.  Checks for x being null and
   ensures that x[0..i-1] are not 0. */
#ifdef NO_CYC_BOUNDS_CHECK
#define _zero_arr_plus_char_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_short_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_int_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_float_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_double_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar_fn(orig_x,orig_sz,orig_i,f,l) ((orig_x)+(orig_i))
#else
static _INLINE char *
_zero_arr_plus_char_fn(char *orig_x, int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE short *
_zero_arr_plus_short_fn(short *orig_x, int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE int *
_zero_arr_plus_int_fn(int *orig_x, int orig_sz, int orig_i, const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE float *
_zero_arr_plus_float_fn(float *orig_x, int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE double *
_zero_arr_plus_double_fn(double *orig_x, int orig_sz, int orig_i,const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE long double *
_zero_arr_plus_longdouble_fn(long double *orig_x, int orig_sz, int orig_i, const char *filename, unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
static _INLINE void *
_zero_arr_plus_voidstar_fn(void **orig_x, int orig_sz, int orig_i,const char *filename,unsigned lineno) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null_fn(filename,lineno);
  if (orig_i < 0) _throw_arraybounds_fn(filename,lineno);
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds_fn(filename,lineno);
  return orig_x + orig_i;
}
#endif

#define _zero_arr_plus_char(x,s,i) \
  (_zero_arr_plus_char_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_short(x,s,i) \
  (_zero_arr_plus_short_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_int(x,s,i) \
  (_zero_arr_plus_int_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_float(x,s,i) \
  (_zero_arr_plus_float_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_double(x,s,i) \
  (_zero_arr_plus_double_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_longdouble(x,s,i) \
  (_zero_arr_plus_longdouble_fn(x,s,i,__FILE__,__LINE__))
#define _zero_arr_plus_voidstar(x,s,i) \
  (_zero_arr_plus_voidstar_fn(x,s,i,__FILE__,__LINE__))


/* Calculates the number of elements in a zero-terminated, thin array.
   If non-null, the array is guaranteed to have orig_offset elements. */
static _INLINE int
_get_zero_arr_size_char(const char *orig_x, unsigned int orig_offset) {
  const char *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_short(const short *orig_x, unsigned int orig_offset) {
  const short *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_int(const int *orig_x, unsigned int orig_offset) {
  const int *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_float(const float *orig_x, unsigned int orig_offset) {
  const float *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_double(const double *orig_x, unsigned int orig_offset) {
  const double *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_longdouble(const long double *orig_x, unsigned int orig_offset) {
  const long double *_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}
static _INLINE int
_get_zero_arr_size_voidstar(const void **orig_x, unsigned int orig_offset) {
  const void **_gres_x = orig_x;
  unsigned int _gres = 0;
  if (_gres_x != 0) {
     _gres = orig_offset;
     _gres_x += orig_offset - 1;
     while (*_gres_x != 0) { _gres_x++; _gres++; }
  }
  return _gres; 
}


/* Does in-place addition of a zero-terminated pointer (x += e and ++x).  
   Note that this expands to call _zero_arr_plus_<type>_fn. */
static _INLINE char *
_zero_arr_inplace_plus_char_fn(char **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_char_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_char(x,i) \
  _zero_arr_inplace_plus_char_fn(x,i,__FILE__,__LINE__)
static _INLINE short *
_zero_arr_inplace_plus_short_fn(short **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_short_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_short(x,i) \
  _zero_arr_inplace_plus_short_fn(x,i,__FILE__,__LINE__)
static _INLINE int *
_zero_arr_inplace_plus_int(int **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_int_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_int(x,i) \
  _zero_arr_inplace_plus_int_fn(x,i,__FILE__,__LINE__)
static _INLINE float *
_zero_arr_inplace_plus_float_fn(float **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_float_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_float(x,i) \
  _zero_arr_inplace_plus_float_fn(x,i,__FILE__,__LINE__)
static _INLINE double *
_zero_arr_inplace_plus_double_fn(double **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_double_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_double(x,i) \
  _zero_arr_inplace_plus_double_fn(x,i,__FILE__,__LINE__)
static _INLINE long double *
_zero_arr_inplace_plus_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_longdouble_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_longdouble(x,i) \
  _zero_arr_inplace_plus_longdouble_fn(x,i,__FILE__,__LINE__)
static _INLINE void *
_zero_arr_inplace_plus_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno) {
  *x = _zero_arr_plus_voidstar_fn(*x,1,orig_i,filename,lineno);
  return *x;
}
#define _zero_arr_inplace_plus_voidstar(x,i) \
  _zero_arr_inplace_plus_voidstar_fn(x,i,__FILE__,__LINE__)

/* Does in-place increment of a zero-terminated pointer (e.g., x++). */
static _INLINE char *
_zero_arr_inplace_plus_post_char_fn(char **x, int orig_i,const char *filename,unsigned lineno){
  char * _zap_res = *x;
  *x = _zero_arr_plus_char_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_char(x,i) \
  _zero_arr_inplace_plus_post_char_fn((char **)(x),(i),__FILE__,__LINE__)
static _INLINE short *
_zero_arr_inplace_plus_post_short_fn(short **x, int orig_i,const char *filename,unsigned lineno){
  short * _zap_res = *x;
  *x = _zero_arr_plus_short_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_short(x,i) \
  _zero_arr_inplace_plus_post_short_fn((short **)(x),(i),__FILE__,__LINE__)
static _INLINE int *
_zero_arr_inplace_plus_post_int_fn(int **x, int orig_i,const char *filename, unsigned lineno){
  int * _zap_res = *x;
  *x = _zero_arr_plus_int_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_int(x,i) \
  _zero_arr_inplace_plus_post_int_fn((int **)(x),(i),__FILE__,__LINE__)
static _INLINE float *
_zero_arr_inplace_plus_post_float_fn(float **x, int orig_i,const char *filename, unsigned lineno){
  float * _zap_res = *x;
  *x = _zero_arr_plus_float_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_float(x,i) \
  _zero_arr_inplace_plus_post_float_fn((float **)(x),(i),__FILE__,__LINE__)
static _INLINE double *
_zero_arr_inplace_plus_post_double_fn(double **x, int orig_i,const char *filename,unsigned lineno){
  double * _zap_res = *x;
  *x = _zero_arr_plus_double_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_double(x,i) \
  _zero_arr_inplace_plus_post_double_fn((double **)(x),(i),__FILE__,__LINE__)
static _INLINE long double *
_zero_arr_inplace_plus_post_longdouble_fn(long double **x, int orig_i,const char *filename,unsigned lineno){
  long double * _zap_res = *x;
  *x = _zero_arr_plus_longdouble_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_longdouble(x,i) \
  _zero_arr_inplace_plus_post_longdouble_fn((long double **)(x),(i),__FILE__,__LINE__)
static _INLINE void **
_zero_arr_inplace_plus_post_voidstar_fn(void ***x, int orig_i,const char *filename,unsigned lineno){
  void ** _zap_res = *x;
  *x = _zero_arr_plus_voidstar_fn(_zap_res,1,orig_i,filename,lineno);
  return _zap_res;
}
#define _zero_arr_inplace_plus_post_voidstar(x,i) \
  _zero_arr_inplace_plus_post_voidstar_fn((void***)(x),(i),__FILE__,__LINE__)

/* functions for dealing with dynamically sized pointers */
#ifdef NO_CYC_BOUNDS_CHECKS
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  return _cus_ans;
}
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
#endif
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_check_dyneither_subscript_fn(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index,const char *filename, unsigned lineno) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  /* JGM: not needed! if (!_cus_arr.base) _throw_null(); */ 
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds_fn(filename,lineno);
  return _cus_ans;
}
#define _check_dyneither_subscript(a,s,i) \
  _check_dyneither_subscript_fn(a,s,i,__FILE__,__LINE__)
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  /* JGM: not needed! if (!_cus_arr.base) _throw_null();*/ \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_tag_dyneither(const void *tcurr,unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _tag_arr_ans;
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr);
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts);
  return _tag_arr_ans;
}
#else
#define _tag_dyneither(tcurr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr *
_init_dyneither_ptr(struct _dyneither_ptr *arr_ptr,
                    void *arr, unsigned elt_sz, unsigned num_elts) {
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr);
  void* _itarr = (arr);
  _itarr_ptr->base = _itarr_ptr->curr = _itarr;
  _itarr_ptr->last_plus_one = ((unsigned char *)_itarr) + (elt_sz) * (num_elts);
  return _itarr_ptr;
}
#else
#define _init_dyneither_ptr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->base = _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ((arr).curr)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_untag_dyneither_ptr_fn(struct _dyneither_ptr arr, 
                        unsigned elt_sz,unsigned num_elts,
                        const char *filename, unsigned lineno) {
  struct _dyneither_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds_fn(filename,lineno);
  return _curr;
}
#define _untag_dyneither_ptr(a,s,e) \
  _untag_dyneither_ptr_fn(a,s,e,__FILE__,__LINE__)
#else
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
    _throw_arraybounds(); \
  _curr; })
#endif
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned
_get_dyneither_size(struct _dyneither_ptr arr,unsigned elt_sz) {
  struct _dyneither_ptr _get_arr_size_temp = (arr);
  unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr;
  unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one;
  return (_get_arr_size_curr < _get_arr_size_temp.base ||
          _get_arr_size_curr >= _get_arr_size_last) ? 0 :
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));
}
#else
#define _get_dyneither_size(arr,elt_sz) \
  ({struct _dyneither_ptr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_plus(struct _dyneither_ptr arr,unsigned elt_sz,int change) {
  struct _dyneither_ptr _ans = (arr);
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_plus(arr,elt_sz,change) ({ \
  struct _dyneither_ptr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_inplace_plus(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,
                            int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return *_arr_ptr;
}
#else
#define _dyneither_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_inplace_plus_post(struct _dyneither_ptr *arr_ptr,unsigned elt_sz,int change) {
  struct _dyneither_ptr * _arr_ptr = (arr_ptr);
  struct _dyneither_ptr _ans = *_arr_ptr;
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  struct _dyneither_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })
#endif

/* Decrease the upper bound on a fat pointer by numelts where sz is
   the size of the pointer's type.  Note that this can't be a macro
   if we're to get initializers right. */
static struct 
_dyneither_ptr _dyneither_ptr_decrease_size(struct _dyneither_ptr x,
                                            unsigned int sz,
                                            unsigned int numelts) {
  x.last_plus_one -= sz * numelts; 
  return x; 
}

/* Allocation */

extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);

/* FIX?  Not sure if we want to pass filename and lineno in here... */
static _INLINE void* _cycalloc(int n) {
  void * ans = (void *)GC_malloc(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cycalloc_atomic(int n) {
  void * ans = (void *)GC_malloc_atomic(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cyccalloc(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
static _INLINE void* _cyccalloc_atomic(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc_atomic(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
#define MAX_MALLOC_SIZE (1 << 28)
static _INLINE unsigned int _check_times(unsigned x, unsigned y) {
  _CYC_U_LONG_LONG_T whole_ans = 
    ((_CYC_U_LONG_LONG_T)x)*((_CYC_U_LONG_LONG_T)y);
  unsigned word_ans = (unsigned)whole_ans;
  if(word_ans < whole_ans || word_ans > MAX_MALLOC_SIZE)
    _throw_badalloc();
  return word_ans;
}

#if defined(CYC_REGION_PROFILE) 
extern void* _profile_GC_malloc(int,const char *file,const char *func,
                                int lineno);
extern void* _profile_GC_malloc_atomic(int,const char *file,
                                       const char *func,int lineno);
extern void* _profile_region_malloc(struct _RegionHandle *, unsigned,
                                    const char *file,
                                    const char *func,
                                    int lineno);
extern void* _profile_region_calloc(struct _RegionHandle *, unsigned,
                                    unsigned,
                                    const char *file,
                                    const char *func,
                                    int lineno);
extern struct _RegionHandle _profile_new_region(const char *rgn_name,
						const char *file,
						const char *func,
                                                int lineno);
extern void _profile_free_region(struct _RegionHandle *,
				 const char *file,
                                 const char *func,
                                 int lineno);
#  if !defined(RUNTIME_CYC)
#define _new_region(n) _profile_new_region(n,__FILE__,__FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__,__FUNCTION__,__LINE__)
#define _region_malloc(rh,n) _profile_region_malloc(rh,n,__FILE__,__FUNCTION__,__LINE__)
#define _region_calloc(rh,n,t) _profile_region_calloc(rh,n,t,__FILE__,__FUNCTION__,__LINE__)
#  endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__,__FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__,__FUNCTION__,__LINE__)
#endif
#endif

/* the next two routines swap [x] and [y]; not thread safe! */
static _INLINE void _swap_word(void *x, void *y) {
  unsigned long *lx = (unsigned long *)x, *ly = (unsigned long *)y, tmp;
  tmp = *lx;
  *lx = *ly;
  *ly = tmp;
}
static _INLINE void _swap_dyneither(struct _dyneither_ptr *x, 
				   struct _dyneither_ptr *y) {
  struct _dyneither_ptr tmp = *x;
  *x = *y;
  *y = tmp;
}

# 35 "core.h"
 typedef char*Cyc_Cstring;
# 36
typedef char*Cyc_CstringNN;
# 37
typedef struct _dyneither_ptr Cyc_string_t;
# 40
typedef struct _dyneither_ptr Cyc_mstring_t;
# 43
typedef struct _dyneither_ptr*Cyc_stringptr_t;
# 47
typedef struct _dyneither_ptr*Cyc_mstringptr_t;
# 50
typedef char*Cyc_Cbuffer_t;
# 52
typedef char*Cyc_CbufferNN_t;
# 54
typedef struct _dyneither_ptr Cyc_buffer_t;
# 56
typedef struct _dyneither_ptr Cyc_mbuffer_t;
# 59
typedef int Cyc_bool;struct Cyc_Core_NewRegion{struct _DynRegionHandle*dynregion;};
# 26 "cycboot.h"
typedef unsigned long Cyc_size_t;
# 33
typedef unsigned short Cyc_mode_t;
# 41
double modf(double,double*);struct Cyc___cycFILE;
# 49
typedef struct Cyc___cycFILE Cyc_FILE;
# 51
extern struct Cyc___cycFILE*Cyc_stdout;struct Cyc_String_pa_PrintArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Int_pa_PrintArg_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_PrintArg_struct{int tag;double f1;};struct Cyc_LongDouble_pa_PrintArg_struct{int tag;long double f1;};struct Cyc_ShortPtr_pa_PrintArg_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_PrintArg_struct{int tag;unsigned long*f1;};
# 68
typedef void*Cyc_parg_t;
# 73
struct _dyneither_ptr Cyc_aprintf(struct _dyneither_ptr,struct _dyneither_ptr);
# 100
int Cyc_fprintf(struct Cyc___cycFILE*,struct _dyneither_ptr,struct _dyneither_ptr);struct Cyc_ShortPtr_sa_ScanfArg_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_ScanfArg_struct{int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_ScanfArg_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_ScanfArg_struct{int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_DoublePtr_sa_ScanfArg_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_ScanfArg_struct{int tag;float*f1;};struct Cyc_CharPtr_sa_ScanfArg_struct{int tag;struct _dyneither_ptr f1;};
# 127
typedef void*Cyc_sarg_t;
# 157 "cycboot.h"
int Cyc_printf(struct _dyneither_ptr,struct _dyneither_ptr);
# 161
int Cyc_putc(int,struct Cyc___cycFILE*);
# 174
struct _dyneither_ptr Cyc_rprintf(struct _RegionHandle*,struct _dyneither_ptr,struct _dyneither_ptr);
# 224 "cycboot.h"
int Cyc_vfprintf(struct Cyc___cycFILE*,struct _dyneither_ptr,struct _dyneither_ptr);
# 228
int Cyc_vprintf(struct _dyneither_ptr,struct _dyneither_ptr);
# 232
struct _dyneither_ptr Cyc_vrprintf(struct _RegionHandle*,struct _dyneither_ptr,struct _dyneither_ptr);
# 239
int Cyc_vsnprintf(struct _dyneither_ptr,unsigned long,struct _dyneither_ptr,struct _dyneither_ptr);
# 243
int Cyc_vsprintf(struct _dyneither_ptr,struct _dyneither_ptr,struct _dyneither_ptr);extern char Cyc_FileCloseError[15];struct Cyc_FileCloseError_exn_struct{char*tag;};extern char Cyc_FileOpenError[14];struct Cyc_FileOpenError_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 89 "core.h"
typedef unsigned int Cyc_Core_sizeof_t;struct Cyc_Core_Opt{void*v;};
# 93
typedef struct Cyc_Core_Opt*Cyc_Core_opt_t;extern char Cyc_Core_Invalid_argument[17];struct Cyc_Core_Invalid_argument_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[8];struct Cyc_Core_Failure_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[11];struct Cyc_Core_Impossible_exn_struct{char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[10];struct Cyc_Core_Not_found_exn_struct{char*tag;};extern char Cyc_Core_Unreachable[12];struct Cyc_Core_Unreachable_exn_struct{char*tag;struct _dyneither_ptr f1;};
# 160 "core.h"
extern struct _RegionHandle*Cyc_Core_heap_region;
# 162
extern struct _RegionHandle*Cyc_Core_unique_region;extern char Cyc_Core_Open_Region[12];struct Cyc_Core_Open_Region_exn_struct{char*tag;};extern char Cyc_Core_Free_Region[12];struct Cyc_Core_Free_Region_exn_struct{char*tag;};
# 244 "core.h"
inline static void* arrcast(struct _dyneither_ptr dyn,unsigned int bd,unsigned int sz){
# 249
if(bd >> 20  || sz >> 12)
# 250
return 0;{
# 251
unsigned char*ptrbd=dyn.curr + bd * sz;
# 252
if(((ptrbd < dyn.curr  || dyn.curr == 0) || dyn.curr < dyn.base) || ptrbd > dyn.last_plus_one)
# 256
return 0;
# 257
return dyn.curr;};}struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};
# 39 "list.h"
typedef struct Cyc_List_List*Cyc_List_list_t;
# 49 "list.h"
typedef struct Cyc_List_List*Cyc_List_List_t;extern char Cyc_List_List_mismatch[14];struct Cyc_List_List_mismatch_exn_struct{char*tag;};extern char Cyc_List_Nth[4];struct Cyc_List_Nth_exn_struct{char*tag;};
# 87 "printf.cyc"
static struct _dyneither_ptr Cyc_parg2string(void*x){
# 88
void*_tmp0=x;_LL1: {struct Cyc_String_pa_PrintArg_struct*_tmp1=(struct Cyc_String_pa_PrintArg_struct*)_tmp0;if(_tmp1->tag != 0)goto _LL3;}_LL2: {
# 89
const char*_tmpCB;return(_tmpCB="string",_tag_dyneither(_tmpCB,sizeof(char),7));}_LL3: {struct Cyc_Int_pa_PrintArg_struct*_tmp2=(struct Cyc_Int_pa_PrintArg_struct*)_tmp0;if(_tmp2->tag != 1)goto _LL5;}_LL4: {
# 90
const char*_tmpCC;return(_tmpCC="int",_tag_dyneither(_tmpCC,sizeof(char),4));}_LL5: {struct Cyc_Double_pa_PrintArg_struct*_tmp3=(struct Cyc_Double_pa_PrintArg_struct*)_tmp0;if(_tmp3->tag != 2)goto _LL7;}_LL6: {
# 92
const char*_tmpCD;return(_tmpCD="double",_tag_dyneither(_tmpCD,sizeof(char),7));}_LL7: {struct Cyc_LongDouble_pa_PrintArg_struct*_tmp4=(struct Cyc_LongDouble_pa_PrintArg_struct*)_tmp0;if(_tmp4->tag != 3)goto _LL9;}_LL8: {
# 93
const char*_tmpCE;return(_tmpCE="long double",_tag_dyneither(_tmpCE,sizeof(char),12));}_LL9: {struct Cyc_ShortPtr_pa_PrintArg_struct*_tmp5=(struct Cyc_ShortPtr_pa_PrintArg_struct*)_tmp0;if(_tmp5->tag != 4)goto _LLB;}_LLA: {
# 94
const char*_tmpCF;return(_tmpCF="short *",_tag_dyneither(_tmpCF,sizeof(char),8));}_LLB: {struct Cyc_IntPtr_pa_PrintArg_struct*_tmp6=(struct Cyc_IntPtr_pa_PrintArg_struct*)_tmp0;if(_tmp6->tag != 5)goto _LL0;}_LLC: {
# 95
const char*_tmpD0;return(_tmpD0="unsigned long *",_tag_dyneither(_tmpD0,sizeof(char),16));}_LL0:;}
# 99
static void*Cyc_badarg(struct _dyneither_ptr s){
# 100
struct Cyc_Core_Invalid_argument_exn_struct _tmpD3;struct Cyc_Core_Invalid_argument_exn_struct*_tmpD2;return(void*)_throw((void*)((_tmpD2=_cycalloc(sizeof(*_tmpD2)),((_tmpD2[0]=((_tmpD3.tag=Cyc_Core_Invalid_argument,((_tmpD3.f1=s,_tmpD3)))),_tmpD2)))));}
# 104
static int Cyc_va_arg_int(struct _dyneither_ptr ap){
# 105
void*_tmpF=*((void**)_check_dyneither_subscript(ap,sizeof(void*),0));unsigned long _tmp11;_LLE: {struct Cyc_Int_pa_PrintArg_struct*_tmp10=(struct Cyc_Int_pa_PrintArg_struct*)_tmpF;if(_tmp10->tag != 1)goto _LL10;else{_tmp11=_tmp10->f1;}}_LLF:
# 106
 return(int)_tmp11;_LL10:;_LL11: {
# 107
const char*_tmpD4;return((int(*)(struct _dyneither_ptr s))Cyc_badarg)(((_tmpD4="printf expected int",_tag_dyneither(_tmpD4,sizeof(char),20))));}_LLD:;}
# 111
static long Cyc_va_arg_long(struct _dyneither_ptr ap){
# 112
void*_tmp13=*((void**)_check_dyneither_subscript(ap,sizeof(void*),0));unsigned long _tmp15;_LL13: {struct Cyc_Int_pa_PrintArg_struct*_tmp14=(struct Cyc_Int_pa_PrintArg_struct*)_tmp13;if(_tmp14->tag != 1)goto _LL15;else{_tmp15=_tmp14->f1;}}_LL14:
# 113
 return(long)_tmp15;_LL15:;_LL16: {
# 114
const char*_tmpD5;return((long(*)(struct _dyneither_ptr s))Cyc_badarg)(((_tmpD5="printf expected int",_tag_dyneither(_tmpD5,sizeof(char),20))));}_LL12:;}
# 118
static unsigned long Cyc_va_arg_ulong(struct _dyneither_ptr ap){
# 119
void*_tmp17=*((void**)_check_dyneither_subscript(ap,sizeof(void*),0));unsigned long _tmp19;_LL18: {struct Cyc_Int_pa_PrintArg_struct*_tmp18=(struct Cyc_Int_pa_PrintArg_struct*)_tmp17;if(_tmp18->tag != 1)goto _LL1A;else{_tmp19=_tmp18->f1;}}_LL19:
# 120
 return(unsigned long)_tmp19;_LL1A:;_LL1B: {
# 121
const char*_tmpD6;return((unsigned long(*)(struct _dyneither_ptr s))Cyc_badarg)(((_tmpD6="printf expected int",_tag_dyneither(_tmpD6,sizeof(char),20))));}_LL17:;}
# 125
static unsigned long Cyc_va_arg_uint(struct _dyneither_ptr ap){
# 126
void*_tmp1B=*((void**)_check_dyneither_subscript(ap,sizeof(void*),0));unsigned long _tmp1D;_LL1D: {struct Cyc_Int_pa_PrintArg_struct*_tmp1C=(struct Cyc_Int_pa_PrintArg_struct*)_tmp1B;if(_tmp1C->tag != 1)goto _LL1F;else{_tmp1D=_tmp1C->f1;}}_LL1E:
# 127
 return(unsigned int)_tmp1D;_LL1F:;_LL20: {
# 128
const char*_tmpD7;return((unsigned long(*)(struct _dyneither_ptr s))Cyc_badarg)(((_tmpD7="printf expected int",_tag_dyneither(_tmpD7,sizeof(char),20))));}_LL1C:;}
# 133
static double Cyc_va_arg_double(struct _dyneither_ptr ap){
# 134
void*_tmp1F=*((void**)_check_dyneither_subscript(ap,sizeof(void*),0));double _tmp21;long double _tmp23;_LL22: {struct Cyc_Double_pa_PrintArg_struct*_tmp20=(struct Cyc_Double_pa_PrintArg_struct*)_tmp1F;if(_tmp20->tag != 2)goto _LL24;else{_tmp21=_tmp20->f1;}}_LL23:
# 135
 return _tmp21;_LL24: {struct Cyc_LongDouble_pa_PrintArg_struct*_tmp22=(struct Cyc_LongDouble_pa_PrintArg_struct*)_tmp1F;if(_tmp22->tag != 3)goto _LL26;else{_tmp23=_tmp22->f1;}}_LL25:
# 136
 return(double)_tmp23;_LL26:;_LL27: {
# 138
const char*_tmpDB;void*_tmpDA[1];struct Cyc_String_pa_PrintArg_struct _tmpD9;(int)_throw(((void*(*)(struct _dyneither_ptr s))Cyc_badarg)((struct _dyneither_ptr)((_tmpD9.tag=0,((_tmpD9.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_parg2string(*((void**)_check_dyneither_subscript(ap,sizeof(void*),0)))),((_tmpDA[0]=& _tmpD9,Cyc_aprintf(((_tmpDB="printf expected double but found %s",_tag_dyneither(_tmpDB,sizeof(char),36))),_tag_dyneither(_tmpDA,sizeof(void*),1))))))))));}_LL21:;}
# 144
static short*Cyc_va_arg_short_ptr(struct _dyneither_ptr ap){
# 145
void*_tmp27=*((void**)_check_dyneither_subscript(ap,sizeof(void*),0));short*_tmp29;_LL29: {struct Cyc_ShortPtr_pa_PrintArg_struct*_tmp28=(struct Cyc_ShortPtr_pa_PrintArg_struct*)_tmp27;if(_tmp28->tag != 4)goto _LL2B;else{_tmp29=_tmp28->f1;}}_LL2A:
# 146
 return _tmp29;_LL2B:;_LL2C: {
# 147
const char*_tmpDC;(int)_throw(((void*(*)(struct _dyneither_ptr s))Cyc_badarg)(((_tmpDC="printf expected short pointer",_tag_dyneither(_tmpDC,sizeof(char),30)))));}_LL28:;}
# 152
static unsigned long*Cyc_va_arg_int_ptr(struct _dyneither_ptr ap){
# 153
void*_tmp2B=*((void**)_check_dyneither_subscript(ap,sizeof(void*),0));unsigned long*_tmp2D;_LL2E: {struct Cyc_IntPtr_pa_PrintArg_struct*_tmp2C=(struct Cyc_IntPtr_pa_PrintArg_struct*)_tmp2B;if(_tmp2C->tag != 5)goto _LL30;else{_tmp2D=_tmp2C->f1;}}_LL2F:
# 154
 return _tmp2D;_LL30:;_LL31: {
# 155
const char*_tmpDD;(int)_throw(((void*(*)(struct _dyneither_ptr s))Cyc_badarg)(((_tmpDD="printf expected int pointer",_tag_dyneither(_tmpDD,sizeof(char),28)))));}_LL2D:;}
# 160
static const struct _dyneither_ptr Cyc_va_arg_string(struct _dyneither_ptr ap){
# 161
void*_tmp2F=*((void**)_check_dyneither_subscript(ap,sizeof(void*),0));struct _dyneither_ptr _tmp31;_LL33: {struct Cyc_String_pa_PrintArg_struct*_tmp30=(struct Cyc_String_pa_PrintArg_struct*)_tmp2F;if(_tmp30->tag != 0)goto _LL35;else{_tmp31=_tmp30->f1;}}_LL34:
# 162
 return _tmp31;_LL35:;_LL36: {
# 163
const char*_tmpDE;(int)_throw(((void*(*)(struct _dyneither_ptr s))Cyc_badarg)(((_tmpDE="printf expected string",_tag_dyneither(_tmpDE,sizeof(char),23)))));}_LL32:;}
# 177 "printf.cyc"
int Cyc___cvt_double(double number,int prec,int flags,int*signp,int fmtch,struct _dyneither_ptr startp,struct _dyneither_ptr endp);
# 206 "printf.cyc"
enum Cyc_BASE{Cyc_OCT  = 0,Cyc_DEC  = 1,Cyc_HEX  = 2};
# 207
typedef enum Cyc_BASE Cyc_base_t;
# 212
inline static int Cyc__IO_sputn(int(*ioputc)(int,void*),void*ioputc_env,struct _dyneither_ptr s,int howmany){
# 214
int i=0;
# 215
while(i < howmany){
# 216
if(ioputc((int)*((const char*)_check_dyneither_subscript(s,sizeof(char),0)),ioputc_env)== - 1)return i;
# 217
_dyneither_ptr_inplace_plus(& s,sizeof(char),1);
# 218
++ i;}
# 220
return i;}
# 223
static int Cyc__IO_nzsputn(int(*ioputc)(int,void*),void*ioputc_env,struct _dyneither_ptr s,int howmany){
# 225
int i=0;
# 226
while(i < howmany){
# 227
char c;
# 228
if((c=*((const char*)_check_dyneither_subscript(s,sizeof(char),0)))== 0  || ioputc((int)c,ioputc_env)== - 1)return i;
# 229
_dyneither_ptr_inplace_plus(& s,sizeof(char),1);
# 230
++ i;}
# 232
return i;}
# 238
static int Cyc__IO_padn(int(*ioputc)(int,void*),void*ioputc_env,char c,int howmany){
# 240
int i=0;
# 241
while(i < howmany){
# 242
if(ioputc((int)c,ioputc_env)== - 1)return i;
# 243
++ i;}
# 245
return i;}
# 249
static struct _dyneither_ptr Cyc_my_memchr(struct _dyneither_ptr s,char c,int n){
# 250
int sz=(int)_get_dyneither_size(s,sizeof(char));
# 251
n=n < sz?n: sz;
# 252
for(0;n != 0;(n --,_dyneither_ptr_inplace_plus_post(& s,sizeof(char),1))){
# 253
if(*((const char*)_check_dyneither_subscript(s,sizeof(char),0))== c)return s;}
# 255
return _tag_dyneither(0,0,0);}
# 258
static struct _dyneither_ptr Cyc_my_nzmemchr(struct _dyneither_ptr s,char c,int n){
# 259
int sz=(int)_get_dyneither_size(s,sizeof(char));
# 260
n=n < sz?n: sz;
# 261
for(0;n != 0;(n --,_dyneither_ptr_inplace_plus_post(& s,sizeof(char),1))){
# 262
if(*((const char*)_check_dyneither_subscript(s,sizeof(char),0))== c)return s;}
# 264
return _tag_dyneither(0,0,0);}
# 267
inline static const unsigned long Cyc_my_strlen(struct _dyneither_ptr s){
# 268
unsigned int sz=_get_dyneither_size(s,sizeof(char));
# 269
unsigned int i=0;
# 270
while(i < sz  && ((const char*)s.curr)[(int)i]!= 0){++ i;}
# 271
return i;}
# 278
int Cyc__IO_vfprintf(int(*ioputc)(int,void*),void*ioputc_env,struct _dyneither_ptr fmt0,struct _dyneither_ptr ap){
# 282
struct _dyneither_ptr fmt;
# 283
register int ch;
# 284
register int n;
# 286
struct _dyneither_ptr cp=_tag_dyneither(0,0,0);
# 289
struct _dyneither_ptr cp2=_tag_dyneither(0,0,0);
# 292
struct _dyneither_ptr cp3=_tag_dyneither(0,0,0);
# 295
int which_cp;
# 296
struct _dyneither_ptr fmark;
# 297
register int flags;
# 298
int ret;
# 299
int width;
# 300
int prec;
# 301
char sign;
# 303
char sign_string[2]={'\000','\000'};
# 304
int softsign=0;
# 305
double _double;
# 306
int fpprec;
# 307
unsigned long _ulong;
# 308
int dprec;
# 309
int dpad;
# 310
int fieldsz;
# 314
int size=0;
# 316
char buf[349];
# 317
{unsigned int _tmp58=348;unsigned int i;for(i=0;i < _tmp58;i ++){buf[i]='\000';}buf[_tmp58]=(char)0;}{char ox[2]={'\000','\000'};
# 318
enum Cyc_BASE base;
# 342 "printf.cyc"
fmt=fmt0;
# 343
ret=0;
# 348
for(0;1;0){
# 351
fmark=fmt;{
# 352
unsigned int fmt_sz=_get_dyneither_size(fmt,sizeof(char));
# 353
for(n=0;(n < fmt_sz  && (ch=(int)((const char*)fmt.curr)[n])!= '\000') && ch != '%';++ n){
# 354
;}
# 355
fmt=_dyneither_ptr_plus(fmt,sizeof(char),n);
# 357
if((n=(fmt.curr - fmark.curr)/ sizeof(char))!= 0){
# 358
do{if(Cyc__IO_sputn(ioputc,ioputc_env,(struct _dyneither_ptr)fmark,n)!= n)goto error;}while(0);
# 359
ret +=n;}
# 361
if(ch == '\000')
# 362
goto done;
# 363
_dyneither_ptr_inplace_plus(& fmt,sizeof(char),1);
# 365
flags=0;
# 366
dprec=0;
# 367
fpprec=0;
# 368
width=0;
# 369
prec=- 1;
# 370
sign='\000';
# 372
rflag: ch=(int)*((const char*)_check_dyneither_subscript(_dyneither_ptr_inplace_plus_post(& fmt,sizeof(char),1),sizeof(char),0));
# 373
reswitch: which_cp=0;
# 374
switch(ch){case ' ': _LL37:
# 381
 if(!((int)sign))
# 382
sign=' ';
# 383
goto rflag;case '#': _LL38:
# 385
 flags |=8;
# 386
goto rflag;case '*': _LL39:
# 394
 width=Cyc_va_arg_int(ap);_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 395
if(width >= 0)
# 396
goto rflag;
# 397
width=- width;
# 398
goto _LL3A;case '-': _LL3A:
# 400
 flags |=16;
# 401
flags &=~ 32;
# 402
goto rflag;case '+': _LL3B:
# 404
 sign='+';
# 405
goto rflag;case '.': _LL3C:
# 407
 if((ch=(int)*((const char*)_check_dyneither_subscript(_dyneither_ptr_inplace_plus_post(& fmt,sizeof(char),1),sizeof(char),0)))== '*'){
# 408
n=Cyc_va_arg_int(ap);_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 409
prec=n < 0?- 1: n;
# 410
goto rflag;}
# 412
n=0;
# 413
while((unsigned int)(ch - '0')<= 9){
# 414
n=10 * n + (ch - '0');
# 415
ch=(int)*((const char*)_check_dyneither_subscript(_dyneither_ptr_inplace_plus_post(& fmt,sizeof(char),1),sizeof(char),0));}
# 417
prec=n < 0?- 1: n;
# 418
goto reswitch;case '0': _LL3D:
# 425
 if(!(flags & 16))
# 426
flags |=32;
# 427
goto rflag;case '1': _LL3E:
# 428
 goto _LL3F;case '2': _LL3F: goto _LL40;case '3': _LL40: goto _LL41;case '4': _LL41: goto _LL42;case '5': _LL42:
# 429
 goto _LL43;case '6': _LL43: goto _LL44;case '7': _LL44: goto _LL45;case '8': _LL45: goto _LL46;case '9': _LL46:
# 430
 n=0;
# 431
do{
# 432
n=10 * n + (ch - '0');
# 433
ch=(int)*((const char*)_check_dyneither_subscript(_dyneither_ptr_inplace_plus_post(& fmt,sizeof(char),1),sizeof(char),0));}while((unsigned int)(ch - '0')<= 9);
# 435
width=n;
# 436
goto reswitch;case 'L': _LL47:
# 438
 flags |=2;
# 439
goto rflag;case 'h': _LL48:
# 441
 flags |=4;
# 442
goto rflag;case 'l': _LL49:
# 444
 flags |=1;
# 445
goto rflag;case 'c': _LL4A:
# 447
{char*_tmpDF;cp=((_tmpDF=buf,_tag_dyneither(_tmpDF,sizeof(char),_get_zero_arr_size_char((void*)_tmpDF,349))));}
# 448
{char _tmpE2;char _tmpE1;struct _dyneither_ptr _tmpE0;(_tmpE0=cp,((_tmpE1=*((char*)_check_dyneither_subscript(_tmpE0,sizeof(char),0)),((_tmpE2=(char)Cyc_va_arg_int(ap),((_get_dyneither_size(_tmpE0,sizeof(char))== 1  && (_tmpE1 == '\000'  && _tmpE2 != '\000')?_throw_arraybounds(): 1,*((char*)_tmpE0.curr)=_tmpE2)))))));}_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 449
size=1;
# 450
sign='\000';
# 451
break;case 'D': _LL4B:
# 453
 flags |=1;
# 454
goto _LL4C;case 'd': _LL4C:
# 455
 goto _LL4D;case 'i': _LL4D:
# 456
 _ulong=(unsigned long)(flags & 1?Cyc_va_arg_long(ap):(flags & 4?(long)((short)Cyc_va_arg_int(ap)):(long)Cyc_va_arg_int(ap)));_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 457
if((long)_ulong < 0){
# 458
_ulong=- _ulong;
# 459
sign='-';}
# 461
base=Cyc_DEC;
# 462
goto number;case 'e': _LL4E:
# 463
 goto _LL4F;case 'E': _LL4F: goto _LL50;case 'f': _LL50: goto _LL51;case 'F': _LL51: goto _LL52;case 'g': _LL52:
# 464
 goto _LL53;case 'G': _LL53:
# 465
 _double=Cyc_va_arg_double(ap);_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 470
if(prec > 39){
# 471
if(ch != 'g'  && ch != 'G'  || flags & 8)
# 472
fpprec=prec - 39;
# 473
prec=39;}else{
# 474
if(prec == - 1)
# 475
prec=6;}
# 482
{char*_tmpE3;cp=((_tmpE3=buf,_tag_dyneither(_tmpE3,sizeof(char),_get_zero_arr_size_char((void*)_tmpE3,349))));}
# 483
{char _tmpE6;char _tmpE5;struct _dyneither_ptr _tmpE4;(_tmpE4=cp,((_tmpE5=*((char*)_check_dyneither_subscript(_tmpE4,sizeof(char),0)),((_tmpE6='\000',((_get_dyneither_size(_tmpE4,sizeof(char))== 1  && (_tmpE5 == '\000'  && _tmpE6 != '\000')?_throw_arraybounds(): 1,*((char*)_tmpE4.curr)=_tmpE6)))))));}
# 484
{char*_tmpE7;size=Cyc___cvt_double(_double,prec,flags,& softsign,ch,cp,
# 486
_dyneither_ptr_plus(((_tmpE7=buf,_tag_dyneither(_tmpE7,sizeof(char),_get_zero_arr_size_char((void*)_tmpE7,349)))),sizeof(char),(int)(sizeof(buf)- 1)));}
# 487
if(softsign)
# 488
sign='-';
# 489
if(*((char*)_check_dyneither_subscript(cp,sizeof(char),0))== '\000')
# 490
_dyneither_ptr_inplace_plus(& cp,sizeof(char),1);
# 491
break;case 'n': _LL54:
# 493
 if(flags & 1)
# 494
*Cyc_va_arg_int_ptr(ap)=(unsigned long)ret;else{
# 495
if(flags & 4)
# 496
*Cyc_va_arg_short_ptr(ap)=(short)ret;else{
# 498
*Cyc_va_arg_int_ptr(ap)=(unsigned long)ret;}}
# 499
_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 500
continue;case 'O': _LL55:
# 502
 flags |=1;
# 503
goto _LL56;case 'o': _LL56:
# 505
 _ulong=flags & 1?Cyc_va_arg_ulong(ap):(flags & 4?(unsigned long)((unsigned short)Cyc_va_arg_int(ap)):(unsigned long)Cyc_va_arg_uint(ap));_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 506
base=Cyc_OCT;
# 507
goto nosign;case 'p': _LL57:
# 517 "printf.cyc"
 _ulong=(unsigned long)Cyc_va_arg_long(ap);_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 518
base=Cyc_HEX;
# 519
flags |=64;
# 520
ch=(int)'x';
# 521
goto nosign;case 's': _LL58: {
# 523
struct _dyneither_ptr b=Cyc_va_arg_string(ap);_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 524
which_cp=3;cp3=b;
# 525
if(prec >= 0){
# 526
struct _dyneither_ptr p=Cyc_my_nzmemchr(cp3,'\000',prec);
# 527
if((char*)p.curr != (char*)(_tag_dyneither(0,0,0)).curr){
# 528
size=(p.curr - cp3.curr)/ sizeof(char);
# 529
if(size > prec)
# 530
size=prec;}else{
# 532
size=prec;}}else{
# 534
size=(int)Cyc_my_strlen(cp3);}
# 535
sign='\000';
# 536
break;}case 'U': _LL59:
# 538
 flags |=1;
# 539
goto _LL5A;case 'u': _LL5A:
# 541
 _ulong=flags & 1?Cyc_va_arg_ulong(ap):(flags & 4?(unsigned long)((unsigned short)Cyc_va_arg_int(ap)):(unsigned long)Cyc_va_arg_uint(ap));_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 542
base=Cyc_DEC;
# 543
goto nosign;case 'X': _LL5B:
# 544
 goto _LL5C;case 'x': _LL5C:
# 545
 _ulong=flags & 1?Cyc_va_arg_ulong(ap):(flags & 4?(unsigned long)((unsigned short)Cyc_va_arg_int(ap)):(unsigned long)Cyc_va_arg_uint(ap));_dyneither_ptr_inplace_plus(& ap,sizeof(void*),1);
# 546
base=Cyc_HEX;
# 548
if(flags & 8  && _ulong != 0)
# 549
flags |=64;
# 552
nosign: sign='\000';
# 558
number: if((dprec=prec)>= 0)
# 559
flags &=~ 32;
# 566
{char*_tmpE8;cp=_dyneither_ptr_plus(((_tmpE8=buf,_tag_dyneither(_tmpE8,sizeof(char),_get_zero_arr_size_char((void*)_tmpE8,349)))),sizeof(char),(308 + 39)+ 1);}
# 567
if(_ulong != 0  || prec != 0){
# 568
struct _dyneither_ptr xdigs;
# 574
switch(base){case Cyc_OCT: _LL5E:
# 576
 do{
# 577
{char _tmpEB;char _tmpEA;struct _dyneither_ptr _tmpE9;(_tmpE9=_dyneither_ptr_inplace_plus(& cp,sizeof(char),-1),((_tmpEA=*((char*)_check_dyneither_subscript(_tmpE9,sizeof(char),0)),((_tmpEB=(char)((_ulong & 7)+ '0'),((_get_dyneither_size(_tmpE9,sizeof(char))== 1  && (_tmpEA == '\000'  && _tmpEB != '\000')?_throw_arraybounds(): 1,*((char*)_tmpE9.curr)=_tmpEB)))))));}
# 578
_ulong >>=3;}while((int)_ulong);
# 581
if(flags & 8  && *((char*)_check_dyneither_subscript(cp,sizeof(char),0))!= '0'){
# 582
char _tmpEE;char _tmpED;struct _dyneither_ptr _tmpEC;(_tmpEC=_dyneither_ptr_inplace_plus(& cp,sizeof(char),-1),((_tmpED=*((char*)_check_dyneither_subscript(_tmpEC,sizeof(char),0)),((_tmpEE='0',((_get_dyneither_size(_tmpEC,sizeof(char))== 1  && (_tmpED == '\000'  && _tmpEE != '\000')?_throw_arraybounds(): 1,*((char*)_tmpEC.curr)=_tmpEE)))))));}
# 583
break;case Cyc_DEC: _LL5F:
# 587
 while(_ulong >= 10){
# 588
{char _tmpF1;char _tmpF0;struct _dyneither_ptr _tmpEF;(_tmpEF=_dyneither_ptr_inplace_plus(& cp,sizeof(char),-1),((_tmpF0=*((char*)_check_dyneither_subscript(_tmpEF,sizeof(char),0)),((_tmpF1=(char)(_ulong % 10 + '0'),((_get_dyneither_size(_tmpEF,sizeof(char))== 1  && (_tmpF0 == '\000'  && _tmpF1 != '\000')?_throw_arraybounds(): 1,*((char*)_tmpEF.curr)=_tmpF1)))))));}
# 589
_ulong /=10;}
# 591
{char _tmpF4;char _tmpF3;struct _dyneither_ptr _tmpF2;(_tmpF2=_dyneither_ptr_inplace_plus(& cp,sizeof(char),-1),((_tmpF3=*((char*)_check_dyneither_subscript(_tmpF2,sizeof(char),0)),((_tmpF4=(char)(_ulong + '0'),((_get_dyneither_size(_tmpF2,sizeof(char))== 1  && (_tmpF3 == '\000'  && _tmpF4 != '\000')?_throw_arraybounds(): 1,*((char*)_tmpF2.curr)=_tmpF4)))))));}
# 592
break;case Cyc_HEX: _LL60:
# 595
 if(ch == 'X'){
# 596
const char*_tmpF5;xdigs=((_tmpF5="0123456789ABCDEF",_tag_dyneither(_tmpF5,sizeof(char),17)));}else{
# 598
const char*_tmpF6;xdigs=((_tmpF6="0123456789abcdef",_tag_dyneither(_tmpF6,sizeof(char),17)));}
# 599
do{
# 600
{char _tmpF9;char _tmpF8;struct _dyneither_ptr _tmpF7;(_tmpF7=_dyneither_ptr_inplace_plus(& cp,sizeof(char),-1),((_tmpF8=*((char*)_check_dyneither_subscript(_tmpF7,sizeof(char),0)),((_tmpF9=*((const char*)_check_dyneither_subscript(xdigs,sizeof(char),(int)(_ulong & 15))),((_get_dyneither_size(_tmpF7,sizeof(char))== 1  && (_tmpF8 == '\000'  && _tmpF9 != '\000')?_throw_arraybounds(): 1,*((char*)_tmpF7.curr)=_tmpF9)))))));}
# 601
_ulong >>=4;}while((int)_ulong);
# 603
break;}}
# 611
{char*_tmpFA;size=((_dyneither_ptr_plus(((_tmpFA=buf,_tag_dyneither(_tmpFA,sizeof(char),_get_zero_arr_size_char((void*)_tmpFA,349)))),sizeof(char),(308 + 39)+ 1)).curr - cp.curr)/ sizeof(char);}
# 612
skipsize:
# 613
 break;default: _LL5D:
# 615
 if(ch == '\000')
# 616
goto done;
# 618
{char*_tmpFB;cp=((_tmpFB=buf,_tag_dyneither(_tmpFB,sizeof(char),_get_zero_arr_size_char((void*)_tmpFB,349))));}
# 619
{char _tmpFE;char _tmpFD;struct _dyneither_ptr _tmpFC;(_tmpFC=cp,((_tmpFD=*((char*)_check_dyneither_subscript(_tmpFC,sizeof(char),0)),((_tmpFE=(char)ch,((_get_dyneither_size(_tmpFC,sizeof(char))== 1  && (_tmpFD == '\000'  && _tmpFE != '\000')?_throw_arraybounds(): 1,*((char*)_tmpFC.curr)=_tmpFE)))))));}
# 620
size=1;
# 621
sign='\000';
# 622
break;}
# 647 "printf.cyc"
fieldsz=size + fpprec;
# 648
dpad=dprec - size;
# 649
if(dpad < 0)
# 650
dpad=0;
# 652
if((int)sign)
# 653
++ fieldsz;else{
# 654
if(flags & 64)
# 655
fieldsz +=2;}
# 656
fieldsz +=dpad;
# 659
if((flags & (16 | 32))== 0){
# 660
if(Cyc__IO_padn(ioputc,ioputc_env,' ',width - fieldsz)< width - fieldsz)goto error;}
# 663
if((int)sign){
# 664
{char _tmp103;char _tmp102;struct _dyneither_ptr _tmp101;char*_tmp100;(_tmp101=_dyneither_ptr_plus(((_tmp100=sign_string,_tag_dyneither(_tmp100,sizeof(char),_get_zero_arr_size_char((void*)_tmp100,2)))),sizeof(char),0),((_tmp102=*((char*)_check_dyneither_subscript(_tmp101,sizeof(char),0)),((_tmp103=sign,((_get_dyneither_size(_tmp101,sizeof(char))== 1  && (_tmp102 == '\000'  && _tmp103 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp101.curr)=_tmp103)))))));}
# 665
do{char*_tmp104;if(Cyc__IO_sputn(ioputc,ioputc_env,(struct _dyneither_ptr)((_tmp104=sign_string,_tag_dyneither(_tmp104,sizeof(char),_get_zero_arr_size_char((void*)_tmp104,2)))),1)!= 1)goto error;}while(0);}else{
# 666
if(flags & 64){
# 667
ox[0]='0';
# 668
ox[1]=(char)ch;
# 669
do{if(Cyc__IO_nzsputn(ioputc,ioputc_env,_tag_dyneither(ox,sizeof(char),2),2)!= 2)goto error;}while(0);}}
# 673
if((flags & (16 | 32))== 32){
# 674
if(Cyc__IO_padn(ioputc,ioputc_env,'0',width - fieldsz)< width - fieldsz)goto error;}
# 677
if(Cyc__IO_padn(ioputc,ioputc_env,'0',dpad)< dpad)goto error;
# 680
if(which_cp == 0)
# 681
do{if(Cyc__IO_sputn(ioputc,ioputc_env,(struct _dyneither_ptr)cp,size)!= size)goto error;}while(0);else{
# 682
if(which_cp == 2)
# 683
do{if(Cyc__IO_sputn(ioputc,ioputc_env,(struct _dyneither_ptr)cp2,size)!= size)goto error;}while(0);else{
# 684
if(which_cp == 3)
# 685
do{if(Cyc__IO_nzsputn(ioputc,ioputc_env,(struct _dyneither_ptr)cp3,size)!= size)goto error;}while(0);}}
# 688
if(Cyc__IO_padn(ioputc,ioputc_env,'0',fpprec)< fpprec)goto error;
# 691
if(flags & 16){
# 692
if(Cyc__IO_padn(ioputc,ioputc_env,' ',width - fieldsz)< width - fieldsz)goto error;}
# 695
ret +=width > fieldsz?width: fieldsz;};}
# 698
done:
# 699
 return ret;
# 700
error:
# 701
 return - 1;};}
# 705
static struct _dyneither_ptr Cyc_exponent(struct _dyneither_ptr p,int exp,int fmtch){
# 707
struct _dyneither_ptr t;
# 708
char expbuffer[309];
# 709
{unsigned int _tmp72=308;unsigned int i;for(i=0;i < _tmp72;i ++){expbuffer[i]='0';}expbuffer[_tmp72]=(char)0;}{char*_tmp105;struct _dyneither_ptr expbuf=(_tmp105=(char*)expbuffer,_tag_dyneither(_tmp105,sizeof(char),_get_zero_arr_size_char((void*)_tmp105,309)));
# 710
{char _tmp108;char _tmp107;struct _dyneither_ptr _tmp106;(_tmp106=_dyneither_ptr_inplace_plus_post(& p,sizeof(char),1),((_tmp107=*((char*)_check_dyneither_subscript(_tmp106,sizeof(char),0)),((_tmp108=(char)fmtch,((_get_dyneither_size(_tmp106,sizeof(char))== 1  && (_tmp107 == '\000'  && _tmp108 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp106.curr)=_tmp108)))))));}
# 711
if(exp < 0){
# 712
exp=- exp;{
# 713
char _tmp10B;char _tmp10A;struct _dyneither_ptr _tmp109;(_tmp109=_dyneither_ptr_inplace_plus_post(& p,sizeof(char),1),((_tmp10A=*((char*)_check_dyneither_subscript(_tmp109,sizeof(char),0)),((_tmp10B='-',((_get_dyneither_size(_tmp109,sizeof(char))== 1  && (_tmp10A == '\000'  && _tmp10B != '\000')?_throw_arraybounds(): 1,*((char*)_tmp109.curr)=_tmp10B)))))));};}else{
# 716
char _tmp10E;char _tmp10D;struct _dyneither_ptr _tmp10C;(_tmp10C=_dyneither_ptr_inplace_plus_post(& p,sizeof(char),1),((_tmp10D=*((char*)_check_dyneither_subscript(_tmp10C,sizeof(char),0)),((_tmp10E='+',((_get_dyneither_size(_tmp10C,sizeof(char))== 1  && (_tmp10D == '\000'  && _tmp10E != '\000')?_throw_arraybounds(): 1,*((char*)_tmp10C.curr)=_tmp10E)))))));}
# 717
t=_dyneither_ptr_plus(expbuf,sizeof(char),308);
# 718
if(exp > 9){
# 719
do{
# 720
char _tmp111;char _tmp110;struct _dyneither_ptr _tmp10F;(_tmp10F=_dyneither_ptr_inplace_plus(& t,sizeof(char),-1),((_tmp110=*((char*)_check_dyneither_subscript(_tmp10F,sizeof(char),0)),((_tmp111=(char)(exp % 10 + '0'),((_get_dyneither_size(_tmp10F,sizeof(char))== 1  && (_tmp110 == '\000'  && _tmp111 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp10F.curr)=_tmp111)))))));}while((exp /=10)> 9);
# 722
{char _tmp114;char _tmp113;struct _dyneither_ptr _tmp112;(_tmp112=_dyneither_ptr_inplace_plus(& t,sizeof(char),-1),((_tmp113=*((char*)_check_dyneither_subscript(_tmp112,sizeof(char),0)),((_tmp114=(char)(exp + '0'),((_get_dyneither_size(_tmp112,sizeof(char))== 1  && (_tmp113 == '\000'  && _tmp114 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp112.curr)=_tmp114)))))));}{
# 723
char _tmp117;char _tmp116;struct _dyneither_ptr _tmp115;for(0;(char*)t.curr < (char*)(_dyneither_ptr_plus(expbuf,sizeof(char),308)).curr;(_tmp115=_dyneither_ptr_inplace_plus_post(& p,sizeof(char),1),((_tmp116=*((char*)_check_dyneither_subscript(_tmp115,sizeof(char),0)),((_tmp117=*((char*)_check_dyneither_subscript(_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),sizeof(char),0)),((_get_dyneither_size(_tmp115,sizeof(char))== 1  && (_tmp116 == '\000'  && _tmp117 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp115.curr)=_tmp117)))))))){;}};}else{
# 726
{char _tmp11A;char _tmp119;struct _dyneither_ptr _tmp118;(_tmp118=_dyneither_ptr_inplace_plus_post(& p,sizeof(char),1),((_tmp119=*((char*)_check_dyneither_subscript(_tmp118,sizeof(char),0)),((_tmp11A='0',((_get_dyneither_size(_tmp118,sizeof(char))== 1  && (_tmp119 == '\000'  && _tmp11A != '\000')?_throw_arraybounds(): 1,*((char*)_tmp118.curr)=_tmp11A)))))));}{
# 727
char _tmp11D;char _tmp11C;struct _dyneither_ptr _tmp11B;(_tmp11B=_dyneither_ptr_inplace_plus_post(& p,sizeof(char),1),((_tmp11C=*((char*)_check_dyneither_subscript(_tmp11B,sizeof(char),0)),((_tmp11D=(char)(exp + '0'),((_get_dyneither_size(_tmp11B,sizeof(char))== 1  && (_tmp11C == '\000'  && _tmp11D != '\000')?_throw_arraybounds(): 1,*((char*)_tmp11B.curr)=_tmp11D)))))));};}
# 729
return p;};}
# 732
static struct _dyneither_ptr Cyc_sround(double fract,int*exp,struct _dyneither_ptr start,struct _dyneither_ptr end,char ch,int*signp){
# 736
double tmp=0.0;
# 738
if(fract != 0.0)
# 739
modf(fract * 10,& tmp);else{
# 741
tmp=(double)(ch - '0');}
# 742
if(tmp > 4)
# 743
for(0;1;_dyneither_ptr_inplace_plus(& end,sizeof(char),-1)){
# 744
if(*((char*)_check_dyneither_subscript(end,sizeof(char),0))== '.')
# 745
_dyneither_ptr_inplace_plus(& end,sizeof(char),-1);
# 746
{char _tmp120;char _tmp11F;struct _dyneither_ptr _tmp11E;if(((_tmp11E=end,((_tmp11F=*((char*)_check_dyneither_subscript(_tmp11E,sizeof(char),0)),((_tmp120=_tmp11F + '\001',((_get_dyneither_size(_tmp11E,sizeof(char))== 1  && (_tmp11F == '\000'  && _tmp120 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp11E.curr)=_tmp120))))))))<= '9')
# 747
break;}
# 748
{char _tmp123;char _tmp122;struct _dyneither_ptr _tmp121;(_tmp121=end,((_tmp122=*((char*)_check_dyneither_subscript(_tmp121,sizeof(char),0)),((_tmp123='0',((_get_dyneither_size(_tmp121,sizeof(char))== 1  && (_tmp122 == '\000'  && _tmp123 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp121.curr)=_tmp123)))))));}
# 749
if((char*)end.curr == (char*)start.curr){
# 750
if((unsigned int)exp){
# 751
{char _tmp126;char _tmp125;struct _dyneither_ptr _tmp124;(_tmp124=end,((_tmp125=*((char*)_check_dyneither_subscript(_tmp124,sizeof(char),0)),((_tmp126='1',((_get_dyneither_size(_tmp124,sizeof(char))== 1  && (_tmp125 == '\000'  && _tmp126 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp124.curr)=_tmp126)))))));}
# 752
++(*exp);}else{
# 755
{char _tmp129;char _tmp128;struct _dyneither_ptr _tmp127;(_tmp127=_dyneither_ptr_inplace_plus(& end,sizeof(char),-1),((_tmp128=*((char*)_check_dyneither_subscript(_tmp127,sizeof(char),0)),((_tmp129='1',((_get_dyneither_size(_tmp127,sizeof(char))== 1  && (_tmp128 == '\000'  && _tmp129 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp127.curr)=_tmp129)))))));}
# 756
_dyneither_ptr_inplace_plus(& start,sizeof(char),-1);}
# 758
break;}}else{
# 762
if(*signp == '-')
# 763
for(0;1;_dyneither_ptr_inplace_plus(& end,sizeof(char),-1)){
# 764
if(*((char*)_check_dyneither_subscript(end,sizeof(char),0))== '.')
# 765
_dyneither_ptr_inplace_plus(& end,sizeof(char),-1);
# 766
if(*((char*)_check_dyneither_subscript(end,sizeof(char),0))!= '0')
# 767
break;
# 768
if((char*)end.curr == (char*)start.curr)
# 769
*signp=0;}}
# 771
return start;}
# 774
int Cyc___cvt_double(double number,int prec,int flags,int*signp,int fmtch,struct _dyneither_ptr startp,struct _dyneither_ptr endp){
# 777
struct _dyneither_ptr p;struct _dyneither_ptr t;
# 778
register double fract;
# 779
int dotrim=0;int expcnt;int gformat=0;
# 780
double integer=0.0;double tmp=0.0;
# 782
expcnt=0;
# 783
if(number < 0){
# 784
number=- number;
# 785
*signp=(int)'-';}else{
# 787
*signp=0;}
# 789
fract=modf(number,& integer);
# 792
t=_dyneither_ptr_inplace_plus(& startp,sizeof(char),1);
# 798
for(p=_dyneither_ptr_plus(endp,sizeof(char),- 1);(char*)p.curr >= (char*)startp.curr  && integer != 0.0;++ expcnt){
# 799
tmp=modf(integer / 10,& integer);{
# 800
char _tmp12C;char _tmp12B;struct _dyneither_ptr _tmp12A;(_tmp12A=_dyneither_ptr_inplace_plus_post(& p,sizeof(char),-1),((_tmp12B=*((char*)_check_dyneither_subscript(_tmp12A,sizeof(char),0)),((_tmp12C=(char)((int)((tmp + .01)* 10)+ '0'),((_get_dyneither_size(_tmp12A,sizeof(char))== 1  && (_tmp12B == '\000'  && _tmp12C != '\000')?_throw_arraybounds(): 1,*((char*)_tmp12A.curr)=_tmp12C)))))));};}
# 802
switch(fmtch){case 'f': _LL63:
# 803
 goto _LL64;case 'F': _LL64:
# 805
 if(expcnt){
# 806
char _tmp12F;char _tmp12E;struct _dyneither_ptr _tmp12D;for(0;(char*)(_dyneither_ptr_inplace_plus(& p,sizeof(char),1)).curr < (char*)endp.curr;(_tmp12D=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp12E=*((char*)_check_dyneither_subscript(_tmp12D,sizeof(char),0)),((_tmp12F=*((char*)_check_dyneither_subscript(p,sizeof(char),0)),((_get_dyneither_size(_tmp12D,sizeof(char))== 1  && (_tmp12E == '\000'  && _tmp12F != '\000')?_throw_arraybounds(): 1,*((char*)_tmp12D.curr)=_tmp12F)))))))){;}}else{
# 808
char _tmp132;char _tmp131;struct _dyneither_ptr _tmp130;(_tmp130=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp131=*((char*)_check_dyneither_subscript(_tmp130,sizeof(char),0)),((_tmp132='0',((_get_dyneither_size(_tmp130,sizeof(char))== 1  && (_tmp131 == '\000'  && _tmp132 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp130.curr)=_tmp132)))))));}
# 813
if(prec  || flags & 8){
# 814
char _tmp135;char _tmp134;struct _dyneither_ptr _tmp133;(_tmp133=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp134=*((char*)_check_dyneither_subscript(_tmp133,sizeof(char),0)),((_tmp135='.',((_get_dyneither_size(_tmp133,sizeof(char))== 1  && (_tmp134 == '\000'  && _tmp135 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp133.curr)=_tmp135)))))));}
# 816
if(fract != 0.0){
# 817
if(prec)
# 818
do{
# 819
fract=modf(fract * 10,& tmp);{
# 820
char _tmp138;char _tmp137;struct _dyneither_ptr _tmp136;(_tmp136=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp137=*((char*)_check_dyneither_subscript(_tmp136,sizeof(char),0)),((_tmp138=(char)((int)tmp + '0'),((_get_dyneither_size(_tmp136,sizeof(char))== 1  && (_tmp137 == '\000'  && _tmp138 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp136.curr)=_tmp138)))))));};}while(
# 821
-- prec  && fract != 0.0);
# 822
if(fract != 0.0)
# 823
startp=Cyc_sround(fract,0,startp,
# 824
_dyneither_ptr_plus(t,sizeof(char),- 1),(char)'\000',signp);}
# 826
{char _tmp13B;char _tmp13A;struct _dyneither_ptr _tmp139;for(0;prec --;(_tmp139=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp13A=*((char*)_check_dyneither_subscript(_tmp139,sizeof(char),0)),((_tmp13B='0',((_get_dyneither_size(_tmp139,sizeof(char))== 1  && (_tmp13A == '\000'  && _tmp13B != '\000')?_throw_arraybounds(): 1,*((char*)_tmp139.curr)=_tmp13B)))))))){;}}
# 827
break;case 'e': _LL65:
# 828
 goto _LL66;case 'E': _LL66:
# 829
 eformat: if(expcnt){
# 830
{char _tmp13E;char _tmp13D;struct _dyneither_ptr _tmp13C;(_tmp13C=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp13D=*((char*)_check_dyneither_subscript(_tmp13C,sizeof(char),0)),((_tmp13E=*((char*)_check_dyneither_subscript(_dyneither_ptr_inplace_plus(& p,sizeof(char),1),sizeof(char),0)),((_get_dyneither_size(_tmp13C,sizeof(char))== 1  && (_tmp13D == '\000'  && _tmp13E != '\000')?_throw_arraybounds(): 1,*((char*)_tmp13C.curr)=_tmp13E)))))));}
# 831
if(prec  || flags & 8){
# 832
char _tmp141;char _tmp140;struct _dyneither_ptr _tmp13F;(_tmp13F=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp140=*((char*)_check_dyneither_subscript(_tmp13F,sizeof(char),0)),((_tmp141='.',((_get_dyneither_size(_tmp13F,sizeof(char))== 1  && (_tmp140 == '\000'  && _tmp141 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp13F.curr)=_tmp141)))))));}
# 834
for(0;prec  && (char*)(_dyneither_ptr_inplace_plus(& p,sizeof(char),1)).curr < (char*)endp.curr;-- prec){
# 835
char _tmp144;char _tmp143;struct _dyneither_ptr _tmp142;(_tmp142=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp143=*((char*)_check_dyneither_subscript(_tmp142,sizeof(char),0)),((_tmp144=*((char*)_check_dyneither_subscript(p,sizeof(char),0)),((_get_dyneither_size(_tmp142,sizeof(char))== 1  && (_tmp143 == '\000'  && _tmp144 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp142.curr)=_tmp144)))))));}
# 841
if(!prec  && (char*)(_dyneither_ptr_inplace_plus(& p,sizeof(char),1)).curr < (char*)endp.curr){
# 842
fract=(double)0;
# 843
startp=Cyc_sround((double)0,(int*)& expcnt,startp,
# 844
_dyneither_ptr_plus(t,sizeof(char),- 1),*((char*)_check_dyneither_subscript(p,sizeof(char),0)),signp);}
# 847
-- expcnt;}else{
# 850
if(fract != 0.0){
# 852
for(expcnt=- 1;1;-- expcnt){
# 853
fract=modf(fract * 10,& tmp);
# 854
if(tmp != 0.0)
# 855
break;}
# 857
{char _tmp147;char _tmp146;struct _dyneither_ptr _tmp145;(_tmp145=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp146=*((char*)_check_dyneither_subscript(_tmp145,sizeof(char),0)),((_tmp147=(char)((int)tmp + '0'),((_get_dyneither_size(_tmp145,sizeof(char))== 1  && (_tmp146 == '\000'  && _tmp147 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp145.curr)=_tmp147)))))));}
# 858
if(prec  || flags & 8){
# 859
char _tmp14A;char _tmp149;struct _dyneither_ptr _tmp148;(_tmp148=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp149=*((char*)_check_dyneither_subscript(_tmp148,sizeof(char),0)),((_tmp14A='.',((_get_dyneither_size(_tmp148,sizeof(char))== 1  && (_tmp149 == '\000'  && _tmp14A != '\000')?_throw_arraybounds(): 1,*((char*)_tmp148.curr)=_tmp14A)))))));}}else{
# 862
{char _tmp14D;char _tmp14C;struct _dyneither_ptr _tmp14B;(_tmp14B=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp14C=*((char*)_check_dyneither_subscript(_tmp14B,sizeof(char),0)),((_tmp14D='0',((_get_dyneither_size(_tmp14B,sizeof(char))== 1  && (_tmp14C == '\000'  && _tmp14D != '\000')?_throw_arraybounds(): 1,*((char*)_tmp14B.curr)=_tmp14D)))))));}
# 863
if(prec  || flags & 8){
# 864
char _tmp150;char _tmp14F;struct _dyneither_ptr _tmp14E;(_tmp14E=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp14F=*((char*)_check_dyneither_subscript(_tmp14E,sizeof(char),0)),((_tmp150='.',((_get_dyneither_size(_tmp14E,sizeof(char))== 1  && (_tmp14F == '\000'  && _tmp150 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp14E.curr)=_tmp150)))))));}}}
# 867
if(fract != 0.0){
# 868
if(prec)
# 869
do{
# 870
fract=modf(fract * 10,& tmp);{
# 871
char _tmp153;char _tmp152;struct _dyneither_ptr _tmp151;(_tmp151=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp152=*((char*)_check_dyneither_subscript(_tmp151,sizeof(char),0)),((_tmp153=(char)((int)tmp + '0'),((_get_dyneither_size(_tmp151,sizeof(char))== 1  && (_tmp152 == '\000'  && _tmp153 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp151.curr)=_tmp153)))))));};}while(
# 872
-- prec  && fract != 0.0);
# 873
if(fract != 0.0)
# 874
startp=Cyc_sround(fract,(int*)& expcnt,startp,
# 875
_dyneither_ptr_plus(t,sizeof(char),- 1),(char)'\000',signp);}
# 878
{char _tmp156;char _tmp155;struct _dyneither_ptr _tmp154;for(0;prec --;(_tmp154=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp155=*((char*)_check_dyneither_subscript(_tmp154,sizeof(char),0)),((_tmp156='0',((_get_dyneither_size(_tmp154,sizeof(char))== 1  && (_tmp155 == '\000'  && _tmp156 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp154.curr)=_tmp156)))))))){;}}
# 881
if(gformat  && !(flags & 8)){
# 882
while((char*)t.curr > (char*)startp.curr  && *((char*)_check_dyneither_subscript(_dyneither_ptr_inplace_plus(& t,sizeof(char),-1),sizeof(char),0))== '0'){;}
# 883
if(*((char*)_check_dyneither_subscript(t,sizeof(char),0))== '.')
# 884
_dyneither_ptr_inplace_plus(& t,sizeof(char),-1);
# 885
_dyneither_ptr_inplace_plus(& t,sizeof(char),1);}
# 887
t=Cyc_exponent(t,expcnt,fmtch);
# 888
break;case 'g': _LL67:
# 889
 goto _LL68;case 'G': _LL68:
# 891
 if(!prec)
# 892
++ prec;
# 899
if(expcnt > prec  || (!expcnt  && fract != 0.0) && fract < .0001){
# 907
-- prec;
# 908
fmtch -=2;
# 909
gformat=1;
# 910
goto eformat;}
# 916
if(expcnt){
# 917
char _tmp159;char _tmp158;struct _dyneither_ptr _tmp157;for(0;(char*)(_dyneither_ptr_inplace_plus(& p,sizeof(char),1)).curr < (char*)endp.curr;(((_tmp157=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp158=*((char*)_check_dyneither_subscript(_tmp157,sizeof(char),0)),((_tmp159=*((char*)_check_dyneither_subscript(p,sizeof(char),0)),((_get_dyneither_size(_tmp157,sizeof(char))== 1  && (_tmp158 == '\000'  && _tmp159 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp157.curr)=_tmp159)))))))),-- prec)){;}}else{
# 919
char _tmp15C;char _tmp15B;struct _dyneither_ptr _tmp15A;(_tmp15A=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp15B=*((char*)_check_dyneither_subscript(_tmp15A,sizeof(char),0)),((_tmp15C='0',((_get_dyneither_size(_tmp15A,sizeof(char))== 1  && (_tmp15B == '\000'  && _tmp15C != '\000')?_throw_arraybounds(): 1,*((char*)_tmp15A.curr)=_tmp15C)))))));}
# 924
if(prec  || flags & 8){
# 925
dotrim=1;{
# 926
char _tmp15F;char _tmp15E;struct _dyneither_ptr _tmp15D;(_tmp15D=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp15E=*((char*)_check_dyneither_subscript(_tmp15D,sizeof(char),0)),((_tmp15F='.',((_get_dyneither_size(_tmp15D,sizeof(char))== 1  && (_tmp15E == '\000'  && _tmp15F != '\000')?_throw_arraybounds(): 1,*((char*)_tmp15D.curr)=_tmp15F)))))));};}else{
# 929
dotrim=0;}
# 931
if(fract != 0.0){
# 932
if(prec){
# 935
do{
# 936
fract=modf(fract * 10,& tmp);{
# 937
char _tmp162;char _tmp161;struct _dyneither_ptr _tmp160;(_tmp160=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp161=*((char*)_check_dyneither_subscript(_tmp160,sizeof(char),0)),((_tmp162=(char)((int)tmp + '0'),((_get_dyneither_size(_tmp160,sizeof(char))== 1  && (_tmp161 == '\000'  && _tmp162 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp160.curr)=_tmp162)))))));};}while(
# 938
tmp == 0.0  && !expcnt);
# 939
while(-- prec  && fract != 0.0){
# 940
fract=modf(fract * 10,& tmp);{
# 941
char _tmp165;char _tmp164;struct _dyneither_ptr _tmp163;(_tmp163=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp164=*((char*)_check_dyneither_subscript(_tmp163,sizeof(char),0)),((_tmp165=(char)((int)tmp + '0'),((_get_dyneither_size(_tmp163,sizeof(char))== 1  && (_tmp164 == '\000'  && _tmp165 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp163.curr)=_tmp165)))))));};}}
# 944
if(fract != 0.0)
# 945
startp=Cyc_sround(fract,0,startp,
# 946
_dyneither_ptr_plus(t,sizeof(char),- 1),(char)'\000',signp);}
# 949
if(flags & 8){
# 950
char _tmp168;char _tmp167;struct _dyneither_ptr _tmp166;for(0;prec --;(_tmp166=_dyneither_ptr_inplace_plus_post(& t,sizeof(char),1),((_tmp167=*((char*)_check_dyneither_subscript(_tmp166,sizeof(char),0)),((_tmp168='0',((_get_dyneither_size(_tmp166,sizeof(char))== 1  && (_tmp167 == '\000'  && _tmp168 != '\000')?_throw_arraybounds(): 1,*((char*)_tmp166.curr)=_tmp168)))))))){;}}else{
# 951
if(dotrim){
# 952
while((char*)t.curr > (char*)startp.curr  && *((char*)_check_dyneither_subscript(_dyneither_ptr_inplace_plus(& t,sizeof(char),-1),sizeof(char),0))== '0'){;}
# 953
if(*((char*)_check_dyneither_subscript(t,sizeof(char),0))!= '.')
# 954
_dyneither_ptr_inplace_plus(& t,sizeof(char),1);}}
# 956
break;default: _LL69: {
# 957
struct Cyc_Core_Impossible_exn_struct _tmp16E;const char*_tmp16D;struct Cyc_Core_Impossible_exn_struct*_tmp16C;(int)_throw((void*)((_tmp16C=_cycalloc(sizeof(*_tmp16C)),((_tmp16C[0]=((_tmp16E.tag=Cyc_Core_Impossible,((_tmp16E.f1=((_tmp16D="__cvt_double",_tag_dyneither(_tmp16D,sizeof(char),13))),_tmp16E)))),_tmp16C)))));}}
# 959
return(t.curr - startp.curr)/ sizeof(char);}
# 963
int Cyc_vfprintf(struct Cyc___cycFILE*f,struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 966
int ans;
# 967
ans=((int(*)(int(*ioputc)(int,struct Cyc___cycFILE*),struct Cyc___cycFILE*ioputc_env,struct _dyneither_ptr fmt0,struct _dyneither_ptr ap))Cyc__IO_vfprintf)(Cyc_putc,f,fmt,ap);
# 968
return ans;}
# 971
int Cyc_fprintf(struct Cyc___cycFILE*f,struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 975
return Cyc_vfprintf(f,fmt,ap);}
# 978
int Cyc_vprintf(struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 981
int ans;
# 982
ans=((int(*)(int(*ioputc)(int,struct Cyc___cycFILE*),struct Cyc___cycFILE*ioputc_env,struct _dyneither_ptr fmt0,struct _dyneither_ptr ap))Cyc__IO_vfprintf)(Cyc_putc,Cyc_stdout,fmt,ap);
# 983
return ans;}
# 986
int Cyc_printf(struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 989
int ans;
# 990
ans=Cyc_vprintf(fmt,ap);
# 991
return ans;}struct _tuple0{struct _dyneither_ptr*f1;unsigned long*f2;};
# 994
static int Cyc_putc_string(int c,struct _tuple0*sptr_n){
# 995
struct _tuple0 _tmpC2;struct _dyneither_ptr*_tmpC3;unsigned long*_tmpC4;struct _tuple0*_tmpC1=sptr_n;_tmpC2=*_tmpC1;_tmpC3=_tmpC2.f1;_tmpC4=_tmpC2.f2;{
# 996
struct _dyneither_ptr s=*_tmpC3;
# 997
unsigned long n=*_tmpC4;
# 998
if(n == 0)return - 1;
# 999
*((char*)_check_dyneither_subscript(s,sizeof(char),0))=(char)c;
# 1000
_dyneither_ptr_inplace_plus(_tmpC3,sizeof(char),1);
# 1001
*_tmpC4=n - 1;
# 1002
return 1;};}
# 1005
int Cyc_vsnprintf(struct _dyneither_ptr s,unsigned long n,struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 1008
int ans;
# 1009
struct _dyneither_ptr _tmpC5=s;
# 1010
unsigned long _tmpC6=n;
# 1011
struct _tuple0 _tmp16F;struct _tuple0 _tmpC7=(_tmp16F.f1=& _tmpC5,((_tmp16F.f2=& _tmpC6,_tmp16F)));
# 1012
ans=((int(*)(int(*ioputc)(int,struct _tuple0*),struct _tuple0*ioputc_env,struct _dyneither_ptr fmt0,struct _dyneither_ptr ap))Cyc__IO_vfprintf)(Cyc_putc_string,& _tmpC7,fmt,ap);
# 1013
if(0 <= ans)
# 1014
*((char*)_check_dyneither_subscript(s,sizeof(char),ans))='\000';
# 1015
return ans;}
# 1018
int Cyc_snprintf(struct _dyneither_ptr s,unsigned long n,struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 1021
return Cyc_vsnprintf(s,n,fmt,ap);}
# 1024
int Cyc_vsprintf(struct _dyneither_ptr s,struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 1027
return Cyc_vsnprintf(s,_get_dyneither_size(s,sizeof(char)),fmt,ap);}
# 1030
int Cyc_sprintf(struct _dyneither_ptr s,struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 1033
return Cyc_vsnprintf(s,_get_dyneither_size(s,sizeof(char)),fmt,ap);}
# 1036
static int Cyc_putc_void(int c,int dummy){
# 1037
return 1;}
# 1040
struct _dyneither_ptr Cyc_vrprintf(struct _RegionHandle*r1,struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 1044
int size=((int(*)(int(*ioputc)(int,int),int ioputc_env,struct _dyneither_ptr fmt0,struct _dyneither_ptr ap))Cyc__IO_vfprintf)(Cyc_putc_void,0,fmt,ap)+ 1;
# 1045
char*_tmp171;unsigned int _tmp170;struct _dyneither_ptr s=(struct _dyneither_ptr)((_tmp170=size + 1,((_tmp171=_cyccalloc_atomic(sizeof(char),_tmp170),_tag_dyneither(_tmp171,sizeof(char),_tmp170)))));
# 1046
Cyc_vsprintf(_dyneither_ptr_decrease_size(s,sizeof(char),1),fmt,ap);
# 1047
return s;}
# 1050
struct _dyneither_ptr Cyc_rprintf(struct _RegionHandle*r1,struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 1053
return Cyc_vrprintf(r1,fmt,ap);}
# 1056
struct _dyneither_ptr Cyc_aprintf(struct _dyneither_ptr fmt,struct _dyneither_ptr ap){
# 1059
return Cyc_vrprintf(Cyc_Core_heap_region,fmt,ap);}
