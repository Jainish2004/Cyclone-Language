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
_check_dyneither_subscript(struct _dyneither_ptr arr,unsigned elt_sz,unsigned index) {
  struct _dyneither_ptr _cus_arr = (arr);
  unsigned _cus_elt_sz = (elt_sz);
  unsigned _cus_index = (index);
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index;
  /* JGM: not needed! if (!_cus_arr.base) _throw_null(); */ 
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one)
    _throw_arraybounds();
  return _cus_ans;
}
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
_untag_dyneither_ptr(struct _dyneither_ptr arr, 
                     unsigned elt_sz,unsigned num_elts) {
  struct _dyneither_ptr _arr = (arr);
  unsigned char *_curr = _arr.curr;
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)
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
 struct Cyc_Core_NewRegion{struct _DynRegionHandle*dynregion;};struct Cyc_Core_Opt{
void*v;};extern char Cyc_Core_Invalid_argument[21];struct Cyc_Core_Invalid_argument_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Failure[12];struct Cyc_Core_Failure_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Impossible[15];struct Cyc_Core_Impossible_struct{
char*tag;struct _dyneither_ptr f1;};extern char Cyc_Core_Not_found[14];extern char Cyc_Core_Unreachable[
16];struct Cyc_Core_Unreachable_struct{char*tag;struct _dyneither_ptr f1;};extern
struct _RegionHandle*Cyc_Core_heap_region;extern char Cyc_Core_Open_Region[16];
extern char Cyc_Core_Free_Region[16];struct Cyc_List_List{void*hd;struct Cyc_List_List*
tl;};int Cyc_List_length(struct Cyc_List_List*x);extern char Cyc_List_List_mismatch[
18];void Cyc_List_iter(void(*f)(void*),struct Cyc_List_List*x);void Cyc_List_iter_c(
void(*f)(void*,void*),void*env,struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_append(
struct Cyc_List_List*x,struct Cyc_List_List*y);struct Cyc_List_List*Cyc_List_imp_append(
struct Cyc_List_List*x,struct Cyc_List_List*y);extern char Cyc_List_Nth[8];int Cyc_List_mem(
int(*compare)(void*,void*),struct Cyc_List_List*l,void*x);struct Cyc_List_List*Cyc_List_filter_c(
int(*f)(void*,void*),void*env,struct Cyc_List_List*x);int Cyc_strcmp(struct
_dyneither_ptr s1,struct _dyneither_ptr s2);int Cyc_strptrcmp(struct _dyneither_ptr*
s1,struct _dyneither_ptr*s2);struct Cyc_Lineno_Pos{struct _dyneither_ptr
logical_file;struct _dyneither_ptr line;int line_no;int col;};extern char Cyc_Position_Exit[
9];struct Cyc_Position_Segment;struct Cyc_Position_Error{struct _dyneither_ptr
source;struct Cyc_Position_Segment*seg;void*kind;struct _dyneither_ptr desc;};
extern char Cyc_Position_Nocontext[14];struct Cyc_Absyn_Loc_n_struct{int tag;};
struct Cyc_Absyn_Rel_n_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Abs_n_struct{
int tag;struct Cyc_List_List*f1;};union Cyc_Absyn_Nmspace_union{struct Cyc_Absyn_Loc_n_struct
Loc_n;struct Cyc_Absyn_Rel_n_struct Rel_n;struct Cyc_Absyn_Abs_n_struct Abs_n;};
struct _tuple0{union Cyc_Absyn_Nmspace_union f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Conref;
struct Cyc_Absyn_Tqual{int print_const;int q_volatile;int q_restrict;int real_const;
struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Eq_constr_struct{int tag;void*f1;
};struct Cyc_Absyn_Forward_constr_struct{int tag;struct Cyc_Absyn_Conref*f1;};
struct Cyc_Absyn_No_constr_struct{int tag;};union Cyc_Absyn_Constraint_union{struct
Cyc_Absyn_Eq_constr_struct Eq_constr;struct Cyc_Absyn_Forward_constr_struct
Forward_constr;struct Cyc_Absyn_No_constr_struct No_constr;};struct Cyc_Absyn_Conref{
union Cyc_Absyn_Constraint_union v;};struct Cyc_Absyn_Eq_kb_struct{int tag;void*f1;}
;struct Cyc_Absyn_Unknown_kb_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_struct{
int tag;struct Cyc_Core_Opt*f1;void*f2;};struct Cyc_Absyn_Tvar{struct _dyneither_ptr*
name;int identity;void*kind;};struct Cyc_Absyn_Upper_b_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_PtrLoc{struct Cyc_Position_Segment*ptr_loc;struct Cyc_Position_Segment*
rgn_loc;struct Cyc_Position_Segment*zt_loc;};struct Cyc_Absyn_PtrAtts{void*rgn;
struct Cyc_Absyn_Conref*nullable;struct Cyc_Absyn_Conref*bounds;struct Cyc_Absyn_Conref*
zero_term;struct Cyc_Absyn_PtrLoc*ptrloc;};struct Cyc_Absyn_PtrInfo{void*elt_typ;
struct Cyc_Absyn_Tqual elt_tq;struct Cyc_Absyn_PtrAtts ptr_atts;};struct Cyc_Absyn_VarargInfo{
struct Cyc_Core_Opt*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{
struct Cyc_List_List*tvars;struct Cyc_Core_Opt*effect;void*ret_typ;struct Cyc_List_List*
args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*
rgn_po;struct Cyc_List_List*attributes;};struct Cyc_Absyn_UnknownTunionInfo{struct
_tuple0*name;int is_xtunion;int is_flat;};struct Cyc_Absyn_UnknownTunion_struct{int
tag;struct Cyc_Absyn_UnknownTunionInfo f1;};struct Cyc_Absyn_KnownTunion_struct{int
tag;struct Cyc_Absyn_Tuniondecl**f1;};union Cyc_Absyn_TunionInfoU_union{struct Cyc_Absyn_UnknownTunion_struct
UnknownTunion;struct Cyc_Absyn_KnownTunion_struct KnownTunion;};struct Cyc_Absyn_TunionInfo{
union Cyc_Absyn_TunionInfoU_union tunion_info;struct Cyc_List_List*targs;struct Cyc_Core_Opt*
rgn;};struct Cyc_Absyn_UnknownTunionFieldInfo{struct _tuple0*tunion_name;struct
_tuple0*field_name;int is_xtunion;};struct Cyc_Absyn_UnknownTunionfield_struct{int
tag;struct Cyc_Absyn_UnknownTunionFieldInfo f1;};struct Cyc_Absyn_KnownTunionfield_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};union Cyc_Absyn_TunionFieldInfoU_union{
struct Cyc_Absyn_UnknownTunionfield_struct UnknownTunionfield;struct Cyc_Absyn_KnownTunionfield_struct
KnownTunionfield;};struct Cyc_Absyn_TunionFieldInfo{union Cyc_Absyn_TunionFieldInfoU_union
field_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownAggr_struct{int tag;
void*f1;struct _tuple0*f2;};struct Cyc_Absyn_KnownAggr_struct{int tag;struct Cyc_Absyn_Aggrdecl**
f1;};union Cyc_Absyn_AggrInfoU_union{struct Cyc_Absyn_UnknownAggr_struct
UnknownAggr;struct Cyc_Absyn_KnownAggr_struct KnownAggr;};struct Cyc_Absyn_AggrInfo{
union Cyc_Absyn_AggrInfoU_union aggr_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_ArrayInfo{
void*elt_type;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Exp*num_elts;struct Cyc_Absyn_Conref*
zero_term;struct Cyc_Position_Segment*zt_loc;};struct Cyc_Absyn_Evar_struct{int tag;
struct Cyc_Core_Opt*f1;struct Cyc_Core_Opt*f2;int f3;struct Cyc_Core_Opt*f4;};struct
Cyc_Absyn_VarType_struct{int tag;struct Cyc_Absyn_Tvar*f1;};struct Cyc_Absyn_TunionType_struct{
int tag;struct Cyc_Absyn_TunionInfo f1;};struct Cyc_Absyn_TunionFieldType_struct{int
tag;struct Cyc_Absyn_TunionFieldInfo f1;};struct Cyc_Absyn_PointerType_struct{int
tag;struct Cyc_Absyn_PtrInfo f1;};struct Cyc_Absyn_IntType_struct{int tag;void*f1;
void*f2;};struct Cyc_Absyn_DoubleType_struct{int tag;int f1;};struct Cyc_Absyn_ArrayType_struct{
int tag;struct Cyc_Absyn_ArrayInfo f1;};struct Cyc_Absyn_FnType_struct{int tag;struct
Cyc_Absyn_FnInfo f1;};struct Cyc_Absyn_TupleType_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_AggrType_struct{int tag;struct Cyc_Absyn_AggrInfo f1;};struct
Cyc_Absyn_AnonAggrType_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_struct{
int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_RgnHandleType_struct{int tag;void*
f1;};struct Cyc_Absyn_DynRgnType_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_TypedefType_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Typedefdecl*f3;
void**f4;};struct Cyc_Absyn_ValueofType_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_TagType_struct{int tag;void*f1;};struct Cyc_Absyn_AccessEff_struct{
int tag;void*f1;};struct Cyc_Absyn_JoinEff_struct{int tag;struct Cyc_List_List*f1;};
struct Cyc_Absyn_RgnsEff_struct{int tag;void*f1;};struct Cyc_Absyn_NoTypes_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*f2;};struct Cyc_Absyn_WithTypes_struct{
int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;struct Cyc_Core_Opt*
f4;struct Cyc_List_List*f5;};struct Cyc_Absyn_Regparm_att_struct{int tag;int f1;};
struct Cyc_Absyn_Aligned_att_struct{int tag;int f1;};struct Cyc_Absyn_Section_att_struct{
int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Format_att_struct{int tag;void*f1;
int f2;int f3;};struct Cyc_Absyn_Initializes_att_struct{int tag;int f1;};struct Cyc_Absyn_Mode_att_struct{
int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Carray_mod_struct{int tag;struct
Cyc_Absyn_Conref*f1;struct Cyc_Position_Segment*f2;};struct Cyc_Absyn_ConstArray_mod_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Conref*f2;struct Cyc_Position_Segment*
f3;};struct Cyc_Absyn_Pointer_mod_struct{int tag;struct Cyc_Absyn_PtrAtts f1;struct
Cyc_Absyn_Tqual f2;};struct Cyc_Absyn_Function_mod_struct{int tag;void*f1;};struct
Cyc_Absyn_TypeParams_mod_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*
f2;int f3;};struct Cyc_Absyn_Attributes_mod_struct{int tag;struct Cyc_Position_Segment*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Char_c_struct{int tag;void*f1;char f2;
};struct Cyc_Absyn_Short_c_struct{int tag;void*f1;short f2;};struct Cyc_Absyn_Int_c_struct{
int tag;void*f1;int f2;};struct Cyc_Absyn_LongLong_c_struct{int tag;void*f1;
long long f2;};struct Cyc_Absyn_Float_c_struct{int tag;struct _dyneither_ptr f1;};
struct Cyc_Absyn_String_c_struct{int tag;struct _dyneither_ptr f1;};struct Cyc_Absyn_Null_c_struct{
int tag;};union Cyc_Absyn_Cnst_union{struct Cyc_Absyn_Char_c_struct Char_c;struct Cyc_Absyn_Short_c_struct
Short_c;struct Cyc_Absyn_Int_c_struct Int_c;struct Cyc_Absyn_LongLong_c_struct
LongLong_c;struct Cyc_Absyn_Float_c_struct Float_c;struct Cyc_Absyn_String_c_struct
String_c;struct Cyc_Absyn_Null_c_struct Null_c;};struct Cyc_Absyn_VarargCallInfo{
int num_varargs;struct Cyc_List_List*injectors;struct Cyc_Absyn_VarargInfo*vai;};
struct Cyc_Absyn_StructField_struct{int tag;struct _dyneither_ptr*f1;};struct Cyc_Absyn_TupleIndex_struct{
int tag;unsigned int f1;};struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*
rgn;void**elt_type;struct Cyc_Absyn_Exp*num_elts;int fat_result;};struct Cyc_Absyn_Const_e_struct{
int tag;union Cyc_Absyn_Cnst_union f1;};struct Cyc_Absyn_Var_e_struct{int tag;struct
_tuple0*f1;void*f2;};struct Cyc_Absyn_UnknownId_e_struct{int tag;struct _tuple0*f1;
};struct Cyc_Absyn_Primop_e_struct{int tag;void*f1;struct Cyc_List_List*f2;};struct
Cyc_Absyn_AssignOp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;
struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;void*f2;};struct Cyc_Absyn_Conditional_e_struct{int tag;struct Cyc_Absyn_Exp*f1;
struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_And_e_struct{int
tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Or_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_SeqExp_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnknownCall_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_FnCall_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*
f3;};struct Cyc_Absyn_Throw_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoInstantiate_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_struct{
int tag;void*f1;struct Cyc_Absyn_Exp*f2;int f3;void*f4;};struct Cyc_Absyn_Address_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_New_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Sizeoftyp_e_struct{int tag;void*f1;};
struct Cyc_Absyn_Sizeofexp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_struct{
int tag;void*f1;void*f2;};struct Cyc_Absyn_Gentyp_e_struct{int tag;struct Cyc_List_List*
f1;void*f2;};struct Cyc_Absyn_Deref_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_AggrMember_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
_dyneither_ptr*f2;};struct Cyc_Absyn_AggrArrow_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct _dyneither_ptr*f2;};struct Cyc_Absyn_Subscript_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_struct{int tag;struct Cyc_List_List*
f1;};struct _tuple1{struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Tqual f2;void*f3;};
struct Cyc_Absyn_CompoundLit_e_struct{int tag;struct _tuple1*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Array_e_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Comprehension_e_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;
int f4;};struct Cyc_Absyn_Struct_e_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*
f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*f4;};struct Cyc_Absyn_AnonStruct_e_struct{
int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Tunion_e_struct{int tag;
struct Cyc_List_List*f1;struct Cyc_Absyn_Tuniondecl*f2;struct Cyc_Absyn_Tunionfield*
f3;};struct Cyc_Absyn_Enum_e_struct{int tag;struct _tuple0*f1;struct Cyc_Absyn_Enumdecl*
f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_AnonEnum_e_struct{int tag;
struct _tuple0*f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_Malloc_e_struct{
int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_Swap_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_UnresolvedMem_e_struct{
int tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Valueof_e_struct{int tag;void*f1;
};struct Cyc_Absyn_Exp{struct Cyc_Core_Opt*topt;void*r;struct Cyc_Position_Segment*
loc;void*annot;};struct Cyc_Absyn_Exp_s_struct{int tag;struct Cyc_Absyn_Exp*f1;};
struct Cyc_Absyn_Seq_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_Absyn_Stmt*
f2;};struct Cyc_Absyn_Return_s_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_IfThenElse_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};
struct _tuple2{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_While_s_struct{
int tag;struct _tuple2 f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Continue_s_struct{int tag;struct
Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Goto_s_struct{int tag;struct _dyneither_ptr*f1;
struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_For_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct _tuple2 f2;struct _tuple2 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Fallthru_s_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_struct{
int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Label_s_struct{
int tag;struct _dyneither_ptr*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;struct _tuple2 f2;};struct Cyc_Absyn_TryCatch_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ResetRegion_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Stmt{void*r;struct Cyc_Position_Segment*
loc;struct Cyc_List_List*non_local_preds;int try_depth;void*annot;};struct Cyc_Absyn_Var_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_Reference_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;struct Cyc_Absyn_Pat*f2;};struct Cyc_Absyn_TagInt_p_struct{
int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*f2;};struct Cyc_Absyn_Tuple_p_struct{
int tag;struct Cyc_List_List*f1;int f2;};struct Cyc_Absyn_Pointer_p_struct{int tag;
struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Aggr_p_struct{int tag;struct Cyc_Absyn_AggrInfo
f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;int f4;};struct Cyc_Absyn_Tunion_p_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;struct Cyc_List_List*
f3;int f4;};struct Cyc_Absyn_Int_p_struct{int tag;void*f1;int f2;};struct Cyc_Absyn_Char_p_struct{
int tag;char f1;};struct Cyc_Absyn_Float_p_struct{int tag;struct _dyneither_ptr f1;};
struct Cyc_Absyn_Enum_p_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Absyn_AnonEnum_p_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Absyn_UnknownId_p_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;int f3;};struct Cyc_Absyn_Exp_p_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pat{void*r;struct Cyc_Core_Opt*
topt;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Switch_clause{struct Cyc_Absyn_Pat*
pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*where_clause;struct Cyc_Absyn_Stmt*
body;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Global_b_struct{int tag;
struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Param_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Local_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{void*sc;struct
_tuple0*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;
struct Cyc_Core_Opt*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{
void*sc;int is_inline;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
effect;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*
cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;struct Cyc_Core_Opt*
cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_Absyn_Vardecl*fn_vardecl;
struct Cyc_List_List*attributes;};struct Cyc_Absyn_Aggrfield{struct _dyneither_ptr*
name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*
attributes;};struct Cyc_Absyn_AggrdeclImpl{struct Cyc_List_List*exist_vars;struct
Cyc_List_List*rgn_po;struct Cyc_List_List*fields;};struct Cyc_Absyn_Aggrdecl{void*
kind;void*sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Absyn_AggrdeclImpl*
impl;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Tunionfield{struct _tuple0*
name;struct Cyc_List_List*typs;struct Cyc_Position_Segment*loc;void*sc;};struct Cyc_Absyn_Tuniondecl{
void*sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*fields;int
is_xtunion;int is_flat;};struct Cyc_Absyn_Enumfield{struct _tuple0*name;struct Cyc_Absyn_Exp*
tag;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Enumdecl{void*sc;struct
_tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{struct
_tuple0*name;struct Cyc_Absyn_Tqual tq;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
kind;struct Cyc_Core_Opt*defn;struct Cyc_List_List*atts;};struct Cyc_Absyn_Var_d_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Let_d_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*
f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Letv_d_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_Region_d_struct{int tag;struct Cyc_Absyn_Tvar*f1;struct Cyc_Absyn_Vardecl*
f2;int f3;struct Cyc_Absyn_Exp*f4;};struct Cyc_Absyn_Alias_d_struct{int tag;struct
Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Tvar*f2;struct Cyc_Absyn_Vardecl*f3;};struct Cyc_Absyn_Aggr_d_struct{
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Absyn_Tunion_d_struct{int tag;
struct Cyc_Absyn_Tuniondecl*f1;};struct Cyc_Absyn_Enum_d_struct{int tag;struct Cyc_Absyn_Enumdecl*
f1;};struct Cyc_Absyn_Typedef_d_struct{int tag;struct Cyc_Absyn_Typedefdecl*f1;};
struct Cyc_Absyn_Namespace_d_struct{int tag;struct _dyneither_ptr*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_Using_d_struct{int tag;struct _tuple0*f1;struct Cyc_List_List*
f2;};struct Cyc_Absyn_ExternC_d_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_ExternCinclude_d_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Decl{void*
r;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_ArrayElement_struct{int tag;
struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_FieldName_struct{int tag;struct
_dyneither_ptr*f1;};extern char Cyc_Absyn_EmptyAnnot[15];int Cyc_Absyn_qvar_cmp(
struct _tuple0*,struct _tuple0*);struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual(struct
Cyc_Position_Segment*);struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual(struct Cyc_Position_Segment*);
struct Cyc_Absyn_Conref*Cyc_Absyn_empty_conref();void*Cyc_Absyn_compress_kb(void*);
void*Cyc_Absyn_new_evar(struct Cyc_Core_Opt*k,struct Cyc_Core_Opt*tenv);void*Cyc_Absyn_string_typ(
void*rgn);void*Cyc_Absyn_const_string_typ(void*rgn);void*Cyc_Absyn_dyneither_typ(
void*t,void*rgn,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Conref*zero_term);void*
Cyc_Absyn_array_typ(void*elt_type,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Exp*
num_elts,struct Cyc_Absyn_Conref*zero_term,struct Cyc_Position_Segment*ztloc);
struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(unsigned int,struct Cyc_Position_Segment*);
struct _dyneither_ptr Cyc_Absyn_attribute2string(void*);struct Cyc___cycFILE;struct
Cyc_Cstdio___abstractFILE;struct Cyc_String_pa_struct{int tag;struct _dyneither_ptr
f1;};struct Cyc_Int_pa_struct{int tag;unsigned long f1;};struct Cyc_Double_pa_struct{
int tag;double f1;};struct Cyc_LongDouble_pa_struct{int tag;long double f1;};struct
Cyc_ShortPtr_pa_struct{int tag;short*f1;};struct Cyc_IntPtr_pa_struct{int tag;
unsigned long*f1;};struct Cyc_ShortPtr_sa_struct{int tag;short*f1;};struct Cyc_UShortPtr_sa_struct{
int tag;unsigned short*f1;};struct Cyc_IntPtr_sa_struct{int tag;int*f1;};struct Cyc_UIntPtr_sa_struct{
int tag;unsigned int*f1;};struct Cyc_StringPtr_sa_struct{int tag;struct
_dyneither_ptr f1;};struct Cyc_DoublePtr_sa_struct{int tag;double*f1;};struct Cyc_FloatPtr_sa_struct{
int tag;float*f1;};struct Cyc_CharPtr_sa_struct{int tag;struct _dyneither_ptr f1;};
extern char Cyc_FileCloseError[19];extern char Cyc_FileOpenError[18];struct Cyc_FileOpenError_struct{
char*tag;struct _dyneither_ptr f1;};struct Cyc_PP_Ppstate;struct Cyc_PP_Out;struct
Cyc_PP_Doc;struct Cyc_Absynpp_Params{int expand_typedefs: 1;int qvar_to_Cids: 1;int
add_cyc_prefix: 1;int to_VC: 1;int decls_first: 1;int rewrite_temp_tvars: 1;int
print_all_tvars: 1;int print_all_kinds: 1;int print_all_effects: 1;int
print_using_stmts: 1;int print_externC_stmts: 1;int print_full_evars: 1;int
print_zeroterm: 1;int generate_line_directives: 1;int use_curr_namespace: 1;struct Cyc_List_List*
curr_namespace;};void Cyc_Absynpp_set_params(struct Cyc_Absynpp_Params*fs);extern
struct Cyc_Absynpp_Params Cyc_Absynpp_tc_params_r;struct _dyneither_ptr Cyc_Absynpp_typ2string(
void*);struct _dyneither_ptr Cyc_Absynpp_qvar2string(struct _tuple0*);struct Cyc_Iter_Iter{
void*env;int(*next)(void*env,void*dest);};int Cyc_Iter_next(struct Cyc_Iter_Iter,
void*);struct Cyc_Set_Set;struct Cyc_Set_Set*Cyc_Set_rempty(struct _RegionHandle*r,
int(*cmp)(void*,void*));struct Cyc_Set_Set*Cyc_Set_rinsert(struct _RegionHandle*r,
struct Cyc_Set_Set*s,void*elt);int Cyc_Set_member(struct Cyc_Set_Set*s,void*elt);
extern char Cyc_Set_Absent[11];struct Cyc_Dict_T;struct Cyc_Dict_Dict{int(*rel)(void*,
void*);struct _RegionHandle*r;struct Cyc_Dict_T*t;};extern char Cyc_Dict_Present[12];
extern char Cyc_Dict_Absent[11];int Cyc_Dict_is_empty(struct Cyc_Dict_Dict d);int Cyc_Dict_member(
struct Cyc_Dict_Dict d,void*k);struct Cyc_Dict_Dict Cyc_Dict_insert(struct Cyc_Dict_Dict
d,void*k,void*v);void*Cyc_Dict_lookup(struct Cyc_Dict_Dict d,void*k);void**Cyc_Dict_lookup_opt(
struct Cyc_Dict_Dict d,void*k);struct Cyc_Dict_Dict Cyc_Dict_rmap_c(struct
_RegionHandle*,void*(*f)(void*,void*),void*env,struct Cyc_Dict_Dict d);struct
_tuple3{void*f1;void*f2;};struct _tuple3*Cyc_Dict_rchoose(struct _RegionHandle*r,
struct Cyc_Dict_Dict d);struct _tuple3*Cyc_Dict_rchoose(struct _RegionHandle*,struct
Cyc_Dict_Dict d);struct Cyc_Dict_Dict Cyc_Dict_rfilter_c(struct _RegionHandle*,int(*
f)(void*,void*,void*),void*env,struct Cyc_Dict_Dict d);struct Cyc_Iter_Iter Cyc_Dict_make_iter(
struct _RegionHandle*rgn,struct Cyc_Dict_Dict d);struct Cyc_RgnOrder_RgnPO;struct Cyc_RgnOrder_RgnPO*
Cyc_RgnOrder_initial_fn_po(struct _RegionHandle*,struct Cyc_List_List*tvs,struct
Cyc_List_List*po,void*effect,struct Cyc_Absyn_Tvar*fst_rgn,struct Cyc_Position_Segment*);
struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_outlives_constraint(struct
_RegionHandle*,struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn,struct Cyc_Position_Segment*
loc);struct Cyc_RgnOrder_RgnPO*Cyc_RgnOrder_add_youngest(struct _RegionHandle*,
struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*rgn,int resetable,int opened);int
Cyc_RgnOrder_is_region_resetable(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_Absyn_Tvar*
r);int Cyc_RgnOrder_effect_outlives(struct Cyc_RgnOrder_RgnPO*po,void*eff,void*rgn);
int Cyc_RgnOrder_satisfies_constraints(struct Cyc_RgnOrder_RgnPO*po,struct Cyc_List_List*
constraints,void*default_bound,int do_pin);int Cyc_RgnOrder_eff_outlives_eff(
struct Cyc_RgnOrder_RgnPO*po,void*eff1,void*eff2);void Cyc_RgnOrder_print_region_po(
struct Cyc_RgnOrder_RgnPO*po);struct Cyc_Tcenv_CList{void*hd;struct Cyc_Tcenv_CList*
tl;};struct Cyc_Tcenv_VarRes_struct{int tag;void*f1;};struct Cyc_Tcenv_AggrRes_struct{
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_TunionRes_struct{int tag;
struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Tcenv_EnumRes_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_struct{
int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_Genv{struct
_RegionHandle*grgn;struct Cyc_Set_Set*namespaces;struct Cyc_Dict_Dict aggrdecls;
struct Cyc_Dict_Dict tuniondecls;struct Cyc_Dict_Dict enumdecls;struct Cyc_Dict_Dict
typedefs;struct Cyc_Dict_Dict ordinaries;struct Cyc_List_List*availables;};struct
Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Stmt_j_struct{int tag;struct Cyc_Absyn_Stmt*f1;};
struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict ae;struct Cyc_Tcenv_Fenv*
le;int allow_valueof;};struct _RegionHandle*Cyc_Tcenv_get_fnrgn(struct Cyc_Tcenv_Tenv*);
struct Cyc_Tcenv_Genv*Cyc_Tcenv_empty_genv(struct _RegionHandle*);struct Cyc_Tcenv_Fenv*
Cyc_Tcenv_new_fenv(struct _RegionHandle*,struct Cyc_Position_Segment*,struct Cyc_Absyn_Fndecl*);
struct Cyc_List_List*Cyc_Tcenv_resolve_namespace(struct Cyc_Tcenv_Tenv*,struct Cyc_Position_Segment*,
struct _dyneither_ptr*,struct Cyc_List_List*);struct Cyc_Absyn_Tuniondecl***Cyc_Tcenv_lookup_xtuniondecl(
struct _RegionHandle*,struct Cyc_Tcenv_Tenv*,struct Cyc_Position_Segment*,struct
_tuple0*);int Cyc_Tcenv_all_labels_resolved(struct Cyc_Tcenv_Tenv*);void Cyc_Tcenv_check_delayed_effects(
struct Cyc_Tcenv_Tenv*te);void Cyc_Tcenv_check_delayed_constraints(struct Cyc_Tcenv_Tenv*
te);void*Cyc_Tcutil_impos(struct _dyneither_ptr fmt,struct _dyneither_ptr ap);void
Cyc_Tcutil_terr(struct Cyc_Position_Segment*,struct _dyneither_ptr fmt,struct
_dyneither_ptr ap);void Cyc_Tcutil_warn(struct Cyc_Position_Segment*,struct
_dyneither_ptr fmt,struct _dyneither_ptr ap);void*Cyc_Tcutil_compress(void*t);int
Cyc_Tcutil_coerce_assign(struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Exp*,void*);int
Cyc_Tcutil_is_function_type(void*t);void*Cyc_Tcutil_kind_to_bound(void*k);void
Cyc_Tcutil_explain_failure();int Cyc_Tcutil_unify(void*,void*);void*Cyc_Tcutil_fndecl2typ(
struct Cyc_Absyn_Fndecl*);void Cyc_Tcutil_check_bitfield(struct Cyc_Position_Segment*
loc,struct Cyc_Tcenv_Tenv*te,void*field_typ,struct Cyc_Absyn_Exp*width,struct
_dyneither_ptr*fn);void Cyc_Tcutil_check_valid_toplevel_type(struct Cyc_Position_Segment*,
struct Cyc_Tcenv_Tenv*,void*);void Cyc_Tcutil_check_fndecl_valid_type(struct Cyc_Position_Segment*,
struct Cyc_Tcenv_Tenv*,struct Cyc_Absyn_Fndecl*);void Cyc_Tcutil_check_type(struct
Cyc_Position_Segment*,struct Cyc_Tcenv_Tenv*,struct Cyc_List_List*bound_tvars,void*
k,int allow_evars,void*);void Cyc_Tcutil_check_unique_tvars(struct Cyc_Position_Segment*,
struct Cyc_List_List*);int Cyc_Tcutil_is_noalias_pointer_or_aggr(struct
_RegionHandle*,void*t);void Cyc_Tcutil_add_tvar_identities(struct Cyc_List_List*);
int Cyc_Tcutil_bits_only(void*t);int Cyc_Tcutil_is_const_exp(struct Cyc_Tcenv_Tenv*
te,struct Cyc_Absyn_Exp*e);int Cyc_Tcutil_supports_default(void*);int Cyc_Tcutil_extract_const_from_typedef(
struct Cyc_Position_Segment*,int declared_const,void*);struct Cyc_List_List*Cyc_Tcutil_transfer_fn_type_atts(
void*t,struct Cyc_List_List*atts);void*Cyc_Tcexp_tcExpInitializer(struct Cyc_Tcenv_Tenv*,
void**,struct Cyc_Absyn_Exp*);struct Cyc_Tcexp_TestEnv{struct _tuple3*eq;int isTrue;
};struct Cyc_Tcexp_TestEnv Cyc_Tcexp_tcTest(struct Cyc_Tcenv_Tenv*te,struct Cyc_Absyn_Exp*
e,struct _dyneither_ptr msg_part);void Cyc_Tcstmt_tcStmt(struct Cyc_Tcenv_Tenv*te,
struct Cyc_Absyn_Stmt*s,int new_block);struct _tuple4{unsigned int f1;int f2;};struct
_tuple4 Cyc_Evexp_eval_const_uint_exp(struct Cyc_Absyn_Exp*e);void Cyc_Tc_tc(struct
_RegionHandle*,struct Cyc_Tcenv_Tenv*te,int var_default_init,struct Cyc_List_List*
ds);struct Cyc_List_List*Cyc_Tc_treeshake(struct Cyc_Tcenv_Tenv*te,struct Cyc_List_List*);
void Cyc_Tc_tcAggrdecl(struct Cyc_Tcenv_Tenv*,struct Cyc_Tcenv_Genv*,struct Cyc_Position_Segment*,
struct Cyc_Absyn_Aggrdecl*);void Cyc_Tc_tcTuniondecl(struct Cyc_Tcenv_Tenv*,struct
Cyc_Tcenv_Genv*,struct Cyc_Position_Segment*,struct Cyc_Absyn_Tuniondecl*);void Cyc_Tc_tcEnumdecl(
struct Cyc_Tcenv_Tenv*,struct Cyc_Tcenv_Genv*,struct Cyc_Position_Segment*,struct
Cyc_Absyn_Enumdecl*);extern char Cyc_Tcdecl_Incompatible[17];struct Cyc_Tcdecl_Xtunionfielddecl{
struct Cyc_Absyn_Tuniondecl*base;struct Cyc_Absyn_Tunionfield*field;};struct Cyc_Absyn_Aggrdecl*
Cyc_Tcdecl_merge_aggrdecl(struct Cyc_Absyn_Aggrdecl*d0,struct Cyc_Absyn_Aggrdecl*
d1,struct Cyc_Position_Segment*loc,struct _dyneither_ptr*msg);struct Cyc_Absyn_Tuniondecl*
Cyc_Tcdecl_merge_tuniondecl(struct Cyc_Absyn_Tuniondecl*d0,struct Cyc_Absyn_Tuniondecl*
d1,struct Cyc_Position_Segment*loc,struct _dyneither_ptr*msg);struct Cyc_Absyn_Enumdecl*
Cyc_Tcdecl_merge_enumdecl(struct Cyc_Absyn_Enumdecl*d0,struct Cyc_Absyn_Enumdecl*
d1,struct Cyc_Position_Segment*loc,struct _dyneither_ptr*msg);void*Cyc_Tcdecl_merge_binding(
void*d0,void*d1,struct Cyc_Position_Segment*loc,struct _dyneither_ptr*msg);struct
Cyc_List_List*Cyc_Tcdecl_sort_xtunion_fields(struct Cyc_List_List*f,int*res,
struct _dyneither_ptr*v,struct Cyc_Position_Segment*loc,struct _dyneither_ptr*msg);
struct Cyc_Tcgenrep_RepInfo;struct Cyc_Dict_Dict Cyc_Tcgenrep_empty_typerep_dict();
struct _tuple5{struct Cyc_Dict_Dict f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Exp*
f3;};struct _tuple5 Cyc_Tcgenrep_tcGenrep(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*
ge,struct Cyc_Position_Segment*loc,void*type,struct Cyc_Dict_Dict dict);static char
_tmp0[1]="";static struct _dyneither_ptr Cyc_Tc_tc_msg_c={_tmp0,_tmp0,_tmp0 + 1};
static struct _dyneither_ptr*Cyc_Tc_tc_msg=(struct _dyneither_ptr*)& Cyc_Tc_tc_msg_c;
struct _tuple6{struct Cyc_Position_Segment*f1;struct _tuple0*f2;int f3;};static int
Cyc_Tc_export_member(struct _tuple0*x,struct Cyc_List_List*exports);static int Cyc_Tc_export_member(
struct _tuple0*x,struct Cyc_List_List*exports){for(0;exports != 0;exports=exports->tl){
struct _tuple6*_tmp1=(struct _tuple6*)exports->hd;if(Cyc_Absyn_qvar_cmp(x,(*_tmp1).f2)
== 0){(*_tmp1).f3=1;return 1;}}return 0;}struct _tuple7{void*f1;int f2;};static void
Cyc_Tc_tcVardecl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*
loc,struct Cyc_Absyn_Vardecl*vd,int check_var_init,int ignore_init,struct Cyc_List_List**
exports);static void Cyc_Tc_tcVardecl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*
ge,struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Vardecl*vd,int check_var_init,
int ignore_init,struct Cyc_List_List**exports){struct Cyc_Absyn_Vardecl _tmp3;void*
_tmp4;struct _tuple0*_tmp5;struct _tuple0 _tmp6;union Cyc_Absyn_Nmspace_union _tmp7;
struct _dyneither_ptr*_tmp8;void*_tmp9;struct Cyc_Absyn_Exp*_tmpA;struct Cyc_List_List*
_tmpB;struct Cyc_Absyn_Vardecl*_tmp2=vd;_tmp3=*_tmp2;_tmp4=(void*)_tmp3.sc;_tmp5=
_tmp3.name;_tmp6=*_tmp5;_tmp7=_tmp6.f1;_tmp8=_tmp6.f2;_tmp9=(void*)_tmp3.type;
_tmpA=_tmp3.initializer;_tmpB=_tmp3.attributes;{union Cyc_Absyn_Nmspace_union
_tmpC=_tmp7;struct Cyc_List_List*_tmpD;struct Cyc_List_List*_tmpE;_LL1: if((_tmpC.Rel_n).tag
!= 1)goto _LL3;_tmpD=(_tmpC.Rel_n).f1;if(_tmpD != 0)goto _LL3;_LL2: goto _LL4;_LL3:
if((_tmpC.Abs_n).tag != 2)goto _LL5;_tmpE=(_tmpC.Abs_n).f1;if(_tmpE != 0)goto _LL5;
_LL4: goto _LL0;_LL5:;_LL6:{const char*_tmp229;void*_tmp228[1];struct Cyc_String_pa_struct
_tmp227;(_tmp227.tag=0,((_tmp227.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_qvar2string(vd->name)),((_tmp228[0]=& _tmp227,Cyc_Tcutil_terr(loc,((
_tmp229="qualified variable declarations are not implemented (%s)",
_tag_dyneither(_tmp229,sizeof(char),57))),_tag_dyneither(_tmp228,sizeof(void*),1)))))));}
return;_LL0:;}{union Cyc_Absyn_Nmspace_union _tmp22A;(*vd->name).f1=(union Cyc_Absyn_Nmspace_union)(((
_tmp22A.Abs_n).tag=2,(((_tmp22A.Abs_n).f1=te->ns,_tmp22A))));}{void*_tmp13=Cyc_Tcutil_compress(
_tmp9);struct Cyc_Absyn_ArrayInfo _tmp14;void*_tmp15;struct Cyc_Absyn_Tqual _tmp16;
struct Cyc_Absyn_Exp*_tmp17;struct Cyc_Absyn_Conref*_tmp18;struct Cyc_Position_Segment*
_tmp19;_LL8: if(_tmp13 <= (void*)4)goto _LLA;if(*((int*)_tmp13)!= 7)goto _LLA;_tmp14=((
struct Cyc_Absyn_ArrayType_struct*)_tmp13)->f1;_tmp15=(void*)_tmp14.elt_type;
_tmp16=_tmp14.tq;_tmp17=_tmp14.num_elts;if(_tmp17 != 0)goto _LLA;_tmp18=_tmp14.zero_term;
_tmp19=_tmp14.zt_loc;if(!(_tmpA != 0))goto _LLA;_LL9:{void*_tmp1A=(void*)_tmpA->r;
union Cyc_Absyn_Cnst_union _tmp1B;struct _dyneither_ptr _tmp1C;struct Cyc_Absyn_Exp*
_tmp1D;struct Cyc_List_List*_tmp1E;struct Cyc_List_List*_tmp1F;_LLD: if(*((int*)
_tmp1A)!= 0)goto _LLF;_tmp1B=((struct Cyc_Absyn_Const_e_struct*)_tmp1A)->f1;if(((((
struct Cyc_Absyn_Const_e_struct*)_tmp1A)->f1).String_c).tag != 5)goto _LLF;_tmp1C=(
_tmp1B.String_c).f1;_LLE: _tmp9=(void*)(vd->type=(void*)Cyc_Absyn_array_typ(
_tmp15,_tmp16,(struct Cyc_Absyn_Exp*)Cyc_Absyn_uint_exp(_get_dyneither_size(
_tmp1C,sizeof(char)),0),_tmp18,_tmp19));goto _LLC;_LLF: if(*((int*)_tmp1A)!= 29)
goto _LL11;_tmp1D=((struct Cyc_Absyn_Comprehension_e_struct*)_tmp1A)->f2;_LL10:
_tmp9=(void*)(vd->type=(void*)Cyc_Absyn_array_typ(_tmp15,_tmp16,(struct Cyc_Absyn_Exp*)
_tmp1D,_tmp18,_tmp19));goto _LLC;_LL11: if(*((int*)_tmp1A)!= 37)goto _LL13;_tmp1E=((
struct Cyc_Absyn_UnresolvedMem_e_struct*)_tmp1A)->f2;_LL12: _tmp1F=_tmp1E;goto
_LL14;_LL13: if(*((int*)_tmp1A)!= 28)goto _LL15;_tmp1F=((struct Cyc_Absyn_Array_e_struct*)
_tmp1A)->f1;_LL14: _tmp9=(void*)(vd->type=(void*)Cyc_Absyn_array_typ(_tmp15,
_tmp16,(struct Cyc_Absyn_Exp*)Cyc_Absyn_uint_exp((unsigned int)((int(*)(struct Cyc_List_List*
x))Cyc_List_length)(_tmp1F),0),_tmp18,_tmp19));goto _LLC;_LL15:;_LL16: goto _LLC;
_LLC:;}goto _LL7;_LLA:;_LLB: goto _LL7;_LL7:;}Cyc_Tcutil_check_valid_toplevel_type(
loc,te,_tmp9);(vd->tq).real_const=Cyc_Tcutil_extract_const_from_typedef(loc,(vd->tq).print_const,
_tmp9);{void*_tmp20=Cyc_Tcutil_compress(_tmp9);_LL18: if(_tmp20 <= (void*)4)goto
_LL1A;if(*((int*)_tmp20)!= 7)goto _LL1A;_LL19: vd->escapes=0;goto _LL17;_LL1A:;
_LL1B: vd->escapes=1;goto _LL17;_LL17:;}if(Cyc_Tcutil_is_function_type(_tmp9))
_tmpB=Cyc_Tcutil_transfer_fn_type_atts(_tmp9,_tmpB);if(_tmp4 == (void*)3  || _tmp4
== (void*)4){if(_tmpA != 0){const char*_tmp22D;void*_tmp22C;(_tmp22C=0,Cyc_Tcutil_terr(
loc,((_tmp22D="extern declaration should not have initializer",_tag_dyneither(
_tmp22D,sizeof(char),47))),_tag_dyneither(_tmp22C,sizeof(void*),0)));}}else{if(!
Cyc_Tcutil_is_function_type(_tmp9)){for(0;_tmpB != 0;_tmpB=_tmpB->tl){void*_tmp23=(
void*)_tmpB->hd;_LL1D: if(_tmp23 <= (void*)17)goto _LL21;if(*((int*)_tmp23)!= 1)
goto _LL1F;_LL1E: goto _LL20;_LL1F: if(*((int*)_tmp23)!= 2)goto _LL21;_LL20: goto _LL22;
_LL21: if((int)_tmp23 != 6)goto _LL23;_LL22: goto _LL24;_LL23: if((int)_tmp23 != 7)goto
_LL25;_LL24: goto _LL26;_LL25: if((int)_tmp23 != 8)goto _LL27;_LL26: goto _LL28;_LL27:
if((int)_tmp23 != 9)goto _LL29;_LL28: goto _LL2A;_LL29: if((int)_tmp23 != 10)goto _LL2B;
_LL2A: goto _LL2C;_LL2B: if((int)_tmp23 != 11)goto _LL2D;_LL2C: continue;_LL2D:;_LL2E:{
const char*_tmp232;void*_tmp231[2];struct Cyc_String_pa_struct _tmp230;struct Cyc_String_pa_struct
_tmp22F;(_tmp22F.tag=0,((_tmp22F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_qvar2string(vd->name)),((_tmp230.tag=0,((_tmp230.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Absyn_attribute2string((void*)_tmpB->hd)),((
_tmp231[0]=& _tmp230,((_tmp231[1]=& _tmp22F,Cyc_Tcutil_terr(loc,((_tmp232="bad attribute %s for variable %s",
_tag_dyneither(_tmp232,sizeof(char),33))),_tag_dyneither(_tmp231,sizeof(void*),2)))))))))))));}
goto _LL1C;_LL1C:;}if(_tmpA == 0  || ignore_init){if(check_var_init  && !Cyc_Tcutil_supports_default(
_tmp9)){const char*_tmp237;void*_tmp236[2];struct Cyc_String_pa_struct _tmp235;
struct Cyc_String_pa_struct _tmp234;(_tmp234.tag=0,((_tmp234.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp9)),((_tmp235.tag=
0,((_tmp235.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(
vd->name)),((_tmp236[0]=& _tmp235,((_tmp236[1]=& _tmp234,Cyc_Tcutil_terr(loc,((
_tmp237="initializer required for variable %s of type %s",_tag_dyneither(_tmp237,
sizeof(char),48))),_tag_dyneither(_tmp236,sizeof(void*),2)))))))))))));}}else{
struct Cyc_Absyn_Exp*_tmp2C=(struct Cyc_Absyn_Exp*)_tmpA;void*_tmp2D=Cyc_Tcexp_tcExpInitializer(
te,(void**)& _tmp9,_tmp2C);if(!Cyc_Tcutil_coerce_assign(te,_tmp2C,_tmp9)){{const
char*_tmp23D;void*_tmp23C[3];struct Cyc_String_pa_struct _tmp23B;struct Cyc_String_pa_struct
_tmp23A;struct Cyc_String_pa_struct _tmp239;(_tmp239.tag=0,((_tmp239.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp2D)),((_tmp23A.tag=
0,((_tmp23A.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(
_tmp9)),((_tmp23B.tag=0,((_tmp23B.f1=(struct _dyneither_ptr)((struct
_dyneither_ptr)Cyc_Absynpp_qvar2string(vd->name)),((_tmp23C[0]=& _tmp23B,((
_tmp23C[1]=& _tmp23A,((_tmp23C[2]=& _tmp239,Cyc_Tcutil_terr(loc,((_tmp23D="%s declared with type \n%s\n but initialized with type \n%s",
_tag_dyneither(_tmp23D,sizeof(char),57))),_tag_dyneither(_tmp23C,sizeof(void*),3)))))))))))))))))));}
Cyc_Tcutil_explain_failure();}if(!Cyc_Tcutil_is_const_exp(te,_tmp2C)){const char*
_tmp240;void*_tmp23F;(_tmp23F=0,Cyc_Tcutil_terr(loc,((_tmp240="initializer is not a constant expression",
_tag_dyneither(_tmp240,sizeof(char),41))),_tag_dyneither(_tmp23F,sizeof(void*),0)));}}}
else{for(0;_tmpB != 0;_tmpB=_tmpB->tl){void*_tmp35=(void*)_tmpB->hd;_LL30: if(
_tmp35 <= (void*)17)goto _LL32;if(*((int*)_tmp35)!= 0)goto _LL32;_LL31: goto _LL33;
_LL32: if((int)_tmp35 != 0)goto _LL34;_LL33: goto _LL35;_LL34: if((int)_tmp35 != 1)goto
_LL36;_LL35: goto _LL37;_LL36: if((int)_tmp35 != 2)goto _LL38;_LL37: goto _LL39;_LL38:
if((int)_tmp35 != 3)goto _LL3A;_LL39: goto _LL3B;_LL3A: if(_tmp35 <= (void*)17)goto
_LL3E;if(*((int*)_tmp35)!= 3)goto _LL3C;_LL3B: goto _LL3D;_LL3C: if(*((int*)_tmp35)
!= 4)goto _LL3E;_LL3D: goto _LL3F;_LL3E: if((int)_tmp35 != 16)goto _LL40;_LL3F: goto
_LL41;_LL40: if((int)_tmp35 != 4)goto _LL42;_LL41: {const char*_tmp243;void*_tmp242;(
_tmp242=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp243="tcVardecl: fn type atts in function var decl",_tag_dyneither(_tmp243,
sizeof(char),45))),_tag_dyneither(_tmp242,sizeof(void*),0)));}_LL42: if(_tmp35 <= (
void*)17)goto _LL44;if(*((int*)_tmp35)!= 1)goto _LL44;_LL43: goto _LL45;_LL44: if((
int)_tmp35 != 5)goto _LL46;_LL45:{const char*_tmp247;void*_tmp246[1];struct Cyc_String_pa_struct
_tmp245;(_tmp245.tag=0,((_tmp245.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absyn_attribute2string((void*)_tmpB->hd)),((_tmp246[0]=& _tmp245,Cyc_Tcutil_terr(
loc,((_tmp247="bad attribute %s in function declaration",_tag_dyneither(_tmp247,
sizeof(char),41))),_tag_dyneither(_tmp246,sizeof(void*),1)))))));}goto _LL2F;
_LL46:;_LL47: continue;_LL2F:;}}}{struct _handler_cons _tmp3B;_push_handler(& _tmp3B);{
int _tmp3D=0;if(setjmp(_tmp3B.handler))_tmp3D=1;if(!_tmp3D){{struct _tuple7*_tmp3E=((
struct _tuple7*(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup)(
ge->ordinaries,_tmp8);void*_tmp3F=(*_tmp3E).f1;void*_tmp40;_LL49: if(*((int*)
_tmp3F)!= 0)goto _LL4B;_tmp40=(void*)((struct Cyc_Tcenv_VarRes_struct*)_tmp3F)->f1;
_LL4A: {struct Cyc_Absyn_Global_b_struct _tmp24A;struct Cyc_Absyn_Global_b_struct*
_tmp249;struct Cyc_Absyn_Global_b_struct*_tmp41=(_tmp249=_cycalloc(sizeof(*
_tmp249)),((_tmp249[0]=((_tmp24A.tag=0,((_tmp24A.f1=vd,_tmp24A)))),_tmp249)));
void*_tmp42=Cyc_Tcdecl_merge_binding(_tmp40,(void*)_tmp41,loc,Cyc_Tc_tc_msg);if(
_tmp42 == (void*)0){_npop_handler(0);return;}if(_tmp42 == _tmp40  && (*_tmp3E).f2){
_npop_handler(0);return;}if(exports == 0  || Cyc_Tc_export_member(vd->name,*
exports)){struct Cyc_Tcenv_VarRes_struct*_tmp250;struct Cyc_Tcenv_VarRes_struct
_tmp24F;struct _tuple7*_tmp24E;ge->ordinaries=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict
d,struct _dyneither_ptr*k,struct _tuple7*v))Cyc_Dict_insert)(ge->ordinaries,_tmp8,(
struct _tuple7*)((_tmp24E=_cycalloc(sizeof(*_tmp24E)),((_tmp24E->f1=(void*)((
_tmp250=_cycalloc(sizeof(*_tmp250)),((_tmp250[0]=((_tmp24F.tag=0,((_tmp24F.f1=(
void*)_tmp42,_tmp24F)))),_tmp250)))),((_tmp24E->f2=1,_tmp24E)))))));}
_npop_handler(0);return;}_LL4B: if(*((int*)_tmp3F)!= 1)goto _LL4D;_LL4C:{const char*
_tmp253;void*_tmp252;(_tmp252=0,Cyc_Tcutil_warn(loc,((_tmp253="variable declaration shadows previous struct declaration",
_tag_dyneither(_tmp253,sizeof(char),57))),_tag_dyneither(_tmp252,sizeof(void*),0)));}
goto _LL48;_LL4D: if(*((int*)_tmp3F)!= 2)goto _LL4F;_LL4E:{const char*_tmp256;void*
_tmp255;(_tmp255=0,Cyc_Tcutil_warn(loc,((_tmp256="variable declaration shadows previous [x]tunion constructor",
_tag_dyneither(_tmp256,sizeof(char),60))),_tag_dyneither(_tmp255,sizeof(void*),0)));}
goto _LL48;_LL4F: if(*((int*)_tmp3F)!= 4)goto _LL51;_LL50: goto _LL52;_LL51: if(*((int*)
_tmp3F)!= 3)goto _LL48;_LL52:{const char*_tmp259;void*_tmp258;(_tmp258=0,Cyc_Tcutil_warn(
loc,((_tmp259="variable declaration shadows previous enum tag",_tag_dyneither(
_tmp259,sizeof(char),47))),_tag_dyneither(_tmp258,sizeof(void*),0)));}goto _LL48;
_LL48:;};_pop_handler();}else{void*_tmp3C=(void*)_exn_thrown;void*_tmp4F=_tmp3C;
_LL54: if(_tmp4F != Cyc_Dict_Absent)goto _LL56;_LL55: goto _LL53;_LL56:;_LL57:(void)
_throw(_tmp4F);_LL53:;}}}if(exports == 0  || Cyc_Tc_export_member(vd->name,*
exports)){struct Cyc_Tcenv_VarRes_struct*_tmp268;struct Cyc_Absyn_Global_b_struct*
_tmp267;struct Cyc_Absyn_Global_b_struct _tmp266;struct Cyc_Tcenv_VarRes_struct
_tmp265;struct _tuple7*_tmp264;ge->ordinaries=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict
d,struct _dyneither_ptr*k,struct _tuple7*v))Cyc_Dict_insert)(ge->ordinaries,_tmp8,(
struct _tuple7*)((_tmp264=_cycalloc(sizeof(*_tmp264)),((_tmp264->f1=(void*)((
_tmp268=_cycalloc(sizeof(*_tmp268)),((_tmp268[0]=((_tmp265.tag=0,((_tmp265.f1=(
void*)((void*)((_tmp267=_cycalloc(sizeof(*_tmp267)),((_tmp267[0]=((_tmp266.tag=0,((
_tmp266.f1=vd,_tmp266)))),_tmp267))))),_tmp265)))),_tmp268)))),((_tmp264->f2=0,
_tmp264)))))));}}static int Cyc_Tc_is_main(struct _tuple0*n);static int Cyc_Tc_is_main(
struct _tuple0*n){struct _tuple0 _tmp56;union Cyc_Absyn_Nmspace_union _tmp57;struct
_dyneither_ptr*_tmp58;struct _tuple0*_tmp55=n;_tmp56=*_tmp55;_tmp57=_tmp56.f1;
_tmp58=_tmp56.f2;{union Cyc_Absyn_Nmspace_union _tmp59=_tmp57;struct Cyc_List_List*
_tmp5A;_LL59: if((_tmp59.Abs_n).tag != 2)goto _LL5B;_tmp5A=(_tmp59.Abs_n).f1;if(
_tmp5A != 0)goto _LL5B;_LL5A: {const char*_tmp269;return Cyc_strcmp((struct
_dyneither_ptr)*_tmp58,((_tmp269="main",_tag_dyneither(_tmp269,sizeof(char),5))))
== 0;}_LL5B:;_LL5C: return 0;_LL58:;}}struct _tuple8{struct _dyneither_ptr*f1;struct
Cyc_Absyn_Tqual f2;void*f3;};static void Cyc_Tc_tcFndecl(struct Cyc_Tcenv_Tenv*te,
struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Fndecl*fd,
int ignore_body,struct Cyc_List_List**exports);static void Cyc_Tc_tcFndecl(struct Cyc_Tcenv_Tenv*
te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Fndecl*
fd,int ignore_body,struct Cyc_List_List**exports){struct _dyneither_ptr*v=(*fd->name).f2;
if((void*)fd->sc == (void*)4  && !ignore_body){const char*_tmp26C;void*_tmp26B;(
_tmp26B=0,Cyc_Tcutil_terr(loc,((_tmp26C="extern \"C\" functions cannot be implemented in Cyclone",
_tag_dyneither(_tmp26C,sizeof(char),54))),_tag_dyneither(_tmp26B,sizeof(void*),0)));}{
union Cyc_Absyn_Nmspace_union _tmp5E=(*fd->name).f1;struct Cyc_List_List*_tmp5F;
struct Cyc_List_List*_tmp60;_LL5E: if((_tmp5E.Rel_n).tag != 1)goto _LL60;_tmp5F=(
_tmp5E.Rel_n).f1;if(_tmp5F != 0)goto _LL60;_LL5F: goto _LL5D;_LL60: if((_tmp5E.Abs_n).tag
!= 2)goto _LL62;_tmp60=(_tmp5E.Abs_n).f1;_LL61: {const char*_tmp26F;void*_tmp26E;(
_tmp26E=0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp26F="tc: Abs_n in tcFndecl",_tag_dyneither(_tmp26F,sizeof(char),22))),
_tag_dyneither(_tmp26E,sizeof(void*),0)));}_LL62:;_LL63:{const char*_tmp273;void*
_tmp272[1];struct Cyc_String_pa_struct _tmp271;(_tmp271.tag=0,((_tmp271.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(fd->name)),((
_tmp272[0]=& _tmp271,Cyc_Tcutil_terr(loc,((_tmp273="qualified function declarations are not implemented (%s)",
_tag_dyneither(_tmp273,sizeof(char),57))),_tag_dyneither(_tmp272,sizeof(void*),1)))))));}
return;_LL5D:;}{union Cyc_Absyn_Nmspace_union _tmp274;(*fd->name).f1=(union Cyc_Absyn_Nmspace_union)(((
_tmp274.Abs_n).tag=2,(((_tmp274.Abs_n).f1=te->ns,_tmp274))));}Cyc_Tcutil_check_fndecl_valid_type(
loc,te,fd);{void*t=Cyc_Tcutil_fndecl2typ(fd);fd->attributes=Cyc_Tcutil_transfer_fn_type_atts(
t,fd->attributes);{struct Cyc_List_List*atts=fd->attributes;for(0;atts != 0;atts=
atts->tl){void*_tmp67=(void*)atts->hd;_LL65: if((int)_tmp67 != 5)goto _LL67;_LL66:
goto _LL68;_LL67: if(_tmp67 <= (void*)17)goto _LL69;if(*((int*)_tmp67)!= 1)goto _LL69;
_LL68:{const char*_tmp278;void*_tmp277[1];struct Cyc_String_pa_struct _tmp276;(
_tmp276.tag=0,((_tmp276.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absyn_attribute2string((
void*)atts->hd)),((_tmp277[0]=& _tmp276,Cyc_Tcutil_terr(loc,((_tmp278="bad attribute %s for function",
_tag_dyneither(_tmp278,sizeof(char),30))),_tag_dyneither(_tmp277,sizeof(void*),1)))))));}
goto _LL64;_LL69:;_LL6A: goto _LL64;_LL64:;}}{struct _handler_cons _tmp6B;
_push_handler(& _tmp6B);{int _tmp6D=0;if(setjmp(_tmp6B.handler))_tmp6D=1;if(!
_tmp6D){{struct _tuple7*_tmp6E=((struct _tuple7*(*)(struct Cyc_Dict_Dict d,struct
_dyneither_ptr*k))Cyc_Dict_lookup)(ge->ordinaries,v);void*_tmp6F=(*_tmp6E).f1;
void*_tmp70;_LL6C: if(*((int*)_tmp6F)!= 0)goto _LL6E;_tmp70=(void*)((struct Cyc_Tcenv_VarRes_struct*)
_tmp6F)->f1;_LL6D: {struct Cyc_Absyn_Funname_b_struct _tmp27B;struct Cyc_Absyn_Funname_b_struct*
_tmp27A;struct Cyc_Absyn_Funname_b_struct*_tmp71=(_tmp27A=_cycalloc(sizeof(*
_tmp27A)),((_tmp27A[0]=((_tmp27B.tag=1,((_tmp27B.f1=fd,_tmp27B)))),_tmp27A)));
void*_tmp72=Cyc_Tcdecl_merge_binding(_tmp70,(void*)_tmp71,loc,Cyc_Tc_tc_msg);if(
_tmp72 == (void*)0)goto _LL6B;if(_tmp72 == _tmp70  && (*_tmp6E).f2)goto _LL6B;if(
exports == 0  || Cyc_Tc_export_member(fd->name,*exports)){struct Cyc_Tcenv_VarRes_struct*
_tmp281;struct Cyc_Tcenv_VarRes_struct _tmp280;struct _tuple7*_tmp27F;ge->ordinaries=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k,struct _tuple7*
v))Cyc_Dict_insert)(ge->ordinaries,v,(struct _tuple7*)((_tmp27F=_cycalloc(sizeof(*
_tmp27F)),((_tmp27F->f1=(void*)((_tmp281=_cycalloc(sizeof(*_tmp281)),((_tmp281[0]=((
_tmp280.tag=0,((_tmp280.f1=(void*)_tmp72,_tmp280)))),_tmp281)))),((_tmp27F->f2=1,
_tmp27F)))))));}goto _LL6B;}_LL6E: if(*((int*)_tmp6F)!= 1)goto _LL70;_LL6F:{const
char*_tmp284;void*_tmp283;(_tmp283=0,Cyc_Tcutil_warn(loc,((_tmp284="function declaration shadows previous type declaration",
_tag_dyneither(_tmp284,sizeof(char),55))),_tag_dyneither(_tmp283,sizeof(void*),0)));}
goto _LL6B;_LL70: if(*((int*)_tmp6F)!= 2)goto _LL72;_LL71:{const char*_tmp287;void*
_tmp286;(_tmp286=0,Cyc_Tcutil_warn(loc,((_tmp287="function declaration shadows previous [x]tunion constructor",
_tag_dyneither(_tmp287,sizeof(char),60))),_tag_dyneither(_tmp286,sizeof(void*),0)));}
goto _LL6B;_LL72: if(*((int*)_tmp6F)!= 4)goto _LL74;_LL73: goto _LL75;_LL74: if(*((int*)
_tmp6F)!= 3)goto _LL6B;_LL75:{const char*_tmp28A;void*_tmp289;(_tmp289=0,Cyc_Tcutil_warn(
loc,((_tmp28A="function declaration shadows previous enum tag",_tag_dyneither(
_tmp28A,sizeof(char),47))),_tag_dyneither(_tmp289,sizeof(void*),0)));}goto _LL6B;
_LL6B:;};_pop_handler();}else{void*_tmp6C=(void*)_exn_thrown;void*_tmp7F=_tmp6C;
_LL77: if(_tmp7F != Cyc_Dict_Absent)goto _LL79;_LL78: if(exports == 0  || Cyc_Tc_export_member(
fd->name,*exports)){struct Cyc_Tcenv_VarRes_struct*_tmp299;struct Cyc_Absyn_Funname_b_struct*
_tmp298;struct Cyc_Absyn_Funname_b_struct _tmp297;struct Cyc_Tcenv_VarRes_struct
_tmp296;struct _tuple7*_tmp295;ge->ordinaries=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict
d,struct _dyneither_ptr*k,struct _tuple7*v))Cyc_Dict_insert)(ge->ordinaries,v,(
struct _tuple7*)((_tmp295=_cycalloc(sizeof(*_tmp295)),((_tmp295->f1=(void*)((
_tmp299=_cycalloc(sizeof(*_tmp299)),((_tmp299[0]=((_tmp296.tag=0,((_tmp296.f1=(
void*)((void*)((_tmp298=_cycalloc(sizeof(*_tmp298)),((_tmp298[0]=((_tmp297.tag=1,((
_tmp297.f1=fd,_tmp297)))),_tmp298))))),_tmp296)))),_tmp299)))),((_tmp295->f2=0,
_tmp295)))))));}goto _LL76;_LL79:;_LL7A:(void)_throw(_tmp7F);_LL76:;}}}if(
ignore_body)return;{struct _RegionHandle _tmp85=_new_region("fnrgn");struct
_RegionHandle*fnrgn=& _tmp85;_push_region(fnrgn);{struct Cyc_Tcenv_Fenv*_tmp86=Cyc_Tcenv_new_fenv(
fnrgn,loc,fd);struct Cyc_Tcenv_Tenv*_tmp29A;struct Cyc_Tcenv_Tenv*_tmp87=(_tmp29A=
_region_malloc(fnrgn,sizeof(*_tmp29A)),((_tmp29A->ns=te->ns,((_tmp29A->ae=te->ae,((
_tmp29A->le=(struct Cyc_Tcenv_Fenv*)_tmp86,((_tmp29A->allow_valueof=0,_tmp29A)))))))));
Cyc_Tcstmt_tcStmt(_tmp87,fd->body,0);Cyc_Tcenv_check_delayed_effects(_tmp87);Cyc_Tcenv_check_delayed_constraints(
_tmp87);if(!Cyc_Tcenv_all_labels_resolved(_tmp87)){const char*_tmp29D;void*
_tmp29C;(_tmp29C=0,Cyc_Tcutil_terr(loc,((_tmp29D="function has goto statements to undefined labels",
_tag_dyneither(_tmp29D,sizeof(char),49))),_tag_dyneither(_tmp29C,sizeof(void*),0)));}}
if(Cyc_Tc_is_main(fd->name)){{void*_tmp8B=Cyc_Tcutil_compress((void*)fd->ret_type);
void*_tmp8C;_LL7C: if((int)_tmp8B != 0)goto _LL7E;_LL7D:{const char*_tmp2A0;void*
_tmp29F;(_tmp29F=0,Cyc_Tcutil_warn(loc,((_tmp2A0="main declared with return type void",
_tag_dyneither(_tmp2A0,sizeof(char),36))),_tag_dyneither(_tmp29F,sizeof(void*),0)));}
goto _LL7B;_LL7E: if(_tmp8B <= (void*)4)goto _LL80;if(*((int*)_tmp8B)!= 5)goto _LL80;
_tmp8C=(void*)((struct Cyc_Absyn_IntType_struct*)_tmp8B)->f2;_LL7F: goto _LL7B;
_LL80:;_LL81:{const char*_tmp2A4;void*_tmp2A3[1];struct Cyc_String_pa_struct
_tmp2A2;(_tmp2A2.tag=0,((_tmp2A2.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_typ2string((void*)fd->ret_type)),((_tmp2A3[0]=& _tmp2A2,Cyc_Tcutil_terr(
loc,((_tmp2A4="main declared with return type %s instead of int or void",
_tag_dyneither(_tmp2A4,sizeof(char),57))),_tag_dyneither(_tmp2A3,sizeof(void*),1)))))));}
goto _LL7B;_LL7B:;}if(fd->c_varargs  || fd->cyc_varargs != 0){const char*_tmp2A7;
void*_tmp2A6;(_tmp2A6=0,Cyc_Tcutil_terr(loc,((_tmp2A7="main declared with varargs",
_tag_dyneither(_tmp2A7,sizeof(char),27))),_tag_dyneither(_tmp2A6,sizeof(void*),0)));}{
struct Cyc_List_List*_tmp94=fd->args;if(_tmp94 != 0){struct _tuple8 _tmp96;void*
_tmp97;struct _tuple8*_tmp95=(struct _tuple8*)_tmp94->hd;_tmp96=*_tmp95;_tmp97=
_tmp96.f3;{void*_tmp98=Cyc_Tcutil_compress(_tmp97);void*_tmp99;_LL83: if(_tmp98 <= (
void*)4)goto _LL85;if(*((int*)_tmp98)!= 5)goto _LL85;_tmp99=(void*)((struct Cyc_Absyn_IntType_struct*)
_tmp98)->f2;_LL84: goto _LL82;_LL85:;_LL86:{const char*_tmp2AB;void*_tmp2AA[1];
struct Cyc_String_pa_struct _tmp2A9;(_tmp2A9.tag=0,((_tmp2A9.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string(_tmp97)),((_tmp2AA[
0]=& _tmp2A9,Cyc_Tcutil_terr(loc,((_tmp2AB="main declared with first argument of type %s instead of int",
_tag_dyneither(_tmp2AB,sizeof(char),60))),_tag_dyneither(_tmp2AA,sizeof(void*),1)))))));}
goto _LL82;_LL82:;}_tmp94=_tmp94->tl;if(_tmp94 != 0){struct _tuple8 _tmp9E;void*
_tmp9F;struct _tuple8*_tmp9D=(struct _tuple8*)_tmp94->hd;_tmp9E=*_tmp9D;_tmp9F=
_tmp9E.f3;_tmp94=_tmp94->tl;if(_tmp94 != 0){const char*_tmp2AE;void*_tmp2AD;(
_tmp2AD=0,Cyc_Tcutil_terr(loc,((_tmp2AE="main declared with too many arguments",
_tag_dyneither(_tmp2AE,sizeof(char),38))),_tag_dyneither(_tmp2AD,sizeof(void*),0)));}{
struct Cyc_Core_Opt*_tmp2AF;struct Cyc_Core_Opt*tvs=(_tmp2AF=_cycalloc(sizeof(*
_tmp2AF)),((_tmp2AF->v=fd->tvs,_tmp2AF)));struct Cyc_Core_Opt*_tmp2B7;struct Cyc_Core_Opt*
_tmp2B6;struct Cyc_Core_Opt*_tmp2B5;struct Cyc_Core_Opt*_tmp2B4;struct Cyc_Core_Opt*
_tmp2B3;struct Cyc_Core_Opt*_tmp2B2;struct Cyc_Core_Opt*_tmp2B1;struct Cyc_Core_Opt*
_tmp2B0;if(((!Cyc_Tcutil_unify(_tmp9F,Cyc_Absyn_dyneither_typ(Cyc_Absyn_string_typ(
Cyc_Absyn_new_evar(((_tmp2B6=_cycalloc(sizeof(*_tmp2B6)),((_tmp2B6->v=(void*)((
void*)3),_tmp2B6)))),tvs)),Cyc_Absyn_new_evar(((_tmp2B7=_cycalloc(sizeof(*
_tmp2B7)),((_tmp2B7->v=(void*)((void*)3),_tmp2B7)))),tvs),Cyc_Absyn_empty_tqual(
0),((struct Cyc_Absyn_Conref*(*)())Cyc_Absyn_empty_conref)())) && !Cyc_Tcutil_unify(
_tmp9F,Cyc_Absyn_dyneither_typ(Cyc_Absyn_const_string_typ(Cyc_Absyn_new_evar(((
_tmp2B4=_cycalloc(sizeof(*_tmp2B4)),((_tmp2B4->v=(void*)((void*)3),_tmp2B4)))),
tvs)),Cyc_Absyn_new_evar(((_tmp2B5=_cycalloc(sizeof(*_tmp2B5)),((_tmp2B5->v=(
void*)((void*)3),_tmp2B5)))),tvs),Cyc_Absyn_empty_tqual(0),((struct Cyc_Absyn_Conref*(*)())
Cyc_Absyn_empty_conref)()))) && !Cyc_Tcutil_unify(_tmp9F,Cyc_Absyn_dyneither_typ(
Cyc_Absyn_string_typ(Cyc_Absyn_new_evar(((_tmp2B2=_cycalloc(sizeof(*_tmp2B2)),((
_tmp2B2->v=(void*)((void*)3),_tmp2B2)))),tvs)),Cyc_Absyn_new_evar(((_tmp2B3=
_cycalloc(sizeof(*_tmp2B3)),((_tmp2B3->v=(void*)((void*)3),_tmp2B3)))),tvs),Cyc_Absyn_const_tqual(
0),((struct Cyc_Absyn_Conref*(*)())Cyc_Absyn_empty_conref)()))) && !Cyc_Tcutil_unify(
_tmp9F,Cyc_Absyn_dyneither_typ(Cyc_Absyn_const_string_typ(Cyc_Absyn_new_evar(((
_tmp2B0=_cycalloc(sizeof(*_tmp2B0)),((_tmp2B0->v=(void*)((void*)3),_tmp2B0)))),
tvs)),Cyc_Absyn_new_evar(((_tmp2B1=_cycalloc(sizeof(*_tmp2B1)),((_tmp2B1->v=(
void*)((void*)3),_tmp2B1)))),tvs),Cyc_Absyn_const_tqual(0),((struct Cyc_Absyn_Conref*(*)())
Cyc_Absyn_empty_conref)()))){const char*_tmp2BB;void*_tmp2BA[1];struct Cyc_String_pa_struct
_tmp2B9;(_tmp2B9.tag=0,((_tmp2B9.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_typ2string(_tmp9F)),((_tmp2BA[0]=& _tmp2B9,Cyc_Tcutil_terr(loc,((
_tmp2BB="second argument of main has type %s instead of char??",_tag_dyneither(
_tmp2BB,sizeof(char),54))),_tag_dyneither(_tmp2BA,sizeof(void*),1)))))));}}}}}};
_pop_region(fnrgn);}}}static void Cyc_Tc_tcTypedefdecl(struct Cyc_Tcenv_Tenv*te,
struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Typedefdecl*
td);static void Cyc_Tc_tcTypedefdecl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*
ge,struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Typedefdecl*td){struct
_dyneither_ptr*v=(*td->name).f2;{union Cyc_Absyn_Nmspace_union _tmpAE=(*td->name).f1;
struct Cyc_List_List*_tmpAF;struct Cyc_List_List*_tmpB0;_LL88: if((_tmpAE.Rel_n).tag
!= 1)goto _LL8A;_tmpAF=(_tmpAE.Rel_n).f1;if(_tmpAF != 0)goto _LL8A;_LL89: goto _LL8B;
_LL8A: if((_tmpAE.Abs_n).tag != 2)goto _LL8C;_tmpB0=(_tmpAE.Abs_n).f1;if(_tmpB0 != 0)
goto _LL8C;_LL8B: goto _LL87;_LL8C:;_LL8D:{const char*_tmp2BF;void*_tmp2BE[1];struct
Cyc_String_pa_struct _tmp2BD;(_tmp2BD.tag=0,((_tmp2BD.f1=(struct _dyneither_ptr)((
struct _dyneither_ptr)Cyc_Absynpp_qvar2string(td->name)),((_tmp2BE[0]=& _tmp2BD,
Cyc_Tcutil_terr(loc,((_tmp2BF="qualified typedef declarations are not implemented (%s)",
_tag_dyneither(_tmp2BF,sizeof(char),56))),_tag_dyneither(_tmp2BE,sizeof(void*),1)))))));}
return;_LL87:;}if(((int(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_member)(
ge->typedefs,v)){{const char*_tmp2C3;void*_tmp2C2[1];struct Cyc_String_pa_struct
_tmp2C1;(_tmp2C1.tag=0,((_tmp2C1.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*
v),((_tmp2C2[0]=& _tmp2C1,Cyc_Tcutil_terr(loc,((_tmp2C3="redeclaration of typedef %s",
_tag_dyneither(_tmp2C3,sizeof(char),28))),_tag_dyneither(_tmp2C2,sizeof(void*),1)))))));}
return;}{union Cyc_Absyn_Nmspace_union _tmp2C4;(*td->name).f1=(union Cyc_Absyn_Nmspace_union)(((
_tmp2C4.Abs_n).tag=2,(((_tmp2C4.Abs_n).f1=te->ns,_tmp2C4))));}Cyc_Tcutil_check_unique_tvars(
loc,td->tvs);Cyc_Tcutil_add_tvar_identities(td->tvs);if(td->defn != 0){Cyc_Tcutil_check_type(
loc,te,td->tvs,(void*)0,0,(void*)((struct Cyc_Core_Opt*)_check_null(td->defn))->v);(
td->tq).real_const=Cyc_Tcutil_extract_const_from_typedef(loc,(td->tq).print_const,(
void*)((struct Cyc_Core_Opt*)_check_null(td->defn))->v);}{struct Cyc_List_List*tvs=
td->tvs;for(0;tvs != 0;tvs=tvs->tl){void*_tmpB8=Cyc_Absyn_compress_kb((void*)((
struct Cyc_Absyn_Tvar*)tvs->hd)->kind);struct Cyc_Core_Opt*_tmpB9;struct Cyc_Core_Opt**
_tmpBA;struct Cyc_Core_Opt*_tmpBB;struct Cyc_Core_Opt**_tmpBC;void*_tmpBD;_LL8F:
if(*((int*)_tmpB8)!= 1)goto _LL91;_tmpB9=((struct Cyc_Absyn_Unknown_kb_struct*)
_tmpB8)->f1;_tmpBA=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_struct*)
_tmpB8)->f1;_LL90: if(td->defn != 0){const char*_tmp2C8;void*_tmp2C7[1];struct Cyc_String_pa_struct
_tmp2C6;(_tmp2C6.tag=0,((_tmp2C6.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*((
struct Cyc_Absyn_Tvar*)tvs->hd)->name),((_tmp2C7[0]=& _tmp2C6,Cyc_Tcutil_warn(loc,((
_tmp2C8="type variable %s is not used in typedef definition",_tag_dyneither(
_tmp2C8,sizeof(char),51))),_tag_dyneither(_tmp2C7,sizeof(void*),1)))))));}{
struct Cyc_Core_Opt*_tmp2C9;*_tmpBA=((_tmp2C9=_cycalloc(sizeof(*_tmp2C9)),((
_tmp2C9->v=(void*)Cyc_Tcutil_kind_to_bound((void*)2),_tmp2C9))));}goto _LL8E;
_LL91: if(*((int*)_tmpB8)!= 2)goto _LL93;_tmpBB=((struct Cyc_Absyn_Less_kb_struct*)
_tmpB8)->f1;_tmpBC=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)
_tmpB8)->f1;_tmpBD=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmpB8)->f2;_LL92:{
struct Cyc_Core_Opt*_tmp2CA;*_tmpBC=((_tmp2CA=_cycalloc(sizeof(*_tmp2CA)),((
_tmp2CA->v=(void*)Cyc_Tcutil_kind_to_bound(_tmpBD),_tmp2CA))));}goto _LL8E;_LL93:;
_LL94: continue;_LL8E:;}}ge->typedefs=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict
d,struct _dyneither_ptr*k,struct Cyc_Absyn_Typedefdecl*v))Cyc_Dict_insert)(ge->typedefs,
v,td);}static void Cyc_Tc_tcAggrImpl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*
ge,struct Cyc_Position_Segment*loc,struct Cyc_List_List*tvs,struct Cyc_List_List*
rpo,struct Cyc_List_List*fields);static void Cyc_Tc_tcAggrImpl(struct Cyc_Tcenv_Tenv*
te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,struct Cyc_List_List*
tvs,struct Cyc_List_List*rpo,struct Cyc_List_List*fields){struct _RegionHandle
_tmpC3=_new_region("uprev_rgn");struct _RegionHandle*uprev_rgn=& _tmpC3;
_push_region(uprev_rgn);for(0;rpo != 0;rpo=rpo->tl){struct _tuple3 _tmpC5;void*
_tmpC6;void*_tmpC7;struct _tuple3*_tmpC4=(struct _tuple3*)rpo->hd;_tmpC5=*_tmpC4;
_tmpC6=_tmpC5.f1;_tmpC7=_tmpC5.f2;Cyc_Tcutil_check_type(loc,te,tvs,(void*)6,0,
_tmpC6);Cyc_Tcutil_check_type(loc,te,tvs,(void*)5,0,_tmpC7);}{struct Cyc_List_List*
prev_fields=0;struct Cyc_List_List*_tmpC8=fields;for(0;_tmpC8 != 0;_tmpC8=_tmpC8->tl){
struct Cyc_Absyn_Aggrfield _tmpCA;struct _dyneither_ptr*_tmpCB;struct Cyc_Absyn_Tqual
_tmpCC;void*_tmpCD;struct Cyc_Absyn_Exp*_tmpCE;struct Cyc_List_List*_tmpCF;struct
Cyc_Absyn_Aggrfield*_tmpC9=(struct Cyc_Absyn_Aggrfield*)_tmpC8->hd;_tmpCA=*_tmpC9;
_tmpCB=_tmpCA.name;_tmpCC=_tmpCA.tq;_tmpCD=(void*)_tmpCA.type;_tmpCE=_tmpCA.width;
_tmpCF=_tmpCA.attributes;if(((int(*)(int(*compare)(struct _dyneither_ptr*,struct
_dyneither_ptr*),struct Cyc_List_List*l,struct _dyneither_ptr*x))Cyc_List_mem)(Cyc_strptrcmp,
prev_fields,_tmpCB)){const char*_tmp2CE;void*_tmp2CD[1];struct Cyc_String_pa_struct
_tmp2CC;(_tmp2CC.tag=0,((_tmp2CC.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*
_tmpCB),((_tmp2CD[0]=& _tmp2CC,Cyc_Tcutil_terr(loc,((_tmp2CE="duplicate field %s",
_tag_dyneither(_tmp2CE,sizeof(char),19))),_tag_dyneither(_tmp2CD,sizeof(void*),1)))))));}{
const char*_tmp2CF;if(Cyc_strcmp((struct _dyneither_ptr)*_tmpCB,((_tmp2CF="",
_tag_dyneither(_tmp2CF,sizeof(char),1))))!= 0){struct Cyc_List_List*_tmp2D0;
prev_fields=((_tmp2D0=_region_malloc(uprev_rgn,sizeof(*_tmp2D0)),((_tmp2D0->hd=
_tmpCB,((_tmp2D0->tl=prev_fields,_tmp2D0))))));}}Cyc_Tcutil_check_type(loc,te,
tvs,(void*)1,0,_tmpCD);(((struct Cyc_Absyn_Aggrfield*)_tmpC8->hd)->tq).real_const=
Cyc_Tcutil_extract_const_from_typedef(loc,(((struct Cyc_Absyn_Aggrfield*)_tmpC8->hd)->tq).print_const,
_tmpCD);Cyc_Tcutil_check_bitfield(loc,te,_tmpCD,_tmpCE,_tmpCB);}};_pop_region(
uprev_rgn);}struct _tuple9{struct Cyc_Absyn_AggrdeclImpl*f1;struct Cyc_Absyn_Aggrdecl***
f2;};void Cyc_Tc_tcAggrdecl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,
struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Aggrdecl*ad);void Cyc_Tc_tcAggrdecl(
struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,
struct Cyc_Absyn_Aggrdecl*ad){struct _dyneither_ptr*_tmpD5=(*ad->name).f2;{struct
Cyc_List_List*atts=ad->attributes;for(0;atts != 0;atts=atts->tl){void*_tmpD6=(
void*)atts->hd;_LL96: if((int)_tmpD6 != 5)goto _LL98;_LL97: goto _LL99;_LL98: if(
_tmpD6 <= (void*)17)goto _LL9A;if(*((int*)_tmpD6)!= 1)goto _LL9A;_LL99: continue;
_LL9A:;_LL9B:{const char*_tmp2D5;void*_tmp2D4[2];struct Cyc_String_pa_struct
_tmp2D3;struct Cyc_String_pa_struct _tmp2D2;(_tmp2D2.tag=0,((_tmp2D2.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)*_tmpD5),((_tmp2D3.tag=0,((_tmp2D3.f1=(
struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absyn_attribute2string((void*)
atts->hd)),((_tmp2D4[0]=& _tmp2D3,((_tmp2D4[1]=& _tmp2D2,Cyc_Tcutil_terr(loc,((
_tmp2D5="bad attribute %s in  %s definition",_tag_dyneither(_tmp2D5,sizeof(char),
35))),_tag_dyneither(_tmp2D4,sizeof(void*),2)))))))))))));}goto _LL95;_LL95:;}}{
struct Cyc_List_List*_tmpDB=ad->tvs;{struct Cyc_List_List*tvs2=_tmpDB;for(0;tvs2 != 
0;tvs2=tvs2->tl){void*_tmpDC=Cyc_Absyn_compress_kb((void*)((struct Cyc_Absyn_Tvar*)
tvs2->hd)->kind);struct Cyc_Core_Opt*_tmpDD;struct Cyc_Core_Opt**_tmpDE;struct Cyc_Core_Opt*
_tmpDF;struct Cyc_Core_Opt**_tmpE0;void*_tmpE1;struct Cyc_Core_Opt*_tmpE2;struct
Cyc_Core_Opt**_tmpE3;void*_tmpE4;void*_tmpE5;_LL9D: if(*((int*)_tmpDC)!= 1)goto
_LL9F;_tmpDD=((struct Cyc_Absyn_Unknown_kb_struct*)_tmpDC)->f1;_tmpDE=(struct Cyc_Core_Opt**)&((
struct Cyc_Absyn_Unknown_kb_struct*)_tmpDC)->f1;_LL9E: _tmpE0=_tmpDE;goto _LLA0;
_LL9F: if(*((int*)_tmpDC)!= 2)goto _LLA1;_tmpDF=((struct Cyc_Absyn_Less_kb_struct*)
_tmpDC)->f1;_tmpE0=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)
_tmpDC)->f1;_tmpE1=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmpDC)->f2;if((int)
_tmpE1 != 1)goto _LLA1;_LLA0: _tmpE3=_tmpE0;goto _LLA2;_LLA1: if(*((int*)_tmpDC)!= 2)
goto _LLA3;_tmpE2=((struct Cyc_Absyn_Less_kb_struct*)_tmpDC)->f1;_tmpE3=(struct Cyc_Core_Opt**)&((
struct Cyc_Absyn_Less_kb_struct*)_tmpDC)->f1;_tmpE4=(void*)((struct Cyc_Absyn_Less_kb_struct*)
_tmpDC)->f2;if((int)_tmpE4 != 0)goto _LLA3;_LLA2:{struct Cyc_Core_Opt*_tmp2D6;*
_tmpE3=((_tmp2D6=_cycalloc(sizeof(*_tmp2D6)),((_tmp2D6->v=(void*)Cyc_Tcutil_kind_to_bound((
void*)2),_tmp2D6))));}continue;_LLA3: if(*((int*)_tmpDC)!= 0)goto _LLA5;_tmpE5=(
void*)((struct Cyc_Absyn_Eq_kb_struct*)_tmpDC)->f1;if((int)_tmpE5 != 1)goto _LLA5;
_LLA4:{const char*_tmp2DB;void*_tmp2DA[2];struct Cyc_String_pa_struct _tmp2D9;
struct Cyc_String_pa_struct _tmp2D8;(_tmp2D8.tag=0,((_tmp2D8.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)*((struct Cyc_Absyn_Tvar*)tvs2->hd)->name),((
_tmp2D9.tag=0,((_tmp2D9.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*_tmpD5),((
_tmp2DA[0]=& _tmp2D9,((_tmp2DA[1]=& _tmp2D8,Cyc_Tcutil_terr(loc,((_tmp2DB="type %s attempts to abstract type variable %s of kind M",
_tag_dyneither(_tmp2DB,sizeof(char),56))),_tag_dyneither(_tmp2DA,sizeof(void*),2)))))))))))));}
continue;_LLA5:;_LLA6: continue;_LL9C:;}}{union Cyc_Absyn_Nmspace_union _tmpEB=(*ad->name).f1;
struct Cyc_List_List*_tmpEC;struct Cyc_List_List*_tmpED;_LLA8: if((_tmpEB.Rel_n).tag
!= 1)goto _LLAA;_tmpEC=(_tmpEB.Rel_n).f1;if(_tmpEC != 0)goto _LLAA;_LLA9: goto _LLAB;
_LLAA: if((_tmpEB.Abs_n).tag != 2)goto _LLAC;_tmpED=(_tmpEB.Abs_n).f1;if(_tmpED != 0)
goto _LLAC;_LLAB: goto _LLA7;_LLAC:;_LLAD:{const char*_tmp2DF;void*_tmp2DE[1];struct
Cyc_String_pa_struct _tmp2DD;(_tmp2DD.tag=0,((_tmp2DD.f1=(struct _dyneither_ptr)((
struct _dyneither_ptr)Cyc_Absynpp_qvar2string(ad->name)),((_tmp2DE[0]=& _tmp2DD,
Cyc_Tcutil_terr(loc,((_tmp2DF="qualified struct declarations are not implemented (%s)",
_tag_dyneither(_tmp2DF,sizeof(char),55))),_tag_dyneither(_tmp2DE,sizeof(void*),1)))))));}
return;_LLA7:;}{union Cyc_Absyn_Nmspace_union _tmp2E0;(*ad->name).f1=(union Cyc_Absyn_Nmspace_union)(((
_tmp2E0.Abs_n).tag=2,(((_tmp2E0.Abs_n).f1=te->ns,_tmp2E0))));}Cyc_Tcutil_check_unique_tvars(
loc,ad->tvs);Cyc_Tcutil_add_tvar_identities(ad->tvs);{struct _tuple9 _tmp2E1;
struct _tuple9 _tmpF3=(_tmp2E1.f1=ad->impl,((_tmp2E1.f2=((struct Cyc_Absyn_Aggrdecl***(*)(
struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup_opt)(ge->aggrdecls,
_tmpD5),_tmp2E1)));struct Cyc_Absyn_AggrdeclImpl*_tmpF4;struct Cyc_Absyn_Aggrdecl***
_tmpF5;struct Cyc_Absyn_AggrdeclImpl*_tmpF6;struct Cyc_Absyn_AggrdeclImpl _tmpF7;
struct Cyc_List_List*_tmpF8;struct Cyc_List_List*_tmpF9;struct Cyc_List_List*_tmpFA;
struct Cyc_Absyn_Aggrdecl***_tmpFB;struct Cyc_Absyn_AggrdeclImpl*_tmpFC;struct Cyc_Absyn_AggrdeclImpl
_tmpFD;struct Cyc_List_List*_tmpFE;struct Cyc_List_List*_tmpFF;struct Cyc_List_List*
_tmp100;struct Cyc_Absyn_Aggrdecl***_tmp101;struct Cyc_Absyn_Aggrdecl**_tmp102;
struct Cyc_Absyn_AggrdeclImpl*_tmp103;struct Cyc_Absyn_Aggrdecl***_tmp104;struct
Cyc_Absyn_Aggrdecl**_tmp105;_LLAF: _tmpF4=_tmpF3.f1;if(_tmpF4 != 0)goto _LLB1;
_tmpF5=_tmpF3.f2;if(_tmpF5 != 0)goto _LLB1;_LLB0:{struct Cyc_Absyn_Aggrdecl**
_tmp2E2;ge->aggrdecls=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct
_dyneither_ptr*k,struct Cyc_Absyn_Aggrdecl**v))Cyc_Dict_insert)(ge->aggrdecls,
_tmpD5,((_tmp2E2=_cycalloc(sizeof(*_tmp2E2)),((_tmp2E2[0]=ad,_tmp2E2)))));}goto
_LLAE;_LLB1: _tmpF6=_tmpF3.f1;if(_tmpF6 == 0)goto _LLB3;_tmpF7=*_tmpF6;_tmpF8=
_tmpF7.exist_vars;_tmpF9=_tmpF7.rgn_po;_tmpFA=_tmpF7.fields;_tmpFB=_tmpF3.f2;if(
_tmpFB != 0)goto _LLB3;_LLB2: {struct Cyc_Absyn_Aggrdecl*_tmp2E5;struct Cyc_Absyn_Aggrdecl**
_tmp2E4;struct Cyc_Absyn_Aggrdecl**_tmp107=(_tmp2E4=_cycalloc(sizeof(*_tmp2E4)),((
_tmp2E4[0]=((_tmp2E5=_cycalloc(sizeof(*_tmp2E5)),((_tmp2E5->kind=(void*)((void*)
ad->kind),((_tmp2E5->sc=(void*)((void*)3),((_tmp2E5->name=ad->name,((_tmp2E5->tvs=
_tmpDB,((_tmp2E5->impl=0,((_tmp2E5->attributes=ad->attributes,_tmp2E5)))))))))))))),
_tmp2E4)));ge->aggrdecls=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct
_dyneither_ptr*k,struct Cyc_Absyn_Aggrdecl**v))Cyc_Dict_insert)(ge->aggrdecls,
_tmpD5,_tmp107);Cyc_Tcutil_check_unique_tvars(loc,_tmpF8);Cyc_Tcutil_add_tvar_identities(
_tmpF8);{struct Cyc_List_List*tvs2=_tmpF8;for(0;tvs2 != 0;tvs2=tvs2->tl){void*
_tmp108=Cyc_Absyn_compress_kb((void*)((struct Cyc_Absyn_Tvar*)tvs2->hd)->kind);
struct Cyc_Core_Opt*_tmp109;struct Cyc_Core_Opt**_tmp10A;struct Cyc_Core_Opt*
_tmp10B;struct Cyc_Core_Opt**_tmp10C;void*_tmp10D;struct Cyc_Core_Opt*_tmp10E;
struct Cyc_Core_Opt**_tmp10F;void*_tmp110;void*_tmp111;_LLB8: if(*((int*)_tmp108)
!= 1)goto _LLBA;_tmp109=((struct Cyc_Absyn_Unknown_kb_struct*)_tmp108)->f1;_tmp10A=(
struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_struct*)_tmp108)->f1;_LLB9:
_tmp10C=_tmp10A;goto _LLBB;_LLBA: if(*((int*)_tmp108)!= 2)goto _LLBC;_tmp10B=((
struct Cyc_Absyn_Less_kb_struct*)_tmp108)->f1;_tmp10C=(struct Cyc_Core_Opt**)&((
struct Cyc_Absyn_Less_kb_struct*)_tmp108)->f1;_tmp10D=(void*)((struct Cyc_Absyn_Less_kb_struct*)
_tmp108)->f2;if((int)_tmp10D != 1)goto _LLBC;_LLBB: _tmp10F=_tmp10C;goto _LLBD;_LLBC:
if(*((int*)_tmp108)!= 2)goto _LLBE;_tmp10E=((struct Cyc_Absyn_Less_kb_struct*)
_tmp108)->f1;_tmp10F=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)
_tmp108)->f1;_tmp110=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmp108)->f2;if((
int)_tmp110 != 0)goto _LLBE;_LLBD:{struct Cyc_Core_Opt*_tmp2E6;*_tmp10F=((_tmp2E6=
_cycalloc(sizeof(*_tmp2E6)),((_tmp2E6->v=(void*)Cyc_Tcutil_kind_to_bound((void*)
2),_tmp2E6))));}continue;_LLBE: if(*((int*)_tmp108)!= 0)goto _LLC0;_tmp111=(void*)((
struct Cyc_Absyn_Eq_kb_struct*)_tmp108)->f1;if((int)_tmp111 != 1)goto _LLC0;_LLBF:{
const char*_tmp2EB;void*_tmp2EA[2];struct Cyc_String_pa_struct _tmp2E9;struct Cyc_String_pa_struct
_tmp2E8;(_tmp2E8.tag=0,((_tmp2E8.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*((
struct Cyc_Absyn_Tvar*)tvs2->hd)->name),((_tmp2E9.tag=0,((_tmp2E9.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)*_tmpD5),((_tmp2EA[0]=& _tmp2E9,((_tmp2EA[1]=&
_tmp2E8,Cyc_Tcutil_terr(loc,((_tmp2EB="type %s attempts to abstract type variable %s of kind M",
_tag_dyneither(_tmp2EB,sizeof(char),56))),_tag_dyneither(_tmp2EA,sizeof(void*),2)))))))))))));}
continue;_LLC0:;_LLC1: continue;_LLB7:;}}Cyc_Tc_tcAggrImpl(te,ge,loc,((struct Cyc_List_List*(*)(
struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(_tmpDB,_tmpF8),
_tmpF9,_tmpFA);if((void*)ad->kind == (void*)1){struct Cyc_List_List*f=_tmpFA;for(0;
f != 0;f=f->tl){if(!Cyc_Tcutil_bits_only((void*)((struct Cyc_Absyn_Aggrfield*)f->hd)->type)){
const char*_tmp2F1;void*_tmp2F0[3];struct Cyc_String_pa_struct _tmp2EF;struct Cyc_String_pa_struct
_tmp2EE;struct Cyc_String_pa_struct _tmp2ED;(_tmp2ED.tag=0,((_tmp2ED.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_typ2string((void*)((struct Cyc_Absyn_Aggrfield*)
f->hd)->type)),((_tmp2EE.tag=0,((_tmp2EE.f1=(struct _dyneither_ptr)((struct
_dyneither_ptr)*_tmpD5),((_tmp2EF.tag=0,((_tmp2EF.f1=(struct _dyneither_ptr)((
struct _dyneither_ptr)*((struct Cyc_Absyn_Aggrfield*)f->hd)->name),((_tmp2F0[0]=&
_tmp2EF,((_tmp2F0[1]=& _tmp2EE,((_tmp2F0[2]=& _tmp2ED,Cyc_Tcutil_warn(loc,((
_tmp2F1="member %s of union %s has type %s which is not `bits-only' so it can only be written and not read",
_tag_dyneither(_tmp2F1,sizeof(char),98))),_tag_dyneither(_tmp2F0,sizeof(void*),3)))))))))))))))))));}}}*
_tmp107=ad;goto _LLAE;}_LLB3: _tmpFC=_tmpF3.f1;if(_tmpFC == 0)goto _LLB5;_tmpFD=*
_tmpFC;_tmpFE=_tmpFD.exist_vars;_tmpFF=_tmpFD.rgn_po;_tmp100=_tmpFD.fields;
_tmp101=_tmpF3.f2;if(_tmp101 == 0)goto _LLB5;_tmp102=*_tmp101;_LLB4: if((void*)ad->kind
!= (void*)(*_tmp102)->kind){const char*_tmp2F4;void*_tmp2F3;(_tmp2F3=0,Cyc_Tcutil_terr(
loc,((_tmp2F4="cannot reuse struct names for unions and vice-versa",
_tag_dyneither(_tmp2F4,sizeof(char),52))),_tag_dyneither(_tmp2F3,sizeof(void*),0)));}{
struct Cyc_Absyn_Aggrdecl*_tmp120=*_tmp102;{struct Cyc_Absyn_Aggrdecl*_tmp2F5;*
_tmp102=((_tmp2F5=_cycalloc(sizeof(*_tmp2F5)),((_tmp2F5->kind=(void*)((void*)ad->kind),((
_tmp2F5->sc=(void*)((void*)3),((_tmp2F5->name=ad->name,((_tmp2F5->tvs=_tmpDB,((
_tmp2F5->impl=0,((_tmp2F5->attributes=ad->attributes,_tmp2F5))))))))))))));}Cyc_Tcutil_check_unique_tvars(
loc,_tmpFE);Cyc_Tcutil_add_tvar_identities(_tmpFE);Cyc_Tc_tcAggrImpl(te,ge,loc,((
struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(
_tmpDB,_tmpFE),_tmpFF,_tmp100);*_tmp102=_tmp120;_tmp105=_tmp102;goto _LLB6;}_LLB5:
_tmp103=_tmpF3.f1;if(_tmp103 != 0)goto _LLAE;_tmp104=_tmpF3.f2;if(_tmp104 == 0)goto
_LLAE;_tmp105=*_tmp104;_LLB6: {struct Cyc_Absyn_Aggrdecl*_tmp122=Cyc_Tcdecl_merge_aggrdecl(*
_tmp105,ad,loc,Cyc_Tc_tc_msg);if(_tmp122 == 0)return;else{*_tmp105=(struct Cyc_Absyn_Aggrdecl*)
_tmp122;ad=(struct Cyc_Absyn_Aggrdecl*)_tmp122;goto _LLAE;}}_LLAE:;}{struct Cyc_Tcenv_AggrRes_struct*
_tmp2FB;struct Cyc_Tcenv_AggrRes_struct _tmp2FA;struct _tuple7*_tmp2F9;ge->ordinaries=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k,struct _tuple7*
v))Cyc_Dict_insert)(ge->ordinaries,_tmpD5,(struct _tuple7*)((_tmp2F9=_cycalloc(
sizeof(*_tmp2F9)),((_tmp2F9->f1=(void*)((_tmp2FB=_cycalloc(sizeof(*_tmp2FB)),((
_tmp2FB[0]=((_tmp2FA.tag=1,((_tmp2FA.f1=ad,_tmp2FA)))),_tmp2FB)))),((_tmp2F9->f2=
1,_tmp2F9)))))));}}}struct _tuple10{struct Cyc_Absyn_Tqual f1;void*f2;};static
struct Cyc_List_List*Cyc_Tc_tcTunionFields(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*
ge,struct Cyc_Position_Segment*loc,struct _dyneither_ptr obj,int is_xtunion,struct
_tuple0*name,struct Cyc_List_List*fields,struct Cyc_List_List*tvs,struct Cyc_Absyn_Tuniondecl*
tudres);static struct Cyc_List_List*Cyc_Tc_tcTunionFields(struct Cyc_Tcenv_Tenv*te,
struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,struct _dyneither_ptr obj,
int is_xtunion,struct _tuple0*name,struct Cyc_List_List*fields,struct Cyc_List_List*
tvs,struct Cyc_Absyn_Tuniondecl*tudres){{struct Cyc_List_List*_tmp126=fields;for(0;
_tmp126 != 0;_tmp126=_tmp126->tl){struct Cyc_Absyn_Tunionfield*_tmp127=(struct Cyc_Absyn_Tunionfield*)
_tmp126->hd;{struct Cyc_List_List*typs=_tmp127->typs;for(0;typs != 0;typs=typs->tl){
Cyc_Tcutil_check_type(_tmp127->loc,te,tvs,(void*)1,0,(*((struct _tuple10*)typs->hd)).f2);{
struct _RegionHandle*_tmp128=Cyc_Tcenv_get_fnrgn(te);if(!tudres->is_flat  && Cyc_Tcutil_is_noalias_pointer_or_aggr(
_tmp128,(*((struct _tuple10*)typs->hd)).f2)){const char*_tmp2FF;void*_tmp2FE[1];
struct Cyc_String_pa_struct _tmp2FD;(_tmp2FD.tag=0,((_tmp2FD.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp127->name)),((
_tmp2FE[0]=& _tmp2FD,Cyc_Tcutil_terr(_tmp127->loc,((_tmp2FF="noalias pointers in non-flat tunions are not allowed (%s)",
_tag_dyneither(_tmp2FF,sizeof(char),58))),_tag_dyneither(_tmp2FE,sizeof(void*),1)))))));}((*((
struct _tuple10*)typs->hd)).f1).real_const=Cyc_Tcutil_extract_const_from_typedef(
_tmp127->loc,((*((struct _tuple10*)typs->hd)).f1).print_const,(*((struct _tuple10*)
typs->hd)).f2);}}}{union Cyc_Absyn_Nmspace_union _tmp12C=(*_tmp127->name).f1;
struct Cyc_List_List*_tmp12D;_LLC3: if((_tmp12C.Rel_n).tag != 1)goto _LLC5;_tmp12D=(
_tmp12C.Rel_n).f1;if(_tmp12D != 0)goto _LLC5;_LLC4: if(is_xtunion){union Cyc_Absyn_Nmspace_union
_tmp300;(*_tmp127->name).f1=(union Cyc_Absyn_Nmspace_union)(((_tmp300.Abs_n).tag=
2,(((_tmp300.Abs_n).f1=te->ns,_tmp300))));}else{(*_tmp127->name).f1=(*name).f1;}
goto _LLC2;_LLC5: if((_tmp12C.Rel_n).tag != 1)goto _LLC7;_LLC6:{const char*_tmp304;
void*_tmp303[1];struct Cyc_String_pa_struct _tmp302;(_tmp302.tag=0,((_tmp302.f1=(
struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(_tmp127->name)),((
_tmp303[0]=& _tmp302,Cyc_Tcutil_terr(_tmp127->loc,((_tmp304="qualified tunionfield declarations are not allowed (%s)",
_tag_dyneither(_tmp304,sizeof(char),56))),_tag_dyneither(_tmp303,sizeof(void*),1)))))));}
goto _LLC2;_LLC7: if((_tmp12C.Abs_n).tag != 2)goto _LLC9;_LLC8: goto _LLC2;_LLC9: if((
_tmp12C.Loc_n).tag != 0)goto _LLC2;_LLCA: {const char*_tmp307;void*_tmp306;(_tmp306=
0,((int(*)(struct _dyneither_ptr fmt,struct _dyneither_ptr ap))Cyc_Tcutil_impos)(((
_tmp307="tcTunionFields: Loc_n",_tag_dyneither(_tmp307,sizeof(char),22))),
_tag_dyneither(_tmp306,sizeof(void*),0)));}_LLC2:;}}}{struct Cyc_List_List*
fields2;if(is_xtunion){int _tmp134=1;struct Cyc_List_List*_tmp135=Cyc_Tcdecl_sort_xtunion_fields(
fields,& _tmp134,(*name).f2,loc,Cyc_Tc_tc_msg);if(_tmp134)fields2=_tmp135;else{
fields2=0;}}else{struct _RegionHandle _tmp136=_new_region("uprev_rgn");struct
_RegionHandle*uprev_rgn=& _tmp136;_push_region(uprev_rgn);{struct Cyc_List_List*
prev_fields=0;{struct Cyc_List_List*fs=fields;for(0;fs != 0;fs=fs->tl){struct Cyc_Absyn_Tunionfield*
_tmp137=(struct Cyc_Absyn_Tunionfield*)fs->hd;if(((int(*)(int(*compare)(struct
_dyneither_ptr*,struct _dyneither_ptr*),struct Cyc_List_List*l,struct
_dyneither_ptr*x))Cyc_List_mem)(Cyc_strptrcmp,prev_fields,(*_tmp137->name).f2)){
const char*_tmp30C;void*_tmp30B[2];struct Cyc_String_pa_struct _tmp30A;struct Cyc_String_pa_struct
_tmp309;(_tmp309.tag=0,((_tmp309.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
obj),((_tmp30A.tag=0,((_tmp30A.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*(*
_tmp137->name).f2),((_tmp30B[0]=& _tmp30A,((_tmp30B[1]=& _tmp309,Cyc_Tcutil_terr(
_tmp137->loc,((_tmp30C="duplicate field name %s in %s",_tag_dyneither(_tmp30C,
sizeof(char),30))),_tag_dyneither(_tmp30B,sizeof(void*),2)))))))))))));}else{
struct Cyc_List_List*_tmp30D;prev_fields=((_tmp30D=_region_malloc(uprev_rgn,
sizeof(*_tmp30D)),((_tmp30D->hd=(*_tmp137->name).f2,((_tmp30D->tl=prev_fields,
_tmp30D))))));}if((void*)_tmp137->sc != (void*)2){{const char*_tmp311;void*_tmp310[
1];struct Cyc_String_pa_struct _tmp30F;(_tmp30F.tag=0,((_tmp30F.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)*(*_tmp137->name).f2),((_tmp310[0]=&
_tmp30F,Cyc_Tcutil_warn(loc,((_tmp311="ignoring scope of field %s",
_tag_dyneither(_tmp311,sizeof(char),27))),_tag_dyneither(_tmp310,sizeof(void*),1)))))));}(
void*)(_tmp137->sc=(void*)((void*)2));}}}fields2=fields;};_pop_region(uprev_rgn);}{
struct Cyc_List_List*_tmp140=fields;for(0;_tmp140 != 0;_tmp140=_tmp140->tl){struct
Cyc_Absyn_Tunionfield*_tmp141=(struct Cyc_Absyn_Tunionfield*)_tmp140->hd;struct
Cyc_Tcenv_TunionRes_struct*_tmp317;struct Cyc_Tcenv_TunionRes_struct _tmp316;
struct _tuple7*_tmp315;ge->ordinaries=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict
d,struct _dyneither_ptr*k,struct _tuple7*v))Cyc_Dict_insert)(ge->ordinaries,(*
_tmp141->name).f2,(struct _tuple7*)((_tmp315=_cycalloc(sizeof(*_tmp315)),((
_tmp315->f1=(void*)((_tmp317=_cycalloc(sizeof(*_tmp317)),((_tmp317[0]=((_tmp316.tag=
2,((_tmp316.f1=tudres,((_tmp316.f2=_tmp141,_tmp316)))))),_tmp317)))),((_tmp315->f2=
1,_tmp315)))))));}}return fields2;}}struct _tuple11{struct Cyc_Core_Opt*f1;struct
Cyc_Absyn_Tuniondecl***f2;};void Cyc_Tc_tcTuniondecl(struct Cyc_Tcenv_Tenv*te,
struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Tuniondecl*
tud);void Cyc_Tc_tcTuniondecl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,
struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Tuniondecl*tud){struct
_dyneither_ptr*v=(*tud->name).f2;const char*_tmp319;const char*_tmp318;struct
_dyneither_ptr obj=tud->is_xtunion?(_tmp319="xtunion",_tag_dyneither(_tmp319,
sizeof(char),8)):((_tmp318="tunion",_tag_dyneither(_tmp318,sizeof(char),7)));
struct Cyc_List_List*tvs=tud->tvs;{struct Cyc_List_List*tvs2=tvs;for(0;tvs2 != 0;
tvs2=tvs2->tl){void*_tmp145=Cyc_Absyn_compress_kb((void*)((struct Cyc_Absyn_Tvar*)
tvs2->hd)->kind);struct Cyc_Core_Opt*_tmp146;struct Cyc_Core_Opt**_tmp147;struct
Cyc_Core_Opt*_tmp148;struct Cyc_Core_Opt**_tmp149;void*_tmp14A;struct Cyc_Core_Opt*
_tmp14B;struct Cyc_Core_Opt**_tmp14C;void*_tmp14D;struct Cyc_Core_Opt*_tmp14E;
struct Cyc_Core_Opt**_tmp14F;void*_tmp150;void*_tmp151;struct Cyc_Core_Opt*_tmp152;
struct Cyc_Core_Opt**_tmp153;void*_tmp154;void*_tmp155;void*_tmp156;_LLCC: if(*((
int*)_tmp145)!= 1)goto _LLCE;_tmp146=((struct Cyc_Absyn_Unknown_kb_struct*)_tmp145)->f1;
_tmp147=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_struct*)_tmp145)->f1;
_LLCD: _tmp149=_tmp147;goto _LLCF;_LLCE: if(*((int*)_tmp145)!= 2)goto _LLD0;_tmp148=((
struct Cyc_Absyn_Less_kb_struct*)_tmp145)->f1;_tmp149=(struct Cyc_Core_Opt**)&((
struct Cyc_Absyn_Less_kb_struct*)_tmp145)->f1;_tmp14A=(void*)((struct Cyc_Absyn_Less_kb_struct*)
_tmp145)->f2;if((int)_tmp14A != 1)goto _LLD0;_LLCF: _tmp14C=_tmp149;goto _LLD1;_LLD0:
if(*((int*)_tmp145)!= 2)goto _LLD2;_tmp14B=((struct Cyc_Absyn_Less_kb_struct*)
_tmp145)->f1;_tmp14C=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)
_tmp145)->f1;_tmp14D=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmp145)->f2;if((
int)_tmp14D != 0)goto _LLD2;_LLD1:{struct Cyc_Core_Opt*_tmp31A;*_tmp14C=((_tmp31A=
_cycalloc(sizeof(*_tmp31A)),((_tmp31A->v=(void*)Cyc_Tcutil_kind_to_bound((void*)
2),_tmp31A))));}goto _LLCB;_LLD2: if(*((int*)_tmp145)!= 2)goto _LLD4;_tmp14E=((
struct Cyc_Absyn_Less_kb_struct*)_tmp145)->f1;_tmp14F=(struct Cyc_Core_Opt**)&((
struct Cyc_Absyn_Less_kb_struct*)_tmp145)->f1;_tmp150=(void*)((struct Cyc_Absyn_Less_kb_struct*)
_tmp145)->f2;if((int)_tmp150 != 5)goto _LLD4;_LLD3:{struct Cyc_Core_Opt*_tmp31B;*
_tmp14F=((_tmp31B=_cycalloc(sizeof(*_tmp31B)),((_tmp31B->v=(void*)Cyc_Tcutil_kind_to_bound((
void*)3),_tmp31B))));}goto _LLCB;_LLD4: if(*((int*)_tmp145)!= 0)goto _LLD6;_tmp151=(
void*)((struct Cyc_Absyn_Eq_kb_struct*)_tmp145)->f1;if((int)_tmp151 != 5)goto _LLD6;
_LLD5: if(!tud->is_flat){const char*_tmp321;void*_tmp320[3];struct Cyc_String_pa_struct
_tmp31F;struct Cyc_String_pa_struct _tmp31E;struct Cyc_String_pa_struct _tmp31D;(
_tmp31D.tag=0,((_tmp31D.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*((
struct Cyc_Absyn_Tvar*)tvs2->hd)->name),((_tmp31E.tag=0,((_tmp31E.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)*v),((_tmp31F.tag=0,((_tmp31F.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)obj),((_tmp320[0]=& _tmp31F,((_tmp320[1]=&
_tmp31E,((_tmp320[2]=& _tmp31D,Cyc_Tcutil_terr(loc,((_tmp321="%s %s attempts to abstract type variable %s of kind TR",
_tag_dyneither(_tmp321,sizeof(char),55))),_tag_dyneither(_tmp320,sizeof(void*),3)))))))))))))))))));}
goto _LLCB;_LLD6: if(*((int*)_tmp145)!= 2)goto _LLD8;_tmp152=((struct Cyc_Absyn_Less_kb_struct*)
_tmp145)->f1;_tmp153=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)
_tmp145)->f1;_tmp154=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmp145)->f2;_LLD7:
goto _LLD9;_LLD8: if(*((int*)_tmp145)!= 0)goto _LLDA;_tmp155=(void*)((struct Cyc_Absyn_Eq_kb_struct*)
_tmp145)->f1;if((int)_tmp155 != 4)goto _LLDA;_LLD9: if(!tud->is_flat){const char*
_tmp327;void*_tmp326[3];struct Cyc_String_pa_struct _tmp325;struct Cyc_String_pa_struct
_tmp324;struct Cyc_String_pa_struct _tmp323;(_tmp323.tag=0,((_tmp323.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)*((struct Cyc_Absyn_Tvar*)tvs2->hd)->name),((
_tmp324.tag=0,((_tmp324.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*v),((
_tmp325.tag=0,((_tmp325.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)obj),((
_tmp326[0]=& _tmp325,((_tmp326[1]=& _tmp324,((_tmp326[2]=& _tmp323,Cyc_Tcutil_terr(
loc,((_tmp327="%s %s attempts to abstract type variable %s of kind UR",
_tag_dyneither(_tmp327,sizeof(char),55))),_tag_dyneither(_tmp326,sizeof(void*),3)))))))))))))))))));}
goto _LLCB;_LLDA: if(*((int*)_tmp145)!= 0)goto _LLDC;_tmp156=(void*)((struct Cyc_Absyn_Eq_kb_struct*)
_tmp145)->f1;if((int)_tmp156 != 1)goto _LLDC;_LLDB:{const char*_tmp32D;void*_tmp32C[
3];struct Cyc_String_pa_struct _tmp32B;struct Cyc_String_pa_struct _tmp32A;struct Cyc_String_pa_struct
_tmp329;(_tmp329.tag=0,((_tmp329.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*((
struct Cyc_Absyn_Tvar*)tvs2->hd)->name),((_tmp32A.tag=0,((_tmp32A.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)*v),((_tmp32B.tag=0,((_tmp32B.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)obj),((_tmp32C[0]=& _tmp32B,((_tmp32C[1]=&
_tmp32A,((_tmp32C[2]=& _tmp329,Cyc_Tcutil_terr(loc,((_tmp32D="%s %s attempts to abstract type variable %s of kind M",
_tag_dyneither(_tmp32D,sizeof(char),54))),_tag_dyneither(_tmp32C,sizeof(void*),3)))))))))))))))))));}
goto _LLCB;_LLDC:;_LLDD: goto _LLCB;_LLCB:;}}Cyc_Tcutil_check_unique_tvars(loc,tvs);
Cyc_Tcutil_add_tvar_identities(tvs);{struct _RegionHandle _tmp168=_new_region("temp");
struct _RegionHandle*temp=& _tmp168;_push_region(temp);{struct Cyc_Absyn_Tuniondecl***
tud_opt;if(tud->is_xtunion){{struct _handler_cons _tmp169;_push_handler(& _tmp169);{
int _tmp16B=0;if(setjmp(_tmp169.handler))_tmp16B=1;if(!_tmp16B){tud_opt=Cyc_Tcenv_lookup_xtuniondecl(
temp,te,loc,tud->name);;_pop_handler();}else{void*_tmp16A=(void*)_exn_thrown;
void*_tmp16D=_tmp16A;_LLDF: if(_tmp16D != Cyc_Dict_Absent)goto _LLE1;_LLE0:{const
char*_tmp331;void*_tmp330[1];struct Cyc_String_pa_struct _tmp32F;(_tmp32F.tag=0,((
_tmp32F.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(
tud->name)),((_tmp330[0]=& _tmp32F,Cyc_Tcutil_terr(loc,((_tmp331="qualified xtunion declaration %s is not an existing xtunion",
_tag_dyneither(_tmp331,sizeof(char),60))),_tag_dyneither(_tmp330,sizeof(void*),1)))))));}
_npop_handler(0);return;_LLE1:;_LLE2:(void)_throw(_tmp16D);_LLDE:;}}}if(tud_opt
!= 0)tud->name=(*(*tud_opt))->name;else{union Cyc_Absyn_Nmspace_union _tmp332;(*
tud->name).f1=(union Cyc_Absyn_Nmspace_union)(((_tmp332.Abs_n).tag=2,(((_tmp332.Abs_n).f1=
te->ns,_tmp332))));}}else{{union Cyc_Absyn_Nmspace_union _tmp172=(*tud->name).f1;
struct Cyc_List_List*_tmp173;_LLE4: if((_tmp172.Rel_n).tag != 1)goto _LLE6;_tmp173=(
_tmp172.Rel_n).f1;if(_tmp173 != 0)goto _LLE6;_LLE5:{union Cyc_Absyn_Nmspace_union
_tmp333;(*tud->name).f1=(union Cyc_Absyn_Nmspace_union)(((_tmp333.Abs_n).tag=2,(((
_tmp333.Abs_n).f1=te->ns,_tmp333))));}goto _LLE3;_LLE6: if((_tmp172.Abs_n).tag != 2)
goto _LLE8;_LLE7: goto _LLE9;_LLE8:;_LLE9:{const char*_tmp337;void*_tmp336[1];struct
Cyc_String_pa_struct _tmp335;(_tmp335.tag=0,((_tmp335.f1=(struct _dyneither_ptr)((
struct _dyneither_ptr)Cyc_Absynpp_qvar2string(tud->name)),((_tmp336[0]=& _tmp335,
Cyc_Tcutil_terr(loc,((_tmp337="qualified tunion declarations are not implemented (%s)",
_tag_dyneither(_tmp337,sizeof(char),55))),_tag_dyneither(_tmp336,sizeof(void*),1)))))));}
_npop_handler(0);return;_LLE3:;}{struct Cyc_Absyn_Tuniondecl***_tmp178=((struct
Cyc_Absyn_Tuniondecl***(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k))Cyc_Dict_lookup_opt)(
ge->tuniondecls,v);struct Cyc_Absyn_Tuniondecl***_tmp338;tud_opt=(unsigned int)
_tmp178?(_tmp338=_region_malloc(temp,sizeof(*_tmp338)),((_tmp338[0]=*_tmp178,
_tmp338))): 0;}}{struct _tuple11 _tmp339;struct _tuple11 _tmp17B=(_tmp339.f1=tud->fields,((
_tmp339.f2=tud_opt,_tmp339)));struct Cyc_Core_Opt*_tmp17C;struct Cyc_Absyn_Tuniondecl***
_tmp17D;struct Cyc_Core_Opt*_tmp17E;struct Cyc_Core_Opt _tmp17F;struct Cyc_List_List*
_tmp180;struct Cyc_List_List**_tmp181;struct Cyc_Absyn_Tuniondecl***_tmp182;struct
Cyc_Core_Opt*_tmp183;struct Cyc_Core_Opt _tmp184;struct Cyc_List_List*_tmp185;
struct Cyc_List_List**_tmp186;struct Cyc_Absyn_Tuniondecl***_tmp187;struct Cyc_Absyn_Tuniondecl**
_tmp188;struct Cyc_Core_Opt*_tmp189;struct Cyc_Absyn_Tuniondecl***_tmp18A;struct
Cyc_Absyn_Tuniondecl**_tmp18B;_LLEB: _tmp17C=_tmp17B.f1;if(_tmp17C != 0)goto _LLED;
_tmp17D=_tmp17B.f2;if(_tmp17D != 0)goto _LLED;_LLEC:{struct Cyc_Absyn_Tuniondecl**
_tmp33A;ge->tuniondecls=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct
_dyneither_ptr*k,struct Cyc_Absyn_Tuniondecl**v))Cyc_Dict_insert)(ge->tuniondecls,
v,((_tmp33A=_cycalloc(sizeof(*_tmp33A)),((_tmp33A[0]=tud,_tmp33A)))));}goto _LLEA;
_LLED: _tmp17E=_tmp17B.f1;if(_tmp17E == 0)goto _LLEF;_tmp17F=*_tmp17E;_tmp180=(
struct Cyc_List_List*)_tmp17F.v;_tmp181=(struct Cyc_List_List**)&(*_tmp17B.f1).v;
_tmp182=_tmp17B.f2;if(_tmp182 != 0)goto _LLEF;_LLEE: {struct Cyc_Absyn_Tuniondecl*
_tmp33D;struct Cyc_Absyn_Tuniondecl**_tmp33C;struct Cyc_Absyn_Tuniondecl**_tmp18D=(
_tmp33C=_cycalloc(sizeof(*_tmp33C)),((_tmp33C[0]=((_tmp33D=_cycalloc(sizeof(*
_tmp33D)),((_tmp33D->sc=(void*)((void*)3),((_tmp33D->name=tud->name,((_tmp33D->tvs=
tvs,((_tmp33D->fields=0,((_tmp33D->is_xtunion=tud->is_xtunion,((_tmp33D->is_flat=
tud->is_flat,_tmp33D)))))))))))))),_tmp33C)));ge->tuniondecls=((struct Cyc_Dict_Dict(*)(
struct Cyc_Dict_Dict d,struct _dyneither_ptr*k,struct Cyc_Absyn_Tuniondecl**v))Cyc_Dict_insert)(
ge->tuniondecls,v,_tmp18D);*_tmp181=Cyc_Tc_tcTunionFields(te,ge,loc,obj,tud->is_xtunion,
tud->name,*_tmp181,tvs,tud);*_tmp18D=tud;goto _LLEA;}_LLEF: _tmp183=_tmp17B.f1;if(
_tmp183 == 0)goto _LLF1;_tmp184=*_tmp183;_tmp185=(struct Cyc_List_List*)_tmp184.v;
_tmp186=(struct Cyc_List_List**)&(*_tmp17B.f1).v;_tmp187=_tmp17B.f2;if(_tmp187 == 
0)goto _LLF1;_tmp188=*_tmp187;_LLF0: {struct Cyc_Absyn_Tuniondecl*_tmp190=*_tmp188;{
struct Cyc_Absyn_Tuniondecl*_tmp33E;*_tmp188=((_tmp33E=_cycalloc(sizeof(*_tmp33E)),((
_tmp33E->sc=(void*)((void*)3),((_tmp33E->name=tud->name,((_tmp33E->tvs=tvs,((
_tmp33E->fields=0,((_tmp33E->is_xtunion=tud->is_xtunion,((_tmp33E->is_flat=tud->is_flat,
_tmp33E))))))))))))));}*_tmp186=Cyc_Tc_tcTunionFields(te,ge,loc,obj,tud->is_xtunion,
tud->name,*_tmp186,tvs,tud);*_tmp188=_tmp190;_tmp18B=_tmp188;goto _LLF2;}_LLF1:
_tmp189=_tmp17B.f1;if(_tmp189 != 0)goto _LLEA;_tmp18A=_tmp17B.f2;if(_tmp18A == 0)
goto _LLEA;_tmp18B=*_tmp18A;_LLF2: {struct Cyc_Absyn_Tuniondecl*_tmp192=Cyc_Tcdecl_merge_tuniondecl(*
_tmp18B,tud,loc,Cyc_Tc_tc_msg);if(_tmp192 == 0){_npop_handler(0);return;}else{*
_tmp18B=(struct Cyc_Absyn_Tuniondecl*)_tmp192;goto _LLEA;}}_LLEA:;}};_pop_region(
temp);}}void Cyc_Tc_tcEnumdecl(struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,
struct Cyc_Position_Segment*loc,struct Cyc_Absyn_Enumdecl*ed);void Cyc_Tc_tcEnumdecl(
struct Cyc_Tcenv_Tenv*te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,
struct Cyc_Absyn_Enumdecl*ed){struct _dyneither_ptr*v=(*ed->name).f2;{union Cyc_Absyn_Nmspace_union
_tmp195=(*ed->name).f1;struct Cyc_List_List*_tmp196;struct Cyc_List_List*_tmp197;
_LLF4: if((_tmp195.Rel_n).tag != 1)goto _LLF6;_tmp196=(_tmp195.Rel_n).f1;if(_tmp196
!= 0)goto _LLF6;_LLF5: goto _LLF7;_LLF6: if((_tmp195.Abs_n).tag != 2)goto _LLF8;
_tmp197=(_tmp195.Abs_n).f1;if(_tmp197 != 0)goto _LLF8;_LLF7: goto _LLF3;_LLF8:;_LLF9:{
const char*_tmp342;void*_tmp341[1];struct Cyc_String_pa_struct _tmp340;(_tmp340.tag=
0,((_tmp340.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)Cyc_Absynpp_qvar2string(
ed->name)),((_tmp341[0]=& _tmp340,Cyc_Tcutil_terr(loc,((_tmp342="qualified enum declarations are not implemented (%s)",
_tag_dyneither(_tmp342,sizeof(char),53))),_tag_dyneither(_tmp341,sizeof(void*),1)))))));}
return;_LLF3:;}{union Cyc_Absyn_Nmspace_union _tmp343;(*ed->name).f1=(union Cyc_Absyn_Nmspace_union)(((
_tmp343.Abs_n).tag=2,(((_tmp343.Abs_n).f1=te->ns,_tmp343))));}if(ed->fields != 0){
struct _RegionHandle _tmp19C=_new_region("uprev_rgn");struct _RegionHandle*
uprev_rgn=& _tmp19C;_push_region(uprev_rgn);{struct Cyc_List_List*prev_fields=0;
unsigned int tag_count=0;struct Cyc_List_List*fs=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(ed->fields))->v;for(0;fs != 0;fs=fs->tl){struct Cyc_Absyn_Enumfield*
_tmp19D=(struct Cyc_Absyn_Enumfield*)fs->hd;if(((int(*)(int(*compare)(struct
_dyneither_ptr*,struct _dyneither_ptr*),struct Cyc_List_List*l,struct
_dyneither_ptr*x))Cyc_List_mem)(Cyc_strptrcmp,prev_fields,(*_tmp19D->name).f2)){
const char*_tmp347;void*_tmp346[1];struct Cyc_String_pa_struct _tmp345;(_tmp345.tag=
0,((_tmp345.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*(*_tmp19D->name).f2),((
_tmp346[0]=& _tmp345,Cyc_Tcutil_terr(_tmp19D->loc,((_tmp347="duplicate field name %s",
_tag_dyneither(_tmp347,sizeof(char),24))),_tag_dyneither(_tmp346,sizeof(void*),1)))))));}
else{struct Cyc_List_List*_tmp348;prev_fields=((_tmp348=_region_malloc(uprev_rgn,
sizeof(*_tmp348)),((_tmp348->hd=(*_tmp19D->name).f2,((_tmp348->tl=prev_fields,
_tmp348))))));}if(_tmp19D->tag == 0)_tmp19D->tag=(struct Cyc_Absyn_Exp*)Cyc_Absyn_uint_exp(
tag_count,_tmp19D->loc);else{if(!Cyc_Tcutil_is_const_exp(te,(struct Cyc_Absyn_Exp*)
_check_null(_tmp19D->tag))){const char*_tmp34D;void*_tmp34C[2];struct Cyc_String_pa_struct
_tmp34B;struct Cyc_String_pa_struct _tmp34A;(_tmp34A.tag=0,((_tmp34A.f1=(struct
_dyneither_ptr)((struct _dyneither_ptr)*(*_tmp19D->name).f2),((_tmp34B.tag=0,((
_tmp34B.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)*v),((_tmp34C[0]=&
_tmp34B,((_tmp34C[1]=& _tmp34A,Cyc_Tcutil_terr(loc,((_tmp34D="enum %s, field %s: expression is not constant",
_tag_dyneither(_tmp34D,sizeof(char),46))),_tag_dyneither(_tmp34C,sizeof(void*),2)))))))))))));}}{
unsigned int _tmp1A7;int _tmp1A8;struct _tuple4 _tmp1A6=Cyc_Evexp_eval_const_uint_exp((
struct Cyc_Absyn_Exp*)_check_null(_tmp19D->tag));_tmp1A7=_tmp1A6.f1;_tmp1A8=
_tmp1A6.f2;if(!_tmp1A8){const char*_tmp350;void*_tmp34F;(_tmp34F=0,Cyc_Tcutil_terr(
loc,((_tmp350="Cyclone enum tags cannot use sizeof or offsetof",_tag_dyneither(
_tmp350,sizeof(char),48))),_tag_dyneither(_tmp34F,sizeof(void*),0)));}tag_count=
_tmp1A7 + 1;{union Cyc_Absyn_Nmspace_union _tmp351;(*_tmp19D->name).f1=(union Cyc_Absyn_Nmspace_union)(((
_tmp351.Abs_n).tag=2,(((_tmp351.Abs_n).f1=te->ns,_tmp351))));}}}};_pop_region(
uprev_rgn);}{struct _handler_cons _tmp1AC;_push_handler(& _tmp1AC);{int _tmp1AE=0;
if(setjmp(_tmp1AC.handler))_tmp1AE=1;if(!_tmp1AE){{struct Cyc_Absyn_Enumdecl**
_tmp1AF=((struct Cyc_Absyn_Enumdecl**(*)(struct Cyc_Dict_Dict d,struct
_dyneither_ptr*k))Cyc_Dict_lookup)(ge->enumdecls,v);struct Cyc_Absyn_Enumdecl*
_tmp1B0=Cyc_Tcdecl_merge_enumdecl(*_tmp1AF,ed,loc,Cyc_Tc_tc_msg);if(_tmp1B0 == 0){
_npop_handler(0);return;}*_tmp1AF=(struct Cyc_Absyn_Enumdecl*)_tmp1B0;};
_pop_handler();}else{void*_tmp1AD=(void*)_exn_thrown;void*_tmp1B2=_tmp1AD;_LLFB:
if(_tmp1B2 != Cyc_Dict_Absent)goto _LLFD;_LLFC: {struct Cyc_Absyn_Enumdecl**_tmp352;
struct Cyc_Absyn_Enumdecl**_tmp1B3=(_tmp352=_cycalloc(sizeof(*_tmp352)),((_tmp352[
0]=ed,_tmp352)));ge->enumdecls=((struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,
struct _dyneither_ptr*k,struct Cyc_Absyn_Enumdecl**v))Cyc_Dict_insert)(ge->enumdecls,
v,_tmp1B3);goto _LLFA;}_LLFD:;_LLFE:(void)_throw(_tmp1B2);_LLFA:;}}}if(ed->fields
!= 0){struct Cyc_List_List*fs=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(ed->fields))->v;for(0;fs != 0;fs=fs->tl){struct Cyc_Absyn_Enumfield*
_tmp1B5=(struct Cyc_Absyn_Enumfield*)fs->hd;struct Cyc_Tcenv_EnumRes_struct*
_tmp358;struct Cyc_Tcenv_EnumRes_struct _tmp357;struct _tuple7*_tmp356;ge->ordinaries=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct _dyneither_ptr*k,struct _tuple7*
v))Cyc_Dict_insert)(ge->ordinaries,(*_tmp1B5->name).f2,(struct _tuple7*)((_tmp356=
_cycalloc(sizeof(*_tmp356)),((_tmp356->f1=(void*)((_tmp358=_cycalloc(sizeof(*
_tmp358)),((_tmp358[0]=((_tmp357.tag=3,((_tmp357.f1=ed,((_tmp357.f2=_tmp1B5,
_tmp357)))))),_tmp358)))),((_tmp356->f2=1,_tmp356)))))));}}}static int Cyc_Tc_okay_externC(
struct Cyc_Position_Segment*loc,void*sc);static int Cyc_Tc_okay_externC(struct Cyc_Position_Segment*
loc,void*sc){void*_tmp1B9=sc;_LL100: if((int)_tmp1B9 != 0)goto _LL102;_LL101:{const
char*_tmp35B;void*_tmp35A;(_tmp35A=0,Cyc_Tcutil_warn(loc,((_tmp35B="static declaration nested within extern \"C\"",
_tag_dyneither(_tmp35B,sizeof(char),44))),_tag_dyneither(_tmp35A,sizeof(void*),0)));}
return 0;_LL102: if((int)_tmp1B9 != 1)goto _LL104;_LL103:{const char*_tmp35E;void*
_tmp35D;(_tmp35D=0,Cyc_Tcutil_warn(loc,((_tmp35E="abstract declaration nested within extern \"C\"",
_tag_dyneither(_tmp35E,sizeof(char),46))),_tag_dyneither(_tmp35D,sizeof(void*),0)));}
return 0;_LL104: if((int)_tmp1B9 != 2)goto _LL106;_LL105: goto _LL107;_LL106: if((int)
_tmp1B9 != 5)goto _LL108;_LL107: goto _LL109;_LL108: if((int)_tmp1B9 != 3)goto _LL10A;
_LL109: return 1;_LL10A: if((int)_tmp1B9 != 4)goto _LLFF;_LL10B:{const char*_tmp361;
void*_tmp360;(_tmp360=0,Cyc_Tcutil_warn(loc,((_tmp361="nested extern \"C\" declaration",
_tag_dyneither(_tmp361,sizeof(char),30))),_tag_dyneither(_tmp360,sizeof(void*),0)));}
return 1;_LLFF:;}static void Cyc_Tc_resolve_export_namespace(struct Cyc_Tcenv_Tenv*
te,struct _tuple6*exp);static void Cyc_Tc_resolve_export_namespace(struct Cyc_Tcenv_Tenv*
te,struct _tuple6*exp){struct Cyc_Position_Segment*_tmp1C1;struct _tuple0*_tmp1C2;
struct _tuple6 _tmp1C0=*exp;_tmp1C1=_tmp1C0.f1;_tmp1C2=_tmp1C0.f2;{struct _tuple0
_tmp1C4;union Cyc_Absyn_Nmspace_union _tmp1C5;struct _dyneither_ptr*_tmp1C6;struct
_tuple0*_tmp1C3=_tmp1C2;_tmp1C4=*_tmp1C3;_tmp1C5=_tmp1C4.f1;_tmp1C6=_tmp1C4.f2;{
union Cyc_Absyn_Nmspace_union _tmp1C7=_tmp1C5;struct Cyc_List_List*_tmp1C8;struct
Cyc_List_List*_tmp1C9;_LL10D: if((_tmp1C7.Rel_n).tag != 1)goto _LL10F;_tmp1C8=(
_tmp1C7.Rel_n).f1;if(_tmp1C8 != 0)goto _LL10F;_LL10E: goto _LL110;_LL10F: if((_tmp1C7.Abs_n).tag
!= 2)goto _LL111;_tmp1C9=(_tmp1C7.Abs_n).f1;if(_tmp1C9 != 0)goto _LL111;_LL110: goto
_LL10C;_LL111:;_LL112:{const char*_tmp365;void*_tmp364[1];struct Cyc_String_pa_struct
_tmp363;(_tmp363.tag=0,((_tmp363.f1=(struct _dyneither_ptr)((struct _dyneither_ptr)
Cyc_Absynpp_qvar2string(_tmp1C2)),((_tmp364[0]=& _tmp363,Cyc_Tcutil_terr(_tmp1C1,((
_tmp365="qualified export variables are not implemented (%s)",_tag_dyneither(
_tmp365,sizeof(char),52))),_tag_dyneither(_tmp364,sizeof(void*),1)))))));}
return;_LL10C:;}{union Cyc_Absyn_Nmspace_union _tmp366;(*_tmp1C2).f1=(union Cyc_Absyn_Nmspace_union)(((
_tmp366.Abs_n).tag=2,(((_tmp366.Abs_n).f1=te->ns,_tmp366))));}}}static void Cyc_Tc_tc_decls(
struct Cyc_Tcenv_Tenv*te,struct Cyc_List_List*ds0,int in_externC,int
in_externCinclude,int check_var_init,struct _RegionHandle*grgn,struct Cyc_List_List**
exports);static void Cyc_Tc_tc_decls(struct Cyc_Tcenv_Tenv*te,struct Cyc_List_List*
ds0,int in_externC,int in_externCinclude,int check_var_init,struct _RegionHandle*
grgn,struct Cyc_List_List**exports){struct Cyc_Tcenv_Genv*ge=((struct Cyc_Tcenv_Genv*(*)(
struct Cyc_Dict_Dict d,struct Cyc_List_List*k))Cyc_Dict_lookup)(te->ae,te->ns);
struct Cyc_List_List*last=0;struct Cyc_Dict_Dict dict=Cyc_Tcgenrep_empty_typerep_dict();
struct Cyc_List_List*_tmp1CE=ds0;for(0;_tmp1CE != 0;(last=_tmp1CE,_tmp1CE=_tmp1CE->tl)){
struct Cyc_Absyn_Decl*d=(struct Cyc_Absyn_Decl*)_tmp1CE->hd;struct Cyc_Position_Segment*
loc=d->loc;void*_tmp1CF=(void*)d->r;struct Cyc_Absyn_Vardecl*_tmp1D0;struct Cyc_Absyn_Fndecl*
_tmp1D1;struct Cyc_Absyn_Typedefdecl*_tmp1D2;struct Cyc_Absyn_Aggrdecl*_tmp1D3;
struct Cyc_Absyn_Tuniondecl*_tmp1D4;struct Cyc_Absyn_Enumdecl*_tmp1D5;struct
_dyneither_ptr*_tmp1D6;struct Cyc_List_List*_tmp1D7;struct _tuple0*_tmp1D8;struct
_tuple0 _tmp1D9;union Cyc_Absyn_Nmspace_union _tmp1DA;struct _dyneither_ptr*_tmp1DB;
struct Cyc_List_List*_tmp1DC;struct Cyc_List_List*_tmp1DD;struct Cyc_List_List*
_tmp1DE;struct Cyc_List_List*_tmp1DF;_LL114: if(_tmp1CF <= (void*)2)goto _LL128;if(*((
int*)_tmp1CF)!= 2)goto _LL116;_LL115: goto _LL117;_LL116: if(*((int*)_tmp1CF)!= 3)
goto _LL118;_LL117:{const char*_tmp369;void*_tmp368;(_tmp368=0,Cyc_Tcutil_terr(loc,((
_tmp369="top level let-declarations are not implemented",_tag_dyneither(_tmp369,
sizeof(char),47))),_tag_dyneither(_tmp368,sizeof(void*),0)));}goto _LL113;_LL118:
if(*((int*)_tmp1CF)!= 4)goto _LL11A;_LL119:{const char*_tmp36C;void*_tmp36B;(
_tmp36B=0,Cyc_Tcutil_terr(loc,((_tmp36C="top level region declarations are not implemented",
_tag_dyneither(_tmp36C,sizeof(char),50))),_tag_dyneither(_tmp36B,sizeof(void*),0)));}
goto _LL113;_LL11A: if(*((int*)_tmp1CF)!= 5)goto _LL11C;_LL11B:{const char*_tmp36F;
void*_tmp36E;(_tmp36E=0,Cyc_Tcutil_terr(loc,((_tmp36F="top level alias declarations are not implemented",
_tag_dyneither(_tmp36F,sizeof(char),49))),_tag_dyneither(_tmp36E,sizeof(void*),0)));}
goto _LL113;_LL11C: if(*((int*)_tmp1CF)!= 0)goto _LL11E;_tmp1D0=((struct Cyc_Absyn_Var_d_struct*)
_tmp1CF)->f1;_LL11D: if(in_externC  && Cyc_Tc_okay_externC(d->loc,(void*)_tmp1D0->sc))(
void*)(_tmp1D0->sc=(void*)((void*)4));if(_tmp1D0->initializer != 0){void*_tmp1E6=(
void*)((struct Cyc_Absyn_Exp*)_check_null(_tmp1D0->initializer))->r;void*_tmp1E7;
_LL135: if(*((int*)_tmp1E6)!= 21)goto _LL137;_tmp1E7=(void*)((struct Cyc_Absyn_Gentyp_e_struct*)
_tmp1E6)->f2;_LL136: {struct Cyc_Dict_Dict _tmp1E9;struct Cyc_List_List*_tmp1EA;
struct Cyc_Absyn_Exp*_tmp1EB;struct _tuple5 _tmp1E8=Cyc_Tcgenrep_tcGenrep(te,ge,loc,
_tmp1E7,dict);_tmp1E9=_tmp1E8.f1;_tmp1EA=_tmp1E8.f2;_tmp1EB=_tmp1E8.f3;dict=
_tmp1E9;Cyc_Tc_tc_decls(te,_tmp1EA,in_externC,in_externCinclude,check_var_init,
grgn,exports);_tmp1D0->initializer=(struct Cyc_Absyn_Exp*)_tmp1EB;Cyc_Tc_tcVardecl(
te,ge,loc,_tmp1D0,check_var_init,in_externCinclude,exports);if(_tmp1EA != 0){if(
last != 0){((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_imp_append)(_tmp1EA,_tmp1CE);last->tl=_tmp1EA;}else{struct Cyc_List_List
_tmp370;struct Cyc_List_List tmp=(_tmp370.hd=(struct Cyc_Absyn_Decl*)_tmp1CE->hd,((
_tmp370.tl=_tmp1CE->tl,_tmp370)));(struct Cyc_Absyn_Decl*)(_tmp1CE->hd=(void*)((
struct Cyc_Absyn_Decl*)_tmp1EA->hd));_tmp1CE->tl=_tmp1EA->tl;(struct Cyc_Absyn_Decl*)(
_tmp1EA->hd=(void*)((struct Cyc_Absyn_Decl*)tmp.hd));_tmp1EA->tl=tmp.tl;((struct
Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(
_tmp1CE,_tmp1EA);}}continue;}_LL137:;_LL138: goto _LL134;_LL134:;}Cyc_Tc_tcVardecl(
te,ge,loc,_tmp1D0,check_var_init,in_externCinclude,exports);goto _LL113;_LL11E:
if(*((int*)_tmp1CF)!= 1)goto _LL120;_tmp1D1=((struct Cyc_Absyn_Fn_d_struct*)
_tmp1CF)->f1;_LL11F: if(in_externC  && Cyc_Tc_okay_externC(d->loc,(void*)_tmp1D1->sc))(
void*)(_tmp1D1->sc=(void*)((void*)4));Cyc_Tc_tcFndecl(te,ge,loc,_tmp1D1,
in_externCinclude,exports);goto _LL113;_LL120: if(*((int*)_tmp1CF)!= 9)goto _LL122;
_tmp1D2=((struct Cyc_Absyn_Typedef_d_struct*)_tmp1CF)->f1;_LL121: Cyc_Tc_tcTypedefdecl(
te,ge,loc,_tmp1D2);goto _LL113;_LL122: if(*((int*)_tmp1CF)!= 6)goto _LL124;_tmp1D3=((
struct Cyc_Absyn_Aggr_d_struct*)_tmp1CF)->f1;_LL123: if(in_externC  && Cyc_Tc_okay_externC(
d->loc,(void*)_tmp1D3->sc))(void*)(_tmp1D3->sc=(void*)((void*)4));Cyc_Tc_tcAggrdecl(
te,ge,loc,_tmp1D3);goto _LL113;_LL124: if(*((int*)_tmp1CF)!= 7)goto _LL126;_tmp1D4=((
struct Cyc_Absyn_Tunion_d_struct*)_tmp1CF)->f1;_LL125: if(in_externC  && Cyc_Tc_okay_externC(
d->loc,(void*)_tmp1D4->sc))(void*)(_tmp1D4->sc=(void*)((void*)4));Cyc_Tc_tcTuniondecl(
te,ge,loc,_tmp1D4);goto _LL113;_LL126: if(*((int*)_tmp1CF)!= 8)goto _LL128;_tmp1D5=((
struct Cyc_Absyn_Enum_d_struct*)_tmp1CF)->f1;_LL127: if(in_externC  && Cyc_Tc_okay_externC(
d->loc,(void*)_tmp1D5->sc))(void*)(_tmp1D5->sc=(void*)((void*)4));Cyc_Tc_tcEnumdecl(
te,ge,loc,_tmp1D5);goto _LL113;_LL128: if((int)_tmp1CF != 0)goto _LL12A;_LL129:{
const char*_tmp373;void*_tmp372;(_tmp372=0,Cyc_Tcutil_warn(d->loc,((_tmp373="spurious __cyclone_port_on__",
_tag_dyneither(_tmp373,sizeof(char),29))),_tag_dyneither(_tmp372,sizeof(void*),0)));}
goto _LL113;_LL12A: if((int)_tmp1CF != 1)goto _LL12C;_LL12B: goto _LL113;_LL12C: if(
_tmp1CF <= (void*)2)goto _LL12E;if(*((int*)_tmp1CF)!= 10)goto _LL12E;_tmp1D6=((
struct Cyc_Absyn_Namespace_d_struct*)_tmp1CF)->f1;_tmp1D7=((struct Cyc_Absyn_Namespace_d_struct*)
_tmp1CF)->f2;_LL12D: {struct Cyc_List_List*_tmp1EF=te->ns;struct Cyc_List_List*
_tmp374;struct Cyc_List_List*_tmp1F0=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_append)(_tmp1EF,((_tmp374=_cycalloc(sizeof(*
_tmp374)),((_tmp374->hd=_tmp1D6,((_tmp374->tl=0,_tmp374)))))));if(!((int(*)(
struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_member)(ge->namespaces,
_tmp1D6)){ge->namespaces=((struct Cyc_Set_Set*(*)(struct _RegionHandle*r,struct Cyc_Set_Set*
s,struct _dyneither_ptr*elt))Cyc_Set_rinsert)(grgn,ge->namespaces,_tmp1D6);te->ae=((
struct Cyc_Dict_Dict(*)(struct Cyc_Dict_Dict d,struct Cyc_List_List*k,struct Cyc_Tcenv_Genv*
v))Cyc_Dict_insert)(te->ae,_tmp1F0,Cyc_Tcenv_empty_genv(grgn));}te->ns=_tmp1F0;
Cyc_Tc_tc_decls(te,_tmp1D7,in_externC,in_externCinclude,check_var_init,grgn,
exports);te->ns=_tmp1EF;goto _LL113;}_LL12E: if(_tmp1CF <= (void*)2)goto _LL130;if(*((
int*)_tmp1CF)!= 11)goto _LL130;_tmp1D8=((struct Cyc_Absyn_Using_d_struct*)_tmp1CF)->f1;
_tmp1D9=*_tmp1D8;_tmp1DA=_tmp1D9.f1;_tmp1DB=_tmp1D9.f2;_tmp1DC=((struct Cyc_Absyn_Using_d_struct*)
_tmp1CF)->f2;_LL12F: {struct _dyneither_ptr*first;struct Cyc_List_List*rest;{union
Cyc_Absyn_Nmspace_union _tmp1F2=_tmp1DA;struct Cyc_List_List*_tmp1F3;struct Cyc_List_List*
_tmp1F4;struct Cyc_List_List*_tmp1F5;struct Cyc_List_List _tmp1F6;struct
_dyneither_ptr*_tmp1F7;struct Cyc_List_List*_tmp1F8;struct Cyc_List_List*_tmp1F9;
struct Cyc_List_List _tmp1FA;struct _dyneither_ptr*_tmp1FB;struct Cyc_List_List*
_tmp1FC;_LL13A: if((_tmp1F2.Loc_n).tag != 0)goto _LL13C;_LL13B: goto _LL13D;_LL13C:
if((_tmp1F2.Rel_n).tag != 1)goto _LL13E;_tmp1F3=(_tmp1F2.Rel_n).f1;if(_tmp1F3 != 0)
goto _LL13E;_LL13D: goto _LL13F;_LL13E: if((_tmp1F2.Abs_n).tag != 2)goto _LL140;
_tmp1F4=(_tmp1F2.Abs_n).f1;if(_tmp1F4 != 0)goto _LL140;_LL13F: first=_tmp1DB;rest=0;
goto _LL139;_LL140: if((_tmp1F2.Rel_n).tag != 1)goto _LL142;_tmp1F5=(_tmp1F2.Rel_n).f1;
if(_tmp1F5 == 0)goto _LL142;_tmp1F6=*_tmp1F5;_tmp1F7=(struct _dyneither_ptr*)
_tmp1F6.hd;_tmp1F8=_tmp1F6.tl;_LL141: _tmp1FB=_tmp1F7;_tmp1FC=_tmp1F8;goto _LL143;
_LL142: if((_tmp1F2.Abs_n).tag != 2)goto _LL139;_tmp1F9=(_tmp1F2.Abs_n).f1;if(
_tmp1F9 == 0)goto _LL139;_tmp1FA=*_tmp1F9;_tmp1FB=(struct _dyneither_ptr*)_tmp1FA.hd;
_tmp1FC=_tmp1FA.tl;_LL143: first=_tmp1FB;{struct Cyc_List_List*_tmp375;rest=((
struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_append)(
_tmp1FC,((_tmp375=_cycalloc(sizeof(*_tmp375)),((_tmp375->hd=_tmp1DB,((_tmp375->tl=
0,_tmp375)))))));}goto _LL139;_LL139:;}{struct Cyc_List_List*_tmp1FE=Cyc_Tcenv_resolve_namespace(
te,loc,first,rest);{struct Cyc_List_List*_tmp376;ge->availables=(struct Cyc_List_List*)((
_tmp376=_cycalloc(sizeof(*_tmp376)),((_tmp376->hd=_tmp1FE,((_tmp376->tl=ge->availables,
_tmp376))))));}Cyc_Tc_tc_decls(te,_tmp1DC,in_externC,in_externCinclude,
check_var_init,grgn,exports);ge->availables=((struct Cyc_List_List*)_check_null(
ge->availables))->tl;goto _LL113;}}_LL130: if(_tmp1CF <= (void*)2)goto _LL132;if(*((
int*)_tmp1CF)!= 12)goto _LL132;_tmp1DD=((struct Cyc_Absyn_ExternC_d_struct*)
_tmp1CF)->f1;_LL131: Cyc_Tc_tc_decls(te,_tmp1DD,1,in_externCinclude,
check_var_init,grgn,exports);goto _LL113;_LL132: if(_tmp1CF <= (void*)2)goto _LL113;
if(*((int*)_tmp1CF)!= 13)goto _LL113;_tmp1DE=((struct Cyc_Absyn_ExternCinclude_d_struct*)
_tmp1CF)->f1;_tmp1DF=((struct Cyc_Absyn_ExternCinclude_d_struct*)_tmp1CF)->f2;
_LL133:((void(*)(void(*f)(struct Cyc_Tcenv_Tenv*,struct _tuple6*),struct Cyc_Tcenv_Tenv*
env,struct Cyc_List_List*x))Cyc_List_iter_c)(Cyc_Tc_resolve_export_namespace,te,
_tmp1DF);{struct Cyc_List_List*newexs=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_append)(_tmp1DF,(unsigned int)exports?*exports:
0);Cyc_Tc_tc_decls(te,_tmp1DE,1,1,check_var_init,grgn,(struct Cyc_List_List**)&
newexs);for(0;_tmp1DF != 0;_tmp1DF=_tmp1DF->tl){struct _tuple6*_tmp200=(struct
_tuple6*)_tmp1DF->hd;if(!(*_tmp200).f3){const char*_tmp37A;void*_tmp379[1];struct
Cyc_String_pa_struct _tmp378;(_tmp378.tag=0,((_tmp378.f1=(struct _dyneither_ptr)((
struct _dyneither_ptr)Cyc_Absynpp_qvar2string((*_tmp200).f2)),((_tmp379[0]=&
_tmp378,Cyc_Tcutil_warn((*_tmp200).f1,((_tmp37A="%s is exported but not defined",
_tag_dyneither(_tmp37A,sizeof(char),31))),_tag_dyneither(_tmp379,sizeof(void*),1)))))));}}
goto _LL113;}_LL113:;}}void Cyc_Tc_tc(struct _RegionHandle*g,struct Cyc_Tcenv_Tenv*
te,int check_var_init,struct Cyc_List_List*ds);void Cyc_Tc_tc(struct _RegionHandle*g,
struct Cyc_Tcenv_Tenv*te,int check_var_init,struct Cyc_List_List*ds){Cyc_Absynpp_set_params(&
Cyc_Absynpp_tc_params_r);Cyc_Tc_tc_decls(te,ds,0,0,check_var_init,g,0);}struct
Cyc_Tc_TreeshakeEnv{struct _RegionHandle*rgn;int in_cinclude;struct Cyc_Dict_Dict
nsdict;};static int Cyc_Tc_vardecl_needed(struct Cyc_Tc_TreeshakeEnv*env,struct Cyc_Absyn_Decl*
d);static struct Cyc_List_List*Cyc_Tc_treeshake_f(struct Cyc_Tc_TreeshakeEnv*env,
struct Cyc_List_List*ds);static struct Cyc_List_List*Cyc_Tc_treeshake_f(struct Cyc_Tc_TreeshakeEnv*
env,struct Cyc_List_List*ds){return((struct Cyc_List_List*(*)(int(*f)(struct Cyc_Tc_TreeshakeEnv*,
struct Cyc_Absyn_Decl*),struct Cyc_Tc_TreeshakeEnv*env,struct Cyc_List_List*x))Cyc_List_filter_c)(
Cyc_Tc_vardecl_needed,env,ds);}struct _tuple12{struct Cyc_Tcenv_Genv*f1;struct Cyc_Set_Set*
f2;};static int Cyc_Tc_vardecl_needed(struct Cyc_Tc_TreeshakeEnv*env,struct Cyc_Absyn_Decl*
d);static int Cyc_Tc_vardecl_needed(struct Cyc_Tc_TreeshakeEnv*env,struct Cyc_Absyn_Decl*
d){void*_tmp204=(void*)d->r;struct Cyc_Absyn_Vardecl*_tmp205;struct Cyc_List_List*
_tmp206;struct Cyc_List_List**_tmp207;struct Cyc_List_List*_tmp208;struct Cyc_List_List**
_tmp209;struct Cyc_List_List*_tmp20A;struct Cyc_List_List**_tmp20B;struct Cyc_List_List*
_tmp20C;struct Cyc_List_List**_tmp20D;_LL145: if(_tmp204 <= (void*)2)goto _LL14F;if(*((
int*)_tmp204)!= 0)goto _LL147;_tmp205=((struct Cyc_Absyn_Var_d_struct*)_tmp204)->f1;
_LL146: if(env->in_cinclude  || (void*)_tmp205->sc != (void*)3  && (void*)_tmp205->sc
!= (void*)4)return 1;{struct _tuple0 _tmp20F;union Cyc_Absyn_Nmspace_union _tmp210;
struct _dyneither_ptr*_tmp211;struct _tuple0*_tmp20E=_tmp205->name;_tmp20F=*
_tmp20E;_tmp210=_tmp20F.f1;_tmp211=_tmp20F.f2;{struct Cyc_List_List*ns;{union Cyc_Absyn_Nmspace_union
_tmp212=_tmp210;struct Cyc_List_List*_tmp213;struct Cyc_List_List*_tmp214;_LL152:
if((_tmp212.Loc_n).tag != 0)goto _LL154;_LL153: ns=0;goto _LL151;_LL154: if((_tmp212.Rel_n).tag
!= 1)goto _LL156;_tmp213=(_tmp212.Rel_n).f1;_LL155: ns=_tmp213;goto _LL151;_LL156:
if((_tmp212.Abs_n).tag != 2)goto _LL151;_tmp214=(_tmp212.Abs_n).f1;_LL157: ns=
_tmp214;goto _LL151;_LL151:;}{struct _tuple12*_tmp215=((struct _tuple12*(*)(struct
Cyc_Dict_Dict d,struct Cyc_List_List*k))Cyc_Dict_lookup)(env->nsdict,ns);struct Cyc_Tcenv_Genv*
_tmp216=(*_tmp215).f1;int _tmp217=(*((struct _tuple7*(*)(struct Cyc_Dict_Dict d,
struct _dyneither_ptr*k))Cyc_Dict_lookup)(_tmp216->ordinaries,_tmp211)).f2;if(!
_tmp217)(*_tmp215).f2=((struct Cyc_Set_Set*(*)(struct _RegionHandle*r,struct Cyc_Set_Set*
s,struct _dyneither_ptr*elt))Cyc_Set_rinsert)(env->rgn,(*_tmp215).f2,_tmp211);
return _tmp217;}}}_LL147: if(*((int*)_tmp204)!= 12)goto _LL149;_tmp206=((struct Cyc_Absyn_ExternC_d_struct*)
_tmp204)->f1;_tmp207=(struct Cyc_List_List**)&((struct Cyc_Absyn_ExternC_d_struct*)
_tmp204)->f1;_LL148: _tmp209=_tmp207;goto _LL14A;_LL149: if(*((int*)_tmp204)!= 11)
goto _LL14B;_tmp208=((struct Cyc_Absyn_Using_d_struct*)_tmp204)->f2;_tmp209=(
struct Cyc_List_List**)&((struct Cyc_Absyn_Using_d_struct*)_tmp204)->f2;_LL14A:
_tmp20B=_tmp209;goto _LL14C;_LL14B: if(*((int*)_tmp204)!= 10)goto _LL14D;_tmp20A=((
struct Cyc_Absyn_Namespace_d_struct*)_tmp204)->f2;_tmp20B=(struct Cyc_List_List**)&((
struct Cyc_Absyn_Namespace_d_struct*)_tmp204)->f2;_LL14C:*_tmp20B=Cyc_Tc_treeshake_f(
env,*_tmp20B);return 1;_LL14D: if(*((int*)_tmp204)!= 13)goto _LL14F;_tmp20C=((
struct Cyc_Absyn_ExternCinclude_d_struct*)_tmp204)->f1;_tmp20D=(struct Cyc_List_List**)&((
struct Cyc_Absyn_ExternCinclude_d_struct*)_tmp204)->f1;_LL14E: {int in_cinclude=
env->in_cinclude;env->in_cinclude=1;*_tmp20D=Cyc_Tc_treeshake_f(env,*_tmp20D);
env->in_cinclude=in_cinclude;return 1;}_LL14F:;_LL150: return 1;_LL144:;}static int
Cyc_Tc_treeshake_remove_f(struct Cyc_Set_Set*set,struct _dyneither_ptr*x,void*y);
static int Cyc_Tc_treeshake_remove_f(struct Cyc_Set_Set*set,struct _dyneither_ptr*x,
void*y){return !((int(*)(struct Cyc_Set_Set*s,struct _dyneither_ptr*elt))Cyc_Set_member)(
set,x);}static struct _tuple12*Cyc_Tc_treeshake_make_env_f(struct _RegionHandle*rgn,
struct Cyc_Tcenv_Genv*ge);static struct _tuple12*Cyc_Tc_treeshake_make_env_f(struct
_RegionHandle*rgn,struct Cyc_Tcenv_Genv*ge){struct _tuple12*_tmp37B;return(_tmp37B=
_region_malloc(rgn,sizeof(*_tmp37B)),((_tmp37B->f1=ge,((_tmp37B->f2=((struct Cyc_Set_Set*(*)(
struct _RegionHandle*r,int(*cmp)(struct _dyneither_ptr*,struct _dyneither_ptr*)))
Cyc_Set_rempty)(rgn,Cyc_strptrcmp),_tmp37B)))));}struct _tuple13{struct Cyc_List_List*
f1;struct _tuple12*f2;};struct Cyc_List_List*Cyc_Tc_treeshake(struct Cyc_Tcenv_Tenv*
te,struct Cyc_List_List*ds);struct Cyc_List_List*Cyc_Tc_treeshake(struct Cyc_Tcenv_Tenv*
te,struct Cyc_List_List*ds){struct _RegionHandle _tmp219=_new_region("rgn");struct
_RegionHandle*rgn=& _tmp219;_push_region(rgn);{struct Cyc_Tc_TreeshakeEnv _tmp37C;
struct Cyc_Tc_TreeshakeEnv _tmp21A=(_tmp37C.rgn=rgn,((_tmp37C.in_cinclude=0,((
_tmp37C.nsdict=((struct Cyc_Dict_Dict(*)(struct _RegionHandle*,struct _tuple12*(*f)(
struct _RegionHandle*,struct Cyc_Tcenv_Genv*),struct _RegionHandle*env,struct Cyc_Dict_Dict
d))Cyc_Dict_rmap_c)(rgn,Cyc_Tc_treeshake_make_env_f,rgn,te->ae),_tmp37C)))));
struct Cyc_List_List*_tmp21B=Cyc_Tc_treeshake_f(& _tmp21A,ds);if(((int(*)(struct
Cyc_Dict_Dict d))Cyc_Dict_is_empty)(_tmp21A.nsdict)){struct Cyc_List_List*_tmp21C=
_tmp21B;_npop_handler(0);return _tmp21C;}{struct Cyc_Iter_Iter _tmp21D=((struct Cyc_Iter_Iter(*)(
struct _RegionHandle*rgn,struct Cyc_Dict_Dict d))Cyc_Dict_make_iter)(Cyc_Core_heap_region,
_tmp21A.nsdict);struct _tuple13 _tmp21E=*((struct _tuple13*(*)(struct _RegionHandle*
r,struct Cyc_Dict_Dict d))Cyc_Dict_rchoose)(rgn,_tmp21A.nsdict);while(((int(*)(
struct Cyc_Iter_Iter,struct _tuple13*))Cyc_Iter_next)(_tmp21D,& _tmp21E)){struct
_tuple12*_tmp220;struct _tuple12 _tmp221;struct Cyc_Tcenv_Genv*_tmp222;struct Cyc_Set_Set*
_tmp223;struct _tuple13 _tmp21F=_tmp21E;_tmp220=_tmp21F.f2;_tmp221=*_tmp220;
_tmp222=_tmp221.f1;_tmp223=_tmp221.f2;_tmp222->ordinaries=((struct Cyc_Dict_Dict(*)(
struct _RegionHandle*,int(*f)(struct Cyc_Set_Set*,struct _dyneither_ptr*,struct
_tuple7*),struct Cyc_Set_Set*env,struct Cyc_Dict_Dict d))Cyc_Dict_rfilter_c)(
_tmp222->grgn,(int(*)(struct Cyc_Set_Set*set,struct _dyneither_ptr*x,struct _tuple7*
y))Cyc_Tc_treeshake_remove_f,_tmp223,_tmp222->ordinaries);}{struct Cyc_List_List*
_tmp224=_tmp21B;_npop_handler(0);return _tmp224;}}};_pop_region(rgn);}
