// This is a C header file to be used by the output of the Cyclone
// to C translator.  The corresponding definitions are in file lib/runtime_cyc.c
#ifndef _CYC_INCLUDE_H_
#define _CYC_INCLUDE_H_

#include <setjmp.h>

#ifdef NO_CYC_PREFIX
#define ADD_PREFIX(x) x
#else
#define ADD_PREFIX(x) Cyc_##x
#endif

#ifndef offsetof
// should be size_t, but int is fine.
#define offsetof(t,n) ((int)(&(((t *)0)->n)))
#endif

//// Tagged arrays
struct _tagged_arr { 
  unsigned char *curr; 
  unsigned char *base; 
  unsigned char *last_plus_one; 
};

//// Discriminated Unions
struct _xtunion_struct { char *tag; };

// Need one of these per thread (we don't have threads)
// The runtime maintains a stack that contains either _handler_cons
// structs or _RegionHandle structs.  The tag is 0 for a handler_cons
// and 1 for a region handle.  
struct _RuntimeStack {
  int tag; // 0 for an exception handler, 1 for a region handle
  struct _RuntimeStack *next;
};

//// Regions
struct _RegionPage {
#ifdef CYC_REGION_PROFILE
  unsigned total_bytes;
  unsigned free_bytes;
#endif
  struct _RegionPage *next;
  char data[0];
};

struct _RegionHandle {
  struct _RuntimeStack s;
  struct _RegionPage *curr;
  char               *offset;
  char               *last_plus_one;
#ifdef CYC_REGION_PROFILE
  const char         *name;
#endif
};

extern struct _RegionHandle _new_region(const char *);
extern void* _region_malloc(struct _RegionHandle *, unsigned);
extern void* _region_calloc(struct _RegionHandle *, unsigned t, unsigned n);
extern void  _free_region(struct _RegionHandle *);

//// Exceptions 
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

//// Built-in Exceptions
extern struct _xtunion_struct ADD_PREFIX(Null_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Null_Exception);
extern struct _xtunion_struct ADD_PREFIX(Array_bounds_struct);
extern struct _xtunion_struct * ADD_PREFIX(Array_bounds);
extern struct _xtunion_struct ADD_PREFIX(Match_Exception_struct);
extern struct _xtunion_struct * ADD_PREFIX(Match_Exception);
extern struct _xtunion_struct ADD_PREFIX(Bad_alloc_struct);
extern struct _xtunion_struct * ADD_PREFIX(Bad_alloc);

//// Built-in Run-time Checks and company
#ifdef NO_CYC_NULL_CHECKS
#define _check_null(ptr) (ptr)
#else
#define _check_null(ptr) \
  ({ void*_check_null_temp = (void*)(ptr); \
     if (!_check_null_temp) _throw_null(); \
     _check_null_temp; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  ((char *)ptr) + (elt_sz)*(index); })
#else
#define _check_known_subscript_null(ptr,bound,elt_sz,index) ({ \
  void*_cks_ptr = (void*)(ptr); \
  unsigned _cks_bound = (bound); \
  unsigned _cks_elt_sz = (elt_sz); \
  unsigned _cks_index = (index); \
  if (!_cks_ptr) _throw_null(); \
  if (!_cks_index >= _cks_bound) _throw_arraybounds(); \
  ((char *)cks_ptr) + cks_elt_sz*cks_index; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_known_subscript_notnull(bound,index) (index)
#else
#define _check_known_subscript_notnull(bound,index) ({ \
  unsigned _cksnn_bound = (bound); \
  unsigned _cksnn_index = (index); \
  if (_cksnn_index >= _cksnn_bound) _throw_arraybounds(); \
  _cksnn_index; })
#endif

#ifdef NO_CYC_BOUNDS_CHECKS
#define _check_unknown_subscript(arr,elt_sz,index) ({ \
  struct _tagged_arr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  _cus_ans; })
#else
#define _check_unknown_subscript(arr,elt_sz,index) ({ \
  struct _tagged_arr _cus_arr = (arr); \
  unsigned _cus_elt_sz = (elt_sz); \
  unsigned _cus_index = (index); \
  unsigned char *_cus_ans = _cus_arr.curr + _cus_elt_sz * _cus_index; \
  if (!_cus_arr.base) _throw_null(); \
  if (_cus_ans < _cus_arr.base || _cus_ans >= _cus_arr.last_plus_one) \
    _throw_arraybounds(); \
  _cus_ans; })
#endif

#define _tag_arr(tcurr,elt_sz,num_elts) ({ \
  struct _tagged_arr _tag_arr_ans; \
  _tag_arr_ans.base = _tag_arr_ans.curr = (void*)(tcurr); \
  _tag_arr_ans.last_plus_one = _tag_arr_ans.base + (elt_sz) * (num_elts); \
  _tag_arr_ans; })

#define _init_tag_arr(arr_ptr,arr,elt_sz,num_elts) ({ \
  struct _tagged_arr *_itarr_ptr = (arr_ptr); \
  void* _itarr = (arr); \
  _itarr_ptr->base = _itarr_ptr->curr = _itarr; \
  _itarr_ptr->last_plus_one = ((char *)_itarr) + (elt_sz) * (num_elts); \
  _itarr_ptr; })

#ifdef NO_CYC_BOUNDS_CHECKS
#define _untag_arr(arr,elt_sz,num_elts) ((arr).curr)
#else
#define _untag_arr(arr,elt_sz,num_elts) ({ \
  struct _tagged_arr _arr = (arr); \
  unsigned char *_curr = _arr.curr; \
  if (_curr < _arr.base || _curr + (elt_sz) * (num_elts) > _arr.last_plus_one)\
    _throw_arraybounds(); \
  _curr; })
#endif

#define _get_arr_size(arr,elt_sz) \
  ({struct _tagged_arr _get_arr_size_temp = (arr); \
    unsigned char *_get_arr_size_curr=_get_arr_size_temp.curr; \
    unsigned char *_get_arr_size_last=_get_arr_size_temp.last_plus_one; \
    (_get_arr_size_curr < _get_arr_size_temp.base || \
     _get_arr_size_curr >= _get_arr_size_last) ? 0 : \
    ((_get_arr_size_last - _get_arr_size_curr) / (elt_sz));})

#define _tagged_arr_plus(arr,elt_sz,change) ({ \
  struct _tagged_arr _ans = (arr); \
  _ans.curr += ((int)(elt_sz))*(change); \
  _ans; })

#define _tagged_arr_inplace_plus(arr_ptr,elt_sz,change) ({ \
  struct _tagged_arr * _arr_ptr = (arr_ptr); \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  *_arr_ptr; })

#define _tagged_arr_inplace_plus_post(arr_ptr,elt_sz,change) ({ \
  struct _tagged_arr * _arr_ptr = (arr_ptr); \
  struct _tagged_arr _ans = *_arr_ptr; \
  _arr_ptr->curr += ((int)(elt_sz))*(change); \
  _ans; })

//// Allocation
extern void* GC_malloc(int);
extern void* GC_malloc_atomic(int);
extern void* GC_calloc(unsigned,unsigned);
extern void* GC_calloc_atomic(unsigned,unsigned);

static inline void* _cycalloc(int n) {
  void * ans = (void *)GC_malloc(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static inline void* _cycalloc_atomic(int n) {
  void * ans = (void *)GC_malloc_atomic(n);
  if(!ans)
    _throw_badalloc();
  return ans;
}
static inline void* _cyccalloc(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
static inline void* _cyccalloc_atomic(unsigned n, unsigned s) {
  void* ans = (void*)GC_calloc_atomic(n,s);
  if (!ans)
    _throw_badalloc();
  return ans;
}
#define MAX_MALLOC_SIZE (1 << 28)
static inline unsigned int _check_times(unsigned x, unsigned y) {
  unsigned long long whole_ans = 
    ((unsigned long long)x)*((unsigned long long)y);
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
 struct Cyc_Core_Opt{void*v;};void*Cyc_Core_identity(void*);extern unsigned char Cyc_Core_Invalid_argument[
21];struct Cyc_Core_Invalid_argument_struct{unsigned char*tag;struct _tagged_arr f1;
};extern unsigned char Cyc_Core_Failure[12];struct Cyc_Core_Failure_struct{
unsigned char*tag;struct _tagged_arr f1;};extern unsigned char Cyc_Core_Impossible[
15];struct Cyc_Core_Impossible_struct{unsigned char*tag;struct _tagged_arr f1;};
extern unsigned char Cyc_Core_Not_found[14];extern unsigned char Cyc_Core_Unreachable[
16];struct Cyc_Core_Unreachable_struct{unsigned char*tag;struct _tagged_arr f1;};
int isalnum(int);struct Cyc_List_List{void*hd;struct Cyc_List_List*tl;};struct Cyc_List_List*
Cyc_List_list(struct _tagged_arr);int Cyc_List_length(struct Cyc_List_List*x);
struct Cyc_List_List*Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);struct
Cyc_List_List*Cyc_List_map_c(void*(*f)(void*,void*),void*env,struct Cyc_List_List*
x);extern unsigned char Cyc_List_List_mismatch[18];void*Cyc_List_fold_left(void*(*
f)(void*,void*),void*accum,struct Cyc_List_List*x);void*Cyc_List_fold_right_c(
void*(*f)(void*,void*,void*),void*,struct Cyc_List_List*x,void*accum);struct Cyc_List_List*
Cyc_List_imp_append(struct Cyc_List_List*x,struct Cyc_List_List*y);struct Cyc_List_List*
Cyc_List_flatten(struct Cyc_List_List*x);extern unsigned char Cyc_List_Nth[8];int
Cyc_List_forall(int(*pred)(void*),struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_zip(
struct Cyc_List_List*x,struct Cyc_List_List*y);struct Cyc_List_List*Cyc_List_zip3(
struct Cyc_List_List*x,struct Cyc_List_List*y,struct Cyc_List_List*z);struct _tuple0{
struct Cyc_List_List*f1;struct Cyc_List_List*f2;};struct _tuple0 Cyc_List_split(
struct Cyc_List_List*x);struct Cyc_List_List*Cyc_List_tabulate(int n,void*(*f)(int));
struct Cyc_List_List*Cyc_List_filter(int(*f)(void*),struct Cyc_List_List*x);struct
Cyc_Iter_Iter{void*env;int(*next)(void*env,void*dest);};int Cyc_Iter_next(struct
Cyc_Iter_Iter,void*);struct Cyc_Dict_Dict;extern unsigned char Cyc_Dict_Present[12];
extern unsigned char Cyc_Dict_Absent[11];struct Cyc_Dict_Dict*Cyc_Dict_empty(int(*
cmp)(void*,void*));struct Cyc_Dict_Dict*Cyc_Dict_insert(struct Cyc_Dict_Dict*d,
void*k,void*v);void*Cyc_Dict_lookup(struct Cyc_Dict_Dict*d,void*k);struct Cyc_Core_Opt*
Cyc_Dict_lookup_opt(struct Cyc_Dict_Dict*d,void*k);void Cyc_Dict_iter(void(*f)(
void*,void*),struct Cyc_Dict_Dict*d);struct _tuple1{void*f1;void*f2;};struct
_tuple1*Cyc_Dict_rchoose(struct _RegionHandle*r,struct Cyc_Dict_Dict*d);struct
_tuple1*Cyc_Dict_rchoose(struct _RegionHandle*,struct Cyc_Dict_Dict*d);struct Cyc_Dict_Dict*
Cyc_Dict_delete(struct Cyc_Dict_Dict*,void*);unsigned int Cyc_Std_strlen(struct
_tagged_arr s);int Cyc_Std_strcmp(struct _tagged_arr s1,struct _tagged_arr s2);struct
_tagged_arr Cyc_Std_strconcat(struct _tagged_arr,struct _tagged_arr);struct
_tagged_arr Cyc_Std_strdup(struct _tagged_arr src);struct Cyc_Hashtable_Table;struct
Cyc_Hashtable_Table*Cyc_Hashtable_create(int sz,int(*cmp)(void*,void*),int(*hash)(
void*));void Cyc_Hashtable_insert(struct Cyc_Hashtable_Table*t,void*key,void*val);
void*Cyc_Hashtable_lookup(struct Cyc_Hashtable_Table*t,void*key);struct Cyc_Lineno_Pos{
struct _tagged_arr logical_file;struct _tagged_arr line;int line_no;int col;};extern
unsigned char Cyc_Position_Exit[9];struct Cyc_Position_Segment;struct Cyc_Position_Segment*
Cyc_Position_segment_of_abs(int,int);struct Cyc_Position_Error{struct _tagged_arr
source;struct Cyc_Position_Segment*seg;void*kind;struct _tagged_arr desc;};extern
unsigned char Cyc_Position_Nocontext[14];struct Cyc_Typerep_Int_struct{int tag;int
f1;unsigned int f2;};struct Cyc_Typerep_ThinPtr_struct{int tag;unsigned int f1;void*
f2;};struct Cyc_Typerep_FatPtr_struct{int tag;void*f1;};struct _tuple2{unsigned int
f1;struct _tagged_arr f2;void*f3;};struct Cyc_Typerep_Struct_struct{int tag;struct
_tagged_arr*f1;unsigned int f2;struct _tagged_arr f3;};struct _tuple3{unsigned int f1;
void*f2;};struct Cyc_Typerep_Tuple_struct{int tag;unsigned int f1;struct _tagged_arr
f2;};struct _tuple4{unsigned int f1;struct _tagged_arr f2;};struct Cyc_Typerep_TUnion_struct{
int tag;struct _tagged_arr f1;struct _tagged_arr f2;struct _tagged_arr f3;};struct Cyc_Typerep_TUnionField_struct{
int tag;struct _tagged_arr f1;struct _tagged_arr f2;unsigned int f3;struct _tagged_arr
f4;};struct _tuple5{struct _tagged_arr f1;void*f2;};struct Cyc_Typerep_XTUnion_struct{
int tag;struct _tagged_arr f1;struct _tagged_arr f2;};struct Cyc_Typerep_Union_struct{
int tag;struct _tagged_arr*f1;int f2;struct _tagged_arr f3;};struct Cyc_Typerep_Enum_struct{
int tag;struct _tagged_arr*f1;int f2;struct _tagged_arr f3;};unsigned int Cyc_Typerep_size_type(
void*rep);struct Cyc_Std_tm{int tm_sec;int tm_min;int tm_hour;int tm_mday;int tm_mon;
int tm_year;int tm_wday;int tm_yday;int tm_isdst;};struct Cyc_Absyn_Rel_n_struct{int
tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Abs_n_struct{int tag;struct Cyc_List_List*
f1;};struct _tuple6{void*f1;struct _tagged_arr*f2;};struct Cyc_Absyn_Conref;struct
Cyc_Absyn_Tqual{int q_const: 1;int q_volatile: 1;int q_restrict: 1;};struct Cyc_Absyn_Conref{
void*v;};struct Cyc_Absyn_Eq_constr_struct{int tag;void*f1;};struct Cyc_Absyn_Forward_constr_struct{
int tag;struct Cyc_Absyn_Conref*f1;};struct Cyc_Absyn_Eq_kb_struct{int tag;void*f1;}
;struct Cyc_Absyn_Unknown_kb_struct{int tag;struct Cyc_Core_Opt*f1;};struct Cyc_Absyn_Less_kb_struct{
int tag;struct Cyc_Core_Opt*f1;void*f2;};struct Cyc_Absyn_Tvar{struct _tagged_arr*
name;int*identity;void*kind;};struct Cyc_Absyn_Upper_b_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_PtrInfo{void*elt_typ;void*rgn_typ;struct Cyc_Absyn_Conref*
nullable;struct Cyc_Absyn_Tqual tq;struct Cyc_Absyn_Conref*bounds;};struct Cyc_Absyn_VarargInfo{
struct Cyc_Core_Opt*name;struct Cyc_Absyn_Tqual tq;void*type;int inject;};struct Cyc_Absyn_FnInfo{
struct Cyc_List_List*tvars;struct Cyc_Core_Opt*effect;void*ret_typ;struct Cyc_List_List*
args;int c_varargs;struct Cyc_Absyn_VarargInfo*cyc_varargs;struct Cyc_List_List*
rgn_po;struct Cyc_List_List*attributes;};struct Cyc_Absyn_UnknownTunionInfo{struct
_tuple6*name;int is_xtunion;};struct Cyc_Absyn_UnknownTunion_struct{int tag;struct
Cyc_Absyn_UnknownTunionInfo f1;};struct Cyc_Absyn_KnownTunion_struct{int tag;struct
Cyc_Absyn_Tuniondecl**f1;};struct Cyc_Absyn_TunionInfo{void*tunion_info;struct Cyc_List_List*
targs;void*rgn;};struct Cyc_Absyn_UnknownTunionFieldInfo{struct _tuple6*
tunion_name;struct _tuple6*field_name;int is_xtunion;};struct Cyc_Absyn_UnknownTunionfield_struct{
int tag;struct Cyc_Absyn_UnknownTunionFieldInfo f1;};struct Cyc_Absyn_KnownTunionfield_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Absyn_TunionFieldInfo{
void*field_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownAggr_struct{
int tag;void*f1;struct _tuple6*f2;};struct Cyc_Absyn_KnownAggr_struct{int tag;struct
Cyc_Absyn_Aggrdecl**f1;};struct Cyc_Absyn_AggrInfo{void*aggr_info;struct Cyc_List_List*
targs;};struct Cyc_Absyn_Evar_struct{int tag;struct Cyc_Core_Opt*f1;struct Cyc_Core_Opt*
f2;int f3;struct Cyc_Core_Opt*f4;};struct Cyc_Absyn_VarType_struct{int tag;struct Cyc_Absyn_Tvar*
f1;};struct Cyc_Absyn_TunionType_struct{int tag;struct Cyc_Absyn_TunionInfo f1;};
struct Cyc_Absyn_TunionFieldType_struct{int tag;struct Cyc_Absyn_TunionFieldInfo f1;
};struct Cyc_Absyn_PointerType_struct{int tag;struct Cyc_Absyn_PtrInfo f1;};struct
Cyc_Absyn_IntType_struct{int tag;void*f1;void*f2;};struct Cyc_Absyn_DoubleType_struct{
int tag;int f1;};struct Cyc_Absyn_ArrayType_struct{int tag;void*f1;struct Cyc_Absyn_Tqual
f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_FnType_struct{int tag;struct Cyc_Absyn_FnInfo
f1;};struct Cyc_Absyn_TupleType_struct{int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_AggrType_struct{
int tag;struct Cyc_Absyn_AggrInfo f1;};struct Cyc_Absyn_AnonAggrType_struct{int tag;
void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_EnumType_struct{int tag;struct
_tuple6*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_struct{int
tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_SizeofType_struct{int tag;void*f1;};
struct Cyc_Absyn_RgnHandleType_struct{int tag;void*f1;};struct Cyc_Absyn_TypedefType_struct{
int tag;struct _tuple6*f1;struct Cyc_List_List*f2;struct Cyc_Core_Opt*f3;};struct Cyc_Absyn_AccessEff_struct{
int tag;void*f1;};struct Cyc_Absyn_JoinEff_struct{int tag;struct Cyc_List_List*f1;};
struct Cyc_Absyn_RgnsEff_struct{int tag;void*f1;};struct Cyc_Absyn_NoTypes_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Position_Segment*f2;};struct Cyc_Absyn_WithTypes_struct{
int tag;struct Cyc_List_List*f1;int f2;struct Cyc_Absyn_VarargInfo*f3;struct Cyc_Core_Opt*
f4;struct Cyc_List_List*f5;};struct Cyc_Absyn_NonNullable_ps_struct{int tag;struct
Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Nullable_ps_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_Regparm_att_struct{int tag;int f1;};struct Cyc_Absyn_Aligned_att_struct{
int tag;int f1;};struct Cyc_Absyn_Section_att_struct{int tag;struct _tagged_arr f1;};
struct Cyc_Absyn_Format_att_struct{int tag;void*f1;int f2;int f3;};struct Cyc_Absyn_ConstArray_mod_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Pointer_mod_struct{int tag;void*
f1;void*f2;struct Cyc_Absyn_Tqual f3;};struct Cyc_Absyn_Function_mod_struct{int tag;
void*f1;};struct Cyc_Absyn_TypeParams_mod_struct{int tag;struct Cyc_List_List*f1;
struct Cyc_Position_Segment*f2;int f3;};struct Cyc_Absyn_Attributes_mod_struct{int
tag;struct Cyc_Position_Segment*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Char_c_struct{
int tag;void*f1;unsigned char f2;};struct Cyc_Absyn_Short_c_struct{int tag;void*f1;
short f2;};struct Cyc_Absyn_Int_c_struct{int tag;void*f1;int f2;};struct Cyc_Absyn_LongLong_c_struct{
int tag;void*f1;long long f2;};struct Cyc_Absyn_Float_c_struct{int tag;struct
_tagged_arr f1;};struct Cyc_Absyn_String_c_struct{int tag;struct _tagged_arr f1;};
struct Cyc_Absyn_VarargCallInfo{int num_varargs;struct Cyc_List_List*injectors;
struct Cyc_Absyn_VarargInfo*vai;};struct Cyc_Absyn_StructField_struct{int tag;
struct _tagged_arr*f1;};struct Cyc_Absyn_TupleIndex_struct{int tag;unsigned int f1;}
;struct Cyc_Absyn_MallocInfo{int is_calloc;struct Cyc_Absyn_Exp*rgn;void**elt_type;
struct Cyc_Absyn_Exp*num_elts;int fat_result;};struct Cyc_Absyn_Const_e_struct{int
tag;void*f1;};struct Cyc_Absyn_Var_e_struct{int tag;struct _tuple6*f1;void*f2;};
struct Cyc_Absyn_UnknownId_e_struct{int tag;struct _tuple6*f1;};struct Cyc_Absyn_Primop_e_struct{
int tag;void*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_AssignOp_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_Core_Opt*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Increment_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;void*f2;};struct Cyc_Absyn_Conditional_e_struct{int
tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};
struct Cyc_Absyn_SeqExp_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*
f2;};struct Cyc_Absyn_UnknownCall_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_FnCall_e_struct{int tag;struct Cyc_Absyn_Exp*f1;
struct Cyc_List_List*f2;struct Cyc_Absyn_VarargCallInfo*f3;};struct Cyc_Absyn_Throw_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_NoInstantiate_e_struct{int tag;
struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Instantiate_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Cast_e_struct{int tag;void*f1;struct
Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Address_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_New_e_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*
f2;};struct Cyc_Absyn_Sizeoftyp_e_struct{int tag;void*f1;};struct Cyc_Absyn_Sizeofexp_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Offsetof_e_struct{int tag;void*f1;
void*f2;};struct Cyc_Absyn_Gentyp_e_struct{int tag;struct Cyc_List_List*f1;void*f2;
};struct Cyc_Absyn_Deref_e_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_AggrMember_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct _tagged_arr*f2;};struct Cyc_Absyn_AggrArrow_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct _tagged_arr*f2;};struct Cyc_Absyn_Subscript_e_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Tuple_e_struct{
int tag;struct Cyc_List_List*f1;};struct _tuple7{struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Tqual
f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_struct{int tag;struct _tuple7*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_struct{int tag;struct Cyc_List_List*f1;
};struct Cyc_Absyn_Comprehension_e_struct{int tag;struct Cyc_Absyn_Vardecl*f1;
struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Struct_e_struct{
int tag;struct _tuple6*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*
f4;};struct Cyc_Absyn_AnonStruct_e_struct{int tag;void*f1;struct Cyc_List_List*f2;}
;struct Cyc_Absyn_Tunion_e_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Tuniondecl*
f2;struct Cyc_Absyn_Tunionfield*f3;};struct Cyc_Absyn_Enum_e_struct{int tag;struct
_tuple6*f1;struct Cyc_Absyn_Enumdecl*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_AnonEnum_e_struct{
int tag;struct _tuple6*f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_Malloc_e_struct{
int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_UnresolvedMem_e_struct{int
tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Codegen_e_struct{int tag;struct
Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Fill_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_Exp{struct Cyc_Core_Opt*topt;void*r;struct Cyc_Position_Segment*
loc;void*annot;};struct _tuple8{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};
struct Cyc_Absyn_ForArrayInfo{struct Cyc_List_List*defns;struct _tuple8 condition;
struct _tuple8 delta;struct Cyc_Absyn_Stmt*body;};struct Cyc_Absyn_Exp_s_struct{int
tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_IfThenElse_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*
f2;struct Cyc_Absyn_Stmt*f3;};struct Cyc_Absyn_While_s_struct{int tag;struct _tuple8
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;};struct Cyc_Absyn_Continue_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct
Cyc_Absyn_Goto_s_struct{int tag;struct _tagged_arr*f1;struct Cyc_Absyn_Stmt*f2;};
struct Cyc_Absyn_For_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple8 f2;
struct _tuple8 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_SwitchC_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Fallthru_s_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_struct{
int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Cut_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Splice_s_struct{int tag;struct
Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Label_s_struct{int tag;struct _tagged_arr*f1;
struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct _tuple8 f2;};struct Cyc_Absyn_TryCatch_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Region_s_struct{int tag;struct Cyc_Absyn_Tvar*
f1;struct Cyc_Absyn_Vardecl*f2;struct Cyc_Absyn_Stmt*f3;};struct Cyc_Absyn_ForArray_s_struct{
int tag;struct Cyc_Absyn_ForArrayInfo f1;};struct Cyc_Absyn_Stmt{void*r;struct Cyc_Position_Segment*
loc;struct Cyc_List_List*non_local_preds;int try_depth;void*annot;};struct Cyc_Absyn_Var_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Int_p_struct{int tag;void*f1;
int f2;};struct Cyc_Absyn_Char_p_struct{int tag;unsigned char f1;};struct Cyc_Absyn_Float_p_struct{
int tag;struct _tagged_arr f1;};struct Cyc_Absyn_Tuple_p_struct{int tag;struct Cyc_List_List*
f1;};struct Cyc_Absyn_Pointer_p_struct{int tag;struct Cyc_Absyn_Pat*f1;};struct Cyc_Absyn_Reference_p_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Aggr_p_struct{int tag;struct
Cyc_Absyn_AggrInfo f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;};struct Cyc_Absyn_Tunion_p_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;struct Cyc_List_List*
f3;};struct Cyc_Absyn_Enum_p_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Absyn_AnonEnum_p_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Absyn_UnknownId_p_struct{int tag;struct _tuple6*f1;};struct Cyc_Absyn_UnknownCall_p_struct{
int tag;struct _tuple6*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Pat{void*r;
struct Cyc_Core_Opt*topt;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Switch_clause{
struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*
where_clause;struct Cyc_Absyn_Stmt*body;struct Cyc_Position_Segment*loc;};struct
Cyc_Absyn_SwitchC_clause{struct Cyc_Absyn_Exp*cnst_exp;struct Cyc_Absyn_Stmt*body;
struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Global_b_struct{int tag;struct
Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Param_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Local_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{void*sc;struct
_tuple6*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;
struct Cyc_Core_Opt*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{
void*sc;int is_inline;struct _tuple6*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
effect;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*
cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;struct Cyc_Core_Opt*
cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_List_List*attributes;};
struct Cyc_Absyn_Aggrfield{struct _tagged_arr*name;struct Cyc_Absyn_Tqual tq;void*
type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Aggrdecl{
void*kind;void*sc;struct _tuple6*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
exist_vars;struct Cyc_Core_Opt*fields;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Tunionfield{
struct _tuple6*name;struct Cyc_List_List*typs;struct Cyc_Position_Segment*loc;void*
sc;};struct Cyc_Absyn_Tuniondecl{void*sc;struct _tuple6*name;struct Cyc_List_List*
tvs;struct Cyc_Core_Opt*fields;int is_xtunion;};struct Cyc_Absyn_Enumfield{struct
_tuple6*name;struct Cyc_Absyn_Exp*tag;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Enumdecl{
void*sc;struct _tuple6*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{
struct _tuple6*name;struct Cyc_List_List*tvs;void*defn;};struct Cyc_Absyn_Var_d_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Let_d_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*
f2;struct Cyc_Core_Opt*f3;struct Cyc_Absyn_Exp*f4;int f5;};struct Cyc_Absyn_Letv_d_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Aggr_d_struct{int tag;struct Cyc_Absyn_Aggrdecl*
f1;};struct Cyc_Absyn_Tunion_d_struct{int tag;struct Cyc_Absyn_Tuniondecl*f1;};
struct Cyc_Absyn_Enum_d_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_struct{
int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_struct{int
tag;struct _tagged_arr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_struct{
int tag;struct _tuple6*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Decl{void*r;struct Cyc_Position_Segment*
loc;};struct Cyc_Absyn_ArrayElement_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct
Cyc_Absyn_FieldName_struct{int tag;struct _tagged_arr*f1;};extern unsigned char Cyc_Absyn_EmptyAnnot[
15];int Cyc_Absyn_qvar_cmp(struct _tuple6*,struct _tuple6*);void*Cyc_Absyn_conref_val(
struct Cyc_Absyn_Conref*x);extern void*Cyc_Absyn_uchar_t;extern void*Cyc_Absyn_uint_t;
void*Cyc_Absyn_const_string_typ(void*rgn);void*Cyc_Absyn_star_typ(void*t,void*
rgn,struct Cyc_Absyn_Tqual tq);void*Cyc_Absyn_at_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual
tq);void*Cyc_Absyn_tagged_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual tq);struct Cyc_Absyn_Exp*
Cyc_Absyn_const_exp(void*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_null_exp(
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_unknownid_exp(struct
_tuple6*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_unknowncall_exp(
struct Cyc_Absyn_Exp*,struct Cyc_List_List*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_cast_exp(void*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_address_exp(struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_sizeoftyp_exp(void*t,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_offsetof_exp(void*,void*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_tuple_exp(struct Cyc_List_List*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_unresolvedmem_exp(struct Cyc_Core_Opt*,struct Cyc_List_List*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Decl*Cyc_Absyn_new_decl(void*r,
struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Vardecl*Cyc_Absyn_new_vardecl(
struct _tuple6*x,void*t,struct Cyc_Absyn_Exp*init);struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_get_known_aggrdecl(
void*info);struct Cyc_Cstdio___abstractFILE;struct Cyc_Std___cycFILE;extern
unsigned char Cyc_Std_FileCloseError[19];extern unsigned char Cyc_Std_FileOpenError[
18];struct Cyc_Std_FileOpenError_struct{unsigned char*tag;struct _tagged_arr f1;};
struct Cyc_Std_String_pa_struct{int tag;struct _tagged_arr f1;};struct Cyc_Std_Int_pa_struct{
int tag;unsigned int f1;};struct Cyc_Std_Double_pa_struct{int tag;double f1;};struct
Cyc_Std_ShortPtr_pa_struct{int tag;short*f1;};struct Cyc_Std_IntPtr_pa_struct{int
tag;unsigned int*f1;};int Cyc_Std_printf(struct _tagged_arr fmt,struct _tagged_arr);
struct _tagged_arr Cyc_Std_aprintf(struct _tagged_arr fmt,struct _tagged_arr);struct
Cyc_Std_ShortPtr_sa_struct{int tag;short*f1;};struct Cyc_Std_UShortPtr_sa_struct{
int tag;unsigned short*f1;};struct Cyc_Std_IntPtr_sa_struct{int tag;int*f1;};struct
Cyc_Std_UIntPtr_sa_struct{int tag;unsigned int*f1;};struct Cyc_Std_StringPtr_sa_struct{
int tag;struct _tagged_arr f1;};struct Cyc_Std_DoublePtr_sa_struct{int tag;double*f1;
};struct Cyc_Std_FloatPtr_sa_struct{int tag;float*f1;};struct Cyc_PP_Ppstate;struct
Cyc_PP_Out;struct Cyc_PP_Doc;struct Cyc_Absynpp_Params{int expand_typedefs: 1;int
qvar_to_Cids: 1;int add_cyc_prefix: 1;int to_VC: 1;int decls_first: 1;int
rewrite_temp_tvars: 1;int print_all_tvars: 1;int print_all_kinds: 1;int
print_using_stmts: 1;int print_externC_stmts: 1;int print_full_evars: 1;int
generate_line_directives: 1;int use_curr_namespace: 1;struct Cyc_List_List*
curr_namespace;};struct _tagged_arr Cyc_Absynpp_typ2string(void*);struct
_tagged_arr Cyc_Absynpp_typ2cstring(void*);unsigned int Cyc_Evexp_eval_const_uint_exp(
struct Cyc_Absyn_Exp*e);struct Cyc_Set_Set;extern unsigned char Cyc_Set_Absent[11];
struct Cyc_Tcenv_VarRes_struct{int tag;void*f1;};struct Cyc_Tcenv_AggrRes_struct{
int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct Cyc_Tcenv_TunionRes_struct{int tag;
struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Tcenv_EnumRes_struct{
int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_AnonEnumRes_struct{
int tag;void*f1;struct Cyc_Absyn_Enumfield*f2;};struct Cyc_Tcenv_Genv{struct Cyc_Set_Set*
namespaces;struct Cyc_Dict_Dict*aggrdecls;struct Cyc_Dict_Dict*tuniondecls;struct
Cyc_Dict_Dict*enumdecls;struct Cyc_Dict_Dict*typedefs;struct Cyc_Dict_Dict*
ordinaries;struct Cyc_List_List*availables;};struct Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Stmt_j_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Tcenv_Outermost_struct{int tag;void*f1;
};struct Cyc_Tcenv_Frame_struct{int tag;void*f1;void*f2;};struct Cyc_Tcenv_Hidden_struct{
int tag;void*f1;void*f2;};struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict*
ae;struct Cyc_Core_Opt*le;};void*Cyc_Tcutil_impos(struct _tagged_arr fmt,struct
_tagged_arr ap);void Cyc_Tcutil_terr(struct Cyc_Position_Segment*,struct _tagged_arr
fmt,struct _tagged_arr ap);void*Cyc_Tcutil_compress(void*t);int Cyc_Tcutil_typecmp(
void*,void*);void*Cyc_Tcutil_substitute(struct Cyc_List_List*,void*);void Cyc_Tcutil_check_valid_toplevel_type(
struct Cyc_Position_Segment*,struct Cyc_Tcenv_Tenv*,void*);struct Cyc_Tcgenrep_RepInfo{
struct Cyc_List_List*decls;struct Cyc_Absyn_Exp*exp;struct Cyc_List_List*
dependencies;struct Cyc_Core_Opt*fwd_decl;int emitted;int is_extern;};struct Cyc_Dict_Dict*
Cyc_Tcgenrep_empty_typerep_dict(){return((struct Cyc_Dict_Dict*(*)(int(*cmp)(void*,
void*)))Cyc_Dict_empty)(Cyc_Tcutil_typecmp);}void Cyc_Tcgenrep_print_dict_entry(
void*type,struct Cyc_Tcgenrep_RepInfo*info){({struct Cyc_Std_Int_pa_struct _tmp3;
_tmp3.tag=1;_tmp3.f1=(int)((unsigned int)info->emitted);{struct Cyc_Std_Int_pa_struct
_tmp2;_tmp2.tag=1;_tmp2.f1=(unsigned int)info;{struct Cyc_Std_String_pa_struct
_tmp1;_tmp1.tag=0;_tmp1.f1=(struct _tagged_arr)Cyc_Absynpp_typ2string(type);{void*
_tmp0[3]={& _tmp1,& _tmp2,& _tmp3};Cyc_Std_printf(_tag_arr("(%s,%x:%d,",sizeof(
unsigned char),11),_tag_arr(_tmp0,sizeof(void*),3));}}}});{struct Cyc_List_List*p=
info->dependencies;for(0;p != 0;p=p->tl){({struct Cyc_Std_Int_pa_struct _tmp5;_tmp5.tag=
1;_tmp5.f1=(unsigned int)((struct Cyc_Tcgenrep_RepInfo*)p->hd);{void*_tmp4[1]={&
_tmp5};Cyc_Std_printf(_tag_arr("%x",sizeof(unsigned char),3),_tag_arr(_tmp4,
sizeof(void*),1));}});if(p->tl != 0)({void*_tmp6[0]={};Cyc_Std_printf(_tag_arr(",",
sizeof(unsigned char),2),_tag_arr(_tmp6,sizeof(void*),0));});}}({void*_tmp7[0]={};
Cyc_Std_printf(_tag_arr(")\n",sizeof(unsigned char),3),_tag_arr(_tmp7,sizeof(
void*),0));});}void Cyc_Tcgenrep_print_typerep_dict(struct Cyc_Dict_Dict*dict){((
void(*)(void(*f)(void*,struct Cyc_Tcgenrep_RepInfo*),struct Cyc_Dict_Dict*d))Cyc_Dict_iter)(
Cyc_Tcgenrep_print_dict_entry,dict);}static int Cyc_Tcgenrep_rephash(struct Cyc_Tcgenrep_RepInfo*
ri){return(int)ri;}static int Cyc_Tcgenrep_repcmp(struct Cyc_Tcgenrep_RepInfo*r1,
struct Cyc_Tcgenrep_RepInfo*r2){unsigned int r1p;unsigned int r2p;r1p=(unsigned int)
r1;r2p=(unsigned int)r2;if(r1 < r2)return - 1;else{if(r1 == r2)return 0;else{return 1;}}}
static struct Cyc_Absyn_Tunionfield*Cyc_Tcgenrep_getField(struct Cyc_Absyn_Tuniondecl*
td,struct _tuple6*fieldname){if(td->fields == 0)({void*_tmp8[0]={};((int(*)(struct
_tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("Could not find field in tuniondecl",
sizeof(unsigned char),35),_tag_arr(_tmp8,sizeof(void*),0));});else{struct Cyc_List_List*
l=(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(td->fields))->v;for(0;
l != 0;l=l->tl){if(!Cyc_Absyn_qvar_cmp(((struct Cyc_Absyn_Tunionfield*)l->hd)->name,
fieldname))return(struct Cyc_Absyn_Tunionfield*)l->hd;}}({void*_tmp9[0]={};((int(*)(
struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("Could not find field in tuniondecl",
sizeof(unsigned char),35),_tag_arr(_tmp9,sizeof(void*),0));});}static
unsigned char _tmpA[9]="Tcgenrep";static struct _tagged_arr Cyc_Tcgenrep_tcgenrep_nm={
_tmpA,_tmpA,_tmpA + 9};static struct Cyc_List_List Cyc_Tcgenrep_l={(void*)& Cyc_Tcgenrep_tcgenrep_nm,
0};static struct Cyc_Absyn_Abs_n_struct Cyc_Tcgenrep_tcgenrep_nmspace={1,(struct Cyc_List_List*)&
Cyc_Tcgenrep_l};static void*Cyc_Tcgenrep_tcgenrep_nmspace2=(void*)& Cyc_Tcgenrep_tcgenrep_nmspace;
static unsigned char _tmpC[8]="Typerep";static struct _tagged_arr Cyc_Tcgenrep_typerep_nm={
_tmpC,_tmpC,_tmpC + 8};static struct Cyc_List_List Cyc_Tcgenrep_l2={(void*)& Cyc_Tcgenrep_typerep_nm,
0};static struct Cyc_Absyn_Abs_n_struct Cyc_Tcgenrep_typerep_nmspace={1,(struct Cyc_List_List*)&
Cyc_Tcgenrep_l2};static struct Cyc_List_List*Cyc_Tcgenrep_dfsvisit(struct Cyc_Tcgenrep_RepInfo*
ri,struct Cyc_Hashtable_Table*visited){struct _handler_cons _tmpE;_push_handler(&
_tmpE);{int _tmp10=0;if(setjmp(_tmpE.handler))_tmp10=1;if(!_tmp10){((int(*)(
struct Cyc_Hashtable_Table*t,struct Cyc_Tcgenrep_RepInfo*key))Cyc_Hashtable_lookup)(
visited,ri);{struct Cyc_List_List*_tmp11=0;_npop_handler(0);return _tmp11;};
_pop_handler();}else{void*_tmpF=(void*)_exn_thrown;void*_tmp13=_tmpF;_LL1: if(
_tmp13 != Cyc_Core_Not_found)goto _LL3;_LL2:((void(*)(struct Cyc_Hashtable_Table*t,
struct Cyc_Tcgenrep_RepInfo*key,int val))Cyc_Hashtable_insert)(visited,ri,1);{
struct Cyc_List_List*ds=0;{struct Cyc_List_List*l=ri->dependencies;for(0;l != 0;l=l->tl){
ds=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(
ds,Cyc_Tcgenrep_dfsvisit((struct Cyc_Tcgenrep_RepInfo*)l->hd,visited));}}if(ri->emitted
== 0){if(ri->fwd_decl != 0)ds=({struct Cyc_List_List*_tmp14=_cycalloc(sizeof(*
_tmp14));_tmp14->hd=({struct Cyc_Absyn_Decl*_tmp15[1];_tmp15[0]=(struct Cyc_Absyn_Decl*)((
struct Cyc_Core_Opt*)_check_null(ri->fwd_decl))->v;((struct Cyc_List_List*(*)(
struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp15,sizeof(struct Cyc_Absyn_Decl*),
1));});_tmp14->tl=ds;_tmp14;});{struct Cyc_List_List*_tmp16=({struct Cyc_List_List*
_tmp17=_cycalloc(sizeof(*_tmp17));_tmp17->hd=ri->decls;_tmp17->tl=0;_tmp17;});ri->emitted=
1;return((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_imp_append)(ds,_tmp16);}}else{return ds;}}_LL3:;_LL4:(void)_throw(_tmp13);
_LL0:;}}}static struct Cyc_List_List*Cyc_Tcgenrep_dfs(struct Cyc_Tcgenrep_RepInfo*
ri){struct Cyc_Hashtable_Table*tab=((struct Cyc_Hashtable_Table*(*)(int sz,int(*cmp)(
struct Cyc_Tcgenrep_RepInfo*,struct Cyc_Tcgenrep_RepInfo*),int(*hash)(struct Cyc_Tcgenrep_RepInfo*)))
Cyc_Hashtable_create)(53,Cyc_Tcgenrep_repcmp,Cyc_Tcgenrep_rephash);struct Cyc_List_List*
ds=Cyc_Tcgenrep_dfsvisit(ri,tab);return((struct Cyc_List_List*(*)(struct Cyc_List_List*
x))Cyc_List_flatten)(ds);}static unsigned char _tmp18[11]="Typestruct";static
struct _tagged_arr Cyc_Tcgenrep_typestruct_str={_tmp18,_tmp18,_tmp18 + 11};static
unsigned char _tmp19[4]="Var";static struct _tagged_arr Cyc_Tcgenrep_var_str={_tmp19,
_tmp19,_tmp19 + 4};static unsigned char _tmp1A[4]="Int";static struct _tagged_arr Cyc_Tcgenrep_int_str={
_tmp1A,_tmp1A,_tmp1A + 4};static unsigned char _tmp1B[6]="Float";static struct
_tagged_arr Cyc_Tcgenrep_float_str={_tmp1B,_tmp1B,_tmp1B + 6};static unsigned char
_tmp1C[7]="Double";static struct _tagged_arr Cyc_Tcgenrep_double_str={_tmp1C,_tmp1C,
_tmp1C + 7};static unsigned char _tmp1D[8]="ThinPtr";static struct _tagged_arr Cyc_Tcgenrep_thinptr_str={
_tmp1D,_tmp1D,_tmp1D + 8};static unsigned char _tmp1E[7]="FatPtr";static struct
_tagged_arr Cyc_Tcgenrep_fatptr_str={_tmp1E,_tmp1E,_tmp1E + 7};static unsigned char
_tmp1F[6]="Tuple";static struct _tagged_arr Cyc_Tcgenrep_tuple_str={_tmp1F,_tmp1F,
_tmp1F + 6};static unsigned char _tmp20[12]="TUnionField";static struct _tagged_arr
Cyc_Tcgenrep_tunionfield_str={_tmp20,_tmp20,_tmp20 + 12};static unsigned char
_tmp21[7]="Struct";static struct _tagged_arr Cyc_Tcgenrep_struct_str={_tmp21,_tmp21,
_tmp21 + 7};static unsigned char _tmp22[7]="TUnion";static struct _tagged_arr Cyc_Tcgenrep_tunion_str={
_tmp22,_tmp22,_tmp22 + 7};static unsigned char _tmp23[8]="XTUnion";static struct
_tagged_arr Cyc_Tcgenrep_xtunion_str={_tmp23,_tmp23,_tmp23 + 8};static
unsigned char _tmp24[6]="Union";static struct _tagged_arr Cyc_Tcgenrep_union_str={
_tmp24,_tmp24,_tmp24 + 6};static unsigned char _tmp25[5]="Enum";static struct
_tagged_arr Cyc_Tcgenrep_enum_str={_tmp25,_tmp25,_tmp25 + 5};static unsigned char
_tmp26[7]="name_t";static struct _tagged_arr Cyc_Tcgenrep_name_t_str={_tmp26,_tmp26,
_tmp26 + 7};static struct _tuple6*Cyc_Tcgenrep_typerep_name(struct _tagged_arr*name){
return({struct _tuple6*_tmp27=_cycalloc(sizeof(*_tmp27));_tmp27->f1=(void*)& Cyc_Tcgenrep_typerep_nmspace;
_tmp27->f2=name;_tmp27;});}static int Cyc_Tcgenrep_gen_id_counter=0;static struct
_tagged_arr*Cyc_Tcgenrep_new_gen_id(struct _tagged_arr name){return({struct
_tagged_arr*_tmp28=_cycalloc(sizeof(*_tmp28));_tmp28[0]=(struct _tagged_arr)({
struct Cyc_Std_Int_pa_struct _tmp2B;_tmp2B.tag=1;_tmp2B.f1=(int)((unsigned int)Cyc_Tcgenrep_gen_id_counter
++);{struct Cyc_Std_String_pa_struct _tmp2A;_tmp2A.tag=0;_tmp2A.f1=(struct
_tagged_arr)name;{void*_tmp29[2]={& _tmp2A,& _tmp2B};Cyc_Std_aprintf(_tag_arr("_gen%s_%d",
sizeof(unsigned char),10),_tag_arr(_tmp29,sizeof(void*),2));}}});_tmp28;});}
static void Cyc_Tcgenrep_print_params(struct Cyc_List_List*l){({void*_tmp2C[0]={};
Cyc_Std_printf(_tag_arr("<",sizeof(unsigned char),2),_tag_arr(_tmp2C,sizeof(void*),
0));});{struct Cyc_List_List*p=l;for(0;p != 0;p=p->tl){({struct Cyc_Std_Int_pa_struct
_tmp2F;_tmp2F.tag=1;_tmp2F.f1=(int)((unsigned int)((int)(p->tl != 0?',':' ')));{
struct Cyc_Std_String_pa_struct _tmp2E;_tmp2E.tag=0;_tmp2E.f1=(struct _tagged_arr)
Cyc_Absynpp_typ2string((void*)p->hd);{void*_tmp2D[2]={& _tmp2E,& _tmp2F};Cyc_Std_printf(
_tag_arr("%s%c",sizeof(unsigned char),5),_tag_arr(_tmp2D,sizeof(void*),2));}}});}}({
void*_tmp30[0]={};Cyc_Std_printf(_tag_arr(">\n",sizeof(unsigned char),3),
_tag_arr(_tmp30,sizeof(void*),0));});}static void Cyc_Tcgenrep_print_tvars(struct
Cyc_List_List*l){({void*_tmp31[0]={};Cyc_Std_printf(_tag_arr("<",sizeof(
unsigned char),2),_tag_arr(_tmp31,sizeof(void*),0));});{struct Cyc_List_List*p=l;
for(0;p != 0;p=p->tl){({struct Cyc_Std_Int_pa_struct _tmp34;_tmp34.tag=1;_tmp34.f1=(
int)((unsigned int)((int)(p->tl != 0?',':' ')));{struct Cyc_Std_String_pa_struct
_tmp33;_tmp33.tag=0;_tmp33.f1=(struct _tagged_arr)*((struct Cyc_Absyn_Tvar*)p->hd)->name;{
void*_tmp32[2]={& _tmp33,& _tmp34};Cyc_Std_printf(_tag_arr("%s%c",sizeof(
unsigned char),5),_tag_arr(_tmp32,sizeof(void*),2));}}});}}({void*_tmp35[0]={};
Cyc_Std_printf(_tag_arr(">\n",sizeof(unsigned char),3),_tag_arr(_tmp35,sizeof(
void*),0));});}static struct Cyc_Absyn_Rel_n_struct Cyc_Tcgenrep_top_nm_0={0,0};
static void*Cyc_Tcgenrep_top_nm=(void*)& Cyc_Tcgenrep_top_nm_0;static struct _tuple6*
Cyc_Tcgenrep_toplevel_name(struct _tagged_arr*name){return({struct _tuple6*_tmp37=
_cycalloc(sizeof(*_tmp37));_tmp37->f1=Cyc_Tcgenrep_top_nm;_tmp37->f2=(struct
_tagged_arr*)name;_tmp37;});}static struct _tuple6*Cyc_Tcgenrep_relative_name(
struct Cyc_List_List*pathl,struct _tagged_arr*name){struct Cyc_Absyn_Rel_n_struct*
path=({struct Cyc_Absyn_Rel_n_struct*_tmp39=_cycalloc(sizeof(*_tmp39));_tmp39[0]=({
struct Cyc_Absyn_Rel_n_struct _tmp3A;_tmp3A.tag=0;_tmp3A.f1=pathl;_tmp3A;});_tmp39;});
return({struct _tuple6*_tmp38=_cycalloc(sizeof(*_tmp38));_tmp38->f1=(void*)path;
_tmp38->f2=(struct _tagged_arr*)name;_tmp38;});}static struct Cyc_Absyn_Decl*Cyc_Tcgenrep_gen_decl(
struct _tagged_arr*name,void*type,struct Cyc_Absyn_Exp*init,struct Cyc_Position_Segment*
seg){struct _tuple6*qvar=Cyc_Tcgenrep_toplevel_name(name);struct Cyc_Absyn_Vardecl*
vd=Cyc_Absyn_new_vardecl(qvar,type,init);(void*)(vd->sc=(void*)((void*)0));{
struct Cyc_Absyn_Var_d_struct*r1=({struct Cyc_Absyn_Var_d_struct*_tmp3B=_cycalloc(
sizeof(*_tmp3B));_tmp3B[0]=({struct Cyc_Absyn_Var_d_struct _tmp3C;_tmp3C.tag=0;
_tmp3C.f1=vd;_tmp3C;});_tmp3B;});void*r2=(void*)r1;return Cyc_Absyn_new_decl(r2,
seg);}}static struct Cyc_Absyn_Decl*Cyc_Tcgenrep_gen_vardecl(struct _tuple6*name,
void*type,struct Cyc_Absyn_Exp*init,void*sc,struct Cyc_Position_Segment*seg){
struct _tuple6*topname=Cyc_Tcgenrep_toplevel_name((*name).f2);struct Cyc_Absyn_Vardecl*
vd=Cyc_Absyn_new_vardecl(topname,type,init);(void*)(vd->sc=(void*)sc);{struct Cyc_Absyn_Var_d_struct*
r1=({struct Cyc_Absyn_Var_d_struct*_tmp3D=_cycalloc(sizeof(*_tmp3D));_tmp3D[0]=({
struct Cyc_Absyn_Var_d_struct _tmp3E;_tmp3E.tag=0;_tmp3E.f1=vd;_tmp3E;});_tmp3D;});
void*r2=(void*)r1;return Cyc_Absyn_new_decl(r2,seg);}}static struct Cyc_Absyn_Exp*
Cyc_Tcgenrep_cnst_string(struct _tagged_arr s,struct Cyc_Position_Segment*seg){
return Cyc_Absyn_const_exp((void*)({struct Cyc_Absyn_String_c_struct*_tmp3F=
_cycalloc(sizeof(*_tmp3F));_tmp3F[0]=({struct Cyc_Absyn_String_c_struct _tmp40;
_tmp40.tag=5;_tmp40.f1=s;_tmp40;});_tmp3F;}),seg);}static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_cnst_string_cls(
struct Cyc_Position_Segment*seg,struct _tagged_arr*s){return Cyc_Absyn_const_exp((
void*)({struct Cyc_Absyn_String_c_struct*_tmp41=_cycalloc(sizeof(*_tmp41));_tmp41[
0]=({struct Cyc_Absyn_String_c_struct _tmp42;_tmp42.tag=5;_tmp42.f1=*s;_tmp42;});
_tmp41;}),seg);}static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_cnst_qvar_string_cls(
struct Cyc_Position_Segment*seg,struct _tuple6*s){return Cyc_Absyn_const_exp((void*)({
struct Cyc_Absyn_String_c_struct*_tmp43=_cycalloc(sizeof(*_tmp43));_tmp43[0]=({
struct Cyc_Absyn_String_c_struct _tmp44;_tmp44.tag=5;_tmp44.f1=*(*s).f2;_tmp44;});
_tmp43;}),seg);}static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_cnst_int(int i,struct Cyc_Position_Segment*
seg){return Cyc_Absyn_const_exp((void*)({struct Cyc_Absyn_Int_c_struct*_tmp45=
_cycalloc(sizeof(*_tmp45));_tmp45[0]=({struct Cyc_Absyn_Int_c_struct _tmp46;_tmp46.tag=
2;_tmp46.f1=(void*)((void*)0);_tmp46.f2=i;_tmp46;});_tmp45;}),seg);}static struct
Cyc_Absyn_Exp*Cyc_Tcgenrep_cnst_int_cls(struct Cyc_Position_Segment*seg,int i){
return Cyc_Absyn_const_exp((void*)({struct Cyc_Absyn_Int_c_struct*_tmp47=_cycalloc(
sizeof(*_tmp47));_tmp47[0]=({struct Cyc_Absyn_Int_c_struct _tmp48;_tmp48.tag=2;
_tmp48.f1=(void*)((void*)0);_tmp48.f2=i;_tmp48;});_tmp47;}),seg);}static int Cyc_Tcgenrep_size_of2int(
void*sz){void*_tmp49=sz;_LL6: if((int)_tmp49 != 0)goto _LL8;_LL7: return 8;_LL8: if((
int)_tmp49 != 1)goto _LLA;_LL9: return 16;_LLA: if((int)_tmp49 != 2)goto _LLC;_LLB:
return 32;_LLC: if((int)_tmp49 != 3)goto _LL5;_LLD: return 64;_LL5:;}static void*Cyc_Tcgenrep_array_typ(
void*ty,struct Cyc_Absyn_Tqual tq,struct Cyc_Absyn_Exp*e){return(void*)({struct Cyc_Absyn_ArrayType_struct*
_tmp4A=_cycalloc(sizeof(*_tmp4A));_tmp4A[0]=({struct Cyc_Absyn_ArrayType_struct
_tmp4B;_tmp4B.tag=7;_tmp4B.f1=(void*)ty;_tmp4B.f2=tq;_tmp4B.f3=e;_tmp4B;});
_tmp4A;});}static void*Cyc_Tcgenrep_tunion_typ(struct _tuple6*name){return(void*)({
struct Cyc_Absyn_TunionType_struct*_tmp4C=_cycalloc(sizeof(*_tmp4C));_tmp4C[0]=({
struct Cyc_Absyn_TunionType_struct _tmp4D;_tmp4D.tag=2;_tmp4D.f1=({struct Cyc_Absyn_TunionInfo
_tmp4E;_tmp4E.tunion_info=(void*)((void*)({struct Cyc_Absyn_UnknownTunion_struct*
_tmp4F=_cycalloc(sizeof(*_tmp4F));_tmp4F[0]=({struct Cyc_Absyn_UnknownTunion_struct
_tmp50;_tmp50.tag=0;_tmp50.f1=({struct Cyc_Absyn_UnknownTunionInfo _tmp51;_tmp51.name=
name;_tmp51.is_xtunion=0;_tmp51;});_tmp50;});_tmp4F;}));_tmp4E.targs=0;_tmp4E.rgn=(
void*)((void*)2);_tmp4E;});_tmp4D;});_tmp4C;});}static void*Cyc_Tcgenrep_tunionfield_typ(
struct _tuple6*name,struct _tuple6*fieldname){return(void*)({struct Cyc_Absyn_TunionFieldType_struct*
_tmp52=_cycalloc(sizeof(*_tmp52));_tmp52[0]=({struct Cyc_Absyn_TunionFieldType_struct
_tmp53;_tmp53.tag=3;_tmp53.f1=({struct Cyc_Absyn_TunionFieldInfo _tmp54;_tmp54.field_info=(
void*)((void*)({struct Cyc_Absyn_UnknownTunionfield_struct*_tmp55=_cycalloc(
sizeof(*_tmp55));_tmp55[0]=({struct Cyc_Absyn_UnknownTunionfield_struct _tmp56;
_tmp56.tag=0;_tmp56.f1=({struct Cyc_Absyn_UnknownTunionFieldInfo _tmp57;_tmp57.tunion_name=
name;_tmp57.field_name=fieldname;_tmp57.is_xtunion=0;_tmp57;});_tmp56;});_tmp55;}));
_tmp54.targs=0;_tmp54;});_tmp53;});_tmp52;});}static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_call_exp(
struct _tuple6*name,struct Cyc_List_List*args,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_unknowncall_exp(Cyc_Absyn_unknownid_exp(name,loc),args,loc);}
static struct Cyc_Absyn_Decl*Cyc_Tcgenrep_tunion_constructor_decl(struct _tuple6*
tunionname,struct _tuple6*fieldname,struct _tuple6*varname,struct Cyc_List_List*
args,void*sc,struct Cyc_Position_Segment*loc){void*_tmp58=Cyc_Tcgenrep_tunionfield_typ(
tunionname,fieldname);struct Cyc_Absyn_Exp*_tmp59=Cyc_Tcgenrep_call_exp(fieldname,
args,loc);struct Cyc_Absyn_Decl*_tmp5A=Cyc_Tcgenrep_gen_vardecl(varname,_tmp58,(
struct Cyc_Absyn_Exp*)_tmp59,sc,loc);return _tmp5A;}struct _tuple9{struct Cyc_Absyn_Tqual
f1;void*f2;};static void*Cyc_Tcgenrep_get_second(struct _tuple9*pair){return(*pair).f2;}
struct _tuple10{struct _tagged_arr*f1;struct Cyc_Absyn_Exp*f2;};static struct
_tagged_arr*Cyc_Tcgenrep_get_first(struct _tuple10*pair){return(*pair).f1;}static
unsigned char _tmp5B[5]="list";static struct _tagged_arr Cyc_Tcgenrep_list_str={
_tmp5B,_tmp5B,_tmp5B + 5};static unsigned char _tmp5C[5]="List";static struct
_tagged_arr Cyc_Tcgenrep_List_str={_tmp5C,_tmp5C,_tmp5C + 5};struct _tuple11{struct
Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;};static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_tuple2_exp_cls(
struct Cyc_Position_Segment*loc,struct _tuple11*es){struct _tuple11 _tmp5E;struct Cyc_Absyn_Exp*
_tmp5F;struct Cyc_Absyn_Exp*_tmp60;struct _tuple11*_tmp5D=es;_tmp5E=*_tmp5D;_tmp5F=
_tmp5E.f1;_tmp60=_tmp5E.f2;return Cyc_Absyn_tuple_exp(({struct Cyc_Absyn_Exp*
_tmp61[2];_tmp61[1]=_tmp60;_tmp61[0]=_tmp5F;((struct Cyc_List_List*(*)(struct
_tagged_arr))Cyc_List_list)(_tag_arr(_tmp61,sizeof(struct Cyc_Absyn_Exp*),2));}),
loc);}struct _tuple12{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*
f3;};static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_tuple3_exp_cls(struct Cyc_Position_Segment*
loc,struct _tuple12*es){struct _tuple12 _tmp63;struct Cyc_Absyn_Exp*_tmp64;struct Cyc_Absyn_Exp*
_tmp65;struct Cyc_Absyn_Exp*_tmp66;struct _tuple12*_tmp62=es;_tmp63=*_tmp62;_tmp64=
_tmp63.f1;_tmp65=_tmp63.f2;_tmp66=_tmp63.f3;return Cyc_Absyn_tuple_exp(({struct
Cyc_Absyn_Exp*_tmp67[3];_tmp67[2]=_tmp66;_tmp67[1]=_tmp65;_tmp67[0]=_tmp64;((
struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp67,
sizeof(struct Cyc_Absyn_Exp*),3));}),loc);}static unsigned char _tmp68[5]="NULL";
static struct _tagged_arr Cyc_Tcgenrep_null_str={_tmp68,_tmp68,_tmp68 + 5};static
struct Cyc_Absyn_Exp*Cyc_Tcgenrep_list_exp(struct Cyc_List_List*l,struct Cyc_Position_Segment*
loc){if(l == 0)return Cyc_Absyn_null_exp(loc);return Cyc_Tcgenrep_call_exp(Cyc_Tcgenrep_toplevel_name(&
Cyc_Tcgenrep_list_str),l,loc);}struct _tuple13{void*f1;struct Cyc_Position_Segment*
f2;};static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_make_offsetof_exp(struct _tuple13*
typeloc,int index){struct _tuple13 _tmp6A;void*_tmp6B;struct Cyc_Position_Segment*
_tmp6C;struct _tuple13*_tmp69=typeloc;_tmp6A=*_tmp69;_tmp6B=_tmp6A.f1;_tmp6C=
_tmp6A.f2;return Cyc_Absyn_offsetof_exp(_tmp6B,(void*)({struct Cyc_Absyn_TupleIndex_struct*
_tmp6D=_cycalloc_atomic(sizeof(*_tmp6D));_tmp6D[0]=({struct Cyc_Absyn_TupleIndex_struct
_tmp6E;_tmp6E.tag=1;_tmp6E.f1=(unsigned int)index;_tmp6E;});_tmp6D;}),_tmp6C);}
static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_get_and_cast_ri_exp(struct Cyc_Position_Segment*
loc,struct Cyc_Tcgenrep_RepInfo*info){return Cyc_Absyn_cast_exp(Cyc_Tcgenrep_tunion_typ(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str)),(struct Cyc_Absyn_Exp*)
_check_null(info->exp),loc);}static struct _tuple10*Cyc_Tcgenrep_gen_id_for_exp(
struct Cyc_Absyn_Exp*e){return({struct _tuple10*_tmp6F=_cycalloc(sizeof(*_tmp6F));
_tmp6F->f1=Cyc_Tcgenrep_new_gen_id(_tag_arr("tuple",sizeof(unsigned char),6));
_tmp6F->f2=e;_tmp6F;});}static struct Cyc_Absyn_Decl*Cyc_Tcgenrep_gen_decl_cls(
struct _tuple13*env,struct _tuple10*name_exp){struct _tuple13 _tmp71;void*_tmp72;
struct Cyc_Position_Segment*_tmp73;struct _tuple13*_tmp70=env;_tmp71=*_tmp70;
_tmp72=_tmp71.f1;_tmp73=_tmp71.f2;{struct _tuple10 _tmp75;struct _tagged_arr*_tmp76;
struct Cyc_Absyn_Exp*_tmp77;struct _tuple10*_tmp74=name_exp;_tmp75=*_tmp74;_tmp76=
_tmp75.f1;_tmp77=_tmp75.f2;return Cyc_Tcgenrep_gen_decl(_tmp76,_tmp72,(struct Cyc_Absyn_Exp*)
_tmp77,_tmp73);}}struct _tuple14{struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};
static struct _tuple14*Cyc_Tcgenrep_null_designator_exp(struct Cyc_Absyn_Exp*e){
return({struct _tuple14*_tmp78=_cycalloc(sizeof(*_tmp78));_tmp78->f1=0;_tmp78->f2=
e;_tmp78;});}static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_arr_init_exp(struct Cyc_List_List*
l,struct Cyc_Position_Segment*loc){struct Cyc_List_List*_tmp79=((struct Cyc_List_List*(*)(
struct _tuple14*(*f)(struct Cyc_Absyn_Exp*),struct Cyc_List_List*x))Cyc_List_map)(
Cyc_Tcgenrep_null_designator_exp,l);return Cyc_Absyn_unresolvedmem_exp(0,_tmp79,
loc);}static struct Cyc_Absyn_Exp*Cyc_Tcgenrep_address_exp_cls(struct Cyc_Position_Segment*
loc,struct Cyc_Absyn_Exp*e){return Cyc_Absyn_address_exp(e,loc);}static struct Cyc_Absyn_Exp*
Cyc_Tcgenrep_unknownid_exp_cls(struct Cyc_Position_Segment*loc,struct _tuple6*e){
return Cyc_Absyn_unknownid_exp(e,loc);}static int Cyc_Tcgenrep_has_bitfield(struct
Cyc_Absyn_Aggrfield*sf){return sf->width != 0;}static int Cyc_Tcgenrep_add_bitfield_sizes(
int total,struct Cyc_Absyn_Aggrfield*sf){return(int)(Cyc_Evexp_eval_const_uint_exp((
struct Cyc_Absyn_Exp*)_check_null(sf->width))+ total);}static void*Cyc_Tcgenrep_select_structfield_type(
struct Cyc_Absyn_Aggrfield*sf){if(Cyc_Std_strcmp(*sf->name,_tag_arr("",sizeof(
unsigned char),1))== 0)({void*_tmp7A[0]={};((int(*)(struct _tagged_arr fmt,struct
_tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen(): anonymous (padding) structfield not handled yet",
sizeof(unsigned char),55),_tag_arr(_tmp7A,sizeof(void*),0));});if(Cyc_Tcgenrep_has_bitfield(
sf))({void*_tmp7B[0]={};((int(*)(struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(
_tag_arr("gen(): mixed struct bitfields and union bitfields not handled",sizeof(
unsigned char),62),_tag_arr(_tmp7B,sizeof(void*),0));});return(void*)sf->type;}
struct _tuple15{struct _tagged_arr*f1;void*f2;};static struct _tuple15*Cyc_Tcgenrep_select_structfield_nmtype(
struct Cyc_Absyn_Aggrfield*sf){if(Cyc_Std_strcmp(*sf->name,_tag_arr("",sizeof(
unsigned char),1))== 0)({void*_tmp7C[0]={};((int(*)(struct _tagged_arr fmt,struct
_tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen(): anonymous (padding) structfield not handled yet",
sizeof(unsigned char),55),_tag_arr(_tmp7C,sizeof(void*),0));});if(Cyc_Tcgenrep_has_bitfield(
sf))({void*_tmp7D[0]={};((int(*)(struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(
_tag_arr("gen(): mixed struct bitfields and union bitfields not handled",sizeof(
unsigned char),62),_tag_arr(_tmp7D,sizeof(void*),0));});return({struct _tuple15*
_tmp7E=_cycalloc(sizeof(*_tmp7E));_tmp7E->f1=sf->name;_tmp7E->f2=(void*)sf->type;
_tmp7E;});}struct _tuple16{int f1;struct _tuple6*f2;};struct _tuple16*Cyc_Tcgenrep_select_enumfield_tagnm(
struct Cyc_Absyn_Enumfield*ef){if(ef->tag == 0)({void*_tmp7F[0]={};((int(*)(struct
_tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("Enum tag exp should be filled in by now",
sizeof(unsigned char),40),_tag_arr(_tmp7F,sizeof(void*),0));});return({struct
_tuple16*_tmp80=_cycalloc(sizeof(*_tmp80));_tmp80->f1=(int)Cyc_Evexp_eval_const_uint_exp((
struct Cyc_Absyn_Exp*)_check_null(ef->tag));_tmp80->f2=ef->name;_tmp80;});}static
struct Cyc_Dict_Dict*Cyc_Tcgenrep_update_info(struct Cyc_Dict_Dict*dict,void*type,
struct Cyc_List_List*decls,struct Cyc_Absyn_Exp*exp,struct Cyc_List_List*
dependencies,struct Cyc_Core_Opt*fwd_decl){struct Cyc_Core_Opt*_tmp81=((struct Cyc_Core_Opt*(*)(
struct Cyc_Dict_Dict*d,void*k))Cyc_Dict_lookup_opt)(dict,type);if(_tmp81 != 0){if(((
struct Cyc_Tcgenrep_RepInfo*)_tmp81->v)->decls != 0){Cyc_Tcgenrep_print_typerep_dict(
dict);({void*_tmp82[0]={};((int(*)(struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(
_tag_arr("Updating non-forward declaration",sizeof(unsigned char),33),_tag_arr(
_tmp82,sizeof(void*),0));});}((struct Cyc_Tcgenrep_RepInfo*)_tmp81->v)->decls=
decls;((struct Cyc_Tcgenrep_RepInfo*)_tmp81->v)->exp=exp;((struct Cyc_Tcgenrep_RepInfo*)
_tmp81->v)->dependencies=dependencies;return dict;}else{return((struct Cyc_Dict_Dict*(*)(
struct Cyc_Dict_Dict*d,void*k,struct Cyc_Tcgenrep_RepInfo*v))Cyc_Dict_insert)(dict,
type,({struct Cyc_Tcgenrep_RepInfo*_tmp83=_cycalloc(sizeof(*_tmp83));_tmp83->decls=
decls;_tmp83->exp=exp;_tmp83->dependencies=dependencies;_tmp83->fwd_decl=
fwd_decl;_tmp83->emitted=0;_tmp83->is_extern=0;_tmp83;}));}}static struct Cyc_Dict_Dict*
Cyc_Tcgenrep_make_fwd_decl_info(struct Cyc_Dict_Dict*dict,void*type,struct Cyc_Absyn_Exp*
exp,struct Cyc_Core_Opt*fwd_decl,int is_extern){struct Cyc_Core_Opt*_tmp84=((struct
Cyc_Core_Opt*(*)(struct Cyc_Dict_Dict*d,void*k))Cyc_Dict_lookup_opt)(dict,type);
if(_tmp84 != 0){({struct Cyc_Std_String_pa_struct _tmp86;_tmp86.tag=0;_tmp86.f1=(
struct _tagged_arr)Cyc_Absynpp_typ2string(type);{void*_tmp85[1]={& _tmp86};((int(*)(
struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("Repinfo for fwd declaration of %s already exists",
sizeof(unsigned char),49),_tag_arr(_tmp85,sizeof(void*),1));}});return dict;}
else{return((struct Cyc_Dict_Dict*(*)(struct Cyc_Dict_Dict*d,void*k,struct Cyc_Tcgenrep_RepInfo*
v))Cyc_Dict_insert)(dict,type,({struct Cyc_Tcgenrep_RepInfo*_tmp87=_cycalloc(
sizeof(*_tmp87));_tmp87->decls=0;_tmp87->exp=exp;_tmp87->dependencies=0;_tmp87->fwd_decl=
fwd_decl;_tmp87->emitted=0;_tmp87->is_extern=is_extern;_tmp87;}));}}static struct
Cyc_Absyn_Tqual Cyc_Tcgenrep_tq_none={0,0,0};static struct _tuple9*Cyc_Tcgenrep_tqual_type(
struct Cyc_Absyn_Tqual*tq,void*type){return({struct _tuple9*_tmp88=_cycalloc(
sizeof(*_tmp88));_tmp88->f1=*((struct Cyc_Absyn_Tqual*)_check_null(tq));_tmp88->f2=
type;_tmp88;});}static void*Cyc_Tcgenrep_tuple_typ(struct Cyc_List_List*types){
struct Cyc_List_List*_tmp89=((struct Cyc_List_List*(*)(struct _tuple9*(*f)(struct
Cyc_Absyn_Tqual*,void*),struct Cyc_Absyn_Tqual*env,struct Cyc_List_List*x))Cyc_List_map_c)(
Cyc_Tcgenrep_tqual_type,(struct Cyc_Absyn_Tqual*)& Cyc_Tcgenrep_tq_none,types);
struct Cyc_Absyn_TupleType_struct*tuple_type_base=({struct Cyc_Absyn_TupleType_struct*
_tmp8A=_cycalloc(sizeof(*_tmp8A));_tmp8A[0]=({struct Cyc_Absyn_TupleType_struct
_tmp8B;_tmp8B.tag=9;_tmp8B.f1=_tmp89;_tmp8B;});_tmp8A;});void*tuple_type=(void*)
tuple_type_base;return tuple_type;}static struct _tuple14*Cyc_Tcgenrep_array_decls(
void*type,struct Cyc_List_List*exps,struct Cyc_Position_Segment*loc){struct Cyc_List_List*
_tmp8C=((struct Cyc_List_List*(*)(struct _tuple10*(*f)(struct Cyc_Absyn_Exp*),
struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcgenrep_gen_id_for_exp,exps);struct Cyc_List_List*
_tmp8D=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Decl*(*f)(struct _tuple13*,
struct _tuple10*),struct _tuple13*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_gen_decl_cls,({
struct _tuple13*_tmp9A=_cycalloc(sizeof(*_tmp9A));_tmp9A->f1=type;_tmp9A->f2=loc;
_tmp9A;}),_tmp8C);struct Cyc_List_List*_tmp8E=((struct Cyc_List_List*(*)(struct
_tagged_arr*(*f)(struct _tuple10*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcgenrep_get_first,
_tmp8C);struct Cyc_List_List*_tmp8F=((struct Cyc_List_List*(*)(struct _tuple6*(*f)(
struct _tagged_arr*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcgenrep_toplevel_name,
_tmp8E);struct Cyc_List_List*_tmp90=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct _tuple6*),struct Cyc_Position_Segment*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_unknownid_exp_cls,loc,_tmp8F);
struct Cyc_List_List*_tmp91=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(
struct Cyc_Position_Segment*,struct Cyc_Absyn_Exp*),struct Cyc_Position_Segment*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_address_exp_cls,loc,_tmp90);
struct Cyc_Absyn_Exp*_tmp92=Cyc_Tcgenrep_arr_init_exp(_tmp91,loc);struct
_tagged_arr*_tmp93=Cyc_Tcgenrep_new_gen_id(_tag_arr("arr",sizeof(unsigned char),
4));void*_tmp94=Cyc_Absyn_at_typ(type,(void*)2,Cyc_Tcgenrep_tq_none);void*_tmp95=
Cyc_Tcgenrep_array_typ(_tmp94,Cyc_Tcgenrep_tq_none,0);struct Cyc_Absyn_Decl*
_tmp96=Cyc_Tcgenrep_gen_decl(_tmp93,_tmp95,(struct Cyc_Absyn_Exp*)_tmp92,loc);
struct Cyc_Absyn_Exp*_tmp97=Cyc_Absyn_unknownid_exp(Cyc_Tcgenrep_toplevel_name(
_tmp93),loc);return({struct _tuple14*_tmp98=_cycalloc(sizeof(*_tmp98));_tmp98->f1=((
struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(
_tmp8D,({struct Cyc_Absyn_Decl*_tmp99[1];_tmp99[0]=_tmp96;((struct Cyc_List_List*(*)(
struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp99,sizeof(struct Cyc_Absyn_Decl*),
1));}));_tmp98->f2=_tmp97;_tmp98;});}static void*Cyc_Tcgenrep_check_tunionfield_and_get_type(
struct Cyc_Absyn_Tunionfield*tuf){return(void*)({struct Cyc_Absyn_TupleType_struct*
_tmp9B=_cycalloc(sizeof(*_tmp9B));_tmp9B[0]=({struct Cyc_Absyn_TupleType_struct
_tmp9C;_tmp9C.tag=9;_tmp9C.f1=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,
struct Cyc_List_List*y))Cyc_List_imp_append)(({struct _tuple9*_tmp9D[1];_tmp9D[0]=({
struct _tuple9*_tmp9E=_cycalloc(sizeof(*_tmp9E));_tmp9E->f1=Cyc_Tcgenrep_tq_none;
_tmp9E->f2=Cyc_Absyn_uint_t;_tmp9E;});((struct Cyc_List_List*(*)(struct
_tagged_arr))Cyc_List_list)(_tag_arr(_tmp9D,sizeof(struct _tuple9*),1));}),tuf->typs);
_tmp9C;});_tmp9B;});}static struct _tuple6*Cyc_Tcgenrep_check_tunionfield_and_get_name(
struct Cyc_Absyn_Tunionfield*tuf){return tuf->name;}struct _tuple17{struct _tuple6*
f1;void*f2;};static struct _tuple17*Cyc_Tcgenrep_check_tunionfield_and_get_nmtype(
struct Cyc_Absyn_Tunionfield*tuf){return({struct _tuple17*_tmp9F=_cycalloc(sizeof(*
_tmp9F));_tmp9F->f1=tuf->name;_tmp9F->f2=(void*)({struct Cyc_Absyn_TupleType_struct*
_tmpA0=_cycalloc(sizeof(*_tmpA0));_tmpA0[0]=({struct Cyc_Absyn_TupleType_struct
_tmpA1;_tmpA1.tag=9;_tmpA1.f1=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,
struct Cyc_List_List*y))Cyc_List_imp_append)(({struct _tuple9*_tmpA2[1];_tmpA2[0]=({
struct _tuple9*_tmpA3=_cycalloc(sizeof(*_tmpA3));_tmpA3->f1=Cyc_Tcgenrep_tq_none;
_tmpA3->f2=Cyc_Absyn_uint_t;_tmpA3;});((struct Cyc_List_List*(*)(struct
_tagged_arr))Cyc_List_list)(_tag_arr(_tmpA2,sizeof(struct _tuple9*),1));}),tuf->typs);
_tmpA1;});_tmpA0;});_tmp9F;});}static struct _tuple15*Cyc_Tcgenrep_check_xtunionfield_and_get_name_type(
struct Cyc_Absyn_Tunionfield*tuf){return({struct _tuple15*_tmpA4=_cycalloc(sizeof(*
_tmpA4));_tmpA4->f1=(*tuf->name).f2;_tmpA4->f2=(void*)({struct Cyc_Absyn_TupleType_struct*
_tmpA5=_cycalloc(sizeof(*_tmpA5));_tmpA5[0]=({struct Cyc_Absyn_TupleType_struct
_tmpA6;_tmpA6.tag=9;_tmpA6.f1=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,
struct Cyc_List_List*y))Cyc_List_imp_append)(({struct _tuple9*_tmpA7[1];_tmpA7[0]=({
struct _tuple9*_tmpA8=_cycalloc(sizeof(*_tmpA8));_tmpA8->f1=Cyc_Tcgenrep_tq_none;
_tmpA8->f2=Cyc_Absyn_star_typ(Cyc_Absyn_uchar_t,(void*)2,Cyc_Tcgenrep_tq_none);
_tmpA8;});((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(
_tmpA7,sizeof(struct _tuple9*),1));}),tuf->typs);_tmpA6;});_tmpA5;});_tmpA4;});}
static int Cyc_Tcgenrep_filter_empty_tunionfield(struct Cyc_Absyn_Tunionfield*tuf){
return tuf->typs != 0;}static int Cyc_Tcgenrep_filter_nonempty_tunionfield(struct Cyc_Absyn_Tunionfield*
tuf){return tuf->typs == 0;}static struct _tagged_arr*Cyc_Tcgenrep_get_tunionfield_name(
struct Cyc_Absyn_Tunionfield*tuf){return(*tuf->name).f2;}struct Cyc_Absyn_Aggrfield*
Cyc_Tcgenrep_substitute_structfield_type(struct Cyc_List_List*subst,struct Cyc_Absyn_Aggrfield*
sf){return({struct Cyc_Absyn_Aggrfield*_tmpA9=_cycalloc(sizeof(*_tmpA9));_tmpA9->name=
sf->name;_tmpA9->tq=sf->tq;_tmpA9->type=(void*)Cyc_Tcutil_substitute(subst,(void*)
sf->type);_tmpA9->width=sf->width;_tmpA9->attributes=sf->attributes;_tmpA9;});}
struct _tuple9*Cyc_Tcgenrep_substitute_tqual_type(struct Cyc_List_List*subst,
struct _tuple9*pair){struct _tuple9 _tmpAB;struct Cyc_Absyn_Tqual _tmpAC;void*_tmpAD;
struct _tuple9*_tmpAA=pair;_tmpAB=*_tmpAA;_tmpAC=_tmpAB.f1;_tmpAD=_tmpAB.f2;
return({struct _tuple9*_tmpAE=_cycalloc(sizeof(*_tmpAE));_tmpAE->f1=_tmpAC;_tmpAE->f2=
Cyc_Tcutil_substitute(subst,_tmpAD);_tmpAE;});}struct Cyc_Absyn_Tunionfield*Cyc_Tcgenrep_substitute_tunionfield_type(
struct Cyc_List_List*subst,struct Cyc_Absyn_Tunionfield*tf){return({struct Cyc_Absyn_Tunionfield*
_tmpAF=_cycalloc(sizeof(*_tmpAF));_tmpAF->name=tf->name;_tmpAF->typs=((struct Cyc_List_List*(*)(
struct _tuple9*(*f)(struct Cyc_List_List*,struct _tuple9*),struct Cyc_List_List*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_substitute_tqual_type,subst,
tf->typs);_tmpAF->loc=tf->loc;_tmpAF->sc=(void*)((void*)tf->sc);_tmpAF;});}void*
Cyc_Tcgenrep_monomorphize_type(void*type){void*_tmpB0=Cyc_Tcutil_compress(type);
struct Cyc_Absyn_AggrInfo _tmpB1;void*_tmpB2;struct Cyc_List_List*_tmpB3;struct Cyc_Absyn_TunionInfo
_tmpB4;void*_tmpB5;struct Cyc_Absyn_Tuniondecl**_tmpB6;struct Cyc_Absyn_Tuniondecl*
_tmpB7;struct Cyc_List_List*_tmpB8;void*_tmpB9;_LLF: if(_tmpB0 <= (void*)3?1:*((int*)
_tmpB0)!= 10)goto _LL11;_tmpB1=((struct Cyc_Absyn_AggrType_struct*)_tmpB0)->f1;
_tmpB2=(void*)_tmpB1.aggr_info;_tmpB3=_tmpB1.targs;_LL10: {struct Cyc_Absyn_Aggrdecl*
_tmpBA=Cyc_Absyn_get_known_aggrdecl(_tmpB2);struct Cyc_List_List*_tmpBB=_tmpBA->tvs;
if(Cyc_List_length(_tmpB3)!= ((int(*)(struct Cyc_List_List*x))Cyc_List_length)(
_tmpBB))({struct Cyc_Std_Int_pa_struct _tmpBE;_tmpBE.tag=1;_tmpBE.f1=(int)((
unsigned int)((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmpBB));{struct
Cyc_Std_Int_pa_struct _tmpBD;_tmpBD.tag=1;_tmpBD.f1=(int)((unsigned int)Cyc_List_length(
_tmpB3));{void*_tmpBC[2]={& _tmpBD,& _tmpBE};((int(*)(struct _tagged_arr fmt,struct
_tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen(): number of params %d differs from number of tyvars %d",
sizeof(unsigned char),60),_tag_arr(_tmpBC,sizeof(void*),2));}}});{struct Cyc_List_List*
_tmpBF=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_zip)(_tmpBB,_tmpB3);struct Cyc_Core_Opt*fields=0;if(_tmpBA->fields != 0)
fields=({struct Cyc_Core_Opt*_tmpC0=_cycalloc(sizeof(*_tmpC0));_tmpC0->v=((struct
Cyc_List_List*(*)(struct Cyc_Absyn_Aggrfield*(*f)(struct Cyc_List_List*,struct Cyc_Absyn_Aggrfield*),
struct Cyc_List_List*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_substitute_structfield_type,
_tmpBF,(struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(_tmpBA->fields))->v);
_tmpC0;});{struct Cyc_Absyn_Aggrdecl*ad2=({struct Cyc_Absyn_Aggrdecl*_tmpC7=
_cycalloc(sizeof(*_tmpC7));_tmpC7->kind=(void*)((void*)_tmpBA->kind);_tmpC7->sc=(
void*)((void*)_tmpBA->sc);_tmpC7->name=_tmpBA->name;_tmpC7->tvs=0;_tmpC7->exist_vars=
_tmpBA->exist_vars;_tmpC7->fields=fields;_tmpC7->attributes=_tmpBA->attributes;
_tmpC7;});return(void*)({struct Cyc_Absyn_AggrType_struct*_tmpC1=_cycalloc(
sizeof(*_tmpC1));_tmpC1[0]=({struct Cyc_Absyn_AggrType_struct _tmpC2;_tmpC2.tag=10;
_tmpC2.f1=({struct Cyc_Absyn_AggrInfo _tmpC3;_tmpC3.aggr_info=(void*)((void*)({
struct Cyc_Absyn_KnownAggr_struct*_tmpC4=_cycalloc(sizeof(*_tmpC4));_tmpC4[0]=({
struct Cyc_Absyn_KnownAggr_struct _tmpC5;_tmpC5.tag=1;_tmpC5.f1=({struct Cyc_Absyn_Aggrdecl**
_tmpC6=_cycalloc(sizeof(*_tmpC6));_tmpC6[0]=ad2;_tmpC6;});_tmpC5;});_tmpC4;}));
_tmpC3.targs=0;_tmpC3;});_tmpC2;});_tmpC1;});}}}_LL11: if(_tmpB0 <= (void*)3?1:*((
int*)_tmpB0)!= 2)goto _LL13;_tmpB4=((struct Cyc_Absyn_TunionType_struct*)_tmpB0)->f1;
_tmpB5=(void*)_tmpB4.tunion_info;if(*((int*)_tmpB5)!= 1)goto _LL13;_tmpB6=((
struct Cyc_Absyn_KnownTunion_struct*)_tmpB5)->f1;_tmpB7=*_tmpB6;_tmpB8=_tmpB4.targs;
_tmpB9=(void*)_tmpB4.rgn;_LL12: {struct Cyc_List_List*_tmpC8=_tmpB7->tvs;if(Cyc_List_length(
_tmpB8)!= ((int(*)(struct Cyc_List_List*x))Cyc_List_length)(_tmpC8))({struct Cyc_Std_Int_pa_struct
_tmpCB;_tmpCB.tag=1;_tmpCB.f1=(int)((unsigned int)((int(*)(struct Cyc_List_List*x))
Cyc_List_length)(_tmpC8));{struct Cyc_Std_Int_pa_struct _tmpCA;_tmpCA.tag=1;_tmpCA.f1=(
int)((unsigned int)Cyc_List_length(_tmpB8));{void*_tmpC9[2]={& _tmpCA,& _tmpCB};((
int(*)(struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen(): number of params %d differs from number of tyvars %d",
sizeof(unsigned char),60),_tag_arr(_tmpC9,sizeof(void*),2));}}});{struct Cyc_List_List*
_tmpCC=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_zip)(_tmpC8,_tmpB8);struct Cyc_Core_Opt*fields=0;if(_tmpB7->fields != 0)
fields=({struct Cyc_Core_Opt*_tmpCD=_cycalloc(sizeof(*_tmpCD));_tmpCD->v=((struct
Cyc_List_List*(*)(struct Cyc_Absyn_Tunionfield*(*f)(struct Cyc_List_List*,struct
Cyc_Absyn_Tunionfield*),struct Cyc_List_List*env,struct Cyc_List_List*x))Cyc_List_map_c)(
Cyc_Tcgenrep_substitute_tunionfield_type,_tmpCC,(struct Cyc_List_List*)((struct
Cyc_Core_Opt*)_check_null(_tmpB7->fields))->v);_tmpCD;});{struct Cyc_Absyn_Tuniondecl*
_tmpCE=({struct Cyc_Absyn_Tuniondecl*_tmpD7=_cycalloc(sizeof(*_tmpD7));_tmpD7->sc=(
void*)((void*)_tmpB7->sc);_tmpD7->name=_tmpB7->name;_tmpD7->tvs=0;_tmpD7->fields=
fields;_tmpD7->is_xtunion=_tmpB7->is_xtunion;_tmpD7;});struct Cyc_Absyn_TunionType_struct*
_tmpCF=({struct Cyc_Absyn_TunionType_struct*_tmpD1=_cycalloc(sizeof(*_tmpD1));
_tmpD1[0]=({struct Cyc_Absyn_TunionType_struct _tmpD2;_tmpD2.tag=2;_tmpD2.f1=({
struct Cyc_Absyn_TunionInfo _tmpD3;_tmpD3.tunion_info=(void*)((void*)({struct Cyc_Absyn_KnownTunion_struct*
_tmpD4=_cycalloc(sizeof(*_tmpD4));_tmpD4[0]=({struct Cyc_Absyn_KnownTunion_struct
_tmpD5;_tmpD5.tag=1;_tmpD5.f1=({struct Cyc_Absyn_Tuniondecl**_tmpD6=_cycalloc(
sizeof(*_tmpD6));_tmpD6[0]=_tmpCE;_tmpD6;});_tmpD5;});_tmpD4;}));_tmpD3.targs=
_tmpB8;_tmpD3.rgn=(void*)_tmpB9;_tmpD3;});_tmpD2;});_tmpD1;});struct Cyc_Position_Segment*
_tmpD0=Cyc_Position_segment_of_abs(0,0);return(void*)_tmpCF;}}}_LL13:;_LL14:
return type;_LLE:;}struct _tagged_arr Cyc_Tcgenrep_make_type_cstring(void*t){struct
_tagged_arr s=Cyc_Std_strdup(Cyc_Absynpp_typ2cstring(t));{int i=0;for(0;i < Cyc_Std_strlen((
struct _tagged_arr)s);i ++){if(((unsigned char*)s.curr)[i]== ' ')((unsigned char*)
s.curr)[i]='_';else{if(!isalnum((int)((unsigned char*)s.curr)[i])?((
unsigned char*)s.curr)[i]!= '_': 0)((unsigned char*)s.curr)[i]=(unsigned char)('0'
+ ((unsigned char*)s.curr)[i]% 10);}}}return(struct _tagged_arr)Cyc_Std_strconcat((
struct _tagged_arr)s,_tag_arr("_rep",sizeof(unsigned char),5));}struct _tuple18{
struct Cyc_Dict_Dict*f1;struct Cyc_Tcgenrep_RepInfo*f2;};static struct _tuple18*Cyc_Tcgenrep_lookupRep(
struct Cyc_Tcenv_Tenv*te,struct Cyc_Dict_Dict*dict,struct Cyc_Position_Segment*loc,
void*type);struct _tuple19{struct Cyc_Dict_Dict*f1;struct Cyc_List_List*f2;};struct
_tuple20{struct Cyc_Tcenv_Tenv*f1;struct Cyc_Position_Segment*f2;};static struct
_tuple19*Cyc_Tcgenrep_lookupRepsCls(struct _tuple20*env,void*type,struct _tuple19*
carry){struct _tuple18 _tmpD9;struct Cyc_Dict_Dict*_tmpDA;struct Cyc_Tcgenrep_RepInfo*
_tmpDB;struct _tuple18*_tmpD8=Cyc_Tcgenrep_lookupRep((*env).f1,(*carry).f1,(*env).f2,
type);_tmpD9=*_tmpD8;_tmpDA=_tmpD9.f1;_tmpDB=_tmpD9.f2;return({struct _tuple19*
_tmpDC=_cycalloc(sizeof(*_tmpDC));_tmpDC->f1=_tmpDA;_tmpDC->f2=({struct Cyc_List_List*
_tmpDD=_cycalloc(sizeof(*_tmpDD));_tmpDD->hd=_tmpDB;_tmpDD->tl=(*carry).f2;
_tmpDD;});_tmpDC;});}static struct Cyc_Dict_Dict*Cyc_Tcgenrep_buildRepTuple(struct
Cyc_Tcenv_Tenv*te,struct Cyc_Dict_Dict*dict,struct Cyc_Position_Segment*loc,struct
_tuple6*varname,void*sc,void*type,struct Cyc_List_List*types){struct Cyc_Absyn_Exp*
_tmpDE=Cyc_Absyn_sizeoftyp_exp(type,loc);struct _tuple20*_tmpDF=({struct _tuple20*
_tmpF4=_cycalloc(sizeof(*_tmpF4));_tmpF4->f1=te;_tmpF4->f2=loc;_tmpF4;});struct
_tuple19*_tmpE0=({struct _tuple19*_tmpF3=_cycalloc(sizeof(*_tmpF3));_tmpF3->f1=
dict;_tmpF3->f2=0;_tmpF3;});struct _tuple19 _tmpE2;struct Cyc_Dict_Dict*_tmpE3;
struct Cyc_List_List*_tmpE4;struct _tuple19*_tmpE1=((struct _tuple19*(*)(struct
_tuple19*(*f)(struct _tuple20*,void*,struct _tuple19*),struct _tuple20*,struct Cyc_List_List*
x,struct _tuple19*accum))Cyc_List_fold_right_c)(Cyc_Tcgenrep_lookupRepsCls,_tmpDF,
types,_tmpE0);_tmpE2=*_tmpE1;_tmpE3=_tmpE2.f1;_tmpE4=_tmpE2.f2;dict=_tmpE3;{
struct Cyc_List_List*_tmpE5=((struct Cyc_List_List*(*)(int n,int(*f)(int)))Cyc_List_tabulate)(
Cyc_List_length(types),(int(*)(int))Cyc_Core_identity);struct Cyc_List_List*
_tmpE6=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct _tuple13*,int),
struct _tuple13*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_make_offsetof_exp,({
struct _tuple13*_tmpF2=_cycalloc(sizeof(*_tmpF2));_tmpF2->f1=type;_tmpF2->f2=loc;
_tmpF2;}),_tmpE5);struct Cyc_List_List*_tmpE7=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct Cyc_Tcgenrep_RepInfo*),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_get_and_cast_ri_exp,loc,
_tmpE4);struct Cyc_List_List*_tmpE8=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct _tuple11*),struct Cyc_Position_Segment*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_tuple2_exp_cls,loc,((struct
Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_zip)(
_tmpE6,_tmpE7));void*tuple_type=Cyc_Tcgenrep_tuple_typ(({void*_tmpF1[2];_tmpF1[1]=
Cyc_Tcgenrep_tunion_typ(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str));
_tmpF1[0]=Cyc_Absyn_uint_t;Cyc_List_list(_tag_arr(_tmpF1,sizeof(void*),2));}));
struct _tuple14 _tmpEA;struct Cyc_List_List*_tmpEB;struct Cyc_Absyn_Exp*_tmpEC;
struct _tuple14*_tmpE9=Cyc_Tcgenrep_array_decls(tuple_type,_tmpE8,loc);_tmpEA=*
_tmpE9;_tmpEB=_tmpEA.f1;_tmpEC=_tmpEA.f2;{struct Cyc_Absyn_Decl*_tmpED=Cyc_Tcgenrep_tunion_constructor_decl(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_tuple_str),varname,({struct Cyc_Absyn_Exp*_tmpF0[2];_tmpF0[1]=_tmpEC;
_tmpF0[0]=_tmpDE;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmpF0,sizeof(struct Cyc_Absyn_Exp*),2));}),sc,loc);struct Cyc_List_List*
_tmpEE=((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_imp_append)(_tmpEB,({struct Cyc_Absyn_Decl*_tmpEF[1];_tmpEF[0]=_tmpED;((
struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmpEF,
sizeof(struct Cyc_Absyn_Decl*),1));}));return Cyc_Tcgenrep_update_info(dict,type,
_tmpEE,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
varname,loc),loc),_tmpE4,0);}}}static struct Cyc_Dict_Dict*Cyc_Tcgenrep_buildRepTunionfield(
struct _tuple6*tname,struct _tuple6*fname,struct Cyc_Tcenv_Tenv*te,struct Cyc_Dict_Dict*
dict,struct Cyc_Position_Segment*loc,struct _tuple6*varname,void*sc,void*type,
struct Cyc_List_List*types){struct Cyc_Absyn_Exp*_tmpF5=Cyc_Absyn_sizeoftyp_exp(
type,loc);struct _tuple20*_tmpF6=({struct _tuple20*_tmp10D=_cycalloc(sizeof(*
_tmp10D));_tmp10D->f1=te;_tmp10D->f2=loc;_tmp10D;});struct _tuple19*_tmpF7=({
struct _tuple19*_tmp10C=_cycalloc(sizeof(*_tmp10C));_tmp10C->f1=dict;_tmp10C->f2=
0;_tmp10C;});struct _tuple19 _tmpF9;struct Cyc_Dict_Dict*_tmpFA;struct Cyc_List_List*
_tmpFB;struct _tuple19*_tmpF8=((struct _tuple19*(*)(struct _tuple19*(*f)(struct
_tuple20*,void*,struct _tuple19*),struct _tuple20*,struct Cyc_List_List*x,struct
_tuple19*accum))Cyc_List_fold_right_c)(Cyc_Tcgenrep_lookupRepsCls,_tmpF6,types,
_tmpF7);_tmpF9=*_tmpF8;_tmpFA=_tmpF9.f1;_tmpFB=_tmpF9.f2;dict=_tmpFA;{struct Cyc_List_List*
_tmpFC=((struct Cyc_List_List*(*)(int n,int(*f)(int)))Cyc_List_tabulate)(Cyc_List_length(
types),(int(*)(int))Cyc_Core_identity);struct Cyc_List_List*_tmpFD=((struct Cyc_List_List*(*)(
struct Cyc_Absyn_Exp*(*f)(struct _tuple13*,int),struct _tuple13*env,struct Cyc_List_List*
x))Cyc_List_map_c)(Cyc_Tcgenrep_make_offsetof_exp,({struct _tuple13*_tmp10B=
_cycalloc(sizeof(*_tmp10B));_tmp10B->f1=type;_tmp10B->f2=loc;_tmp10B;}),_tmpFC);
struct Cyc_List_List*_tmpFE=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(
struct Cyc_Position_Segment*,struct Cyc_Tcgenrep_RepInfo*),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_get_and_cast_ri_exp,loc,
_tmpFB);struct Cyc_List_List*_tmpFF=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct _tuple11*),struct Cyc_Position_Segment*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_tuple2_exp_cls,loc,((struct
Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_zip)(
_tmpFD,_tmpFE));void*tuple_type=Cyc_Tcgenrep_tuple_typ(({void*_tmp10A[2];_tmp10A[
1]=Cyc_Tcgenrep_tunion_typ(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str));
_tmp10A[0]=Cyc_Absyn_uint_t;Cyc_List_list(_tag_arr(_tmp10A,sizeof(void*),2));}));
struct Cyc_Absyn_Exp*_tmp100=Cyc_Tcgenrep_cnst_string(*(*tname).f2,loc);struct Cyc_Absyn_Exp*
_tmp101=Cyc_Tcgenrep_cnst_string(*(*fname).f2,loc);struct _tuple14 _tmp103;struct
Cyc_List_List*_tmp104;struct Cyc_Absyn_Exp*_tmp105;struct _tuple14*_tmp102=Cyc_Tcgenrep_array_decls(
tuple_type,_tmpFF,loc);_tmp103=*_tmp102;_tmp104=_tmp103.f1;_tmp105=_tmp103.f2;{
struct Cyc_Absyn_Decl*_tmp106=Cyc_Tcgenrep_tunion_constructor_decl(Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_tunionfield_str),
varname,({struct Cyc_Absyn_Exp*_tmp109[4];_tmp109[3]=_tmp105;_tmp109[2]=_tmpF5;
_tmp109[1]=_tmp101;_tmp109[0]=_tmp100;((struct Cyc_List_List*(*)(struct
_tagged_arr))Cyc_List_list)(_tag_arr(_tmp109,sizeof(struct Cyc_Absyn_Exp*),4));}),
sc,loc);struct Cyc_List_List*_tmp107=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_imp_append)(_tmp104,({struct Cyc_Absyn_Decl*
_tmp108[1];_tmp108[0]=_tmp106;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmp108,sizeof(struct Cyc_Absyn_Decl*),1));}));return Cyc_Tcgenrep_update_info(
dict,type,_tmp107,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
varname,loc),loc),_tmpFB,0);}}}static struct Cyc_Dict_Dict*Cyc_Tcgenrep_buildRepStruct(
struct Cyc_Core_Opt*sname,struct Cyc_Tcenv_Tenv*te,struct Cyc_Dict_Dict*dict,struct
Cyc_Position_Segment*loc,struct _tuple6*varname,void*sc,void*type,struct Cyc_List_List*
nmtypes){struct Cyc_Absyn_Exp*_tmp10E=Cyc_Absyn_sizeoftyp_exp(type,loc);struct
_tuple20*_tmp10F=({struct _tuple20*_tmp12B=_cycalloc(sizeof(*_tmp12B));_tmp12B->f1=
te;_tmp12B->f2=loc;_tmp12B;});struct _tuple19*_tmp110=({struct _tuple19*_tmp12A=
_cycalloc(sizeof(*_tmp12A));_tmp12A->f1=dict;_tmp12A->f2=0;_tmp12A;});struct Cyc_List_List*
_tmp112;struct Cyc_List_List*_tmp113;struct _tuple0 _tmp111=((struct _tuple0(*)(
struct Cyc_List_List*x))Cyc_List_split)(nmtypes);_tmp112=_tmp111.f1;_tmp113=
_tmp111.f2;{struct _tuple19 _tmp115;struct Cyc_Dict_Dict*_tmp116;struct Cyc_List_List*
_tmp117;struct _tuple19*_tmp114=((struct _tuple19*(*)(struct _tuple19*(*f)(struct
_tuple20*,void*,struct _tuple19*),struct _tuple20*,struct Cyc_List_List*x,struct
_tuple19*accum))Cyc_List_fold_right_c)(Cyc_Tcgenrep_lookupRepsCls,_tmp10F,
_tmp113,_tmp110);_tmp115=*_tmp114;_tmp116=_tmp115.f1;_tmp117=_tmp115.f2;dict=
_tmp116;{struct Cyc_List_List*_tmp118=((struct Cyc_List_List*(*)(int n,int(*f)(int)))
Cyc_List_tabulate)(Cyc_List_length(_tmp113),(int(*)(int))Cyc_Core_identity);
struct Cyc_List_List*_tmp119=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(
struct _tuple13*,int),struct _tuple13*env,struct Cyc_List_List*x))Cyc_List_map_c)(
Cyc_Tcgenrep_make_offsetof_exp,({struct _tuple13*_tmp129=_cycalloc(sizeof(*
_tmp129));_tmp129->f1=type;_tmp129->f2=loc;_tmp129;}),_tmp118);struct Cyc_List_List*
_tmp11A=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct Cyc_Position_Segment*,
struct _tagged_arr*),struct Cyc_Position_Segment*env,struct Cyc_List_List*x))Cyc_List_map_c)(
Cyc_Tcgenrep_cnst_string_cls,loc,_tmp112);struct Cyc_List_List*_tmp11B=((struct
Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct Cyc_Position_Segment*,struct Cyc_Tcgenrep_RepInfo*),
struct Cyc_Position_Segment*env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_get_and_cast_ri_exp,
loc,_tmp117);struct Cyc_List_List*_tmp11C=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct _tuple12*),struct Cyc_Position_Segment*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_tuple3_exp_cls,loc,((struct
Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y,struct Cyc_List_List*
z))Cyc_List_zip3)(_tmp119,_tmp11A,_tmp11B));void*tuple_type=Cyc_Tcgenrep_tuple_typ(({
void*_tmp128[3];_tmp128[2]=Cyc_Tcgenrep_tunion_typ(Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_typestruct_str));_tmp128[1]=Cyc_Absyn_const_string_typ((void*)2);
_tmp128[0]=Cyc_Absyn_uint_t;Cyc_List_list(_tag_arr(_tmp128,sizeof(void*),3));}));
struct _tuple14 _tmp11E;struct Cyc_List_List*_tmp11F;struct Cyc_Absyn_Exp*_tmp120;
struct _tuple14*_tmp11D=Cyc_Tcgenrep_array_decls(tuple_type,_tmp11C,loc);_tmp11E=*
_tmp11D;_tmp11F=_tmp11E.f1;_tmp120=_tmp11E.f2;{struct Cyc_Absyn_Exp*name;if(sname
== 0)name=Cyc_Absyn_null_exp(loc);else{struct _tagged_arr*_tmp121=Cyc_Tcgenrep_new_gen_id(
_tag_arr("name",sizeof(unsigned char),5));struct Cyc_Absyn_Decl*_tmp122=Cyc_Tcgenrep_gen_decl(
_tmp121,Cyc_Absyn_const_string_typ((void*)2),(struct Cyc_Absyn_Exp*)Cyc_Tcgenrep_cnst_string(*(*((
struct _tuple6*)sname->v)).f2,loc),loc);_tmp11F=({struct Cyc_List_List*_tmp123=
_cycalloc(sizeof(*_tmp123));_tmp123->hd=_tmp122;_tmp123->tl=_tmp11F;_tmp123;});
name=Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(Cyc_Tcgenrep_toplevel_name(
_tmp121),loc),loc);}{struct Cyc_Absyn_Decl*_tmp124=Cyc_Tcgenrep_tunion_constructor_decl(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_struct_str),varname,({struct Cyc_Absyn_Exp*_tmp127[3];_tmp127[2]=
_tmp120;_tmp127[1]=_tmp10E;_tmp127[0]=name;((struct Cyc_List_List*(*)(struct
_tagged_arr))Cyc_List_list)(_tag_arr(_tmp127,sizeof(struct Cyc_Absyn_Exp*),3));}),
sc,loc);struct Cyc_List_List*_tmp125=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_imp_append)(_tmp11F,({struct Cyc_Absyn_Decl*
_tmp126[1];_tmp126[0]=_tmp124;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmp126,sizeof(struct Cyc_Absyn_Decl*),1));}));return Cyc_Tcgenrep_update_info(
dict,type,_tmp125,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
varname,loc),loc),_tmp117,0);}}}}}static struct Cyc_Dict_Dict*Cyc_Tcgenrep_buildRepUnion(
struct Cyc_Core_Opt*uname,struct Cyc_Tcenv_Tenv*te,struct Cyc_Dict_Dict*dict,struct
Cyc_Position_Segment*loc,struct _tuple6*varname,void*sc,void*type,struct Cyc_List_List*
nmtypes){struct Cyc_Absyn_Exp*_tmp12C=Cyc_Absyn_sizeoftyp_exp(type,loc);struct
_tuple20*_tmp12D=({struct _tuple20*_tmp146=_cycalloc(sizeof(*_tmp146));_tmp146->f1=
te;_tmp146->f2=loc;_tmp146;});struct _tuple19*_tmp12E=({struct _tuple19*_tmp145=
_cycalloc(sizeof(*_tmp145));_tmp145->f1=dict;_tmp145->f2=0;_tmp145;});struct Cyc_List_List*
_tmp130;struct Cyc_List_List*_tmp131;struct _tuple0 _tmp12F=((struct _tuple0(*)(
struct Cyc_List_List*x))Cyc_List_split)(nmtypes);_tmp130=_tmp12F.f1;_tmp131=
_tmp12F.f2;{struct _tuple19 _tmp133;struct Cyc_Dict_Dict*_tmp134;struct Cyc_List_List*
_tmp135;struct _tuple19*_tmp132=((struct _tuple19*(*)(struct _tuple19*(*f)(struct
_tuple20*,void*,struct _tuple19*),struct _tuple20*,struct Cyc_List_List*x,struct
_tuple19*accum))Cyc_List_fold_right_c)(Cyc_Tcgenrep_lookupRepsCls,_tmp12D,
_tmp131,_tmp12E);_tmp133=*_tmp132;_tmp134=_tmp133.f1;_tmp135=_tmp133.f2;dict=
_tmp134;{struct Cyc_List_List*_tmp136=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct _tagged_arr*),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_cnst_string_cls,loc,
_tmp130);struct Cyc_List_List*_tmp137=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct Cyc_Tcgenrep_RepInfo*),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_get_and_cast_ri_exp,loc,
_tmp135);struct Cyc_List_List*_tmp138=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct _tuple11*),struct Cyc_Position_Segment*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_tuple2_exp_cls,loc,((struct
Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_zip)(
_tmp136,_tmp137));void*_tmp139=Cyc_Tcgenrep_tuple_typ(({void*_tmp144[2];_tmp144[
1]=Cyc_Tcgenrep_tunion_typ(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str));
_tmp144[0]=Cyc_Absyn_const_string_typ((void*)2);Cyc_List_list(_tag_arr(_tmp144,
sizeof(void*),2));}));struct _tuple14 _tmp13B;struct Cyc_List_List*_tmp13C;struct
Cyc_Absyn_Exp*_tmp13D;struct _tuple14*_tmp13A=Cyc_Tcgenrep_array_decls(_tmp139,
_tmp138,loc);_tmp13B=*_tmp13A;_tmp13C=_tmp13B.f1;_tmp13D=_tmp13B.f2;{struct Cyc_Absyn_Exp*
name;if(uname == 0)name=Cyc_Absyn_null_exp(loc);else{struct _tagged_arr*_tmp13E=
Cyc_Tcgenrep_new_gen_id(_tag_arr("name",sizeof(unsigned char),5));struct Cyc_Absyn_Decl*
_tmp13F=Cyc_Tcgenrep_gen_decl(_tmp13E,Cyc_Absyn_const_string_typ((void*)2),(
struct Cyc_Absyn_Exp*)Cyc_Tcgenrep_cnst_string(*(*((struct _tuple6*)uname->v)).f2,
loc),loc);_tmp13C=({struct Cyc_List_List*_tmp140=_cycalloc(sizeof(*_tmp140));
_tmp140->hd=_tmp13F;_tmp140->tl=_tmp13C;_tmp140;});name=Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(Cyc_Tcgenrep_toplevel_name(_tmp13E),loc),loc);}{struct
Cyc_Absyn_Decl*_tmp141=Cyc_Tcgenrep_tunion_constructor_decl(Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_union_str),
varname,({struct Cyc_Absyn_Exp*_tmp143[3];_tmp143[2]=_tmp13D;_tmp143[1]=_tmp12C;
_tmp143[0]=name;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmp143,sizeof(struct Cyc_Absyn_Exp*),3));}),sc,loc);return Cyc_Tcgenrep_update_info(
dict,type,((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_imp_append)(_tmp13C,({struct Cyc_Absyn_Decl*_tmp142[1];_tmp142[0]=
_tmp141;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(
_tmp142,sizeof(struct Cyc_Absyn_Decl*),1));})),(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(varname,loc),loc),_tmp135,0);}}}}}static struct Cyc_Dict_Dict*
Cyc_Tcgenrep_buildRepEnum(struct _tuple6**ename,struct Cyc_Tcenv_Tenv*te,struct Cyc_Dict_Dict*
dict,struct Cyc_Position_Segment*loc,struct _tuple6*varname,void*sc,void*type,
struct Cyc_List_List*tagnms){struct Cyc_Absyn_Exp*_tmp147=Cyc_Absyn_sizeoftyp_exp(
type,loc);struct Cyc_List_List*_tmp149;struct Cyc_List_List*_tmp14A;struct _tuple0
_tmp148=((struct _tuple0(*)(struct Cyc_List_List*x))Cyc_List_split)(tagnms);
_tmp149=_tmp148.f1;_tmp14A=_tmp148.f2;{struct Cyc_List_List*_tmp14B=((struct Cyc_List_List*(*)(
struct Cyc_Absyn_Exp*(*f)(struct Cyc_Position_Segment*,struct _tuple6*),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_cnst_qvar_string_cls,loc,
_tmp14A);struct Cyc_List_List*_tmp14C=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,int),struct Cyc_Position_Segment*env,struct Cyc_List_List*
x))Cyc_List_map_c)(Cyc_Tcgenrep_cnst_int_cls,loc,_tmp149);struct Cyc_List_List*
_tmp14D=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct Cyc_Position_Segment*,
struct _tuple11*),struct Cyc_Position_Segment*env,struct Cyc_List_List*x))Cyc_List_map_c)(
Cyc_Tcgenrep_tuple2_exp_cls,loc,((struct Cyc_List_List*(*)(struct Cyc_List_List*x,
struct Cyc_List_List*y))Cyc_List_zip)(_tmp14C,_tmp14B));void*_tmp14E=Cyc_Tcgenrep_tuple_typ(({
void*_tmp159[2];_tmp159[1]=Cyc_Absyn_const_string_typ((void*)2);_tmp159[0]=Cyc_Absyn_uint_t;
Cyc_List_list(_tag_arr(_tmp159,sizeof(void*),2));}));struct _tuple14 _tmp150;
struct Cyc_List_List*_tmp151;struct Cyc_Absyn_Exp*_tmp152;struct _tuple14*_tmp14F=
Cyc_Tcgenrep_array_decls(_tmp14E,_tmp14D,loc);_tmp150=*_tmp14F;_tmp151=_tmp150.f1;
_tmp152=_tmp150.f2;{struct Cyc_Absyn_Exp*name;if(ename == 0)name=Cyc_Absyn_null_exp(
loc);else{struct _tagged_arr*_tmp153=Cyc_Tcgenrep_new_gen_id(_tag_arr("name",
sizeof(unsigned char),5));struct Cyc_Absyn_Decl*_tmp154=Cyc_Tcgenrep_gen_decl(
_tmp153,Cyc_Absyn_const_string_typ((void*)2),(struct Cyc_Absyn_Exp*)Cyc_Tcgenrep_cnst_string(*(*(*
ename)).f2,loc),loc);_tmp151=({struct Cyc_List_List*_tmp155=_cycalloc(sizeof(*
_tmp155));_tmp155->hd=_tmp154;_tmp155->tl=_tmp151;_tmp155;});name=Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(Cyc_Tcgenrep_toplevel_name(_tmp153),loc),loc);}{struct
Cyc_Absyn_Decl*_tmp156=Cyc_Tcgenrep_tunion_constructor_decl(Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_enum_str),
varname,({struct Cyc_Absyn_Exp*_tmp158[3];_tmp158[2]=_tmp152;_tmp158[1]=_tmp147;
_tmp158[0]=name;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmp158,sizeof(struct Cyc_Absyn_Exp*),3));}),sc,loc);return Cyc_Tcgenrep_update_info(
dict,type,((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_imp_append)(_tmp151,({struct Cyc_Absyn_Decl*_tmp157[1];_tmp157[0]=
_tmp156;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(
_tmp157,sizeof(struct Cyc_Absyn_Decl*),1));})),(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(varname,loc),loc),0,0);}}}}static struct Cyc_Dict_Dict*Cyc_Tcgenrep_buildRepTunion(
struct _tuple6*tname,struct Cyc_Tcenv_Tenv*te,struct Cyc_Dict_Dict*dict,struct Cyc_Position_Segment*
loc,struct _tuple6*varname,void*sc,void*type,struct Cyc_List_List*tonames,struct
Cyc_List_List*nmtypes){struct Cyc_List_List*_tmp15A=((struct Cyc_List_List*(*)(int
n,int(*f)(int)))Cyc_List_tabulate)(((int(*)(struct Cyc_List_List*x))Cyc_List_length)(
tonames),(int(*)(int))Cyc_Core_identity);struct Cyc_List_List*_tmp15B=((struct Cyc_List_List*(*)(
struct Cyc_Absyn_Exp*(*f)(struct Cyc_Position_Segment*,int),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_cnst_int_cls,loc,_tmp15A);
struct Cyc_List_List*_tmp15C=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(
struct Cyc_Position_Segment*,struct _tuple6*),struct Cyc_Position_Segment*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_cnst_qvar_string_cls,loc,
tonames);struct Cyc_List_List*_tmp15D=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_zip)(_tmp15B,_tmp15C);struct Cyc_List_List*
_tmp15E=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct Cyc_Position_Segment*,
struct _tuple11*),struct Cyc_Position_Segment*env,struct Cyc_List_List*x))Cyc_List_map_c)(
Cyc_Tcgenrep_tuple2_exp_cls,loc,_tmp15D);void*_tmp15F=Cyc_Tcgenrep_tuple_typ(({
void*_tmp17E[2];_tmp17E[1]=Cyc_Absyn_const_string_typ((void*)2);_tmp17E[0]=Cyc_Absyn_uint_t;
Cyc_List_list(_tag_arr(_tmp17E,sizeof(void*),2));}));struct _tuple14 _tmp161;
struct Cyc_List_List*_tmp162;struct Cyc_Absyn_Exp*_tmp163;struct _tuple14*_tmp160=
Cyc_Tcgenrep_array_decls(_tmp15F,_tmp15E,loc);_tmp161=*_tmp160;_tmp162=_tmp161.f1;
_tmp163=_tmp161.f2;{struct _tuple20*_tmp164=({struct _tuple20*_tmp17D=_cycalloc(
sizeof(*_tmp17D));_tmp17D->f1=te;_tmp17D->f2=loc;_tmp17D;});struct _tuple19*
_tmp165=({struct _tuple19*_tmp17C=_cycalloc(sizeof(*_tmp17C));_tmp17C->f1=dict;
_tmp17C->f2=0;_tmp17C;});struct Cyc_List_List*_tmp167;struct Cyc_List_List*_tmp168;
struct _tuple0 _tmp166=((struct _tuple0(*)(struct Cyc_List_List*x))Cyc_List_split)(
nmtypes);_tmp167=_tmp166.f1;_tmp168=_tmp166.f2;{struct _tuple19 _tmp16A;struct Cyc_Dict_Dict*
_tmp16B;struct Cyc_List_List*_tmp16C;struct _tuple19*_tmp169=((struct _tuple19*(*)(
struct _tuple19*(*f)(struct _tuple20*,void*,struct _tuple19*),struct _tuple20*,
struct Cyc_List_List*x,struct _tuple19*accum))Cyc_List_fold_right_c)(Cyc_Tcgenrep_lookupRepsCls,
_tmp164,_tmp168,_tmp165);_tmp16A=*_tmp169;_tmp16B=_tmp16A.f1;_tmp16C=_tmp16A.f2;
dict=_tmp16B;{struct Cyc_List_List*_tmp16D=((struct Cyc_List_List*(*)(int n,int(*f)(
int)))Cyc_List_tabulate)(((int(*)(struct Cyc_List_List*x))Cyc_List_length)(
_tmp16C),(int(*)(int))Cyc_Core_identity);struct Cyc_List_List*_tmp16E=((struct Cyc_List_List*(*)(
struct Cyc_Absyn_Exp*(*f)(struct Cyc_Position_Segment*,int),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_cnst_int_cls,loc,_tmp16D);
struct Cyc_List_List*_tmp16F=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(
struct Cyc_Position_Segment*,struct _tuple6*),struct Cyc_Position_Segment*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_cnst_qvar_string_cls,loc,
_tmp167);struct Cyc_List_List*_tmp170=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct Cyc_Tcgenrep_RepInfo*),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_get_and_cast_ri_exp,loc,
_tmp16C);struct Cyc_List_List*_tmp171=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y,struct Cyc_List_List*z))Cyc_List_zip3)(_tmp16E,_tmp16F,
_tmp170);struct Cyc_List_List*_tmp172=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct _tuple12*),struct Cyc_Position_Segment*env,
struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_tuple3_exp_cls,loc,_tmp171);
void*tuple_type2=Cyc_Tcgenrep_tuple_typ(({void*_tmp17B[3];_tmp17B[2]=Cyc_Tcgenrep_tunion_typ(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str));_tmp17B[1]=Cyc_Absyn_const_string_typ((
void*)2);_tmp17B[0]=Cyc_Absyn_uint_t;Cyc_List_list(_tag_arr(_tmp17B,sizeof(void*),
3));}));struct _tuple14 _tmp174;struct Cyc_List_List*_tmp175;struct Cyc_Absyn_Exp*
_tmp176;struct _tuple14*_tmp173=Cyc_Tcgenrep_array_decls(tuple_type2,_tmp172,loc);
_tmp174=*_tmp173;_tmp175=_tmp174.f1;_tmp176=_tmp174.f2;{struct Cyc_Absyn_Exp*
_tmp177=Cyc_Tcgenrep_cnst_string(*(*tname).f2,loc);struct Cyc_Absyn_Decl*_tmp178=
Cyc_Tcgenrep_tunion_constructor_decl(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_tunion_str),varname,({struct Cyc_Absyn_Exp*
_tmp17A[3];_tmp17A[2]=_tmp176;_tmp17A[1]=_tmp163;_tmp17A[0]=_tmp177;((struct Cyc_List_List*(*)(
struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp17A,sizeof(struct Cyc_Absyn_Exp*),
3));}),sc,loc);return Cyc_Tcgenrep_update_info(dict,type,((struct Cyc_List_List*(*)(
struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(_tmp162,((
struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))Cyc_List_imp_append)(
_tmp175,({struct Cyc_Absyn_Decl*_tmp179[1];_tmp179[0]=_tmp178;((struct Cyc_List_List*(*)(
struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp179,sizeof(struct Cyc_Absyn_Decl*),
1));}))),(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
varname,loc),loc),_tmp16C,0);return dict;}}}}}static struct Cyc_Dict_Dict*Cyc_Tcgenrep_buildRepXTunion(
struct _tuple6*xname,struct Cyc_Tcenv_Tenv*te,struct Cyc_Dict_Dict*dict,struct Cyc_Position_Segment*
loc,struct _tuple6*varname,void*sc,void*type,struct Cyc_List_List*nametypes){
struct Cyc_List_List*_tmp180;struct Cyc_List_List*_tmp181;struct _tuple0 _tmp17F=((
struct _tuple0(*)(struct Cyc_List_List*x))Cyc_List_split)(nametypes);_tmp180=
_tmp17F.f1;_tmp181=_tmp17F.f2;{struct _tuple20*_tmp182=({struct _tuple20*_tmp197=
_cycalloc(sizeof(*_tmp197));_tmp197->f1=te;_tmp197->f2=loc;_tmp197;});struct
_tuple19*_tmp183=({struct _tuple19*_tmp196=_cycalloc(sizeof(*_tmp196));_tmp196->f1=
dict;_tmp196->f2=0;_tmp196;});struct _tuple19 _tmp185;struct Cyc_Dict_Dict*_tmp186;
struct Cyc_List_List*_tmp187;struct _tuple19*_tmp184=((struct _tuple19*(*)(struct
_tuple19*(*f)(struct _tuple20*,void*,struct _tuple19*),struct _tuple20*,struct Cyc_List_List*
x,struct _tuple19*accum))Cyc_List_fold_right_c)(Cyc_Tcgenrep_lookupRepsCls,
_tmp182,_tmp181,_tmp183);_tmp185=*_tmp184;_tmp186=_tmp185.f1;_tmp187=_tmp185.f2;
dict=_tmp186;{struct Cyc_List_List*_tmp188=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct _tagged_arr*),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_cnst_string_cls,loc,
_tmp180);struct Cyc_List_List*_tmp189=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*
f)(struct Cyc_Position_Segment*,struct Cyc_Tcgenrep_RepInfo*),struct Cyc_Position_Segment*
env,struct Cyc_List_List*x))Cyc_List_map_c)(Cyc_Tcgenrep_get_and_cast_ri_exp,loc,
_tmp187);struct Cyc_List_List*_tmp18A=((struct Cyc_List_List*(*)(struct Cyc_List_List*
x,struct Cyc_List_List*y))Cyc_List_zip)(_tmp188,_tmp189);struct Cyc_List_List*
_tmp18B=((struct Cyc_List_List*(*)(struct Cyc_Absyn_Exp*(*f)(struct Cyc_Position_Segment*,
struct _tuple11*),struct Cyc_Position_Segment*env,struct Cyc_List_List*x))Cyc_List_map_c)(
Cyc_Tcgenrep_tuple2_exp_cls,loc,_tmp18A);void*name_type=Cyc_Absyn_tagged_typ(Cyc_Absyn_uchar_t,(
void*)2,({struct Cyc_Absyn_Tqual _tmp195;_tmp195.q_const=1;_tmp195.q_volatile=0;
_tmp195.q_restrict=0;_tmp195;}));void*tuple_type=Cyc_Tcgenrep_tuple_typ(({void*
_tmp194[2];_tmp194[1]=Cyc_Tcgenrep_tunion_typ(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str));
_tmp194[0]=name_type;Cyc_List_list(_tag_arr(_tmp194,sizeof(void*),2));}));struct
_tuple14 _tmp18D;struct Cyc_List_List*_tmp18E;struct Cyc_Absyn_Exp*_tmp18F;struct
_tuple14*_tmp18C=Cyc_Tcgenrep_array_decls(tuple_type,_tmp18B,loc);_tmp18D=*
_tmp18C;_tmp18E=_tmp18D.f1;_tmp18F=_tmp18D.f2;{struct Cyc_Absyn_Exp*_tmp190=Cyc_Tcgenrep_cnst_string(*(*
xname).f2,loc);struct Cyc_Absyn_Decl*_tmp191=Cyc_Tcgenrep_tunion_constructor_decl(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_xtunion_str),varname,({struct Cyc_Absyn_Exp*_tmp193[2];_tmp193[1]=
_tmp18F;_tmp193[0]=_tmp190;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmp193,sizeof(struct Cyc_Absyn_Exp*),2));}),sc,loc);return Cyc_Tcgenrep_update_info(
dict,type,((struct Cyc_List_List*(*)(struct Cyc_List_List*x,struct Cyc_List_List*y))
Cyc_List_imp_append)(_tmp18E,({struct Cyc_Absyn_Decl*_tmp192[1];_tmp192[0]=
_tmp191;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(
_tmp192,sizeof(struct Cyc_Absyn_Decl*),1));})),(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(varname,loc),loc),_tmp187,0);return dict;}}}}static struct
_tuple18*Cyc_Tcgenrep_lookupRep(struct Cyc_Tcenv_Tenv*te,struct Cyc_Dict_Dict*dict,
struct Cyc_Position_Segment*loc,void*type){struct Cyc_Core_Opt*info=((struct Cyc_Core_Opt*(*)(
struct Cyc_Dict_Dict*d,void*k))Cyc_Dict_lookup_opt)(dict,type);if(info != 0){if(((
struct Cyc_Tcgenrep_RepInfo*)info->v)->is_extern)dict=((struct Cyc_Dict_Dict*(*)(
struct Cyc_Dict_Dict*,void*))Cyc_Dict_delete)(dict,type);else{struct _tuple18*
_tmp198=({struct _tuple18*_tmp199=_cycalloc(sizeof(*_tmp199));_tmp199->f1=dict;
_tmp199->f2=(struct Cyc_Tcgenrep_RepInfo*)info->v;_tmp199;});return _tmp198;}}{
void*_tmp19A=Cyc_Tcutil_compress(Cyc_Tcgenrep_monomorphize_type(type));void*
_tmp19B;void*_tmp19C;int _tmp19D;struct Cyc_Absyn_PtrInfo _tmp19E;void*_tmp19F;
struct Cyc_Absyn_Tqual _tmp1A0;struct Cyc_Absyn_Exp*_tmp1A1;struct Cyc_List_List*
_tmp1A2;struct _tuple6*_tmp1A3;struct Cyc_List_List*_tmp1A4;struct Cyc_Core_Opt*
_tmp1A5;struct Cyc_Absyn_Tvar*_tmp1A6;struct Cyc_Absyn_FnInfo _tmp1A7;struct Cyc_Absyn_TunionInfo
_tmp1A8;void*_tmp1A9;struct Cyc_Absyn_Tuniondecl**_tmp1AA;struct Cyc_Absyn_Tuniondecl*
_tmp1AB;struct Cyc_List_List*_tmp1AC;void*_tmp1AD;struct Cyc_Absyn_TunionFieldInfo
_tmp1AE;struct Cyc_Absyn_AggrInfo _tmp1AF;void*_tmp1B0;struct Cyc_List_List*_tmp1B1;
void*_tmp1B2;struct Cyc_List_List*_tmp1B3;void*_tmp1B4;struct Cyc_List_List*
_tmp1B5;struct Cyc_Absyn_Enumdecl*_tmp1B6;struct Cyc_List_List*_tmp1B7;_LL16: if(
_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 5)goto _LL18;_tmp19B=(void*)((struct Cyc_Absyn_IntType_struct*)
_tmp19A)->f1;_tmp19C=(void*)((struct Cyc_Absyn_IntType_struct*)_tmp19A)->f2;_LL17: {
struct _tuple6*_tmp1B8=Cyc_Tcgenrep_toplevel_name(Cyc_Tcgenrep_new_gen_id(
_tag_arr("rep",sizeof(unsigned char),4)));struct Cyc_Absyn_Decl*_tmp1B9=Cyc_Tcgenrep_tunion_constructor_decl(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_int_str),_tmp1B8,({struct Cyc_Absyn_Exp*_tmp1BB[2];_tmp1BB[1]=Cyc_Tcgenrep_cnst_int(
Cyc_Tcgenrep_size_of2int(_tmp19C),loc);_tmp1BB[0]=Cyc_Tcgenrep_cnst_int(_tmp19B
== (void*)0?1: 0,loc);((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmp1BB,sizeof(struct Cyc_Absyn_Exp*),2));}),(void*)0,loc);dict=Cyc_Tcgenrep_update_info(
dict,type,({struct Cyc_Absyn_Decl*_tmp1BA[1];_tmp1BA[0]=_tmp1B9;((struct Cyc_List_List*(*)(
struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp1BA,sizeof(struct Cyc_Absyn_Decl*),
1));}),(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
_tmp1B8,loc),loc),0,0);goto _LL15;}_LL18: if((int)_tmp19A != 1)goto _LL1A;_LL19: dict=
Cyc_Tcgenrep_update_info(dict,type,0,(struct Cyc_Absyn_Exp*)Cyc_Absyn_unknownid_exp(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_float_str),loc),0,0);goto _LL15;_LL1A: if(
_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 6)goto _LL1C;_tmp19D=((struct Cyc_Absyn_DoubleType_struct*)
_tmp19A)->f1;_LL1B: dict=Cyc_Tcgenrep_update_info(dict,type,0,(struct Cyc_Absyn_Exp*)
Cyc_Absyn_unknownid_exp(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_double_str),loc),
0,0);goto _LL15;_LL1C: if(_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 4)goto _LL1E;
_tmp19E=((struct Cyc_Absyn_PointerType_struct*)_tmp19A)->f1;_LL1D:{void*_tmp1BC=
Cyc_Absyn_conref_val(_tmp19E.bounds);struct Cyc_Absyn_Exp*_tmp1BD;_LL49: if(
_tmp1BC <= (void*)1?1:*((int*)_tmp1BC)!= 0)goto _LL4B;_tmp1BD=((struct Cyc_Absyn_Upper_b_struct*)
_tmp1BC)->f1;_LL4A: {struct _tuple6*_tmp1BE=Cyc_Tcgenrep_toplevel_name(Cyc_Tcgenrep_new_gen_id(
_tag_arr("rep",sizeof(unsigned char),4)));struct Cyc_Core_Opt*_tmp1BF=({struct Cyc_Core_Opt*
_tmp1C8=_cycalloc(sizeof(*_tmp1C8));_tmp1C8->v=Cyc_Tcgenrep_gen_vardecl(_tmp1BE,
Cyc_Tcgenrep_tunionfield_typ(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_thinptr_str)),0,(void*)3,loc);_tmp1C8;});
dict=Cyc_Tcgenrep_make_fwd_decl_info(dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(_tmp1BE,loc),loc),_tmp1BF,0);{struct _tuple18 _tmp1C1;
struct Cyc_Dict_Dict*_tmp1C2;struct Cyc_Tcgenrep_RepInfo*_tmp1C3;struct _tuple18*
_tmp1C0=Cyc_Tcgenrep_lookupRep(te,dict,loc,(void*)_tmp19E.elt_typ);_tmp1C1=*
_tmp1C0;_tmp1C2=_tmp1C1.f1;_tmp1C3=_tmp1C1.f2;dict=_tmp1C2;{unsigned int bound=
Cyc_Evexp_eval_const_uint_exp(_tmp1BD);struct Cyc_Absyn_Decl*_tmp1C4=Cyc_Tcgenrep_tunion_constructor_decl(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_thinptr_str),_tmp1BE,({struct Cyc_Absyn_Exp*_tmp1C7[2];_tmp1C7[1]=(
struct Cyc_Absyn_Exp*)_check_null(_tmp1C3->exp);_tmp1C7[0]=Cyc_Tcgenrep_cnst_int((
int)bound,loc);((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmp1C7,sizeof(struct Cyc_Absyn_Exp*),2));}),(void*)0,loc);dict=Cyc_Tcgenrep_update_info(
dict,type,({struct Cyc_Absyn_Decl*_tmp1C5[1];_tmp1C5[0]=_tmp1C4;((struct Cyc_List_List*(*)(
struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp1C5,sizeof(struct Cyc_Absyn_Decl*),
1));}),(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
_tmp1BE,loc),loc),({struct Cyc_Tcgenrep_RepInfo*_tmp1C6[1];_tmp1C6[0]=_tmp1C3;((
struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp1C6,
sizeof(struct Cyc_Tcgenrep_RepInfo*),1));}),0);goto _LL48;}}}_LL4B: if((int)_tmp1BC
!= 0)goto _LL48;_LL4C: {struct _tuple6*_tmp1C9=Cyc_Tcgenrep_toplevel_name(Cyc_Tcgenrep_new_gen_id(
_tag_arr("rep",sizeof(unsigned char),4)));struct Cyc_Core_Opt*_tmp1CA=({struct Cyc_Core_Opt*
_tmp1D3=_cycalloc(sizeof(*_tmp1D3));_tmp1D3->v=Cyc_Tcgenrep_gen_vardecl(_tmp1C9,
Cyc_Tcgenrep_tunionfield_typ(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_fatptr_str)),0,(void*)3,loc);_tmp1D3;});
dict=Cyc_Tcgenrep_make_fwd_decl_info(dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(_tmp1C9,loc),loc),_tmp1CA,0);{struct _tuple18 _tmp1CC;
struct Cyc_Dict_Dict*_tmp1CD;struct Cyc_Tcgenrep_RepInfo*_tmp1CE;struct _tuple18*
_tmp1CB=Cyc_Tcgenrep_lookupRep(te,dict,loc,(void*)_tmp19E.elt_typ);_tmp1CC=*
_tmp1CB;_tmp1CD=_tmp1CC.f1;_tmp1CE=_tmp1CC.f2;dict=_tmp1CD;{struct Cyc_Absyn_Decl*
_tmp1CF=Cyc_Tcgenrep_tunion_constructor_decl(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_fatptr_str),_tmp1C9,({struct Cyc_Absyn_Exp*
_tmp1D2[1];_tmp1D2[0]=(struct Cyc_Absyn_Exp*)_check_null(_tmp1CE->exp);((struct
Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp1D2,sizeof(
struct Cyc_Absyn_Exp*),1));}),(void*)0,loc);dict=Cyc_Tcgenrep_update_info(dict,
type,({struct Cyc_Absyn_Decl*_tmp1D0[1];_tmp1D0[0]=_tmp1CF;((struct Cyc_List_List*(*)(
struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp1D0,sizeof(struct Cyc_Absyn_Decl*),
1));}),(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
_tmp1C9,loc),loc),({struct Cyc_Tcgenrep_RepInfo*_tmp1D1[1];_tmp1D1[0]=_tmp1CE;((
struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp1D1,
sizeof(struct Cyc_Tcgenrep_RepInfo*),1));}),0);goto _LL48;}}}_LL48:;}goto _LL15;
_LL1E: if(_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 7)goto _LL20;_tmp19F=(void*)((
struct Cyc_Absyn_ArrayType_struct*)_tmp19A)->f1;_tmp1A0=((struct Cyc_Absyn_ArrayType_struct*)
_tmp19A)->f2;_tmp1A1=((struct Cyc_Absyn_ArrayType_struct*)_tmp19A)->f3;_LL1F: if(
_tmp1A1 == 0)({void*_tmp1D4[0]={};((int(*)(struct _tagged_arr fmt,struct _tagged_arr
ap))Cyc_Tcutil_impos)(_tag_arr("tcgenrep:At this point, array bounds must be constants",
sizeof(unsigned char),55),_tag_arr(_tmp1D4,sizeof(void*),0));});{struct _tuple18
_tmp1D6;struct Cyc_Dict_Dict*_tmp1D7;struct Cyc_Tcgenrep_RepInfo*_tmp1D8;struct
_tuple18*_tmp1D5=Cyc_Tcgenrep_lookupRep(te,dict,loc,_tmp19F);_tmp1D6=*_tmp1D5;
_tmp1D7=_tmp1D6.f1;_tmp1D8=_tmp1D6.f2;dict=_tmp1D7;{unsigned int bound=Cyc_Evexp_eval_const_uint_exp((
struct Cyc_Absyn_Exp*)_check_null(_tmp1A1));struct _tuple6*_tmp1D9=Cyc_Tcgenrep_toplevel_name(
Cyc_Tcgenrep_new_gen_id(_tag_arr("rep",sizeof(unsigned char),4)));struct Cyc_Absyn_Decl*
_tmp1DA=Cyc_Tcgenrep_tunion_constructor_decl(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_thinptr_str),_tmp1D9,({struct Cyc_Absyn_Exp*
_tmp1DD[2];_tmp1DD[1]=(struct Cyc_Absyn_Exp*)_check_null(_tmp1D8->exp);_tmp1DD[0]=
Cyc_Tcgenrep_cnst_int((int)bound,loc);((struct Cyc_List_List*(*)(struct
_tagged_arr))Cyc_List_list)(_tag_arr(_tmp1DD,sizeof(struct Cyc_Absyn_Exp*),2));}),(
void*)0,loc);dict=Cyc_Tcgenrep_update_info(dict,type,({struct Cyc_Absyn_Decl*
_tmp1DB[1];_tmp1DB[0]=_tmp1DA;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmp1DB,sizeof(struct Cyc_Absyn_Decl*),1));}),(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(_tmp1D9,loc),loc),({struct Cyc_Tcgenrep_RepInfo*_tmp1DC[1];
_tmp1DC[0]=_tmp1D8;((struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(
_tag_arr(_tmp1DC,sizeof(struct Cyc_Tcgenrep_RepInfo*),1));}),0);goto _LL15;}}_LL20:
if(_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 9)goto _LL22;_tmp1A2=((struct Cyc_Absyn_TupleType_struct*)
_tmp19A)->f1;_LL21: {struct _tuple6*_tmp1DE=Cyc_Tcgenrep_toplevel_name(Cyc_Tcgenrep_new_gen_id(
_tag_arr("rep",sizeof(unsigned char),4)));struct Cyc_Core_Opt*_tmp1DF=({struct Cyc_Core_Opt*
_tmp1E1=_cycalloc(sizeof(*_tmp1E1));_tmp1E1->v=Cyc_Tcgenrep_gen_vardecl(_tmp1DE,
Cyc_Tcgenrep_tunionfield_typ(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_tuple_str)),0,(void*)3,loc);_tmp1E1;});
dict=Cyc_Tcgenrep_make_fwd_decl_info(dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(_tmp1DE,loc),loc),_tmp1DF,0);{struct Cyc_List_List*
_tmp1E0=((struct Cyc_List_List*(*)(void*(*f)(struct _tuple9*),struct Cyc_List_List*
x))Cyc_List_map)(Cyc_Tcgenrep_get_second,_tmp1A2);dict=Cyc_Tcgenrep_buildRepTuple(
te,dict,loc,_tmp1DE,(void*)0,type,_tmp1E0);goto _LL15;}}_LL22: if(_tmp19A <= (void*)
3?1:*((int*)_tmp19A)!= 16)goto _LL24;_tmp1A3=((struct Cyc_Absyn_TypedefType_struct*)
_tmp19A)->f1;_tmp1A4=((struct Cyc_Absyn_TypedefType_struct*)_tmp19A)->f2;_tmp1A5=((
struct Cyc_Absyn_TypedefType_struct*)_tmp19A)->f3;_LL23: if(_tmp1A5 == 0)({void*
_tmp1E2[0]={};((int(*)(struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(
_tag_arr("gen(): typedef definition must be present by now",sizeof(unsigned char),
49),_tag_arr(_tmp1E2,sizeof(void*),0));});{struct _tuple18 _tmp1E4;struct Cyc_Dict_Dict*
_tmp1E5;struct Cyc_Tcgenrep_RepInfo*_tmp1E6;struct _tuple18*_tmp1E3=Cyc_Tcgenrep_lookupRep(
te,dict,loc,(void*)_tmp1A5->v);_tmp1E4=*_tmp1E3;_tmp1E5=_tmp1E4.f1;_tmp1E6=
_tmp1E4.f2;dict=((struct Cyc_Dict_Dict*(*)(struct Cyc_Dict_Dict*d,void*k,struct Cyc_Tcgenrep_RepInfo*
v))Cyc_Dict_insert)(_tmp1E5,type,_tmp1E6);goto _LL15;}_LL24: if((int)_tmp19A != 0)
goto _LL26;_LL25:({void*_tmp1E7[0]={};Cyc_Tcutil_terr(loc,_tag_arr("found void in gen() expression",
sizeof(unsigned char),31),_tag_arr(_tmp1E7,sizeof(void*),0));});goto _LL15;_LL26:
if(_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 0)goto _LL28;_LL27:({void*_tmp1E8[0]={};
Cyc_Tcutil_terr(loc,_tag_arr("found evar in gen() expression",sizeof(
unsigned char),31),_tag_arr(_tmp1E8,sizeof(void*),0));});goto _LL15;_LL28: if(
_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 1)goto _LL2A;_tmp1A6=((struct Cyc_Absyn_VarType_struct*)
_tmp19A)->f1;_LL29:({void*_tmp1E9[0]={};Cyc_Tcutil_terr(loc,_tag_arr("found tyvar in gen() expression",
sizeof(unsigned char),32),_tag_arr(_tmp1E9,sizeof(void*),0));});goto _LL15;({void*
_tmp1EA[0]={};Cyc_Tcutil_terr(loc,_tag_arr("found variable type in gen() expression",
sizeof(unsigned char),40),_tag_arr(_tmp1EA,sizeof(void*),0));});goto _LL15;_LL2A:
if(_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 8)goto _LL2C;_tmp1A7=((struct Cyc_Absyn_FnType_struct*)
_tmp19A)->f1;_LL2B:({void*_tmp1EB[0]={};Cyc_Tcutil_terr(loc,_tag_arr("found function type in gen() expression",
sizeof(unsigned char),40),_tag_arr(_tmp1EB,sizeof(void*),0));});goto _LL15;_LL2C:
if(_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 15)goto _LL2E;_LL2D: goto _LL2F;_LL2E:
if((int)_tmp19A != 2)goto _LL30;_LL2F: goto _LL31;_LL30: if(_tmp19A <= (void*)3?1:*((
int*)_tmp19A)!= 17)goto _LL32;_LL31: goto _LL33;_LL32: if(_tmp19A <= (void*)3?1:*((
int*)_tmp19A)!= 18)goto _LL34;_LL33: goto _LL35;_LL34: if(_tmp19A <= (void*)3?1:*((
int*)_tmp19A)!= 19)goto _LL36;_LL35:({void*_tmp1EC[0]={};Cyc_Tcutil_terr(loc,
_tag_arr("gen(): unhandled region, handle or effect type",sizeof(unsigned char),
47),_tag_arr(_tmp1EC,sizeof(void*),0));});goto _LL15;_LL36: if(_tmp19A <= (void*)3?
1:*((int*)_tmp19A)!= 2)goto _LL38;_tmp1A8=((struct Cyc_Absyn_TunionType_struct*)
_tmp19A)->f1;_tmp1A9=(void*)_tmp1A8.tunion_info;if(*((int*)_tmp1A9)!= 1)goto
_LL38;_tmp1AA=((struct Cyc_Absyn_KnownTunion_struct*)_tmp1A9)->f1;_tmp1AB=*
_tmp1AA;_tmp1AC=_tmp1A8.targs;_tmp1AD=(void*)_tmp1A8.rgn;_LL37: if(_tmp1AB->tvs != 
0)({void*_tmp1ED[0]={};((int(*)(struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(
_tag_arr("gen(): tunion type variables not handled yet",sizeof(unsigned char),45),
_tag_arr(_tmp1ED,sizeof(void*),0));});if(_tmp1AB->fields == 0){struct _tuple6*
_tmp1EE=Cyc_Tcgenrep_toplevel_name(({struct _tagged_arr*_tmp1F1=_cycalloc(sizeof(*
_tmp1F1));_tmp1F1[0]=Cyc_Tcgenrep_make_type_cstring(type);_tmp1F1;}));struct Cyc_Core_Opt*
_tmp1EF=({struct Cyc_Core_Opt*_tmp1F0=_cycalloc(sizeof(*_tmp1F0));_tmp1F0->v=Cyc_Tcgenrep_gen_vardecl(
_tmp1EE,Cyc_Tcgenrep_tunionfield_typ(Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_tunion_str)),0,(void*)3,loc);_tmp1F0;});
dict=Cyc_Tcgenrep_make_fwd_decl_info(dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(
Cyc_Absyn_unknownid_exp(_tmp1EE,loc),loc),_tmp1EF,1);goto _LL15;}if(!_tmp1AB->is_xtunion){
struct _tuple6*_tmp1F2=Cyc_Tcgenrep_toplevel_name(({struct _tagged_arr*_tmp1FA=
_cycalloc(sizeof(*_tmp1FA));_tmp1FA[0]=Cyc_Tcgenrep_make_type_cstring(type);
_tmp1FA;}));struct Cyc_Core_Opt*_tmp1F3=({struct Cyc_Core_Opt*_tmp1F9=_cycalloc(
sizeof(*_tmp1F9));_tmp1F9->v=Cyc_Tcgenrep_gen_vardecl(_tmp1F2,Cyc_Tcgenrep_tunionfield_typ(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_tunion_str)),0,(void*)3,loc);_tmp1F9;});dict=Cyc_Tcgenrep_make_fwd_decl_info(
dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
_tmp1F2,loc),loc),_tmp1F3,0);{struct Cyc_List_List*_tmp1F4=(struct Cyc_List_List*)((
struct Cyc_Core_Opt*)_check_null(_tmp1AB->fields))->v;struct Cyc_List_List*_tmp1F5=((
struct Cyc_List_List*(*)(int(*f)(struct Cyc_Absyn_Tunionfield*),struct Cyc_List_List*
x))Cyc_List_filter)(Cyc_Tcgenrep_filter_empty_tunionfield,_tmp1F4);struct Cyc_List_List*
_tmp1F6=((struct Cyc_List_List*(*)(int(*f)(struct Cyc_Absyn_Tunionfield*),struct
Cyc_List_List*x))Cyc_List_filter)(Cyc_Tcgenrep_filter_nonempty_tunionfield,
_tmp1F4);struct Cyc_List_List*_tmp1F7=((struct Cyc_List_List*(*)(struct _tuple17*(*
f)(struct Cyc_Absyn_Tunionfield*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcgenrep_check_tunionfield_and_get_nmtype,
_tmp1F5);struct Cyc_List_List*_tmp1F8=((struct Cyc_List_List*(*)(struct _tuple6*(*f)(
struct Cyc_Absyn_Tunionfield*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcgenrep_check_tunionfield_and_get_name,
_tmp1F6);dict=Cyc_Tcgenrep_buildRepTunion(_tmp1AB->name,te,dict,loc,_tmp1F2,(
void*)2,type,_tmp1F8,_tmp1F7);}}else{struct _tuple6*_tmp1FB=Cyc_Tcgenrep_toplevel_name(({
struct _tagged_arr*_tmp201=_cycalloc(sizeof(*_tmp201));_tmp201[0]=Cyc_Tcgenrep_make_type_cstring(
type);_tmp201;}));struct Cyc_Core_Opt*_tmp1FC=({struct Cyc_Core_Opt*_tmp200=
_cycalloc(sizeof(*_tmp200));_tmp200->v=Cyc_Tcgenrep_gen_vardecl(_tmp1FB,Cyc_Tcgenrep_tunionfield_typ(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_xtunion_str)),0,(void*)3,loc);_tmp200;});dict=Cyc_Tcgenrep_make_fwd_decl_info(
dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
_tmp1FB,loc),loc),_tmp1FC,0);{struct Cyc_List_List*_tmp1FD=(struct Cyc_List_List*)((
struct Cyc_Core_Opt*)_check_null(_tmp1AB->fields))->v;struct Cyc_List_List*_tmp1FE=((
struct Cyc_List_List*(*)(int(*f)(struct Cyc_Absyn_Tunionfield*),struct Cyc_List_List*
x))Cyc_List_filter)(Cyc_Tcgenrep_filter_empty_tunionfield,_tmp1FD);struct Cyc_List_List*
_tmp1FF=((struct Cyc_List_List*(*)(struct _tuple15*(*f)(struct Cyc_Absyn_Tunionfield*),
struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcgenrep_check_xtunionfield_and_get_name_type,
_tmp1FE);dict=Cyc_Tcgenrep_buildRepXTunion(_tmp1AB->name,te,dict,loc,_tmp1FB,(
void*)2,type,_tmp1FF);}}goto _LL15;_LL38: if(_tmp19A <= (void*)3?1:*((int*)_tmp19A)
!= 2)goto _LL3A;_LL39:({void*_tmp202[0]={};((int(*)(struct _tagged_arr fmt,struct
_tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen(): tunion must be resolved by now",
sizeof(unsigned char),38),_tag_arr(_tmp202,sizeof(void*),0));});_LL3A: if(_tmp19A
<= (void*)3?1:*((int*)_tmp19A)!= 3)goto _LL3C;_tmp1AE=((struct Cyc_Absyn_TunionFieldType_struct*)
_tmp19A)->f1;_LL3B: if(_tmp1AE.targs != 0)({void*_tmp203[0]={};((int(*)(struct
_tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen(): tunionfield type parameters not handled yet",
sizeof(unsigned char),51),_tag_arr(_tmp203,sizeof(void*),0));});{void*_tmp204=(
void*)_tmp1AE.field_info;struct Cyc_Absyn_Tuniondecl*_tmp205;struct Cyc_Absyn_Tunionfield*
_tmp206;_LL4E: if(*((int*)_tmp204)!= 1)goto _LL50;_tmp205=((struct Cyc_Absyn_KnownTunionfield_struct*)
_tmp204)->f1;_tmp206=((struct Cyc_Absyn_KnownTunionfield_struct*)_tmp204)->f2;
_LL4F: {struct Cyc_List_List*_tmp207=((struct Cyc_List_List*(*)(void*(*f)(struct
_tuple9*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Tcgenrep_get_second,_tmp206->typs);
struct Cyc_List_List*_tmp208=({struct Cyc_List_List*_tmp20A=_cycalloc(sizeof(*
_tmp20A));_tmp20A->hd=(void*)Cyc_Absyn_uint_t;_tmp20A->tl=_tmp207;_tmp20A;});
struct _tuple6*_tmp209=Cyc_Tcgenrep_toplevel_name(Cyc_Tcgenrep_new_gen_id(
_tag_arr("rep",sizeof(unsigned char),4)));dict=Cyc_Tcgenrep_buildRepTunionfield(
_tmp205->name,_tmp206->name,te,dict,loc,_tmp209,(void*)0,type,_tmp208);goto _LL4D;}
_LL50:;_LL51:({void*_tmp20B[0]={};((int(*)(struct _tagged_arr fmt,struct
_tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen(): tunionfield must be known at this point",
sizeof(unsigned char),47),_tag_arr(_tmp20B,sizeof(void*),0));});_LL4D:;}goto
_LL15;_LL3C: if(_tmp19A <= (void*)3?1:*((int*)_tmp19A)!= 10)goto _LL3E;_tmp1AF=((
struct Cyc_Absyn_AggrType_struct*)_tmp19A)->f1;_tmp1B0=(void*)_tmp1AF.aggr_info;
_tmp1B1=_tmp1AF.targs;_LL3D: {struct Cyc_Absyn_Aggrdecl*_tmp20C=Cyc_Absyn_get_known_aggrdecl(
_tmp1B0);if(_tmp20C->exist_vars != 0?(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(_tmp20C->exist_vars))->v != 0: 0)({void*_tmp20D[0]={};((int(*)(struct
_tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen(): existential type variables not yet supported",
sizeof(unsigned char),52),_tag_arr(_tmp20D,sizeof(void*),0));});if((void*)
_tmp20C->kind == (void*)0){struct Cyc_Absyn_Aggrdecl*_tmp20E=_tmp20C;if(_tmp20E->fields
== 0){struct _tuple6*_tmp20F=Cyc_Tcgenrep_toplevel_name(({struct _tagged_arr*
_tmp212=_cycalloc(sizeof(*_tmp212));_tmp212[0]=Cyc_Tcgenrep_make_type_cstring(
type);_tmp212;}));struct Cyc_Core_Opt*_tmp210=({struct Cyc_Core_Opt*_tmp211=
_cycalloc(sizeof(*_tmp211));_tmp211->v=Cyc_Tcgenrep_gen_vardecl(_tmp20F,Cyc_Tcgenrep_tunionfield_typ(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_struct_str)),0,(void*)3,loc);_tmp211;});dict=Cyc_Tcgenrep_make_fwd_decl_info(
dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
_tmp20F,loc),loc),_tmp210,1);goto _LL15;}if((struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(_tmp20E->fields))->v != 0?((int(*)(int(*pred)(struct Cyc_Absyn_Aggrfield*),
struct Cyc_List_List*x))Cyc_List_forall)(Cyc_Tcgenrep_has_bitfield,(struct Cyc_List_List*)((
struct Cyc_Core_Opt*)_check_null(_tmp20E->fields))->v): 0){int _tmp213=((int(*)(int(*
f)(int,struct Cyc_Absyn_Aggrfield*),int accum,struct Cyc_List_List*x))Cyc_List_fold_left)(
Cyc_Tcgenrep_add_bitfield_sizes,0,(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(_tmp20E->fields))->v);int _tmp214=_tmp213 / 8 + (_tmp213 % 8 == 0?0: 1);
struct Cyc_List_List*chars=0;{int i=0;for(0;i < _tmp214;i ++){chars=({struct Cyc_List_List*
_tmp215=_cycalloc(sizeof(*_tmp215));_tmp215->hd=({struct _tuple9*_tmp216=
_cycalloc(sizeof(*_tmp216));_tmp216->f1=Cyc_Tcgenrep_tq_none;_tmp216->f2=Cyc_Absyn_uchar_t;
_tmp216;});_tmp215->tl=chars;_tmp215;});}}{void*base_type=(void*)({struct Cyc_Absyn_TupleType_struct*
_tmp21B=_cycalloc(sizeof(*_tmp21B));_tmp21B[0]=({struct Cyc_Absyn_TupleType_struct
_tmp21C;_tmp21C.tag=9;_tmp21C.f1=chars;_tmp21C;});_tmp21B;});struct _tuple18
_tmp218;struct Cyc_Dict_Dict*_tmp219;struct Cyc_Tcgenrep_RepInfo*_tmp21A;struct
_tuple18*_tmp217=Cyc_Tcgenrep_lookupRep(te,dict,loc,base_type);_tmp218=*_tmp217;
_tmp219=_tmp218.f1;_tmp21A=_tmp218.f2;dict=((struct Cyc_Dict_Dict*(*)(struct Cyc_Dict_Dict*
d,void*k,struct Cyc_Tcgenrep_RepInfo*v))Cyc_Dict_insert)(_tmp219,type,_tmp21A);}}
else{struct _tuple6*_tmp21D=Cyc_Tcgenrep_toplevel_name(({struct _tagged_arr*
_tmp222=_cycalloc(sizeof(*_tmp222));_tmp222[0]=Cyc_Tcgenrep_make_type_cstring(
type);_tmp222;}));struct Cyc_Core_Opt*_tmp21E=({struct Cyc_Core_Opt*_tmp221=
_cycalloc(sizeof(*_tmp221));_tmp221->v=Cyc_Tcgenrep_gen_vardecl(_tmp21D,Cyc_Tcgenrep_tunionfield_typ(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_struct_str)),0,(void*)3,loc);_tmp221;});dict=Cyc_Tcgenrep_make_fwd_decl_info(
dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
_tmp21D,loc),loc),_tmp21E,0);{struct Cyc_List_List*_tmp21F=((struct Cyc_List_List*(*)(
struct _tuple15*(*f)(struct Cyc_Absyn_Aggrfield*),struct Cyc_List_List*x))Cyc_List_map)(
Cyc_Tcgenrep_select_structfield_nmtype,(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(_tmp20E->fields))->v);dict=Cyc_Tcgenrep_buildRepStruct(({struct Cyc_Core_Opt*
_tmp220=_cycalloc(sizeof(*_tmp220));_tmp220->v=_tmp20E->name;_tmp220;}),te,dict,
loc,_tmp21D,(void*)2,type,_tmp21F);}}}else{struct Cyc_Absyn_Aggrdecl*_tmp223=
_tmp20C;if(_tmp223->tvs != 0)({void*_tmp224[0]={};((int(*)(struct _tagged_arr fmt,
struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen: unions with parameters not yet supported",
sizeof(unsigned char),46),_tag_arr(_tmp224,sizeof(void*),0));});if(_tmp223->fields
== 0){struct _tuple6*_tmp225=Cyc_Tcgenrep_toplevel_name(({struct _tagged_arr*
_tmp228=_cycalloc(sizeof(*_tmp228));_tmp228[0]=Cyc_Tcgenrep_make_type_cstring(
type);_tmp228;}));struct Cyc_Core_Opt*_tmp226=({struct Cyc_Core_Opt*_tmp227=
_cycalloc(sizeof(*_tmp227));_tmp227->v=Cyc_Tcgenrep_gen_vardecl(_tmp225,Cyc_Tcgenrep_tunionfield_typ(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_union_str)),0,(void*)3,loc);_tmp227;});dict=Cyc_Tcgenrep_make_fwd_decl_info(
dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
_tmp225,loc),loc),_tmp226,1);goto _LL15;}{struct Cyc_List_List*_tmp229=((struct Cyc_List_List*(*)(
struct _tuple15*(*f)(struct Cyc_Absyn_Aggrfield*),struct Cyc_List_List*x))Cyc_List_map)(
Cyc_Tcgenrep_select_structfield_nmtype,(struct Cyc_List_List*)((struct Cyc_Core_Opt*)
_check_null(_tmp223->fields))->v);struct _tuple6*_tmp22A=Cyc_Tcgenrep_toplevel_name(({
struct _tagged_arr*_tmp22C=_cycalloc(sizeof(*_tmp22C));_tmp22C[0]=Cyc_Tcgenrep_make_type_cstring(
type);_tmp22C;}));dict=Cyc_Tcgenrep_buildRepUnion(({struct Cyc_Core_Opt*_tmp22B=
_cycalloc(sizeof(*_tmp22B));_tmp22B->v=_tmp223->name;_tmp22B;}),te,dict,loc,
_tmp22A,(void*)2,type,_tmp229);}}goto _LL15;}_LL3E: if(_tmp19A <= (void*)3?1:*((int*)
_tmp19A)!= 11)goto _LL40;_tmp1B2=(void*)((struct Cyc_Absyn_AnonAggrType_struct*)
_tmp19A)->f1;if((int)_tmp1B2 != 0)goto _LL40;_tmp1B3=((struct Cyc_Absyn_AnonAggrType_struct*)
_tmp19A)->f2;_LL3F: if(_tmp1B3 != 0?((int(*)(int(*pred)(struct Cyc_Absyn_Aggrfield*),
struct Cyc_List_List*x))Cyc_List_forall)(Cyc_Tcgenrep_has_bitfield,_tmp1B3): 0){
int _tmp22D=((int(*)(int(*f)(int,struct Cyc_Absyn_Aggrfield*),int accum,struct Cyc_List_List*
x))Cyc_List_fold_left)(Cyc_Tcgenrep_add_bitfield_sizes,0,_tmp1B3);int _tmp22E=
_tmp22D / 8 + (_tmp22D % 8 == 0?0: 1);void*base_type=(void*)({struct Cyc_Absyn_ArrayType_struct*
_tmp233=_cycalloc(sizeof(*_tmp233));_tmp233[0]=({struct Cyc_Absyn_ArrayType_struct
_tmp234;_tmp234.tag=7;_tmp234.f1=(void*)Cyc_Absyn_uchar_t;_tmp234.f2=Cyc_Tcgenrep_tq_none;
_tmp234.f3=(struct Cyc_Absyn_Exp*)Cyc_Tcgenrep_cnst_int(_tmp22E,loc);_tmp234;});
_tmp233;});struct _tuple18 _tmp230;struct Cyc_Dict_Dict*_tmp231;struct Cyc_Tcgenrep_RepInfo*
_tmp232;struct _tuple18*_tmp22F=Cyc_Tcgenrep_lookupRep(te,dict,loc,base_type);
_tmp230=*_tmp22F;_tmp231=_tmp230.f1;_tmp232=_tmp230.f2;dict=((struct Cyc_Dict_Dict*(*)(
struct Cyc_Dict_Dict*d,void*k,struct Cyc_Tcgenrep_RepInfo*v))Cyc_Dict_insert)(
_tmp231,type,_tmp232);}else{struct Cyc_List_List*_tmp235=((struct Cyc_List_List*(*)(
struct _tuple15*(*f)(struct Cyc_Absyn_Aggrfield*),struct Cyc_List_List*x))Cyc_List_map)(
Cyc_Tcgenrep_select_structfield_nmtype,_tmp1B3);struct _tuple6*_tmp236=Cyc_Tcgenrep_toplevel_name(
Cyc_Tcgenrep_new_gen_id(_tag_arr("rep",sizeof(unsigned char),4)));dict=Cyc_Tcgenrep_buildRepStruct(
0,te,dict,loc,_tmp236,(void*)0,type,_tmp235);}goto _LL15;_LL40: if(_tmp19A <= (void*)
3?1:*((int*)_tmp19A)!= 11)goto _LL42;_tmp1B4=(void*)((struct Cyc_Absyn_AnonAggrType_struct*)
_tmp19A)->f1;if((int)_tmp1B4 != 1)goto _LL42;_tmp1B5=((struct Cyc_Absyn_AnonAggrType_struct*)
_tmp19A)->f2;_LL41: {struct Cyc_List_List*_tmp237=((struct Cyc_List_List*(*)(
struct _tuple15*(*f)(struct Cyc_Absyn_Aggrfield*),struct Cyc_List_List*x))Cyc_List_map)(
Cyc_Tcgenrep_select_structfield_nmtype,_tmp1B5);struct _tuple6*_tmp238=Cyc_Tcgenrep_toplevel_name(
Cyc_Tcgenrep_new_gen_id(_tag_arr("rep",sizeof(unsigned char),4)));dict=Cyc_Tcgenrep_buildRepUnion(
0,te,dict,loc,_tmp238,(void*)0,type,_tmp237);goto _LL15;}_LL42: if(_tmp19A <= (void*)
3?1:*((int*)_tmp19A)!= 12)goto _LL44;_tmp1B6=((struct Cyc_Absyn_EnumType_struct*)
_tmp19A)->f2;_LL43: if(_tmp1B6 == 0)({void*_tmp239[0]={};((int(*)(struct
_tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen(): enum declaration must be present by now",
sizeof(unsigned char),47),_tag_arr(_tmp239,sizeof(void*),0));});{struct Cyc_Absyn_Enumdecl
_tmp23A=*_tmp1B6;if(_tmp23A.fields == 0){struct _tuple6*_tmp23B=Cyc_Tcgenrep_toplevel_name(({
struct _tagged_arr*_tmp23E=_cycalloc(sizeof(*_tmp23E));_tmp23E[0]=Cyc_Tcgenrep_make_type_cstring(
type);_tmp23E;}));struct Cyc_Core_Opt*_tmp23C=({struct Cyc_Core_Opt*_tmp23D=
_cycalloc(sizeof(*_tmp23D));_tmp23D->v=Cyc_Tcgenrep_gen_vardecl(_tmp23B,Cyc_Tcgenrep_tunionfield_typ(
Cyc_Tcgenrep_typerep_name(& Cyc_Tcgenrep_typestruct_str),Cyc_Tcgenrep_typerep_name(&
Cyc_Tcgenrep_enum_str)),0,(void*)3,loc);_tmp23D;});dict=Cyc_Tcgenrep_make_fwd_decl_info(
dict,type,(struct Cyc_Absyn_Exp*)Cyc_Absyn_address_exp(Cyc_Absyn_unknownid_exp(
_tmp23B,loc),loc),_tmp23C,1);goto _LL15;}{struct Cyc_List_List*_tmp23F=((struct Cyc_List_List*(*)(
struct _tuple16*(*f)(struct Cyc_Absyn_Enumfield*),struct Cyc_List_List*x))Cyc_List_map)(
Cyc_Tcgenrep_select_enumfield_tagnm,(struct Cyc_List_List*)(_tmp23A.fields)->v);
struct _tuple6*_tmp240=Cyc_Tcgenrep_toplevel_name(({struct _tagged_arr*_tmp241=
_cycalloc(sizeof(*_tmp241));_tmp241[0]=Cyc_Tcgenrep_make_type_cstring(type);
_tmp241;}));dict=Cyc_Tcgenrep_buildRepEnum((struct _tuple6**)& _tmp23A.name,te,
dict,loc,_tmp240,(void*)2,type,_tmp23F);goto _LL15;}}_LL44: if(_tmp19A <= (void*)3?
1:*((int*)_tmp19A)!= 13)goto _LL46;_tmp1B7=((struct Cyc_Absyn_AnonEnumType_struct*)
_tmp19A)->f1;_LL45: {struct Cyc_List_List*_tmp242=((struct Cyc_List_List*(*)(
struct _tuple16*(*f)(struct Cyc_Absyn_Enumfield*),struct Cyc_List_List*x))Cyc_List_map)(
Cyc_Tcgenrep_select_enumfield_tagnm,_tmp1B7);struct _tuple6*_tmp243=Cyc_Tcgenrep_toplevel_name(
Cyc_Tcgenrep_new_gen_id(_tag_arr("rep",sizeof(unsigned char),4)));dict=Cyc_Tcgenrep_buildRepEnum(
0,te,dict,loc,_tmp243,(void*)0,type,_tmp242);goto _LL15;}_LL46: if(_tmp19A <= (void*)
3?1:*((int*)_tmp19A)!= 14)goto _LL15;_LL47:({void*_tmp244[0]={};((int(*)(struct
_tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("gen() for sizeof_t<-> not yet supported",
sizeof(unsigned char),40),_tag_arr(_tmp244,sizeof(void*),0));});_LL15:;}return({
struct _tuple18*_tmp245=_cycalloc(sizeof(*_tmp245));_tmp245->f1=dict;_tmp245->f2=((
struct Cyc_Tcgenrep_RepInfo*(*)(struct Cyc_Dict_Dict*d,void*k))Cyc_Dict_lookup)(
dict,type);_tmp245;});}static int Cyc_Tcgenrep_not_emitted_filter(struct Cyc_Tcgenrep_RepInfo*
ri){return ri->emitted == 0;}static void Cyc_Tcgenrep_mark_emitted(struct Cyc_Tcgenrep_RepInfo*
ri){ri->emitted=1;}struct _tuple21{struct Cyc_Dict_Dict*f1;struct Cyc_List_List*f2;
struct Cyc_Absyn_Exp*f3;};struct _tuple21 Cyc_Tcgenrep_tcGenrep(struct Cyc_Tcenv_Tenv*
te,struct Cyc_Tcenv_Genv*ge,struct Cyc_Position_Segment*loc,void*type,struct Cyc_Dict_Dict*
dict){Cyc_Tcutil_check_valid_toplevel_type(loc,te,type);Cyc_Tcutil_check_valid_toplevel_type(
loc,te,type);{struct _tuple18 _tmp247;struct Cyc_Dict_Dict*_tmp248;struct Cyc_Tcgenrep_RepInfo*
_tmp249;struct _tuple18*_tmp246=Cyc_Tcgenrep_lookupRep(te,dict,loc,type);_tmp247=*
_tmp246;_tmp248=_tmp247.f1;_tmp249=_tmp247.f2;{struct Cyc_List_List*_tmp24A=Cyc_Tcgenrep_dfs(
_tmp249);return({struct _tuple21 _tmp24B;_tmp24B.f1=_tmp248;_tmp24B.f2=_tmp24A;
_tmp24B.f3=(struct Cyc_Absyn_Exp*)_check_null(_tmp249->exp);_tmp24B;});}}}
