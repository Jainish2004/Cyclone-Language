#ifndef _SETJMP_H_
#define _SETJMP_H_
#ifndef _jmp_buf_def_
#define _jmp_buf_def_
typedef int jmp_buf[192];
#endif
extern int setjmp(jmp_buf);
#endif
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
struct _dynforward_ptr {
  unsigned char *curr;
  unsigned char *last_plus_one;
};

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
extern int _throw_null();
extern int _throw_arraybounds();
extern int _throw_badalloc();
extern int _throw(void* e);
#endif

extern struct _xtunion_struct *_exn_thrown;

/* Built-in Exceptions */
extern struct _xtunion_struct ADD_PREFIX(Null_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Null_Exception);
extern struct _xtunion_struct ADD_PREFIX(Array_bounds_struct);
extern struct _xtunion_struct * ADD_PREFIX(Array_bounds);
extern struct _xtunion_struct ADD_PREFIX(Match_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Match_Exception);
extern struct _xtunion_struct ADD_PREFIX(Bad_alloc_struct);
extern struct _xtunion_struct * ADD_PREFIX(Bad_alloc);

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
_check_null(void *ptr) {
  void*_check_null_temp = (void*)(ptr);
  if (!_check_null_temp) _throw_null();
  return _check_null_temp;
}
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
_check_known_subscript_null(void *ptr, unsigned bound, unsigned elt_sz, unsigned index) {
  void*_cks_ptr = (void*)(ptr);
  unsigned _cks_bound = (bound);
  unsigned _cks_elt_sz = (elt_sz);
  unsigned _cks_index = (index);
  if (!_cks_ptr) _throw_null();
  if (_cks_index >= _cks_bound) _throw_arraybounds();
  return ((char *)_cks_ptr) + _cks_elt_sz*_cks_index;
}
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
_check_known_subscript_notnull(unsigned bound,unsigned index) { 
  unsigned _cksnn_bound = (bound); 
  unsigned _cksnn_index = (index); 
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); 
  return _cksnn_index;
}
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
#define _zero_arr_plus_char(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_short(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_int(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_float(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_double(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_longdouble(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#define _zero_arr_plus_voidstar(orig_x,orig_sz,orig_i) ((orig_x)+(orig_i))
#else
static _INLINE char *
_zero_arr_plus_char(char *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE short *
_zero_arr_plus_short(short *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE int *
_zero_arr_plus_int(int *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE float *
_zero_arr_plus_float(float *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE double *
_zero_arr_plus_double(double *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE long double *
_zero_arr_plus_longdouble(long double *orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
static _INLINE void *
_zero_arr_plus_voidstar(void **orig_x, int orig_sz, int orig_i) {
  unsigned int _czs_temp;
  if ((orig_x) == 0) _throw_null();
  if (orig_i < 0) _throw_arraybounds();
  for (_czs_temp=orig_sz; _czs_temp < orig_i; _czs_temp++)
    if (orig_x[_czs_temp] == 0) _throw_arraybounds();
  return orig_x + orig_i;
}
#endif


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
   Note that this expands to call _zero_arr_plus. */
/*#define _zero_arr_inplace_plus(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  *_zap_x = _zero_arr_plus(*_zap_x,1,(orig_i)); })
  */
static _INLINE void 
_zero_arr_inplace_plus_char(char *x, int orig_i) {
  char **_zap_x = &x;
  *_zap_x = _zero_arr_plus_char(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_short(short *x, int orig_i) {
  short **_zap_x = &x;
  *_zap_x = _zero_arr_plus_short(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_int(int *x, int orig_i) {
  int **_zap_x = &x;
  *_zap_x = _zero_arr_plus_int(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_float(float *x, int orig_i) {
  float **_zap_x = &x;
  *_zap_x = _zero_arr_plus_float(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_double(double *x, int orig_i) {
  double **_zap_x = &x;
  *_zap_x = _zero_arr_plus_double(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_longdouble(long double *x, int orig_i) {
  long double **_zap_x = &x;
  *_zap_x = _zero_arr_plus_longdouble(*_zap_x,1,orig_i);
}
static _INLINE void 
_zero_arr_inplace_plus_voidstar(void **x, int orig_i) {
  void ***_zap_x = &x;
  *_zap_x = _zero_arr_plus_voidstar(*_zap_x,1,orig_i);
}




/* Does in-place increment of a zero-terminated pointer (e.g., x++).
   Note that this expands to call _zero_arr_plus. */
/*#define _zero_arr_inplace_plus_post(x,orig_i) ({ \
  typedef _zap_tx = (*x); \
  _zap_tx **_zap_x = &((_zap_tx*)x); \
  _zap_tx *_zap_res = *_zap_x; \
  *_zap_x = _zero_arr_plus(_zap_res,1,(orig_i)); \
  _zap_res; })*/
  
static _INLINE char *
_zero_arr_inplace_plus_post_char(char *x, int orig_i){
  char ** _zap_x = &x;
  char * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_char(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE short *
_zero_arr_inplace_plus_post_short(short *x, int orig_i){
  short **_zap_x = &x;
  short * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_short(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE int *
_zero_arr_inplace_plus_post_int(int *x, int orig_i){
  int **_zap_x = &x;
  int * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_int(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE float *
_zero_arr_inplace_plus_post_float(float *x, int orig_i){
  float **_zap_x = &x;
  float * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_float(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE double *
_zero_arr_inplace_plus_post_double(double *x, int orig_i){
  double **_zap_x = &x;
  double * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_double(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE long double *
_zero_arr_inplace_plus_post_longdouble(long double *x, int orig_i){
  long double **_zap_x = &x;
  long double * _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_longdouble(_zap_res,1,orig_i);
  return _zap_res;
}
static _INLINE void **
_zero_arr_inplace_plus_post_voidstar(void **x, int orig_i){
  void ***_zap_x = &x;
  void ** _zap_res = *_zap_x;
  *_zap_x = _zero_arr_plus_voidstar(_zap_res,1,orig_i);
  return _zap_res;
}



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
static _INLINE unsigned char *
_check_dynforward_subscript(struct _dynforward_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dynforward_ptr _cus_arr = (arr);
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
#define _check_dynforward_subscript(arr,elt_sz,index) ({ \
  struct _dynforward_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
#endif
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  if (!_cus_arr.base) _throw_null();
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds();
  return _cus_ans;
}
static _INLINE unsigned char *
_check_dynforward_subscript(struct _dynforward_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dynforward_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_curr = _cus_arr.curr;
  unsigned char *_cus_ans = _cus_curr + _cus_elt_sz * _cus_index;
  if (!_cus_arr.last_plus_one) _throw_null();
  if (_cus_ans < _cus_curr || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds();
  return _cus_ans;
}
#else
#define _check_dyneither_subscript(arr,elt_sz,index) ({ \
  struct _dyneither_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  if (!_cus_arr.base) _throw_null(); \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#define _check_dynforward_subscript(arr,elt_sz,index) ({ \
  struct _dynforward_ptr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_curr = _cus_arr.curr; \
  unsigned char *_cus_ans = _cus_curr + _cus_elt_sz * _cus_index; \
  if (!_cus_arr.last_plus_one) _throw_null(); \
  if (_cus_ans < _cus_curr || _cus_ans >= _cus_arr.last_plus_one) \
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
static _INLINE struct _dynforward_ptr
_tag_dynforward(const void *tcurr,unsigned elt_sz,unsigned num_elts) {
  struct _dynforward_ptr _tag_arr_ans;
  _tag_arr_ans.curr = (void*)(tcurr);
  _tag_arr_ans.last_plus_one = _tag_arr_ans.curr + (elt_sz) * (num_elts);
  return _tag_arr_ans;
}
#else
#define _tag_dyneither(tcurr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })
#define _tag_dynforward(tcurr,elt_sz,num_elts) ({ \
  struct _dynforward_ptr _tag_arr_ans; \
  _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.curr + (elt_sz) * (num_elts); \
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
static _INLINE struct _dynforward_ptr *
_init_dynforward_ptr(struct _dynforward_ptr *arr_ptr,
                    void *arr, unsigned elt_sz, unsigned num_elts) {
  struct _dynforward_ptr *_itarr_ptr = (arr_ptr);
  void* _itarr = (arr);
  _itarr_ptr->curr = _itarr;
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
#define _init_dynforward_ptr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _dynforward_ptr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_dynforward_ptr(arr,elt_sz,num_elts) ((arr).curr)
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ((arr).curr)
#else
#ifdef _INLINE_FUNCTIONS
static _INLINE unsigned char *
_untag_dyneither_ptr(struct _dyneither_ptr arr, 
                     unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds();
  return _curr;
}
static _INLINE unsigned char *
_untag_dynforward_ptr(struct _dynforward_ptr arr, 
                      unsigned elt_sz,unsigned num_elts) {
  struct _dynforward_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
    _throw_arraybounds();
  return _curr;
}
#else
#define _untag_dyneither_ptr(arr,elt_sz,num_elts) ({ \
  struct _dyneither_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
    _throw_arraybounds(); \
  _curr; })
#define _untag_dynforward_ptr(arr,elt_sz,num_elts) ({ \
  struct _dynforward_ptr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
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
static _INLINE unsigned
_get_dynforward_size(struct _dynforward_ptr arr,unsigned elt_sz) {
  struct _dynforward_ptr _get_arr_size_temp = (arr);
  unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr;
  unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one;
  return (_get_arr_size_curr >= _get_arr_size_last) ? 0 :
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
#define _get_dynforward_size(arr,elt_sz) \
  ({struct _dynforward_ptr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})
#endif

#ifdef _INLINE_FUNCTIONS
static _INLINE struct _dyneither_ptr
_dyneither_ptr_plus(struct _dyneither_ptr arr,unsigned elt_sz,int change) {
  struct _dyneither_ptr _ans = (arr);
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
/* Here we have to worry about wrapping around, so if we go past the
 * end, we set the end to 0. */
static _INLINE struct _dynforward_ptr
_dynforward_ptr_plus(struct _dynforward_ptr arr,unsigned elt_sz,int change) {
  struct _dynforward_ptr _ans = (arr);
  unsigned int _dfpp_elts = (((unsigned)_ans.last_plus_one) - 
                             ((unsigned)_ans.curr)) / elt_sz;
  if (change < 0 || ((unsigned)change) > _dfpp_elts)
    _ans.last_plus_one = 0;
  _ans.curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_plus(arr,elt_sz,change) ({ \
  struct _dyneither_ptr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })
#define _dynforward_ptr_plus(arr,elt_sz,change) ({ \
  struct _dynforward_ptr _ans = (arr); \
  unsigned _dfpp_elt_sz = (elt_sz); \
  int _dfpp_change = (change); \
  unsigned int _dfpp_elts = (((unsigned)_ans.last_plus_one) - \
                            ((unsigned)_ans.curr)) / _dfpp_elt_sz; \
  if (_dfpp_change < 0 || ((unsigned)_dfpp_change) > _dfpp_elts) \
    _ans.last_plus_one = 0; \
  _ans.curr += ((int)(_dfpp_elt_sz))*(_dfpp_change); \
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
static _INLINE struct _dynforward_ptr
_dynforward_ptr_inplace_plus(struct _dynforward_ptr *arr_ptr,unsigned elt_sz,
                             int change) {
  struct _dynforward_ptr * _arr_ptr = (arr_ptr);
  unsigned int _dfpp_elts = (((unsigned)_arr_ptr->last_plus_one) - 
                             ((unsigned)_arr_ptr->curr)) / elt_sz;
  if (change < 0 || ((unsigned)change) > _dfpp_elts) 
    _arr_ptr->last_plus_one = 0;
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return *_arr_ptr;
}
#else
#define _dyneither_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })
#define _dynforward_ptr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _dynforward_ptr * _arr_ptr = (arr_ptr); \
  unsigned _dfpp_elt_sz = (elt_sz); \
  int _dfpp_change = (change); \
  unsigned int _dfpp_elts = (((unsigned)_arr_ptr->last_plus_one) - \
                            ((unsigned)_arr_ptr->curr)) / _dfpp_elt_sz; \
  if (_dfpp_change < 0 || ((unsigned)_dfpp_change) > _dfpp_elts) \
    _arr_ptr->last_plus_one = 0; \
  _arr_ptr->curr += ((int)(_dfpp_elt_sz))*(_dfpp_change); \
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
static _INLINE struct _dynforward_ptr
_dynforward_ptr_inplace_plus_post(struct _dynforward_ptr *arr_ptr,unsigned elt_sz,int change) {
  struct _dynforward_ptr * _arr_ptr = (arr_ptr);
  struct _dynforward_ptr _ans = *_arr_ptr;
  unsigned int _dfpp_elts = (((unsigned)_arr_ptr->last_plus_one) - 
                            ((unsigned)_arr_ptr->curr)) / elt_sz; 
  if (change < 0 || ((unsigned)change) > _dfpp_elts) 
    _arr_ptr->last_plus_one = 0; 
  _arr_ptr->curr += ((int)(elt_sz))*(change);
  return _ans;
}
#else
#define _dyneither_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dyneither_ptr * _arr_ptr = (arr_ptr); \
  struct _dyneither_ptr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })
#define _dynforward_ptr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _dynforward_ptr * _arr_ptr = (arr_ptr); \
  struct _dynforward_ptr _ans = *_arr_ptr; \
  unsigned _dfpp_elt_sz = (elt_sz); \
  int _dfpp_change = (change); \
  unsigned int _dfpp_elts = (((unsigned)_arr_ptr->last_plus_one) - \
                            ((unsigned)_arr_ptr->curr)) / _dfpp_elt_sz; \
  if (_dfpp_change < 0 || ((unsigned)_dfpp_change) > _dfpp_elts) \
    _arr_ptr->last_plus_one = 0; \
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
static struct 
_dynforward_ptr _dynforward_ptr_decrease_size(struct _dynforward_ptr x,
                                            unsigned int sz,
                                            unsigned int numelts) {
  if (x.last_plus_one != 0)
    x.last_plus_one -= sz * numelts; 
  return x; 
}

/* Convert between the two forms of dynamic pointers */
#ifdef _INLINE_FUNCTIONS 
static struct _dynforward_ptr
_dyneither_to_dynforward(struct _dyneither_ptr p) {
  struct _dynforward_ptr res;
  res.curr = p.curr;
  res.last_plus_one = (p.base == 0) ? 0 : p.last_plus_one;
  return res;
}
static struct _dyneither_ptr
_dynforward_to_dyneither(struct _dynforward_ptr p) {
  struct _dyneither_ptr res;
  res.base = res.curr = p.curr;
  res.last_plus_one = p.last_plus_one;
  if (p.last_plus_one == 0) 
    res.base = 0;
  return res;
}
#else 
#define _dyneither_to_dynforward(_dnfptr) ({ \
  struct _dyneither_ptr _dnfp = (_dnfptr); \
  struct _dynforward_ptr _dnfpres; \
  _dnfpres.curr = _dnfp.curr; \
  _dnfpres.last_plus_one = (_dnfp.base == 0) ? 0 : _dnfp.last_plus_one; \
  _dnfpres; })
#define _dynforward_to_dyneither(_dfnptr) ({ \
  struct _dynforward_ptr _dfnp = (_dfnptr); \
  struct _dyneither_ptr _dfnres; \
  _dfnres.base = _dfnres.curr = _dfnp.curr; \
  _dfnres.last_plus_one = _dfnp.last_plus_one; \
  if (_dfnp.last_plus_one == 0) \
    _dfnres.base = 0; \
  _dfnres; })
#endif 

/* Allocation */
extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);

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
extern void* _profile_GC_malloc(int,char *file,int lineno);
extern void* _profile_GC_malloc_atomic(int,char *file,int lineno);
extern void* _profile_region_malloc(struct _RegionHandle *, unsigned,
                                     char *file,int lineno);
extern struct _RegionHandle _profile_new_region(const char *rgn_name,
						char *file,int lineno);
extern void _profile_free_region(struct _RegionHandle *,
				 char *file,int lineno);
#  if !defined(RUNTIME_CYC)
#define _new_region(n) _profile_new_region(n,__FILE__ ":" __FUNCTION__,__LINE__)
#define _free_region(r) _profile_free_region(r,__FILE__ ":" __FUNCTION__,__LINE__)
#define _region_malloc(rh,n) _profile_region_malloc(rh,n,__FILE__ ":" __FUNCTION__,__LINE__)
#  endif
#define _cycalloc(n) _profile_GC_malloc(n,__FILE__ ":" __FUNCTION__,__LINE__)
#define _cycalloc_atomic(n) _profile_GC_malloc_atomic(n,__FILE__ ":" __FUNCTION__,__LINE__)
#endif
#endif

/* the next three routines swap [x] and [y]; not thread safe! */
static _INLINE void _swap_word(void *x, void *y) {
  unsigned long *lx = (unsigned long *)x, *ly = (unsigned long *)y, tmp;
  tmp = *lx;
  *lx = *ly;
  *ly = tmp;
}
static _INLINE void _swap_dynforward(struct _dynforward_ptr *x, 
				    struct _dynforward_ptr *y) {
  struct _dynforward_ptr tmp = *x;
  *x = *y;
  *y = tmp;
}
static _INLINE void _swap_dyneither(struct _dyneither_ptr *x, 
				   struct _dyneither_ptr *y) {
  struct _dyneither_ptr tmp = *x;
  *x = *y;
  *y = tmp;
}
 struct Cyc_Core_NewRegion{struct _DynRegionHandle*dynregion;};struct Cyc_Core_Opt{
void*v;};extern char Cyc_Core_Invalid_argument[21];struct Cyc_Core_Invalid_argument_struct{
char*tag;struct _dynforward_ptr f1;};extern char Cyc_Core_Failure[12];struct Cyc_Core_Failure_struct{
char*tag;struct _dynforward_ptr f1;};extern char Cyc_Core_Impossible[15];struct Cyc_Core_Impossible_struct{
char*tag;struct _dynforward_ptr f1;};extern char Cyc_Core_Not_found[14];extern char
Cyc_Core_Unreachable[16];struct Cyc_Core_Unreachable_struct{char*tag;struct
_dynforward_ptr f1;};extern char Cyc_Core_Open_Region[16];extern char Cyc_Core_Free_Region[
16];struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};extern char Cyc_List_List_mismatch[
18];extern char Cyc_List_Nth[8];struct Cyc_Splay_node;struct Cyc_Splay_noderef{
struct Cyc_Splay_node*v;};struct Cyc_Splay_Node_struct{int tag;struct Cyc_Splay_noderef*
f1;};struct Cyc_Splay_node{void*key;void*data;void*left;void*right;};int Cyc_Splay_splay(
int(*f)(void*,void*),void*,void*);struct Cyc_SlowDict_Dict;extern char Cyc_SlowDict_Present[
12];extern char Cyc_SlowDict_Absent[11];struct Cyc_SlowDict_Dict*Cyc_SlowDict_empty(
int(*cmp)(void*,void*));int Cyc_SlowDict_is_empty(struct Cyc_SlowDict_Dict*d);int
Cyc_SlowDict_member(struct Cyc_SlowDict_Dict*d,void*k);struct Cyc_SlowDict_Dict*
Cyc_SlowDict_insert(struct Cyc_SlowDict_Dict*d,void*k,void*v);struct Cyc_SlowDict_Dict*
Cyc_SlowDict_insert_new(struct Cyc_SlowDict_Dict*d,void*k,void*v);struct Cyc_SlowDict_Dict*
Cyc_SlowDict_inserts(struct Cyc_SlowDict_Dict*d,struct Cyc_List_List*l);struct Cyc_SlowDict_Dict*
Cyc_SlowDict_singleton(int(*cmp)(void*,void*),void*k,void*v);void*Cyc_SlowDict_lookup(
struct Cyc_SlowDict_Dict*d,void*k);struct Cyc_Core_Opt*Cyc_SlowDict_lookup_opt(
struct Cyc_SlowDict_Dict*d,void*k);struct Cyc_SlowDict_Dict*Cyc_SlowDict_delete(
struct Cyc_SlowDict_Dict*d,void*k);struct Cyc_SlowDict_Dict*Cyc_SlowDict_delete_present(
struct Cyc_SlowDict_Dict*d,void*k);void*Cyc_SlowDict_fold(void*(*f)(void*,void*,
void*),struct Cyc_SlowDict_Dict*d,void*accum);void*Cyc_SlowDict_fold_c(void*(*f)(
void*,void*,void*,void*),void*env,struct Cyc_SlowDict_Dict*d,void*accum);void Cyc_SlowDict_app(
void*(*f)(void*,void*),struct Cyc_SlowDict_Dict*d);void Cyc_SlowDict_app_c(void*(*
f)(void*,void*,void*),void*env,struct Cyc_SlowDict_Dict*d);void Cyc_SlowDict_iter(
void(*f)(void*,void*),struct Cyc_SlowDict_Dict*d);void Cyc_SlowDict_iter_c(void(*f)(
void*,void*,void*),void*env,struct Cyc_SlowDict_Dict*d);struct Cyc_SlowDict_Dict*
Cyc_SlowDict_map(void*(*f)(void*),struct Cyc_SlowDict_Dict*d);struct Cyc_SlowDict_Dict*
Cyc_SlowDict_map_c(void*(*f)(void*,void*),void*env,struct Cyc_SlowDict_Dict*d);
struct _tuple0{void*f1;void*f2;};struct _tuple0*Cyc_SlowDict_choose(struct Cyc_SlowDict_Dict*
d);struct Cyc_List_List*Cyc_SlowDict_to_list(struct Cyc_SlowDict_Dict*d);char Cyc_SlowDict_Absent[
11]="\000\000\000\000Absent\000";char Cyc_SlowDict_Present[12]="\000\000\000\000Present\000";
struct Cyc_SlowDict_Dict{int(*reln)(void*,void*);void*tree;};struct Cyc_SlowDict_Dict*
Cyc_SlowDict_empty(int(*comp)(void*,void*));struct Cyc_SlowDict_Dict*Cyc_SlowDict_empty(
int(*comp)(void*,void*)){void*t=(void*)0;struct Cyc_SlowDict_Dict*_tmp56;return(
_tmp56=_cycalloc(sizeof(*_tmp56)),((_tmp56->reln=comp,((_tmp56->tree=(void*)t,
_tmp56)))));}int Cyc_SlowDict_is_empty(struct Cyc_SlowDict_Dict*d);int Cyc_SlowDict_is_empty(
struct Cyc_SlowDict_Dict*d){void*_tmp1=(void*)d->tree;_LL1: if((int)_tmp1 != 0)goto
_LL3;_LL2: return 1;_LL3: if(_tmp1 <= (void*)1)goto _LL0;if(*((int*)_tmp1)!= 0)goto
_LL0;_LL4: return 0;_LL0:;}int Cyc_SlowDict_member(struct Cyc_SlowDict_Dict*d,void*
key);int Cyc_SlowDict_member(struct Cyc_SlowDict_Dict*d,void*key){return Cyc_Splay_splay(
d->reln,key,(void*)d->tree);}struct Cyc_SlowDict_Dict*Cyc_SlowDict_insert(struct
Cyc_SlowDict_Dict*d,void*key,void*data);struct Cyc_SlowDict_Dict*Cyc_SlowDict_insert(
struct Cyc_SlowDict_Dict*d,void*key,void*data){void*newleft=(void*)0;void*
newright=(void*)0;if(Cyc_Splay_splay(d->reln,key,(void*)d->tree)){void*_tmp2=(
void*)d->tree;struct Cyc_Splay_noderef*_tmp3;_LL6: if(_tmp2 <= (void*)1)goto _LL8;
if(*((int*)_tmp2)!= 0)goto _LL8;_tmp3=((struct Cyc_Splay_Node_struct*)_tmp2)->f1;
_LL7: newleft=(void*)(_tmp3->v)->left;newright=(void*)(_tmp3->v)->right;goto _LL5;
_LL8:;_LL9: goto _LL5;_LL5:;}else{void*_tmp4=(void*)d->tree;struct Cyc_Splay_noderef*
_tmp5;_LLB: if(_tmp4 <= (void*)1)goto _LLD;if(*((int*)_tmp4)!= 0)goto _LLD;_tmp5=((
struct Cyc_Splay_Node_struct*)_tmp4)->f1;_LLC: {struct Cyc_Splay_node*_tmp6=_tmp5->v;
if((d->reln)(key,(void*)_tmp6->key)< 0){newleft=(void*)_tmp6->left;{struct Cyc_Splay_Node_struct
_tmp60;struct Cyc_Splay_node*_tmp5F;struct Cyc_Splay_noderef*_tmp5E;struct Cyc_Splay_Node_struct*
_tmp5D;newright=(void*)((_tmp5D=_cycalloc(sizeof(*_tmp5D)),((_tmp5D[0]=((_tmp60.tag=
0,((_tmp60.f1=((_tmp5E=_cycalloc(sizeof(*_tmp5E)),((_tmp5E->v=((_tmp5F=_cycalloc(
sizeof(*_tmp5F)),((_tmp5F->key=(void*)((void*)_tmp6->key),((_tmp5F->data=(void*)((
void*)_tmp6->data),((_tmp5F->left=(void*)((void*)0),((_tmp5F->right=(void*)((
void*)_tmp6->right),_tmp5F)))))))))),_tmp5E)))),_tmp60)))),_tmp5D))));}}else{{
struct Cyc_Splay_Node_struct _tmp6A;struct Cyc_Splay_node*_tmp69;struct Cyc_Splay_noderef*
_tmp68;struct Cyc_Splay_Node_struct*_tmp67;newleft=(void*)((_tmp67=_cycalloc(
sizeof(*_tmp67)),((_tmp67[0]=((_tmp6A.tag=0,((_tmp6A.f1=((_tmp68=_cycalloc(
sizeof(*_tmp68)),((_tmp68->v=((_tmp69=_cycalloc(sizeof(*_tmp69)),((_tmp69->key=(
void*)((void*)_tmp6->key),((_tmp69->data=(void*)((void*)_tmp6->data),((_tmp69->left=(
void*)((void*)_tmp6->left),((_tmp69->right=(void*)((void*)0),_tmp69)))))))))),
_tmp68)))),_tmp6A)))),_tmp67))));}newright=(void*)_tmp6->right;}goto _LLA;}_LLD:
if((int)_tmp4 != 0)goto _LLA;_LLE: goto _LLA;_LLA:;}{struct Cyc_Splay_Node_struct*
_tmp79;struct Cyc_Splay_noderef*_tmp78;struct Cyc_Splay_node*_tmp77;struct Cyc_Splay_Node_struct
_tmp76;struct Cyc_SlowDict_Dict*_tmp75;return(_tmp75=_cycalloc(sizeof(*_tmp75)),((
_tmp75->reln=d->reln,((_tmp75->tree=(void*)((void*)((_tmp79=_cycalloc(sizeof(*
_tmp79)),((_tmp79[0]=((_tmp76.tag=0,((_tmp76.f1=((_tmp78=_cycalloc(sizeof(*
_tmp78)),((_tmp78->v=((_tmp77=_cycalloc(sizeof(*_tmp77)),((_tmp77->key=(void*)
key,((_tmp77->data=(void*)data,((_tmp77->left=(void*)newleft,((_tmp77->right=(
void*)newright,_tmp77)))))))))),_tmp78)))),_tmp76)))),_tmp79))))),_tmp75)))));}}
struct Cyc_SlowDict_Dict*Cyc_SlowDict_insert_new(struct Cyc_SlowDict_Dict*d,void*
key,void*data);struct Cyc_SlowDict_Dict*Cyc_SlowDict_insert_new(struct Cyc_SlowDict_Dict*
d,void*key,void*data){if(Cyc_Splay_splay(d->reln,key,(void*)d->tree))(int)_throw((
void*)Cyc_SlowDict_Present);return Cyc_SlowDict_insert(d,key,data);}struct Cyc_SlowDict_Dict*
Cyc_SlowDict_inserts(struct Cyc_SlowDict_Dict*d,struct Cyc_List_List*kds);struct
Cyc_SlowDict_Dict*Cyc_SlowDict_inserts(struct Cyc_SlowDict_Dict*d,struct Cyc_List_List*
kds){for(0;kds != 0;kds=kds->tl){d=Cyc_SlowDict_insert(d,(*((struct _tuple0*)kds->hd)).f1,(*((
struct _tuple0*)kds->hd)).f2);}return d;}struct Cyc_SlowDict_Dict*Cyc_SlowDict_singleton(
int(*comp)(void*,void*),void*key,void*data);struct Cyc_SlowDict_Dict*Cyc_SlowDict_singleton(
int(*comp)(void*,void*),void*key,void*data){struct Cyc_Splay_Node_struct*_tmp88;
struct Cyc_Splay_noderef*_tmp87;struct Cyc_Splay_node*_tmp86;struct Cyc_Splay_Node_struct
_tmp85;struct Cyc_SlowDict_Dict*_tmp84;return(_tmp84=_cycalloc(sizeof(*_tmp84)),((
_tmp84->reln=comp,((_tmp84->tree=(void*)((void*)((_tmp88=_cycalloc(sizeof(*
_tmp88)),((_tmp88[0]=((_tmp85.tag=0,((_tmp85.f1=((_tmp87=_cycalloc(sizeof(*
_tmp87)),((_tmp87->v=((_tmp86=_cycalloc(sizeof(*_tmp86)),((_tmp86->key=(void*)
key,((_tmp86->data=(void*)data,((_tmp86->left=(void*)((void*)0),((_tmp86->right=(
void*)((void*)0),_tmp86)))))))))),_tmp87)))),_tmp85)))),_tmp88))))),_tmp84)))));}
void*Cyc_SlowDict_lookup(struct Cyc_SlowDict_Dict*d,void*key);void*Cyc_SlowDict_lookup(
struct Cyc_SlowDict_Dict*d,void*key){if(Cyc_Splay_splay(d->reln,key,(void*)d->tree)){
void*_tmp19=(void*)d->tree;struct Cyc_Splay_noderef*_tmp1A;_LL10: if(_tmp19 <= (
void*)1)goto _LL12;if(*((int*)_tmp19)!= 0)goto _LL12;_tmp1A=((struct Cyc_Splay_Node_struct*)
_tmp19)->f1;_LL11: return(void*)(_tmp1A->v)->data;_LL12: if((int)_tmp19 != 0)goto
_LLF;_LL13: {struct Cyc_Core_Impossible_struct _tmp8E;const char*_tmp8D;struct Cyc_Core_Impossible_struct*
_tmp8C;(int)_throw((void*)((_tmp8C=_cycalloc(sizeof(*_tmp8C)),((_tmp8C[0]=((
_tmp8E.tag=Cyc_Core_Impossible,((_tmp8E.f1=((_tmp8D="Dict::lookup",
_tag_dynforward(_tmp8D,sizeof(char),_get_zero_arr_size_char(_tmp8D,13)))),_tmp8E)))),
_tmp8C)))));}_LLF:;}(int)_throw((void*)Cyc_SlowDict_Absent);}struct Cyc_Core_Opt*
Cyc_SlowDict_lookup_opt(struct Cyc_SlowDict_Dict*d,void*key);struct Cyc_Core_Opt*
Cyc_SlowDict_lookup_opt(struct Cyc_SlowDict_Dict*d,void*key){if(Cyc_Splay_splay(d->reln,
key,(void*)d->tree)){void*_tmp1E=(void*)d->tree;struct Cyc_Splay_noderef*_tmp1F;
_LL15: if(_tmp1E <= (void*)1)goto _LL17;if(*((int*)_tmp1E)!= 0)goto _LL17;_tmp1F=((
struct Cyc_Splay_Node_struct*)_tmp1E)->f1;_LL16: {struct Cyc_Core_Opt*_tmp8F;
return(_tmp8F=_cycalloc(sizeof(*_tmp8F)),((_tmp8F->v=(void*)((void*)(_tmp1F->v)->data),
_tmp8F)));}_LL17: if((int)_tmp1E != 0)goto _LL14;_LL18: {struct Cyc_Core_Impossible_struct
_tmp95;const char*_tmp94;struct Cyc_Core_Impossible_struct*_tmp93;(int)_throw((
void*)((_tmp93=_cycalloc(sizeof(*_tmp93)),((_tmp93[0]=((_tmp95.tag=Cyc_Core_Impossible,((
_tmp95.f1=((_tmp94="Dict::lookup",_tag_dynforward(_tmp94,sizeof(char),
_get_zero_arr_size_char(_tmp94,13)))),_tmp95)))),_tmp93)))));}_LL14:;}return 0;}
static int Cyc_SlowDict_get_largest(void*x,void*y);static int Cyc_SlowDict_get_largest(
void*x,void*y){return 1;}struct Cyc_SlowDict_Dict*Cyc_SlowDict_delete(struct Cyc_SlowDict_Dict*
d,void*key);struct Cyc_SlowDict_Dict*Cyc_SlowDict_delete(struct Cyc_SlowDict_Dict*
d,void*key){if(Cyc_Splay_splay(d->reln,key,(void*)d->tree)){void*_tmp24=(void*)d->tree;
struct Cyc_Splay_noderef*_tmp25;_LL1A: if((int)_tmp24 != 0)goto _LL1C;_LL1B: {struct
Cyc_Core_Impossible_struct _tmp9B;const char*_tmp9A;struct Cyc_Core_Impossible_struct*
_tmp99;(int)_throw((void*)((_tmp99=_cycalloc(sizeof(*_tmp99)),((_tmp99[0]=((
_tmp9B.tag=Cyc_Core_Impossible,((_tmp9B.f1=((_tmp9A="Dict::lookup",
_tag_dynforward(_tmp9A,sizeof(char),_get_zero_arr_size_char(_tmp9A,13)))),_tmp9B)))),
_tmp99)))));}_LL1C: if(_tmp24 <= (void*)1)goto _LL19;if(*((int*)_tmp24)!= 0)goto
_LL19;_tmp25=((struct Cyc_Splay_Node_struct*)_tmp24)->f1;_LL1D: {struct Cyc_Splay_node*
n=_tmp25->v;void*_tmp29=(void*)n->left;struct Cyc_Splay_noderef*_tmp2A;_LL1F: if((
int)_tmp29 != 0)goto _LL21;_LL20: {struct Cyc_SlowDict_Dict*_tmp9C;return(_tmp9C=
_cycalloc(sizeof(*_tmp9C)),((_tmp9C->reln=d->reln,((_tmp9C->tree=(void*)((void*)
n->right),_tmp9C)))));}_LL21: if(_tmp29 <= (void*)1)goto _LL1E;if(*((int*)_tmp29)!= 
0)goto _LL1E;_tmp2A=((struct Cyc_Splay_Node_struct*)_tmp29)->f1;_LL22: {void*
_tmp2C=(void*)n->right;struct Cyc_Splay_noderef*_tmp2D;_LL24: if((int)_tmp2C != 0)
goto _LL26;_LL25: {struct Cyc_SlowDict_Dict*_tmp9D;return(_tmp9D=_cycalloc(sizeof(*
_tmp9D)),((_tmp9D->reln=d->reln,((_tmp9D->tree=(void*)((void*)n->left),_tmp9D)))));}
_LL26: if(_tmp2C <= (void*)1)goto _LL23;if(*((int*)_tmp2C)!= 0)goto _LL23;_tmp2D=((
struct Cyc_Splay_Node_struct*)_tmp2C)->f1;_LL27: Cyc_Splay_splay(Cyc_SlowDict_get_largest,
key,(void*)n->left);{struct Cyc_Splay_node*newtop=_tmp2A->v;struct Cyc_Splay_Node_struct*
_tmpAC;struct Cyc_Splay_noderef*_tmpAB;struct Cyc_Splay_node*_tmpAA;struct Cyc_Splay_Node_struct
_tmpA9;struct Cyc_SlowDict_Dict*_tmpA8;return(_tmpA8=_cycalloc(sizeof(*_tmpA8)),((
_tmpA8->reln=d->reln,((_tmpA8->tree=(void*)((void*)((_tmpAC=_cycalloc(sizeof(*
_tmpAC)),((_tmpAC[0]=((_tmpA9.tag=0,((_tmpA9.f1=((_tmpAB=_cycalloc(sizeof(*
_tmpAB)),((_tmpAB->v=((_tmpAA=_cycalloc(sizeof(*_tmpAA)),((_tmpAA->key=(void*)((
void*)newtop->key),((_tmpAA->data=(void*)((void*)newtop->data),((_tmpAA->left=(
void*)((void*)newtop->left),((_tmpAA->right=(void*)((void*)n->right),_tmpAA)))))))))),
_tmpAB)))),_tmpA9)))),_tmpAC))))),_tmpA8)))));}_LL23:;}_LL1E:;}_LL19:;}else{
return d;}}struct Cyc_SlowDict_Dict*Cyc_SlowDict_delete_present(struct Cyc_SlowDict_Dict*
d,void*key);struct Cyc_SlowDict_Dict*Cyc_SlowDict_delete_present(struct Cyc_SlowDict_Dict*
d,void*key){struct Cyc_SlowDict_Dict*_tmp34=Cyc_SlowDict_delete(d,key);if(d == 
_tmp34)(int)_throw((void*)Cyc_SlowDict_Absent);return _tmp34;}static void*Cyc_SlowDict_fold_tree(
void*(*f)(void*,void*,void*),void*t,void*accum);static void*Cyc_SlowDict_fold_tree(
void*(*f)(void*,void*,void*),void*t,void*accum){void*_tmp35=t;struct Cyc_Splay_noderef*
_tmp36;_LL29: if((int)_tmp35 != 0)goto _LL2B;_LL2A: return accum;_LL2B: if(_tmp35 <= (
void*)1)goto _LL28;if(*((int*)_tmp35)!= 0)goto _LL28;_tmp36=((struct Cyc_Splay_Node_struct*)
_tmp35)->f1;_LL2C: {struct Cyc_Splay_node*n=_tmp36->v;return f((void*)n->key,(void*)
n->data,Cyc_SlowDict_fold_tree(f,(void*)n->left,Cyc_SlowDict_fold_tree(f,(void*)
n->right,accum)));}_LL28:;}void*Cyc_SlowDict_fold(void*(*f)(void*,void*,void*),
struct Cyc_SlowDict_Dict*d,void*accum);void*Cyc_SlowDict_fold(void*(*f)(void*,
void*,void*),struct Cyc_SlowDict_Dict*d,void*accum){return Cyc_SlowDict_fold_tree(
f,(void*)d->tree,accum);}static void*Cyc_SlowDict_fold_tree_c(void*(*f)(void*,
void*,void*,void*),void*env,void*t,void*accum);static void*Cyc_SlowDict_fold_tree_c(
void*(*f)(void*,void*,void*,void*),void*env,void*t,void*accum){void*_tmp37=t;
struct Cyc_Splay_noderef*_tmp38;_LL2E: if((int)_tmp37 != 0)goto _LL30;_LL2F: return
accum;_LL30: if(_tmp37 <= (void*)1)goto _LL2D;if(*((int*)_tmp37)!= 0)goto _LL2D;
_tmp38=((struct Cyc_Splay_Node_struct*)_tmp37)->f1;_LL31: {struct Cyc_Splay_node*n=
_tmp38->v;return f(env,(void*)n->key,(void*)n->data,Cyc_SlowDict_fold_tree_c(f,
env,(void*)n->left,Cyc_SlowDict_fold_tree_c(f,env,(void*)n->right,accum)));}
_LL2D:;}void*Cyc_SlowDict_fold_c(void*(*f)(void*,void*,void*,void*),void*env,
struct Cyc_SlowDict_Dict*dict,void*accum);void*Cyc_SlowDict_fold_c(void*(*f)(void*,
void*,void*,void*),void*env,struct Cyc_SlowDict_Dict*dict,void*accum){return Cyc_SlowDict_fold_tree_c(
f,env,(void*)dict->tree,accum);}static void Cyc_SlowDict_app_tree(void*(*f)(void*,
void*),void*t);static void Cyc_SlowDict_app_tree(void*(*f)(void*,void*),void*t){
void*_tmp39=t;struct Cyc_Splay_noderef*_tmp3A;_LL33: if((int)_tmp39 != 0)goto _LL35;
_LL34: goto _LL32;_LL35: if(_tmp39 <= (void*)1)goto _LL32;if(*((int*)_tmp39)!= 0)goto
_LL32;_tmp3A=((struct Cyc_Splay_Node_struct*)_tmp39)->f1;_LL36: {struct Cyc_Splay_node*
_tmp3B=_tmp3A->v;Cyc_SlowDict_app_tree(f,(void*)_tmp3B->left);f((void*)_tmp3B->key,(
void*)_tmp3B->data);Cyc_SlowDict_app_tree(f,(void*)_tmp3B->right);goto _LL32;}
_LL32:;}void Cyc_SlowDict_app(void*(*f)(void*,void*),struct Cyc_SlowDict_Dict*d);
void Cyc_SlowDict_app(void*(*f)(void*,void*),struct Cyc_SlowDict_Dict*d){Cyc_SlowDict_app_tree(
f,(void*)d->tree);}static void Cyc_SlowDict_iter_tree(void(*f)(void*,void*),void*t);
static void Cyc_SlowDict_iter_tree(void(*f)(void*,void*),void*t){void*_tmp3C=t;
struct Cyc_Splay_noderef*_tmp3D;_LL38: if((int)_tmp3C != 0)goto _LL3A;_LL39: goto
_LL37;_LL3A: if(_tmp3C <= (void*)1)goto _LL37;if(*((int*)_tmp3C)!= 0)goto _LL37;
_tmp3D=((struct Cyc_Splay_Node_struct*)_tmp3C)->f1;_LL3B: {struct Cyc_Splay_node*n=
_tmp3D->v;Cyc_SlowDict_iter_tree(f,(void*)n->left);f((void*)n->key,(void*)n->data);
Cyc_SlowDict_iter_tree(f,(void*)n->right);goto _LL37;}_LL37:;}void Cyc_SlowDict_iter(
void(*f)(void*,void*),struct Cyc_SlowDict_Dict*d);void Cyc_SlowDict_iter(void(*f)(
void*,void*),struct Cyc_SlowDict_Dict*d){Cyc_SlowDict_iter_tree(f,(void*)d->tree);}
static void Cyc_SlowDict_app_tree_c(void*(*f)(void*,void*,void*),void*env,void*t);
static void Cyc_SlowDict_app_tree_c(void*(*f)(void*,void*,void*),void*env,void*t){
void*_tmp3E=t;struct Cyc_Splay_noderef*_tmp3F;_LL3D: if((int)_tmp3E != 0)goto _LL3F;
_LL3E: goto _LL3C;_LL3F: if(_tmp3E <= (void*)1)goto _LL3C;if(*((int*)_tmp3E)!= 0)goto
_LL3C;_tmp3F=((struct Cyc_Splay_Node_struct*)_tmp3E)->f1;_LL40: {struct Cyc_Splay_node*
n=_tmp3F->v;Cyc_SlowDict_app_tree_c(f,env,(void*)n->left);f(env,(void*)n->key,(
void*)n->data);Cyc_SlowDict_app_tree_c(f,env,(void*)n->right);goto _LL3C;}_LL3C:;}
void Cyc_SlowDict_app_c(void*(*f)(void*,void*,void*),void*env,struct Cyc_SlowDict_Dict*
d);void Cyc_SlowDict_app_c(void*(*f)(void*,void*,void*),void*env,struct Cyc_SlowDict_Dict*
d){Cyc_SlowDict_app_tree_c(f,env,(void*)d->tree);}static void Cyc_SlowDict_iter_tree_c(
void(*f)(void*,void*,void*),void*env,void*t);static void Cyc_SlowDict_iter_tree_c(
void(*f)(void*,void*,void*),void*env,void*t){void*_tmp40=t;struct Cyc_Splay_noderef*
_tmp41;_LL42: if((int)_tmp40 != 0)goto _LL44;_LL43: goto _LL41;_LL44: if(_tmp40 <= (
void*)1)goto _LL41;if(*((int*)_tmp40)!= 0)goto _LL41;_tmp41=((struct Cyc_Splay_Node_struct*)
_tmp40)->f1;_LL45: {struct Cyc_Splay_node*n=_tmp41->v;Cyc_SlowDict_iter_tree_c(f,
env,(void*)n->left);f(env,(void*)n->key,(void*)n->data);Cyc_SlowDict_iter_tree_c(
f,env,(void*)n->right);goto _LL41;}_LL41:;}void Cyc_SlowDict_iter_c(void(*f)(void*,
void*,void*),void*env,struct Cyc_SlowDict_Dict*d);void Cyc_SlowDict_iter_c(void(*f)(
void*,void*,void*),void*env,struct Cyc_SlowDict_Dict*d){Cyc_SlowDict_iter_tree_c(
f,env,(void*)d->tree);}static void*Cyc_SlowDict_map_tree(void*(*f)(void*),void*t);
static void*Cyc_SlowDict_map_tree(void*(*f)(void*),void*t){void*_tmp42=t;struct
Cyc_Splay_noderef*_tmp43;_LL47: if((int)_tmp42 != 0)goto _LL49;_LL48: return(void*)0;
_LL49: if(_tmp42 <= (void*)1)goto _LL46;if(*((int*)_tmp42)!= 0)goto _LL46;_tmp43=((
struct Cyc_Splay_Node_struct*)_tmp42)->f1;_LL4A: {struct Cyc_Splay_node*_tmp44=
_tmp43->v;struct Cyc_Splay_Node_struct _tmpB6;struct Cyc_Splay_node*_tmpB5;struct
Cyc_Splay_noderef*_tmpB4;struct Cyc_Splay_Node_struct*_tmpB3;return(void*)((
_tmpB3=_cycalloc(sizeof(*_tmpB3)),((_tmpB3[0]=((_tmpB6.tag=0,((_tmpB6.f1=((
_tmpB4=_cycalloc(sizeof(*_tmpB4)),((_tmpB4->v=((_tmpB5=_cycalloc(sizeof(*_tmpB5)),((
_tmpB5->key=(void*)((void*)_tmp44->key),((_tmpB5->data=(void*)f((void*)_tmp44->data),((
_tmpB5->left=(void*)Cyc_SlowDict_map_tree(f,(void*)_tmp44->left),((_tmpB5->right=(
void*)Cyc_SlowDict_map_tree(f,(void*)_tmp44->right),_tmpB5)))))))))),_tmpB4)))),
_tmpB6)))),_tmpB3))));}_LL46:;}struct Cyc_SlowDict_Dict*Cyc_SlowDict_map(void*(*f)(
void*),struct Cyc_SlowDict_Dict*d);struct Cyc_SlowDict_Dict*Cyc_SlowDict_map(void*(*
f)(void*),struct Cyc_SlowDict_Dict*d){struct Cyc_SlowDict_Dict*_tmpB7;return(
_tmpB7=_cycalloc(sizeof(*_tmpB7)),((_tmpB7->reln=d->reln,((_tmpB7->tree=(void*)
Cyc_SlowDict_map_tree(f,(void*)d->tree),_tmpB7)))));}static void*Cyc_SlowDict_map_tree_c(
void*(*f)(void*,void*),void*env,void*t);static void*Cyc_SlowDict_map_tree_c(void*(*
f)(void*,void*),void*env,void*t){void*_tmp4A=t;struct Cyc_Splay_noderef*_tmp4B;
_LL4C: if((int)_tmp4A != 0)goto _LL4E;_LL4D: return(void*)0;_LL4E: if(_tmp4A <= (void*)
1)goto _LL4B;if(*((int*)_tmp4A)!= 0)goto _LL4B;_tmp4B=((struct Cyc_Splay_Node_struct*)
_tmp4A)->f1;_LL4F: {struct Cyc_Splay_node*n=_tmp4B->v;struct Cyc_Splay_Node_struct
_tmpC1;struct Cyc_Splay_node*_tmpC0;struct Cyc_Splay_noderef*_tmpBF;struct Cyc_Splay_Node_struct*
_tmpBE;return(void*)((_tmpBE=_cycalloc(sizeof(*_tmpBE)),((_tmpBE[0]=((_tmpC1.tag=
0,((_tmpC1.f1=((_tmpBF=_cycalloc(sizeof(*_tmpBF)),((_tmpBF->v=((_tmpC0=_cycalloc(
sizeof(*_tmpC0)),((_tmpC0->key=(void*)((void*)n->key),((_tmpC0->data=(void*)f(
env,(void*)n->data),((_tmpC0->left=(void*)Cyc_SlowDict_map_tree_c(f,env,(void*)n->left),((
_tmpC0->right=(void*)Cyc_SlowDict_map_tree_c(f,env,(void*)n->right),_tmpC0)))))))))),
_tmpBF)))),_tmpC1)))),_tmpBE))));}_LL4B:;}struct Cyc_SlowDict_Dict*Cyc_SlowDict_map_c(
void*(*f)(void*,void*),void*env,struct Cyc_SlowDict_Dict*d);struct Cyc_SlowDict_Dict*
Cyc_SlowDict_map_c(void*(*f)(void*,void*),void*env,struct Cyc_SlowDict_Dict*d){
struct Cyc_SlowDict_Dict*_tmpC2;return(_tmpC2=_cycalloc(sizeof(*_tmpC2)),((_tmpC2->reln=
d->reln,((_tmpC2->tree=(void*)Cyc_SlowDict_map_tree_c(f,env,(void*)d->tree),
_tmpC2)))));}struct _tuple0*Cyc_SlowDict_choose(struct Cyc_SlowDict_Dict*d);struct
_tuple0*Cyc_SlowDict_choose(struct Cyc_SlowDict_Dict*d){void*_tmp51=(void*)d->tree;
struct Cyc_Splay_noderef*_tmp52;_LL51: if((int)_tmp51 != 0)goto _LL53;_LL52:(int)
_throw((void*)Cyc_SlowDict_Absent);_LL53: if(_tmp51 <= (void*)1)goto _LL50;if(*((
int*)_tmp51)!= 0)goto _LL50;_tmp52=((struct Cyc_Splay_Node_struct*)_tmp51)->f1;
_LL54: {struct _tuple0*_tmpC3;return(_tmpC3=_cycalloc(sizeof(*_tmpC3)),((_tmpC3->f1=(
void*)(_tmp52->v)->key,((_tmpC3->f2=(void*)(_tmp52->v)->data,_tmpC3)))));}_LL50:;}
struct Cyc_List_List*Cyc_SlowDict_to_list_f(void*k,void*v,struct Cyc_List_List*
accum);struct Cyc_List_List*Cyc_SlowDict_to_list_f(void*k,void*v,struct Cyc_List_List*
accum){struct _tuple0*_tmpC6;struct Cyc_List_List*_tmpC5;return(_tmpC5=_cycalloc(
sizeof(*_tmpC5)),((_tmpC5->hd=((_tmpC6=_cycalloc(sizeof(*_tmpC6)),((_tmpC6->f1=k,((
_tmpC6->f2=v,_tmpC6)))))),((_tmpC5->tl=accum,_tmpC5)))));}struct Cyc_List_List*
Cyc_SlowDict_to_list(struct Cyc_SlowDict_Dict*d);struct Cyc_List_List*Cyc_SlowDict_to_list(
struct Cyc_SlowDict_Dict*d){return((struct Cyc_List_List*(*)(struct Cyc_List_List*(*
f)(void*,void*,struct Cyc_List_List*),struct Cyc_SlowDict_Dict*d,struct Cyc_List_List*
accum))Cyc_SlowDict_fold)(Cyc_SlowDict_to_list_f,d,0);}