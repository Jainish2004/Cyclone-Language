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
 struct Cyc_Std__types_fd_set{int fds_bits[2];};struct Cyc_Core_Opt{void*v;};extern
unsigned char Cyc_Core_Invalid_argument[21];struct Cyc_Core_Invalid_argument_struct{
unsigned char*tag;struct _tagged_arr f1;};extern unsigned char Cyc_Core_Failure[12];
struct Cyc_Core_Failure_struct{unsigned char*tag;struct _tagged_arr f1;};extern
unsigned char Cyc_Core_Impossible[15];struct Cyc_Core_Impossible_struct{
unsigned char*tag;struct _tagged_arr f1;};extern unsigned char Cyc_Core_Not_found[14];
extern unsigned char Cyc_Core_Unreachable[16];struct Cyc_Core_Unreachable_struct{
unsigned char*tag;struct _tagged_arr f1;};struct Cyc_List_List{void*hd;struct Cyc_List_List*
tl;};struct Cyc_List_List*Cyc_List_list(struct _tagged_arr);struct Cyc_List_List*
Cyc_List_map(void*(*f)(void*),struct Cyc_List_List*x);extern unsigned char Cyc_List_List_mismatch[
18];struct Cyc_List_List*Cyc_List_imp_rev(struct Cyc_List_List*x);extern
unsigned char Cyc_List_Nth[8];int Cyc_List_list_cmp(int(*cmp)(void*,void*),struct
Cyc_List_List*l1,struct Cyc_List_List*l2);struct Cyc_Lineno_Pos{struct _tagged_arr
logical_file;struct _tagged_arr line;int line_no;int col;};extern unsigned char Cyc_Position_Exit[
9];struct Cyc_Position_Segment;struct Cyc_Position_Error{struct _tagged_arr source;
struct Cyc_Position_Segment*seg;void*kind;struct _tagged_arr desc;};extern
unsigned char Cyc_Position_Nocontext[14];struct Cyc_Absyn_Rel_n_struct{int tag;
struct Cyc_List_List*f1;};struct Cyc_Absyn_Abs_n_struct{int tag;struct Cyc_List_List*
f1;};struct _tuple0{void*f1;struct _tagged_arr*f2;};struct Cyc_Absyn_Conref;struct
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
_tuple0*name;int is_xtunion;};struct Cyc_Absyn_UnknownTunion_struct{int tag;struct
Cyc_Absyn_UnknownTunionInfo f1;};struct Cyc_Absyn_KnownTunion_struct{int tag;struct
Cyc_Absyn_Tuniondecl**f1;};struct Cyc_Absyn_TunionInfo{void*tunion_info;struct Cyc_List_List*
targs;void*rgn;};struct Cyc_Absyn_UnknownTunionFieldInfo{struct _tuple0*
tunion_name;struct _tuple0*field_name;int is_xtunion;};struct Cyc_Absyn_UnknownTunionfield_struct{
int tag;struct Cyc_Absyn_UnknownTunionFieldInfo f1;};struct Cyc_Absyn_KnownTunionfield_struct{
int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*f2;};struct Cyc_Absyn_TunionFieldInfo{
void*field_info;struct Cyc_List_List*targs;};struct Cyc_Absyn_UnknownAggr_struct{
int tag;void*f1;struct _tuple0*f2;};struct Cyc_Absyn_KnownAggr_struct{int tag;struct
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
_tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;};struct Cyc_Absyn_AnonEnumType_struct{int
tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_SizeofType_struct{int tag;void*f1;};
struct Cyc_Absyn_RgnHandleType_struct{int tag;void*f1;};struct Cyc_Absyn_TypedefType_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_Core_Opt*f3;};struct Cyc_Absyn_AccessEff_struct{
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
tag;void*f1;};struct Cyc_Absyn_Var_e_struct{int tag;struct _tuple0*f1;void*f2;};
struct Cyc_Absyn_UnknownId_e_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_Primop_e_struct{
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
int tag;struct Cyc_List_List*f1;};struct _tuple1{struct Cyc_Core_Opt*f1;struct Cyc_Absyn_Tqual
f2;void*f3;};struct Cyc_Absyn_CompoundLit_e_struct{int tag;struct _tuple1*f1;struct
Cyc_List_List*f2;};struct Cyc_Absyn_Array_e_struct{int tag;struct Cyc_List_List*f1;
};struct Cyc_Absyn_Comprehension_e_struct{int tag;struct Cyc_Absyn_Vardecl*f1;
struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*f3;};struct Cyc_Absyn_Struct_e_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;struct Cyc_List_List*f3;struct Cyc_Absyn_Aggrdecl*
f4;};struct Cyc_Absyn_AnonStruct_e_struct{int tag;void*f1;struct Cyc_List_List*f2;}
;struct Cyc_Absyn_Tunion_e_struct{int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Tuniondecl*
f2;struct Cyc_Absyn_Tunionfield*f3;};struct Cyc_Absyn_Enum_e_struct{int tag;struct
_tuple0*f1;struct Cyc_Absyn_Enumdecl*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_AnonEnum_e_struct{
int tag;struct _tuple0*f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};struct Cyc_Absyn_Malloc_e_struct{
int tag;struct Cyc_Absyn_MallocInfo f1;};struct Cyc_Absyn_UnresolvedMem_e_struct{int
tag;struct Cyc_Core_Opt*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_StmtExp_e_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Codegen_e_struct{int tag;struct
Cyc_Absyn_Fndecl*f1;};struct Cyc_Absyn_Fill_e_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_Exp{struct Cyc_Core_Opt*topt;void*r;struct Cyc_Position_Segment*
loc;void*annot;};struct _tuple2{struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*f2;};
struct Cyc_Absyn_ForArrayInfo{struct Cyc_List_List*defns;struct _tuple2 condition;
struct _tuple2 delta;struct Cyc_Absyn_Stmt*body;};struct Cyc_Absyn_Exp_s_struct{int
tag;struct Cyc_Absyn_Exp*f1;};struct Cyc_Absyn_Seq_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Return_s_struct{int tag;struct Cyc_Absyn_Exp*
f1;};struct Cyc_Absyn_IfThenElse_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_Absyn_Stmt*
f2;struct Cyc_Absyn_Stmt*f3;};struct Cyc_Absyn_While_s_struct{int tag;struct _tuple2
f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Break_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;};struct Cyc_Absyn_Continue_s_struct{int tag;struct Cyc_Absyn_Stmt*f1;};struct
Cyc_Absyn_Goto_s_struct{int tag;struct _tagged_arr*f1;struct Cyc_Absyn_Stmt*f2;};
struct Cyc_Absyn_For_s_struct{int tag;struct Cyc_Absyn_Exp*f1;struct _tuple2 f2;
struct _tuple2 f3;struct Cyc_Absyn_Stmt*f4;};struct Cyc_Absyn_Switch_s_struct{int tag;
struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_SwitchC_s_struct{
int tag;struct Cyc_Absyn_Exp*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Fallthru_s_struct{
int tag;struct Cyc_List_List*f1;struct Cyc_Absyn_Switch_clause**f2;};struct Cyc_Absyn_Decl_s_struct{
int tag;struct Cyc_Absyn_Decl*f1;struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Cut_s_struct{
int tag;struct Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Splice_s_struct{int tag;struct
Cyc_Absyn_Stmt*f1;};struct Cyc_Absyn_Label_s_struct{int tag;struct _tagged_arr*f1;
struct Cyc_Absyn_Stmt*f2;};struct Cyc_Absyn_Do_s_struct{int tag;struct Cyc_Absyn_Stmt*
f1;struct _tuple2 f2;};struct Cyc_Absyn_TryCatch_s_struct{int tag;struct Cyc_Absyn_Stmt*
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
f2;};struct Cyc_Absyn_UnknownId_p_struct{int tag;struct _tuple0*f1;};struct Cyc_Absyn_UnknownCall_p_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Pat{void*r;
struct Cyc_Core_Opt*topt;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Switch_clause{
struct Cyc_Absyn_Pat*pattern;struct Cyc_Core_Opt*pat_vars;struct Cyc_Absyn_Exp*
where_clause;struct Cyc_Absyn_Stmt*body;struct Cyc_Position_Segment*loc;};struct
Cyc_Absyn_SwitchC_clause{struct Cyc_Absyn_Exp*cnst_exp;struct Cyc_Absyn_Stmt*body;
struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Global_b_struct{int tag;struct
Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Funname_b_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Param_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct
Cyc_Absyn_Local_b_struct{int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Pat_b_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Vardecl{void*sc;struct
_tuple0*name;struct Cyc_Absyn_Tqual tq;void*type;struct Cyc_Absyn_Exp*initializer;
struct Cyc_Core_Opt*rgn;struct Cyc_List_List*attributes;int escapes;};struct Cyc_Absyn_Fndecl{
void*sc;int is_inline;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
effect;void*ret_type;struct Cyc_List_List*args;int c_varargs;struct Cyc_Absyn_VarargInfo*
cyc_varargs;struct Cyc_List_List*rgn_po;struct Cyc_Absyn_Stmt*body;struct Cyc_Core_Opt*
cached_typ;struct Cyc_Core_Opt*param_vardecls;struct Cyc_List_List*attributes;};
struct Cyc_Absyn_Aggrfield{struct _tagged_arr*name;struct Cyc_Absyn_Tqual tq;void*
type;struct Cyc_Absyn_Exp*width;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Aggrdecl{
void*kind;void*sc;struct _tuple0*name;struct Cyc_List_List*tvs;struct Cyc_Core_Opt*
exist_vars;struct Cyc_Core_Opt*fields;struct Cyc_List_List*attributes;};struct Cyc_Absyn_Tunionfield{
struct _tuple0*name;struct Cyc_List_List*typs;struct Cyc_Position_Segment*loc;void*
sc;};struct Cyc_Absyn_Tuniondecl{void*sc;struct _tuple0*name;struct Cyc_List_List*
tvs;struct Cyc_Core_Opt*fields;int is_xtunion;};struct Cyc_Absyn_Enumfield{struct
_tuple0*name;struct Cyc_Absyn_Exp*tag;struct Cyc_Position_Segment*loc;};struct Cyc_Absyn_Enumdecl{
void*sc;struct _tuple0*name;struct Cyc_Core_Opt*fields;};struct Cyc_Absyn_Typedefdecl{
struct _tuple0*name;struct Cyc_List_List*tvs;void*defn;};struct Cyc_Absyn_Var_d_struct{
int tag;struct Cyc_Absyn_Vardecl*f1;};struct Cyc_Absyn_Fn_d_struct{int tag;struct Cyc_Absyn_Fndecl*
f1;};struct Cyc_Absyn_Let_d_struct{int tag;struct Cyc_Absyn_Pat*f1;struct Cyc_Core_Opt*
f2;struct Cyc_Core_Opt*f3;struct Cyc_Absyn_Exp*f4;int f5;};struct Cyc_Absyn_Letv_d_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Aggr_d_struct{int tag;struct Cyc_Absyn_Aggrdecl*
f1;};struct Cyc_Absyn_Tunion_d_struct{int tag;struct Cyc_Absyn_Tuniondecl*f1;};
struct Cyc_Absyn_Enum_d_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;};struct Cyc_Absyn_Typedef_d_struct{
int tag;struct Cyc_Absyn_Typedefdecl*f1;};struct Cyc_Absyn_Namespace_d_struct{int
tag;struct _tagged_arr*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_Using_d_struct{
int tag;struct _tuple0*f1;struct Cyc_List_List*f2;};struct Cyc_Absyn_ExternC_d_struct{
int tag;struct Cyc_List_List*f1;};struct Cyc_Absyn_Decl{void*r;struct Cyc_Position_Segment*
loc;};struct Cyc_Absyn_ArrayElement_struct{int tag;struct Cyc_Absyn_Exp*f1;};struct
Cyc_Absyn_FieldName_struct{int tag;struct _tagged_arr*f1;};unsigned char Cyc_Absyn_EmptyAnnot[
15]="\000\000\000\000EmptyAnnot";int Cyc_Absyn_qvar_cmp(struct _tuple0*,struct
_tuple0*);int Cyc_Absyn_varlist_cmp(struct Cyc_List_List*,struct Cyc_List_List*);
int Cyc_Absyn_tvar_cmp(struct Cyc_Absyn_Tvar*,struct Cyc_Absyn_Tvar*);extern struct
Cyc_Absyn_Rel_n_struct Cyc_Absyn_rel_ns_null_value;extern void*Cyc_Absyn_rel_ns_null;
int Cyc_Absyn_is_qvar_qualified(struct _tuple0*);struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual();
struct Cyc_Absyn_Tqual Cyc_Absyn_combine_tqual(struct Cyc_Absyn_Tqual x,struct Cyc_Absyn_Tqual
y);struct Cyc_Absyn_Tqual Cyc_Absyn_empty_tqual();struct Cyc_Absyn_Conref*Cyc_Absyn_new_conref(
void*x);struct Cyc_Absyn_Conref*Cyc_Absyn_empty_conref();struct Cyc_Absyn_Conref*
Cyc_Absyn_compress_conref(struct Cyc_Absyn_Conref*x);void*Cyc_Absyn_conref_val(
struct Cyc_Absyn_Conref*x);void*Cyc_Absyn_conref_def(void*,struct Cyc_Absyn_Conref*
x);void*Cyc_Absyn_compress_kb(void*);void*Cyc_Absyn_force_kb(void*kb);void*Cyc_Absyn_new_evar(
struct Cyc_Core_Opt*k,struct Cyc_Core_Opt*tenv);void*Cyc_Absyn_wildtyp(struct Cyc_Core_Opt*);
extern void*Cyc_Absyn_uchar_t;extern void*Cyc_Absyn_ushort_t;extern void*Cyc_Absyn_uint_t;
extern void*Cyc_Absyn_ulong_t;extern void*Cyc_Absyn_ulonglong_t;extern void*Cyc_Absyn_schar_t;
extern void*Cyc_Absyn_sshort_t;extern void*Cyc_Absyn_sint_t;extern void*Cyc_Absyn_slong_t;
extern void*Cyc_Absyn_slonglong_t;extern void*Cyc_Absyn_float_typ;void*Cyc_Absyn_double_typ(
int);extern void*Cyc_Absyn_empty_effect;extern struct _tuple0*Cyc_Absyn_exn_name;
extern struct Cyc_Absyn_Tuniondecl*Cyc_Absyn_exn_tud;extern void*Cyc_Absyn_exn_typ;
extern struct _tuple0*Cyc_Absyn_tunion_print_arg_qvar;extern struct _tuple0*Cyc_Absyn_tunion_scanf_arg_qvar;
void*Cyc_Absyn_string_typ(void*rgn);void*Cyc_Absyn_const_string_typ(void*rgn);
void*Cyc_Absyn_file_typ();extern struct Cyc_Absyn_Exp*Cyc_Absyn_exp_unsigned_one;
extern void*Cyc_Absyn_bounds_one;void*Cyc_Absyn_starb_typ(void*t,void*rgn,struct
Cyc_Absyn_Tqual tq,void*b);void*Cyc_Absyn_atb_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual
tq,void*b);void*Cyc_Absyn_star_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual tq);void*
Cyc_Absyn_at_typ(void*t,void*rgn,struct Cyc_Absyn_Tqual tq);void*Cyc_Absyn_cstar_typ(
void*t,struct Cyc_Absyn_Tqual tq);void*Cyc_Absyn_tagged_typ(void*t,void*rgn,struct
Cyc_Absyn_Tqual tq);void*Cyc_Absyn_void_star_typ();struct Cyc_Core_Opt*Cyc_Absyn_void_star_typ_opt();
void*Cyc_Absyn_strct(struct _tagged_arr*name);void*Cyc_Absyn_strctq(struct _tuple0*
name);void*Cyc_Absyn_unionq_typ(struct _tuple0*name);void*Cyc_Absyn_union_typ(
struct _tagged_arr*name);struct Cyc_Absyn_Exp*Cyc_Absyn_new_exp(void*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_New_exp(struct Cyc_Absyn_Exp*rgn_handle,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_copy_exp(struct Cyc_Absyn_Exp*);
struct Cyc_Absyn_Exp*Cyc_Absyn_const_exp(void*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_null_exp(struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_bool_exp(int,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_true_exp(
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_false_exp(struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_int_exp(void*,int,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_signed_int_exp(int,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(unsigned int,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_char_exp(unsigned char c,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_float_exp(struct _tagged_arr f,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_string_exp(struct _tagged_arr s,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_var_exp(struct _tuple0*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_varb_exp(struct _tuple0*,void*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_unknownid_exp(struct _tuple0*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_primop_exp(void*,struct Cyc_List_List*es,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_prim1_exp(void*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_prim2_exp(void*,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_add_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_times_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_divide_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_eq_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_neq_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_gt_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_lt_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_gte_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_lte_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_assignop_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Core_Opt*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_assign_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_increment_exp(struct
Cyc_Absyn_Exp*,void*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_post_inc_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_post_dec_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_pre_inc_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_pre_dec_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_conditional_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_and_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_or_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_seq_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_unknowncall_exp(struct Cyc_Absyn_Exp*,struct Cyc_List_List*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_fncall_exp(struct Cyc_Absyn_Exp*,struct Cyc_List_List*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_throw_exp(struct Cyc_Absyn_Exp*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_noinstantiate_exp(
struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_instantiate_exp(
struct Cyc_Absyn_Exp*,struct Cyc_List_List*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_cast_exp(void*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_address_exp(struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_sizeoftyp_exp(void*t,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_sizeofexp_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_offsetof_exp(void*,void*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_gentyp_exp(struct Cyc_List_List*,void*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_deref_exp(struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_aggrmember_exp(struct Cyc_Absyn_Exp*,struct
_tagged_arr*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*Cyc_Absyn_aggrarrow_exp(
struct Cyc_Absyn_Exp*,struct _tagged_arr*,struct Cyc_Position_Segment*);struct Cyc_Absyn_Exp*
Cyc_Absyn_subscript_exp(struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_tuple_exp(struct Cyc_List_List*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_stmt_exp(struct Cyc_Absyn_Stmt*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_match_exn_exp(struct Cyc_Position_Segment*);struct
Cyc_Absyn_Exp*Cyc_Absyn_array_exp(struct Cyc_List_List*,struct Cyc_Position_Segment*);
struct Cyc_Absyn_Exp*Cyc_Absyn_unresolvedmem_exp(struct Cyc_Core_Opt*,struct Cyc_List_List*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Stmt*Cyc_Absyn_new_stmt(void*s,
struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_skip_stmt(struct
Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_exp_stmt(struct Cyc_Absyn_Exp*
e,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmt(struct
Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*s2,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmts(struct Cyc_List_List*,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_return_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_ifthenelse_stmt(struct Cyc_Absyn_Exp*e,struct
Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*s2,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Stmt*Cyc_Absyn_while_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Stmt*s,
struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_break_stmt(struct
Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_continue_stmt(struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_for_stmt(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Absyn_Exp*e3,struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc);
struct Cyc_Absyn_Stmt*Cyc_Absyn_switch_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*,
struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_fallthru_stmt(
struct Cyc_List_List*el,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_decl_stmt(
struct Cyc_Absyn_Decl*d,struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc);
struct Cyc_Absyn_Stmt*Cyc_Absyn_declare_stmt(struct _tuple0*,void*,struct Cyc_Absyn_Exp*
init,struct Cyc_Absyn_Stmt*,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_cut_stmt(struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Stmt*Cyc_Absyn_splice_stmt(struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_label_stmt(struct _tagged_arr*v,struct Cyc_Absyn_Stmt*
s,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_do_stmt(struct
Cyc_Absyn_Stmt*s,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*
Cyc_Absyn_trycatch_stmt(struct Cyc_Absyn_Stmt*s,struct Cyc_List_List*scs,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_goto_stmt(struct _tagged_arr*lab,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_assign_stmt(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Stmt*Cyc_Absyn_forarray_stmt(
struct Cyc_List_List*,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Exp*,struct Cyc_Absyn_Stmt*,
struct Cyc_Position_Segment*);struct Cyc_Absyn_Pat*Cyc_Absyn_new_pat(void*p,struct
Cyc_Position_Segment*s);struct Cyc_Absyn_Decl*Cyc_Absyn_new_decl(void*r,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Decl*Cyc_Absyn_let_decl(struct Cyc_Absyn_Pat*p,struct Cyc_Core_Opt*
t_opt,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Decl*
Cyc_Absyn_letv_decl(struct Cyc_List_List*,struct Cyc_Position_Segment*loc);struct
Cyc_Absyn_Vardecl*Cyc_Absyn_new_vardecl(struct _tuple0*x,void*t,struct Cyc_Absyn_Exp*
init);struct Cyc_Absyn_Vardecl*Cyc_Absyn_static_vardecl(struct _tuple0*x,void*t,
struct Cyc_Absyn_Exp*init);struct Cyc_Absyn_Decl*Cyc_Absyn_aggr_decl(void*k,void*s,
struct _tuple0*n,struct Cyc_List_List*ts,struct Cyc_Core_Opt*exist_ts,struct Cyc_Core_Opt*
fs,struct Cyc_List_List*atts,struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Decl*
Cyc_Absyn_struct_decl(void*s,struct _tuple0*n,struct Cyc_List_List*ts,struct Cyc_Core_Opt*
exist_ts,struct Cyc_Core_Opt*fs,struct Cyc_List_List*atts,struct Cyc_Position_Segment*
loc);struct Cyc_Absyn_Decl*Cyc_Absyn_union_decl(void*s,struct _tuple0*n,struct Cyc_List_List*
ts,struct Cyc_Core_Opt*exist_ts,struct Cyc_Core_Opt*fs,struct Cyc_List_List*atts,
struct Cyc_Position_Segment*loc);struct Cyc_Absyn_Decl*Cyc_Absyn_tunion_decl(void*
s,struct _tuple0*n,struct Cyc_List_List*ts,struct Cyc_Core_Opt*fs,int is_xtunion,
struct Cyc_Position_Segment*loc);void*Cyc_Absyn_function_typ(struct Cyc_List_List*
tvs,struct Cyc_Core_Opt*eff_typ,void*ret_typ,struct Cyc_List_List*args,int
c_varargs,struct Cyc_Absyn_VarargInfo*cyc_varargs,struct Cyc_List_List*rgn_po,
struct Cyc_List_List*);void*Cyc_Absyn_pointer_expand(void*);int Cyc_Absyn_is_lvalue(
struct Cyc_Absyn_Exp*);struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_field(struct Cyc_List_List*,
struct _tagged_arr*);struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_decl_field(struct
Cyc_Absyn_Aggrdecl*,struct _tagged_arr*);struct _tuple3{struct Cyc_Absyn_Tqual f1;
void*f2;};struct _tuple3*Cyc_Absyn_lookup_tuple_field(struct Cyc_List_List*,int);
struct _tagged_arr Cyc_Absyn_attribute2string(void*);int Cyc_Absyn_fntype_att(void*
a);struct _tagged_arr*Cyc_Absyn_fieldname(int);struct _tuple4{void*f1;struct
_tuple0*f2;};struct _tuple4 Cyc_Absyn_aggr_kinded_name(void*);struct Cyc_Absyn_Aggrdecl*
Cyc_Absyn_get_known_aggrdecl(void*info);int Cyc_Absyn_is_union_type(void*);void
Cyc_Absyn_print_decls(struct Cyc_List_List*);struct Cyc_Typerep_Int_struct{int tag;
int f1;unsigned int f2;};struct Cyc_Typerep_ThinPtr_struct{int tag;unsigned int f1;
void*f2;};struct Cyc_Typerep_FatPtr_struct{int tag;void*f1;};struct _tuple5{
unsigned int f1;struct _tagged_arr f2;void*f3;};struct Cyc_Typerep_Struct_struct{int
tag;struct _tagged_arr*f1;unsigned int f2;struct _tagged_arr f3;};struct _tuple6{
unsigned int f1;void*f2;};struct Cyc_Typerep_Tuple_struct{int tag;unsigned int f1;
struct _tagged_arr f2;};struct _tuple7{unsigned int f1;struct _tagged_arr f2;};struct
Cyc_Typerep_TUnion_struct{int tag;struct _tagged_arr f1;struct _tagged_arr f2;struct
_tagged_arr f3;};struct Cyc_Typerep_TUnionField_struct{int tag;struct _tagged_arr f1;
struct _tagged_arr f2;unsigned int f3;struct _tagged_arr f4;};struct _tuple8{struct
_tagged_arr f1;void*f2;};struct Cyc_Typerep_XTUnion_struct{int tag;struct
_tagged_arr f1;struct _tagged_arr f2;};struct Cyc_Typerep_Union_struct{int tag;struct
_tagged_arr*f1;int f2;struct _tagged_arr f3;};struct Cyc_Typerep_Enum_struct{int tag;
struct _tagged_arr*f1;int f2;struct _tagged_arr f3;};unsigned int Cyc_Typerep_size_type(
void*rep);extern void*Cyc_decls_rep;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Position_Segment_rep;
int Cyc_Std_zstrptrcmp(struct _tagged_arr*,struct _tagged_arr*);struct Cyc_Cstdio___abstractFILE;
struct Cyc_Std___cycFILE;extern unsigned char Cyc_Std_FileCloseError[19];extern
unsigned char Cyc_Std_FileOpenError[18];struct Cyc_Std_FileOpenError_struct{
unsigned char*tag;struct _tagged_arr f1;};struct Cyc_Std_String_pa_struct{int tag;
struct _tagged_arr f1;};struct Cyc_Std_Int_pa_struct{int tag;unsigned int f1;};struct
Cyc_Std_Double_pa_struct{int tag;double f1;};struct Cyc_Std_ShortPtr_pa_struct{int
tag;short*f1;};struct Cyc_Std_IntPtr_pa_struct{int tag;unsigned int*f1;};int Cyc_Std_printf(
struct _tagged_arr fmt,struct _tagged_arr);struct _tagged_arr Cyc_Std_aprintf(struct
_tagged_arr fmt,struct _tagged_arr);struct Cyc_Std_ShortPtr_sa_struct{int tag;short*
f1;};struct Cyc_Std_UShortPtr_sa_struct{int tag;unsigned short*f1;};struct Cyc_Std_IntPtr_sa_struct{
int tag;int*f1;};struct Cyc_Std_UIntPtr_sa_struct{int tag;unsigned int*f1;};struct
Cyc_Std_StringPtr_sa_struct{int tag;struct _tagged_arr f1;};struct Cyc_Std_DoublePtr_sa_struct{
int tag;double*f1;};struct Cyc_Std_FloatPtr_sa_struct{int tag;float*f1;};struct Cyc_Iter_Iter{
void*env;int(*next)(void*env,void*dest);};int Cyc_Iter_next(struct Cyc_Iter_Iter,
void*);struct Cyc_Set_Set;extern unsigned char Cyc_Set_Absent[11];struct Cyc_Dict_Dict;
extern unsigned char Cyc_Dict_Present[12];extern unsigned char Cyc_Dict_Absent[11];
struct _tuple9{void*f1;void*f2;};struct _tuple9*Cyc_Dict_rchoose(struct
_RegionHandle*r,struct Cyc_Dict_Dict*d);struct _tuple9*Cyc_Dict_rchoose(struct
_RegionHandle*,struct Cyc_Dict_Dict*d);struct Cyc_Tcenv_VarRes_struct{int tag;void*
f1;};struct Cyc_Tcenv_AggrRes_struct{int tag;struct Cyc_Absyn_Aggrdecl*f1;};struct
Cyc_Tcenv_TunionRes_struct{int tag;struct Cyc_Absyn_Tuniondecl*f1;struct Cyc_Absyn_Tunionfield*
f2;};struct Cyc_Tcenv_EnumRes_struct{int tag;struct Cyc_Absyn_Enumdecl*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Tcenv_AnonEnumRes_struct{int tag;void*f1;struct Cyc_Absyn_Enumfield*
f2;};struct Cyc_Tcenv_Genv{struct Cyc_Set_Set*namespaces;struct Cyc_Dict_Dict*
aggrdecls;struct Cyc_Dict_Dict*tuniondecls;struct Cyc_Dict_Dict*enumdecls;struct
Cyc_Dict_Dict*typedefs;struct Cyc_Dict_Dict*ordinaries;struct Cyc_List_List*
availables;};struct Cyc_Tcenv_Fenv;struct Cyc_Tcenv_Stmt_j_struct{int tag;struct Cyc_Absyn_Stmt*
f1;};struct Cyc_Tcenv_Outermost_struct{int tag;void*f1;};struct Cyc_Tcenv_Frame_struct{
int tag;void*f1;void*f2;};struct Cyc_Tcenv_Hidden_struct{int tag;void*f1;void*f2;};
struct Cyc_Tcenv_Tenv{struct Cyc_List_List*ns;struct Cyc_Dict_Dict*ae;struct Cyc_Core_Opt*
le;};void*Cyc_Tcutil_impos(struct _tagged_arr fmt,struct _tagged_arr ap);void*Cyc_Tcutil_compress(
void*t);void Cyc_Marshal_print_type(void*rep,void*val);static int Cyc_Absyn_zstrlist_cmp(
struct Cyc_List_List*ss1,struct Cyc_List_List*ss2){return((int(*)(int(*cmp)(struct
_tagged_arr*,struct _tagged_arr*),struct Cyc_List_List*l1,struct Cyc_List_List*l2))
Cyc_List_list_cmp)(Cyc_Std_zstrptrcmp,ss1,ss2);}int Cyc_Absyn_varlist_cmp(struct
Cyc_List_List*vs1,struct Cyc_List_List*vs2){if((int)vs1 == (int)vs2)return 0;return
Cyc_Absyn_zstrlist_cmp(vs1,vs2);}int Cyc_Absyn_qvar_cmp(struct _tuple0*q1,struct
_tuple0*q2){void*_tmp0=(*q1).f1;void*_tmp1=(*q2).f1;{struct _tuple9 _tmp3=({struct
_tuple9 _tmp2;_tmp2.f1=_tmp0;_tmp2.f2=_tmp1;_tmp2;});void*_tmp4;void*_tmp5;void*
_tmp6;struct Cyc_List_List*_tmp7;void*_tmp8;struct Cyc_List_List*_tmp9;void*_tmpA;
struct Cyc_List_List*_tmpB;void*_tmpC;struct Cyc_List_List*_tmpD;void*_tmpE;void*
_tmpF;void*_tmp10;void*_tmp11;_LL1: _tmp4=_tmp3.f1;if((int)_tmp4 != 0)goto _LL3;
_tmp5=_tmp3.f2;if((int)_tmp5 != 0)goto _LL3;_LL2: goto _LL0;_LL3: _tmp6=_tmp3.f1;if(
_tmp6 <= (void*)1?1:*((int*)_tmp6)!= 0)goto _LL5;_tmp7=((struct Cyc_Absyn_Rel_n_struct*)
_tmp6)->f1;_tmp8=_tmp3.f2;if(_tmp8 <= (void*)1?1:*((int*)_tmp8)!= 0)goto _LL5;
_tmp9=((struct Cyc_Absyn_Rel_n_struct*)_tmp8)->f1;_LL4: _tmpB=_tmp7;_tmpD=_tmp9;
goto _LL6;_LL5: _tmpA=_tmp3.f1;if(_tmpA <= (void*)1?1:*((int*)_tmpA)!= 1)goto _LL7;
_tmpB=((struct Cyc_Absyn_Abs_n_struct*)_tmpA)->f1;_tmpC=_tmp3.f2;if(_tmpC <= (void*)
1?1:*((int*)_tmpC)!= 1)goto _LL7;_tmpD=((struct Cyc_Absyn_Abs_n_struct*)_tmpC)->f1;
_LL6: {int i=Cyc_Absyn_zstrlist_cmp(_tmpB,_tmpD);if(i != 0)return i;goto _LL0;}_LL7:
_tmpE=_tmp3.f1;if((int)_tmpE != 0)goto _LL9;_LL8: return - 1;_LL9: _tmpF=_tmp3.f2;if((
int)_tmpF != 0)goto _LLB;_LLA: return 1;_LLB: _tmp10=_tmp3.f1;if(_tmp10 <= (void*)1?1:*((
int*)_tmp10)!= 0)goto _LLD;_LLC: return - 1;_LLD: _tmp11=_tmp3.f2;if(_tmp11 <= (void*)
1?1:*((int*)_tmp11)!= 0)goto _LL0;_LLE: return 1;_LL0:;}return Cyc_Std_zstrptrcmp((*
q1).f2,(*q2).f2);}int Cyc_Absyn_tvar_cmp(struct Cyc_Absyn_Tvar*tv1,struct Cyc_Absyn_Tvar*
tv2){int i=Cyc_Std_zstrptrcmp(tv1->name,tv2->name);if(i != 0)return i;if(tv1->identity
== tv2->identity)return 0;if(tv1->identity != 0?tv2->identity != 0: 0)return*((int*)
_check_null(tv1->identity))- *((int*)_check_null(tv2->identity));else{if(tv1->identity
== 0)return - 1;else{return 1;}}}struct Cyc_Absyn_Rel_n_struct Cyc_Absyn_rel_ns_null_value={
0,0};void*Cyc_Absyn_rel_ns_null=(void*)& Cyc_Absyn_rel_ns_null_value;int Cyc_Absyn_is_qvar_qualified(
struct _tuple0*qv){void*_tmp13=(*qv).f1;struct Cyc_List_List*_tmp14;struct Cyc_List_List*
_tmp15;_LL10: if(_tmp13 <= (void*)1?1:*((int*)_tmp13)!= 0)goto _LL12;_tmp14=((
struct Cyc_Absyn_Rel_n_struct*)_tmp13)->f1;if(_tmp14 != 0)goto _LL12;_LL11: goto
_LL13;_LL12: if(_tmp13 <= (void*)1?1:*((int*)_tmp13)!= 1)goto _LL14;_tmp15=((struct
Cyc_Absyn_Abs_n_struct*)_tmp13)->f1;if(_tmp15 != 0)goto _LL14;_LL13: goto _LL15;
_LL14: if((int)_tmp13 != 0)goto _LL16;_LL15: return 0;_LL16:;_LL17: return 1;_LLF:;}
static int Cyc_Absyn_new_type_counter=0;void*Cyc_Absyn_new_evar(struct Cyc_Core_Opt*
k,struct Cyc_Core_Opt*env){return(void*)({struct Cyc_Absyn_Evar_struct*_tmp16=
_cycalloc(sizeof(*_tmp16));_tmp16[0]=({struct Cyc_Absyn_Evar_struct _tmp17;_tmp17.tag=
0;_tmp17.f1=k;_tmp17.f2=0;_tmp17.f3=Cyc_Absyn_new_type_counter ++;_tmp17.f4=env;
_tmp17;});_tmp16;});}static struct Cyc_Core_Opt Cyc_Absyn_mk={(void*)((void*)1)};
void*Cyc_Absyn_wildtyp(struct Cyc_Core_Opt*tenv){return Cyc_Absyn_new_evar((struct
Cyc_Core_Opt*)& Cyc_Absyn_mk,tenv);}struct Cyc_Absyn_Tqual Cyc_Absyn_combine_tqual(
struct Cyc_Absyn_Tqual x,struct Cyc_Absyn_Tqual y){return({struct Cyc_Absyn_Tqual
_tmp18;_tmp18.q_const=x.q_const?1: y.q_const;_tmp18.q_volatile=x.q_volatile?1: y.q_volatile;
_tmp18.q_restrict=x.q_restrict?1: y.q_restrict;_tmp18;});}struct Cyc_Absyn_Tqual
Cyc_Absyn_empty_tqual(){return({struct Cyc_Absyn_Tqual _tmp19;_tmp19.q_const=0;
_tmp19.q_volatile=0;_tmp19.q_restrict=0;_tmp19;});}struct Cyc_Absyn_Tqual Cyc_Absyn_const_tqual(){
return({struct Cyc_Absyn_Tqual _tmp1A;_tmp1A.q_const=1;_tmp1A.q_volatile=0;_tmp1A.q_restrict=
0;_tmp1A;});}struct Cyc_Absyn_Conref*Cyc_Absyn_new_conref(void*x){return({struct
Cyc_Absyn_Conref*_tmp1B=_cycalloc(sizeof(*_tmp1B));_tmp1B->v=(void*)((void*)({
struct Cyc_Absyn_Eq_constr_struct*_tmp1C=_cycalloc(sizeof(*_tmp1C));_tmp1C[0]=({
struct Cyc_Absyn_Eq_constr_struct _tmp1D;_tmp1D.tag=0;_tmp1D.f1=(void*)x;_tmp1D;});
_tmp1C;}));_tmp1B;});}struct Cyc_Absyn_Conref*Cyc_Absyn_empty_conref(){return({
struct Cyc_Absyn_Conref*_tmp1E=_cycalloc(sizeof(*_tmp1E));_tmp1E->v=(void*)((void*)
0);_tmp1E;});}static struct Cyc_Absyn_Eq_constr_struct Cyc_Absyn_true_constraint={0,(
void*)1};static struct Cyc_Absyn_Eq_constr_struct Cyc_Absyn_false_constraint={0,(
void*)0};struct Cyc_Absyn_Conref Cyc_Absyn_true_conref_v={(void*)((void*)& Cyc_Absyn_true_constraint)};
struct Cyc_Absyn_Conref Cyc_Absyn_false_conref_v={(void*)((void*)& Cyc_Absyn_false_constraint)};
struct Cyc_Absyn_Conref*Cyc_Absyn_true_conref=& Cyc_Absyn_true_conref_v;struct Cyc_Absyn_Conref*
Cyc_Absyn_false_conref=& Cyc_Absyn_false_conref_v;struct Cyc_Absyn_Conref*Cyc_Absyn_compress_conref(
struct Cyc_Absyn_Conref*x){void*_tmp21=(void*)x->v;struct Cyc_Absyn_Conref*_tmp22;
struct Cyc_Absyn_Conref**_tmp23;_LL19: if((int)_tmp21 != 0)goto _LL1B;_LL1A: goto
_LL1C;_LL1B: if(_tmp21 <= (void*)1?1:*((int*)_tmp21)!= 0)goto _LL1D;_LL1C: return x;
_LL1D: if(_tmp21 <= (void*)1?1:*((int*)_tmp21)!= 1)goto _LL18;_tmp22=((struct Cyc_Absyn_Forward_constr_struct*)
_tmp21)->f1;_tmp23=(struct Cyc_Absyn_Conref**)&((struct Cyc_Absyn_Forward_constr_struct*)
_tmp21)->f1;_LL1E: {struct Cyc_Absyn_Conref*_tmp24=Cyc_Absyn_compress_conref(*
_tmp23);*_tmp23=_tmp24;return _tmp24;}_LL18:;}void*Cyc_Absyn_conref_val(struct Cyc_Absyn_Conref*
x){void*_tmp25=(void*)(Cyc_Absyn_compress_conref(x))->v;void*_tmp26;_LL20: if(
_tmp25 <= (void*)1?1:*((int*)_tmp25)!= 0)goto _LL22;_tmp26=(void*)((struct Cyc_Absyn_Eq_constr_struct*)
_tmp25)->f1;_LL21: return _tmp26;_LL22:;_LL23:({void*_tmp27[0]={};((int(*)(struct
_tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("conref_val",
sizeof(unsigned char),11),_tag_arr(_tmp27,sizeof(void*),0));});_LL1F:;}void*Cyc_Absyn_conref_def(
void*y,struct Cyc_Absyn_Conref*x){void*_tmp28=(void*)(Cyc_Absyn_compress_conref(x))->v;
void*_tmp29;_LL25: if(_tmp28 <= (void*)1?1:*((int*)_tmp28)!= 0)goto _LL27;_tmp29=(
void*)((struct Cyc_Absyn_Eq_constr_struct*)_tmp28)->f1;_LL26: return _tmp29;_LL27:;
_LL28: return y;_LL24:;}void*Cyc_Absyn_compress_kb(void*k){void*_tmp2A=k;struct Cyc_Core_Opt*
_tmp2B;struct Cyc_Core_Opt*_tmp2C;struct Cyc_Core_Opt*_tmp2D;struct Cyc_Core_Opt
_tmp2E;void*_tmp2F;void**_tmp30;struct Cyc_Core_Opt*_tmp31;struct Cyc_Core_Opt
_tmp32;void*_tmp33;void**_tmp34;_LL2A: if(*((int*)_tmp2A)!= 0)goto _LL2C;_LL2B:
goto _LL2D;_LL2C: if(*((int*)_tmp2A)!= 1)goto _LL2E;_tmp2B=((struct Cyc_Absyn_Unknown_kb_struct*)
_tmp2A)->f1;if(_tmp2B != 0)goto _LL2E;_LL2D: goto _LL2F;_LL2E: if(*((int*)_tmp2A)!= 2)
goto _LL30;_tmp2C=((struct Cyc_Absyn_Less_kb_struct*)_tmp2A)->f1;if(_tmp2C != 0)
goto _LL30;_LL2F: return k;_LL30: if(*((int*)_tmp2A)!= 1)goto _LL32;_tmp2D=((struct
Cyc_Absyn_Unknown_kb_struct*)_tmp2A)->f1;if(_tmp2D == 0)goto _LL32;_tmp2E=*_tmp2D;
_tmp2F=(void*)_tmp2E.v;_tmp30=(void**)&(*((struct Cyc_Absyn_Unknown_kb_struct*)
_tmp2A)->f1).v;_LL31: _tmp34=_tmp30;goto _LL33;_LL32: if(*((int*)_tmp2A)!= 2)goto
_LL29;_tmp31=((struct Cyc_Absyn_Less_kb_struct*)_tmp2A)->f1;if(_tmp31 == 0)goto
_LL29;_tmp32=*_tmp31;_tmp33=(void*)_tmp32.v;_tmp34=(void**)&(*((struct Cyc_Absyn_Less_kb_struct*)
_tmp2A)->f1).v;_LL33:*_tmp34=Cyc_Absyn_compress_kb(*_tmp34);return*_tmp34;_LL29:;}
void*Cyc_Absyn_force_kb(void*kb){void*_tmp35=Cyc_Absyn_compress_kb(kb);void*
_tmp36;struct Cyc_Core_Opt*_tmp37;struct Cyc_Core_Opt**_tmp38;struct Cyc_Core_Opt*
_tmp39;struct Cyc_Core_Opt**_tmp3A;void*_tmp3B;_LL35: if(*((int*)_tmp35)!= 0)goto
_LL37;_tmp36=(void*)((struct Cyc_Absyn_Eq_kb_struct*)_tmp35)->f1;_LL36: return
_tmp36;_LL37: if(*((int*)_tmp35)!= 1)goto _LL39;_tmp37=((struct Cyc_Absyn_Unknown_kb_struct*)
_tmp35)->f1;_tmp38=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Unknown_kb_struct*)
_tmp35)->f1;_LL38: _tmp3A=_tmp38;_tmp3B=(void*)2;goto _LL3A;_LL39: if(*((int*)
_tmp35)!= 2)goto _LL34;_tmp39=((struct Cyc_Absyn_Less_kb_struct*)_tmp35)->f1;
_tmp3A=(struct Cyc_Core_Opt**)&((struct Cyc_Absyn_Less_kb_struct*)_tmp35)->f1;
_tmp3B=(void*)((struct Cyc_Absyn_Less_kb_struct*)_tmp35)->f2;_LL3A:*_tmp3A=({
struct Cyc_Core_Opt*_tmp3C=_cycalloc(sizeof(*_tmp3C));_tmp3C->v=(void*)((void*)({
struct Cyc_Absyn_Eq_kb_struct*_tmp3D=_cycalloc(sizeof(*_tmp3D));_tmp3D[0]=({
struct Cyc_Absyn_Eq_kb_struct _tmp3E;_tmp3E.tag=0;_tmp3E.f1=(void*)_tmp3B;_tmp3E;});
_tmp3D;}));_tmp3C;});return _tmp3B;_LL34:;}static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_uchar_tt={5,(void*)((void*)1),(void*)((void*)0)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_ushort_tt={5,(void*)((void*)1),(void*)((void*)1)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_uint_tt={5,(void*)((void*)1),(void*)((void*)2)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_ulong_tt={5,(void*)((void*)1),(void*)((void*)2)};static struct Cyc_Absyn_IntType_struct
Cyc_Absyn_ulonglong_tt={5,(void*)((void*)1),(void*)((void*)3)};void*Cyc_Absyn_uchar_t=(
void*)& Cyc_Absyn_uchar_tt;void*Cyc_Absyn_ushort_t=(void*)& Cyc_Absyn_ushort_tt;
void*Cyc_Absyn_uint_t=(void*)& Cyc_Absyn_uint_tt;void*Cyc_Absyn_ulong_t=(void*)&
Cyc_Absyn_ulong_tt;void*Cyc_Absyn_ulonglong_t=(void*)& Cyc_Absyn_ulonglong_tt;
static struct Cyc_Absyn_IntType_struct Cyc_Absyn_schar_tt={5,(void*)((void*)0),(
void*)((void*)0)};static struct Cyc_Absyn_IntType_struct Cyc_Absyn_sshort_tt={5,(
void*)((void*)0),(void*)((void*)1)};static struct Cyc_Absyn_IntType_struct Cyc_Absyn_sint_tt={
5,(void*)((void*)0),(void*)((void*)2)};static struct Cyc_Absyn_IntType_struct Cyc_Absyn_slong_tt={
5,(void*)((void*)0),(void*)((void*)2)};static struct Cyc_Absyn_IntType_struct Cyc_Absyn_slonglong_tt={
5,(void*)((void*)0),(void*)((void*)3)};void*Cyc_Absyn_schar_t=(void*)& Cyc_Absyn_schar_tt;
void*Cyc_Absyn_sshort_t=(void*)& Cyc_Absyn_sshort_tt;void*Cyc_Absyn_sint_t=(void*)&
Cyc_Absyn_sint_tt;void*Cyc_Absyn_slong_t=(void*)& Cyc_Absyn_slong_tt;void*Cyc_Absyn_slonglong_t=(
void*)& Cyc_Absyn_slonglong_tt;void*Cyc_Absyn_float_typ=(void*)1;void*Cyc_Absyn_double_typ(
int b){return(void*)({struct Cyc_Absyn_DoubleType_struct*_tmp49=_cycalloc_atomic(
sizeof(*_tmp49));_tmp49[0]=({struct Cyc_Absyn_DoubleType_struct _tmp4A;_tmp4A.tag=
6;_tmp4A.f1=b;_tmp4A;});_tmp49;});}static struct Cyc_Absyn_Abs_n_struct Cyc_Absyn_abs_null={
1,0};static unsigned char _tmp4C[4]="exn";static struct _tagged_arr Cyc_Absyn_exn_str={
_tmp4C,_tmp4C,_tmp4C + 4};static struct _tuple0 Cyc_Absyn_exn_name_v={(void*)& Cyc_Absyn_abs_null,&
Cyc_Absyn_exn_str};struct _tuple0*Cyc_Absyn_exn_name=& Cyc_Absyn_exn_name_v;static
unsigned char _tmp4D[15]="Null_Exception";static struct _tagged_arr Cyc_Absyn_Null_Exception_str={
_tmp4D,_tmp4D,_tmp4D + 15};static struct _tuple0 Cyc_Absyn_Null_Exception_pr={(void*)&
Cyc_Absyn_abs_null,& Cyc_Absyn_Null_Exception_str};struct _tuple0*Cyc_Absyn_Null_Exception_name=&
Cyc_Absyn_Null_Exception_pr;static struct Cyc_Absyn_Tunionfield Cyc_Absyn_Null_Exception_tuf_v={&
Cyc_Absyn_Null_Exception_pr,0,0,(void*)((void*)3)};struct Cyc_Absyn_Tunionfield*
Cyc_Absyn_Null_Exception_tuf=& Cyc_Absyn_Null_Exception_tuf_v;static unsigned char
_tmp4E[13]="Array_bounds";static struct _tagged_arr Cyc_Absyn_Array_bounds_str={
_tmp4E,_tmp4E,_tmp4E + 13};static struct _tuple0 Cyc_Absyn_Array_bounds_pr={(void*)&
Cyc_Absyn_abs_null,& Cyc_Absyn_Array_bounds_str};struct _tuple0*Cyc_Absyn_Array_bounds_name=&
Cyc_Absyn_Array_bounds_pr;static struct Cyc_Absyn_Tunionfield Cyc_Absyn_Array_bounds_tuf_v={&
Cyc_Absyn_Array_bounds_pr,0,0,(void*)((void*)3)};struct Cyc_Absyn_Tunionfield*Cyc_Absyn_Array_bounds_tuf=&
Cyc_Absyn_Array_bounds_tuf_v;static unsigned char _tmp4F[16]="Match_Exception";
static struct _tagged_arr Cyc_Absyn_Match_Exception_str={_tmp4F,_tmp4F,_tmp4F + 16};
static struct _tuple0 Cyc_Absyn_Match_Exception_pr={(void*)& Cyc_Absyn_abs_null,& Cyc_Absyn_Match_Exception_str};
struct _tuple0*Cyc_Absyn_Match_Exception_name=& Cyc_Absyn_Match_Exception_pr;
static struct Cyc_Absyn_Tunionfield Cyc_Absyn_Match_Exception_tuf_v={& Cyc_Absyn_Match_Exception_pr,
0,0,(void*)((void*)3)};struct Cyc_Absyn_Tunionfield*Cyc_Absyn_Match_Exception_tuf=&
Cyc_Absyn_Match_Exception_tuf_v;static unsigned char _tmp50[10]="Bad_alloc";static
struct _tagged_arr Cyc_Absyn_Bad_alloc_str={_tmp50,_tmp50,_tmp50 + 10};static struct
_tuple0 Cyc_Absyn_Bad_alloc_pr={(void*)& Cyc_Absyn_abs_null,& Cyc_Absyn_Bad_alloc_str};
struct _tuple0*Cyc_Absyn_Bad_alloc_name=& Cyc_Absyn_Bad_alloc_pr;static struct Cyc_Absyn_Tunionfield
Cyc_Absyn_Bad_alloc_tuf_v={& Cyc_Absyn_Bad_alloc_pr,0,0,(void*)((void*)3)};struct
Cyc_Absyn_Tunionfield*Cyc_Absyn_Bad_alloc_tuf=& Cyc_Absyn_Bad_alloc_tuf_v;static
struct Cyc_List_List Cyc_Absyn_exn_l0={(void*)& Cyc_Absyn_Null_Exception_tuf_v,0};
static struct Cyc_List_List Cyc_Absyn_exn_l1={(void*)& Cyc_Absyn_Array_bounds_tuf_v,(
struct Cyc_List_List*)& Cyc_Absyn_exn_l0};static struct Cyc_List_List Cyc_Absyn_exn_l2={(
void*)& Cyc_Absyn_Match_Exception_tuf_v,(struct Cyc_List_List*)& Cyc_Absyn_exn_l1};
static struct Cyc_List_List Cyc_Absyn_exn_l3={(void*)& Cyc_Absyn_Bad_alloc_tuf_v,(
struct Cyc_List_List*)& Cyc_Absyn_exn_l2};static struct Cyc_Core_Opt Cyc_Absyn_exn_ol={(
void*)((struct Cyc_List_List*)& Cyc_Absyn_exn_l3)};static struct Cyc_Absyn_Tuniondecl
Cyc_Absyn_exn_tud_v={(void*)((void*)3),& Cyc_Absyn_exn_name_v,0,(struct Cyc_Core_Opt*)&
Cyc_Absyn_exn_ol,1};struct Cyc_Absyn_Tuniondecl*Cyc_Absyn_exn_tud=& Cyc_Absyn_exn_tud_v;
static struct Cyc_Absyn_KnownTunion_struct Cyc_Absyn_exn_tinfou={1,& Cyc_Absyn_exn_tud};
static struct Cyc_Absyn_TunionType_struct Cyc_Absyn_exn_typ_tt={2,{(void*)((void*)&
Cyc_Absyn_exn_tinfou),0,(void*)((void*)2)}};void*Cyc_Absyn_exn_typ=(void*)& Cyc_Absyn_exn_typ_tt;
static unsigned char _tmp53[9]="PrintArg";static struct _tagged_arr Cyc_Absyn_printarg_str={
_tmp53,_tmp53,_tmp53 + 9};static unsigned char _tmp54[9]="ScanfArg";static struct
_tagged_arr Cyc_Absyn_scanfarg_str={_tmp54,_tmp54,_tmp54 + 9};static unsigned char
_tmp55[4]="Std";static struct _tagged_arr Cyc_Absyn_std_str={_tmp55,_tmp55,_tmp55 + 
4};static struct Cyc_List_List Cyc_Absyn_std_list={(void*)& Cyc_Absyn_std_str,0};
static struct Cyc_Absyn_Abs_n_struct Cyc_Absyn_std_namespace={1,(struct Cyc_List_List*)&
Cyc_Absyn_std_list};static struct _tuple0 Cyc_Absyn_tunion_print_arg_qvar_p={(void*)&
Cyc_Absyn_std_namespace,& Cyc_Absyn_printarg_str};static struct _tuple0 Cyc_Absyn_tunion_scanf_arg_qvar_p={(
void*)& Cyc_Absyn_std_namespace,& Cyc_Absyn_scanfarg_str};struct _tuple0*Cyc_Absyn_tunion_print_arg_qvar=&
Cyc_Absyn_tunion_print_arg_qvar_p;struct _tuple0*Cyc_Absyn_tunion_scanf_arg_qvar=&
Cyc_Absyn_tunion_scanf_arg_qvar_p;static struct Cyc_Core_Opt*Cyc_Absyn_string_t_opt=
0;void*Cyc_Absyn_string_typ(void*rgn){if(rgn != (void*)2)return Cyc_Absyn_starb_typ(
Cyc_Absyn_uchar_t,rgn,Cyc_Absyn_empty_tqual(),(void*)0);else{if(Cyc_Absyn_string_t_opt
== 0){void*t=Cyc_Absyn_starb_typ(Cyc_Absyn_uchar_t,(void*)2,Cyc_Absyn_empty_tqual(),(
void*)0);Cyc_Absyn_string_t_opt=({struct Cyc_Core_Opt*_tmp57=_cycalloc(sizeof(*
_tmp57));_tmp57->v=(void*)t;_tmp57;});}return(void*)((struct Cyc_Core_Opt*)
_check_null(Cyc_Absyn_string_t_opt))->v;}}static struct Cyc_Core_Opt*Cyc_Absyn_const_string_t_opt=
0;void*Cyc_Absyn_const_string_typ(void*rgn){if(rgn != (void*)2)return Cyc_Absyn_starb_typ(
Cyc_Absyn_uchar_t,rgn,Cyc_Absyn_const_tqual(),(void*)0);else{if(Cyc_Absyn_const_string_t_opt
== 0){void*t=Cyc_Absyn_starb_typ(Cyc_Absyn_uchar_t,(void*)2,Cyc_Absyn_const_tqual(),(
void*)0);Cyc_Absyn_const_string_t_opt=({struct Cyc_Core_Opt*_tmp58=_cycalloc(
sizeof(*_tmp58));_tmp58->v=(void*)t;_tmp58;});}return(void*)((struct Cyc_Core_Opt*)
_check_null(Cyc_Absyn_const_string_t_opt))->v;}}static struct Cyc_Absyn_Int_c_struct
Cyc_Absyn_one_intc={2,(void*)((void*)1),1};static struct Cyc_Absyn_Const_e_struct
Cyc_Absyn_one_b_raw={0,(void*)((void*)& Cyc_Absyn_one_intc)};struct Cyc_Absyn_Exp
Cyc_Absyn_exp_unsigned_one_v={0,(void*)((void*)& Cyc_Absyn_one_b_raw),0,(void*)((
void*)Cyc_Absyn_EmptyAnnot)};struct Cyc_Absyn_Exp*Cyc_Absyn_exp_unsigned_one=& Cyc_Absyn_exp_unsigned_one_v;
static struct Cyc_Absyn_Upper_b_struct Cyc_Absyn_one_bt={0,& Cyc_Absyn_exp_unsigned_one_v};
void*Cyc_Absyn_bounds_one=(void*)& Cyc_Absyn_one_bt;void*Cyc_Absyn_starb_typ(void*
t,void*r,struct Cyc_Absyn_Tqual tq,void*b){return(void*)({struct Cyc_Absyn_PointerType_struct*
_tmp5C=_cycalloc(sizeof(*_tmp5C));_tmp5C[0]=({struct Cyc_Absyn_PointerType_struct
_tmp5D;_tmp5D.tag=4;_tmp5D.f1=({struct Cyc_Absyn_PtrInfo _tmp5E;_tmp5E.elt_typ=(
void*)t;_tmp5E.rgn_typ=(void*)r;_tmp5E.nullable=Cyc_Absyn_true_conref;_tmp5E.tq=
tq;_tmp5E.bounds=Cyc_Absyn_new_conref(b);_tmp5E;});_tmp5D;});_tmp5C;});}void*Cyc_Absyn_atb_typ(
void*t,void*r,struct Cyc_Absyn_Tqual tq,void*b){return(void*)({struct Cyc_Absyn_PointerType_struct*
_tmp5F=_cycalloc(sizeof(*_tmp5F));_tmp5F[0]=({struct Cyc_Absyn_PointerType_struct
_tmp60;_tmp60.tag=4;_tmp60.f1=({struct Cyc_Absyn_PtrInfo _tmp61;_tmp61.elt_typ=(
void*)t;_tmp61.rgn_typ=(void*)r;_tmp61.nullable=Cyc_Absyn_false_conref;_tmp61.tq=
tq;_tmp61.bounds=Cyc_Absyn_new_conref(b);_tmp61;});_tmp60;});_tmp5F;});}void*Cyc_Absyn_star_typ(
void*t,void*r,struct Cyc_Absyn_Tqual tq){return Cyc_Absyn_starb_typ(t,r,tq,Cyc_Absyn_bounds_one);}
void*Cyc_Absyn_cstar_typ(void*t,struct Cyc_Absyn_Tqual tq){return Cyc_Absyn_starb_typ(
t,(void*)2,tq,Cyc_Absyn_bounds_one);}void*Cyc_Absyn_at_typ(void*t,void*r,struct
Cyc_Absyn_Tqual tq){return Cyc_Absyn_atb_typ(t,r,tq,Cyc_Absyn_bounds_one);}void*
Cyc_Absyn_tagged_typ(void*t,void*r,struct Cyc_Absyn_Tqual tq){return(void*)({
struct Cyc_Absyn_PointerType_struct*_tmp62=_cycalloc(sizeof(*_tmp62));_tmp62[0]=({
struct Cyc_Absyn_PointerType_struct _tmp63;_tmp63.tag=4;_tmp63.f1=({struct Cyc_Absyn_PtrInfo
_tmp64;_tmp64.elt_typ=(void*)t;_tmp64.rgn_typ=(void*)r;_tmp64.nullable=Cyc_Absyn_true_conref;
_tmp64.tq=tq;_tmp64.bounds=Cyc_Absyn_new_conref((void*)0);_tmp64;});_tmp63;});
_tmp62;});}static struct Cyc_Core_Opt*Cyc_Absyn_file_t_opt=0;static unsigned char
_tmp65[8]="__sFILE";static struct _tagged_arr Cyc_Absyn_sf_str={_tmp65,_tmp65,
_tmp65 + 8};static unsigned char _tmp66[4]="Std";static struct _tagged_arr Cyc_Absyn_st_str={
_tmp66,_tmp66,_tmp66 + 4};static struct _tagged_arr*Cyc_Absyn_sf=& Cyc_Absyn_sf_str;
static struct _tagged_arr*Cyc_Absyn_st=& Cyc_Absyn_st_str;void*Cyc_Absyn_file_typ(){
if(Cyc_Absyn_file_t_opt == 0){struct _tuple0*file_t_name=({struct _tuple0*_tmp6F=
_cycalloc(sizeof(*_tmp6F));_tmp6F->f1=(void*)({struct Cyc_Absyn_Abs_n_struct*
_tmp70=_cycalloc(sizeof(*_tmp70));_tmp70[0]=({struct Cyc_Absyn_Abs_n_struct _tmp71;
_tmp71.tag=1;_tmp71.f1=({struct _tagged_arr*_tmp72[1];_tmp72[0]=Cyc_Absyn_st;((
struct Cyc_List_List*(*)(struct _tagged_arr))Cyc_List_list)(_tag_arr(_tmp72,
sizeof(struct _tagged_arr*),1));});_tmp71;});_tmp70;});_tmp6F->f2=Cyc_Absyn_sf;
_tmp6F;});struct Cyc_Absyn_Aggrdecl*sd=({struct Cyc_Absyn_Aggrdecl*_tmp6E=
_cycalloc(sizeof(*_tmp6E));_tmp6E->kind=(void*)((void*)0);_tmp6E->sc=(void*)((
void*)1);_tmp6E->name=file_t_name;_tmp6E->tvs=0;_tmp6E->exist_vars=0;_tmp6E->fields=
0;_tmp6E->attributes=0;_tmp6E;});void*file_struct_typ=(void*)({struct Cyc_Absyn_AggrType_struct*
_tmp68=_cycalloc(sizeof(*_tmp68));_tmp68[0]=({struct Cyc_Absyn_AggrType_struct
_tmp69;_tmp69.tag=10;_tmp69.f1=({struct Cyc_Absyn_AggrInfo _tmp6A;_tmp6A.aggr_info=(
void*)((void*)({struct Cyc_Absyn_KnownAggr_struct*_tmp6B=_cycalloc(sizeof(*_tmp6B));
_tmp6B[0]=({struct Cyc_Absyn_KnownAggr_struct _tmp6C;_tmp6C.tag=1;_tmp6C.f1=({
struct Cyc_Absyn_Aggrdecl**_tmp6D=_cycalloc(sizeof(*_tmp6D));_tmp6D[0]=sd;_tmp6D;});
_tmp6C;});_tmp6B;}));_tmp6A.targs=0;_tmp6A;});_tmp69;});_tmp68;});Cyc_Absyn_file_t_opt=({
struct Cyc_Core_Opt*_tmp67=_cycalloc(sizeof(*_tmp67));_tmp67->v=(void*)Cyc_Absyn_at_typ(
file_struct_typ,(void*)2,Cyc_Absyn_empty_tqual());_tmp67;});}return(void*)((
struct Cyc_Core_Opt*)_check_null(Cyc_Absyn_file_t_opt))->v;}static struct Cyc_Core_Opt*
Cyc_Absyn_void_star_t_opt=0;void*Cyc_Absyn_void_star_typ(){if(Cyc_Absyn_void_star_t_opt
== 0)Cyc_Absyn_void_star_t_opt=({struct Cyc_Core_Opt*_tmp73=_cycalloc(sizeof(*
_tmp73));_tmp73->v=(void*)Cyc_Absyn_star_typ((void*)0,(void*)2,Cyc_Absyn_empty_tqual());
_tmp73;});return(void*)((struct Cyc_Core_Opt*)_check_null(Cyc_Absyn_void_star_t_opt))->v;}
struct Cyc_Core_Opt*Cyc_Absyn_void_star_typ_opt(){if(Cyc_Absyn_void_star_t_opt == 
0)Cyc_Absyn_void_star_t_opt=({struct Cyc_Core_Opt*_tmp74=_cycalloc(sizeof(*_tmp74));
_tmp74->v=(void*)Cyc_Absyn_star_typ((void*)0,(void*)2,Cyc_Absyn_empty_tqual());
_tmp74;});return Cyc_Absyn_void_star_t_opt;}static struct Cyc_Absyn_JoinEff_struct
Cyc_Absyn_empty_eff={18,0};void*Cyc_Absyn_empty_effect=(void*)& Cyc_Absyn_empty_eff;
void*Cyc_Absyn_aggr_typ(void*k,struct _tagged_arr*name){return(void*)({struct Cyc_Absyn_AggrType_struct*
_tmp76=_cycalloc(sizeof(*_tmp76));_tmp76[0]=({struct Cyc_Absyn_AggrType_struct
_tmp77;_tmp77.tag=10;_tmp77.f1=({struct Cyc_Absyn_AggrInfo _tmp78;_tmp78.aggr_info=(
void*)((void*)({struct Cyc_Absyn_UnknownAggr_struct*_tmp79=_cycalloc(sizeof(*
_tmp79));_tmp79[0]=({struct Cyc_Absyn_UnknownAggr_struct _tmp7A;_tmp7A.tag=0;
_tmp7A.f1=(void*)k;_tmp7A.f2=({struct _tuple0*_tmp7B=_cycalloc(sizeof(*_tmp7B));
_tmp7B->f1=Cyc_Absyn_rel_ns_null;_tmp7B->f2=name;_tmp7B;});_tmp7A;});_tmp79;}));
_tmp78.targs=0;_tmp78;});_tmp77;});_tmp76;});}void*Cyc_Absyn_strct(struct
_tagged_arr*name){return Cyc_Absyn_aggr_typ((void*)0,name);}void*Cyc_Absyn_union_typ(
struct _tagged_arr*name){return Cyc_Absyn_aggr_typ((void*)1,name);}void*Cyc_Absyn_strctq(
struct _tuple0*name){return(void*)({struct Cyc_Absyn_AggrType_struct*_tmp7C=
_cycalloc(sizeof(*_tmp7C));_tmp7C[0]=({struct Cyc_Absyn_AggrType_struct _tmp7D;
_tmp7D.tag=10;_tmp7D.f1=({struct Cyc_Absyn_AggrInfo _tmp7E;_tmp7E.aggr_info=(void*)((
void*)({struct Cyc_Absyn_UnknownAggr_struct*_tmp7F=_cycalloc(sizeof(*_tmp7F));
_tmp7F[0]=({struct Cyc_Absyn_UnknownAggr_struct _tmp80;_tmp80.tag=0;_tmp80.f1=(
void*)((void*)0);_tmp80.f2=name;_tmp80;});_tmp7F;}));_tmp7E.targs=0;_tmp7E;});
_tmp7D;});_tmp7C;});}void*Cyc_Absyn_unionq_typ(struct _tuple0*name){return(void*)({
struct Cyc_Absyn_AggrType_struct*_tmp81=_cycalloc(sizeof(*_tmp81));_tmp81[0]=({
struct Cyc_Absyn_AggrType_struct _tmp82;_tmp82.tag=10;_tmp82.f1=({struct Cyc_Absyn_AggrInfo
_tmp83;_tmp83.aggr_info=(void*)((void*)({struct Cyc_Absyn_UnknownAggr_struct*
_tmp84=_cycalloc(sizeof(*_tmp84));_tmp84[0]=({struct Cyc_Absyn_UnknownAggr_struct
_tmp85;_tmp85.tag=0;_tmp85.f1=(void*)((void*)1);_tmp85.f2=name;_tmp85;});_tmp84;}));
_tmp83.targs=0;_tmp83;});_tmp82;});_tmp81;});}struct Cyc_Absyn_Exp*Cyc_Absyn_new_exp(
void*r,struct Cyc_Position_Segment*loc){return({struct Cyc_Absyn_Exp*_tmp86=
_cycalloc(sizeof(*_tmp86));_tmp86->topt=0;_tmp86->r=(void*)r;_tmp86->loc=loc;
_tmp86->annot=(void*)((void*)Cyc_Absyn_EmptyAnnot);_tmp86;});}struct Cyc_Absyn_Exp*
Cyc_Absyn_New_exp(struct Cyc_Absyn_Exp*rgn_handle,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_New_e_struct*_tmp87=
_cycalloc(sizeof(*_tmp87));_tmp87[0]=({struct Cyc_Absyn_New_e_struct _tmp88;_tmp88.tag=
15;_tmp88.f1=rgn_handle;_tmp88.f2=e;_tmp88;});_tmp87;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_copy_exp(struct Cyc_Absyn_Exp*e){return({struct Cyc_Absyn_Exp*_tmp89=
_cycalloc(sizeof(*_tmp89));_tmp89[0]=*e;_tmp89;});}struct Cyc_Absyn_Exp*Cyc_Absyn_const_exp(
void*c,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct
Cyc_Absyn_Const_e_struct*_tmp8A=_cycalloc(sizeof(*_tmp8A));_tmp8A[0]=({struct Cyc_Absyn_Const_e_struct
_tmp8B;_tmp8B.tag=0;_tmp8B.f1=(void*)c;_tmp8B;});_tmp8A;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_null_exp(struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_Const_e_struct*_tmp8C=_cycalloc(sizeof(*_tmp8C));_tmp8C[0]=({
struct Cyc_Absyn_Const_e_struct _tmp8D;_tmp8D.tag=0;_tmp8D.f1=(void*)((void*)0);
_tmp8D;});_tmp8C;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_int_exp(void*s,int i,
struct Cyc_Position_Segment*seg){return Cyc_Absyn_const_exp((void*)({struct Cyc_Absyn_Int_c_struct*
_tmp8E=_cycalloc(sizeof(*_tmp8E));_tmp8E[0]=({struct Cyc_Absyn_Int_c_struct _tmp8F;
_tmp8F.tag=2;_tmp8F.f1=(void*)s;_tmp8F.f2=i;_tmp8F;});_tmp8E;}),seg);}struct Cyc_Absyn_Exp*
Cyc_Absyn_signed_int_exp(int i,struct Cyc_Position_Segment*loc){return Cyc_Absyn_int_exp((
void*)0,i,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_uint_exp(unsigned int i,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_int_exp((void*)1,(int)i,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_bool_exp(
int b,struct Cyc_Position_Segment*loc){return Cyc_Absyn_signed_int_exp(b?1: 0,loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_true_exp(struct Cyc_Position_Segment*loc){return Cyc_Absyn_bool_exp(
1,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_false_exp(struct Cyc_Position_Segment*loc){
return Cyc_Absyn_bool_exp(0,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_char_exp(
unsigned char c,struct Cyc_Position_Segment*loc){return Cyc_Absyn_const_exp((void*)({
struct Cyc_Absyn_Char_c_struct*_tmp90=_cycalloc(sizeof(*_tmp90));_tmp90[0]=({
struct Cyc_Absyn_Char_c_struct _tmp91;_tmp91.tag=0;_tmp91.f1=(void*)((void*)1);
_tmp91.f2=c;_tmp91;});_tmp90;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_float_exp(
struct _tagged_arr f,struct Cyc_Position_Segment*loc){return Cyc_Absyn_const_exp((
void*)({struct Cyc_Absyn_Float_c_struct*_tmp92=_cycalloc(sizeof(*_tmp92));_tmp92[
0]=({struct Cyc_Absyn_Float_c_struct _tmp93;_tmp93.tag=4;_tmp93.f1=f;_tmp93;});
_tmp92;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_string_exp(struct _tagged_arr s,
struct Cyc_Position_Segment*loc){return Cyc_Absyn_const_exp((void*)({struct Cyc_Absyn_String_c_struct*
_tmp94=_cycalloc(sizeof(*_tmp94));_tmp94[0]=({struct Cyc_Absyn_String_c_struct
_tmp95;_tmp95.tag=5;_tmp95.f1=s;_tmp95;});_tmp94;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_var_exp(struct _tuple0*q,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_Var_e_struct*_tmp96=_cycalloc(sizeof(*_tmp96));_tmp96[0]=({
struct Cyc_Absyn_Var_e_struct _tmp97;_tmp97.tag=1;_tmp97.f1=q;_tmp97.f2=(void*)((
void*)0);_tmp97;});_tmp96;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_varb_exp(struct
_tuple0*q,void*b,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_Var_e_struct*_tmp98=_cycalloc(sizeof(*_tmp98));_tmp98[0]=({
struct Cyc_Absyn_Var_e_struct _tmp99;_tmp99.tag=1;_tmp99.f1=q;_tmp99.f2=(void*)b;
_tmp99;});_tmp98;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_unknownid_exp(struct
_tuple0*q,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_UnknownId_e_struct*_tmp9A=_cycalloc(sizeof(*_tmp9A));_tmp9A[0]=({
struct Cyc_Absyn_UnknownId_e_struct _tmp9B;_tmp9B.tag=2;_tmp9B.f1=q;_tmp9B;});
_tmp9A;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_primop_exp(void*p,struct Cyc_List_List*
es,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Primop_e_struct*
_tmp9C=_cycalloc(sizeof(*_tmp9C));_tmp9C[0]=({struct Cyc_Absyn_Primop_e_struct
_tmp9D;_tmp9D.tag=3;_tmp9D.f1=(void*)p;_tmp9D.f2=es;_tmp9D;});_tmp9C;}),loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_prim1_exp(void*p,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_primop_exp(p,({struct Cyc_List_List*_tmp9E=_cycalloc(sizeof(*
_tmp9E));_tmp9E->hd=e;_tmp9E->tl=0;_tmp9E;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_prim2_exp(
void*p,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_primop_exp(p,({struct Cyc_List_List*_tmp9F=_cycalloc(sizeof(*
_tmp9F));_tmp9F->hd=e1;_tmp9F->tl=({struct Cyc_List_List*_tmpA0=_cycalloc(sizeof(*
_tmpA0));_tmpA0->hd=e2;_tmpA0->tl=0;_tmpA0;});_tmp9F;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_add_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_prim2_exp((void*)0,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_times_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)1,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_divide_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)3,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_eq_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)5,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_neq_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)6,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_gt_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)7,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_lt_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)8,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_gte_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)9,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_lte_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_prim2_exp((void*)10,e1,e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_assignop_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Core_Opt*popt,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_AssignOp_e_struct*_tmpA1=
_cycalloc(sizeof(*_tmpA1));_tmpA1[0]=({struct Cyc_Absyn_AssignOp_e_struct _tmpA2;
_tmpA2.tag=4;_tmpA2.f1=e1;_tmpA2.f2=popt;_tmpA2.f3=e2;_tmpA2;});_tmpA1;}),loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_assign_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Position_Segment*loc){return Cyc_Absyn_assignop_exp(e1,0,e2,loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_increment_exp(struct Cyc_Absyn_Exp*e,void*i,struct
Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Increment_e_struct*
_tmpA3=_cycalloc(sizeof(*_tmpA3));_tmpA3[0]=({struct Cyc_Absyn_Increment_e_struct
_tmpA4;_tmpA4.tag=5;_tmpA4.f1=e;_tmpA4.f2=(void*)i;_tmpA4;});_tmpA3;}),loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_post_inc_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_increment_exp(e,(void*)1,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_pre_inc_exp(
struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_increment_exp(
e,(void*)0,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_pre_dec_exp(struct Cyc_Absyn_Exp*e,
struct Cyc_Position_Segment*loc){return Cyc_Absyn_increment_exp(e,(void*)2,loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_post_dec_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_increment_exp(e,(void*)3,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_conditional_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Absyn_Exp*e3,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Conditional_e_struct*
_tmpA5=_cycalloc(sizeof(*_tmpA5));_tmpA5[0]=({struct Cyc_Absyn_Conditional_e_struct
_tmpA6;_tmpA6.tag=6;_tmpA6.f1=e1;_tmpA6.f2=e2;_tmpA6.f3=e3;_tmpA6;});_tmpA5;}),
loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_and_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Position_Segment*loc){return Cyc_Absyn_conditional_exp(e1,e2,Cyc_Absyn_false_exp(
loc),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_or_exp(struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*
e2,struct Cyc_Position_Segment*loc){return Cyc_Absyn_conditional_exp(e1,Cyc_Absyn_true_exp(
loc),e2,loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_seq_exp(struct Cyc_Absyn_Exp*e1,
struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_SeqExp_e_struct*_tmpA7=_cycalloc(sizeof(*_tmpA7));_tmpA7[
0]=({struct Cyc_Absyn_SeqExp_e_struct _tmpA8;_tmpA8.tag=7;_tmpA8.f1=e1;_tmpA8.f2=
e2;_tmpA8;});_tmpA7;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_unknowncall_exp(
struct Cyc_Absyn_Exp*e,struct Cyc_List_List*es,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_UnknownCall_e_struct*_tmpA9=
_cycalloc(sizeof(*_tmpA9));_tmpA9[0]=({struct Cyc_Absyn_UnknownCall_e_struct
_tmpAA;_tmpAA.tag=8;_tmpAA.f1=e;_tmpAA.f2=es;_tmpAA;});_tmpA9;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_fncall_exp(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*es,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_FnCall_e_struct*_tmpAB=
_cycalloc(sizeof(*_tmpAB));_tmpAB[0]=({struct Cyc_Absyn_FnCall_e_struct _tmpAC;
_tmpAC.tag=9;_tmpAC.f1=e;_tmpAC.f2=es;_tmpAC.f3=0;_tmpAC;});_tmpAB;}),loc);}
struct Cyc_Absyn_Exp*Cyc_Absyn_noinstantiate_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_NoInstantiate_e_struct*
_tmpAD=_cycalloc(sizeof(*_tmpAD));_tmpAD[0]=({struct Cyc_Absyn_NoInstantiate_e_struct
_tmpAE;_tmpAE.tag=11;_tmpAE.f1=e;_tmpAE;});_tmpAD;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_instantiate_exp(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*ts,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Instantiate_e_struct*
_tmpAF=_cycalloc(sizeof(*_tmpAF));_tmpAF[0]=({struct Cyc_Absyn_Instantiate_e_struct
_tmpB0;_tmpB0.tag=12;_tmpB0.f1=e;_tmpB0.f2=ts;_tmpB0;});_tmpAF;}),loc);}struct
Cyc_Absyn_Exp*Cyc_Absyn_cast_exp(void*t,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Cast_e_struct*_tmpB1=
_cycalloc(sizeof(*_tmpB1));_tmpB1[0]=({struct Cyc_Absyn_Cast_e_struct _tmpB2;
_tmpB2.tag=13;_tmpB2.f1=(void*)t;_tmpB2.f2=e;_tmpB2;});_tmpB1;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_throw_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return
Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Throw_e_struct*_tmpB3=_cycalloc(
sizeof(*_tmpB3));_tmpB3[0]=({struct Cyc_Absyn_Throw_e_struct _tmpB4;_tmpB4.tag=10;
_tmpB4.f1=e;_tmpB4;});_tmpB3;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_address_exp(
struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_Address_e_struct*_tmpB5=_cycalloc(sizeof(*_tmpB5));
_tmpB5[0]=({struct Cyc_Absyn_Address_e_struct _tmpB6;_tmpB6.tag=14;_tmpB6.f1=e;
_tmpB6;});_tmpB5;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_sizeoftyp_exp(void*t,
struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Sizeoftyp_e_struct*
_tmpB7=_cycalloc(sizeof(*_tmpB7));_tmpB7[0]=({struct Cyc_Absyn_Sizeoftyp_e_struct
_tmpB8;_tmpB8.tag=16;_tmpB8.f1=(void*)t;_tmpB8;});_tmpB7;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_sizeofexp_exp(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Sizeofexp_e_struct*_tmpB9=
_cycalloc(sizeof(*_tmpB9));_tmpB9[0]=({struct Cyc_Absyn_Sizeofexp_e_struct _tmpBA;
_tmpBA.tag=17;_tmpBA.f1=e;_tmpBA;});_tmpB9;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_offsetof_exp(
void*t,void*of,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_Offsetof_e_struct*_tmpBB=_cycalloc(sizeof(*_tmpBB));_tmpBB[0]=({
struct Cyc_Absyn_Offsetof_e_struct _tmpBC;_tmpBC.tag=18;_tmpBC.f1=(void*)t;_tmpBC.f2=(
void*)of;_tmpBC;});_tmpBB;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_gentyp_exp(
struct Cyc_List_List*tvs,void*t,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_Gentyp_e_struct*_tmpBD=_cycalloc(sizeof(*_tmpBD));_tmpBD[
0]=({struct Cyc_Absyn_Gentyp_e_struct _tmpBE;_tmpBE.tag=19;_tmpBE.f1=tvs;_tmpBE.f2=(
void*)t;_tmpBE;});_tmpBD;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_deref_exp(struct
Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_Deref_e_struct*_tmpBF=_cycalloc(sizeof(*_tmpBF));_tmpBF[0]=({
struct Cyc_Absyn_Deref_e_struct _tmpC0;_tmpC0.tag=20;_tmpC0.f1=e;_tmpC0;});_tmpBF;}),
loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_aggrmember_exp(struct Cyc_Absyn_Exp*e,struct
_tagged_arr*n,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((void*)({
struct Cyc_Absyn_AggrMember_e_struct*_tmpC1=_cycalloc(sizeof(*_tmpC1));_tmpC1[0]=({
struct Cyc_Absyn_AggrMember_e_struct _tmpC2;_tmpC2.tag=21;_tmpC2.f1=e;_tmpC2.f2=n;
_tmpC2;});_tmpC1;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_aggrarrow_exp(struct Cyc_Absyn_Exp*
e,struct _tagged_arr*n,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_AggrArrow_e_struct*_tmpC3=_cycalloc(sizeof(*_tmpC3));
_tmpC3[0]=({struct Cyc_Absyn_AggrArrow_e_struct _tmpC4;_tmpC4.tag=22;_tmpC4.f1=e;
_tmpC4.f2=n;_tmpC4;});_tmpC3;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_subscript_exp(
struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Subscript_e_struct*_tmpC5=
_cycalloc(sizeof(*_tmpC5));_tmpC5[0]=({struct Cyc_Absyn_Subscript_e_struct _tmpC6;
_tmpC6.tag=23;_tmpC6.f1=e1;_tmpC6.f2=e2;_tmpC6;});_tmpC5;}),loc);}struct Cyc_Absyn_Exp*
Cyc_Absyn_tuple_exp(struct Cyc_List_List*es,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Tuple_e_struct*_tmpC7=_cycalloc(
sizeof(*_tmpC7));_tmpC7[0]=({struct Cyc_Absyn_Tuple_e_struct _tmpC8;_tmpC8.tag=24;
_tmpC8.f1=es;_tmpC8;});_tmpC7;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_stmt_exp(
struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_exp((
void*)({struct Cyc_Absyn_StmtExp_e_struct*_tmpC9=_cycalloc(sizeof(*_tmpC9));
_tmpC9[0]=({struct Cyc_Absyn_StmtExp_e_struct _tmpCA;_tmpCA.tag=35;_tmpCA.f1=s;
_tmpCA;});_tmpC9;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_match_exn_exp(struct Cyc_Position_Segment*
loc){return Cyc_Absyn_var_exp(Cyc_Absyn_Match_Exception_name,loc);}struct _tuple10{
struct Cyc_List_List*f1;struct Cyc_Absyn_Exp*f2;};struct Cyc_Absyn_Exp*Cyc_Absyn_array_exp(
struct Cyc_List_List*es,struct Cyc_Position_Segment*loc){struct Cyc_List_List*dles=
0;for(0;es != 0;es=es->tl){dles=({struct Cyc_List_List*_tmpCB=_cycalloc(sizeof(*
_tmpCB));_tmpCB->hd=({struct _tuple10*_tmpCC=_cycalloc(sizeof(*_tmpCC));_tmpCC->f1=
0;_tmpCC->f2=(struct Cyc_Absyn_Exp*)es->hd;_tmpCC;});_tmpCB->tl=dles;_tmpCB;});}
dles=((struct Cyc_List_List*(*)(struct Cyc_List_List*x))Cyc_List_imp_rev)(dles);
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_Array_e_struct*_tmpCD=_cycalloc(
sizeof(*_tmpCD));_tmpCD[0]=({struct Cyc_Absyn_Array_e_struct _tmpCE;_tmpCE.tag=26;
_tmpCE.f1=dles;_tmpCE;});_tmpCD;}),loc);}struct Cyc_Absyn_Exp*Cyc_Absyn_unresolvedmem_exp(
struct Cyc_Core_Opt*n,struct Cyc_List_List*dles,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_exp((void*)({struct Cyc_Absyn_UnresolvedMem_e_struct*_tmpCF=
_cycalloc(sizeof(*_tmpCF));_tmpCF[0]=({struct Cyc_Absyn_UnresolvedMem_e_struct
_tmpD0;_tmpD0.tag=34;_tmpD0.f1=n;_tmpD0.f2=dles;_tmpD0;});_tmpCF;}),loc);}struct
Cyc_Absyn_Stmt*Cyc_Absyn_new_stmt(void*s,struct Cyc_Position_Segment*loc){return({
struct Cyc_Absyn_Stmt*_tmpD1=_cycalloc(sizeof(*_tmpD1));_tmpD1->r=(void*)s;_tmpD1->loc=
loc;_tmpD1->non_local_preds=0;_tmpD1->try_depth=0;_tmpD1->annot=(void*)((void*)
Cyc_Absyn_EmptyAnnot);_tmpD1;});}struct Cyc_Absyn_Stmt*Cyc_Absyn_skip_stmt(struct
Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)0,loc);}struct Cyc_Absyn_Stmt*
Cyc_Absyn_exp_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return
Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Exp_s_struct*_tmpD2=_cycalloc(
sizeof(*_tmpD2));_tmpD2[0]=({struct Cyc_Absyn_Exp_s_struct _tmpD3;_tmpD3.tag=0;
_tmpD3.f1=e;_tmpD3;});_tmpD2;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmts(
struct Cyc_List_List*ss,struct Cyc_Position_Segment*loc){if(ss == 0)return Cyc_Absyn_skip_stmt(
loc);else{if(ss->tl == 0)return(struct Cyc_Absyn_Stmt*)ss->hd;else{return Cyc_Absyn_seq_stmt((
struct Cyc_Absyn_Stmt*)ss->hd,Cyc_Absyn_seq_stmts(ss->tl,loc),loc);}}}struct Cyc_Absyn_Stmt*
Cyc_Absyn_return_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Return_s_struct*_tmpD4=
_cycalloc(sizeof(*_tmpD4));_tmpD4[0]=({struct Cyc_Absyn_Return_s_struct _tmpD5;
_tmpD5.tag=2;_tmpD5.f1=e;_tmpD5;});_tmpD4;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_ifthenelse_stmt(
struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*s2,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_IfThenElse_s_struct*
_tmpD6=_cycalloc(sizeof(*_tmpD6));_tmpD6[0]=({struct Cyc_Absyn_IfThenElse_s_struct
_tmpD7;_tmpD7.tag=3;_tmpD7.f1=e;_tmpD7.f2=s1;_tmpD7.f3=s2;_tmpD7;});_tmpD6;}),
loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_while_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_Absyn_Stmt*
s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_While_s_struct*
_tmpD8=_cycalloc(sizeof(*_tmpD8));_tmpD8[0]=({struct Cyc_Absyn_While_s_struct
_tmpD9;_tmpD9.tag=4;_tmpD9.f1=({struct _tuple2 _tmpDA;_tmpDA.f1=e;_tmpDA.f2=Cyc_Absyn_skip_stmt(
e->loc);_tmpDA;});_tmpD9.f2=s;_tmpD9;});_tmpD8;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_break_stmt(
struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Break_s_struct*
_tmpDB=_cycalloc(sizeof(*_tmpDB));_tmpDB[0]=({struct Cyc_Absyn_Break_s_struct
_tmpDC;_tmpDC.tag=5;_tmpDC.f1=0;_tmpDC;});_tmpDB;}),loc);}struct Cyc_Absyn_Stmt*
Cyc_Absyn_continue_stmt(struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Continue_s_struct*_tmpDD=_cycalloc(sizeof(*_tmpDD));
_tmpDD[0]=({struct Cyc_Absyn_Continue_s_struct _tmpDE;_tmpDE.tag=6;_tmpDE.f1=0;
_tmpDE;});_tmpDD;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_for_stmt(struct Cyc_Absyn_Exp*
e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Absyn_Exp*e3,struct Cyc_Absyn_Stmt*s,struct
Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_For_s_struct*
_tmpDF=_cycalloc(sizeof(*_tmpDF));_tmpDF[0]=({struct Cyc_Absyn_For_s_struct _tmpE0;
_tmpE0.tag=8;_tmpE0.f1=e1;_tmpE0.f2=({struct _tuple2 _tmpE1;_tmpE1.f1=e2;_tmpE1.f2=
Cyc_Absyn_skip_stmt(e3->loc);_tmpE1;});_tmpE0.f3=({struct _tuple2 _tmpE2;_tmpE2.f1=
e3;_tmpE2.f2=Cyc_Absyn_skip_stmt(e3->loc);_tmpE2;});_tmpE0.f4=s;_tmpE0;});_tmpDF;}),
loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_switch_stmt(struct Cyc_Absyn_Exp*e,struct Cyc_List_List*
scs,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Switch_s_struct*
_tmpE3=_cycalloc(sizeof(*_tmpE3));_tmpE3[0]=({struct Cyc_Absyn_Switch_s_struct
_tmpE4;_tmpE4.tag=9;_tmpE4.f1=e;_tmpE4.f2=scs;_tmpE4;});_tmpE3;}),loc);}struct
Cyc_Absyn_Stmt*Cyc_Absyn_seq_stmt(struct Cyc_Absyn_Stmt*s1,struct Cyc_Absyn_Stmt*
s2,struct Cyc_Position_Segment*loc){struct _tuple9 _tmpE6=({struct _tuple9 _tmpE5;
_tmpE5.f1=(void*)s1->r;_tmpE5.f2=(void*)s2->r;_tmpE5;});void*_tmpE7;void*_tmpE8;
_LL3C: _tmpE7=_tmpE6.f1;if((int)_tmpE7 != 0)goto _LL3E;_LL3D: return s2;_LL3E: _tmpE8=
_tmpE6.f2;if((int)_tmpE8 != 0)goto _LL40;_LL3F: return s1;_LL40:;_LL41: return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Seq_s_struct*_tmpE9=_cycalloc(sizeof(*_tmpE9));_tmpE9[0]=({
struct Cyc_Absyn_Seq_s_struct _tmpEA;_tmpEA.tag=1;_tmpEA.f1=s1;_tmpEA.f2=s2;_tmpEA;});
_tmpE9;}),loc);_LL3B:;}struct Cyc_Absyn_Stmt*Cyc_Absyn_fallthru_stmt(struct Cyc_List_List*
el,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Fallthru_s_struct*
_tmpEB=_cycalloc(sizeof(*_tmpEB));_tmpEB[0]=({struct Cyc_Absyn_Fallthru_s_struct
_tmpEC;_tmpEC.tag=11;_tmpEC.f1=el;_tmpEC.f2=0;_tmpEC;});_tmpEB;}),loc);}struct
Cyc_Absyn_Stmt*Cyc_Absyn_decl_stmt(struct Cyc_Absyn_Decl*d,struct Cyc_Absyn_Stmt*s,
struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Decl_s_struct*
_tmpED=_cycalloc(sizeof(*_tmpED));_tmpED[0]=({struct Cyc_Absyn_Decl_s_struct
_tmpEE;_tmpEE.tag=12;_tmpEE.f1=d;_tmpEE.f2=s;_tmpEE;});_tmpED;}),loc);}struct Cyc_Absyn_Stmt*
Cyc_Absyn_declare_stmt(struct _tuple0*x,void*t,struct Cyc_Absyn_Exp*init,struct Cyc_Absyn_Stmt*
s,struct Cyc_Position_Segment*loc){struct Cyc_Absyn_Decl*d=Cyc_Absyn_new_decl((
void*)({struct Cyc_Absyn_Var_d_struct*_tmpF1=_cycalloc(sizeof(*_tmpF1));_tmpF1[0]=({
struct Cyc_Absyn_Var_d_struct _tmpF2;_tmpF2.tag=0;_tmpF2.f1=Cyc_Absyn_new_vardecl(
x,t,init);_tmpF2;});_tmpF1;}),loc);return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Decl_s_struct*
_tmpEF=_cycalloc(sizeof(*_tmpEF));_tmpEF[0]=({struct Cyc_Absyn_Decl_s_struct
_tmpF0;_tmpF0.tag=12;_tmpF0.f1=d;_tmpF0.f2=s;_tmpF0;});_tmpEF;}),loc);}struct Cyc_Absyn_Stmt*
Cyc_Absyn_cut_stmt(struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc){return
Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_Cut_s_struct*_tmpF3=_cycalloc(
sizeof(*_tmpF3));_tmpF3[0]=({struct Cyc_Absyn_Cut_s_struct _tmpF4;_tmpF4.tag=13;
_tmpF4.f1=s;_tmpF4;});_tmpF3;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_splice_stmt(
struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Splice_s_struct*_tmpF5=_cycalloc(sizeof(*_tmpF5));_tmpF5[
0]=({struct Cyc_Absyn_Splice_s_struct _tmpF6;_tmpF6.tag=14;_tmpF6.f1=s;_tmpF6;});
_tmpF5;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_label_stmt(struct _tagged_arr*v,
struct Cyc_Absyn_Stmt*s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Label_s_struct*_tmpF7=_cycalloc(sizeof(*_tmpF7));_tmpF7[
0]=({struct Cyc_Absyn_Label_s_struct _tmpF8;_tmpF8.tag=15;_tmpF8.f1=v;_tmpF8.f2=s;
_tmpF8;});_tmpF7;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_do_stmt(struct Cyc_Absyn_Stmt*
s,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Do_s_struct*_tmpF9=_cycalloc(sizeof(*_tmpF9));_tmpF9[0]=({
struct Cyc_Absyn_Do_s_struct _tmpFA;_tmpFA.tag=16;_tmpFA.f1=s;_tmpFA.f2=({struct
_tuple2 _tmpFB;_tmpFB.f1=e;_tmpFB.f2=Cyc_Absyn_skip_stmt(e->loc);_tmpFB;});_tmpFA;});
_tmpF9;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_trycatch_stmt(struct Cyc_Absyn_Stmt*
s,struct Cyc_List_List*scs,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_TryCatch_s_struct*_tmpFC=_cycalloc(sizeof(*_tmpFC));
_tmpFC[0]=({struct Cyc_Absyn_TryCatch_s_struct _tmpFD;_tmpFD.tag=17;_tmpFD.f1=s;
_tmpFD.f2=scs;_tmpFD;});_tmpFC;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_goto_stmt(
struct _tagged_arr*lab,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((
void*)({struct Cyc_Absyn_Goto_s_struct*_tmpFE=_cycalloc(sizeof(*_tmpFE));_tmpFE[0]=({
struct Cyc_Absyn_Goto_s_struct _tmpFF;_tmpFF.tag=7;_tmpFF.f1=lab;_tmpFF.f2=0;
_tmpFF;});_tmpFE;}),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_assign_stmt(struct Cyc_Absyn_Exp*
e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Position_Segment*loc){return Cyc_Absyn_exp_stmt(
Cyc_Absyn_assign_exp(e1,e2,loc),loc);}struct Cyc_Absyn_Stmt*Cyc_Absyn_forarray_stmt(
struct Cyc_List_List*d,struct Cyc_Absyn_Exp*e1,struct Cyc_Absyn_Exp*e2,struct Cyc_Absyn_Stmt*
s,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_stmt((void*)({struct Cyc_Absyn_ForArray_s_struct*
_tmp100=_cycalloc(sizeof(*_tmp100));_tmp100[0]=({struct Cyc_Absyn_ForArray_s_struct
_tmp101;_tmp101.tag=19;_tmp101.f1=({struct Cyc_Absyn_ForArrayInfo _tmp102;_tmp102.defns=
d;_tmp102.condition=({struct _tuple2 _tmp104;_tmp104.f1=e1;_tmp104.f2=Cyc_Absyn_skip_stmt(
e1->loc);_tmp104;});_tmp102.delta=({struct _tuple2 _tmp103;_tmp103.f1=e2;_tmp103.f2=
Cyc_Absyn_skip_stmt(e2->loc);_tmp103;});_tmp102.body=s;_tmp102;});_tmp101;});
_tmp100;}),loc);}struct Cyc_Absyn_Pat*Cyc_Absyn_new_pat(void*p,struct Cyc_Position_Segment*
s){return({struct Cyc_Absyn_Pat*_tmp105=_cycalloc(sizeof(*_tmp105));_tmp105->r=(
void*)p;_tmp105->topt=0;_tmp105->loc=s;_tmp105;});}struct Cyc_Absyn_Decl*Cyc_Absyn_new_decl(
void*r,struct Cyc_Position_Segment*loc){return({struct Cyc_Absyn_Decl*_tmp106=
_cycalloc(sizeof(*_tmp106));_tmp106->r=(void*)r;_tmp106->loc=loc;_tmp106;});}
struct Cyc_Absyn_Decl*Cyc_Absyn_let_decl(struct Cyc_Absyn_Pat*p,struct Cyc_Core_Opt*
t_opt,struct Cyc_Absyn_Exp*e,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_decl((
void*)({struct Cyc_Absyn_Let_d_struct*_tmp107=_cycalloc(sizeof(*_tmp107));_tmp107[
0]=({struct Cyc_Absyn_Let_d_struct _tmp108;_tmp108.tag=2;_tmp108.f1=p;_tmp108.f2=0;
_tmp108.f3=t_opt;_tmp108.f4=e;_tmp108.f5=0;_tmp108;});_tmp107;}),loc);}struct Cyc_Absyn_Decl*
Cyc_Absyn_letv_decl(struct Cyc_List_List*vds,struct Cyc_Position_Segment*loc){
return Cyc_Absyn_new_decl((void*)({struct Cyc_Absyn_Letv_d_struct*_tmp109=
_cycalloc(sizeof(*_tmp109));_tmp109[0]=({struct Cyc_Absyn_Letv_d_struct _tmp10A;
_tmp10A.tag=3;_tmp10A.f1=vds;_tmp10A;});_tmp109;}),loc);}struct Cyc_Absyn_Vardecl*
Cyc_Absyn_new_vardecl(struct _tuple0*x,void*t,struct Cyc_Absyn_Exp*init){return({
struct Cyc_Absyn_Vardecl*_tmp10B=_cycalloc(sizeof(*_tmp10B));_tmp10B->sc=(void*)((
void*)2);_tmp10B->name=x;_tmp10B->tq=Cyc_Absyn_empty_tqual();_tmp10B->type=(void*)
t;_tmp10B->initializer=init;_tmp10B->rgn=0;_tmp10B->attributes=0;_tmp10B->escapes=
0;_tmp10B;});}struct Cyc_Absyn_Vardecl*Cyc_Absyn_static_vardecl(struct _tuple0*x,
void*t,struct Cyc_Absyn_Exp*init){return({struct Cyc_Absyn_Vardecl*_tmp10C=
_cycalloc(sizeof(*_tmp10C));_tmp10C->sc=(void*)((void*)0);_tmp10C->name=x;
_tmp10C->tq=Cyc_Absyn_empty_tqual();_tmp10C->type=(void*)t;_tmp10C->initializer=
init;_tmp10C->rgn=0;_tmp10C->attributes=0;_tmp10C->escapes=0;_tmp10C;});}struct
Cyc_Absyn_Decl*Cyc_Absyn_aggr_decl(void*k,void*s,struct _tuple0*n,struct Cyc_List_List*
ts,struct Cyc_Core_Opt*exist_ts,struct Cyc_Core_Opt*fs,struct Cyc_List_List*atts,
struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_decl((void*)({struct Cyc_Absyn_Aggr_d_struct*
_tmp10D=_cycalloc(sizeof(*_tmp10D));_tmp10D[0]=({struct Cyc_Absyn_Aggr_d_struct
_tmp10E;_tmp10E.tag=4;_tmp10E.f1=({struct Cyc_Absyn_Aggrdecl*_tmp10F=_cycalloc(
sizeof(*_tmp10F));_tmp10F->kind=(void*)k;_tmp10F->sc=(void*)s;_tmp10F->name=n;
_tmp10F->tvs=ts;_tmp10F->exist_vars=exist_ts;_tmp10F->fields=fs;_tmp10F->attributes=
atts;_tmp10F;});_tmp10E;});_tmp10D;}),loc);}struct Cyc_Absyn_Decl*Cyc_Absyn_struct_decl(
void*s,struct _tuple0*n,struct Cyc_List_List*ts,struct Cyc_Core_Opt*exist_ts,struct
Cyc_Core_Opt*fs,struct Cyc_List_List*atts,struct Cyc_Position_Segment*loc){return
Cyc_Absyn_aggr_decl((void*)0,s,n,ts,exist_ts,fs,atts,loc);}struct Cyc_Absyn_Decl*
Cyc_Absyn_union_decl(void*s,struct _tuple0*n,struct Cyc_List_List*ts,struct Cyc_Core_Opt*
exist_ts,struct Cyc_Core_Opt*fs,struct Cyc_List_List*atts,struct Cyc_Position_Segment*
loc){return Cyc_Absyn_aggr_decl((void*)1,s,n,ts,exist_ts,fs,atts,loc);}struct Cyc_Absyn_Decl*
Cyc_Absyn_tunion_decl(void*s,struct _tuple0*n,struct Cyc_List_List*ts,struct Cyc_Core_Opt*
fs,int is_xtunion,struct Cyc_Position_Segment*loc){return Cyc_Absyn_new_decl((void*)({
struct Cyc_Absyn_Tunion_d_struct*_tmp110=_cycalloc(sizeof(*_tmp110));_tmp110[0]=({
struct Cyc_Absyn_Tunion_d_struct _tmp111;_tmp111.tag=5;_tmp111.f1=({struct Cyc_Absyn_Tuniondecl*
_tmp112=_cycalloc(sizeof(*_tmp112));_tmp112->sc=(void*)s;_tmp112->name=n;_tmp112->tvs=
ts;_tmp112->fields=fs;_tmp112->is_xtunion=is_xtunion;_tmp112;});_tmp111;});
_tmp110;}),loc);}static struct _tuple1*Cyc_Absyn_expand_arg(struct _tuple1*a){
return({struct _tuple1*_tmp113=_cycalloc(sizeof(*_tmp113));_tmp113->f1=(*a).f1;
_tmp113->f2=(*a).f2;_tmp113->f3=Cyc_Absyn_pointer_expand((*a).f3);_tmp113;});}
void*Cyc_Absyn_function_typ(struct Cyc_List_List*tvs,struct Cyc_Core_Opt*eff_typ,
void*ret_typ,struct Cyc_List_List*args,int c_varargs,struct Cyc_Absyn_VarargInfo*
cyc_varargs,struct Cyc_List_List*rgn_po,struct Cyc_List_List*atts){return(void*)({
struct Cyc_Absyn_FnType_struct*_tmp114=_cycalloc(sizeof(*_tmp114));_tmp114[0]=({
struct Cyc_Absyn_FnType_struct _tmp115;_tmp115.tag=8;_tmp115.f1=({struct Cyc_Absyn_FnInfo
_tmp116;_tmp116.tvars=tvs;_tmp116.ret_typ=(void*)Cyc_Absyn_pointer_expand(
ret_typ);_tmp116.effect=eff_typ;_tmp116.args=((struct Cyc_List_List*(*)(struct
_tuple1*(*f)(struct _tuple1*),struct Cyc_List_List*x))Cyc_List_map)(Cyc_Absyn_expand_arg,
args);_tmp116.c_varargs=c_varargs;_tmp116.cyc_varargs=cyc_varargs;_tmp116.rgn_po=
rgn_po;_tmp116.attributes=atts;_tmp116;});_tmp115;});_tmp114;});}void*Cyc_Absyn_pointer_expand(
void*t){void*_tmp117=Cyc_Tcutil_compress(t);_LL43: if(_tmp117 <= (void*)3?1:*((int*)
_tmp117)!= 8)goto _LL45;_LL44: return Cyc_Absyn_at_typ(t,(void*)2,Cyc_Absyn_empty_tqual());
_LL45:;_LL46: return t;_LL42:;}int Cyc_Absyn_is_lvalue(struct Cyc_Absyn_Exp*e){void*
_tmp118=(void*)e->r;void*_tmp119;void*_tmp11A;struct Cyc_Absyn_Vardecl*_tmp11B;
void*_tmp11C;struct Cyc_Absyn_Vardecl*_tmp11D;struct Cyc_Absyn_Exp*_tmp11E;struct
Cyc_Absyn_Exp*_tmp11F;struct Cyc_Absyn_Exp*_tmp120;_LL48: if(*((int*)_tmp118)!= 1)
goto _LL4A;_tmp119=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp118)->f2;if(_tmp119
<= (void*)1?1:*((int*)_tmp119)!= 1)goto _LL4A;_LL49: return 0;_LL4A: if(*((int*)
_tmp118)!= 1)goto _LL4C;_tmp11A=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp118)->f2;
if(_tmp11A <= (void*)1?1:*((int*)_tmp11A)!= 0)goto _LL4C;_tmp11B=((struct Cyc_Absyn_Global_b_struct*)
_tmp11A)->f1;_LL4B: _tmp11D=_tmp11B;goto _LL4D;_LL4C: if(*((int*)_tmp118)!= 1)goto
_LL4E;_tmp11C=(void*)((struct Cyc_Absyn_Var_e_struct*)_tmp118)->f2;if(_tmp11C <= (
void*)1?1:*((int*)_tmp11C)!= 3)goto _LL4E;_tmp11D=((struct Cyc_Absyn_Local_b_struct*)
_tmp11C)->f1;_LL4D: {void*_tmp121=Cyc_Tcutil_compress((void*)_tmp11D->type);
_LL5F: if(_tmp121 <= (void*)3?1:*((int*)_tmp121)!= 7)goto _LL61;_LL60: return 0;_LL61:;
_LL62: return 1;_LL5E:;}_LL4E: if(*((int*)_tmp118)!= 1)goto _LL50;_LL4F: goto _LL51;
_LL50: if(*((int*)_tmp118)!= 22)goto _LL52;_LL51: goto _LL53;_LL52: if(*((int*)
_tmp118)!= 20)goto _LL54;_LL53: goto _LL55;_LL54: if(*((int*)_tmp118)!= 23)goto _LL56;
_LL55: return 1;_LL56: if(*((int*)_tmp118)!= 21)goto _LL58;_tmp11E=((struct Cyc_Absyn_AggrMember_e_struct*)
_tmp118)->f1;_LL57: return Cyc_Absyn_is_lvalue(_tmp11E);_LL58: if(*((int*)_tmp118)
!= 12)goto _LL5A;_tmp11F=((struct Cyc_Absyn_Instantiate_e_struct*)_tmp118)->f1;
_LL59: return Cyc_Absyn_is_lvalue(_tmp11F);_LL5A: if(*((int*)_tmp118)!= 11)goto
_LL5C;_tmp120=((struct Cyc_Absyn_NoInstantiate_e_struct*)_tmp118)->f1;_LL5B:
return Cyc_Absyn_is_lvalue(_tmp120);_LL5C:;_LL5D: return 0;_LL47:;}struct Cyc_Absyn_Aggrfield*
Cyc_Absyn_lookup_field(struct Cyc_List_List*fields,struct _tagged_arr*v){{struct
Cyc_List_List*_tmp122=fields;for(0;_tmp122 != 0;_tmp122=_tmp122->tl){if(Cyc_Std_zstrptrcmp(((
struct Cyc_Absyn_Aggrfield*)_tmp122->hd)->name,v)== 0)return(struct Cyc_Absyn_Aggrfield*)((
struct Cyc_Absyn_Aggrfield*)_tmp122->hd);}}return 0;}struct Cyc_Absyn_Aggrfield*Cyc_Absyn_lookup_decl_field(
struct Cyc_Absyn_Aggrdecl*ad,struct _tagged_arr*v){return ad->fields == 0?0: Cyc_Absyn_lookup_field((
struct Cyc_List_List*)((struct Cyc_Core_Opt*)_check_null(ad->fields))->v,v);}
struct _tuple3*Cyc_Absyn_lookup_tuple_field(struct Cyc_List_List*ts,int i){for(0;i
!= 0;-- i){if(ts == 0)return 0;ts=ts->tl;}if(ts == 0)return 0;return(struct _tuple3*)((
struct _tuple3*)ts->hd);}struct _tagged_arr Cyc_Absyn_attribute2string(void*a){void*
_tmp123=a;int _tmp124;int _tmp125;struct _tagged_arr _tmp126;void*_tmp127;int _tmp128;
int _tmp129;void*_tmp12A;int _tmp12B;int _tmp12C;_LL64: if(_tmp123 <= (void*)16?1:*((
int*)_tmp123)!= 0)goto _LL66;_tmp124=((struct Cyc_Absyn_Regparm_att_struct*)
_tmp123)->f1;_LL65: return(struct _tagged_arr)({struct Cyc_Std_Int_pa_struct _tmp12E;
_tmp12E.tag=1;_tmp12E.f1=(int)((unsigned int)_tmp124);{void*_tmp12D[1]={& _tmp12E};
Cyc_Std_aprintf(_tag_arr("regparm(%d)",sizeof(unsigned char),12),_tag_arr(
_tmp12D,sizeof(void*),1));}});_LL66: if((int)_tmp123 != 0)goto _LL68;_LL67: return
_tag_arr("stdcall",sizeof(unsigned char),8);_LL68: if((int)_tmp123 != 1)goto _LL6A;
_LL69: return _tag_arr("cdecl",sizeof(unsigned char),6);_LL6A: if((int)_tmp123 != 2)
goto _LL6C;_LL6B: return _tag_arr("fastcall",sizeof(unsigned char),9);_LL6C: if((int)
_tmp123 != 3)goto _LL6E;_LL6D: return _tag_arr("noreturn",sizeof(unsigned char),9);
_LL6E: if((int)_tmp123 != 4)goto _LL70;_LL6F: return _tag_arr("const",sizeof(
unsigned char),6);_LL70: if(_tmp123 <= (void*)16?1:*((int*)_tmp123)!= 1)goto _LL72;
_tmp125=((struct Cyc_Absyn_Aligned_att_struct*)_tmp123)->f1;_LL71: if(_tmp125 == - 1)
return _tag_arr("aligned",sizeof(unsigned char),8);else{return(struct _tagged_arr)({
struct Cyc_Std_Int_pa_struct _tmp130;_tmp130.tag=1;_tmp130.f1=(int)((unsigned int)
_tmp125);{void*_tmp12F[1]={& _tmp130};Cyc_Std_aprintf(_tag_arr("aligned(%d)",
sizeof(unsigned char),12),_tag_arr(_tmp12F,sizeof(void*),1));}});}_LL72: if((int)
_tmp123 != 5)goto _LL74;_LL73: return _tag_arr("packed",sizeof(unsigned char),7);
_LL74: if(_tmp123 <= (void*)16?1:*((int*)_tmp123)!= 2)goto _LL76;_tmp126=((struct
Cyc_Absyn_Section_att_struct*)_tmp123)->f1;_LL75: return(struct _tagged_arr)({
struct Cyc_Std_String_pa_struct _tmp132;_tmp132.tag=0;_tmp132.f1=(struct
_tagged_arr)_tmp126;{void*_tmp131[1]={& _tmp132};Cyc_Std_aprintf(_tag_arr("section(\"%s\")",
sizeof(unsigned char),14),_tag_arr(_tmp131,sizeof(void*),1));}});_LL76: if((int)
_tmp123 != 6)goto _LL78;_LL77: return _tag_arr("nocommon",sizeof(unsigned char),9);
_LL78: if((int)_tmp123 != 7)goto _LL7A;_LL79: return _tag_arr("shared",sizeof(
unsigned char),7);_LL7A: if((int)_tmp123 != 8)goto _LL7C;_LL7B: return _tag_arr("unused",
sizeof(unsigned char),7);_LL7C: if((int)_tmp123 != 9)goto _LL7E;_LL7D: return
_tag_arr("weak",sizeof(unsigned char),5);_LL7E: if((int)_tmp123 != 10)goto _LL80;
_LL7F: return _tag_arr("dllimport",sizeof(unsigned char),10);_LL80: if((int)_tmp123
!= 11)goto _LL82;_LL81: return _tag_arr("dllexport",sizeof(unsigned char),10);_LL82:
if((int)_tmp123 != 12)goto _LL84;_LL83: return _tag_arr("no_instrument_function",
sizeof(unsigned char),23);_LL84: if((int)_tmp123 != 13)goto _LL86;_LL85: return
_tag_arr("constructor",sizeof(unsigned char),12);_LL86: if((int)_tmp123 != 14)goto
_LL88;_LL87: return _tag_arr("destructor",sizeof(unsigned char),11);_LL88: if((int)
_tmp123 != 15)goto _LL8A;_LL89: return _tag_arr("no_check_memory_usage",sizeof(
unsigned char),22);_LL8A: if(_tmp123 <= (void*)16?1:*((int*)_tmp123)!= 3)goto _LL8C;
_tmp127=(void*)((struct Cyc_Absyn_Format_att_struct*)_tmp123)->f1;if((int)_tmp127
!= 0)goto _LL8C;_tmp128=((struct Cyc_Absyn_Format_att_struct*)_tmp123)->f2;_tmp129=((
struct Cyc_Absyn_Format_att_struct*)_tmp123)->f3;_LL8B: return(struct _tagged_arr)({
struct Cyc_Std_Int_pa_struct _tmp135;_tmp135.tag=1;_tmp135.f1=(unsigned int)
_tmp129;{struct Cyc_Std_Int_pa_struct _tmp134;_tmp134.tag=1;_tmp134.f1=(
unsigned int)_tmp128;{void*_tmp133[2]={& _tmp134,& _tmp135};Cyc_Std_aprintf(
_tag_arr("format(printf,%u,%u)",sizeof(unsigned char),21),_tag_arr(_tmp133,
sizeof(void*),2));}}});_LL8C: if(_tmp123 <= (void*)16?1:*((int*)_tmp123)!= 3)goto
_LL63;_tmp12A=(void*)((struct Cyc_Absyn_Format_att_struct*)_tmp123)->f1;if((int)
_tmp12A != 1)goto _LL63;_tmp12B=((struct Cyc_Absyn_Format_att_struct*)_tmp123)->f2;
_tmp12C=((struct Cyc_Absyn_Format_att_struct*)_tmp123)->f3;_LL8D: return(struct
_tagged_arr)({struct Cyc_Std_Int_pa_struct _tmp138;_tmp138.tag=1;_tmp138.f1=(
unsigned int)_tmp12C;{struct Cyc_Std_Int_pa_struct _tmp137;_tmp137.tag=1;_tmp137.f1=(
unsigned int)_tmp12B;{void*_tmp136[2]={& _tmp137,& _tmp138};Cyc_Std_aprintf(
_tag_arr("format(scanf,%u,%u)",sizeof(unsigned char),20),_tag_arr(_tmp136,
sizeof(void*),2));}}});_LL63:;}int Cyc_Absyn_fntype_att(void*a){void*_tmp139=a;
_LL8F: if(_tmp139 <= (void*)16?1:*((int*)_tmp139)!= 0)goto _LL91;_LL90: goto _LL92;
_LL91: if((int)_tmp139 != 2)goto _LL93;_LL92: goto _LL94;_LL93: if((int)_tmp139 != 0)
goto _LL95;_LL94: goto _LL96;_LL95: if((int)_tmp139 != 1)goto _LL97;_LL96: goto _LL98;
_LL97: if((int)_tmp139 != 3)goto _LL99;_LL98: goto _LL9A;_LL99: if(_tmp139 <= (void*)16?
1:*((int*)_tmp139)!= 3)goto _LL9B;_LL9A: goto _LL9C;_LL9B: if((int)_tmp139 != 4)goto
_LL9D;_LL9C: return 1;_LL9D:;_LL9E: return 0;_LL8E:;}static unsigned char _tmp13A[3]="f0";
static struct _tagged_arr Cyc_Absyn_f0={_tmp13A,_tmp13A,_tmp13A + 3};static struct
_tagged_arr*Cyc_Absyn_field_names_v[1]={& Cyc_Absyn_f0};static struct _tagged_arr
Cyc_Absyn_field_names={(void*)((struct _tagged_arr**)Cyc_Absyn_field_names_v),(
void*)((struct _tagged_arr**)Cyc_Absyn_field_names_v),(void*)((struct _tagged_arr**)
Cyc_Absyn_field_names_v + 1)};struct _tagged_arr*Cyc_Absyn_fieldname(int i){
unsigned int fsz=_get_arr_size(Cyc_Absyn_field_names,sizeof(struct _tagged_arr*));
if(i >= fsz)Cyc_Absyn_field_names=({unsigned int _tmp13B=(unsigned int)(i + 1);
struct _tagged_arr**_tmp13C=(struct _tagged_arr**)_cycalloc(_check_times(sizeof(
struct _tagged_arr*),_tmp13B));struct _tagged_arr _tmp141=_tag_arr(_tmp13C,sizeof(
struct _tagged_arr*),(unsigned int)(i + 1));{unsigned int _tmp13D=_tmp13B;
unsigned int j;for(j=0;j < _tmp13D;j ++){_tmp13C[j]=j < fsz?*((struct _tagged_arr**)
_check_unknown_subscript(Cyc_Absyn_field_names,sizeof(struct _tagged_arr*),(int)j)):({
struct _tagged_arr*_tmp13E=_cycalloc(sizeof(*_tmp13E));_tmp13E[0]=(struct
_tagged_arr)({struct Cyc_Std_Int_pa_struct _tmp140;_tmp140.tag=1;_tmp140.f1=(int)j;{
void*_tmp13F[1]={& _tmp140};Cyc_Std_aprintf(_tag_arr("f%d",sizeof(unsigned char),
4),_tag_arr(_tmp13F,sizeof(void*),1));}});_tmp13E;});}}_tmp141;});return*((
struct _tagged_arr**)_check_unknown_subscript(Cyc_Absyn_field_names,sizeof(struct
_tagged_arr*),i));}struct _tuple4 Cyc_Absyn_aggr_kinded_name(void*info){void*
_tmp142=info;void*_tmp143;struct _tuple0*_tmp144;struct Cyc_Absyn_Aggrdecl**
_tmp145;struct Cyc_Absyn_Aggrdecl*_tmp146;struct Cyc_Absyn_Aggrdecl _tmp147;void*
_tmp148;struct _tuple0*_tmp149;_LLA0: if(*((int*)_tmp142)!= 0)goto _LLA2;_tmp143=(
void*)((struct Cyc_Absyn_UnknownAggr_struct*)_tmp142)->f1;_tmp144=((struct Cyc_Absyn_UnknownAggr_struct*)
_tmp142)->f2;_LLA1: return({struct _tuple4 _tmp14A;_tmp14A.f1=_tmp143;_tmp14A.f2=
_tmp144;_tmp14A;});_LLA2: if(*((int*)_tmp142)!= 1)goto _LL9F;_tmp145=((struct Cyc_Absyn_KnownAggr_struct*)
_tmp142)->f1;_tmp146=*_tmp145;_tmp147=*_tmp146;_tmp148=(void*)_tmp147.kind;
_tmp149=_tmp147.name;_LLA3: return({struct _tuple4 _tmp14B;_tmp14B.f1=_tmp148;
_tmp14B.f2=_tmp149;_tmp14B;});_LL9F:;}struct Cyc_Absyn_Aggrdecl*Cyc_Absyn_get_known_aggrdecl(
void*info){void*_tmp14C=info;void*_tmp14D;struct _tuple0*_tmp14E;struct Cyc_Absyn_Aggrdecl**
_tmp14F;struct Cyc_Absyn_Aggrdecl*_tmp150;_LLA5: if(*((int*)_tmp14C)!= 0)goto _LLA7;
_tmp14D=(void*)((struct Cyc_Absyn_UnknownAggr_struct*)_tmp14C)->f1;_tmp14E=((
struct Cyc_Absyn_UnknownAggr_struct*)_tmp14C)->f2;_LLA6:({void*_tmp151[0]={};((
int(*)(struct _tagged_arr fmt,struct _tagged_arr ap))Cyc_Tcutil_impos)(_tag_arr("unchecked aggrdecl",
sizeof(unsigned char),19),_tag_arr(_tmp151,sizeof(void*),0));});_LLA7: if(*((int*)
_tmp14C)!= 1)goto _LLA4;_tmp14F=((struct Cyc_Absyn_KnownAggr_struct*)_tmp14C)->f1;
_tmp150=*_tmp14F;_LLA8: return _tmp150;_LLA4:;}int Cyc_Absyn_is_union_type(void*t){
void*_tmp152=Cyc_Tcutil_compress(t);void*_tmp153;struct Cyc_Absyn_AggrInfo _tmp154;
void*_tmp155;_LLAA: if(_tmp152 <= (void*)3?1:*((int*)_tmp152)!= 11)goto _LLAC;
_tmp153=(void*)((struct Cyc_Absyn_AnonAggrType_struct*)_tmp152)->f1;if((int)
_tmp153 != 1)goto _LLAC;_LLAB: return 1;_LLAC: if(_tmp152 <= (void*)3?1:*((int*)
_tmp152)!= 10)goto _LLAE;_tmp154=((struct Cyc_Absyn_AggrType_struct*)_tmp152)->f1;
_tmp155=(void*)_tmp154.aggr_info;_LLAD: return(Cyc_Absyn_aggr_kinded_name(_tmp155)).f1
== (void*)1;_LLAE:;_LLAF: return 0;_LLA9:;}void Cyc_Absyn_print_decls(struct Cyc_List_List*
decls){((void(*)(void*rep,struct Cyc_List_List**val))Cyc_Marshal_print_type)(Cyc_decls_rep,&
decls);({void*_tmp156[0]={};Cyc_Std_printf(_tag_arr("\n",sizeof(unsigned char),2),
_tag_arr(_tmp156,sizeof(void*),0));});}extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_0;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_decl_t46H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Decl_rep;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_decl_t_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_391;static struct Cyc_Typerep_Int_struct
Cyc__genrep_5={0,0,32};extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_131;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Vardecl_rep;extern struct
Cyc_Typerep_TUnion_struct Cyc_Absyn_scope_t_rep;static unsigned char _tmp158[7]="Static";
static struct _tuple7 Cyc__gentuple_135={0,{_tmp158,_tmp158,_tmp158 + 7}};static
unsigned char _tmp159[9]="Abstract";static struct _tuple7 Cyc__gentuple_136={1,{
_tmp159,_tmp159,_tmp159 + 9}};static unsigned char _tmp15A[7]="Public";static struct
_tuple7 Cyc__gentuple_137={2,{_tmp15A,_tmp15A,_tmp15A + 7}};static unsigned char
_tmp15B[7]="Extern";static struct _tuple7 Cyc__gentuple_138={3,{_tmp15B,_tmp15B,
_tmp15B + 7}};static unsigned char _tmp15C[8]="ExternC";static struct _tuple7 Cyc__gentuple_139={
4,{_tmp15C,_tmp15C,_tmp15C + 8}};static struct _tuple7*Cyc__genarr_140[5]={& Cyc__gentuple_135,&
Cyc__gentuple_136,& Cyc__gentuple_137,& Cyc__gentuple_138,& Cyc__gentuple_139};
static struct _tuple5*Cyc__genarr_141[0]={};static unsigned char _tmp15E[6]="Scope";
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_scope_t_rep={5,{_tmp15E,_tmp15E,_tmp15E
+ 6},{(void*)((struct _tuple7**)Cyc__genarr_140),(void*)((struct _tuple7**)Cyc__genarr_140),(
void*)((struct _tuple7**)Cyc__genarr_140 + 5)},{(void*)((struct _tuple5**)Cyc__genarr_141),(
void*)((struct _tuple5**)Cyc__genarr_141),(void*)((struct _tuple5**)Cyc__genarr_141
+ 0)}};extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_10;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_11;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_nmspace_t_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_17;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_18;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_var_t46H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_12;extern struct Cyc_Typerep_FatPtr_struct
Cyc__genrep_13;static struct Cyc_Typerep_Int_struct Cyc__genrep_14={0,0,8};static
struct Cyc_Typerep_FatPtr_struct Cyc__genrep_13={2,(void*)((void*)& Cyc__genrep_14)};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_12={1,1,(void*)((void*)& Cyc__genrep_13)};
static unsigned char _tmp162[5]="List";static struct _tagged_arr Cyc__genname_22={
_tmp162,_tmp162,_tmp162 + 5};static unsigned char _tmp163[3]="hd";static struct
_tuple5 Cyc__gentuple_19={offsetof(struct Cyc_List_List,hd),{_tmp163,_tmp163,
_tmp163 + 3},(void*)& Cyc__genrep_12};static unsigned char _tmp164[3]="tl";static
struct _tuple5 Cyc__gentuple_20={offsetof(struct Cyc_List_List,tl),{_tmp164,_tmp164,
_tmp164 + 3},(void*)& Cyc__genrep_18};static struct _tuple5*Cyc__genarr_21[2]={& Cyc__gentuple_19,&
Cyc__gentuple_20};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_var_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_22,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_21),(void*)((struct _tuple5**)Cyc__genarr_21),(void*)((
struct _tuple5**)Cyc__genarr_21 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_18={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_var_t46H2_rep)};struct _tuple11{
unsigned int f1;struct Cyc_List_List*f2;};static struct _tuple6 Cyc__gentuple_23={
offsetof(struct _tuple11,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_24={
offsetof(struct _tuple11,f2),(void*)& Cyc__genrep_18};static struct _tuple6*Cyc__genarr_25[
2]={& Cyc__gentuple_23,& Cyc__gentuple_24};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_17={
4,sizeof(struct _tuple11),{(void*)((struct _tuple6**)Cyc__genarr_25),(void*)((
struct _tuple6**)Cyc__genarr_25),(void*)((struct _tuple6**)Cyc__genarr_25 + 2)}};
static unsigned char _tmp168[6]="Loc_n";static struct _tuple7 Cyc__gentuple_15={0,{
_tmp168,_tmp168,_tmp168 + 6}};static struct _tuple7*Cyc__genarr_16[1]={& Cyc__gentuple_15};
static unsigned char _tmp169[6]="Rel_n";static struct _tuple5 Cyc__gentuple_26={0,{
_tmp169,_tmp169,_tmp169 + 6},(void*)& Cyc__genrep_17};static unsigned char _tmp16A[6]="Abs_n";
static struct _tuple5 Cyc__gentuple_27={1,{_tmp16A,_tmp16A,_tmp16A + 6},(void*)& Cyc__genrep_17};
static struct _tuple5*Cyc__genarr_28[2]={& Cyc__gentuple_26,& Cyc__gentuple_27};
static unsigned char _tmp16C[8]="Nmspace";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_nmspace_t_rep={
5,{_tmp16C,_tmp16C,_tmp16C + 8},{(void*)((struct _tuple7**)Cyc__genarr_16),(void*)((
struct _tuple7**)Cyc__genarr_16),(void*)((struct _tuple7**)Cyc__genarr_16 + 1)},{(
void*)((struct _tuple5**)Cyc__genarr_28),(void*)((struct _tuple5**)Cyc__genarr_28),(
void*)((struct _tuple5**)Cyc__genarr_28 + 2)}};static struct _tuple6 Cyc__gentuple_29={
offsetof(struct _tuple0,f1),(void*)& Cyc_Absyn_nmspace_t_rep};static struct _tuple6
Cyc__gentuple_30={offsetof(struct _tuple0,f2),(void*)& Cyc__genrep_12};static
struct _tuple6*Cyc__genarr_31[2]={& Cyc__gentuple_29,& Cyc__gentuple_30};static
struct Cyc_Typerep_Tuple_struct Cyc__genrep_11={4,sizeof(struct _tuple0),{(void*)((
struct _tuple6**)Cyc__genarr_31),(void*)((struct _tuple6**)Cyc__genarr_31),(void*)((
struct _tuple6**)Cyc__genarr_31 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_10={
1,1,(void*)((void*)& Cyc__genrep_11)};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_132;
struct _tuple12{unsigned char f1;};static struct _tuple6 Cyc__gentuple_133={offsetof(
struct _tuple12,f1),(void*)((void*)& Cyc__genrep_14)};static struct _tuple6*Cyc__genarr_134[
1]={& Cyc__gentuple_133};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_132={4,
sizeof(struct _tuple12),{(void*)((struct _tuple6**)Cyc__genarr_134),(void*)((
struct _tuple6**)Cyc__genarr_134),(void*)((struct _tuple6**)Cyc__genarr_134 + 1)}};
extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_type_t_rep;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1067;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1068;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_kind_t2_rep;extern
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_kind_t_rep;static unsigned char _tmp170[8]="AnyKind";
static struct _tuple7 Cyc__gentuple_185={0,{_tmp170,_tmp170,_tmp170 + 8}};static
unsigned char _tmp171[8]="MemKind";static struct _tuple7 Cyc__gentuple_186={1,{
_tmp171,_tmp171,_tmp171 + 8}};static unsigned char _tmp172[8]="BoxKind";static
struct _tuple7 Cyc__gentuple_187={2,{_tmp172,_tmp172,_tmp172 + 8}};static
unsigned char _tmp173[8]="RgnKind";static struct _tuple7 Cyc__gentuple_188={3,{
_tmp173,_tmp173,_tmp173 + 8}};static unsigned char _tmp174[8]="EffKind";static
struct _tuple7 Cyc__gentuple_189={4,{_tmp174,_tmp174,_tmp174 + 8}};static struct
_tuple7*Cyc__genarr_190[5]={& Cyc__gentuple_185,& Cyc__gentuple_186,& Cyc__gentuple_187,&
Cyc__gentuple_188,& Cyc__gentuple_189};static struct _tuple5*Cyc__genarr_191[0]={};
static unsigned char _tmp176[5]="Kind";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_kind_t_rep={
5,{_tmp176,_tmp176,_tmp176 + 5},{(void*)((struct _tuple7**)Cyc__genarr_190),(void*)((
struct _tuple7**)Cyc__genarr_190),(void*)((struct _tuple7**)Cyc__genarr_190 + 5)},{(
void*)((struct _tuple5**)Cyc__genarr_191),(void*)((struct _tuple5**)Cyc__genarr_191),(
void*)((struct _tuple5**)Cyc__genarr_191 + 0)}};static unsigned char _tmp177[4]="Opt";
static struct _tagged_arr Cyc__genname_1071={_tmp177,_tmp177,_tmp177 + 4};static
unsigned char _tmp178[2]="v";static struct _tuple5 Cyc__gentuple_1069={offsetof(
struct Cyc_Core_Opt,v),{_tmp178,_tmp178,_tmp178 + 2},(void*)& Cyc_Absyn_kind_t_rep};
static struct _tuple5*Cyc__genarr_1070[1]={& Cyc__gentuple_1069};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0Absyn_kind_t2_rep={3,(struct _tagged_arr*)& Cyc__genname_1071,
sizeof(struct Cyc_Core_Opt),{(void*)((struct _tuple5**)Cyc__genarr_1070),(void*)((
struct _tuple5**)Cyc__genarr_1070),(void*)((struct _tuple5**)Cyc__genarr_1070 + 1)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1068={1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_kind_t2_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_61;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0Absyn_type_t2_rep;static unsigned char _tmp17B[4]="Opt";static
struct _tagged_arr Cyc__genname_64={_tmp17B,_tmp17B,_tmp17B + 4};static
unsigned char _tmp17C[2]="v";static struct _tuple5 Cyc__gentuple_62={offsetof(struct
Cyc_Core_Opt,v),{_tmp17C,_tmp17C,_tmp17C + 2},(void*)((void*)& Cyc_Absyn_type_t_rep)};
static struct _tuple5*Cyc__genarr_63[1]={& Cyc__gentuple_62};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0Absyn_type_t2_rep={3,(struct _tagged_arr*)& Cyc__genname_64,
sizeof(struct Cyc_Core_Opt),{(void*)((struct _tuple5**)Cyc__genarr_63),(void*)((
struct _tuple5**)Cyc__genarr_63),(void*)((struct _tuple5**)Cyc__genarr_63 + 1)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_61={1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_type_t2_rep)};
static struct Cyc_Typerep_Int_struct Cyc__genrep_102={0,1,32};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_354;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_tvar_t46H22_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_292;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_tvar_t46H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_182;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Tvar_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_212;static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_212={1,1,(void*)((void*)& Cyc__genrep_102)};extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_kindbound_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_204;
static struct _tuple6 Cyc__gentuple_205={offsetof(struct _tuple6,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_206={offsetof(struct _tuple6,f2),(void*)& Cyc_Absyn_kind_t_rep};
static struct _tuple6*Cyc__genarr_207[2]={& Cyc__gentuple_205,& Cyc__gentuple_206};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_204={4,sizeof(struct _tuple6),{(
void*)((struct _tuple6**)Cyc__genarr_207),(void*)((struct _tuple6**)Cyc__genarr_207),(
void*)((struct _tuple6**)Cyc__genarr_207 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_200;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_192;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_kindbound_t2_rep;static
unsigned char _tmp182[4]="Opt";static struct _tagged_arr Cyc__genname_195={_tmp182,
_tmp182,_tmp182 + 4};static unsigned char _tmp183[2]="v";static struct _tuple5 Cyc__gentuple_193={
offsetof(struct Cyc_Core_Opt,v),{_tmp183,_tmp183,_tmp183 + 2},(void*)& Cyc_Absyn_kindbound_t_rep};
static struct _tuple5*Cyc__genarr_194[1]={& Cyc__gentuple_193};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0Absyn_kindbound_t2_rep={3,(struct _tagged_arr*)& Cyc__genname_195,
sizeof(struct Cyc_Core_Opt),{(void*)((struct _tuple5**)Cyc__genarr_194),(void*)((
struct _tuple5**)Cyc__genarr_194),(void*)((struct _tuple5**)Cyc__genarr_194 + 1)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_192={1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_kindbound_t2_rep)};
struct _tuple13{unsigned int f1;struct Cyc_Core_Opt*f2;};static struct _tuple6 Cyc__gentuple_201={
offsetof(struct _tuple13,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_202={
offsetof(struct _tuple13,f2),(void*)& Cyc__genrep_192};static struct _tuple6*Cyc__genarr_203[
2]={& Cyc__gentuple_201,& Cyc__gentuple_202};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_200={4,sizeof(struct _tuple13),{(void*)((struct _tuple6**)Cyc__genarr_203),(
void*)((struct _tuple6**)Cyc__genarr_203),(void*)((struct _tuple6**)Cyc__genarr_203
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_184;struct _tuple14{
unsigned int f1;struct Cyc_Core_Opt*f2;void*f3;};static struct _tuple6 Cyc__gentuple_196={
offsetof(struct _tuple14,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_197={
offsetof(struct _tuple14,f2),(void*)& Cyc__genrep_192};static struct _tuple6 Cyc__gentuple_198={
offsetof(struct _tuple14,f3),(void*)& Cyc_Absyn_kind_t_rep};static struct _tuple6*
Cyc__genarr_199[3]={& Cyc__gentuple_196,& Cyc__gentuple_197,& Cyc__gentuple_198};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_184={4,sizeof(struct _tuple14),{(
void*)((struct _tuple6**)Cyc__genarr_199),(void*)((struct _tuple6**)Cyc__genarr_199),(
void*)((struct _tuple6**)Cyc__genarr_199 + 3)}};static struct _tuple7*Cyc__genarr_183[
0]={};static unsigned char _tmp188[6]="Eq_kb";static struct _tuple5 Cyc__gentuple_208={
0,{_tmp188,_tmp188,_tmp188 + 6},(void*)& Cyc__genrep_204};static unsigned char
_tmp189[11]="Unknown_kb";static struct _tuple5 Cyc__gentuple_209={1,{_tmp189,
_tmp189,_tmp189 + 11},(void*)& Cyc__genrep_200};static unsigned char _tmp18A[8]="Less_kb";
static struct _tuple5 Cyc__gentuple_210={2,{_tmp18A,_tmp18A,_tmp18A + 8},(void*)& Cyc__genrep_184};
static struct _tuple5*Cyc__genarr_211[3]={& Cyc__gentuple_208,& Cyc__gentuple_209,&
Cyc__gentuple_210};static unsigned char _tmp18C[10]="KindBound";struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_kindbound_t_rep={5,{_tmp18C,_tmp18C,_tmp18C + 10},{(void*)((struct
_tuple7**)Cyc__genarr_183),(void*)((struct _tuple7**)Cyc__genarr_183),(void*)((
struct _tuple7**)Cyc__genarr_183 + 0)},{(void*)((struct _tuple5**)Cyc__genarr_211),(
void*)((struct _tuple5**)Cyc__genarr_211),(void*)((struct _tuple5**)Cyc__genarr_211
+ 3)}};static unsigned char _tmp18D[5]="Tvar";static struct _tagged_arr Cyc__genname_217={
_tmp18D,_tmp18D,_tmp18D + 5};static unsigned char _tmp18E[5]="name";static struct
_tuple5 Cyc__gentuple_213={offsetof(struct Cyc_Absyn_Tvar,name),{_tmp18E,_tmp18E,
_tmp18E + 5},(void*)& Cyc__genrep_12};static unsigned char _tmp18F[9]="identity";
static struct _tuple5 Cyc__gentuple_214={offsetof(struct Cyc_Absyn_Tvar,identity),{
_tmp18F,_tmp18F,_tmp18F + 9},(void*)& Cyc__genrep_212};static unsigned char _tmp190[
5]="kind";static struct _tuple5 Cyc__gentuple_215={offsetof(struct Cyc_Absyn_Tvar,kind),{
_tmp190,_tmp190,_tmp190 + 5},(void*)& Cyc_Absyn_kindbound_t_rep};static struct
_tuple5*Cyc__genarr_216[3]={& Cyc__gentuple_213,& Cyc__gentuple_214,& Cyc__gentuple_215};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Tvar_rep={3,(struct _tagged_arr*)&
Cyc__genname_217,sizeof(struct Cyc_Absyn_Tvar),{(void*)((struct _tuple5**)Cyc__genarr_216),(
void*)((struct _tuple5**)Cyc__genarr_216),(void*)((struct _tuple5**)Cyc__genarr_216
+ 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_182={1,1,(void*)((void*)&
Cyc_struct_Absyn_Tvar_rep)};static unsigned char _tmp193[5]="List";static struct
_tagged_arr Cyc__genname_296={_tmp193,_tmp193,_tmp193 + 5};static unsigned char
_tmp194[3]="hd";static struct _tuple5 Cyc__gentuple_293={offsetof(struct Cyc_List_List,hd),{
_tmp194,_tmp194,_tmp194 + 3},(void*)& Cyc__genrep_182};static unsigned char _tmp195[
3]="tl";static struct _tuple5 Cyc__gentuple_294={offsetof(struct Cyc_List_List,tl),{
_tmp195,_tmp195,_tmp195 + 3},(void*)& Cyc__genrep_292};static struct _tuple5*Cyc__genarr_295[
2]={& Cyc__gentuple_293,& Cyc__gentuple_294};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_tvar_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_296,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_295),(void*)((struct _tuple5**)Cyc__genarr_295),(void*)((
struct _tuple5**)Cyc__genarr_295 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_292={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_tvar_t46H2_rep)};static
unsigned char _tmp198[4]="Opt";static struct _tagged_arr Cyc__genname_357={_tmp198,
_tmp198,_tmp198 + 4};static unsigned char _tmp199[2]="v";static struct _tuple5 Cyc__gentuple_355={
offsetof(struct Cyc_Core_Opt,v),{_tmp199,_tmp199,_tmp199 + 2},(void*)& Cyc__genrep_292};
static struct _tuple5*Cyc__genarr_356[1]={& Cyc__gentuple_355};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0List_list_t0Absyn_tvar_t46H22_rep={3,(struct _tagged_arr*)&
Cyc__genname_357,sizeof(struct Cyc_Core_Opt),{(void*)((struct _tuple5**)Cyc__genarr_356),(
void*)((struct _tuple5**)Cyc__genarr_356),(void*)((struct _tuple5**)Cyc__genarr_356
+ 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_354={1,1,(void*)((void*)&
Cyc_struct_Core_Opt0List_list_t0Absyn_tvar_t46H22_rep)};struct _tuple15{
unsigned int f1;struct Cyc_Core_Opt*f2;struct Cyc_Core_Opt*f3;int f4;struct Cyc_Core_Opt*
f5;};static struct _tuple6 Cyc__gentuple_1072={offsetof(struct _tuple15,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1073={offsetof(struct _tuple15,f2),(
void*)& Cyc__genrep_1068};static struct _tuple6 Cyc__gentuple_1074={offsetof(struct
_tuple15,f3),(void*)& Cyc__genrep_61};static struct _tuple6 Cyc__gentuple_1075={
offsetof(struct _tuple15,f4),(void*)((void*)& Cyc__genrep_102)};static struct
_tuple6 Cyc__gentuple_1076={offsetof(struct _tuple15,f5),(void*)& Cyc__genrep_354};
static struct _tuple6*Cyc__genarr_1077[5]={& Cyc__gentuple_1072,& Cyc__gentuple_1073,&
Cyc__gentuple_1074,& Cyc__gentuple_1075,& Cyc__gentuple_1076};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1067={4,sizeof(struct _tuple15),{(void*)((struct _tuple6**)Cyc__genarr_1077),(
void*)((struct _tuple6**)Cyc__genarr_1077),(void*)((struct _tuple6**)Cyc__genarr_1077
+ 5)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1063;struct _tuple16{
unsigned int f1;struct Cyc_Absyn_Tvar*f2;};static struct _tuple6 Cyc__gentuple_1064={
offsetof(struct _tuple16,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1065={
offsetof(struct _tuple16,f2),(void*)& Cyc__genrep_182};static struct _tuple6*Cyc__genarr_1066[
2]={& Cyc__gentuple_1064,& Cyc__gentuple_1065};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1063={4,sizeof(struct _tuple16),{(void*)((struct _tuple6**)Cyc__genarr_1066),(
void*)((struct _tuple6**)Cyc__genarr_1066),(void*)((struct _tuple6**)Cyc__genarr_1066
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1037;extern struct Cyc_Typerep_Struct_struct
Cyc_Absyn_tunion_info_t_rep;extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_TunionInfoU_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1044;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_UnknownTunionInfo_rep;static unsigned char _tmp19E[18]="UnknownTunionInfo";
static struct _tagged_arr Cyc__genname_1048={_tmp19E,_tmp19E,_tmp19E + 18};static
unsigned char _tmp19F[5]="name";static struct _tuple5 Cyc__gentuple_1045={offsetof(
struct Cyc_Absyn_UnknownTunionInfo,name),{_tmp19F,_tmp19F,_tmp19F + 5},(void*)& Cyc__genrep_10};
static unsigned char _tmp1A0[11]="is_xtunion";static struct _tuple5 Cyc__gentuple_1046={
offsetof(struct Cyc_Absyn_UnknownTunionInfo,is_xtunion),{_tmp1A0,_tmp1A0,_tmp1A0 + 
11},(void*)((void*)& Cyc__genrep_102)};static struct _tuple5*Cyc__genarr_1047[2]={&
Cyc__gentuple_1045,& Cyc__gentuple_1046};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_UnknownTunionInfo_rep={
3,(struct _tagged_arr*)& Cyc__genname_1048,sizeof(struct Cyc_Absyn_UnknownTunionInfo),{(
void*)((struct _tuple5**)Cyc__genarr_1047),(void*)((struct _tuple5**)Cyc__genarr_1047),(
void*)((struct _tuple5**)Cyc__genarr_1047 + 2)}};struct _tuple17{unsigned int f1;
struct Cyc_Absyn_UnknownTunionInfo f2;};static struct _tuple6 Cyc__gentuple_1049={
offsetof(struct _tuple17,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1050={
offsetof(struct _tuple17,f2),(void*)& Cyc_struct_Absyn_UnknownTunionInfo_rep};
static struct _tuple6*Cyc__genarr_1051[2]={& Cyc__gentuple_1049,& Cyc__gentuple_1050};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1044={4,sizeof(struct _tuple17),{(
void*)((struct _tuple6**)Cyc__genarr_1051),(void*)((struct _tuple6**)Cyc__genarr_1051),(
void*)((struct _tuple6**)Cyc__genarr_1051 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1039;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1040;extern
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_282;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Tuniondecl_rep;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_283;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_tunionfield_t46H22_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_284;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_tunionfield_t46H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_265;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Tunionfield_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_266;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List060Absyn_tqual_t4Absyn_type_t1_446H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_267;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_268;static struct
_tuple6 Cyc__gentuple_269={offsetof(struct _tuple3,f1),(void*)& Cyc__genrep_132};
static struct _tuple6 Cyc__gentuple_270={offsetof(struct _tuple3,f2),(void*)((void*)&
Cyc_Absyn_type_t_rep)};static struct _tuple6*Cyc__genarr_271[2]={& Cyc__gentuple_269,&
Cyc__gentuple_270};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_268={4,
sizeof(struct _tuple3),{(void*)((struct _tuple6**)Cyc__genarr_271),(void*)((struct
_tuple6**)Cyc__genarr_271),(void*)((struct _tuple6**)Cyc__genarr_271 + 2)}};static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_267={1,1,(void*)((void*)& Cyc__genrep_268)};
static unsigned char _tmp1A5[5]="List";static struct _tagged_arr Cyc__genname_275={
_tmp1A5,_tmp1A5,_tmp1A5 + 5};static unsigned char _tmp1A6[3]="hd";static struct
_tuple5 Cyc__gentuple_272={offsetof(struct Cyc_List_List,hd),{_tmp1A6,_tmp1A6,
_tmp1A6 + 3},(void*)& Cyc__genrep_267};static unsigned char _tmp1A7[3]="tl";static
struct _tuple5 Cyc__gentuple_273={offsetof(struct Cyc_List_List,tl),{_tmp1A7,
_tmp1A7,_tmp1A7 + 3},(void*)& Cyc__genrep_266};static struct _tuple5*Cyc__genarr_274[
2]={& Cyc__gentuple_272,& Cyc__gentuple_273};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060Absyn_tqual_t4Absyn_type_t1_446H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_275,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_274),(void*)((struct _tuple5**)Cyc__genarr_274),(void*)((
struct _tuple5**)Cyc__genarr_274 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_266={
1,1,(void*)((void*)& Cyc_struct_List_List060Absyn_tqual_t4Absyn_type_t1_446H2_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_2;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Position_Segment_rep;static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_2={
1,1,(void*)((void*)& Cyc_struct_Position_Segment_rep)};static unsigned char _tmp1AB[
12]="Tunionfield";static struct _tagged_arr Cyc__genname_281={_tmp1AB,_tmp1AB,
_tmp1AB + 12};static unsigned char _tmp1AC[5]="name";static struct _tuple5 Cyc__gentuple_276={
offsetof(struct Cyc_Absyn_Tunionfield,name),{_tmp1AC,_tmp1AC,_tmp1AC + 5},(void*)&
Cyc__genrep_10};static unsigned char _tmp1AD[5]="typs";static struct _tuple5 Cyc__gentuple_277={
offsetof(struct Cyc_Absyn_Tunionfield,typs),{_tmp1AD,_tmp1AD,_tmp1AD + 5},(void*)&
Cyc__genrep_266};static unsigned char _tmp1AE[4]="loc";static struct _tuple5 Cyc__gentuple_278={
offsetof(struct Cyc_Absyn_Tunionfield,loc),{_tmp1AE,_tmp1AE,_tmp1AE + 4},(void*)&
Cyc__genrep_2};static unsigned char _tmp1AF[3]="sc";static struct _tuple5 Cyc__gentuple_279={
offsetof(struct Cyc_Absyn_Tunionfield,sc),{_tmp1AF,_tmp1AF,_tmp1AF + 3},(void*)&
Cyc_Absyn_scope_t_rep};static struct _tuple5*Cyc__genarr_280[4]={& Cyc__gentuple_276,&
Cyc__gentuple_277,& Cyc__gentuple_278,& Cyc__gentuple_279};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Tunionfield_rep={3,(struct _tagged_arr*)& Cyc__genname_281,
sizeof(struct Cyc_Absyn_Tunionfield),{(void*)((struct _tuple5**)Cyc__genarr_280),(
void*)((struct _tuple5**)Cyc__genarr_280),(void*)((struct _tuple5**)Cyc__genarr_280
+ 4)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_265={1,1,(void*)((void*)&
Cyc_struct_Absyn_Tunionfield_rep)};static unsigned char _tmp1B2[5]="List";static
struct _tagged_arr Cyc__genname_288={_tmp1B2,_tmp1B2,_tmp1B2 + 5};static
unsigned char _tmp1B3[3]="hd";static struct _tuple5 Cyc__gentuple_285={offsetof(
struct Cyc_List_List,hd),{_tmp1B3,_tmp1B3,_tmp1B3 + 3},(void*)& Cyc__genrep_265};
static unsigned char _tmp1B4[3]="tl";static struct _tuple5 Cyc__gentuple_286={
offsetof(struct Cyc_List_List,tl),{_tmp1B4,_tmp1B4,_tmp1B4 + 3},(void*)& Cyc__genrep_284};
static struct _tuple5*Cyc__genarr_287[2]={& Cyc__gentuple_285,& Cyc__gentuple_286};
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_tunionfield_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_288,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_287),(void*)((struct _tuple5**)Cyc__genarr_287),(void*)((
struct _tuple5**)Cyc__genarr_287 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_284={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_tunionfield_t46H2_rep)};static
unsigned char _tmp1B7[4]="Opt";static struct _tagged_arr Cyc__genname_291={_tmp1B7,
_tmp1B7,_tmp1B7 + 4};static unsigned char _tmp1B8[2]="v";static struct _tuple5 Cyc__gentuple_289={
offsetof(struct Cyc_Core_Opt,v),{_tmp1B8,_tmp1B8,_tmp1B8 + 2},(void*)& Cyc__genrep_284};
static struct _tuple5*Cyc__genarr_290[1]={& Cyc__gentuple_289};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0List_list_t0Absyn_tunionfield_t46H22_rep={3,(struct
_tagged_arr*)& Cyc__genname_291,sizeof(struct Cyc_Core_Opt),{(void*)((struct
_tuple5**)Cyc__genarr_290),(void*)((struct _tuple5**)Cyc__genarr_290),(void*)((
struct _tuple5**)Cyc__genarr_290 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_283={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0List_list_t0Absyn_tunionfield_t46H22_rep)};
static unsigned char _tmp1BB[11]="Tuniondecl";static struct _tagged_arr Cyc__genname_303={
_tmp1BB,_tmp1BB,_tmp1BB + 11};static unsigned char _tmp1BC[3]="sc";static struct
_tuple5 Cyc__gentuple_297={offsetof(struct Cyc_Absyn_Tuniondecl,sc),{_tmp1BC,
_tmp1BC,_tmp1BC + 3},(void*)& Cyc_Absyn_scope_t_rep};static unsigned char _tmp1BD[5]="name";
static struct _tuple5 Cyc__gentuple_298={offsetof(struct Cyc_Absyn_Tuniondecl,name),{
_tmp1BD,_tmp1BD,_tmp1BD + 5},(void*)& Cyc__genrep_10};static unsigned char _tmp1BE[4]="tvs";
static struct _tuple5 Cyc__gentuple_299={offsetof(struct Cyc_Absyn_Tuniondecl,tvs),{
_tmp1BE,_tmp1BE,_tmp1BE + 4},(void*)& Cyc__genrep_292};static unsigned char _tmp1BF[
7]="fields";static struct _tuple5 Cyc__gentuple_300={offsetof(struct Cyc_Absyn_Tuniondecl,fields),{
_tmp1BF,_tmp1BF,_tmp1BF + 7},(void*)& Cyc__genrep_283};static unsigned char _tmp1C0[
11]="is_xtunion";static struct _tuple5 Cyc__gentuple_301={offsetof(struct Cyc_Absyn_Tuniondecl,is_xtunion),{
_tmp1C0,_tmp1C0,_tmp1C0 + 11},(void*)((void*)& Cyc__genrep_102)};static struct
_tuple5*Cyc__genarr_302[5]={& Cyc__gentuple_297,& Cyc__gentuple_298,& Cyc__gentuple_299,&
Cyc__gentuple_300,& Cyc__gentuple_301};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Tuniondecl_rep={
3,(struct _tagged_arr*)& Cyc__genname_303,sizeof(struct Cyc_Absyn_Tuniondecl),{(
void*)((struct _tuple5**)Cyc__genarr_302),(void*)((struct _tuple5**)Cyc__genarr_302),(
void*)((struct _tuple5**)Cyc__genarr_302 + 5)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_282={1,1,(void*)((void*)& Cyc_struct_Absyn_Tuniondecl_rep)};static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1040={1,1,(void*)((void*)& Cyc__genrep_282)};
struct _tuple18{unsigned int f1;struct Cyc_Absyn_Tuniondecl**f2;};static struct
_tuple6 Cyc__gentuple_1041={offsetof(struct _tuple18,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_1042={offsetof(struct _tuple18,f2),(void*)& Cyc__genrep_1040};
static struct _tuple6*Cyc__genarr_1043[2]={& Cyc__gentuple_1041,& Cyc__gentuple_1042};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1039={4,sizeof(struct _tuple18),{(
void*)((struct _tuple6**)Cyc__genarr_1043),(void*)((struct _tuple6**)Cyc__genarr_1043),(
void*)((struct _tuple6**)Cyc__genarr_1043 + 2)}};static struct _tuple7*Cyc__genarr_1038[
0]={};static unsigned char _tmp1C5[14]="UnknownTunion";static struct _tuple5 Cyc__gentuple_1052={
0,{_tmp1C5,_tmp1C5,_tmp1C5 + 14},(void*)& Cyc__genrep_1044};static unsigned char
_tmp1C6[12]="KnownTunion";static struct _tuple5 Cyc__gentuple_1053={1,{_tmp1C6,
_tmp1C6,_tmp1C6 + 12},(void*)& Cyc__genrep_1039};static struct _tuple5*Cyc__genarr_1054[
2]={& Cyc__gentuple_1052,& Cyc__gentuple_1053};static unsigned char _tmp1C8[12]="TunionInfoU";
struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_TunionInfoU_rep={5,{_tmp1C8,
_tmp1C8,_tmp1C8 + 12},{(void*)((struct _tuple7**)Cyc__genarr_1038),(void*)((struct
_tuple7**)Cyc__genarr_1038),(void*)((struct _tuple7**)Cyc__genarr_1038 + 0)},{(
void*)((struct _tuple5**)Cyc__genarr_1054),(void*)((struct _tuple5**)Cyc__genarr_1054),(
void*)((struct _tuple5**)Cyc__genarr_1054 + 2)}};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_52;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_type_t46H2_rep;
static unsigned char _tmp1C9[5]="List";static struct _tagged_arr Cyc__genname_56={
_tmp1C9,_tmp1C9,_tmp1C9 + 5};static unsigned char _tmp1CA[3]="hd";static struct
_tuple5 Cyc__gentuple_53={offsetof(struct Cyc_List_List,hd),{_tmp1CA,_tmp1CA,
_tmp1CA + 3},(void*)((void*)& Cyc_Absyn_type_t_rep)};static unsigned char _tmp1CB[3]="tl";
static struct _tuple5 Cyc__gentuple_54={offsetof(struct Cyc_List_List,tl),{_tmp1CB,
_tmp1CB,_tmp1CB + 3},(void*)& Cyc__genrep_52};static struct _tuple5*Cyc__genarr_55[2]={&
Cyc__gentuple_53,& Cyc__gentuple_54};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_type_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_56,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_55),(void*)((struct _tuple5**)Cyc__genarr_55),(void*)((
struct _tuple5**)Cyc__genarr_55 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_52={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_type_t46H2_rep)};static
unsigned char _tmp1CE[11]="TunionInfo";static struct _tagged_arr Cyc__genname_1059={
_tmp1CE,_tmp1CE,_tmp1CE + 11};static unsigned char _tmp1CF[12]="tunion_info";static
struct _tuple5 Cyc__gentuple_1055={offsetof(struct Cyc_Absyn_TunionInfo,tunion_info),{
_tmp1CF,_tmp1CF,_tmp1CF + 12},(void*)& Cyc_tunion_Absyn_TunionInfoU_rep};static
unsigned char _tmp1D0[6]="targs";static struct _tuple5 Cyc__gentuple_1056={offsetof(
struct Cyc_Absyn_TunionInfo,targs),{_tmp1D0,_tmp1D0,_tmp1D0 + 6},(void*)& Cyc__genrep_52};
static unsigned char _tmp1D1[4]="rgn";static struct _tuple5 Cyc__gentuple_1057={
offsetof(struct Cyc_Absyn_TunionInfo,rgn),{_tmp1D1,_tmp1D1,_tmp1D1 + 4},(void*)((
void*)& Cyc_Absyn_type_t_rep)};static struct _tuple5*Cyc__genarr_1058[3]={& Cyc__gentuple_1055,&
Cyc__gentuple_1056,& Cyc__gentuple_1057};struct Cyc_Typerep_Struct_struct Cyc_Absyn_tunion_info_t_rep={
3,(struct _tagged_arr*)& Cyc__genname_1059,sizeof(struct Cyc_Absyn_TunionInfo),{(
void*)((struct _tuple5**)Cyc__genarr_1058),(void*)((struct _tuple5**)Cyc__genarr_1058),(
void*)((struct _tuple5**)Cyc__genarr_1058 + 3)}};struct _tuple19{unsigned int f1;
struct Cyc_Absyn_TunionInfo f2;};static struct _tuple6 Cyc__gentuple_1060={offsetof(
struct _tuple19,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1061={
offsetof(struct _tuple19,f2),(void*)& Cyc_Absyn_tunion_info_t_rep};static struct
_tuple6*Cyc__genarr_1062[2]={& Cyc__gentuple_1060,& Cyc__gentuple_1061};static
struct Cyc_Typerep_Tuple_struct Cyc__genrep_1037={4,sizeof(struct _tuple19),{(void*)((
struct _tuple6**)Cyc__genarr_1062),(void*)((struct _tuple6**)Cyc__genarr_1062),(
void*)((struct _tuple6**)Cyc__genarr_1062 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1011;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_tunion_field_info_t_rep;
extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_TunionFieldInfoU_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1018;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_UnknownTunionFieldInfo_rep;static unsigned char _tmp1D4[23]="UnknownTunionFieldInfo";
static struct _tagged_arr Cyc__genname_1023={_tmp1D4,_tmp1D4,_tmp1D4 + 23};static
unsigned char _tmp1D5[12]="tunion_name";static struct _tuple5 Cyc__gentuple_1019={
offsetof(struct Cyc_Absyn_UnknownTunionFieldInfo,tunion_name),{_tmp1D5,_tmp1D5,
_tmp1D5 + 12},(void*)& Cyc__genrep_10};static unsigned char _tmp1D6[11]="field_name";
static struct _tuple5 Cyc__gentuple_1020={offsetof(struct Cyc_Absyn_UnknownTunionFieldInfo,field_name),{
_tmp1D6,_tmp1D6,_tmp1D6 + 11},(void*)& Cyc__genrep_10};static unsigned char _tmp1D7[
11]="is_xtunion";static struct _tuple5 Cyc__gentuple_1021={offsetof(struct Cyc_Absyn_UnknownTunionFieldInfo,is_xtunion),{
_tmp1D7,_tmp1D7,_tmp1D7 + 11},(void*)((void*)& Cyc__genrep_102)};static struct
_tuple5*Cyc__genarr_1022[3]={& Cyc__gentuple_1019,& Cyc__gentuple_1020,& Cyc__gentuple_1021};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_UnknownTunionFieldInfo_rep={3,(
struct _tagged_arr*)& Cyc__genname_1023,sizeof(struct Cyc_Absyn_UnknownTunionFieldInfo),{(
void*)((struct _tuple5**)Cyc__genarr_1022),(void*)((struct _tuple5**)Cyc__genarr_1022),(
void*)((struct _tuple5**)Cyc__genarr_1022 + 3)}};struct _tuple20{unsigned int f1;
struct Cyc_Absyn_UnknownTunionFieldInfo f2;};static struct _tuple6 Cyc__gentuple_1024={
offsetof(struct _tuple20,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1025={
offsetof(struct _tuple20,f2),(void*)& Cyc_struct_Absyn_UnknownTunionFieldInfo_rep};
static struct _tuple6*Cyc__genarr_1026[2]={& Cyc__gentuple_1024,& Cyc__gentuple_1025};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1018={4,sizeof(struct _tuple20),{(
void*)((struct _tuple6**)Cyc__genarr_1026),(void*)((struct _tuple6**)Cyc__genarr_1026),(
void*)((struct _tuple6**)Cyc__genarr_1026 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1013;struct _tuple21{unsigned int f1;struct Cyc_Absyn_Tuniondecl*f2;
struct Cyc_Absyn_Tunionfield*f3;};static struct _tuple6 Cyc__gentuple_1014={
offsetof(struct _tuple21,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1015={
offsetof(struct _tuple21,f2),(void*)((void*)& Cyc__genrep_282)};static struct
_tuple6 Cyc__gentuple_1016={offsetof(struct _tuple21,f3),(void*)& Cyc__genrep_265};
static struct _tuple6*Cyc__genarr_1017[3]={& Cyc__gentuple_1014,& Cyc__gentuple_1015,&
Cyc__gentuple_1016};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1013={4,
sizeof(struct _tuple21),{(void*)((struct _tuple6**)Cyc__genarr_1017),(void*)((
struct _tuple6**)Cyc__genarr_1017),(void*)((struct _tuple6**)Cyc__genarr_1017 + 3)}};
static struct _tuple7*Cyc__genarr_1012[0]={};static unsigned char _tmp1DB[19]="UnknownTunionfield";
static struct _tuple5 Cyc__gentuple_1027={0,{_tmp1DB,_tmp1DB,_tmp1DB + 19},(void*)&
Cyc__genrep_1018};static unsigned char _tmp1DC[17]="KnownTunionfield";static struct
_tuple5 Cyc__gentuple_1028={1,{_tmp1DC,_tmp1DC,_tmp1DC + 17},(void*)& Cyc__genrep_1013};
static struct _tuple5*Cyc__genarr_1029[2]={& Cyc__gentuple_1027,& Cyc__gentuple_1028};
static unsigned char _tmp1DE[17]="TunionFieldInfoU";struct Cyc_Typerep_TUnion_struct
Cyc_tunion_Absyn_TunionFieldInfoU_rep={5,{_tmp1DE,_tmp1DE,_tmp1DE + 17},{(void*)((
struct _tuple7**)Cyc__genarr_1012),(void*)((struct _tuple7**)Cyc__genarr_1012),(
void*)((struct _tuple7**)Cyc__genarr_1012 + 0)},{(void*)((struct _tuple5**)Cyc__genarr_1029),(
void*)((struct _tuple5**)Cyc__genarr_1029),(void*)((struct _tuple5**)Cyc__genarr_1029
+ 2)}};static unsigned char _tmp1DF[16]="TunionFieldInfo";static struct _tagged_arr
Cyc__genname_1033={_tmp1DF,_tmp1DF,_tmp1DF + 16};static unsigned char _tmp1E0[11]="field_info";
static struct _tuple5 Cyc__gentuple_1030={offsetof(struct Cyc_Absyn_TunionFieldInfo,field_info),{
_tmp1E0,_tmp1E0,_tmp1E0 + 11},(void*)& Cyc_tunion_Absyn_TunionFieldInfoU_rep};
static unsigned char _tmp1E1[6]="targs";static struct _tuple5 Cyc__gentuple_1031={
offsetof(struct Cyc_Absyn_TunionFieldInfo,targs),{_tmp1E1,_tmp1E1,_tmp1E1 + 6},(
void*)& Cyc__genrep_52};static struct _tuple5*Cyc__genarr_1032[2]={& Cyc__gentuple_1030,&
Cyc__gentuple_1031};struct Cyc_Typerep_Struct_struct Cyc_Absyn_tunion_field_info_t_rep={
3,(struct _tagged_arr*)& Cyc__genname_1033,sizeof(struct Cyc_Absyn_TunionFieldInfo),{(
void*)((struct _tuple5**)Cyc__genarr_1032),(void*)((struct _tuple5**)Cyc__genarr_1032),(
void*)((struct _tuple5**)Cyc__genarr_1032 + 2)}};struct _tuple22{unsigned int f1;
struct Cyc_Absyn_TunionFieldInfo f2;};static struct _tuple6 Cyc__gentuple_1034={
offsetof(struct _tuple22,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1035={
offsetof(struct _tuple22,f2),(void*)& Cyc_Absyn_tunion_field_info_t_rep};static
struct _tuple6*Cyc__genarr_1036[2]={& Cyc__gentuple_1034,& Cyc__gentuple_1035};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1011={4,sizeof(struct _tuple22),{(
void*)((struct _tuple6**)Cyc__genarr_1036),(void*)((struct _tuple6**)Cyc__genarr_1036),(
void*)((struct _tuple6**)Cyc__genarr_1036 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_975;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_ptr_info_t_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_997;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Conref0bool2_rep;extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_Constraint0Absyn_bounds_t2_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_983;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_bounds_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_76;extern
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_77;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Exp_rep;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_exp_t_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_834;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_cnst_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_849;extern
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_sign_t_rep;static unsigned char _tmp1E4[7]="Signed";
static struct _tuple7 Cyc__gentuple_408={0,{_tmp1E4,_tmp1E4,_tmp1E4 + 7}};static
unsigned char _tmp1E5[9]="Unsigned";static struct _tuple7 Cyc__gentuple_409={1,{
_tmp1E5,_tmp1E5,_tmp1E5 + 9}};static struct _tuple7*Cyc__genarr_410[2]={& Cyc__gentuple_408,&
Cyc__gentuple_409};static struct _tuple5*Cyc__genarr_411[0]={};static unsigned char
_tmp1E7[5]="Sign";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_sign_t_rep={5,{
_tmp1E7,_tmp1E7,_tmp1E7 + 5},{(void*)((struct _tuple7**)Cyc__genarr_410),(void*)((
struct _tuple7**)Cyc__genarr_410),(void*)((struct _tuple7**)Cyc__genarr_410 + 2)},{(
void*)((struct _tuple5**)Cyc__genarr_411),(void*)((struct _tuple5**)Cyc__genarr_411),(
void*)((struct _tuple5**)Cyc__genarr_411 + 0)}};struct _tuple23{unsigned int f1;void*
f2;unsigned char f3;};static struct _tuple6 Cyc__gentuple_850={offsetof(struct
_tuple23,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_851={
offsetof(struct _tuple23,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_852={
offsetof(struct _tuple23,f3),(void*)((void*)& Cyc__genrep_14)};static struct _tuple6*
Cyc__genarr_853[3]={& Cyc__gentuple_850,& Cyc__gentuple_851,& Cyc__gentuple_852};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_849={4,sizeof(struct _tuple23),{(
void*)((struct _tuple6**)Cyc__genarr_853),(void*)((struct _tuple6**)Cyc__genarr_853),(
void*)((struct _tuple6**)Cyc__genarr_853 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_843;static struct Cyc_Typerep_Int_struct Cyc__genrep_844={0,1,16};
struct _tuple24{unsigned int f1;void*f2;short f3;};static struct _tuple6 Cyc__gentuple_845={
offsetof(struct _tuple24,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_846={
offsetof(struct _tuple24,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_847={
offsetof(struct _tuple24,f3),(void*)& Cyc__genrep_844};static struct _tuple6*Cyc__genarr_848[
3]={& Cyc__gentuple_845,& Cyc__gentuple_846,& Cyc__gentuple_847};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_843={4,sizeof(struct _tuple24),{(void*)((struct _tuple6**)Cyc__genarr_848),(
void*)((struct _tuple6**)Cyc__genarr_848),(void*)((struct _tuple6**)Cyc__genarr_848
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_407;struct _tuple25{
unsigned int f1;void*f2;int f3;};static struct _tuple6 Cyc__gentuple_412={offsetof(
struct _tuple25,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_413={
offsetof(struct _tuple25,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_414={
offsetof(struct _tuple25,f3),(void*)((void*)& Cyc__genrep_102)};static struct
_tuple6*Cyc__genarr_415[3]={& Cyc__gentuple_412,& Cyc__gentuple_413,& Cyc__gentuple_414};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_407={4,sizeof(struct _tuple25),{(
void*)((struct _tuple6**)Cyc__genarr_415),(void*)((struct _tuple6**)Cyc__genarr_415),(
void*)((struct _tuple6**)Cyc__genarr_415 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_837;static struct Cyc_Typerep_Int_struct Cyc__genrep_838={0,1,64};
struct _tuple26{unsigned int f1;void*f2;long long f3;};static struct _tuple6 Cyc__gentuple_839={
offsetof(struct _tuple26,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_840={
offsetof(struct _tuple26,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_841={
offsetof(struct _tuple26,f3),(void*)& Cyc__genrep_838};static struct _tuple6*Cyc__genarr_842[
3]={& Cyc__gentuple_839,& Cyc__gentuple_840,& Cyc__gentuple_841};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_837={4,sizeof(struct _tuple26),{(void*)((struct _tuple6**)Cyc__genarr_842),(
void*)((struct _tuple6**)Cyc__genarr_842),(void*)((struct _tuple6**)Cyc__genarr_842
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_112;static struct _tuple6
Cyc__gentuple_113={offsetof(struct _tuple7,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_114={offsetof(struct _tuple7,f2),(void*)((void*)& Cyc__genrep_13)};
static struct _tuple6*Cyc__genarr_115[2]={& Cyc__gentuple_113,& Cyc__gentuple_114};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_112={4,sizeof(struct _tuple7),{(
void*)((struct _tuple6**)Cyc__genarr_115),(void*)((struct _tuple6**)Cyc__genarr_115),(
void*)((struct _tuple6**)Cyc__genarr_115 + 2)}};static unsigned char _tmp1EF[7]="Null_c";
static struct _tuple7 Cyc__gentuple_835={0,{_tmp1EF,_tmp1EF,_tmp1EF + 7}};static
struct _tuple7*Cyc__genarr_836[1]={& Cyc__gentuple_835};static unsigned char _tmp1F0[
7]="Char_c";static struct _tuple5 Cyc__gentuple_854={0,{_tmp1F0,_tmp1F0,_tmp1F0 + 7},(
void*)& Cyc__genrep_849};static unsigned char _tmp1F1[8]="Short_c";static struct
_tuple5 Cyc__gentuple_855={1,{_tmp1F1,_tmp1F1,_tmp1F1 + 8},(void*)& Cyc__genrep_843};
static unsigned char _tmp1F2[6]="Int_c";static struct _tuple5 Cyc__gentuple_856={2,{
_tmp1F2,_tmp1F2,_tmp1F2 + 6},(void*)& Cyc__genrep_407};static unsigned char _tmp1F3[
11]="LongLong_c";static struct _tuple5 Cyc__gentuple_857={3,{_tmp1F3,_tmp1F3,
_tmp1F3 + 11},(void*)& Cyc__genrep_837};static unsigned char _tmp1F4[8]="Float_c";
static struct _tuple5 Cyc__gentuple_858={4,{_tmp1F4,_tmp1F4,_tmp1F4 + 8},(void*)& Cyc__genrep_112};
static unsigned char _tmp1F5[9]="String_c";static struct _tuple5 Cyc__gentuple_859={5,{
_tmp1F5,_tmp1F5,_tmp1F5 + 9},(void*)& Cyc__genrep_112};static struct _tuple5*Cyc__genarr_860[
6]={& Cyc__gentuple_854,& Cyc__gentuple_855,& Cyc__gentuple_856,& Cyc__gentuple_857,&
Cyc__gentuple_858,& Cyc__gentuple_859};static unsigned char _tmp1F7[5]="Cnst";
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_cnst_t_rep={5,{_tmp1F7,_tmp1F7,_tmp1F7 + 
5},{(void*)((struct _tuple7**)Cyc__genarr_836),(void*)((struct _tuple7**)Cyc__genarr_836),(
void*)((struct _tuple7**)Cyc__genarr_836 + 1)},{(void*)((struct _tuple5**)Cyc__genarr_860),(
void*)((struct _tuple5**)Cyc__genarr_860),(void*)((struct _tuple5**)Cyc__genarr_860
+ 6)}};static struct _tuple6 Cyc__gentuple_861={offsetof(struct _tuple6,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_862={offsetof(struct _tuple6,f2),(
void*)& Cyc_Absyn_cnst_t_rep};static struct _tuple6*Cyc__genarr_863[2]={& Cyc__gentuple_861,&
Cyc__gentuple_862};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_834={4,
sizeof(struct _tuple6),{(void*)((struct _tuple6**)Cyc__genarr_863),(void*)((struct
_tuple6**)Cyc__genarr_863),(void*)((struct _tuple6**)Cyc__genarr_863 + 2)}};extern
struct Cyc_Typerep_Tuple_struct Cyc__genrep_821;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_binding_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_81;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_82;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Fndecl_rep;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_587;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060Absyn_var_t4Absyn_tqual_t4Absyn_type_t1_446H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_588;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_589;struct _tuple27{struct _tagged_arr*f1;struct Cyc_Absyn_Tqual f2;void*
f3;};static struct _tuple6 Cyc__gentuple_590={offsetof(struct _tuple27,f1),(void*)&
Cyc__genrep_12};static struct _tuple6 Cyc__gentuple_591={offsetof(struct _tuple27,f2),(
void*)& Cyc__genrep_132};static struct _tuple6 Cyc__gentuple_592={offsetof(struct
_tuple27,f3),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct _tuple6*Cyc__genarr_593[
3]={& Cyc__gentuple_590,& Cyc__gentuple_591,& Cyc__gentuple_592};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_589={4,sizeof(struct _tuple27),{(void*)((struct _tuple6**)Cyc__genarr_593),(
void*)((struct _tuple6**)Cyc__genarr_593),(void*)((struct _tuple6**)Cyc__genarr_593
+ 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_588={1,1,(void*)((void*)&
Cyc__genrep_589)};static unsigned char _tmp1FB[5]="List";static struct _tagged_arr
Cyc__genname_597={_tmp1FB,_tmp1FB,_tmp1FB + 5};static unsigned char _tmp1FC[3]="hd";
static struct _tuple5 Cyc__gentuple_594={offsetof(struct Cyc_List_List,hd),{_tmp1FC,
_tmp1FC,_tmp1FC + 3},(void*)& Cyc__genrep_588};static unsigned char _tmp1FD[3]="tl";
static struct _tuple5 Cyc__gentuple_595={offsetof(struct Cyc_List_List,tl),{_tmp1FD,
_tmp1FD,_tmp1FD + 3},(void*)& Cyc__genrep_587};static struct _tuple5*Cyc__genarr_596[
2]={& Cyc__gentuple_594,& Cyc__gentuple_595};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060Absyn_var_t4Absyn_tqual_t4Absyn_type_t1_446H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_597,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_596),(void*)((struct _tuple5**)Cyc__genarr_596),(void*)((
struct _tuple5**)Cyc__genarr_596 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_587={
1,1,(void*)((void*)& Cyc_struct_List_List060Absyn_var_t4Absyn_tqual_t4Absyn_type_t1_446H2_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_576;extern struct Cyc_Typerep_Struct_struct
Cyc_Absyn_vararg_info_t_rep;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_577;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_var_t2_rep;static
unsigned char _tmp200[4]="Opt";static struct _tagged_arr Cyc__genname_580={_tmp200,
_tmp200,_tmp200 + 4};static unsigned char _tmp201[2]="v";static struct _tuple5 Cyc__gentuple_578={
offsetof(struct Cyc_Core_Opt,v),{_tmp201,_tmp201,_tmp201 + 2},(void*)& Cyc__genrep_12};
static struct _tuple5*Cyc__genarr_579[1]={& Cyc__gentuple_578};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0Absyn_var_t2_rep={3,(struct _tagged_arr*)& Cyc__genname_580,
sizeof(struct Cyc_Core_Opt),{(void*)((struct _tuple5**)Cyc__genarr_579),(void*)((
struct _tuple5**)Cyc__genarr_579),(void*)((struct _tuple5**)Cyc__genarr_579 + 1)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_577={1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_var_t2_rep)};
static unsigned char _tmp204[11]="VarargInfo";static struct _tagged_arr Cyc__genname_586={
_tmp204,_tmp204,_tmp204 + 11};static unsigned char _tmp205[5]="name";static struct
_tuple5 Cyc__gentuple_581={offsetof(struct Cyc_Absyn_VarargInfo,name),{_tmp205,
_tmp205,_tmp205 + 5},(void*)& Cyc__genrep_577};static unsigned char _tmp206[3]="tq";
static struct _tuple5 Cyc__gentuple_582={offsetof(struct Cyc_Absyn_VarargInfo,tq),{
_tmp206,_tmp206,_tmp206 + 3},(void*)& Cyc__genrep_132};static unsigned char _tmp207[
5]="type";static struct _tuple5 Cyc__gentuple_583={offsetof(struct Cyc_Absyn_VarargInfo,type),{
_tmp207,_tmp207,_tmp207 + 5},(void*)((void*)& Cyc_Absyn_type_t_rep)};static
unsigned char _tmp208[7]="inject";static struct _tuple5 Cyc__gentuple_584={offsetof(
struct Cyc_Absyn_VarargInfo,inject),{_tmp208,_tmp208,_tmp208 + 7},(void*)((void*)&
Cyc__genrep_102)};static struct _tuple5*Cyc__genarr_585[4]={& Cyc__gentuple_581,&
Cyc__gentuple_582,& Cyc__gentuple_583,& Cyc__gentuple_584};struct Cyc_Typerep_Struct_struct
Cyc_Absyn_vararg_info_t_rep={3,(struct _tagged_arr*)& Cyc__genname_586,sizeof(
struct Cyc_Absyn_VarargInfo),{(void*)((struct _tuple5**)Cyc__genarr_585),(void*)((
struct _tuple5**)Cyc__genarr_585),(void*)((struct _tuple5**)Cyc__genarr_585 + 4)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_576={1,1,(void*)((void*)& Cyc_Absyn_vararg_info_t_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_566;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List060Absyn_type_t4Absyn_type_t1_446H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_567;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_568;static struct
_tuple6 Cyc__gentuple_569={offsetof(struct _tuple9,f1),(void*)((void*)& Cyc_Absyn_type_t_rep)};
static struct _tuple6 Cyc__gentuple_570={offsetof(struct _tuple9,f2),(void*)((void*)&
Cyc_Absyn_type_t_rep)};static struct _tuple6*Cyc__genarr_571[2]={& Cyc__gentuple_569,&
Cyc__gentuple_570};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_568={4,
sizeof(struct _tuple9),{(void*)((struct _tuple6**)Cyc__genarr_571),(void*)((struct
_tuple6**)Cyc__genarr_571),(void*)((struct _tuple6**)Cyc__genarr_571 + 2)}};static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_567={1,1,(void*)((void*)& Cyc__genrep_568)};
static unsigned char _tmp20D[5]="List";static struct _tagged_arr Cyc__genname_575={
_tmp20D,_tmp20D,_tmp20D + 5};static unsigned char _tmp20E[3]="hd";static struct
_tuple5 Cyc__gentuple_572={offsetof(struct Cyc_List_List,hd),{_tmp20E,_tmp20E,
_tmp20E + 3},(void*)& Cyc__genrep_567};static unsigned char _tmp20F[3]="tl";static
struct _tuple5 Cyc__gentuple_573={offsetof(struct Cyc_List_List,tl),{_tmp20F,
_tmp20F,_tmp20F + 3},(void*)& Cyc__genrep_566};static struct _tuple5*Cyc__genarr_574[
2]={& Cyc__gentuple_572,& Cyc__gentuple_573};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060Absyn_type_t4Absyn_type_t1_446H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_575,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_574),(void*)((struct _tuple5**)Cyc__genarr_574),(void*)((
struct _tuple5**)Cyc__genarr_574 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_566={
1,1,(void*)((void*)& Cyc_struct_List_List060Absyn_type_t4Absyn_type_t1_446H2_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_159;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Stmt_rep;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_stmt_t_rep;
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_533;struct _tuple28{unsigned int
f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_Absyn_Stmt*f3;};static struct _tuple6 Cyc__gentuple_534={
offsetof(struct _tuple28,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_535={
offsetof(struct _tuple28,f2),(void*)& Cyc__genrep_159};static struct _tuple6 Cyc__gentuple_536={
offsetof(struct _tuple28,f3),(void*)& Cyc__genrep_159};static struct _tuple6*Cyc__genarr_537[
3]={& Cyc__gentuple_534,& Cyc__gentuple_535,& Cyc__gentuple_536};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_533={4,sizeof(struct _tuple28),{(void*)((struct _tuple6**)Cyc__genarr_537),(
void*)((struct _tuple6**)Cyc__genarr_537),(void*)((struct _tuple6**)Cyc__genarr_537
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_529;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_73;static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_73={1,1,(void*)((
void*)& Cyc_struct_Absyn_Exp_rep)};struct _tuple29{unsigned int f1;struct Cyc_Absyn_Exp*
f2;};static struct _tuple6 Cyc__gentuple_530={offsetof(struct _tuple29,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_531={offsetof(struct _tuple29,f2),(
void*)& Cyc__genrep_73};static struct _tuple6*Cyc__genarr_532[2]={& Cyc__gentuple_530,&
Cyc__gentuple_531};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_529={4,
sizeof(struct _tuple29),{(void*)((struct _tuple6**)Cyc__genarr_532),(void*)((
struct _tuple6**)Cyc__genarr_532),(void*)((struct _tuple6**)Cyc__genarr_532 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_523;struct _tuple30{unsigned int
f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Stmt*f3;struct Cyc_Absyn_Stmt*f4;};
static struct _tuple6 Cyc__gentuple_524={offsetof(struct _tuple30,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_525={offsetof(struct _tuple30,f2),(void*)& Cyc__genrep_77};
static struct _tuple6 Cyc__gentuple_526={offsetof(struct _tuple30,f3),(void*)& Cyc__genrep_159};
static struct _tuple6 Cyc__gentuple_527={offsetof(struct _tuple30,f4),(void*)& Cyc__genrep_159};
static struct _tuple6*Cyc__genarr_528[4]={& Cyc__gentuple_524,& Cyc__gentuple_525,&
Cyc__gentuple_526,& Cyc__gentuple_527};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_523={
4,sizeof(struct _tuple30),{(void*)((struct _tuple6**)Cyc__genarr_528),(void*)((
struct _tuple6**)Cyc__genarr_528),(void*)((struct _tuple6**)Cyc__genarr_528 + 4)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_518;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_168;static struct _tuple6 Cyc__gentuple_169={offsetof(struct _tuple2,f1),(
void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_170={offsetof(struct
_tuple2,f2),(void*)& Cyc__genrep_159};static struct _tuple6*Cyc__genarr_171[2]={&
Cyc__gentuple_169,& Cyc__gentuple_170};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_168={
4,sizeof(struct _tuple2),{(void*)((struct _tuple6**)Cyc__genarr_171),(void*)((
struct _tuple6**)Cyc__genarr_171),(void*)((struct _tuple6**)Cyc__genarr_171 + 2)}};
struct _tuple31{unsigned int f1;struct _tuple2 f2;struct Cyc_Absyn_Stmt*f3;};static
struct _tuple6 Cyc__gentuple_519={offsetof(struct _tuple31,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_520={offsetof(struct _tuple31,f2),(void*)& Cyc__genrep_168};
static struct _tuple6 Cyc__gentuple_521={offsetof(struct _tuple31,f3),(void*)& Cyc__genrep_159};
static struct _tuple6*Cyc__genarr_522[3]={& Cyc__gentuple_519,& Cyc__gentuple_520,&
Cyc__gentuple_521};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_518={4,
sizeof(struct _tuple31),{(void*)((struct _tuple6**)Cyc__genarr_522),(void*)((
struct _tuple6**)Cyc__genarr_522),(void*)((struct _tuple6**)Cyc__genarr_522 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_514;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_509;static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_509={1,1,(void*)((
void*)& Cyc_struct_Absyn_Stmt_rep)};struct _tuple32{unsigned int f1;struct Cyc_Absyn_Stmt*
f2;};static struct _tuple6 Cyc__gentuple_515={offsetof(struct _tuple32,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_516={offsetof(struct _tuple32,f2),(
void*)& Cyc__genrep_509};static struct _tuple6*Cyc__genarr_517[2]={& Cyc__gentuple_515,&
Cyc__gentuple_516};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_514={4,
sizeof(struct _tuple32),{(void*)((struct _tuple6**)Cyc__genarr_517),(void*)((
struct _tuple6**)Cyc__genarr_517),(void*)((struct _tuple6**)Cyc__genarr_517 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_508;struct _tuple33{unsigned int
f1;struct _tagged_arr*f2;struct Cyc_Absyn_Stmt*f3;};static struct _tuple6 Cyc__gentuple_510={
offsetof(struct _tuple33,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_511={
offsetof(struct _tuple33,f2),(void*)& Cyc__genrep_12};static struct _tuple6 Cyc__gentuple_512={
offsetof(struct _tuple33,f3),(void*)& Cyc__genrep_509};static struct _tuple6*Cyc__genarr_513[
3]={& Cyc__gentuple_510,& Cyc__gentuple_511,& Cyc__gentuple_512};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_508={4,sizeof(struct _tuple33),{(void*)((struct _tuple6**)Cyc__genarr_513),(
void*)((struct _tuple6**)Cyc__genarr_513),(void*)((struct _tuple6**)Cyc__genarr_513
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_501;struct _tuple34{
unsigned int f1;struct Cyc_Absyn_Exp*f2;struct _tuple2 f3;struct _tuple2 f4;struct Cyc_Absyn_Stmt*
f5;};static struct _tuple6 Cyc__gentuple_502={offsetof(struct _tuple34,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_503={offsetof(struct _tuple34,f2),(
void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_504={offsetof(struct
_tuple34,f3),(void*)& Cyc__genrep_168};static struct _tuple6 Cyc__gentuple_505={
offsetof(struct _tuple34,f4),(void*)& Cyc__genrep_168};static struct _tuple6 Cyc__gentuple_506={
offsetof(struct _tuple34,f5),(void*)& Cyc__genrep_159};static struct _tuple6*Cyc__genarr_507[
5]={& Cyc__gentuple_502,& Cyc__gentuple_503,& Cyc__gentuple_504,& Cyc__gentuple_505,&
Cyc__gentuple_506};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_501={4,
sizeof(struct _tuple34),{(void*)((struct _tuple6**)Cyc__genarr_507),(void*)((
struct _tuple6**)Cyc__genarr_507),(void*)((struct _tuple6**)Cyc__genarr_507 + 5)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_496;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_224;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_switch_clause_t46H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_225;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Switch_clause_rep;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_226;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Pat_rep;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_raw_pat_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_403;
struct _tuple35{unsigned int f1;unsigned char f2;};static struct _tuple6 Cyc__gentuple_404={
offsetof(struct _tuple35,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_405={
offsetof(struct _tuple35,f2),(void*)((void*)& Cyc__genrep_14)};static struct _tuple6*
Cyc__genarr_406[2]={& Cyc__gentuple_404,& Cyc__gentuple_405};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_403={4,sizeof(struct _tuple35),{(void*)((struct _tuple6**)Cyc__genarr_406),(
void*)((struct _tuple6**)Cyc__genarr_406),(void*)((struct _tuple6**)Cyc__genarr_406
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_399;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_231;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_pat_t46H2_rep;
static unsigned char _tmp21D[5]="List";static struct _tagged_arr Cyc__genname_235={
_tmp21D,_tmp21D,_tmp21D + 5};static unsigned char _tmp21E[3]="hd";static struct
_tuple5 Cyc__gentuple_232={offsetof(struct Cyc_List_List,hd),{_tmp21E,_tmp21E,
_tmp21E + 3},(void*)& Cyc__genrep_226};static unsigned char _tmp21F[3]="tl";static
struct _tuple5 Cyc__gentuple_233={offsetof(struct Cyc_List_List,tl),{_tmp21F,
_tmp21F,_tmp21F + 3},(void*)& Cyc__genrep_231};static struct _tuple5*Cyc__genarr_234[
2]={& Cyc__gentuple_232,& Cyc__gentuple_233};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_pat_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_235,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_234),(void*)((struct _tuple5**)Cyc__genarr_234),(void*)((
struct _tuple5**)Cyc__genarr_234 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_231={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_pat_t46H2_rep)};static struct
_tuple6 Cyc__gentuple_400={offsetof(struct _tuple11,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_401={offsetof(struct _tuple11,f2),(void*)& Cyc__genrep_231};
static struct _tuple6*Cyc__genarr_402[2]={& Cyc__gentuple_400,& Cyc__gentuple_401};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_399={4,sizeof(struct _tuple11),{(
void*)((struct _tuple6**)Cyc__genarr_402),(void*)((struct _tuple6**)Cyc__genarr_402),(
void*)((struct _tuple6**)Cyc__genarr_402 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_395;struct _tuple36{unsigned int f1;struct Cyc_Absyn_Pat*f2;};static
struct _tuple6 Cyc__gentuple_396={offsetof(struct _tuple36,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_397={offsetof(struct _tuple36,f2),(void*)& Cyc__genrep_226};
static struct _tuple6*Cyc__genarr_398[2]={& Cyc__gentuple_396,& Cyc__gentuple_397};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_395={4,sizeof(struct _tuple36),{(
void*)((struct _tuple6**)Cyc__genarr_398),(void*)((struct _tuple6**)Cyc__genarr_398),(
void*)((struct _tuple6**)Cyc__genarr_398 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_309;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_aggr_info_t_rep;
extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_AggrInfoU_rep;extern struct
Cyc_Typerep_Tuple_struct Cyc__genrep_374;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_aggr_kind_t_rep;
static unsigned char _tmp224[8]="StructA";static struct _tuple7 Cyc__gentuple_358={0,{
_tmp224,_tmp224,_tmp224 + 8}};static unsigned char _tmp225[7]="UnionA";static struct
_tuple7 Cyc__gentuple_359={1,{_tmp225,_tmp225,_tmp225 + 7}};static struct _tuple7*
Cyc__genarr_360[2]={& Cyc__gentuple_358,& Cyc__gentuple_359};static struct _tuple5*
Cyc__genarr_361[0]={};static unsigned char _tmp227[9]="AggrKind";struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_aggr_kind_t_rep={5,{_tmp227,_tmp227,_tmp227 + 9},{(void*)((struct
_tuple7**)Cyc__genarr_360),(void*)((struct _tuple7**)Cyc__genarr_360),(void*)((
struct _tuple7**)Cyc__genarr_360 + 2)},{(void*)((struct _tuple5**)Cyc__genarr_361),(
void*)((struct _tuple5**)Cyc__genarr_361),(void*)((struct _tuple5**)Cyc__genarr_361
+ 0)}};struct _tuple37{unsigned int f1;void*f2;struct _tuple0*f3;};static struct
_tuple6 Cyc__gentuple_375={offsetof(struct _tuple37,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_376={offsetof(struct _tuple37,f2),(void*)& Cyc_Absyn_aggr_kind_t_rep};
static struct _tuple6 Cyc__gentuple_377={offsetof(struct _tuple37,f3),(void*)& Cyc__genrep_10};
static struct _tuple6*Cyc__genarr_378[3]={& Cyc__gentuple_375,& Cyc__gentuple_376,&
Cyc__gentuple_377};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_374={4,
sizeof(struct _tuple37),{(void*)((struct _tuple6**)Cyc__genarr_378),(void*)((
struct _tuple6**)Cyc__genarr_378),(void*)((struct _tuple6**)Cyc__genarr_378 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_334;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_335;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_336;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Aggrdecl_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_337;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_aggrfield_t46H22_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_338;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_aggrfield_t46H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_339;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Aggrfield_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_83;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_attribute_t46H2_rep;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_attribute_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_116;
struct _tuple38{unsigned int f1;int f2;};static struct _tuple6 Cyc__gentuple_117={
offsetof(struct _tuple38,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_118={
offsetof(struct _tuple38,f2),(void*)((void*)& Cyc__genrep_102)};static struct
_tuple6*Cyc__genarr_119[2]={& Cyc__gentuple_117,& Cyc__gentuple_118};static struct
Cyc_Typerep_Tuple_struct Cyc__genrep_116={4,sizeof(struct _tuple38),{(void*)((
struct _tuple6**)Cyc__genarr_119),(void*)((struct _tuple6**)Cyc__genarr_119),(void*)((
struct _tuple6**)Cyc__genarr_119 + 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_101;
extern struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_Format_Type_rep;static
unsigned char _tmp22A[10]="Printf_ft";static struct _tuple7 Cyc__gentuple_103={0,{
_tmp22A,_tmp22A,_tmp22A + 10}};static unsigned char _tmp22B[9]="Scanf_ft";static
struct _tuple7 Cyc__gentuple_104={1,{_tmp22B,_tmp22B,_tmp22B + 9}};static struct
_tuple7*Cyc__genarr_105[2]={& Cyc__gentuple_103,& Cyc__gentuple_104};static struct
_tuple5*Cyc__genarr_106[0]={};static unsigned char _tmp22D[12]="Format_Type";
struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_Format_Type_rep={5,{_tmp22D,
_tmp22D,_tmp22D + 12},{(void*)((struct _tuple7**)Cyc__genarr_105),(void*)((struct
_tuple7**)Cyc__genarr_105),(void*)((struct _tuple7**)Cyc__genarr_105 + 2)},{(void*)((
struct _tuple5**)Cyc__genarr_106),(void*)((struct _tuple5**)Cyc__genarr_106),(void*)((
struct _tuple5**)Cyc__genarr_106 + 0)}};struct _tuple39{unsigned int f1;void*f2;int
f3;int f4;};static struct _tuple6 Cyc__gentuple_107={offsetof(struct _tuple39,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_108={offsetof(struct
_tuple39,f2),(void*)& Cyc_tunion_Absyn_Format_Type_rep};static struct _tuple6 Cyc__gentuple_109={
offsetof(struct _tuple39,f3),(void*)((void*)& Cyc__genrep_102)};static struct
_tuple6 Cyc__gentuple_110={offsetof(struct _tuple39,f4),(void*)((void*)& Cyc__genrep_102)};
static struct _tuple6*Cyc__genarr_111[4]={& Cyc__gentuple_107,& Cyc__gentuple_108,&
Cyc__gentuple_109,& Cyc__gentuple_110};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_101={
4,sizeof(struct _tuple39),{(void*)((struct _tuple6**)Cyc__genarr_111),(void*)((
struct _tuple6**)Cyc__genarr_111),(void*)((struct _tuple6**)Cyc__genarr_111 + 4)}};
static unsigned char _tmp22F[12]="Stdcall_att";static struct _tuple7 Cyc__gentuple_84={
0,{_tmp22F,_tmp22F,_tmp22F + 12}};static unsigned char _tmp230[10]="Cdecl_att";
static struct _tuple7 Cyc__gentuple_85={1,{_tmp230,_tmp230,_tmp230 + 10}};static
unsigned char _tmp231[13]="Fastcall_att";static struct _tuple7 Cyc__gentuple_86={2,{
_tmp231,_tmp231,_tmp231 + 13}};static unsigned char _tmp232[13]="Noreturn_att";
static struct _tuple7 Cyc__gentuple_87={3,{_tmp232,_tmp232,_tmp232 + 13}};static
unsigned char _tmp233[10]="Const_att";static struct _tuple7 Cyc__gentuple_88={4,{
_tmp233,_tmp233,_tmp233 + 10}};static unsigned char _tmp234[11]="Packed_att";static
struct _tuple7 Cyc__gentuple_89={5,{_tmp234,_tmp234,_tmp234 + 11}};static
unsigned char _tmp235[13]="Nocommon_att";static struct _tuple7 Cyc__gentuple_90={6,{
_tmp235,_tmp235,_tmp235 + 13}};static unsigned char _tmp236[11]="Shared_att";static
struct _tuple7 Cyc__gentuple_91={7,{_tmp236,_tmp236,_tmp236 + 11}};static
unsigned char _tmp237[11]="Unused_att";static struct _tuple7 Cyc__gentuple_92={8,{
_tmp237,_tmp237,_tmp237 + 11}};static unsigned char _tmp238[9]="Weak_att";static
struct _tuple7 Cyc__gentuple_93={9,{_tmp238,_tmp238,_tmp238 + 9}};static
unsigned char _tmp239[14]="Dllimport_att";static struct _tuple7 Cyc__gentuple_94={10,{
_tmp239,_tmp239,_tmp239 + 14}};static unsigned char _tmp23A[14]="Dllexport_att";
static struct _tuple7 Cyc__gentuple_95={11,{_tmp23A,_tmp23A,_tmp23A + 14}};static
unsigned char _tmp23B[27]="No_instrument_function_att";static struct _tuple7 Cyc__gentuple_96={
12,{_tmp23B,_tmp23B,_tmp23B + 27}};static unsigned char _tmp23C[16]="Constructor_att";
static struct _tuple7 Cyc__gentuple_97={13,{_tmp23C,_tmp23C,_tmp23C + 16}};static
unsigned char _tmp23D[15]="Destructor_att";static struct _tuple7 Cyc__gentuple_98={
14,{_tmp23D,_tmp23D,_tmp23D + 15}};static unsigned char _tmp23E[26]="No_check_memory_usage_att";
static struct _tuple7 Cyc__gentuple_99={15,{_tmp23E,_tmp23E,_tmp23E + 26}};static
struct _tuple7*Cyc__genarr_100[16]={& Cyc__gentuple_84,& Cyc__gentuple_85,& Cyc__gentuple_86,&
Cyc__gentuple_87,& Cyc__gentuple_88,& Cyc__gentuple_89,& Cyc__gentuple_90,& Cyc__gentuple_91,&
Cyc__gentuple_92,& Cyc__gentuple_93,& Cyc__gentuple_94,& Cyc__gentuple_95,& Cyc__gentuple_96,&
Cyc__gentuple_97,& Cyc__gentuple_98,& Cyc__gentuple_99};static unsigned char _tmp23F[
12]="Regparm_att";static struct _tuple5 Cyc__gentuple_120={0,{_tmp23F,_tmp23F,
_tmp23F + 12},(void*)& Cyc__genrep_116};static unsigned char _tmp240[12]="Aligned_att";
static struct _tuple5 Cyc__gentuple_121={1,{_tmp240,_tmp240,_tmp240 + 12},(void*)&
Cyc__genrep_116};static unsigned char _tmp241[12]="Section_att";static struct
_tuple5 Cyc__gentuple_122={2,{_tmp241,_tmp241,_tmp241 + 12},(void*)& Cyc__genrep_112};
static unsigned char _tmp242[11]="Format_att";static struct _tuple5 Cyc__gentuple_123={
3,{_tmp242,_tmp242,_tmp242 + 11},(void*)& Cyc__genrep_101};static struct _tuple5*Cyc__genarr_124[
4]={& Cyc__gentuple_120,& Cyc__gentuple_121,& Cyc__gentuple_122,& Cyc__gentuple_123};
static unsigned char _tmp244[10]="Attribute";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_attribute_t_rep={
5,{_tmp244,_tmp244,_tmp244 + 10},{(void*)((struct _tuple7**)Cyc__genarr_100),(void*)((
struct _tuple7**)Cyc__genarr_100),(void*)((struct _tuple7**)Cyc__genarr_100 + 16)},{(
void*)((struct _tuple5**)Cyc__genarr_124),(void*)((struct _tuple5**)Cyc__genarr_124),(
void*)((struct _tuple5**)Cyc__genarr_124 + 4)}};static unsigned char _tmp245[5]="List";
static struct _tagged_arr Cyc__genname_128={_tmp245,_tmp245,_tmp245 + 5};static
unsigned char _tmp246[3]="hd";static struct _tuple5 Cyc__gentuple_125={offsetof(
struct Cyc_List_List,hd),{_tmp246,_tmp246,_tmp246 + 3},(void*)& Cyc_Absyn_attribute_t_rep};
static unsigned char _tmp247[3]="tl";static struct _tuple5 Cyc__gentuple_126={
offsetof(struct Cyc_List_List,tl),{_tmp247,_tmp247,_tmp247 + 3},(void*)& Cyc__genrep_83};
static struct _tuple5*Cyc__genarr_127[2]={& Cyc__gentuple_125,& Cyc__gentuple_126};
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_attribute_t46H2_rep={3,(
struct _tagged_arr*)& Cyc__genname_128,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_127),(void*)((struct _tuple5**)Cyc__genarr_127),(void*)((
struct _tuple5**)Cyc__genarr_127 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_83={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_attribute_t46H2_rep)};static
unsigned char _tmp24A[10]="Aggrfield";static struct _tagged_arr Cyc__genname_346={
_tmp24A,_tmp24A,_tmp24A + 10};static unsigned char _tmp24B[5]="name";static struct
_tuple5 Cyc__gentuple_340={offsetof(struct Cyc_Absyn_Aggrfield,name),{_tmp24B,
_tmp24B,_tmp24B + 5},(void*)& Cyc__genrep_12};static unsigned char _tmp24C[3]="tq";
static struct _tuple5 Cyc__gentuple_341={offsetof(struct Cyc_Absyn_Aggrfield,tq),{
_tmp24C,_tmp24C,_tmp24C + 3},(void*)& Cyc__genrep_132};static unsigned char _tmp24D[
5]="type";static struct _tuple5 Cyc__gentuple_342={offsetof(struct Cyc_Absyn_Aggrfield,type),{
_tmp24D,_tmp24D,_tmp24D + 5},(void*)((void*)& Cyc_Absyn_type_t_rep)};static
unsigned char _tmp24E[6]="width";static struct _tuple5 Cyc__gentuple_343={offsetof(
struct Cyc_Absyn_Aggrfield,width),{_tmp24E,_tmp24E,_tmp24E + 6},(void*)& Cyc__genrep_73};
static unsigned char _tmp24F[11]="attributes";static struct _tuple5 Cyc__gentuple_344={
offsetof(struct Cyc_Absyn_Aggrfield,attributes),{_tmp24F,_tmp24F,_tmp24F + 11},(
void*)& Cyc__genrep_83};static struct _tuple5*Cyc__genarr_345[5]={& Cyc__gentuple_340,&
Cyc__gentuple_341,& Cyc__gentuple_342,& Cyc__gentuple_343,& Cyc__gentuple_344};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Aggrfield_rep={3,(struct
_tagged_arr*)& Cyc__genname_346,sizeof(struct Cyc_Absyn_Aggrfield),{(void*)((
struct _tuple5**)Cyc__genarr_345),(void*)((struct _tuple5**)Cyc__genarr_345),(void*)((
struct _tuple5**)Cyc__genarr_345 + 5)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_339={
1,1,(void*)((void*)& Cyc_struct_Absyn_Aggrfield_rep)};static unsigned char _tmp252[
5]="List";static struct _tagged_arr Cyc__genname_350={_tmp252,_tmp252,_tmp252 + 5};
static unsigned char _tmp253[3]="hd";static struct _tuple5 Cyc__gentuple_347={
offsetof(struct Cyc_List_List,hd),{_tmp253,_tmp253,_tmp253 + 3},(void*)& Cyc__genrep_339};
static unsigned char _tmp254[3]="tl";static struct _tuple5 Cyc__gentuple_348={
offsetof(struct Cyc_List_List,tl),{_tmp254,_tmp254,_tmp254 + 3},(void*)& Cyc__genrep_338};
static struct _tuple5*Cyc__genarr_349[2]={& Cyc__gentuple_347,& Cyc__gentuple_348};
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_aggrfield_t46H2_rep={3,(
struct _tagged_arr*)& Cyc__genname_350,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_349),(void*)((struct _tuple5**)Cyc__genarr_349),(void*)((
struct _tuple5**)Cyc__genarr_349 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_338={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_aggrfield_t46H2_rep)};static
unsigned char _tmp257[4]="Opt";static struct _tagged_arr Cyc__genname_353={_tmp257,
_tmp257,_tmp257 + 4};static unsigned char _tmp258[2]="v";static struct _tuple5 Cyc__gentuple_351={
offsetof(struct Cyc_Core_Opt,v),{_tmp258,_tmp258,_tmp258 + 2},(void*)& Cyc__genrep_338};
static struct _tuple5*Cyc__genarr_352[1]={& Cyc__gentuple_351};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0List_list_t0Absyn_aggrfield_t46H22_rep={3,(struct _tagged_arr*)&
Cyc__genname_353,sizeof(struct Cyc_Core_Opt),{(void*)((struct _tuple5**)Cyc__genarr_352),(
void*)((struct _tuple5**)Cyc__genarr_352),(void*)((struct _tuple5**)Cyc__genarr_352
+ 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_337={1,1,(void*)((void*)&
Cyc_struct_Core_Opt0List_list_t0Absyn_aggrfield_t46H22_rep)};static unsigned char
_tmp25B[9]="Aggrdecl";static struct _tagged_arr Cyc__genname_370={_tmp25B,_tmp25B,
_tmp25B + 9};static unsigned char _tmp25C[5]="kind";static struct _tuple5 Cyc__gentuple_362={
offsetof(struct Cyc_Absyn_Aggrdecl,kind),{_tmp25C,_tmp25C,_tmp25C + 5},(void*)& Cyc_Absyn_aggr_kind_t_rep};
static unsigned char _tmp25D[3]="sc";static struct _tuple5 Cyc__gentuple_363={
offsetof(struct Cyc_Absyn_Aggrdecl,sc),{_tmp25D,_tmp25D,_tmp25D + 3},(void*)& Cyc_Absyn_scope_t_rep};
static unsigned char _tmp25E[5]="name";static struct _tuple5 Cyc__gentuple_364={
offsetof(struct Cyc_Absyn_Aggrdecl,name),{_tmp25E,_tmp25E,_tmp25E + 5},(void*)& Cyc__genrep_10};
static unsigned char _tmp25F[4]="tvs";static struct _tuple5 Cyc__gentuple_365={
offsetof(struct Cyc_Absyn_Aggrdecl,tvs),{_tmp25F,_tmp25F,_tmp25F + 4},(void*)& Cyc__genrep_292};
static unsigned char _tmp260[11]="exist_vars";static struct _tuple5 Cyc__gentuple_366={
offsetof(struct Cyc_Absyn_Aggrdecl,exist_vars),{_tmp260,_tmp260,_tmp260 + 11},(
void*)& Cyc__genrep_354};static unsigned char _tmp261[7]="fields";static struct
_tuple5 Cyc__gentuple_367={offsetof(struct Cyc_Absyn_Aggrdecl,fields),{_tmp261,
_tmp261,_tmp261 + 7},(void*)& Cyc__genrep_337};static unsigned char _tmp262[11]="attributes";
static struct _tuple5 Cyc__gentuple_368={offsetof(struct Cyc_Absyn_Aggrdecl,attributes),{
_tmp262,_tmp262,_tmp262 + 11},(void*)& Cyc__genrep_83};static struct _tuple5*Cyc__genarr_369[
7]={& Cyc__gentuple_362,& Cyc__gentuple_363,& Cyc__gentuple_364,& Cyc__gentuple_365,&
Cyc__gentuple_366,& Cyc__gentuple_367,& Cyc__gentuple_368};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Aggrdecl_rep={3,(struct _tagged_arr*)& Cyc__genname_370,sizeof(
struct Cyc_Absyn_Aggrdecl),{(void*)((struct _tuple5**)Cyc__genarr_369),(void*)((
struct _tuple5**)Cyc__genarr_369),(void*)((struct _tuple5**)Cyc__genarr_369 + 7)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_336={1,1,(void*)((void*)& Cyc_struct_Absyn_Aggrdecl_rep)};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_335={1,1,(void*)((void*)& Cyc__genrep_336)};
struct _tuple40{unsigned int f1;struct Cyc_Absyn_Aggrdecl**f2;};static struct _tuple6
Cyc__gentuple_371={offsetof(struct _tuple40,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_372={offsetof(struct _tuple40,f2),(void*)& Cyc__genrep_335};
static struct _tuple6*Cyc__genarr_373[2]={& Cyc__gentuple_371,& Cyc__gentuple_372};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_334={4,sizeof(struct _tuple40),{(
void*)((struct _tuple6**)Cyc__genarr_373),(void*)((struct _tuple6**)Cyc__genarr_373),(
void*)((struct _tuple6**)Cyc__genarr_373 + 2)}};static struct _tuple7*Cyc__genarr_333[
0]={};static unsigned char _tmp267[12]="UnknownAggr";static struct _tuple5 Cyc__gentuple_379={
0,{_tmp267,_tmp267,_tmp267 + 12},(void*)& Cyc__genrep_374};static unsigned char
_tmp268[10]="KnownAggr";static struct _tuple5 Cyc__gentuple_380={1,{_tmp268,_tmp268,
_tmp268 + 10},(void*)& Cyc__genrep_334};static struct _tuple5*Cyc__genarr_381[2]={&
Cyc__gentuple_379,& Cyc__gentuple_380};static unsigned char _tmp26A[10]="AggrInfoU";
struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_AggrInfoU_rep={5,{_tmp26A,
_tmp26A,_tmp26A + 10},{(void*)((struct _tuple7**)Cyc__genarr_333),(void*)((struct
_tuple7**)Cyc__genarr_333),(void*)((struct _tuple7**)Cyc__genarr_333 + 0)},{(void*)((
struct _tuple5**)Cyc__genarr_381),(void*)((struct _tuple5**)Cyc__genarr_381),(void*)((
struct _tuple5**)Cyc__genarr_381 + 2)}};static unsigned char _tmp26B[9]="AggrInfo";
static struct _tagged_arr Cyc__genname_385={_tmp26B,_tmp26B,_tmp26B + 9};static
unsigned char _tmp26C[10]="aggr_info";static struct _tuple5 Cyc__gentuple_382={
offsetof(struct Cyc_Absyn_AggrInfo,aggr_info),{_tmp26C,_tmp26C,_tmp26C + 10},(void*)&
Cyc_tunion_Absyn_AggrInfoU_rep};static unsigned char _tmp26D[6]="targs";static
struct _tuple5 Cyc__gentuple_383={offsetof(struct Cyc_Absyn_AggrInfo,targs),{
_tmp26D,_tmp26D,_tmp26D + 6},(void*)& Cyc__genrep_52};static struct _tuple5*Cyc__genarr_384[
2]={& Cyc__gentuple_382,& Cyc__gentuple_383};struct Cyc_Typerep_Struct_struct Cyc_Absyn_aggr_info_t_rep={
3,(struct _tagged_arr*)& Cyc__genname_385,sizeof(struct Cyc_Absyn_AggrInfo),{(void*)((
struct _tuple5**)Cyc__genarr_384),(void*)((struct _tuple5**)Cyc__genarr_384),(void*)((
struct _tuple5**)Cyc__genarr_384 + 2)}};extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_310;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_pat_t1_446H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_311;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_312;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_313;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_designator_t46H2_rep;
extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_designator_t_rep;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_315;struct _tuple41{unsigned int f1;struct _tagged_arr*f2;};static
struct _tuple6 Cyc__gentuple_316={offsetof(struct _tuple41,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_317={offsetof(struct _tuple41,f2),(void*)& Cyc__genrep_12};
static struct _tuple6*Cyc__genarr_318[2]={& Cyc__gentuple_316,& Cyc__gentuple_317};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_315={4,sizeof(struct _tuple41),{(
void*)((struct _tuple6**)Cyc__genarr_318),(void*)((struct _tuple6**)Cyc__genarr_318),(
void*)((struct _tuple6**)Cyc__genarr_318 + 2)}};static struct _tuple7*Cyc__genarr_314[
0]={};static unsigned char _tmp270[13]="ArrayElement";static struct _tuple5 Cyc__gentuple_319={
0,{_tmp270,_tmp270,_tmp270 + 13},(void*)& Cyc__genrep_76};static unsigned char
_tmp271[10]="FieldName";static struct _tuple5 Cyc__gentuple_320={1,{_tmp271,_tmp271,
_tmp271 + 10},(void*)& Cyc__genrep_315};static struct _tuple5*Cyc__genarr_321[2]={&
Cyc__gentuple_319,& Cyc__gentuple_320};static unsigned char _tmp273[11]="Designator";
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_designator_t_rep={5,{_tmp273,_tmp273,
_tmp273 + 11},{(void*)((struct _tuple7**)Cyc__genarr_314),(void*)((struct _tuple7**)
Cyc__genarr_314),(void*)((struct _tuple7**)Cyc__genarr_314 + 0)},{(void*)((struct
_tuple5**)Cyc__genarr_321),(void*)((struct _tuple5**)Cyc__genarr_321),(void*)((
struct _tuple5**)Cyc__genarr_321 + 2)}};static unsigned char _tmp274[5]="List";
static struct _tagged_arr Cyc__genname_325={_tmp274,_tmp274,_tmp274 + 5};static
unsigned char _tmp275[3]="hd";static struct _tuple5 Cyc__gentuple_322={offsetof(
struct Cyc_List_List,hd),{_tmp275,_tmp275,_tmp275 + 3},(void*)& Cyc_Absyn_designator_t_rep};
static unsigned char _tmp276[3]="tl";static struct _tuple5 Cyc__gentuple_323={
offsetof(struct Cyc_List_List,tl),{_tmp276,_tmp276,_tmp276 + 3},(void*)& Cyc__genrep_313};
static struct _tuple5*Cyc__genarr_324[2]={& Cyc__gentuple_322,& Cyc__gentuple_323};
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_designator_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_325,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_324),(void*)((struct _tuple5**)Cyc__genarr_324),(void*)((
struct _tuple5**)Cyc__genarr_324 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_313={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_designator_t46H2_rep)};struct
_tuple42{struct Cyc_List_List*f1;struct Cyc_Absyn_Pat*f2;};static struct _tuple6 Cyc__gentuple_326={
offsetof(struct _tuple42,f1),(void*)& Cyc__genrep_313};static struct _tuple6 Cyc__gentuple_327={
offsetof(struct _tuple42,f2),(void*)& Cyc__genrep_226};static struct _tuple6*Cyc__genarr_328[
2]={& Cyc__gentuple_326,& Cyc__gentuple_327};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_312={4,sizeof(struct _tuple42),{(void*)((struct _tuple6**)Cyc__genarr_328),(
void*)((struct _tuple6**)Cyc__genarr_328),(void*)((struct _tuple6**)Cyc__genarr_328
+ 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_311={1,1,(void*)((void*)&
Cyc__genrep_312)};static unsigned char _tmp27B[5]="List";static struct _tagged_arr
Cyc__genname_332={_tmp27B,_tmp27B,_tmp27B + 5};static unsigned char _tmp27C[3]="hd";
static struct _tuple5 Cyc__gentuple_329={offsetof(struct Cyc_List_List,hd),{_tmp27C,
_tmp27C,_tmp27C + 3},(void*)& Cyc__genrep_311};static unsigned char _tmp27D[3]="tl";
static struct _tuple5 Cyc__gentuple_330={offsetof(struct Cyc_List_List,tl),{_tmp27D,
_tmp27D,_tmp27D + 3},(void*)& Cyc__genrep_310};static struct _tuple5*Cyc__genarr_331[
2]={& Cyc__gentuple_329,& Cyc__gentuple_330};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_pat_t1_446H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_332,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_331),(void*)((struct _tuple5**)Cyc__genarr_331),(void*)((
struct _tuple5**)Cyc__genarr_331 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_310={
1,1,(void*)((void*)& Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_pat_t1_446H2_rep)};
struct _tuple43{unsigned int f1;struct Cyc_Absyn_AggrInfo f2;struct Cyc_List_List*f3;
struct Cyc_List_List*f4;};static struct _tuple6 Cyc__gentuple_386={offsetof(struct
_tuple43,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_387={
offsetof(struct _tuple43,f2),(void*)& Cyc_Absyn_aggr_info_t_rep};static struct
_tuple6 Cyc__gentuple_388={offsetof(struct _tuple43,f3),(void*)& Cyc__genrep_292};
static struct _tuple6 Cyc__gentuple_389={offsetof(struct _tuple43,f4),(void*)& Cyc__genrep_310};
static struct _tuple6*Cyc__genarr_390[4]={& Cyc__gentuple_386,& Cyc__gentuple_387,&
Cyc__gentuple_388,& Cyc__gentuple_389};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_309={
4,sizeof(struct _tuple43),{(void*)((struct _tuple6**)Cyc__genarr_390),(void*)((
struct _tuple6**)Cyc__genarr_390),(void*)((struct _tuple6**)Cyc__genarr_390 + 4)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_264;struct _tuple44{unsigned int
f1;struct Cyc_Absyn_Tuniondecl*f2;struct Cyc_Absyn_Tunionfield*f3;struct Cyc_List_List*
f4;};static struct _tuple6 Cyc__gentuple_304={offsetof(struct _tuple44,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_305={offsetof(struct _tuple44,f2),(
void*)((void*)& Cyc__genrep_282)};static struct _tuple6 Cyc__gentuple_306={offsetof(
struct _tuple44,f3),(void*)& Cyc__genrep_265};static struct _tuple6 Cyc__gentuple_307={
offsetof(struct _tuple44,f4),(void*)& Cyc__genrep_231};static struct _tuple6*Cyc__genarr_308[
4]={& Cyc__gentuple_304,& Cyc__gentuple_305,& Cyc__gentuple_306,& Cyc__gentuple_307};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_264={4,sizeof(struct _tuple44),{(
void*)((struct _tuple6**)Cyc__genarr_308),(void*)((struct _tuple6**)Cyc__genarr_308),(
void*)((struct _tuple6**)Cyc__genarr_308 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_249;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_250;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Enumdecl_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_251;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_enumfield_t46H22_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_71;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_enumfield_t46H2_rep;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_72;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Enumfield_rep;
static unsigned char _tmp282[10]="Enumfield";static struct _tagged_arr Cyc__genname_913={
_tmp282,_tmp282,_tmp282 + 10};static unsigned char _tmp283[5]="name";static struct
_tuple5 Cyc__gentuple_909={offsetof(struct Cyc_Absyn_Enumfield,name),{_tmp283,
_tmp283,_tmp283 + 5},(void*)& Cyc__genrep_10};static unsigned char _tmp284[4]="tag";
static struct _tuple5 Cyc__gentuple_910={offsetof(struct Cyc_Absyn_Enumfield,tag),{
_tmp284,_tmp284,_tmp284 + 4},(void*)& Cyc__genrep_73};static unsigned char _tmp285[4]="loc";
static struct _tuple5 Cyc__gentuple_911={offsetof(struct Cyc_Absyn_Enumfield,loc),{
_tmp285,_tmp285,_tmp285 + 4},(void*)& Cyc__genrep_2};static struct _tuple5*Cyc__genarr_912[
3]={& Cyc__gentuple_909,& Cyc__gentuple_910,& Cyc__gentuple_911};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Enumfield_rep={3,(struct _tagged_arr*)& Cyc__genname_913,sizeof(
struct Cyc_Absyn_Enumfield),{(void*)((struct _tuple5**)Cyc__genarr_912),(void*)((
struct _tuple5**)Cyc__genarr_912),(void*)((struct _tuple5**)Cyc__genarr_912 + 3)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_72={1,1,(void*)((void*)& Cyc_struct_Absyn_Enumfield_rep)};
static unsigned char _tmp288[5]="List";static struct _tagged_arr Cyc__genname_917={
_tmp288,_tmp288,_tmp288 + 5};static unsigned char _tmp289[3]="hd";static struct
_tuple5 Cyc__gentuple_914={offsetof(struct Cyc_List_List,hd),{_tmp289,_tmp289,
_tmp289 + 3},(void*)& Cyc__genrep_72};static unsigned char _tmp28A[3]="tl";static
struct _tuple5 Cyc__gentuple_915={offsetof(struct Cyc_List_List,tl),{_tmp28A,
_tmp28A,_tmp28A + 3},(void*)& Cyc__genrep_71};static struct _tuple5*Cyc__genarr_916[
2]={& Cyc__gentuple_914,& Cyc__gentuple_915};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_enumfield_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_917,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_916),(void*)((struct _tuple5**)Cyc__genarr_916),(void*)((
struct _tuple5**)Cyc__genarr_916 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_71={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_enumfield_t46H2_rep)};static
unsigned char _tmp28D[4]="Opt";static struct _tagged_arr Cyc__genname_254={_tmp28D,
_tmp28D,_tmp28D + 4};static unsigned char _tmp28E[2]="v";static struct _tuple5 Cyc__gentuple_252={
offsetof(struct Cyc_Core_Opt,v),{_tmp28E,_tmp28E,_tmp28E + 2},(void*)& Cyc__genrep_71};
static struct _tuple5*Cyc__genarr_253[1]={& Cyc__gentuple_252};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0List_list_t0Absyn_enumfield_t46H22_rep={3,(struct _tagged_arr*)&
Cyc__genname_254,sizeof(struct Cyc_Core_Opt),{(void*)((struct _tuple5**)Cyc__genarr_253),(
void*)((struct _tuple5**)Cyc__genarr_253),(void*)((struct _tuple5**)Cyc__genarr_253
+ 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_251={1,1,(void*)((void*)&
Cyc_struct_Core_Opt0List_list_t0Absyn_enumfield_t46H22_rep)};static unsigned char
_tmp291[9]="Enumdecl";static struct _tagged_arr Cyc__genname_259={_tmp291,_tmp291,
_tmp291 + 9};static unsigned char _tmp292[3]="sc";static struct _tuple5 Cyc__gentuple_255={
offsetof(struct Cyc_Absyn_Enumdecl,sc),{_tmp292,_tmp292,_tmp292 + 3},(void*)& Cyc_Absyn_scope_t_rep};
static unsigned char _tmp293[5]="name";static struct _tuple5 Cyc__gentuple_256={
offsetof(struct Cyc_Absyn_Enumdecl,name),{_tmp293,_tmp293,_tmp293 + 5},(void*)& Cyc__genrep_10};
static unsigned char _tmp294[7]="fields";static struct _tuple5 Cyc__gentuple_257={
offsetof(struct Cyc_Absyn_Enumdecl,fields),{_tmp294,_tmp294,_tmp294 + 7},(void*)&
Cyc__genrep_251};static struct _tuple5*Cyc__genarr_258[3]={& Cyc__gentuple_255,& Cyc__gentuple_256,&
Cyc__gentuple_257};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Enumdecl_rep={
3,(struct _tagged_arr*)& Cyc__genname_259,sizeof(struct Cyc_Absyn_Enumdecl),{(void*)((
struct _tuple5**)Cyc__genarr_258),(void*)((struct _tuple5**)Cyc__genarr_258),(void*)((
struct _tuple5**)Cyc__genarr_258 + 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_250={
1,1,(void*)((void*)& Cyc_struct_Absyn_Enumdecl_rep)};struct _tuple45{unsigned int
f1;struct Cyc_Absyn_Enumdecl*f2;struct Cyc_Absyn_Enumfield*f3;};static struct
_tuple6 Cyc__gentuple_260={offsetof(struct _tuple45,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_261={offsetof(struct _tuple45,f2),(void*)& Cyc__genrep_250};
static struct _tuple6 Cyc__gentuple_262={offsetof(struct _tuple45,f3),(void*)& Cyc__genrep_72};
static struct _tuple6*Cyc__genarr_263[3]={& Cyc__gentuple_260,& Cyc__gentuple_261,&
Cyc__gentuple_262};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_249={4,
sizeof(struct _tuple45),{(void*)((struct _tuple6**)Cyc__genarr_263),(void*)((
struct _tuple6**)Cyc__genarr_263),(void*)((struct _tuple6**)Cyc__genarr_263 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_244;struct _tuple46{unsigned int
f1;void*f2;struct Cyc_Absyn_Enumfield*f3;};static struct _tuple6 Cyc__gentuple_245={
offsetof(struct _tuple46,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_246={
offsetof(struct _tuple46,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6 Cyc__gentuple_247={offsetof(struct _tuple46,f3),(void*)& Cyc__genrep_72};
static struct _tuple6*Cyc__genarr_248[3]={& Cyc__gentuple_245,& Cyc__gentuple_246,&
Cyc__gentuple_247};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_244={4,
sizeof(struct _tuple46),{(void*)((struct _tuple6**)Cyc__genarr_248),(void*)((
struct _tuple6**)Cyc__genarr_248),(void*)((struct _tuple6**)Cyc__genarr_248 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_240;struct _tuple47{unsigned int
f1;struct _tuple0*f2;};static struct _tuple6 Cyc__gentuple_241={offsetof(struct
_tuple47,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_242={
offsetof(struct _tuple47,f2),(void*)& Cyc__genrep_10};static struct _tuple6*Cyc__genarr_243[
2]={& Cyc__gentuple_241,& Cyc__gentuple_242};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_240={4,sizeof(struct _tuple47),{(void*)((struct _tuple6**)Cyc__genarr_243),(
void*)((struct _tuple6**)Cyc__genarr_243),(void*)((struct _tuple6**)Cyc__genarr_243
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_230;struct _tuple48{
unsigned int f1;struct _tuple0*f2;struct Cyc_List_List*f3;};static struct _tuple6 Cyc__gentuple_236={
offsetof(struct _tuple48,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_237={
offsetof(struct _tuple48,f2),(void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_238={
offsetof(struct _tuple48,f3),(void*)& Cyc__genrep_231};static struct _tuple6*Cyc__genarr_239[
3]={& Cyc__gentuple_236,& Cyc__gentuple_237,& Cyc__gentuple_238};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_230={4,sizeof(struct _tuple48),{(void*)((struct _tuple6**)Cyc__genarr_239),(
void*)((struct _tuple6**)Cyc__genarr_239),(void*)((struct _tuple6**)Cyc__genarr_239
+ 3)}};static unsigned char _tmp29B[7]="Wild_p";static struct _tuple7 Cyc__gentuple_227={
0,{_tmp29B,_tmp29B,_tmp29B + 7}};static unsigned char _tmp29C[7]="Null_p";static
struct _tuple7 Cyc__gentuple_228={1,{_tmp29C,_tmp29C,_tmp29C + 7}};static struct
_tuple7*Cyc__genarr_229[2]={& Cyc__gentuple_227,& Cyc__gentuple_228};static
unsigned char _tmp29D[6]="Var_p";static struct _tuple5 Cyc__gentuple_416={0,{_tmp29D,
_tmp29D,_tmp29D + 6},(void*)& Cyc__genrep_391};static unsigned char _tmp29E[6]="Int_p";
static struct _tuple5 Cyc__gentuple_417={1,{_tmp29E,_tmp29E,_tmp29E + 6},(void*)& Cyc__genrep_407};
static unsigned char _tmp29F[7]="Char_p";static struct _tuple5 Cyc__gentuple_418={2,{
_tmp29F,_tmp29F,_tmp29F + 7},(void*)& Cyc__genrep_403};static unsigned char _tmp2A0[
8]="Float_p";static struct _tuple5 Cyc__gentuple_419={3,{_tmp2A0,_tmp2A0,_tmp2A0 + 8},(
void*)& Cyc__genrep_112};static unsigned char _tmp2A1[8]="Tuple_p";static struct
_tuple5 Cyc__gentuple_420={4,{_tmp2A1,_tmp2A1,_tmp2A1 + 8},(void*)& Cyc__genrep_399};
static unsigned char _tmp2A2[10]="Pointer_p";static struct _tuple5 Cyc__gentuple_421={
5,{_tmp2A2,_tmp2A2,_tmp2A2 + 10},(void*)& Cyc__genrep_395};static unsigned char
_tmp2A3[12]="Reference_p";static struct _tuple5 Cyc__gentuple_422={6,{_tmp2A3,
_tmp2A3,_tmp2A3 + 12},(void*)& Cyc__genrep_391};static unsigned char _tmp2A4[7]="Aggr_p";
static struct _tuple5 Cyc__gentuple_423={7,{_tmp2A4,_tmp2A4,_tmp2A4 + 7},(void*)& Cyc__genrep_309};
static unsigned char _tmp2A5[9]="Tunion_p";static struct _tuple5 Cyc__gentuple_424={8,{
_tmp2A5,_tmp2A5,_tmp2A5 + 9},(void*)& Cyc__genrep_264};static unsigned char _tmp2A6[
7]="Enum_p";static struct _tuple5 Cyc__gentuple_425={9,{_tmp2A6,_tmp2A6,_tmp2A6 + 7},(
void*)& Cyc__genrep_249};static unsigned char _tmp2A7[11]="AnonEnum_p";static struct
_tuple5 Cyc__gentuple_426={10,{_tmp2A7,_tmp2A7,_tmp2A7 + 11},(void*)& Cyc__genrep_244};
static unsigned char _tmp2A8[12]="UnknownId_p";static struct _tuple5 Cyc__gentuple_427={
11,{_tmp2A8,_tmp2A8,_tmp2A8 + 12},(void*)& Cyc__genrep_240};static unsigned char
_tmp2A9[14]="UnknownCall_p";static struct _tuple5 Cyc__gentuple_428={12,{_tmp2A9,
_tmp2A9,_tmp2A9 + 14},(void*)& Cyc__genrep_230};static struct _tuple5*Cyc__genarr_429[
13]={& Cyc__gentuple_416,& Cyc__gentuple_417,& Cyc__gentuple_418,& Cyc__gentuple_419,&
Cyc__gentuple_420,& Cyc__gentuple_421,& Cyc__gentuple_422,& Cyc__gentuple_423,& Cyc__gentuple_424,&
Cyc__gentuple_425,& Cyc__gentuple_426,& Cyc__gentuple_427,& Cyc__gentuple_428};
static unsigned char _tmp2AB[8]="Raw_pat";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_pat_t_rep={
5,{_tmp2AB,_tmp2AB,_tmp2AB + 8},{(void*)((struct _tuple7**)Cyc__genarr_229),(void*)((
struct _tuple7**)Cyc__genarr_229),(void*)((struct _tuple7**)Cyc__genarr_229 + 2)},{(
void*)((struct _tuple5**)Cyc__genarr_429),(void*)((struct _tuple5**)Cyc__genarr_429),(
void*)((struct _tuple5**)Cyc__genarr_429 + 13)}};static unsigned char _tmp2AC[4]="Pat";
static struct _tagged_arr Cyc__genname_434={_tmp2AC,_tmp2AC,_tmp2AC + 4};static
unsigned char _tmp2AD[2]="r";static struct _tuple5 Cyc__gentuple_430={offsetof(
struct Cyc_Absyn_Pat,r),{_tmp2AD,_tmp2AD,_tmp2AD + 2},(void*)& Cyc_Absyn_raw_pat_t_rep};
static unsigned char _tmp2AE[5]="topt";static struct _tuple5 Cyc__gentuple_431={
offsetof(struct Cyc_Absyn_Pat,topt),{_tmp2AE,_tmp2AE,_tmp2AE + 5},(void*)& Cyc__genrep_61};
static unsigned char _tmp2AF[4]="loc";static struct _tuple5 Cyc__gentuple_432={
offsetof(struct Cyc_Absyn_Pat,loc),{_tmp2AF,_tmp2AF,_tmp2AF + 4},(void*)& Cyc__genrep_2};
static struct _tuple5*Cyc__genarr_433[3]={& Cyc__gentuple_430,& Cyc__gentuple_431,&
Cyc__gentuple_432};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Pat_rep={3,(
struct _tagged_arr*)& Cyc__genname_434,sizeof(struct Cyc_Absyn_Pat),{(void*)((
struct _tuple5**)Cyc__genarr_433),(void*)((struct _tuple5**)Cyc__genarr_433),(void*)((
struct _tuple5**)Cyc__genarr_433 + 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_226={
1,1,(void*)((void*)& Cyc_struct_Absyn_Pat_rep)};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_129;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0List_list_t0Absyn_vardecl_t46H22_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_130;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_List_List0Absyn_vardecl_t46H2_rep;static unsigned char _tmp2B2[5]="List";
static struct _tagged_arr Cyc__genname_155={_tmp2B2,_tmp2B2,_tmp2B2 + 5};static
unsigned char _tmp2B3[3]="hd";static struct _tuple5 Cyc__gentuple_152={offsetof(
struct Cyc_List_List,hd),{_tmp2B3,_tmp2B3,_tmp2B3 + 3},(void*)& Cyc__genrep_131};
static unsigned char _tmp2B4[3]="tl";static struct _tuple5 Cyc__gentuple_153={
offsetof(struct Cyc_List_List,tl),{_tmp2B4,_tmp2B4,_tmp2B4 + 3},(void*)& Cyc__genrep_130};
static struct _tuple5*Cyc__genarr_154[2]={& Cyc__gentuple_152,& Cyc__gentuple_153};
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_vardecl_t46H2_rep={3,(
struct _tagged_arr*)& Cyc__genname_155,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_154),(void*)((struct _tuple5**)Cyc__genarr_154),(void*)((
struct _tuple5**)Cyc__genarr_154 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_130={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_vardecl_t46H2_rep)};static
unsigned char _tmp2B7[4]="Opt";static struct _tagged_arr Cyc__genname_158={_tmp2B7,
_tmp2B7,_tmp2B7 + 4};static unsigned char _tmp2B8[2]="v";static struct _tuple5 Cyc__gentuple_156={
offsetof(struct Cyc_Core_Opt,v),{_tmp2B8,_tmp2B8,_tmp2B8 + 2},(void*)& Cyc__genrep_130};
static struct _tuple5*Cyc__genarr_157[1]={& Cyc__gentuple_156};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0List_list_t0Absyn_vardecl_t46H22_rep={3,(struct _tagged_arr*)&
Cyc__genname_158,sizeof(struct Cyc_Core_Opt),{(void*)((struct _tuple5**)Cyc__genarr_157),(
void*)((struct _tuple5**)Cyc__genarr_157),(void*)((struct _tuple5**)Cyc__genarr_157
+ 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_129={1,1,(void*)((void*)&
Cyc_struct_Core_Opt0List_list_t0Absyn_vardecl_t46H22_rep)};static unsigned char
_tmp2BB[14]="Switch_clause";static struct _tagged_arr Cyc__genname_441={_tmp2BB,
_tmp2BB,_tmp2BB + 14};static unsigned char _tmp2BC[8]="pattern";static struct _tuple5
Cyc__gentuple_435={offsetof(struct Cyc_Absyn_Switch_clause,pattern),{_tmp2BC,
_tmp2BC,_tmp2BC + 8},(void*)& Cyc__genrep_226};static unsigned char _tmp2BD[9]="pat_vars";
static struct _tuple5 Cyc__gentuple_436={offsetof(struct Cyc_Absyn_Switch_clause,pat_vars),{
_tmp2BD,_tmp2BD,_tmp2BD + 9},(void*)& Cyc__genrep_129};static unsigned char _tmp2BE[
13]="where_clause";static struct _tuple5 Cyc__gentuple_437={offsetof(struct Cyc_Absyn_Switch_clause,where_clause),{
_tmp2BE,_tmp2BE,_tmp2BE + 13},(void*)& Cyc__genrep_73};static unsigned char _tmp2BF[
5]="body";static struct _tuple5 Cyc__gentuple_438={offsetof(struct Cyc_Absyn_Switch_clause,body),{
_tmp2BF,_tmp2BF,_tmp2BF + 5},(void*)& Cyc__genrep_159};static unsigned char _tmp2C0[
4]="loc";static struct _tuple5 Cyc__gentuple_439={offsetof(struct Cyc_Absyn_Switch_clause,loc),{
_tmp2C0,_tmp2C0,_tmp2C0 + 4},(void*)& Cyc__genrep_2};static struct _tuple5*Cyc__genarr_440[
5]={& Cyc__gentuple_435,& Cyc__gentuple_436,& Cyc__gentuple_437,& Cyc__gentuple_438,&
Cyc__gentuple_439};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Switch_clause_rep={
3,(struct _tagged_arr*)& Cyc__genname_441,sizeof(struct Cyc_Absyn_Switch_clause),{(
void*)((struct _tuple5**)Cyc__genarr_440),(void*)((struct _tuple5**)Cyc__genarr_440),(
void*)((struct _tuple5**)Cyc__genarr_440 + 5)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_225={1,1,(void*)((void*)& Cyc_struct_Absyn_Switch_clause_rep)};static
unsigned char _tmp2C3[5]="List";static struct _tagged_arr Cyc__genname_445={_tmp2C3,
_tmp2C3,_tmp2C3 + 5};static unsigned char _tmp2C4[3]="hd";static struct _tuple5 Cyc__gentuple_442={
offsetof(struct Cyc_List_List,hd),{_tmp2C4,_tmp2C4,_tmp2C4 + 3},(void*)((void*)&
Cyc__genrep_225)};static unsigned char _tmp2C5[3]="tl";static struct _tuple5 Cyc__gentuple_443={
offsetof(struct Cyc_List_List,tl),{_tmp2C5,_tmp2C5,_tmp2C5 + 3},(void*)& Cyc__genrep_224};
static struct _tuple5*Cyc__genarr_444[2]={& Cyc__gentuple_442,& Cyc__gentuple_443};
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_switch_clause_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_445,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_444),(void*)((struct _tuple5**)Cyc__genarr_444),(void*)((
struct _tuple5**)Cyc__genarr_444 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_224={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_switch_clause_t46H2_rep)};struct
_tuple49{unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_List_List*f3;};static
struct _tuple6 Cyc__gentuple_497={offsetof(struct _tuple49,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_498={offsetof(struct _tuple49,f2),(void*)& Cyc__genrep_77};
static struct _tuple6 Cyc__gentuple_499={offsetof(struct _tuple49,f3),(void*)& Cyc__genrep_224};
static struct _tuple6*Cyc__genarr_500[3]={& Cyc__gentuple_497,& Cyc__gentuple_498,&
Cyc__gentuple_499};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_496={4,
sizeof(struct _tuple49),{(void*)((struct _tuple6**)Cyc__genarr_500),(void*)((
struct _tuple6**)Cyc__genarr_500),(void*)((struct _tuple6**)Cyc__genarr_500 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_480;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_481;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_switchC_clause_t46H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_482;extern struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_SwitchC_clause_rep;static unsigned char _tmp2C9[15]="SwitchC_clause";
static struct _tagged_arr Cyc__genname_487={_tmp2C9,_tmp2C9,_tmp2C9 + 15};static
unsigned char _tmp2CA[9]="cnst_exp";static struct _tuple5 Cyc__gentuple_483={
offsetof(struct Cyc_Absyn_SwitchC_clause,cnst_exp),{_tmp2CA,_tmp2CA,_tmp2CA + 9},(
void*)& Cyc__genrep_73};static unsigned char _tmp2CB[5]="body";static struct _tuple5
Cyc__gentuple_484={offsetof(struct Cyc_Absyn_SwitchC_clause,body),{_tmp2CB,
_tmp2CB,_tmp2CB + 5},(void*)& Cyc__genrep_159};static unsigned char _tmp2CC[4]="loc";
static struct _tuple5 Cyc__gentuple_485={offsetof(struct Cyc_Absyn_SwitchC_clause,loc),{
_tmp2CC,_tmp2CC,_tmp2CC + 4},(void*)& Cyc__genrep_2};static struct _tuple5*Cyc__genarr_486[
3]={& Cyc__gentuple_483,& Cyc__gentuple_484,& Cyc__gentuple_485};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_SwitchC_clause_rep={3,(struct _tagged_arr*)& Cyc__genname_487,
sizeof(struct Cyc_Absyn_SwitchC_clause),{(void*)((struct _tuple5**)Cyc__genarr_486),(
void*)((struct _tuple5**)Cyc__genarr_486),(void*)((struct _tuple5**)Cyc__genarr_486
+ 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_482={1,1,(void*)((void*)&
Cyc_struct_Absyn_SwitchC_clause_rep)};static unsigned char _tmp2CF[5]="List";
static struct _tagged_arr Cyc__genname_491={_tmp2CF,_tmp2CF,_tmp2CF + 5};static
unsigned char _tmp2D0[3]="hd";static struct _tuple5 Cyc__gentuple_488={offsetof(
struct Cyc_List_List,hd),{_tmp2D0,_tmp2D0,_tmp2D0 + 3},(void*)& Cyc__genrep_482};
static unsigned char _tmp2D1[3]="tl";static struct _tuple5 Cyc__gentuple_489={
offsetof(struct Cyc_List_List,tl),{_tmp2D1,_tmp2D1,_tmp2D1 + 3},(void*)& Cyc__genrep_481};
static struct _tuple5*Cyc__genarr_490[2]={& Cyc__gentuple_488,& Cyc__gentuple_489};
struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_switchC_clause_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_491,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_490),(void*)((struct _tuple5**)Cyc__genarr_490),(void*)((
struct _tuple5**)Cyc__genarr_490 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_481={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_switchC_clause_t46H2_rep)};static
struct _tuple6 Cyc__gentuple_492={offsetof(struct _tuple49,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_493={offsetof(struct _tuple49,f2),(void*)& Cyc__genrep_77};
static struct _tuple6 Cyc__gentuple_494={offsetof(struct _tuple49,f3),(void*)& Cyc__genrep_481};
static struct _tuple6*Cyc__genarr_495[3]={& Cyc__gentuple_492,& Cyc__gentuple_493,&
Cyc__gentuple_494};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_480={4,
sizeof(struct _tuple49),{(void*)((struct _tuple6**)Cyc__genarr_495),(void*)((
struct _tuple6**)Cyc__genarr_495),(void*)((struct _tuple6**)Cyc__genarr_495 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_469;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_471;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_exp_t46H2_rep;
static unsigned char _tmp2D5[5]="List";static struct _tagged_arr Cyc__genname_475={
_tmp2D5,_tmp2D5,_tmp2D5 + 5};static unsigned char _tmp2D6[3]="hd";static struct
_tuple5 Cyc__gentuple_472={offsetof(struct Cyc_List_List,hd),{_tmp2D6,_tmp2D6,
_tmp2D6 + 3},(void*)& Cyc__genrep_77};static unsigned char _tmp2D7[3]="tl";static
struct _tuple5 Cyc__gentuple_473={offsetof(struct Cyc_List_List,tl),{_tmp2D7,
_tmp2D7,_tmp2D7 + 3},(void*)& Cyc__genrep_471};static struct _tuple5*Cyc__genarr_474[
2]={& Cyc__gentuple_472,& Cyc__gentuple_473};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_exp_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_475,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_474),(void*)((struct _tuple5**)Cyc__genarr_474),(void*)((
struct _tuple5**)Cyc__genarr_474 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_471={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_exp_t46H2_rep)};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_470;static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_470={1,1,(void*)((
void*)& Cyc__genrep_225)};struct _tuple50{unsigned int f1;struct Cyc_List_List*f2;
struct Cyc_Absyn_Switch_clause**f3;};static struct _tuple6 Cyc__gentuple_476={
offsetof(struct _tuple50,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_477={
offsetof(struct _tuple50,f2),(void*)& Cyc__genrep_471};static struct _tuple6 Cyc__gentuple_478={
offsetof(struct _tuple50,f3),(void*)& Cyc__genrep_470};static struct _tuple6*Cyc__genarr_479[
3]={& Cyc__gentuple_476,& Cyc__gentuple_477,& Cyc__gentuple_478};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_469={4,sizeof(struct _tuple50),{(void*)((struct _tuple6**)Cyc__genarr_479),(
void*)((struct _tuple6**)Cyc__genarr_479),(void*)((struct _tuple6**)Cyc__genarr_479
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_464;struct _tuple51{
unsigned int f1;struct Cyc_Absyn_Decl*f2;struct Cyc_Absyn_Stmt*f3;};static struct
_tuple6 Cyc__gentuple_465={offsetof(struct _tuple51,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_466={offsetof(struct _tuple51,f2),(void*)& Cyc__genrep_1};
static struct _tuple6 Cyc__gentuple_467={offsetof(struct _tuple51,f3),(void*)& Cyc__genrep_159};
static struct _tuple6*Cyc__genarr_468[3]={& Cyc__gentuple_465,& Cyc__gentuple_466,&
Cyc__gentuple_467};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_464={4,
sizeof(struct _tuple51),{(void*)((struct _tuple6**)Cyc__genarr_468),(void*)((
struct _tuple6**)Cyc__genarr_468),(void*)((struct _tuple6**)Cyc__genarr_468 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_460;static struct _tuple6 Cyc__gentuple_461={
offsetof(struct _tuple32,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_462={
offsetof(struct _tuple32,f2),(void*)& Cyc__genrep_159};static struct _tuple6*Cyc__genarr_463[
2]={& Cyc__gentuple_461,& Cyc__gentuple_462};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_460={4,sizeof(struct _tuple32),{(void*)((struct _tuple6**)Cyc__genarr_463),(
void*)((struct _tuple6**)Cyc__genarr_463),(void*)((struct _tuple6**)Cyc__genarr_463
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_455;static struct _tuple6
Cyc__gentuple_456={offsetof(struct _tuple33,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_457={offsetof(struct _tuple33,f2),(void*)& Cyc__genrep_12};
static struct _tuple6 Cyc__gentuple_458={offsetof(struct _tuple33,f3),(void*)& Cyc__genrep_159};
static struct _tuple6*Cyc__genarr_459[3]={& Cyc__gentuple_456,& Cyc__gentuple_457,&
Cyc__gentuple_458};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_455={4,
sizeof(struct _tuple33),{(void*)((struct _tuple6**)Cyc__genarr_459),(void*)((
struct _tuple6**)Cyc__genarr_459),(void*)((struct _tuple6**)Cyc__genarr_459 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_450;struct _tuple52{unsigned int
f1;struct Cyc_Absyn_Stmt*f2;struct _tuple2 f3;};static struct _tuple6 Cyc__gentuple_451={
offsetof(struct _tuple52,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_452={
offsetof(struct _tuple52,f2),(void*)& Cyc__genrep_159};static struct _tuple6 Cyc__gentuple_453={
offsetof(struct _tuple52,f3),(void*)& Cyc__genrep_168};static struct _tuple6*Cyc__genarr_454[
3]={& Cyc__gentuple_451,& Cyc__gentuple_452,& Cyc__gentuple_453};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_450={4,sizeof(struct _tuple52),{(void*)((struct _tuple6**)Cyc__genarr_454),(
void*)((struct _tuple6**)Cyc__genarr_454),(void*)((struct _tuple6**)Cyc__genarr_454
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_223;struct _tuple53{
unsigned int f1;struct Cyc_Absyn_Stmt*f2;struct Cyc_List_List*f3;};static struct
_tuple6 Cyc__gentuple_446={offsetof(struct _tuple53,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_447={offsetof(struct _tuple53,f2),(void*)& Cyc__genrep_159};
static struct _tuple6 Cyc__gentuple_448={offsetof(struct _tuple53,f3),(void*)& Cyc__genrep_224};
static struct _tuple6*Cyc__genarr_449[3]={& Cyc__gentuple_446,& Cyc__gentuple_447,&
Cyc__gentuple_448};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_223={4,
sizeof(struct _tuple53),{(void*)((struct _tuple6**)Cyc__genarr_449),(void*)((
struct _tuple6**)Cyc__genarr_449),(void*)((struct _tuple6**)Cyc__genarr_449 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_181;struct _tuple54{unsigned int
f1;struct Cyc_Absyn_Tvar*f2;struct Cyc_Absyn_Vardecl*f3;struct Cyc_Absyn_Stmt*f4;};
static struct _tuple6 Cyc__gentuple_218={offsetof(struct _tuple54,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_219={offsetof(struct _tuple54,f2),(void*)& Cyc__genrep_182};
static struct _tuple6 Cyc__gentuple_220={offsetof(struct _tuple54,f3),(void*)& Cyc__genrep_131};
static struct _tuple6 Cyc__gentuple_221={offsetof(struct _tuple54,f4),(void*)& Cyc__genrep_159};
static struct _tuple6*Cyc__genarr_222[4]={& Cyc__gentuple_218,& Cyc__gentuple_219,&
Cyc__gentuple_220,& Cyc__gentuple_221};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_181={
4,sizeof(struct _tuple54),{(void*)((struct _tuple6**)Cyc__genarr_222),(void*)((
struct _tuple6**)Cyc__genarr_222),(void*)((struct _tuple6**)Cyc__genarr_222 + 4)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_167;extern struct Cyc_Typerep_Struct_struct
Cyc_Absyn_forarray_info_t_rep;static unsigned char _tmp2E2[13]="ForArrayInfo";
static struct _tagged_arr Cyc__genname_177={_tmp2E2,_tmp2E2,_tmp2E2 + 13};static
unsigned char _tmp2E3[6]="defns";static struct _tuple5 Cyc__gentuple_172={offsetof(
struct Cyc_Absyn_ForArrayInfo,defns),{_tmp2E3,_tmp2E3,_tmp2E3 + 6},(void*)& Cyc__genrep_130};
static unsigned char _tmp2E4[10]="condition";static struct _tuple5 Cyc__gentuple_173={
offsetof(struct Cyc_Absyn_ForArrayInfo,condition),{_tmp2E4,_tmp2E4,_tmp2E4 + 10},(
void*)& Cyc__genrep_168};static unsigned char _tmp2E5[6]="delta";static struct
_tuple5 Cyc__gentuple_174={offsetof(struct Cyc_Absyn_ForArrayInfo,delta),{_tmp2E5,
_tmp2E5,_tmp2E5 + 6},(void*)& Cyc__genrep_168};static unsigned char _tmp2E6[5]="body";
static struct _tuple5 Cyc__gentuple_175={offsetof(struct Cyc_Absyn_ForArrayInfo,body),{
_tmp2E6,_tmp2E6,_tmp2E6 + 5},(void*)& Cyc__genrep_159};static struct _tuple5*Cyc__genarr_176[
4]={& Cyc__gentuple_172,& Cyc__gentuple_173,& Cyc__gentuple_174,& Cyc__gentuple_175};
struct Cyc_Typerep_Struct_struct Cyc_Absyn_forarray_info_t_rep={3,(struct
_tagged_arr*)& Cyc__genname_177,sizeof(struct Cyc_Absyn_ForArrayInfo),{(void*)((
struct _tuple5**)Cyc__genarr_176),(void*)((struct _tuple5**)Cyc__genarr_176),(void*)((
struct _tuple5**)Cyc__genarr_176 + 4)}};struct _tuple55{unsigned int f1;struct Cyc_Absyn_ForArrayInfo
f2;};static struct _tuple6 Cyc__gentuple_178={offsetof(struct _tuple55,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_179={offsetof(struct _tuple55,f2),(
void*)& Cyc_Absyn_forarray_info_t_rep};static struct _tuple6*Cyc__genarr_180[2]={&
Cyc__gentuple_178,& Cyc__gentuple_179};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_167={
4,sizeof(struct _tuple55),{(void*)((struct _tuple6**)Cyc__genarr_180),(void*)((
struct _tuple6**)Cyc__genarr_180),(void*)((struct _tuple6**)Cyc__genarr_180 + 2)}};
static unsigned char _tmp2E9[7]="Skip_s";static struct _tuple7 Cyc__gentuple_165={0,{
_tmp2E9,_tmp2E9,_tmp2E9 + 7}};static struct _tuple7*Cyc__genarr_166[1]={& Cyc__gentuple_165};
static unsigned char _tmp2EA[6]="Exp_s";static struct _tuple5 Cyc__gentuple_538={0,{
_tmp2EA,_tmp2EA,_tmp2EA + 6},(void*)& Cyc__genrep_76};static unsigned char _tmp2EB[6]="Seq_s";
static struct _tuple5 Cyc__gentuple_539={1,{_tmp2EB,_tmp2EB,_tmp2EB + 6},(void*)& Cyc__genrep_533};
static unsigned char _tmp2EC[9]="Return_s";static struct _tuple5 Cyc__gentuple_540={2,{
_tmp2EC,_tmp2EC,_tmp2EC + 9},(void*)& Cyc__genrep_529};static unsigned char _tmp2ED[
13]="IfThenElse_s";static struct _tuple5 Cyc__gentuple_541={3,{_tmp2ED,_tmp2ED,
_tmp2ED + 13},(void*)& Cyc__genrep_523};static unsigned char _tmp2EE[8]="While_s";
static struct _tuple5 Cyc__gentuple_542={4,{_tmp2EE,_tmp2EE,_tmp2EE + 8},(void*)& Cyc__genrep_518};
static unsigned char _tmp2EF[8]="Break_s";static struct _tuple5 Cyc__gentuple_543={5,{
_tmp2EF,_tmp2EF,_tmp2EF + 8},(void*)& Cyc__genrep_514};static unsigned char _tmp2F0[
11]="Continue_s";static struct _tuple5 Cyc__gentuple_544={6,{_tmp2F0,_tmp2F0,
_tmp2F0 + 11},(void*)& Cyc__genrep_514};static unsigned char _tmp2F1[7]="Goto_s";
static struct _tuple5 Cyc__gentuple_545={7,{_tmp2F1,_tmp2F1,_tmp2F1 + 7},(void*)& Cyc__genrep_508};
static unsigned char _tmp2F2[6]="For_s";static struct _tuple5 Cyc__gentuple_546={8,{
_tmp2F2,_tmp2F2,_tmp2F2 + 6},(void*)& Cyc__genrep_501};static unsigned char _tmp2F3[
9]="Switch_s";static struct _tuple5 Cyc__gentuple_547={9,{_tmp2F3,_tmp2F3,_tmp2F3 + 
9},(void*)& Cyc__genrep_496};static unsigned char _tmp2F4[10]="SwitchC_s";static
struct _tuple5 Cyc__gentuple_548={10,{_tmp2F4,_tmp2F4,_tmp2F4 + 10},(void*)& Cyc__genrep_480};
static unsigned char _tmp2F5[11]="Fallthru_s";static struct _tuple5 Cyc__gentuple_549={
11,{_tmp2F5,_tmp2F5,_tmp2F5 + 11},(void*)& Cyc__genrep_469};static unsigned char
_tmp2F6[7]="Decl_s";static struct _tuple5 Cyc__gentuple_550={12,{_tmp2F6,_tmp2F6,
_tmp2F6 + 7},(void*)& Cyc__genrep_464};static unsigned char _tmp2F7[6]="Cut_s";
static struct _tuple5 Cyc__gentuple_551={13,{_tmp2F7,_tmp2F7,_tmp2F7 + 6},(void*)&
Cyc__genrep_460};static unsigned char _tmp2F8[9]="Splice_s";static struct _tuple5 Cyc__gentuple_552={
14,{_tmp2F8,_tmp2F8,_tmp2F8 + 9},(void*)& Cyc__genrep_460};static unsigned char
_tmp2F9[8]="Label_s";static struct _tuple5 Cyc__gentuple_553={15,{_tmp2F9,_tmp2F9,
_tmp2F9 + 8},(void*)& Cyc__genrep_455};static unsigned char _tmp2FA[5]="Do_s";static
struct _tuple5 Cyc__gentuple_554={16,{_tmp2FA,_tmp2FA,_tmp2FA + 5},(void*)& Cyc__genrep_450};
static unsigned char _tmp2FB[11]="TryCatch_s";static struct _tuple5 Cyc__gentuple_555={
17,{_tmp2FB,_tmp2FB,_tmp2FB + 11},(void*)& Cyc__genrep_223};static unsigned char
_tmp2FC[9]="Region_s";static struct _tuple5 Cyc__gentuple_556={18,{_tmp2FC,_tmp2FC,
_tmp2FC + 9},(void*)& Cyc__genrep_181};static unsigned char _tmp2FD[11]="ForArray_s";
static struct _tuple5 Cyc__gentuple_557={19,{_tmp2FD,_tmp2FD,_tmp2FD + 11},(void*)&
Cyc__genrep_167};static struct _tuple5*Cyc__genarr_558[20]={& Cyc__gentuple_538,&
Cyc__gentuple_539,& Cyc__gentuple_540,& Cyc__gentuple_541,& Cyc__gentuple_542,& Cyc__gentuple_543,&
Cyc__gentuple_544,& Cyc__gentuple_545,& Cyc__gentuple_546,& Cyc__gentuple_547,& Cyc__gentuple_548,&
Cyc__gentuple_549,& Cyc__gentuple_550,& Cyc__gentuple_551,& Cyc__gentuple_552,& Cyc__gentuple_553,&
Cyc__gentuple_554,& Cyc__gentuple_555,& Cyc__gentuple_556,& Cyc__gentuple_557};
static unsigned char _tmp2FF[9]="Raw_stmt";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_stmt_t_rep={
5,{_tmp2FF,_tmp2FF,_tmp2FF + 9},{(void*)((struct _tuple7**)Cyc__genarr_166),(void*)((
struct _tuple7**)Cyc__genarr_166),(void*)((struct _tuple7**)Cyc__genarr_166 + 1)},{(
void*)((struct _tuple5**)Cyc__genarr_558),(void*)((struct _tuple5**)Cyc__genarr_558),(
void*)((struct _tuple5**)Cyc__genarr_558 + 20)}};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_160;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_stmt_t46H2_rep;
static unsigned char _tmp300[5]="List";static struct _tagged_arr Cyc__genname_164={
_tmp300,_tmp300,_tmp300 + 5};static unsigned char _tmp301[3]="hd";static struct
_tuple5 Cyc__gentuple_161={offsetof(struct Cyc_List_List,hd),{_tmp301,_tmp301,
_tmp301 + 3},(void*)& Cyc__genrep_159};static unsigned char _tmp302[3]="tl";static
struct _tuple5 Cyc__gentuple_162={offsetof(struct Cyc_List_List,tl),{_tmp302,
_tmp302,_tmp302 + 3},(void*)& Cyc__genrep_160};static struct _tuple5*Cyc__genarr_163[
2]={& Cyc__gentuple_161,& Cyc__gentuple_162};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_stmt_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_164,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_163),(void*)((struct _tuple5**)Cyc__genarr_163),(void*)((
struct _tuple5**)Cyc__genarr_163 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_160={
1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_stmt_t46H2_rep)};extern struct Cyc_Typerep_XTUnion_struct
Cyc_Absyn_absyn_annot_t_rep;static struct _tuple8*Cyc__genarr_74[0]={};static
unsigned char _tmp306[11]="AbsynAnnot";struct Cyc_Typerep_XTUnion_struct Cyc_Absyn_absyn_annot_t_rep={
7,{_tmp306,_tmp306,_tmp306 + 11},{(void*)((struct _tuple8**)Cyc__genarr_74),(void*)((
struct _tuple8**)Cyc__genarr_74),(void*)((struct _tuple8**)Cyc__genarr_74 + 0)}};
static unsigned char _tmp307[5]="Stmt";static struct _tagged_arr Cyc__genname_565={
_tmp307,_tmp307,_tmp307 + 5};static unsigned char _tmp308[2]="r";static struct
_tuple5 Cyc__gentuple_559={offsetof(struct Cyc_Absyn_Stmt,r),{_tmp308,_tmp308,
_tmp308 + 2},(void*)& Cyc_Absyn_raw_stmt_t_rep};static unsigned char _tmp309[4]="loc";
static struct _tuple5 Cyc__gentuple_560={offsetof(struct Cyc_Absyn_Stmt,loc),{
_tmp309,_tmp309,_tmp309 + 4},(void*)& Cyc__genrep_2};static unsigned char _tmp30A[16]="non_local_preds";
static struct _tuple5 Cyc__gentuple_561={offsetof(struct Cyc_Absyn_Stmt,non_local_preds),{
_tmp30A,_tmp30A,_tmp30A + 16},(void*)& Cyc__genrep_160};static unsigned char _tmp30B[
10]="try_depth";static struct _tuple5 Cyc__gentuple_562={offsetof(struct Cyc_Absyn_Stmt,try_depth),{
_tmp30B,_tmp30B,_tmp30B + 10},(void*)((void*)& Cyc__genrep_102)};static
unsigned char _tmp30C[6]="annot";static struct _tuple5 Cyc__gentuple_563={offsetof(
struct Cyc_Absyn_Stmt,annot),{_tmp30C,_tmp30C,_tmp30C + 6},(void*)& Cyc_Absyn_absyn_annot_t_rep};
static struct _tuple5*Cyc__genarr_564[5]={& Cyc__gentuple_559,& Cyc__gentuple_560,&
Cyc__gentuple_561,& Cyc__gentuple_562,& Cyc__gentuple_563};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Stmt_rep={3,(struct _tagged_arr*)& Cyc__genname_565,sizeof(struct
Cyc_Absyn_Stmt),{(void*)((struct _tuple5**)Cyc__genarr_564),(void*)((struct
_tuple5**)Cyc__genarr_564),(void*)((struct _tuple5**)Cyc__genarr_564 + 5)}};static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_159={1,1,(void*)((void*)& Cyc_struct_Absyn_Stmt_rep)};
static unsigned char _tmp30F[7]="Fndecl";static struct _tagged_arr Cyc__genname_613={
_tmp30F,_tmp30F,_tmp30F + 7};static unsigned char _tmp310[3]="sc";static struct
_tuple5 Cyc__gentuple_598={offsetof(struct Cyc_Absyn_Fndecl,sc),{_tmp310,_tmp310,
_tmp310 + 3},(void*)& Cyc_Absyn_scope_t_rep};static unsigned char _tmp311[10]="is_inline";
static struct _tuple5 Cyc__gentuple_599={offsetof(struct Cyc_Absyn_Fndecl,is_inline),{
_tmp311,_tmp311,_tmp311 + 10},(void*)((void*)& Cyc__genrep_102)};static
unsigned char _tmp312[5]="name";static struct _tuple5 Cyc__gentuple_600={offsetof(
struct Cyc_Absyn_Fndecl,name),{_tmp312,_tmp312,_tmp312 + 5},(void*)& Cyc__genrep_10};
static unsigned char _tmp313[4]="tvs";static struct _tuple5 Cyc__gentuple_601={
offsetof(struct Cyc_Absyn_Fndecl,tvs),{_tmp313,_tmp313,_tmp313 + 4},(void*)& Cyc__genrep_292};
static unsigned char _tmp314[7]="effect";static struct _tuple5 Cyc__gentuple_602={
offsetof(struct Cyc_Absyn_Fndecl,effect),{_tmp314,_tmp314,_tmp314 + 7},(void*)& Cyc__genrep_61};
static unsigned char _tmp315[9]="ret_type";static struct _tuple5 Cyc__gentuple_603={
offsetof(struct Cyc_Absyn_Fndecl,ret_type),{_tmp315,_tmp315,_tmp315 + 9},(void*)((
void*)& Cyc_Absyn_type_t_rep)};static unsigned char _tmp316[5]="args";static struct
_tuple5 Cyc__gentuple_604={offsetof(struct Cyc_Absyn_Fndecl,args),{_tmp316,_tmp316,
_tmp316 + 5},(void*)& Cyc__genrep_587};static unsigned char _tmp317[10]="c_varargs";
static struct _tuple5 Cyc__gentuple_605={offsetof(struct Cyc_Absyn_Fndecl,c_varargs),{
_tmp317,_tmp317,_tmp317 + 10},(void*)((void*)& Cyc__genrep_102)};static
unsigned char _tmp318[12]="cyc_varargs";static struct _tuple5 Cyc__gentuple_606={
offsetof(struct Cyc_Absyn_Fndecl,cyc_varargs),{_tmp318,_tmp318,_tmp318 + 12},(void*)&
Cyc__genrep_576};static unsigned char _tmp319[7]="rgn_po";static struct _tuple5 Cyc__gentuple_607={
offsetof(struct Cyc_Absyn_Fndecl,rgn_po),{_tmp319,_tmp319,_tmp319 + 7},(void*)& Cyc__genrep_566};
static unsigned char _tmp31A[5]="body";static struct _tuple5 Cyc__gentuple_608={
offsetof(struct Cyc_Absyn_Fndecl,body),{_tmp31A,_tmp31A,_tmp31A + 5},(void*)& Cyc__genrep_159};
static unsigned char _tmp31B[11]="cached_typ";static struct _tuple5 Cyc__gentuple_609={
offsetof(struct Cyc_Absyn_Fndecl,cached_typ),{_tmp31B,_tmp31B,_tmp31B + 11},(void*)&
Cyc__genrep_61};static unsigned char _tmp31C[15]="param_vardecls";static struct
_tuple5 Cyc__gentuple_610={offsetof(struct Cyc_Absyn_Fndecl,param_vardecls),{
_tmp31C,_tmp31C,_tmp31C + 15},(void*)& Cyc__genrep_129};static unsigned char _tmp31D[
11]="attributes";static struct _tuple5 Cyc__gentuple_611={offsetof(struct Cyc_Absyn_Fndecl,attributes),{
_tmp31D,_tmp31D,_tmp31D + 11},(void*)& Cyc__genrep_83};static struct _tuple5*Cyc__genarr_612[
14]={& Cyc__gentuple_598,& Cyc__gentuple_599,& Cyc__gentuple_600,& Cyc__gentuple_601,&
Cyc__gentuple_602,& Cyc__gentuple_603,& Cyc__gentuple_604,& Cyc__gentuple_605,& Cyc__gentuple_606,&
Cyc__gentuple_607,& Cyc__gentuple_608,& Cyc__gentuple_609,& Cyc__gentuple_610,& Cyc__gentuple_611};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Fndecl_rep={3,(struct _tagged_arr*)&
Cyc__genname_613,sizeof(struct Cyc_Absyn_Fndecl),{(void*)((struct _tuple5**)Cyc__genarr_612),(
void*)((struct _tuple5**)Cyc__genarr_612),(void*)((struct _tuple5**)Cyc__genarr_612
+ 14)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_82={1,1,(void*)((void*)&
Cyc_struct_Absyn_Fndecl_rep)};struct _tuple56{unsigned int f1;struct Cyc_Absyn_Fndecl*
f2;};static struct _tuple6 Cyc__gentuple_614={offsetof(struct _tuple56,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_615={offsetof(struct _tuple56,f2),(
void*)& Cyc__genrep_82};static struct _tuple6*Cyc__genarr_616[2]={& Cyc__gentuple_614,&
Cyc__gentuple_615};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_81={4,sizeof(
struct _tuple56),{(void*)((struct _tuple6**)Cyc__genarr_616),(void*)((struct
_tuple6**)Cyc__genarr_616),(void*)((struct _tuple6**)Cyc__genarr_616 + 2)}};static
unsigned char _tmp321[13]="Unresolved_b";static struct _tuple7 Cyc__gentuple_822={0,{
_tmp321,_tmp321,_tmp321 + 13}};static struct _tuple7*Cyc__genarr_823[1]={& Cyc__gentuple_822};
static unsigned char _tmp322[9]="Global_b";static struct _tuple5 Cyc__gentuple_824={0,{
_tmp322,_tmp322,_tmp322 + 9},(void*)& Cyc__genrep_391};static unsigned char _tmp323[
10]="Funname_b";static struct _tuple5 Cyc__gentuple_825={1,{_tmp323,_tmp323,_tmp323
+ 10},(void*)& Cyc__genrep_81};static unsigned char _tmp324[8]="Param_b";static
struct _tuple5 Cyc__gentuple_826={2,{_tmp324,_tmp324,_tmp324 + 8},(void*)& Cyc__genrep_391};
static unsigned char _tmp325[8]="Local_b";static struct _tuple5 Cyc__gentuple_827={3,{
_tmp325,_tmp325,_tmp325 + 8},(void*)& Cyc__genrep_391};static unsigned char _tmp326[
6]="Pat_b";static struct _tuple5 Cyc__gentuple_828={4,{_tmp326,_tmp326,_tmp326 + 6},(
void*)& Cyc__genrep_391};static struct _tuple5*Cyc__genarr_829[5]={& Cyc__gentuple_824,&
Cyc__gentuple_825,& Cyc__gentuple_826,& Cyc__gentuple_827,& Cyc__gentuple_828};
static unsigned char _tmp328[8]="Binding";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_binding_t_rep={
5,{_tmp328,_tmp328,_tmp328 + 8},{(void*)((struct _tuple7**)Cyc__genarr_823),(void*)((
struct _tuple7**)Cyc__genarr_823),(void*)((struct _tuple7**)Cyc__genarr_823 + 1)},{(
void*)((struct _tuple5**)Cyc__genarr_829),(void*)((struct _tuple5**)Cyc__genarr_829),(
void*)((struct _tuple5**)Cyc__genarr_829 + 5)}};struct _tuple57{unsigned int f1;
struct _tuple0*f2;void*f3;};static struct _tuple6 Cyc__gentuple_830={offsetof(struct
_tuple57,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_831={
offsetof(struct _tuple57,f2),(void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_832={
offsetof(struct _tuple57,f3),(void*)& Cyc_Absyn_binding_t_rep};static struct _tuple6*
Cyc__genarr_833[3]={& Cyc__gentuple_830,& Cyc__gentuple_831,& Cyc__gentuple_832};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_821={4,sizeof(struct _tuple57),{(
void*)((struct _tuple6**)Cyc__genarr_833),(void*)((struct _tuple6**)Cyc__genarr_833),(
void*)((struct _tuple6**)Cyc__genarr_833 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_816;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_primop_t_rep;
static unsigned char _tmp32A[5]="Plus";static struct _tuple7 Cyc__gentuple_786={0,{
_tmp32A,_tmp32A,_tmp32A + 5}};static unsigned char _tmp32B[6]="Times";static struct
_tuple7 Cyc__gentuple_787={1,{_tmp32B,_tmp32B,_tmp32B + 6}};static unsigned char
_tmp32C[6]="Minus";static struct _tuple7 Cyc__gentuple_788={2,{_tmp32C,_tmp32C,
_tmp32C + 6}};static unsigned char _tmp32D[4]="Div";static struct _tuple7 Cyc__gentuple_789={
3,{_tmp32D,_tmp32D,_tmp32D + 4}};static unsigned char _tmp32E[4]="Mod";static struct
_tuple7 Cyc__gentuple_790={4,{_tmp32E,_tmp32E,_tmp32E + 4}};static unsigned char
_tmp32F[3]="Eq";static struct _tuple7 Cyc__gentuple_791={5,{_tmp32F,_tmp32F,_tmp32F
+ 3}};static unsigned char _tmp330[4]="Neq";static struct _tuple7 Cyc__gentuple_792={
6,{_tmp330,_tmp330,_tmp330 + 4}};static unsigned char _tmp331[3]="Gt";static struct
_tuple7 Cyc__gentuple_793={7,{_tmp331,_tmp331,_tmp331 + 3}};static unsigned char
_tmp332[3]="Lt";static struct _tuple7 Cyc__gentuple_794={8,{_tmp332,_tmp332,_tmp332
+ 3}};static unsigned char _tmp333[4]="Gte";static struct _tuple7 Cyc__gentuple_795={
9,{_tmp333,_tmp333,_tmp333 + 4}};static unsigned char _tmp334[4]="Lte";static struct
_tuple7 Cyc__gentuple_796={10,{_tmp334,_tmp334,_tmp334 + 4}};static unsigned char
_tmp335[4]="Not";static struct _tuple7 Cyc__gentuple_797={11,{_tmp335,_tmp335,
_tmp335 + 4}};static unsigned char _tmp336[7]="Bitnot";static struct _tuple7 Cyc__gentuple_798={
12,{_tmp336,_tmp336,_tmp336 + 7}};static unsigned char _tmp337[7]="Bitand";static
struct _tuple7 Cyc__gentuple_799={13,{_tmp337,_tmp337,_tmp337 + 7}};static
unsigned char _tmp338[6]="Bitor";static struct _tuple7 Cyc__gentuple_800={14,{
_tmp338,_tmp338,_tmp338 + 6}};static unsigned char _tmp339[7]="Bitxor";static struct
_tuple7 Cyc__gentuple_801={15,{_tmp339,_tmp339,_tmp339 + 7}};static unsigned char
_tmp33A[10]="Bitlshift";static struct _tuple7 Cyc__gentuple_802={16,{_tmp33A,
_tmp33A,_tmp33A + 10}};static unsigned char _tmp33B[11]="Bitlrshift";static struct
_tuple7 Cyc__gentuple_803={17,{_tmp33B,_tmp33B,_tmp33B + 11}};static unsigned char
_tmp33C[11]="Bitarshift";static struct _tuple7 Cyc__gentuple_804={18,{_tmp33C,
_tmp33C,_tmp33C + 11}};static unsigned char _tmp33D[5]="Size";static struct _tuple7
Cyc__gentuple_805={19,{_tmp33D,_tmp33D,_tmp33D + 5}};static struct _tuple7*Cyc__genarr_806[
20]={& Cyc__gentuple_786,& Cyc__gentuple_787,& Cyc__gentuple_788,& Cyc__gentuple_789,&
Cyc__gentuple_790,& Cyc__gentuple_791,& Cyc__gentuple_792,& Cyc__gentuple_793,& Cyc__gentuple_794,&
Cyc__gentuple_795,& Cyc__gentuple_796,& Cyc__gentuple_797,& Cyc__gentuple_798,& Cyc__gentuple_799,&
Cyc__gentuple_800,& Cyc__gentuple_801,& Cyc__gentuple_802,& Cyc__gentuple_803,& Cyc__gentuple_804,&
Cyc__gentuple_805};static struct _tuple5*Cyc__genarr_807[0]={};static unsigned char
_tmp33F[7]="Primop";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_primop_t_rep={5,{
_tmp33F,_tmp33F,_tmp33F + 7},{(void*)((struct _tuple7**)Cyc__genarr_806),(void*)((
struct _tuple7**)Cyc__genarr_806),(void*)((struct _tuple7**)Cyc__genarr_806 + 20)},{(
void*)((struct _tuple5**)Cyc__genarr_807),(void*)((struct _tuple5**)Cyc__genarr_807),(
void*)((struct _tuple5**)Cyc__genarr_807 + 0)}};struct _tuple58{unsigned int f1;void*
f2;struct Cyc_List_List*f3;};static struct _tuple6 Cyc__gentuple_817={offsetof(
struct _tuple58,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_818={
offsetof(struct _tuple58,f2),(void*)& Cyc_Absyn_primop_t_rep};static struct _tuple6
Cyc__gentuple_819={offsetof(struct _tuple58,f3),(void*)& Cyc__genrep_471};static
struct _tuple6*Cyc__genarr_820[3]={& Cyc__gentuple_817,& Cyc__gentuple_818,& Cyc__gentuple_819};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_816={4,sizeof(struct _tuple58),{(
void*)((struct _tuple6**)Cyc__genarr_820),(void*)((struct _tuple6**)Cyc__genarr_820),(
void*)((struct _tuple6**)Cyc__genarr_820 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_784;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_785;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_primop_t2_rep;static
unsigned char _tmp341[4]="Opt";static struct _tagged_arr Cyc__genname_810={_tmp341,
_tmp341,_tmp341 + 4};static unsigned char _tmp342[2]="v";static struct _tuple5 Cyc__gentuple_808={
offsetof(struct Cyc_Core_Opt,v),{_tmp342,_tmp342,_tmp342 + 2},(void*)& Cyc_Absyn_primop_t_rep};
static struct _tuple5*Cyc__genarr_809[1]={& Cyc__gentuple_808};struct Cyc_Typerep_Struct_struct
Cyc_struct_Core_Opt0Absyn_primop_t2_rep={3,(struct _tagged_arr*)& Cyc__genname_810,
sizeof(struct Cyc_Core_Opt),{(void*)((struct _tuple5**)Cyc__genarr_809),(void*)((
struct _tuple5**)Cyc__genarr_809),(void*)((struct _tuple5**)Cyc__genarr_809 + 1)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_785={1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_primop_t2_rep)};
struct _tuple59{unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Core_Opt*f3;
struct Cyc_Absyn_Exp*f4;};static struct _tuple6 Cyc__gentuple_811={offsetof(struct
_tuple59,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_812={
offsetof(struct _tuple59,f2),(void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_813={
offsetof(struct _tuple59,f3),(void*)& Cyc__genrep_785};static struct _tuple6 Cyc__gentuple_814={
offsetof(struct _tuple59,f4),(void*)& Cyc__genrep_77};static struct _tuple6*Cyc__genarr_815[
4]={& Cyc__gentuple_811,& Cyc__gentuple_812,& Cyc__gentuple_813,& Cyc__gentuple_814};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_784={4,sizeof(struct _tuple59),{(
void*)((struct _tuple6**)Cyc__genarr_815),(void*)((struct _tuple6**)Cyc__genarr_815),(
void*)((struct _tuple6**)Cyc__genarr_815 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_773;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_incrementor_t_rep;
static unsigned char _tmp346[7]="PreInc";static struct _tuple7 Cyc__gentuple_774={0,{
_tmp346,_tmp346,_tmp346 + 7}};static unsigned char _tmp347[8]="PostInc";static
struct _tuple7 Cyc__gentuple_775={1,{_tmp347,_tmp347,_tmp347 + 8}};static
unsigned char _tmp348[7]="PreDec";static struct _tuple7 Cyc__gentuple_776={2,{
_tmp348,_tmp348,_tmp348 + 7}};static unsigned char _tmp349[8]="PostDec";static
struct _tuple7 Cyc__gentuple_777={3,{_tmp349,_tmp349,_tmp349 + 8}};static struct
_tuple7*Cyc__genarr_778[4]={& Cyc__gentuple_774,& Cyc__gentuple_775,& Cyc__gentuple_776,&
Cyc__gentuple_777};static struct _tuple5*Cyc__genarr_779[0]={};static unsigned char
_tmp34B[12]="Incrementor";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_incrementor_t_rep={
5,{_tmp34B,_tmp34B,_tmp34B + 12},{(void*)((struct _tuple7**)Cyc__genarr_778),(void*)((
struct _tuple7**)Cyc__genarr_778),(void*)((struct _tuple7**)Cyc__genarr_778 + 4)},{(
void*)((struct _tuple5**)Cyc__genarr_779),(void*)((struct _tuple5**)Cyc__genarr_779),(
void*)((struct _tuple5**)Cyc__genarr_779 + 0)}};struct _tuple60{unsigned int f1;
struct Cyc_Absyn_Exp*f2;void*f3;};static struct _tuple6 Cyc__gentuple_780={offsetof(
struct _tuple60,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_781={
offsetof(struct _tuple60,f2),(void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_782={
offsetof(struct _tuple60,f3),(void*)& Cyc_Absyn_incrementor_t_rep};static struct
_tuple6*Cyc__genarr_783[3]={& Cyc__gentuple_780,& Cyc__gentuple_781,& Cyc__gentuple_782};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_773={4,sizeof(struct _tuple60),{(
void*)((struct _tuple6**)Cyc__genarr_783),(void*)((struct _tuple6**)Cyc__genarr_783),(
void*)((struct _tuple6**)Cyc__genarr_783 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_767;struct _tuple61{unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*
f3;struct Cyc_Absyn_Exp*f4;};static struct _tuple6 Cyc__gentuple_768={offsetof(
struct _tuple61,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_769={
offsetof(struct _tuple61,f2),(void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_770={
offsetof(struct _tuple61,f3),(void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_771={
offsetof(struct _tuple61,f4),(void*)& Cyc__genrep_77};static struct _tuple6*Cyc__genarr_772[
4]={& Cyc__gentuple_768,& Cyc__gentuple_769,& Cyc__gentuple_770,& Cyc__gentuple_771};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_767={4,sizeof(struct _tuple61),{(
void*)((struct _tuple6**)Cyc__genarr_772),(void*)((struct _tuple6**)Cyc__genarr_772),(
void*)((struct _tuple6**)Cyc__genarr_772 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_706;struct _tuple62{unsigned int f1;struct Cyc_Absyn_Exp*f2;struct Cyc_Absyn_Exp*
f3;};static struct _tuple6 Cyc__gentuple_707={offsetof(struct _tuple62,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_708={offsetof(struct _tuple62,f2),(
void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_709={offsetof(struct
_tuple62,f3),(void*)& Cyc__genrep_77};static struct _tuple6*Cyc__genarr_710[3]={&
Cyc__gentuple_707,& Cyc__gentuple_708,& Cyc__gentuple_709};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_706={4,sizeof(struct _tuple62),{(void*)((struct _tuple6**)Cyc__genarr_710),(
void*)((struct _tuple6**)Cyc__genarr_710),(void*)((struct _tuple6**)Cyc__genarr_710
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_762;static struct _tuple6
Cyc__gentuple_763={offsetof(struct _tuple49,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_764={offsetof(struct _tuple49,f2),(void*)& Cyc__genrep_77};
static struct _tuple6 Cyc__gentuple_765={offsetof(struct _tuple49,f3),(void*)& Cyc__genrep_471};
static struct _tuple6*Cyc__genarr_766[3]={& Cyc__gentuple_763,& Cyc__gentuple_764,&
Cyc__gentuple_765};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_762={4,
sizeof(struct _tuple49),{(void*)((struct _tuple6**)Cyc__genarr_766),(void*)((
struct _tuple6**)Cyc__genarr_766),(void*)((struct _tuple6**)Cyc__genarr_766 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_749;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_750;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_vararg_call_info_t_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_751;static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_751={1,1,(void*)((void*)& Cyc_Absyn_vararg_info_t_rep)};static
unsigned char _tmp351[15]="VarargCallInfo";static struct _tagged_arr Cyc__genname_756={
_tmp351,_tmp351,_tmp351 + 15};static unsigned char _tmp352[12]="num_varargs";static
struct _tuple5 Cyc__gentuple_752={offsetof(struct Cyc_Absyn_VarargCallInfo,num_varargs),{
_tmp352,_tmp352,_tmp352 + 12},(void*)((void*)& Cyc__genrep_102)};static
unsigned char _tmp353[10]="injectors";static struct _tuple5 Cyc__gentuple_753={
offsetof(struct Cyc_Absyn_VarargCallInfo,injectors),{_tmp353,_tmp353,_tmp353 + 10},(
void*)& Cyc__genrep_284};static unsigned char _tmp354[4]="vai";static struct _tuple5
Cyc__gentuple_754={offsetof(struct Cyc_Absyn_VarargCallInfo,vai),{_tmp354,_tmp354,
_tmp354 + 4},(void*)& Cyc__genrep_751};static struct _tuple5*Cyc__genarr_755[3]={&
Cyc__gentuple_752,& Cyc__gentuple_753,& Cyc__gentuple_754};struct Cyc_Typerep_Struct_struct
Cyc_Absyn_vararg_call_info_t_rep={3,(struct _tagged_arr*)& Cyc__genname_756,
sizeof(struct Cyc_Absyn_VarargCallInfo),{(void*)((struct _tuple5**)Cyc__genarr_755),(
void*)((struct _tuple5**)Cyc__genarr_755),(void*)((struct _tuple5**)Cyc__genarr_755
+ 3)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_750={1,1,(void*)((void*)&
Cyc_Absyn_vararg_call_info_t_rep)};struct _tuple63{unsigned int f1;struct Cyc_Absyn_Exp*
f2;struct Cyc_List_List*f3;struct Cyc_Absyn_VarargCallInfo*f4;};static struct
_tuple6 Cyc__gentuple_757={offsetof(struct _tuple63,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_758={offsetof(struct _tuple63,f2),(void*)& Cyc__genrep_77};
static struct _tuple6 Cyc__gentuple_759={offsetof(struct _tuple63,f3),(void*)& Cyc__genrep_471};
static struct _tuple6 Cyc__gentuple_760={offsetof(struct _tuple63,f4),(void*)& Cyc__genrep_750};
static struct _tuple6*Cyc__genarr_761[4]={& Cyc__gentuple_757,& Cyc__gentuple_758,&
Cyc__gentuple_759,& Cyc__gentuple_760};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_749={
4,sizeof(struct _tuple63),{(void*)((struct _tuple6**)Cyc__genarr_761),(void*)((
struct _tuple6**)Cyc__genarr_761),(void*)((struct _tuple6**)Cyc__genarr_761 + 4)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_744;static struct _tuple6 Cyc__gentuple_745={
offsetof(struct _tuple49,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_746={
offsetof(struct _tuple49,f2),(void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_747={
offsetof(struct _tuple49,f3),(void*)& Cyc__genrep_52};static struct _tuple6*Cyc__genarr_748[
3]={& Cyc__gentuple_745,& Cyc__gentuple_746,& Cyc__gentuple_747};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_744={4,sizeof(struct _tuple49),{(void*)((struct _tuple6**)Cyc__genarr_748),(
void*)((struct _tuple6**)Cyc__genarr_748),(void*)((struct _tuple6**)Cyc__genarr_748
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_739;struct _tuple64{
unsigned int f1;void*f2;struct Cyc_Absyn_Exp*f3;};static struct _tuple6 Cyc__gentuple_740={
offsetof(struct _tuple64,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_741={
offsetof(struct _tuple64,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6 Cyc__gentuple_742={offsetof(struct _tuple64,f3),(void*)& Cyc__genrep_77};
static struct _tuple6*Cyc__genarr_743[3]={& Cyc__gentuple_740,& Cyc__gentuple_741,&
Cyc__gentuple_742};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_739={4,
sizeof(struct _tuple64),{(void*)((struct _tuple6**)Cyc__genarr_743),(void*)((
struct _tuple6**)Cyc__genarr_743),(void*)((struct _tuple6**)Cyc__genarr_743 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_734;static struct _tuple6 Cyc__gentuple_735={
offsetof(struct _tuple62,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_736={
offsetof(struct _tuple62,f2),(void*)& Cyc__genrep_73};static struct _tuple6 Cyc__gentuple_737={
offsetof(struct _tuple62,f3),(void*)& Cyc__genrep_77};static struct _tuple6*Cyc__genarr_738[
3]={& Cyc__gentuple_735,& Cyc__gentuple_736,& Cyc__gentuple_737};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_734={4,sizeof(struct _tuple62),{(void*)((struct _tuple6**)Cyc__genarr_738),(
void*)((struct _tuple6**)Cyc__genarr_738),(void*)((struct _tuple6**)Cyc__genarr_738
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_47;static struct _tuple6 Cyc__gentuple_48={
offsetof(struct _tuple6,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_49={
offsetof(struct _tuple6,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6*Cyc__genarr_50[2]={& Cyc__gentuple_48,& Cyc__gentuple_49};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_47={4,sizeof(struct _tuple6),{(void*)((struct _tuple6**)Cyc__genarr_50),(
void*)((struct _tuple6**)Cyc__genarr_50),(void*)((struct _tuple6**)Cyc__genarr_50 + 
2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_721;extern struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_offsetof_field_t_rep;extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_723;
struct _tuple65{unsigned int f1;unsigned int f2;};static struct _tuple6 Cyc__gentuple_724={
offsetof(struct _tuple65,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_725={
offsetof(struct _tuple65,f2),(void*)& Cyc__genrep_5};static struct _tuple6*Cyc__genarr_726[
2]={& Cyc__gentuple_724,& Cyc__gentuple_725};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_723={4,sizeof(struct _tuple65),{(void*)((struct _tuple6**)Cyc__genarr_726),(
void*)((struct _tuple6**)Cyc__genarr_726),(void*)((struct _tuple6**)Cyc__genarr_726
+ 2)}};static struct _tuple7*Cyc__genarr_722[0]={};static unsigned char _tmp35D[12]="StructField";
static struct _tuple5 Cyc__gentuple_727={0,{_tmp35D,_tmp35D,_tmp35D + 12},(void*)&
Cyc__genrep_315};static unsigned char _tmp35E[11]="TupleIndex";static struct _tuple5
Cyc__gentuple_728={1,{_tmp35E,_tmp35E,_tmp35E + 11},(void*)& Cyc__genrep_723};
static struct _tuple5*Cyc__genarr_729[2]={& Cyc__gentuple_727,& Cyc__gentuple_728};
static unsigned char _tmp360[14]="OffsetofField";struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_offsetof_field_t_rep={5,{_tmp360,_tmp360,_tmp360 + 14},{(void*)((struct
_tuple7**)Cyc__genarr_722),(void*)((struct _tuple7**)Cyc__genarr_722),(void*)((
struct _tuple7**)Cyc__genarr_722 + 0)},{(void*)((struct _tuple5**)Cyc__genarr_729),(
void*)((struct _tuple5**)Cyc__genarr_729),(void*)((struct _tuple5**)Cyc__genarr_729
+ 2)}};struct _tuple66{unsigned int f1;void*f2;void*f3;};static struct _tuple6 Cyc__gentuple_730={
offsetof(struct _tuple66,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_731={
offsetof(struct _tuple66,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6 Cyc__gentuple_732={offsetof(struct _tuple66,f3),(void*)& Cyc_Absyn_offsetof_field_t_rep};
static struct _tuple6*Cyc__genarr_733[3]={& Cyc__gentuple_730,& Cyc__gentuple_731,&
Cyc__gentuple_732};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_721={4,
sizeof(struct _tuple66),{(void*)((struct _tuple6**)Cyc__genarr_733),(void*)((
struct _tuple6**)Cyc__genarr_733),(void*)((struct _tuple6**)Cyc__genarr_733 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_716;struct _tuple67{unsigned int
f1;struct Cyc_List_List*f2;void*f3;};static struct _tuple6 Cyc__gentuple_717={
offsetof(struct _tuple67,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_718={
offsetof(struct _tuple67,f2),(void*)& Cyc__genrep_292};static struct _tuple6 Cyc__gentuple_719={
offsetof(struct _tuple67,f3),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6*Cyc__genarr_720[3]={& Cyc__gentuple_717,& Cyc__gentuple_718,& Cyc__gentuple_719};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_716={4,sizeof(struct _tuple67),{(
void*)((struct _tuple6**)Cyc__genarr_720),(void*)((struct _tuple6**)Cyc__genarr_720),(
void*)((struct _tuple6**)Cyc__genarr_720 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_711;struct _tuple68{unsigned int f1;struct Cyc_Absyn_Exp*f2;struct
_tagged_arr*f3;};static struct _tuple6 Cyc__gentuple_712={offsetof(struct _tuple68,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_713={offsetof(struct
_tuple68,f2),(void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_714={
offsetof(struct _tuple68,f3),(void*)& Cyc__genrep_12};static struct _tuple6*Cyc__genarr_715[
3]={& Cyc__gentuple_712,& Cyc__gentuple_713,& Cyc__gentuple_714};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_711={4,sizeof(struct _tuple68),{(void*)((struct _tuple6**)Cyc__genarr_715),(
void*)((struct _tuple6**)Cyc__genarr_715),(void*)((struct _tuple6**)Cyc__genarr_715
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_702;static struct _tuple6
Cyc__gentuple_703={offsetof(struct _tuple11,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_704={offsetof(struct _tuple11,f2),(void*)& Cyc__genrep_471};
static struct _tuple6*Cyc__genarr_705[2]={& Cyc__gentuple_703,& Cyc__gentuple_704};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_702={4,sizeof(struct _tuple11),{(
void*)((struct _tuple6**)Cyc__genarr_705),(void*)((struct _tuple6**)Cyc__genarr_705),(
void*)((struct _tuple6**)Cyc__genarr_705 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_691;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_692;extern
struct Cyc_Typerep_Tuple_struct Cyc__genrep_693;static struct _tuple6 Cyc__gentuple_694={
offsetof(struct _tuple1,f1),(void*)& Cyc__genrep_577};static struct _tuple6 Cyc__gentuple_695={
offsetof(struct _tuple1,f2),(void*)& Cyc__genrep_132};static struct _tuple6 Cyc__gentuple_696={
offsetof(struct _tuple1,f3),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6*Cyc__genarr_697[3]={& Cyc__gentuple_694,& Cyc__gentuple_695,& Cyc__gentuple_696};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_693={4,sizeof(struct _tuple1),{(
void*)((struct _tuple6**)Cyc__genarr_697),(void*)((struct _tuple6**)Cyc__genarr_697),(
void*)((struct _tuple6**)Cyc__genarr_697 + 3)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_692={1,1,(void*)((void*)& Cyc__genrep_693)};extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_618;extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_exp_t1_446H2_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_619;extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_620;static struct _tuple6 Cyc__gentuple_621={offsetof(struct _tuple10,f1),(
void*)& Cyc__genrep_313};static struct _tuple6 Cyc__gentuple_622={offsetof(struct
_tuple10,f2),(void*)& Cyc__genrep_77};static struct _tuple6*Cyc__genarr_623[2]={&
Cyc__gentuple_621,& Cyc__gentuple_622};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_620={
4,sizeof(struct _tuple10),{(void*)((struct _tuple6**)Cyc__genarr_623),(void*)((
struct _tuple6**)Cyc__genarr_623),(void*)((struct _tuple6**)Cyc__genarr_623 + 2)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_619={1,1,(void*)((void*)& Cyc__genrep_620)};
static unsigned char _tmp369[5]="List";static struct _tagged_arr Cyc__genname_627={
_tmp369,_tmp369,_tmp369 + 5};static unsigned char _tmp36A[3]="hd";static struct
_tuple5 Cyc__gentuple_624={offsetof(struct Cyc_List_List,hd),{_tmp36A,_tmp36A,
_tmp36A + 3},(void*)& Cyc__genrep_619};static unsigned char _tmp36B[3]="tl";static
struct _tuple5 Cyc__gentuple_625={offsetof(struct Cyc_List_List,tl),{_tmp36B,
_tmp36B,_tmp36B + 3},(void*)& Cyc__genrep_618};static struct _tuple5*Cyc__genarr_626[
2]={& Cyc__gentuple_624,& Cyc__gentuple_625};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_exp_t1_446H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_627,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_626),(void*)((struct _tuple5**)Cyc__genarr_626),(void*)((
struct _tuple5**)Cyc__genarr_626 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_618={
1,1,(void*)((void*)& Cyc_struct_List_List060List_list_t0Absyn_designator_t46H24Absyn_exp_t1_446H2_rep)};
struct _tuple69{unsigned int f1;struct _tuple1*f2;struct Cyc_List_List*f3;};static
struct _tuple6 Cyc__gentuple_698={offsetof(struct _tuple69,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_699={offsetof(struct _tuple69,f2),(void*)& Cyc__genrep_692};
static struct _tuple6 Cyc__gentuple_700={offsetof(struct _tuple69,f3),(void*)& Cyc__genrep_618};
static struct _tuple6*Cyc__genarr_701[3]={& Cyc__gentuple_698,& Cyc__gentuple_699,&
Cyc__gentuple_700};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_691={4,
sizeof(struct _tuple69),{(void*)((struct _tuple6**)Cyc__genarr_701),(void*)((
struct _tuple6**)Cyc__genarr_701),(void*)((struct _tuple6**)Cyc__genarr_701 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_687;static struct _tuple6 Cyc__gentuple_688={
offsetof(struct _tuple11,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_689={
offsetof(struct _tuple11,f2),(void*)& Cyc__genrep_618};static struct _tuple6*Cyc__genarr_690[
2]={& Cyc__gentuple_688,& Cyc__gentuple_689};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_687={4,sizeof(struct _tuple11),{(void*)((struct _tuple6**)Cyc__genarr_690),(
void*)((struct _tuple6**)Cyc__genarr_690),(void*)((struct _tuple6**)Cyc__genarr_690
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_681;struct _tuple70{
unsigned int f1;struct Cyc_Absyn_Vardecl*f2;struct Cyc_Absyn_Exp*f3;struct Cyc_Absyn_Exp*
f4;};static struct _tuple6 Cyc__gentuple_682={offsetof(struct _tuple70,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_683={offsetof(struct _tuple70,f2),(
void*)& Cyc__genrep_131};static struct _tuple6 Cyc__gentuple_684={offsetof(struct
_tuple70,f3),(void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_685={
offsetof(struct _tuple70,f4),(void*)& Cyc__genrep_77};static struct _tuple6*Cyc__genarr_686[
4]={& Cyc__gentuple_682,& Cyc__gentuple_683,& Cyc__gentuple_684,& Cyc__gentuple_685};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_681={4,sizeof(struct _tuple70),{(
void*)((struct _tuple6**)Cyc__genarr_686),(void*)((struct _tuple6**)Cyc__genarr_686),(
void*)((struct _tuple6**)Cyc__genarr_686 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_673;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_674;static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_674={1,1,(void*)((void*)& Cyc_struct_Absyn_Aggrdecl_rep)};
struct _tuple71{unsigned int f1;struct _tuple0*f2;struct Cyc_List_List*f3;struct Cyc_List_List*
f4;struct Cyc_Absyn_Aggrdecl*f5;};static struct _tuple6 Cyc__gentuple_675={offsetof(
struct _tuple71,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_676={
offsetof(struct _tuple71,f2),(void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_677={
offsetof(struct _tuple71,f3),(void*)& Cyc__genrep_52};static struct _tuple6 Cyc__gentuple_678={
offsetof(struct _tuple71,f4),(void*)& Cyc__genrep_618};static struct _tuple6 Cyc__gentuple_679={
offsetof(struct _tuple71,f5),(void*)& Cyc__genrep_674};static struct _tuple6*Cyc__genarr_680[
5]={& Cyc__gentuple_675,& Cyc__gentuple_676,& Cyc__gentuple_677,& Cyc__gentuple_678,&
Cyc__gentuple_679};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_673={4,
sizeof(struct _tuple71),{(void*)((struct _tuple6**)Cyc__genarr_680),(void*)((
struct _tuple6**)Cyc__genarr_680),(void*)((struct _tuple6**)Cyc__genarr_680 + 5)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_668;static struct _tuple6 Cyc__gentuple_669={
offsetof(struct _tuple58,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_670={
offsetof(struct _tuple58,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6 Cyc__gentuple_671={offsetof(struct _tuple58,f3),(void*)& Cyc__genrep_618};
static struct _tuple6*Cyc__genarr_672[3]={& Cyc__gentuple_669,& Cyc__gentuple_670,&
Cyc__gentuple_671};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_668={4,
sizeof(struct _tuple58),{(void*)((struct _tuple6**)Cyc__genarr_672),(void*)((
struct _tuple6**)Cyc__genarr_672),(void*)((struct _tuple6**)Cyc__genarr_672 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_662;struct _tuple72{unsigned int
f1;struct Cyc_List_List*f2;struct Cyc_Absyn_Tuniondecl*f3;struct Cyc_Absyn_Tunionfield*
f4;};static struct _tuple6 Cyc__gentuple_663={offsetof(struct _tuple72,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_664={offsetof(struct _tuple72,f2),(
void*)& Cyc__genrep_471};static struct _tuple6 Cyc__gentuple_665={offsetof(struct
_tuple72,f3),(void*)((void*)& Cyc__genrep_282)};static struct _tuple6 Cyc__gentuple_666={
offsetof(struct _tuple72,f4),(void*)& Cyc__genrep_265};static struct _tuple6*Cyc__genarr_667[
4]={& Cyc__gentuple_663,& Cyc__gentuple_664,& Cyc__gentuple_665,& Cyc__gentuple_666};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_662={4,sizeof(struct _tuple72),{(
void*)((struct _tuple6**)Cyc__genarr_667),(void*)((struct _tuple6**)Cyc__genarr_667),(
void*)((struct _tuple6**)Cyc__genarr_667 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_655;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_656;static
struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_656={1,1,(void*)((void*)& Cyc_struct_Absyn_Enumdecl_rep)};
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_649;static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_649={1,1,(void*)((void*)& Cyc_struct_Absyn_Enumfield_rep)};struct
_tuple73{unsigned int f1;struct _tuple0*f2;struct Cyc_Absyn_Enumdecl*f3;struct Cyc_Absyn_Enumfield*
f4;};static struct _tuple6 Cyc__gentuple_657={offsetof(struct _tuple73,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_658={offsetof(struct _tuple73,f2),(
void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_659={offsetof(struct
_tuple73,f3),(void*)& Cyc__genrep_656};static struct _tuple6 Cyc__gentuple_660={
offsetof(struct _tuple73,f4),(void*)& Cyc__genrep_649};static struct _tuple6*Cyc__genarr_661[
4]={& Cyc__gentuple_657,& Cyc__gentuple_658,& Cyc__gentuple_659,& Cyc__gentuple_660};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_655={4,sizeof(struct _tuple73),{(
void*)((struct _tuple6**)Cyc__genarr_661),(void*)((struct _tuple6**)Cyc__genarr_661),(
void*)((struct _tuple6**)Cyc__genarr_661 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_648;struct _tuple74{unsigned int f1;struct _tuple0*f2;void*f3;struct Cyc_Absyn_Enumfield*
f4;};static struct _tuple6 Cyc__gentuple_650={offsetof(struct _tuple74,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_651={offsetof(struct _tuple74,f2),(
void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_652={offsetof(struct
_tuple74,f3),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct _tuple6 Cyc__gentuple_653={
offsetof(struct _tuple74,f4),(void*)& Cyc__genrep_649};static struct _tuple6*Cyc__genarr_654[
4]={& Cyc__gentuple_650,& Cyc__gentuple_651,& Cyc__gentuple_652,& Cyc__gentuple_653};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_648={4,sizeof(struct _tuple74),{(
void*)((struct _tuple6**)Cyc__genarr_654),(void*)((struct _tuple6**)Cyc__genarr_654),(
void*)((struct _tuple6**)Cyc__genarr_654 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_636;extern struct Cyc_Typerep_Struct_struct Cyc_Absyn_malloc_info_t_rep;
extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_637;static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_637={1,1,(void*)((void*)& Cyc_Absyn_type_t_rep)};static unsigned char
_tmp37A[11]="MallocInfo";static struct _tagged_arr Cyc__genname_644={_tmp37A,
_tmp37A,_tmp37A + 11};static unsigned char _tmp37B[10]="is_calloc";static struct
_tuple5 Cyc__gentuple_638={offsetof(struct Cyc_Absyn_MallocInfo,is_calloc),{
_tmp37B,_tmp37B,_tmp37B + 10},(void*)((void*)& Cyc__genrep_102)};static
unsigned char _tmp37C[4]="rgn";static struct _tuple5 Cyc__gentuple_639={offsetof(
struct Cyc_Absyn_MallocInfo,rgn),{_tmp37C,_tmp37C,_tmp37C + 4},(void*)& Cyc__genrep_73};
static unsigned char _tmp37D[9]="elt_type";static struct _tuple5 Cyc__gentuple_640={
offsetof(struct Cyc_Absyn_MallocInfo,elt_type),{_tmp37D,_tmp37D,_tmp37D + 9},(void*)&
Cyc__genrep_637};static unsigned char _tmp37E[9]="num_elts";static struct _tuple5 Cyc__gentuple_641={
offsetof(struct Cyc_Absyn_MallocInfo,num_elts),{_tmp37E,_tmp37E,_tmp37E + 9},(void*)&
Cyc__genrep_77};static unsigned char _tmp37F[11]="fat_result";static struct _tuple5
Cyc__gentuple_642={offsetof(struct Cyc_Absyn_MallocInfo,fat_result),{_tmp37F,
_tmp37F,_tmp37F + 11},(void*)((void*)& Cyc__genrep_102)};static struct _tuple5*Cyc__genarr_643[
5]={& Cyc__gentuple_638,& Cyc__gentuple_639,& Cyc__gentuple_640,& Cyc__gentuple_641,&
Cyc__gentuple_642};struct Cyc_Typerep_Struct_struct Cyc_Absyn_malloc_info_t_rep={3,(
struct _tagged_arr*)& Cyc__genname_644,sizeof(struct Cyc_Absyn_MallocInfo),{(void*)((
struct _tuple5**)Cyc__genarr_643),(void*)((struct _tuple5**)Cyc__genarr_643),(void*)((
struct _tuple5**)Cyc__genarr_643 + 5)}};struct _tuple75{unsigned int f1;struct Cyc_Absyn_MallocInfo
f2;};static struct _tuple6 Cyc__gentuple_645={offsetof(struct _tuple75,f1),(void*)&
Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_646={offsetof(struct _tuple75,f2),(
void*)& Cyc_Absyn_malloc_info_t_rep};static struct _tuple6*Cyc__genarr_647[2]={& Cyc__gentuple_645,&
Cyc__gentuple_646};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_636={4,
sizeof(struct _tuple75),{(void*)((struct _tuple6**)Cyc__genarr_647),(void*)((
struct _tuple6**)Cyc__genarr_647),(void*)((struct _tuple6**)Cyc__genarr_647 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_617;extern struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_628;extern struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_typedef_name_t2_rep;
static unsigned char _tmp382[4]="Opt";static struct _tagged_arr Cyc__genname_631={
_tmp382,_tmp382,_tmp382 + 4};static unsigned char _tmp383[2]="v";static struct
_tuple5 Cyc__gentuple_629={offsetof(struct Cyc_Core_Opt,v),{_tmp383,_tmp383,
_tmp383 + 2},(void*)& Cyc__genrep_10};static struct _tuple5*Cyc__genarr_630[1]={& Cyc__gentuple_629};
struct Cyc_Typerep_Struct_struct Cyc_struct_Core_Opt0Absyn_typedef_name_t2_rep={3,(
struct _tagged_arr*)& Cyc__genname_631,sizeof(struct Cyc_Core_Opt),{(void*)((struct
_tuple5**)Cyc__genarr_630),(void*)((struct _tuple5**)Cyc__genarr_630),(void*)((
struct _tuple5**)Cyc__genarr_630 + 1)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_628={
1,1,(void*)((void*)& Cyc_struct_Core_Opt0Absyn_typedef_name_t2_rep)};struct
_tuple76{unsigned int f1;struct Cyc_Core_Opt*f2;struct Cyc_List_List*f3;};static
struct _tuple6 Cyc__gentuple_632={offsetof(struct _tuple76,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_633={offsetof(struct _tuple76,f2),(void*)& Cyc__genrep_628};
static struct _tuple6 Cyc__gentuple_634={offsetof(struct _tuple76,f3),(void*)& Cyc__genrep_618};
static struct _tuple6*Cyc__genarr_635[3]={& Cyc__gentuple_632,& Cyc__gentuple_633,&
Cyc__gentuple_634};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_617={4,
sizeof(struct _tuple76),{(void*)((struct _tuple6**)Cyc__genarr_635),(void*)((
struct _tuple6**)Cyc__genarr_635),(void*)((struct _tuple6**)Cyc__genarr_635 + 3)}};
static struct _tuple7*Cyc__genarr_75[0]={};static unsigned char _tmp387[8]="Const_e";
static struct _tuple5 Cyc__gentuple_864={0,{_tmp387,_tmp387,_tmp387 + 8},(void*)& Cyc__genrep_834};
static unsigned char _tmp388[6]="Var_e";static struct _tuple5 Cyc__gentuple_865={1,{
_tmp388,_tmp388,_tmp388 + 6},(void*)& Cyc__genrep_821};static unsigned char _tmp389[
12]="UnknownId_e";static struct _tuple5 Cyc__gentuple_866={2,{_tmp389,_tmp389,
_tmp389 + 12},(void*)& Cyc__genrep_240};static unsigned char _tmp38A[9]="Primop_e";
static struct _tuple5 Cyc__gentuple_867={3,{_tmp38A,_tmp38A,_tmp38A + 9},(void*)& Cyc__genrep_816};
static unsigned char _tmp38B[11]="AssignOp_e";static struct _tuple5 Cyc__gentuple_868={
4,{_tmp38B,_tmp38B,_tmp38B + 11},(void*)& Cyc__genrep_784};static unsigned char
_tmp38C[12]="Increment_e";static struct _tuple5 Cyc__gentuple_869={5,{_tmp38C,
_tmp38C,_tmp38C + 12},(void*)& Cyc__genrep_773};static unsigned char _tmp38D[14]="Conditional_e";
static struct _tuple5 Cyc__gentuple_870={6,{_tmp38D,_tmp38D,_tmp38D + 14},(void*)&
Cyc__genrep_767};static unsigned char _tmp38E[9]="SeqExp_e";static struct _tuple5 Cyc__gentuple_871={
7,{_tmp38E,_tmp38E,_tmp38E + 9},(void*)& Cyc__genrep_706};static unsigned char
_tmp38F[14]="UnknownCall_e";static struct _tuple5 Cyc__gentuple_872={8,{_tmp38F,
_tmp38F,_tmp38F + 14},(void*)& Cyc__genrep_762};static unsigned char _tmp390[9]="FnCall_e";
static struct _tuple5 Cyc__gentuple_873={9,{_tmp390,_tmp390,_tmp390 + 9},(void*)& Cyc__genrep_749};
static unsigned char _tmp391[8]="Throw_e";static struct _tuple5 Cyc__gentuple_874={10,{
_tmp391,_tmp391,_tmp391 + 8},(void*)& Cyc__genrep_76};static unsigned char _tmp392[
16]="NoInstantiate_e";static struct _tuple5 Cyc__gentuple_875={11,{_tmp392,_tmp392,
_tmp392 + 16},(void*)& Cyc__genrep_76};static unsigned char _tmp393[14]="Instantiate_e";
static struct _tuple5 Cyc__gentuple_876={12,{_tmp393,_tmp393,_tmp393 + 14},(void*)&
Cyc__genrep_744};static unsigned char _tmp394[7]="Cast_e";static struct _tuple5 Cyc__gentuple_877={
13,{_tmp394,_tmp394,_tmp394 + 7},(void*)& Cyc__genrep_739};static unsigned char
_tmp395[10]="Address_e";static struct _tuple5 Cyc__gentuple_878={14,{_tmp395,
_tmp395,_tmp395 + 10},(void*)& Cyc__genrep_76};static unsigned char _tmp396[6]="New_e";
static struct _tuple5 Cyc__gentuple_879={15,{_tmp396,_tmp396,_tmp396 + 6},(void*)&
Cyc__genrep_734};static unsigned char _tmp397[12]="Sizeoftyp_e";static struct
_tuple5 Cyc__gentuple_880={16,{_tmp397,_tmp397,_tmp397 + 12},(void*)& Cyc__genrep_47};
static unsigned char _tmp398[12]="Sizeofexp_e";static struct _tuple5 Cyc__gentuple_881={
17,{_tmp398,_tmp398,_tmp398 + 12},(void*)& Cyc__genrep_76};static unsigned char
_tmp399[11]="Offsetof_e";static struct _tuple5 Cyc__gentuple_882={18,{_tmp399,
_tmp399,_tmp399 + 11},(void*)& Cyc__genrep_721};static unsigned char _tmp39A[9]="Gentyp_e";
static struct _tuple5 Cyc__gentuple_883={19,{_tmp39A,_tmp39A,_tmp39A + 9},(void*)&
Cyc__genrep_716};static unsigned char _tmp39B[8]="Deref_e";static struct _tuple5 Cyc__gentuple_884={
20,{_tmp39B,_tmp39B,_tmp39B + 8},(void*)& Cyc__genrep_76};static unsigned char
_tmp39C[13]="AggrMember_e";static struct _tuple5 Cyc__gentuple_885={21,{_tmp39C,
_tmp39C,_tmp39C + 13},(void*)& Cyc__genrep_711};static unsigned char _tmp39D[12]="AggrArrow_e";
static struct _tuple5 Cyc__gentuple_886={22,{_tmp39D,_tmp39D,_tmp39D + 12},(void*)&
Cyc__genrep_711};static unsigned char _tmp39E[12]="Subscript_e";static struct
_tuple5 Cyc__gentuple_887={23,{_tmp39E,_tmp39E,_tmp39E + 12},(void*)& Cyc__genrep_706};
static unsigned char _tmp39F[8]="Tuple_e";static struct _tuple5 Cyc__gentuple_888={24,{
_tmp39F,_tmp39F,_tmp39F + 8},(void*)& Cyc__genrep_702};static unsigned char _tmp3A0[
14]="CompoundLit_e";static struct _tuple5 Cyc__gentuple_889={25,{_tmp3A0,_tmp3A0,
_tmp3A0 + 14},(void*)& Cyc__genrep_691};static unsigned char _tmp3A1[8]="Array_e";
static struct _tuple5 Cyc__gentuple_890={26,{_tmp3A1,_tmp3A1,_tmp3A1 + 8},(void*)&
Cyc__genrep_687};static unsigned char _tmp3A2[16]="Comprehension_e";static struct
_tuple5 Cyc__gentuple_891={27,{_tmp3A2,_tmp3A2,_tmp3A2 + 16},(void*)& Cyc__genrep_681};
static unsigned char _tmp3A3[9]="Struct_e";static struct _tuple5 Cyc__gentuple_892={
28,{_tmp3A3,_tmp3A3,_tmp3A3 + 9},(void*)& Cyc__genrep_673};static unsigned char
_tmp3A4[13]="AnonStruct_e";static struct _tuple5 Cyc__gentuple_893={29,{_tmp3A4,
_tmp3A4,_tmp3A4 + 13},(void*)& Cyc__genrep_668};static unsigned char _tmp3A5[9]="Tunion_e";
static struct _tuple5 Cyc__gentuple_894={30,{_tmp3A5,_tmp3A5,_tmp3A5 + 9},(void*)&
Cyc__genrep_662};static unsigned char _tmp3A6[7]="Enum_e";static struct _tuple5 Cyc__gentuple_895={
31,{_tmp3A6,_tmp3A6,_tmp3A6 + 7},(void*)& Cyc__genrep_655};static unsigned char
_tmp3A7[11]="AnonEnum_e";static struct _tuple5 Cyc__gentuple_896={32,{_tmp3A7,
_tmp3A7,_tmp3A7 + 11},(void*)& Cyc__genrep_648};static unsigned char _tmp3A8[9]="Malloc_e";
static struct _tuple5 Cyc__gentuple_897={33,{_tmp3A8,_tmp3A8,_tmp3A8 + 9},(void*)&
Cyc__genrep_636};static unsigned char _tmp3A9[16]="UnresolvedMem_e";static struct
_tuple5 Cyc__gentuple_898={34,{_tmp3A9,_tmp3A9,_tmp3A9 + 16},(void*)& Cyc__genrep_617};
static unsigned char _tmp3AA[10]="StmtExp_e";static struct _tuple5 Cyc__gentuple_899={
35,{_tmp3AA,_tmp3AA,_tmp3AA + 10},(void*)& Cyc__genrep_460};static unsigned char
_tmp3AB[10]="Codegen_e";static struct _tuple5 Cyc__gentuple_900={36,{_tmp3AB,
_tmp3AB,_tmp3AB + 10},(void*)& Cyc__genrep_81};static unsigned char _tmp3AC[7]="Fill_e";
static struct _tuple5 Cyc__gentuple_901={37,{_tmp3AC,_tmp3AC,_tmp3AC + 7},(void*)&
Cyc__genrep_76};static struct _tuple5*Cyc__genarr_902[38]={& Cyc__gentuple_864,& Cyc__gentuple_865,&
Cyc__gentuple_866,& Cyc__gentuple_867,& Cyc__gentuple_868,& Cyc__gentuple_869,& Cyc__gentuple_870,&
Cyc__gentuple_871,& Cyc__gentuple_872,& Cyc__gentuple_873,& Cyc__gentuple_874,& Cyc__gentuple_875,&
Cyc__gentuple_876,& Cyc__gentuple_877,& Cyc__gentuple_878,& Cyc__gentuple_879,& Cyc__gentuple_880,&
Cyc__gentuple_881,& Cyc__gentuple_882,& Cyc__gentuple_883,& Cyc__gentuple_884,& Cyc__gentuple_885,&
Cyc__gentuple_886,& Cyc__gentuple_887,& Cyc__gentuple_888,& Cyc__gentuple_889,& Cyc__gentuple_890,&
Cyc__gentuple_891,& Cyc__gentuple_892,& Cyc__gentuple_893,& Cyc__gentuple_894,& Cyc__gentuple_895,&
Cyc__gentuple_896,& Cyc__gentuple_897,& Cyc__gentuple_898,& Cyc__gentuple_899,& Cyc__gentuple_900,&
Cyc__gentuple_901};static unsigned char _tmp3AE[8]="Raw_exp";struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_raw_exp_t_rep={5,{_tmp3AE,_tmp3AE,_tmp3AE + 8},{(void*)((struct _tuple7**)
Cyc__genarr_75),(void*)((struct _tuple7**)Cyc__genarr_75),(void*)((struct _tuple7**)
Cyc__genarr_75 + 0)},{(void*)((struct _tuple5**)Cyc__genarr_902),(void*)((struct
_tuple5**)Cyc__genarr_902),(void*)((struct _tuple5**)Cyc__genarr_902 + 38)}};
static unsigned char _tmp3AF[4]="Exp";static struct _tagged_arr Cyc__genname_908={
_tmp3AF,_tmp3AF,_tmp3AF + 4};static unsigned char _tmp3B0[5]="topt";static struct
_tuple5 Cyc__gentuple_903={offsetof(struct Cyc_Absyn_Exp,topt),{_tmp3B0,_tmp3B0,
_tmp3B0 + 5},(void*)& Cyc__genrep_61};static unsigned char _tmp3B1[2]="r";static
struct _tuple5 Cyc__gentuple_904={offsetof(struct Cyc_Absyn_Exp,r),{_tmp3B1,_tmp3B1,
_tmp3B1 + 2},(void*)& Cyc_Absyn_raw_exp_t_rep};static unsigned char _tmp3B2[4]="loc";
static struct _tuple5 Cyc__gentuple_905={offsetof(struct Cyc_Absyn_Exp,loc),{_tmp3B2,
_tmp3B2,_tmp3B2 + 4},(void*)& Cyc__genrep_2};static unsigned char _tmp3B3[6]="annot";
static struct _tuple5 Cyc__gentuple_906={offsetof(struct Cyc_Absyn_Exp,annot),{
_tmp3B3,_tmp3B3,_tmp3B3 + 6},(void*)& Cyc_Absyn_absyn_annot_t_rep};static struct
_tuple5*Cyc__genarr_907[4]={& Cyc__gentuple_903,& Cyc__gentuple_904,& Cyc__gentuple_905,&
Cyc__gentuple_906};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Exp_rep={3,(
struct _tagged_arr*)& Cyc__genname_908,sizeof(struct Cyc_Absyn_Exp),{(void*)((
struct _tuple5**)Cyc__genarr_907),(void*)((struct _tuple5**)Cyc__genarr_907),(void*)((
struct _tuple5**)Cyc__genarr_907 + 4)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_77={
1,1,(void*)((void*)& Cyc_struct_Absyn_Exp_rep)};static struct _tuple6 Cyc__gentuple_78={
offsetof(struct _tuple29,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_79={
offsetof(struct _tuple29,f2),(void*)& Cyc__genrep_77};static struct _tuple6*Cyc__genarr_80[
2]={& Cyc__gentuple_78,& Cyc__gentuple_79};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_76={
4,sizeof(struct _tuple29),{(void*)((struct _tuple6**)Cyc__genarr_80),(void*)((
struct _tuple6**)Cyc__genarr_80),(void*)((struct _tuple6**)Cyc__genarr_80 + 2)}};
static unsigned char _tmp3B7[10]="Unknown_b";static struct _tuple7 Cyc__gentuple_984={
0,{_tmp3B7,_tmp3B7,_tmp3B7 + 10}};static struct _tuple7*Cyc__genarr_985[1]={& Cyc__gentuple_984};
static unsigned char _tmp3B8[8]="Upper_b";static struct _tuple5 Cyc__gentuple_986={0,{
_tmp3B8,_tmp3B8,_tmp3B8 + 8},(void*)& Cyc__genrep_76};static struct _tuple5*Cyc__genarr_987[
1]={& Cyc__gentuple_986};static unsigned char _tmp3BA[7]="Bounds";struct Cyc_Typerep_TUnion_struct
Cyc_Absyn_bounds_t_rep={5,{_tmp3BA,_tmp3BA,_tmp3BA + 7},{(void*)((struct _tuple7**)
Cyc__genarr_985),(void*)((struct _tuple7**)Cyc__genarr_985),(void*)((struct
_tuple7**)Cyc__genarr_985 + 1)},{(void*)((struct _tuple5**)Cyc__genarr_987),(void*)((
struct _tuple5**)Cyc__genarr_987),(void*)((struct _tuple5**)Cyc__genarr_987 + 1)}};
static struct _tuple6 Cyc__gentuple_988={offsetof(struct _tuple6,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_989={offsetof(struct _tuple6,f2),(void*)& Cyc_Absyn_bounds_t_rep};
static struct _tuple6*Cyc__genarr_990[2]={& Cyc__gentuple_988,& Cyc__gentuple_989};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_983={4,sizeof(struct _tuple6),{(
void*)((struct _tuple6**)Cyc__genarr_990),(void*)((struct _tuple6**)Cyc__genarr_990),(
void*)((struct _tuple6**)Cyc__genarr_990 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_979;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_976;extern
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Conref0Absyn_bounds_t2_rep;
static unsigned char _tmp3BC[7]="Conref";static struct _tagged_arr Cyc__genname_996={
_tmp3BC,_tmp3BC,_tmp3BC + 7};static unsigned char _tmp3BD[2]="v";static struct
_tuple5 Cyc__gentuple_994={offsetof(struct Cyc_Absyn_Conref,v),{_tmp3BD,_tmp3BD,
_tmp3BD + 2},(void*)& Cyc_tunion_Absyn_Constraint0Absyn_bounds_t2_rep};static
struct _tuple5*Cyc__genarr_995[1]={& Cyc__gentuple_994};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Conref0Absyn_bounds_t2_rep={3,(struct _tagged_arr*)& Cyc__genname_996,
sizeof(struct Cyc_Absyn_Conref),{(void*)((struct _tuple5**)Cyc__genarr_995),(void*)((
struct _tuple5**)Cyc__genarr_995),(void*)((struct _tuple5**)Cyc__genarr_995 + 1)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_976={1,1,(void*)((void*)& Cyc_struct_Absyn_Conref0Absyn_bounds_t2_rep)};
struct _tuple77{unsigned int f1;struct Cyc_Absyn_Conref*f2;};static struct _tuple6 Cyc__gentuple_980={
offsetof(struct _tuple77,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_981={
offsetof(struct _tuple77,f2),(void*)& Cyc__genrep_976};static struct _tuple6*Cyc__genarr_982[
2]={& Cyc__gentuple_980,& Cyc__gentuple_981};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_979={4,sizeof(struct _tuple77),{(void*)((struct _tuple6**)Cyc__genarr_982),(
void*)((struct _tuple6**)Cyc__genarr_982),(void*)((struct _tuple6**)Cyc__genarr_982
+ 2)}};static unsigned char _tmp3C1[10]="No_constr";static struct _tuple7 Cyc__gentuple_977={
0,{_tmp3C1,_tmp3C1,_tmp3C1 + 10}};static struct _tuple7*Cyc__genarr_978[1]={& Cyc__gentuple_977};
static unsigned char _tmp3C2[10]="Eq_constr";static struct _tuple5 Cyc__gentuple_991={
0,{_tmp3C2,_tmp3C2,_tmp3C2 + 10},(void*)& Cyc__genrep_983};static unsigned char
_tmp3C3[15]="Forward_constr";static struct _tuple5 Cyc__gentuple_992={1,{_tmp3C3,
_tmp3C3,_tmp3C3 + 15},(void*)& Cyc__genrep_979};static struct _tuple5*Cyc__genarr_993[
2]={& Cyc__gentuple_991,& Cyc__gentuple_992};static unsigned char _tmp3C5[11]="Constraint";
struct Cyc_Typerep_TUnion_struct Cyc_tunion_Absyn_Constraint0Absyn_bounds_t2_rep={
5,{_tmp3C5,_tmp3C5,_tmp3C5 + 11},{(void*)((struct _tuple7**)Cyc__genarr_978),(void*)((
struct _tuple7**)Cyc__genarr_978),(void*)((struct _tuple7**)Cyc__genarr_978 + 1)},{(
void*)((struct _tuple5**)Cyc__genarr_993),(void*)((struct _tuple5**)Cyc__genarr_993),(
void*)((struct _tuple5**)Cyc__genarr_993 + 2)}};static unsigned char _tmp3C6[7]="Conref";
static struct _tagged_arr Cyc__genname_1000={_tmp3C6,_tmp3C6,_tmp3C6 + 7};static
unsigned char _tmp3C7[2]="v";static struct _tuple5 Cyc__gentuple_998={offsetof(
struct Cyc_Absyn_Conref,v),{_tmp3C7,_tmp3C7,_tmp3C7 + 2},(void*)& Cyc_tunion_Absyn_Constraint0Absyn_bounds_t2_rep};
static struct _tuple5*Cyc__genarr_999[1]={& Cyc__gentuple_998};struct Cyc_Typerep_Struct_struct
Cyc_struct_Absyn_Conref0bool2_rep={3,(struct _tagged_arr*)& Cyc__genname_1000,
sizeof(struct Cyc_Absyn_Conref),{(void*)((struct _tuple5**)Cyc__genarr_999),(void*)((
struct _tuple5**)Cyc__genarr_999),(void*)((struct _tuple5**)Cyc__genarr_999 + 1)}};
static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_997={1,1,(void*)((void*)& Cyc_struct_Absyn_Conref0bool2_rep)};
static unsigned char _tmp3CA[8]="PtrInfo";static struct _tagged_arr Cyc__genname_1007={
_tmp3CA,_tmp3CA,_tmp3CA + 8};static unsigned char _tmp3CB[8]="elt_typ";static struct
_tuple5 Cyc__gentuple_1001={offsetof(struct Cyc_Absyn_PtrInfo,elt_typ),{_tmp3CB,
_tmp3CB,_tmp3CB + 8},(void*)((void*)& Cyc_Absyn_type_t_rep)};static unsigned char
_tmp3CC[8]="rgn_typ";static struct _tuple5 Cyc__gentuple_1002={offsetof(struct Cyc_Absyn_PtrInfo,rgn_typ),{
_tmp3CC,_tmp3CC,_tmp3CC + 8},(void*)((void*)& Cyc_Absyn_type_t_rep)};static
unsigned char _tmp3CD[9]="nullable";static struct _tuple5 Cyc__gentuple_1003={
offsetof(struct Cyc_Absyn_PtrInfo,nullable),{_tmp3CD,_tmp3CD,_tmp3CD + 9},(void*)&
Cyc__genrep_997};static unsigned char _tmp3CE[3]="tq";static struct _tuple5 Cyc__gentuple_1004={
offsetof(struct Cyc_Absyn_PtrInfo,tq),{_tmp3CE,_tmp3CE,_tmp3CE + 3},(void*)& Cyc__genrep_132};
static unsigned char _tmp3CF[7]="bounds";static struct _tuple5 Cyc__gentuple_1005={
offsetof(struct Cyc_Absyn_PtrInfo,bounds),{_tmp3CF,_tmp3CF,_tmp3CF + 7},(void*)&
Cyc__genrep_976};static struct _tuple5*Cyc__genarr_1006[5]={& Cyc__gentuple_1001,&
Cyc__gentuple_1002,& Cyc__gentuple_1003,& Cyc__gentuple_1004,& Cyc__gentuple_1005};
struct Cyc_Typerep_Struct_struct Cyc_Absyn_ptr_info_t_rep={3,(struct _tagged_arr*)&
Cyc__genname_1007,sizeof(struct Cyc_Absyn_PtrInfo),{(void*)((struct _tuple5**)Cyc__genarr_1006),(
void*)((struct _tuple5**)Cyc__genarr_1006),(void*)((struct _tuple5**)Cyc__genarr_1006
+ 5)}};struct _tuple78{unsigned int f1;struct Cyc_Absyn_PtrInfo f2;};static struct
_tuple6 Cyc__gentuple_1008={offsetof(struct _tuple78,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_1009={offsetof(struct _tuple78,f2),(void*)& Cyc_Absyn_ptr_info_t_rep};
static struct _tuple6*Cyc__genarr_1010[2]={& Cyc__gentuple_1008,& Cyc__gentuple_1009};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_975={4,sizeof(struct _tuple78),{(
void*)((struct _tuple6**)Cyc__genarr_1010),(void*)((struct _tuple6**)Cyc__genarr_1010),(
void*)((struct _tuple6**)Cyc__genarr_1010 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_964;extern struct Cyc_Typerep_TUnion_struct Cyc_Absyn_size_of_t_rep;
static unsigned char _tmp3D2[3]="B1";static struct _tuple7 Cyc__gentuple_965={0,{
_tmp3D2,_tmp3D2,_tmp3D2 + 3}};static unsigned char _tmp3D3[3]="B2";static struct
_tuple7 Cyc__gentuple_966={1,{_tmp3D3,_tmp3D3,_tmp3D3 + 3}};static unsigned char
_tmp3D4[3]="B4";static struct _tuple7 Cyc__gentuple_967={2,{_tmp3D4,_tmp3D4,_tmp3D4
+ 3}};static unsigned char _tmp3D5[3]="B8";static struct _tuple7 Cyc__gentuple_968={3,{
_tmp3D5,_tmp3D5,_tmp3D5 + 3}};static struct _tuple7*Cyc__genarr_969[4]={& Cyc__gentuple_965,&
Cyc__gentuple_966,& Cyc__gentuple_967,& Cyc__gentuple_968};static struct _tuple5*Cyc__genarr_970[
0]={};static unsigned char _tmp3D7[8]="Size_of";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_size_of_t_rep={
5,{_tmp3D7,_tmp3D7,_tmp3D7 + 8},{(void*)((struct _tuple7**)Cyc__genarr_969),(void*)((
struct _tuple7**)Cyc__genarr_969),(void*)((struct _tuple7**)Cyc__genarr_969 + 4)},{(
void*)((struct _tuple5**)Cyc__genarr_970),(void*)((struct _tuple5**)Cyc__genarr_970),(
void*)((struct _tuple5**)Cyc__genarr_970 + 0)}};static struct _tuple6 Cyc__gentuple_971={
offsetof(struct _tuple66,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_972={
offsetof(struct _tuple66,f2),(void*)& Cyc_Absyn_sign_t_rep};static struct _tuple6 Cyc__gentuple_973={
offsetof(struct _tuple66,f3),(void*)& Cyc_Absyn_size_of_t_rep};static struct _tuple6*
Cyc__genarr_974[3]={& Cyc__gentuple_971,& Cyc__gentuple_972,& Cyc__gentuple_973};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_964={4,sizeof(struct _tuple66),{(
void*)((struct _tuple6**)Cyc__genarr_974),(void*)((struct _tuple6**)Cyc__genarr_974),(
void*)((struct _tuple6**)Cyc__genarr_974 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_958;struct _tuple79{unsigned int f1;void*f2;struct Cyc_Absyn_Tqual f3;
struct Cyc_Absyn_Exp*f4;};static struct _tuple6 Cyc__gentuple_959={offsetof(struct
_tuple79,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_960={
offsetof(struct _tuple79,f2),(void*)((void*)& Cyc_Absyn_type_t_rep)};static struct
_tuple6 Cyc__gentuple_961={offsetof(struct _tuple79,f3),(void*)& Cyc__genrep_132};
static struct _tuple6 Cyc__gentuple_962={offsetof(struct _tuple79,f4),(void*)& Cyc__genrep_73};
static struct _tuple6*Cyc__genarr_963[4]={& Cyc__gentuple_959,& Cyc__gentuple_960,&
Cyc__gentuple_961,& Cyc__gentuple_962};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_958={
4,sizeof(struct _tuple79),{(void*)((struct _tuple6**)Cyc__genarr_963),(void*)((
struct _tuple6**)Cyc__genarr_963),(void*)((struct _tuple6**)Cyc__genarr_963 + 4)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_939;extern struct Cyc_Typerep_Struct_struct
Cyc_Absyn_fn_info_t_rep;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_940;
extern struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060Core_opt_t0Absyn_var_t46H24Absyn_tqual_t4Absyn_type_t1_44099_6H2_rep;
static unsigned char _tmp3DA[5]="List";static struct _tagged_arr Cyc__genname_944={
_tmp3DA,_tmp3DA,_tmp3DA + 5};static unsigned char _tmp3DB[3]="hd";static struct
_tuple5 Cyc__gentuple_941={offsetof(struct Cyc_List_List,hd),{_tmp3DB,_tmp3DB,
_tmp3DB + 3},(void*)& Cyc__genrep_692};static unsigned char _tmp3DC[3]="tl";static
struct _tuple5 Cyc__gentuple_942={offsetof(struct Cyc_List_List,tl),{_tmp3DC,
_tmp3DC,_tmp3DC + 3},(void*)& Cyc__genrep_940};static struct _tuple5*Cyc__genarr_943[
2]={& Cyc__gentuple_941,& Cyc__gentuple_942};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List060Core_opt_t0Absyn_var_t46H24Absyn_tqual_t4Absyn_type_t1_44099_6H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_944,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_943),(void*)((struct _tuple5**)Cyc__genarr_943),(void*)((
struct _tuple5**)Cyc__genarr_943 + 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_940={
1,1,(void*)((void*)& Cyc_struct_List_List060Core_opt_t0Absyn_var_t46H24Absyn_tqual_t4Absyn_type_t1_44099_6H2_rep)};
static unsigned char _tmp3DF[7]="FnInfo";static struct _tagged_arr Cyc__genname_954={
_tmp3DF,_tmp3DF,_tmp3DF + 7};static unsigned char _tmp3E0[6]="tvars";static struct
_tuple5 Cyc__gentuple_945={offsetof(struct Cyc_Absyn_FnInfo,tvars),{_tmp3E0,
_tmp3E0,_tmp3E0 + 6},(void*)& Cyc__genrep_292};static unsigned char _tmp3E1[7]="effect";
static struct _tuple5 Cyc__gentuple_946={offsetof(struct Cyc_Absyn_FnInfo,effect),{
_tmp3E1,_tmp3E1,_tmp3E1 + 7},(void*)& Cyc__genrep_61};static unsigned char _tmp3E2[8]="ret_typ";
static struct _tuple5 Cyc__gentuple_947={offsetof(struct Cyc_Absyn_FnInfo,ret_typ),{
_tmp3E2,_tmp3E2,_tmp3E2 + 8},(void*)((void*)& Cyc_Absyn_type_t_rep)};static
unsigned char _tmp3E3[5]="args";static struct _tuple5 Cyc__gentuple_948={offsetof(
struct Cyc_Absyn_FnInfo,args),{_tmp3E3,_tmp3E3,_tmp3E3 + 5},(void*)& Cyc__genrep_940};
static unsigned char _tmp3E4[10]="c_varargs";static struct _tuple5 Cyc__gentuple_949={
offsetof(struct Cyc_Absyn_FnInfo,c_varargs),{_tmp3E4,_tmp3E4,_tmp3E4 + 10},(void*)((
void*)& Cyc__genrep_102)};static unsigned char _tmp3E5[12]="cyc_varargs";static
struct _tuple5 Cyc__gentuple_950={offsetof(struct Cyc_Absyn_FnInfo,cyc_varargs),{
_tmp3E5,_tmp3E5,_tmp3E5 + 12},(void*)& Cyc__genrep_576};static unsigned char _tmp3E6[
7]="rgn_po";static struct _tuple5 Cyc__gentuple_951={offsetof(struct Cyc_Absyn_FnInfo,rgn_po),{
_tmp3E6,_tmp3E6,_tmp3E6 + 7},(void*)& Cyc__genrep_566};static unsigned char _tmp3E7[
11]="attributes";static struct _tuple5 Cyc__gentuple_952={offsetof(struct Cyc_Absyn_FnInfo,attributes),{
_tmp3E7,_tmp3E7,_tmp3E7 + 11},(void*)& Cyc__genrep_83};static struct _tuple5*Cyc__genarr_953[
8]={& Cyc__gentuple_945,& Cyc__gentuple_946,& Cyc__gentuple_947,& Cyc__gentuple_948,&
Cyc__gentuple_949,& Cyc__gentuple_950,& Cyc__gentuple_951,& Cyc__gentuple_952};
struct Cyc_Typerep_Struct_struct Cyc_Absyn_fn_info_t_rep={3,(struct _tagged_arr*)&
Cyc__genname_954,sizeof(struct Cyc_Absyn_FnInfo),{(void*)((struct _tuple5**)Cyc__genarr_953),(
void*)((struct _tuple5**)Cyc__genarr_953),(void*)((struct _tuple5**)Cyc__genarr_953
+ 8)}};struct _tuple80{unsigned int f1;struct Cyc_Absyn_FnInfo f2;};static struct
_tuple6 Cyc__gentuple_955={offsetof(struct _tuple80,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_956={offsetof(struct _tuple80,f2),(void*)& Cyc_Absyn_fn_info_t_rep};
static struct _tuple6*Cyc__genarr_957[2]={& Cyc__gentuple_955,& Cyc__gentuple_956};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_939={4,sizeof(struct _tuple80),{(
void*)((struct _tuple6**)Cyc__genarr_957),(void*)((struct _tuple6**)Cyc__genarr_957),(
void*)((struct _tuple6**)Cyc__genarr_957 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_935;static struct _tuple6 Cyc__gentuple_936={offsetof(struct _tuple11,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_937={offsetof(struct
_tuple11,f2),(void*)& Cyc__genrep_266};static struct _tuple6*Cyc__genarr_938[2]={&
Cyc__gentuple_936,& Cyc__gentuple_937};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_935={
4,sizeof(struct _tuple11),{(void*)((struct _tuple6**)Cyc__genarr_938),(void*)((
struct _tuple6**)Cyc__genarr_938),(void*)((struct _tuple6**)Cyc__genarr_938 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_931;struct _tuple81{unsigned int
f1;struct Cyc_Absyn_AggrInfo f2;};static struct _tuple6 Cyc__gentuple_932={offsetof(
struct _tuple81,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_933={
offsetof(struct _tuple81,f2),(void*)& Cyc_Absyn_aggr_info_t_rep};static struct
_tuple6*Cyc__genarr_934[2]={& Cyc__gentuple_932,& Cyc__gentuple_933};static struct
Cyc_Typerep_Tuple_struct Cyc__genrep_931={4,sizeof(struct _tuple81),{(void*)((
struct _tuple6**)Cyc__genarr_934),(void*)((struct _tuple6**)Cyc__genarr_934),(void*)((
struct _tuple6**)Cyc__genarr_934 + 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_926;
static struct _tuple6 Cyc__gentuple_927={offsetof(struct _tuple58,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_928={offsetof(struct _tuple58,f2),(void*)& Cyc_Absyn_aggr_kind_t_rep};
static struct _tuple6 Cyc__gentuple_929={offsetof(struct _tuple58,f3),(void*)& Cyc__genrep_338};
static struct _tuple6*Cyc__genarr_930[3]={& Cyc__gentuple_927,& Cyc__gentuple_928,&
Cyc__gentuple_929};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_926={4,
sizeof(struct _tuple58),{(void*)((struct _tuple6**)Cyc__genarr_930),(void*)((
struct _tuple6**)Cyc__genarr_930),(void*)((struct _tuple6**)Cyc__genarr_930 + 3)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_921;struct _tuple82{unsigned int
f1;struct _tuple0*f2;struct Cyc_Absyn_Enumdecl*f3;};static struct _tuple6 Cyc__gentuple_922={
offsetof(struct _tuple82,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_923={
offsetof(struct _tuple82,f2),(void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_924={
offsetof(struct _tuple82,f3),(void*)& Cyc__genrep_656};static struct _tuple6*Cyc__genarr_925[
3]={& Cyc__gentuple_922,& Cyc__gentuple_923,& Cyc__gentuple_924};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_921={4,sizeof(struct _tuple82),{(void*)((struct _tuple6**)Cyc__genarr_925),(
void*)((struct _tuple6**)Cyc__genarr_925),(void*)((struct _tuple6**)Cyc__genarr_925
+ 3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_70;static struct _tuple6 Cyc__gentuple_918={
offsetof(struct _tuple11,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_919={
offsetof(struct _tuple11,f2),(void*)& Cyc__genrep_71};static struct _tuple6*Cyc__genarr_920[
2]={& Cyc__gentuple_918,& Cyc__gentuple_919};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_70={4,sizeof(struct _tuple11),{(void*)((struct _tuple6**)Cyc__genarr_920),(
void*)((struct _tuple6**)Cyc__genarr_920),(void*)((struct _tuple6**)Cyc__genarr_920
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_60;struct _tuple83{
unsigned int f1;struct _tuple0*f2;struct Cyc_List_List*f3;struct Cyc_Core_Opt*f4;};
static struct _tuple6 Cyc__gentuple_65={offsetof(struct _tuple83,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_66={offsetof(struct _tuple83,f2),(void*)& Cyc__genrep_10};
static struct _tuple6 Cyc__gentuple_67={offsetof(struct _tuple83,f3),(void*)& Cyc__genrep_52};
static struct _tuple6 Cyc__gentuple_68={offsetof(struct _tuple83,f4),(void*)& Cyc__genrep_61};
static struct _tuple6*Cyc__genarr_69[4]={& Cyc__gentuple_65,& Cyc__gentuple_66,& Cyc__gentuple_67,&
Cyc__gentuple_68};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_60={4,sizeof(
struct _tuple83),{(void*)((struct _tuple6**)Cyc__genarr_69),(void*)((struct _tuple6**)
Cyc__genarr_69),(void*)((struct _tuple6**)Cyc__genarr_69 + 4)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_51;static struct _tuple6 Cyc__gentuple_57={offsetof(struct _tuple11,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_58={offsetof(struct
_tuple11,f2),(void*)& Cyc__genrep_52};static struct _tuple6*Cyc__genarr_59[2]={& Cyc__gentuple_57,&
Cyc__gentuple_58};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_51={4,sizeof(
struct _tuple11),{(void*)((struct _tuple6**)Cyc__genarr_59),(void*)((struct _tuple6**)
Cyc__genarr_59),(void*)((struct _tuple6**)Cyc__genarr_59 + 2)}};static
unsigned char _tmp3F1[9]="VoidType";static struct _tuple7 Cyc__gentuple_43={0,{
_tmp3F1,_tmp3F1,_tmp3F1 + 9}};static unsigned char _tmp3F2[10]="FloatType";static
struct _tuple7 Cyc__gentuple_44={1,{_tmp3F2,_tmp3F2,_tmp3F2 + 10}};static
unsigned char _tmp3F3[8]="HeapRgn";static struct _tuple7 Cyc__gentuple_45={2,{
_tmp3F3,_tmp3F3,_tmp3F3 + 8}};static struct _tuple7*Cyc__genarr_46[3]={& Cyc__gentuple_43,&
Cyc__gentuple_44,& Cyc__gentuple_45};static unsigned char _tmp3F4[5]="Evar";static
struct _tuple5 Cyc__gentuple_1078={0,{_tmp3F4,_tmp3F4,_tmp3F4 + 5},(void*)& Cyc__genrep_1067};
static unsigned char _tmp3F5[8]="VarType";static struct _tuple5 Cyc__gentuple_1079={1,{
_tmp3F5,_tmp3F5,_tmp3F5 + 8},(void*)& Cyc__genrep_1063};static unsigned char _tmp3F6[
11]="TunionType";static struct _tuple5 Cyc__gentuple_1080={2,{_tmp3F6,_tmp3F6,
_tmp3F6 + 11},(void*)& Cyc__genrep_1037};static unsigned char _tmp3F7[16]="TunionFieldType";
static struct _tuple5 Cyc__gentuple_1081={3,{_tmp3F7,_tmp3F7,_tmp3F7 + 16},(void*)&
Cyc__genrep_1011};static unsigned char _tmp3F8[12]="PointerType";static struct
_tuple5 Cyc__gentuple_1082={4,{_tmp3F8,_tmp3F8,_tmp3F8 + 12},(void*)& Cyc__genrep_975};
static unsigned char _tmp3F9[8]="IntType";static struct _tuple5 Cyc__gentuple_1083={5,{
_tmp3F9,_tmp3F9,_tmp3F9 + 8},(void*)& Cyc__genrep_964};static unsigned char _tmp3FA[
11]="DoubleType";static struct _tuple5 Cyc__gentuple_1084={6,{_tmp3FA,_tmp3FA,
_tmp3FA + 11},(void*)& Cyc__genrep_116};static unsigned char _tmp3FB[10]="ArrayType";
static struct _tuple5 Cyc__gentuple_1085={7,{_tmp3FB,_tmp3FB,_tmp3FB + 10},(void*)&
Cyc__genrep_958};static unsigned char _tmp3FC[7]="FnType";static struct _tuple5 Cyc__gentuple_1086={
8,{_tmp3FC,_tmp3FC,_tmp3FC + 7},(void*)& Cyc__genrep_939};static unsigned char
_tmp3FD[10]="TupleType";static struct _tuple5 Cyc__gentuple_1087={9,{_tmp3FD,
_tmp3FD,_tmp3FD + 10},(void*)& Cyc__genrep_935};static unsigned char _tmp3FE[9]="AggrType";
static struct _tuple5 Cyc__gentuple_1088={10,{_tmp3FE,_tmp3FE,_tmp3FE + 9},(void*)&
Cyc__genrep_931};static unsigned char _tmp3FF[13]="AnonAggrType";static struct
_tuple5 Cyc__gentuple_1089={11,{_tmp3FF,_tmp3FF,_tmp3FF + 13},(void*)& Cyc__genrep_926};
static unsigned char _tmp400[9]="EnumType";static struct _tuple5 Cyc__gentuple_1090={
12,{_tmp400,_tmp400,_tmp400 + 9},(void*)& Cyc__genrep_921};static unsigned char
_tmp401[13]="AnonEnumType";static struct _tuple5 Cyc__gentuple_1091={13,{_tmp401,
_tmp401,_tmp401 + 13},(void*)& Cyc__genrep_70};static unsigned char _tmp402[11]="SizeofType";
static struct _tuple5 Cyc__gentuple_1092={14,{_tmp402,_tmp402,_tmp402 + 11},(void*)&
Cyc__genrep_47};static unsigned char _tmp403[14]="RgnHandleType";static struct
_tuple5 Cyc__gentuple_1093={15,{_tmp403,_tmp403,_tmp403 + 14},(void*)& Cyc__genrep_47};
static unsigned char _tmp404[12]="TypedefType";static struct _tuple5 Cyc__gentuple_1094={
16,{_tmp404,_tmp404,_tmp404 + 12},(void*)& Cyc__genrep_60};static unsigned char
_tmp405[10]="AccessEff";static struct _tuple5 Cyc__gentuple_1095={17,{_tmp405,
_tmp405,_tmp405 + 10},(void*)& Cyc__genrep_47};static unsigned char _tmp406[8]="JoinEff";
static struct _tuple5 Cyc__gentuple_1096={18,{_tmp406,_tmp406,_tmp406 + 8},(void*)&
Cyc__genrep_51};static unsigned char _tmp407[8]="RgnsEff";static struct _tuple5 Cyc__gentuple_1097={
19,{_tmp407,_tmp407,_tmp407 + 8},(void*)& Cyc__genrep_47};static struct _tuple5*Cyc__genarr_1098[
20]={& Cyc__gentuple_1078,& Cyc__gentuple_1079,& Cyc__gentuple_1080,& Cyc__gentuple_1081,&
Cyc__gentuple_1082,& Cyc__gentuple_1083,& Cyc__gentuple_1084,& Cyc__gentuple_1085,&
Cyc__gentuple_1086,& Cyc__gentuple_1087,& Cyc__gentuple_1088,& Cyc__gentuple_1089,&
Cyc__gentuple_1090,& Cyc__gentuple_1091,& Cyc__gentuple_1092,& Cyc__gentuple_1093,&
Cyc__gentuple_1094,& Cyc__gentuple_1095,& Cyc__gentuple_1096,& Cyc__gentuple_1097};
static unsigned char _tmp409[5]="Type";struct Cyc_Typerep_TUnion_struct Cyc_Absyn_type_t_rep={
5,{_tmp409,_tmp409,_tmp409 + 5},{(void*)((struct _tuple7**)Cyc__genarr_46),(void*)((
struct _tuple7**)Cyc__genarr_46),(void*)((struct _tuple7**)Cyc__genarr_46 + 3)},{(
void*)((struct _tuple5**)Cyc__genarr_1098),(void*)((struct _tuple5**)Cyc__genarr_1098),(
void*)((struct _tuple5**)Cyc__genarr_1098 + 20)}};static unsigned char _tmp40A[8]="Vardecl";
static struct _tagged_arr Cyc__genname_151={_tmp40A,_tmp40A,_tmp40A + 8};static
unsigned char _tmp40B[3]="sc";static struct _tuple5 Cyc__gentuple_142={offsetof(
struct Cyc_Absyn_Vardecl,sc),{_tmp40B,_tmp40B,_tmp40B + 3},(void*)& Cyc_Absyn_scope_t_rep};
static unsigned char _tmp40C[5]="name";static struct _tuple5 Cyc__gentuple_143={
offsetof(struct Cyc_Absyn_Vardecl,name),{_tmp40C,_tmp40C,_tmp40C + 5},(void*)& Cyc__genrep_10};
static unsigned char _tmp40D[3]="tq";static struct _tuple5 Cyc__gentuple_144={
offsetof(struct Cyc_Absyn_Vardecl,tq),{_tmp40D,_tmp40D,_tmp40D + 3},(void*)& Cyc__genrep_132};
static unsigned char _tmp40E[5]="type";static struct _tuple5 Cyc__gentuple_145={
offsetof(struct Cyc_Absyn_Vardecl,type),{_tmp40E,_tmp40E,_tmp40E + 5},(void*)((
void*)& Cyc_Absyn_type_t_rep)};static unsigned char _tmp40F[12]="initializer";
static struct _tuple5 Cyc__gentuple_146={offsetof(struct Cyc_Absyn_Vardecl,initializer),{
_tmp40F,_tmp40F,_tmp40F + 12},(void*)& Cyc__genrep_73};static unsigned char _tmp410[
4]="rgn";static struct _tuple5 Cyc__gentuple_147={offsetof(struct Cyc_Absyn_Vardecl,rgn),{
_tmp410,_tmp410,_tmp410 + 4},(void*)& Cyc__genrep_61};static unsigned char _tmp411[
11]="attributes";static struct _tuple5 Cyc__gentuple_148={offsetof(struct Cyc_Absyn_Vardecl,attributes),{
_tmp411,_tmp411,_tmp411 + 11},(void*)& Cyc__genrep_83};static unsigned char _tmp412[
8]="escapes";static struct _tuple5 Cyc__gentuple_149={offsetof(struct Cyc_Absyn_Vardecl,escapes),{
_tmp412,_tmp412,_tmp412 + 8},(void*)((void*)& Cyc__genrep_102)};static struct
_tuple5*Cyc__genarr_150[8]={& Cyc__gentuple_142,& Cyc__gentuple_143,& Cyc__gentuple_144,&
Cyc__gentuple_145,& Cyc__gentuple_146,& Cyc__gentuple_147,& Cyc__gentuple_148,& Cyc__gentuple_149};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Vardecl_rep={3,(struct
_tagged_arr*)& Cyc__genname_151,sizeof(struct Cyc_Absyn_Vardecl),{(void*)((struct
_tuple5**)Cyc__genarr_150),(void*)((struct _tuple5**)Cyc__genarr_150),(void*)((
struct _tuple5**)Cyc__genarr_150 + 8)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_131={
1,1,(void*)((void*)& Cyc_struct_Absyn_Vardecl_rep)};struct _tuple84{unsigned int f1;
struct Cyc_Absyn_Vardecl*f2;};static struct _tuple6 Cyc__gentuple_392={offsetof(
struct _tuple84,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_393={
offsetof(struct _tuple84,f2),(void*)& Cyc__genrep_131};static struct _tuple6*Cyc__genarr_394[
2]={& Cyc__gentuple_392,& Cyc__gentuple_393};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_391={4,sizeof(struct _tuple84),{(void*)((struct _tuple6**)Cyc__genarr_394),(
void*)((struct _tuple6**)Cyc__genarr_394),(void*)((struct _tuple6**)Cyc__genarr_394
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1123;struct _tuple85{
unsigned int f1;struct Cyc_Absyn_Pat*f2;struct Cyc_Core_Opt*f3;struct Cyc_Core_Opt*
f4;struct Cyc_Absyn_Exp*f5;int f6;};static struct _tuple6 Cyc__gentuple_1124={
offsetof(struct _tuple85,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1125={
offsetof(struct _tuple85,f2),(void*)& Cyc__genrep_226};static struct _tuple6 Cyc__gentuple_1126={
offsetof(struct _tuple85,f3),(void*)& Cyc__genrep_129};static struct _tuple6 Cyc__gentuple_1127={
offsetof(struct _tuple85,f4),(void*)& Cyc__genrep_61};static struct _tuple6 Cyc__gentuple_1128={
offsetof(struct _tuple85,f5),(void*)& Cyc__genrep_77};static struct _tuple6 Cyc__gentuple_1129={
offsetof(struct _tuple85,f6),(void*)((void*)& Cyc__genrep_102)};static struct
_tuple6*Cyc__genarr_1130[6]={& Cyc__gentuple_1124,& Cyc__gentuple_1125,& Cyc__gentuple_1126,&
Cyc__gentuple_1127,& Cyc__gentuple_1128,& Cyc__gentuple_1129};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1123={4,sizeof(struct _tuple85),{(void*)((struct _tuple6**)Cyc__genarr_1130),(
void*)((struct _tuple6**)Cyc__genarr_1130),(void*)((struct _tuple6**)Cyc__genarr_1130
+ 6)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1119;static struct _tuple6
Cyc__gentuple_1120={offsetof(struct _tuple11,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_1121={offsetof(struct _tuple11,f2),(void*)& Cyc__genrep_130};
static struct _tuple6*Cyc__genarr_1122[2]={& Cyc__gentuple_1120,& Cyc__gentuple_1121};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1119={4,sizeof(struct _tuple11),{(
void*)((struct _tuple6**)Cyc__genarr_1122),(void*)((struct _tuple6**)Cyc__genarr_1122),(
void*)((struct _tuple6**)Cyc__genarr_1122 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1115;struct _tuple86{unsigned int f1;struct Cyc_Absyn_Aggrdecl*f2;};
static struct _tuple6 Cyc__gentuple_1116={offsetof(struct _tuple86,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_1117={offsetof(struct _tuple86,f2),(void*)((void*)&
Cyc__genrep_336)};static struct _tuple6*Cyc__genarr_1118[2]={& Cyc__gentuple_1116,&
Cyc__gentuple_1117};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1115={4,
sizeof(struct _tuple86),{(void*)((struct _tuple6**)Cyc__genarr_1118),(void*)((
struct _tuple6**)Cyc__genarr_1118),(void*)((struct _tuple6**)Cyc__genarr_1118 + 2)}};
extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_1111;struct _tuple87{unsigned int
f1;struct Cyc_Absyn_Tuniondecl*f2;};static struct _tuple6 Cyc__gentuple_1112={
offsetof(struct _tuple87,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1113={
offsetof(struct _tuple87,f2),(void*)((void*)& Cyc__genrep_282)};static struct
_tuple6*Cyc__genarr_1114[2]={& Cyc__gentuple_1112,& Cyc__gentuple_1113};static
struct Cyc_Typerep_Tuple_struct Cyc__genrep_1111={4,sizeof(struct _tuple87),{(void*)((
struct _tuple6**)Cyc__genarr_1114),(void*)((struct _tuple6**)Cyc__genarr_1114),(
void*)((struct _tuple6**)Cyc__genarr_1114 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_1107;struct _tuple88{unsigned int f1;struct Cyc_Absyn_Enumdecl*f2;};
static struct _tuple6 Cyc__gentuple_1108={offsetof(struct _tuple88,f1),(void*)& Cyc__genrep_5};
static struct _tuple6 Cyc__gentuple_1109={offsetof(struct _tuple88,f2),(void*)& Cyc__genrep_250};
static struct _tuple6*Cyc__genarr_1110[2]={& Cyc__gentuple_1108,& Cyc__gentuple_1109};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_1107={4,sizeof(struct _tuple88),{(
void*)((struct _tuple6**)Cyc__genarr_1110),(void*)((struct _tuple6**)Cyc__genarr_1110),(
void*)((struct _tuple6**)Cyc__genarr_1110 + 2)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_41;extern struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_42;extern struct
Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Typedefdecl_rep;static unsigned char
_tmp41B[12]="Typedefdecl";static struct _tagged_arr Cyc__genname_1103={_tmp41B,
_tmp41B,_tmp41B + 12};static unsigned char _tmp41C[5]="name";static struct _tuple5 Cyc__gentuple_1099={
offsetof(struct Cyc_Absyn_Typedefdecl,name),{_tmp41C,_tmp41C,_tmp41C + 5},(void*)&
Cyc__genrep_10};static unsigned char _tmp41D[4]="tvs";static struct _tuple5 Cyc__gentuple_1100={
offsetof(struct Cyc_Absyn_Typedefdecl,tvs),{_tmp41D,_tmp41D,_tmp41D + 4},(void*)&
Cyc__genrep_292};static unsigned char _tmp41E[5]="defn";static struct _tuple5 Cyc__gentuple_1101={
offsetof(struct Cyc_Absyn_Typedefdecl,defn),{_tmp41E,_tmp41E,_tmp41E + 5},(void*)&
Cyc_Absyn_type_t_rep};static struct _tuple5*Cyc__genarr_1102[3]={& Cyc__gentuple_1099,&
Cyc__gentuple_1100,& Cyc__gentuple_1101};struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Typedefdecl_rep={
3,(struct _tagged_arr*)& Cyc__genname_1103,sizeof(struct Cyc_Absyn_Typedefdecl),{(
void*)((struct _tuple5**)Cyc__genarr_1102),(void*)((struct _tuple5**)Cyc__genarr_1102),(
void*)((struct _tuple5**)Cyc__genarr_1102 + 3)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_42={1,1,(void*)((void*)& Cyc_struct_Absyn_Typedefdecl_rep)};struct
_tuple89{unsigned int f1;struct Cyc_Absyn_Typedefdecl*f2;};static struct _tuple6 Cyc__gentuple_1104={
offsetof(struct _tuple89,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_1105={
offsetof(struct _tuple89,f2),(void*)& Cyc__genrep_42};static struct _tuple6*Cyc__genarr_1106[
2]={& Cyc__gentuple_1104,& Cyc__gentuple_1105};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_41={4,sizeof(struct _tuple89),{(void*)((struct _tuple6**)Cyc__genarr_1106),(
void*)((struct _tuple6**)Cyc__genarr_1106),(void*)((struct _tuple6**)Cyc__genarr_1106
+ 2)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_36;struct _tuple90{
unsigned int f1;struct _tagged_arr*f2;struct Cyc_List_List*f3;};static struct _tuple6
Cyc__gentuple_37={offsetof(struct _tuple90,f1),(void*)& Cyc__genrep_5};static
struct _tuple6 Cyc__gentuple_38={offsetof(struct _tuple90,f2),(void*)& Cyc__genrep_12};
static struct _tuple6 Cyc__gentuple_39={offsetof(struct _tuple90,f3),(void*)& Cyc__genrep_0};
static struct _tuple6*Cyc__genarr_40[3]={& Cyc__gentuple_37,& Cyc__gentuple_38,& Cyc__gentuple_39};
static struct Cyc_Typerep_Tuple_struct Cyc__genrep_36={4,sizeof(struct _tuple90),{(
void*)((struct _tuple6**)Cyc__genarr_40),(void*)((struct _tuple6**)Cyc__genarr_40),(
void*)((struct _tuple6**)Cyc__genarr_40 + 3)}};extern struct Cyc_Typerep_Tuple_struct
Cyc__genrep_9;static struct _tuple6 Cyc__gentuple_32={offsetof(struct _tuple48,f1),(
void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_33={offsetof(struct
_tuple48,f2),(void*)& Cyc__genrep_10};static struct _tuple6 Cyc__gentuple_34={
offsetof(struct _tuple48,f3),(void*)& Cyc__genrep_0};static struct _tuple6*Cyc__genarr_35[
3]={& Cyc__gentuple_32,& Cyc__gentuple_33,& Cyc__gentuple_34};static struct Cyc_Typerep_Tuple_struct
Cyc__genrep_9={4,sizeof(struct _tuple48),{(void*)((struct _tuple6**)Cyc__genarr_35),(
void*)((struct _tuple6**)Cyc__genarr_35),(void*)((struct _tuple6**)Cyc__genarr_35 + 
3)}};extern struct Cyc_Typerep_Tuple_struct Cyc__genrep_4;static struct _tuple6 Cyc__gentuple_6={
offsetof(struct _tuple11,f1),(void*)& Cyc__genrep_5};static struct _tuple6 Cyc__gentuple_7={
offsetof(struct _tuple11,f2),(void*)& Cyc__genrep_0};static struct _tuple6*Cyc__genarr_8[
2]={& Cyc__gentuple_6,& Cyc__gentuple_7};static struct Cyc_Typerep_Tuple_struct Cyc__genrep_4={
4,sizeof(struct _tuple11),{(void*)((struct _tuple6**)Cyc__genarr_8),(void*)((
struct _tuple6**)Cyc__genarr_8),(void*)((struct _tuple6**)Cyc__genarr_8 + 2)}};
static struct _tuple7*Cyc__genarr_3[0]={};static unsigned char _tmp425[6]="Var_d";
static struct _tuple5 Cyc__gentuple_1131={0,{_tmp425,_tmp425,_tmp425 + 6},(void*)&
Cyc__genrep_391};static unsigned char _tmp426[5]="Fn_d";static struct _tuple5 Cyc__gentuple_1132={
1,{_tmp426,_tmp426,_tmp426 + 5},(void*)& Cyc__genrep_81};static unsigned char
_tmp427[6]="Let_d";static struct _tuple5 Cyc__gentuple_1133={2,{_tmp427,_tmp427,
_tmp427 + 6},(void*)& Cyc__genrep_1123};static unsigned char _tmp428[7]="Letv_d";
static struct _tuple5 Cyc__gentuple_1134={3,{_tmp428,_tmp428,_tmp428 + 7},(void*)&
Cyc__genrep_1119};static unsigned char _tmp429[7]="Aggr_d";static struct _tuple5 Cyc__gentuple_1135={
4,{_tmp429,_tmp429,_tmp429 + 7},(void*)& Cyc__genrep_1115};static unsigned char
_tmp42A[9]="Tunion_d";static struct _tuple5 Cyc__gentuple_1136={5,{_tmp42A,_tmp42A,
_tmp42A + 9},(void*)& Cyc__genrep_1111};static unsigned char _tmp42B[7]="Enum_d";
static struct _tuple5 Cyc__gentuple_1137={6,{_tmp42B,_tmp42B,_tmp42B + 7},(void*)&
Cyc__genrep_1107};static unsigned char _tmp42C[10]="Typedef_d";static struct _tuple5
Cyc__gentuple_1138={7,{_tmp42C,_tmp42C,_tmp42C + 10},(void*)& Cyc__genrep_41};
static unsigned char _tmp42D[12]="Namespace_d";static struct _tuple5 Cyc__gentuple_1139={
8,{_tmp42D,_tmp42D,_tmp42D + 12},(void*)& Cyc__genrep_36};static unsigned char
_tmp42E[8]="Using_d";static struct _tuple5 Cyc__gentuple_1140={9,{_tmp42E,_tmp42E,
_tmp42E + 8},(void*)& Cyc__genrep_9};static unsigned char _tmp42F[10]="ExternC_d";
static struct _tuple5 Cyc__gentuple_1141={10,{_tmp42F,_tmp42F,_tmp42F + 10},(void*)&
Cyc__genrep_4};static struct _tuple5*Cyc__genarr_1142[11]={& Cyc__gentuple_1131,&
Cyc__gentuple_1132,& Cyc__gentuple_1133,& Cyc__gentuple_1134,& Cyc__gentuple_1135,&
Cyc__gentuple_1136,& Cyc__gentuple_1137,& Cyc__gentuple_1138,& Cyc__gentuple_1139,&
Cyc__gentuple_1140,& Cyc__gentuple_1141};static unsigned char _tmp431[9]="Raw_decl";
struct Cyc_Typerep_TUnion_struct Cyc_Absyn_raw_decl_t_rep={5,{_tmp431,_tmp431,
_tmp431 + 9},{(void*)((struct _tuple7**)Cyc__genarr_3),(void*)((struct _tuple7**)
Cyc__genarr_3),(void*)((struct _tuple7**)Cyc__genarr_3 + 0)},{(void*)((struct
_tuple5**)Cyc__genarr_1142),(void*)((struct _tuple5**)Cyc__genarr_1142),(void*)((
struct _tuple5**)Cyc__genarr_1142 + 11)}};static unsigned char _tmp432[5]="Decl";
static struct _tagged_arr Cyc__genname_1146={_tmp432,_tmp432,_tmp432 + 5};static
unsigned char _tmp433[2]="r";static struct _tuple5 Cyc__gentuple_1143={offsetof(
struct Cyc_Absyn_Decl,r),{_tmp433,_tmp433,_tmp433 + 2},(void*)& Cyc_Absyn_raw_decl_t_rep};
static unsigned char _tmp434[4]="loc";static struct _tuple5 Cyc__gentuple_1144={
offsetof(struct Cyc_Absyn_Decl,loc),{_tmp434,_tmp434,_tmp434 + 4},(void*)& Cyc__genrep_2};
static struct _tuple5*Cyc__genarr_1145[2]={& Cyc__gentuple_1143,& Cyc__gentuple_1144};
struct Cyc_Typerep_Struct_struct Cyc_struct_Absyn_Decl_rep={3,(struct _tagged_arr*)&
Cyc__genname_1146,sizeof(struct Cyc_Absyn_Decl),{(void*)((struct _tuple5**)Cyc__genarr_1145),(
void*)((struct _tuple5**)Cyc__genarr_1145),(void*)((struct _tuple5**)Cyc__genarr_1145
+ 2)}};static struct Cyc_Typerep_ThinPtr_struct Cyc__genrep_1={1,1,(void*)((void*)&
Cyc_struct_Absyn_Decl_rep)};static unsigned char _tmp437[5]="List";static struct
_tagged_arr Cyc__genname_1150={_tmp437,_tmp437,_tmp437 + 5};static unsigned char
_tmp438[3]="hd";static struct _tuple5 Cyc__gentuple_1147={offsetof(struct Cyc_List_List,hd),{
_tmp438,_tmp438,_tmp438 + 3},(void*)& Cyc__genrep_1};static unsigned char _tmp439[3]="tl";
static struct _tuple5 Cyc__gentuple_1148={offsetof(struct Cyc_List_List,tl),{_tmp439,
_tmp439,_tmp439 + 3},(void*)& Cyc__genrep_0};static struct _tuple5*Cyc__genarr_1149[
2]={& Cyc__gentuple_1147,& Cyc__gentuple_1148};struct Cyc_Typerep_Struct_struct Cyc_struct_List_List0Absyn_decl_t46H2_rep={
3,(struct _tagged_arr*)& Cyc__genname_1150,sizeof(struct Cyc_List_List),{(void*)((
struct _tuple5**)Cyc__genarr_1149),(void*)((struct _tuple5**)Cyc__genarr_1149),(
void*)((struct _tuple5**)Cyc__genarr_1149 + 2)}};static struct Cyc_Typerep_ThinPtr_struct
Cyc__genrep_0={1,1,(void*)((void*)& Cyc_struct_List_List0Absyn_decl_t46H2_rep)};
void*Cyc_decls_rep=(void*)& Cyc__genrep_0;
