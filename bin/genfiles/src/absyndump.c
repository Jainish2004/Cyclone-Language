 struct Cyc_timeval{ int tv_sec; int tv_usec; } ; struct Cyc_timespec{
unsigned int tv_sec; int tv_nsec; } ; struct Cyc_itimerspec{ struct Cyc_timespec
it_interval; struct Cyc_timespec it_value; } ; struct Cyc__types_fd_set{ int
fds_bits[ 2u]; } ; struct Cyc_dirent{ int d_ino; int d_off; unsigned short
d_reclen; unsigned char d_type; unsigned char d_name[ 256u]; } ; extern void
exit( int); extern void* abort(); struct Cyc_Core_Opt{ void* v; } ; extern
unsigned char Cyc_Core_InvalidArg[ 15u]; struct Cyc_Core_InvalidArg_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char Cyc_Core_Failure[
12u]; struct Cyc_Core_Failure_struct{ unsigned char* tag; struct _tagged_arr f1;
} ; extern unsigned char Cyc_Core_Impossible[ 15u]; struct Cyc_Core_Impossible_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern unsigned char Cyc_Core_Not_found[
14u]; extern unsigned char Cyc_Core_Unreachable[ 16u]; struct Cyc_Core_Unreachable_struct{
unsigned char* tag; struct _tagged_arr f1; } ; extern struct _tagged_arr Cyc_Core_string_of_int(
int); extern unsigned char* string_to_Cstring( struct _tagged_arr); extern
unsigned char* underlying_Cstring( struct _tagged_arr); extern struct
_tagged_arr Cstring_to_string( unsigned char*); extern struct _tagged_arr
wrap_Cstring_as_string( unsigned char*, int); extern struct _tagged_arr
ntCsl_to_ntsl( unsigned char**); extern int system( unsigned char*); extern int*
__errno(); struct Cyc_Stdio___sFILE; extern struct Cyc_Stdio___sFILE* Cyc_Stdio_stdout;
extern int Cyc_Stdio_fputc( int, struct Cyc_Stdio___sFILE*); extern
unsigned char Cyc_Stdio_FileCloseError[ 19u]; extern unsigned char Cyc_Stdio_FileOpenError[
18u]; struct Cyc_Stdio_FileOpenError_struct{ unsigned char* tag; struct
_tagged_arr f1; } ; extern int Cyc_Stdio_file_string_write( struct Cyc_Stdio___sFILE*
fd, struct _tagged_arr src, int src_offset, int max_count); static const int Cyc_Stdio_String_pa=
0; struct Cyc_Stdio_String_pa_struct{ int tag; struct _tagged_arr f1; } ; static
const int Cyc_Stdio_Int_pa= 1; struct Cyc_Stdio_Int_pa_struct{ int tag;
unsigned int f1; } ; static const int Cyc_Stdio_Double_pa= 2; struct Cyc_Stdio_Double_pa_struct{
int tag; double f1; } ; static const int Cyc_Stdio_ShortPtr_pa= 3; struct Cyc_Stdio_ShortPtr_pa_struct{
int tag; short* f1; } ; static const int Cyc_Stdio_IntPtr_pa= 4; struct Cyc_Stdio_IntPtr_pa_struct{
int tag; unsigned int* f1; } ; extern int Cyc_Stdio_fprintf( struct Cyc_Stdio___sFILE*,
struct _tagged_arr fmt, struct _tagged_arr); extern struct _tagged_arr Cyc_Stdio_aprintf(
struct _tagged_arr fmt, struct _tagged_arr); static const int Cyc_Stdio_ShortPtr_sa=
0; struct Cyc_Stdio_ShortPtr_sa_struct{ int tag; short* f1; } ; static const int
Cyc_Stdio_UShortPtr_sa= 1; struct Cyc_Stdio_UShortPtr_sa_struct{ int tag;
unsigned short* f1; } ; static const int Cyc_Stdio_IntPtr_sa= 2; struct Cyc_Stdio_IntPtr_sa_struct{
int tag; int* f1; } ; static const int Cyc_Stdio_UIntPtr_sa= 3; struct Cyc_Stdio_UIntPtr_sa_struct{
int tag; unsigned int* f1; } ; static const int Cyc_Stdio_StringPtr_sa= 4;
struct Cyc_Stdio_StringPtr_sa_struct{ int tag; struct _tagged_arr f1; } ; static
const int Cyc_Stdio_DoublePtr_sa= 5; struct Cyc_Stdio_DoublePtr_sa_struct{ int
tag; double* f1; } ; static const int Cyc_Stdio_FloatPtr_sa= 6; struct Cyc_Stdio_FloatPtr_sa_struct{
int tag; float* f1; } ; struct Cyc_List_List{ void* hd; struct Cyc_List_List* tl;
} ; extern int Cyc_List_length( struct Cyc_List_List* x); extern unsigned char
Cyc_List_List_empty[ 15u]; extern struct Cyc_List_List* Cyc_List_map( void*(* f)(
void*), struct Cyc_List_List* x); extern unsigned char Cyc_List_List_mismatch[
18u]; extern struct Cyc_List_List* Cyc_List_imp_rev( struct Cyc_List_List* x);
extern struct Cyc_List_List* Cyc_List_imp_append( struct Cyc_List_List* x,
struct Cyc_List_List* y); extern unsigned char Cyc_List_Nth[ 8u]; struct Cyc_Lineno_Pos{
struct _tagged_arr logical_file; struct _tagged_arr line; int line_no; int col;
} ; extern unsigned char Cyc_Position_Exit[ 9u]; struct Cyc_Position_Segment;
static const int Cyc_Position_Lex= 0; static const int Cyc_Position_Parse= 1;
static const int Cyc_Position_Elab= 2; struct Cyc_Position_Error{ struct
_tagged_arr source; struct Cyc_Position_Segment* seg; void* kind; struct
_tagged_arr desc; } ; extern unsigned char Cyc_Position_Nocontext[ 14u]; struct
_tuple0{ void* f1; struct _tagged_arr* f2; } ; struct Cyc_Absyn_Tvar; struct Cyc_Absyn_Tqual;
struct Cyc_Absyn_Conref; struct Cyc_Absyn_PtrInfo; struct Cyc_Absyn_VarargInfo;
struct Cyc_Absyn_FnInfo; struct Cyc_Absyn_TunionInfo; struct Cyc_Absyn_TunionFieldInfo;
struct Cyc_Absyn_VarargCallInfo; struct Cyc_Absyn_Exp; struct Cyc_Absyn_Stmt;
struct Cyc_Absyn_Pat; struct Cyc_Absyn_Switch_clause; struct Cyc_Absyn_SwitchC_clause;
struct Cyc_Absyn_Fndecl; struct Cyc_Absyn_Structdecl; struct Cyc_Absyn_Uniondecl;
struct Cyc_Absyn_Tuniondecl; struct Cyc_Absyn_Tunionfield; struct Cyc_Absyn_Enumfield;
struct Cyc_Absyn_Enumdecl; struct Cyc_Absyn_Typedefdecl; struct Cyc_Absyn_Vardecl;
struct Cyc_Absyn_Decl; struct Cyc_Absyn_Structfield; static const int Cyc_Absyn_Loc_n=
0; static const int Cyc_Absyn_Rel_n= 0; struct Cyc_Absyn_Rel_n_struct{ int tag;
struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_Abs_n= 1; struct Cyc_Absyn_Abs_n_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_Static= 0;
static const int Cyc_Absyn_Abstract= 1; static const int Cyc_Absyn_Public= 2;
static const int Cyc_Absyn_Extern= 3; static const int Cyc_Absyn_ExternC= 4;
struct Cyc_Absyn_Tqual{ int q_const: 1; int q_volatile: 1; int q_restrict: 1; }
; static const int Cyc_Absyn_B1= 0; static const int Cyc_Absyn_B2= 1; static
const int Cyc_Absyn_B4= 2; static const int Cyc_Absyn_B8= 3; static const int
Cyc_Absyn_AnyKind= 0; static const int Cyc_Absyn_MemKind= 1; static const int
Cyc_Absyn_BoxKind= 2; static const int Cyc_Absyn_RgnKind= 3; static const int
Cyc_Absyn_EffKind= 4; static const int Cyc_Absyn_Signed= 0; static const int Cyc_Absyn_Unsigned=
1; struct Cyc_Absyn_Conref{ void* v; } ; static const int Cyc_Absyn_Eq_constr= 0;
struct Cyc_Absyn_Eq_constr_struct{ int tag; void* f1; } ; static const int Cyc_Absyn_Forward_constr=
1; struct Cyc_Absyn_Forward_constr_struct{ int tag; struct Cyc_Absyn_Conref* f1;
} ; static const int Cyc_Absyn_No_constr= 0; struct Cyc_Absyn_Tvar{ struct
_tagged_arr* name; int* identity; struct Cyc_Absyn_Conref* kind; } ; static
const int Cyc_Absyn_Unknown_b= 0; static const int Cyc_Absyn_Upper_b= 0; struct
Cyc_Absyn_Upper_b_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ; struct Cyc_Absyn_PtrInfo{
void* elt_typ; void* rgn_typ; struct Cyc_Absyn_Conref* nullable; struct Cyc_Absyn_Tqual
tq; struct Cyc_Absyn_Conref* bounds; } ; struct Cyc_Absyn_VarargInfo{ struct Cyc_Core_Opt*
name; struct Cyc_Absyn_Tqual tq; void* type; void* rgn; int inject; } ; struct
Cyc_Absyn_FnInfo{ struct Cyc_List_List* tvars; struct Cyc_Core_Opt* effect; void*
ret_typ; struct Cyc_List_List* args; int c_varargs; struct Cyc_Absyn_VarargInfo*
cyc_varargs; struct Cyc_List_List* rgn_po; struct Cyc_List_List* attributes; } ;
struct Cyc_Absyn_UnknownTunionInfo{ struct _tuple0* name; int is_xtunion; } ;
static const int Cyc_Absyn_UnknownTunion= 0; struct Cyc_Absyn_UnknownTunion_struct{
int tag; struct Cyc_Absyn_UnknownTunionInfo f1; } ; static const int Cyc_Absyn_KnownTunion=
1; struct Cyc_Absyn_KnownTunion_struct{ int tag; struct Cyc_Absyn_Tuniondecl* f1;
} ; struct Cyc_Absyn_TunionInfo{ void* tunion_info; struct Cyc_List_List* targs;
void* rgn; } ; struct Cyc_Absyn_UnknownTunionFieldInfo{ struct _tuple0*
tunion_name; struct _tuple0* field_name; int is_xtunion; } ; static const int
Cyc_Absyn_UnknownTunionfield= 0; struct Cyc_Absyn_UnknownTunionfield_struct{ int
tag; struct Cyc_Absyn_UnknownTunionFieldInfo f1; } ; static const int Cyc_Absyn_KnownTunionfield=
1; struct Cyc_Absyn_KnownTunionfield_struct{ int tag; struct Cyc_Absyn_Tuniondecl*
f1; struct Cyc_Absyn_Tunionfield* f2; } ; struct Cyc_Absyn_TunionFieldInfo{ void*
field_info; struct Cyc_List_List* targs; } ; static const int Cyc_Absyn_VoidType=
0; static const int Cyc_Absyn_Evar= 0; struct Cyc_Absyn_Evar_struct{ int tag;
struct Cyc_Core_Opt* f1; struct Cyc_Core_Opt* f2; int f3; struct Cyc_Core_Opt*
f4; } ; static const int Cyc_Absyn_VarType= 1; struct Cyc_Absyn_VarType_struct{
int tag; struct Cyc_Absyn_Tvar* f1; } ; static const int Cyc_Absyn_TunionType= 2;
struct Cyc_Absyn_TunionType_struct{ int tag; struct Cyc_Absyn_TunionInfo f1; } ;
static const int Cyc_Absyn_TunionFieldType= 3; struct Cyc_Absyn_TunionFieldType_struct{
int tag; struct Cyc_Absyn_TunionFieldInfo f1; } ; static const int Cyc_Absyn_PointerType=
4; struct Cyc_Absyn_PointerType_struct{ int tag; struct Cyc_Absyn_PtrInfo f1; }
; static const int Cyc_Absyn_IntType= 5; struct Cyc_Absyn_IntType_struct{ int
tag; void* f1; void* f2; } ; static const int Cyc_Absyn_FloatType= 1; static
const int Cyc_Absyn_DoubleType= 2; static const int Cyc_Absyn_ArrayType= 6;
struct Cyc_Absyn_ArrayType_struct{ int tag; void* f1; struct Cyc_Absyn_Tqual f2;
struct Cyc_Absyn_Exp* f3; } ; static const int Cyc_Absyn_FnType= 7; struct Cyc_Absyn_FnType_struct{
int tag; struct Cyc_Absyn_FnInfo f1; } ; static const int Cyc_Absyn_TupleType= 8;
struct Cyc_Absyn_TupleType_struct{ int tag; struct Cyc_List_List* f1; } ; static
const int Cyc_Absyn_StructType= 9; struct Cyc_Absyn_StructType_struct{ int tag;
struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_Absyn_Structdecl** f3;
} ; static const int Cyc_Absyn_UnionType= 10; struct Cyc_Absyn_UnionType_struct{
int tag; struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_Absyn_Uniondecl**
f3; } ; static const int Cyc_Absyn_AnonStructType= 11; struct Cyc_Absyn_AnonStructType_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_AnonUnionType=
12; struct Cyc_Absyn_AnonUnionType_struct{ int tag; struct Cyc_List_List* f1; }
; static const int Cyc_Absyn_EnumType= 13; struct Cyc_Absyn_EnumType_struct{ int
tag; struct _tuple0* f1; struct Cyc_Absyn_Enumdecl* f2; } ; static const int Cyc_Absyn_RgnHandleType=
14; struct Cyc_Absyn_RgnHandleType_struct{ int tag; void* f1; } ; static const
int Cyc_Absyn_TypedefType= 15; struct Cyc_Absyn_TypedefType_struct{ int tag;
struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_Core_Opt* f3; } ;
static const int Cyc_Absyn_HeapRgn= 3; static const int Cyc_Absyn_AccessEff= 16;
struct Cyc_Absyn_AccessEff_struct{ int tag; void* f1; } ; static const int Cyc_Absyn_JoinEff=
17; struct Cyc_Absyn_JoinEff_struct{ int tag; struct Cyc_List_List* f1; } ;
static const int Cyc_Absyn_NoTypes= 0; struct Cyc_Absyn_NoTypes_struct{ int tag;
struct Cyc_List_List* f1; struct Cyc_Position_Segment* f2; } ; static const int
Cyc_Absyn_WithTypes= 1; struct Cyc_Absyn_WithTypes_struct{ int tag; struct Cyc_List_List*
f1; int f2; struct Cyc_Absyn_VarargInfo* f3; struct Cyc_Core_Opt* f4; struct Cyc_List_List*
f5; } ; static const int Cyc_Absyn_NonNullable_ps= 0; struct Cyc_Absyn_NonNullable_ps_struct{
int tag; struct Cyc_Absyn_Exp* f1; } ; static const int Cyc_Absyn_Nullable_ps= 1;
struct Cyc_Absyn_Nullable_ps_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ;
static const int Cyc_Absyn_TaggedArray_ps= 0; static const int Cyc_Absyn_Printf_ft=
0; static const int Cyc_Absyn_Scanf_ft= 1; static const int Cyc_Absyn_Regparm_att=
0; struct Cyc_Absyn_Regparm_att_struct{ int tag; int f1; } ; static const int
Cyc_Absyn_Stdcall_att= 0; static const int Cyc_Absyn_Cdecl_att= 1; static const
int Cyc_Absyn_Fastcall_att= 2; static const int Cyc_Absyn_Noreturn_att= 3;
static const int Cyc_Absyn_Const_att= 4; static const int Cyc_Absyn_Aligned_att=
1; struct Cyc_Absyn_Aligned_att_struct{ int tag; int f1; } ; static const int
Cyc_Absyn_Packed_att= 5; static const int Cyc_Absyn_Section_att= 2; struct Cyc_Absyn_Section_att_struct{
int tag; struct _tagged_arr f1; } ; static const int Cyc_Absyn_Nocommon_att= 6;
static const int Cyc_Absyn_Shared_att= 7; static const int Cyc_Absyn_Unused_att=
8; static const int Cyc_Absyn_Weak_att= 9; static const int Cyc_Absyn_Dllimport_att=
10; static const int Cyc_Absyn_Dllexport_att= 11; static const int Cyc_Absyn_No_instrument_function_att=
12; static const int Cyc_Absyn_Constructor_att= 13; static const int Cyc_Absyn_Destructor_att=
14; static const int Cyc_Absyn_No_check_memory_usage_att= 15; static const int
Cyc_Absyn_Format_att= 3; struct Cyc_Absyn_Format_att_struct{ int tag; void* f1;
int f2; int f3; } ; static const int Cyc_Absyn_Carray_mod= 0; static const int
Cyc_Absyn_ConstArray_mod= 0; struct Cyc_Absyn_ConstArray_mod_struct{ int tag;
struct Cyc_Absyn_Exp* f1; } ; static const int Cyc_Absyn_Pointer_mod= 1; struct
Cyc_Absyn_Pointer_mod_struct{ int tag; void* f1; void* f2; struct Cyc_Absyn_Tqual
f3; } ; static const int Cyc_Absyn_Function_mod= 2; struct Cyc_Absyn_Function_mod_struct{
int tag; void* f1; } ; static const int Cyc_Absyn_TypeParams_mod= 3; struct Cyc_Absyn_TypeParams_mod_struct{
int tag; struct Cyc_List_List* f1; struct Cyc_Position_Segment* f2; int f3; } ;
static const int Cyc_Absyn_Attributes_mod= 4; struct Cyc_Absyn_Attributes_mod_struct{
int tag; struct Cyc_Position_Segment* f1; struct Cyc_List_List* f2; } ; static
const int Cyc_Absyn_Char_c= 0; struct Cyc_Absyn_Char_c_struct{ int tag; void* f1;
unsigned char f2; } ; static const int Cyc_Absyn_Short_c= 1; struct Cyc_Absyn_Short_c_struct{
int tag; void* f1; short f2; } ; static const int Cyc_Absyn_Int_c= 2; struct Cyc_Absyn_Int_c_struct{
int tag; void* f1; int f2; } ; static const int Cyc_Absyn_LongLong_c= 3; struct
Cyc_Absyn_LongLong_c_struct{ int tag; void* f1; long long f2; } ; static const
int Cyc_Absyn_Float_c= 4; struct Cyc_Absyn_Float_c_struct{ int tag; struct
_tagged_arr f1; } ; static const int Cyc_Absyn_String_c= 5; struct Cyc_Absyn_String_c_struct{
int tag; struct _tagged_arr f1; } ; static const int Cyc_Absyn_Null_c= 0; static
const int Cyc_Absyn_Plus= 0; static const int Cyc_Absyn_Times= 1; static const
int Cyc_Absyn_Minus= 2; static const int Cyc_Absyn_Div= 3; static const int Cyc_Absyn_Mod=
4; static const int Cyc_Absyn_Eq= 5; static const int Cyc_Absyn_Neq= 6; static
const int Cyc_Absyn_Gt= 7; static const int Cyc_Absyn_Lt= 8; static const int
Cyc_Absyn_Gte= 9; static const int Cyc_Absyn_Lte= 10; static const int Cyc_Absyn_Not=
11; static const int Cyc_Absyn_Bitnot= 12; static const int Cyc_Absyn_Bitand= 13;
static const int Cyc_Absyn_Bitor= 14; static const int Cyc_Absyn_Bitxor= 15;
static const int Cyc_Absyn_Bitlshift= 16; static const int Cyc_Absyn_Bitlrshift=
17; static const int Cyc_Absyn_Bitarshift= 18; static const int Cyc_Absyn_Size=
19; static const int Cyc_Absyn_PreInc= 0; static const int Cyc_Absyn_PostInc= 1;
static const int Cyc_Absyn_PreDec= 2; static const int Cyc_Absyn_PostDec= 3;
struct Cyc_Absyn_VarargCallInfo{ int num_varargs; struct Cyc_List_List*
injectors; struct Cyc_Absyn_VarargInfo* vai; } ; static const int Cyc_Absyn_Const_e=
0; struct Cyc_Absyn_Const_e_struct{ int tag; void* f1; } ; static const int Cyc_Absyn_Var_e=
1; struct Cyc_Absyn_Var_e_struct{ int tag; struct _tuple0* f1; void* f2; } ;
static const int Cyc_Absyn_UnknownId_e= 2; struct Cyc_Absyn_UnknownId_e_struct{
int tag; struct _tuple0* f1; } ; static const int Cyc_Absyn_Primop_e= 3; struct
Cyc_Absyn_Primop_e_struct{ int tag; void* f1; struct Cyc_List_List* f2; } ;
static const int Cyc_Absyn_AssignOp_e= 4; struct Cyc_Absyn_AssignOp_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_Core_Opt* f2; struct Cyc_Absyn_Exp*
f3; } ; static const int Cyc_Absyn_Increment_e= 5; struct Cyc_Absyn_Increment_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; void* f2; } ; static const int Cyc_Absyn_Conditional_e=
6; struct Cyc_Absyn_Conditional_e_struct{ int tag; struct Cyc_Absyn_Exp* f1;
struct Cyc_Absyn_Exp* f2; struct Cyc_Absyn_Exp* f3; } ; static const int Cyc_Absyn_SeqExp_e=
7; struct Cyc_Absyn_SeqExp_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; struct
Cyc_Absyn_Exp* f2; } ; static const int Cyc_Absyn_UnknownCall_e= 8; struct Cyc_Absyn_UnknownCall_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_List_List* f2; } ; static const
int Cyc_Absyn_FnCall_e= 9; struct Cyc_Absyn_FnCall_e_struct{ int tag; struct Cyc_Absyn_Exp*
f1; struct Cyc_List_List* f2; struct Cyc_Absyn_VarargCallInfo* f3; } ; static
const int Cyc_Absyn_Throw_e= 10; struct Cyc_Absyn_Throw_e_struct{ int tag;
struct Cyc_Absyn_Exp* f1; } ; static const int Cyc_Absyn_NoInstantiate_e= 11;
struct Cyc_Absyn_NoInstantiate_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ;
static const int Cyc_Absyn_Instantiate_e= 12; struct Cyc_Absyn_Instantiate_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_List_List* f2; } ; static const
int Cyc_Absyn_Cast_e= 13; struct Cyc_Absyn_Cast_e_struct{ int tag; void* f1;
struct Cyc_Absyn_Exp* f2; } ; static const int Cyc_Absyn_Address_e= 14; struct
Cyc_Absyn_Address_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ; static const
int Cyc_Absyn_New_e= 15; struct Cyc_Absyn_New_e_struct{ int tag; struct Cyc_Absyn_Exp*
f1; struct Cyc_Absyn_Exp* f2; } ; static const int Cyc_Absyn_Sizeoftyp_e= 16;
struct Cyc_Absyn_Sizeoftyp_e_struct{ int tag; void* f1; } ; static const int Cyc_Absyn_Sizeofexp_e=
17; struct Cyc_Absyn_Sizeofexp_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ;
static const int Cyc_Absyn_Offsetof_e= 18; struct Cyc_Absyn_Offsetof_e_struct{
int tag; void* f1; struct _tagged_arr* f2; } ; static const int Cyc_Absyn_Deref_e=
19; struct Cyc_Absyn_Deref_e_struct{ int tag; struct Cyc_Absyn_Exp* f1; } ;
static const int Cyc_Absyn_StructMember_e= 20; struct Cyc_Absyn_StructMember_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct _tagged_arr* f2; } ; static const int
Cyc_Absyn_StructArrow_e= 21; struct Cyc_Absyn_StructArrow_e_struct{ int tag;
struct Cyc_Absyn_Exp* f1; struct _tagged_arr* f2; } ; static const int Cyc_Absyn_Subscript_e=
22; struct Cyc_Absyn_Subscript_e_struct{ int tag; struct Cyc_Absyn_Exp* f1;
struct Cyc_Absyn_Exp* f2; } ; static const int Cyc_Absyn_Tuple_e= 23; struct Cyc_Absyn_Tuple_e_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_CompoundLit_e=
24; struct _tuple1{ struct Cyc_Core_Opt* f1; struct Cyc_Absyn_Tqual f2; void* f3;
} ; struct Cyc_Absyn_CompoundLit_e_struct{ int tag; struct _tuple1* f1; struct
Cyc_List_List* f2; } ; static const int Cyc_Absyn_Array_e= 25; struct Cyc_Absyn_Array_e_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_Comprehension_e=
26; struct Cyc_Absyn_Comprehension_e_struct{ int tag; struct Cyc_Absyn_Vardecl*
f1; struct Cyc_Absyn_Exp* f2; struct Cyc_Absyn_Exp* f3; } ; static const int Cyc_Absyn_Struct_e=
27; struct Cyc_Absyn_Struct_e_struct{ int tag; struct _tuple0* f1; struct Cyc_Core_Opt*
f2; struct Cyc_List_List* f3; struct Cyc_Absyn_Structdecl* f4; } ; static const
int Cyc_Absyn_AnonStruct_e= 28; struct Cyc_Absyn_AnonStruct_e_struct{ int tag;
void* f1; struct Cyc_List_List* f2; } ; static const int Cyc_Absyn_Tunion_e= 29;
struct Cyc_Absyn_Tunion_e_struct{ int tag; struct Cyc_Core_Opt* f1; struct Cyc_Core_Opt*
f2; struct Cyc_List_List* f3; struct Cyc_Absyn_Tuniondecl* f4; struct Cyc_Absyn_Tunionfield*
f5; } ; static const int Cyc_Absyn_Enum_e= 30; struct Cyc_Absyn_Enum_e_struct{
int tag; struct _tuple0* f1; struct Cyc_Absyn_Enumdecl* f2; struct Cyc_Absyn_Enumfield*
f3; } ; static const int Cyc_Absyn_Malloc_e= 31; struct Cyc_Absyn_Malloc_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; void* f2; } ; static const int Cyc_Absyn_UnresolvedMem_e=
32; struct Cyc_Absyn_UnresolvedMem_e_struct{ int tag; struct Cyc_Core_Opt* f1;
struct Cyc_List_List* f2; } ; static const int Cyc_Absyn_StmtExp_e= 33; struct
Cyc_Absyn_StmtExp_e_struct{ int tag; struct Cyc_Absyn_Stmt* f1; } ; static const
int Cyc_Absyn_Codegen_e= 34; struct Cyc_Absyn_Codegen_e_struct{ int tag; struct
Cyc_Absyn_Fndecl* f1; } ; static const int Cyc_Absyn_Fill_e= 35; struct Cyc_Absyn_Fill_e_struct{
int tag; struct Cyc_Absyn_Exp* f1; } ; struct Cyc_Absyn_Exp{ struct Cyc_Core_Opt*
topt; void* r; struct Cyc_Position_Segment* loc; } ; static const int Cyc_Absyn_Skip_s=
0; static const int Cyc_Absyn_Exp_s= 0; struct Cyc_Absyn_Exp_s_struct{ int tag;
struct Cyc_Absyn_Exp* f1; } ; static const int Cyc_Absyn_Seq_s= 1; struct Cyc_Absyn_Seq_s_struct{
int tag; struct Cyc_Absyn_Stmt* f1; struct Cyc_Absyn_Stmt* f2; } ; static const
int Cyc_Absyn_Return_s= 2; struct Cyc_Absyn_Return_s_struct{ int tag; struct Cyc_Absyn_Exp*
f1; } ; static const int Cyc_Absyn_IfThenElse_s= 3; struct Cyc_Absyn_IfThenElse_s_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_Absyn_Stmt* f2; struct Cyc_Absyn_Stmt*
f3; } ; static const int Cyc_Absyn_While_s= 4; struct _tuple2{ struct Cyc_Absyn_Exp*
f1; struct Cyc_Absyn_Stmt* f2; } ; struct Cyc_Absyn_While_s_struct{ int tag;
struct _tuple2 f1; struct Cyc_Absyn_Stmt* f2; } ; static const int Cyc_Absyn_Break_s=
5; struct Cyc_Absyn_Break_s_struct{ int tag; struct Cyc_Absyn_Stmt* f1; } ;
static const int Cyc_Absyn_Continue_s= 6; struct Cyc_Absyn_Continue_s_struct{
int tag; struct Cyc_Absyn_Stmt* f1; } ; static const int Cyc_Absyn_Goto_s= 7;
struct Cyc_Absyn_Goto_s_struct{ int tag; struct _tagged_arr* f1; struct Cyc_Absyn_Stmt*
f2; } ; static const int Cyc_Absyn_For_s= 8; struct Cyc_Absyn_For_s_struct{ int
tag; struct Cyc_Absyn_Exp* f1; struct _tuple2 f2; struct _tuple2 f3; struct Cyc_Absyn_Stmt*
f4; } ; static const int Cyc_Absyn_Switch_s= 9; struct Cyc_Absyn_Switch_s_struct{
int tag; struct Cyc_Absyn_Exp* f1; struct Cyc_List_List* f2; } ; static const
int Cyc_Absyn_SwitchC_s= 10; struct Cyc_Absyn_SwitchC_s_struct{ int tag; struct
Cyc_Absyn_Exp* f1; struct Cyc_List_List* f2; } ; static const int Cyc_Absyn_Fallthru_s=
11; struct Cyc_Absyn_Fallthru_s_struct{ int tag; struct Cyc_List_List* f1;
struct Cyc_Absyn_Switch_clause** f2; } ; static const int Cyc_Absyn_Decl_s= 12;
struct Cyc_Absyn_Decl_s_struct{ int tag; struct Cyc_Absyn_Decl* f1; struct Cyc_Absyn_Stmt*
f2; } ; static const int Cyc_Absyn_Cut_s= 13; struct Cyc_Absyn_Cut_s_struct{ int
tag; struct Cyc_Absyn_Stmt* f1; } ; static const int Cyc_Absyn_Splice_s= 14;
struct Cyc_Absyn_Splice_s_struct{ int tag; struct Cyc_Absyn_Stmt* f1; } ; static
const int Cyc_Absyn_Label_s= 15; struct Cyc_Absyn_Label_s_struct{ int tag;
struct _tagged_arr* f1; struct Cyc_Absyn_Stmt* f2; } ; static const int Cyc_Absyn_Do_s=
16; struct Cyc_Absyn_Do_s_struct{ int tag; struct Cyc_Absyn_Stmt* f1; struct
_tuple2 f2; } ; static const int Cyc_Absyn_TryCatch_s= 17; struct Cyc_Absyn_TryCatch_s_struct{
int tag; struct Cyc_Absyn_Stmt* f1; struct Cyc_List_List* f2; } ; static const
int Cyc_Absyn_Region_s= 18; struct Cyc_Absyn_Region_s_struct{ int tag; struct
Cyc_Absyn_Tvar* f1; struct Cyc_Absyn_Vardecl* f2; struct Cyc_Absyn_Stmt* f3; } ;
struct Cyc_Absyn_Stmt{ void* r; struct Cyc_Position_Segment* loc; struct Cyc_List_List*
non_local_preds; int try_depth; void* annot; } ; static const int Cyc_Absyn_Wild_p=
0; static const int Cyc_Absyn_Var_p= 0; struct Cyc_Absyn_Var_p_struct{ int tag;
struct Cyc_Absyn_Vardecl* f1; } ; static const int Cyc_Absyn_Null_p= 1; static
const int Cyc_Absyn_Int_p= 1; struct Cyc_Absyn_Int_p_struct{ int tag; void* f1;
int f2; } ; static const int Cyc_Absyn_Char_p= 2; struct Cyc_Absyn_Char_p_struct{
int tag; unsigned char f1; } ; static const int Cyc_Absyn_Float_p= 3; struct Cyc_Absyn_Float_p_struct{
int tag; struct _tagged_arr f1; } ; static const int Cyc_Absyn_Tuple_p= 4;
struct Cyc_Absyn_Tuple_p_struct{ int tag; struct Cyc_List_List* f1; } ; static
const int Cyc_Absyn_Pointer_p= 5; struct Cyc_Absyn_Pointer_p_struct{ int tag;
struct Cyc_Absyn_Pat* f1; } ; static const int Cyc_Absyn_Reference_p= 6; struct
Cyc_Absyn_Reference_p_struct{ int tag; struct Cyc_Absyn_Vardecl* f1; } ; static
const int Cyc_Absyn_Struct_p= 7; struct Cyc_Absyn_Struct_p_struct{ int tag;
struct Cyc_Absyn_Structdecl* f1; struct Cyc_Core_Opt* f2; struct Cyc_List_List*
f3; struct Cyc_List_List* f4; } ; static const int Cyc_Absyn_Tunion_p= 8; struct
Cyc_Absyn_Tunion_p_struct{ int tag; struct Cyc_Absyn_Tuniondecl* f1; struct Cyc_Absyn_Tunionfield*
f2; struct Cyc_List_List* f3; struct Cyc_List_List* f4; } ; static const int Cyc_Absyn_Enum_p=
9; struct Cyc_Absyn_Enum_p_struct{ int tag; struct Cyc_Absyn_Enumdecl* f1;
struct Cyc_Absyn_Enumfield* f2; } ; static const int Cyc_Absyn_UnknownId_p= 10;
struct Cyc_Absyn_UnknownId_p_struct{ int tag; struct _tuple0* f1; } ; static
const int Cyc_Absyn_UnknownCall_p= 11; struct Cyc_Absyn_UnknownCall_p_struct{
int tag; struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_List_List* f3;
} ; static const int Cyc_Absyn_UnknownFields_p= 12; struct Cyc_Absyn_UnknownFields_p_struct{
int tag; struct _tuple0* f1; struct Cyc_List_List* f2; struct Cyc_List_List* f3;
} ; struct Cyc_Absyn_Pat{ void* r; struct Cyc_Core_Opt* topt; struct Cyc_Position_Segment*
loc; } ; struct Cyc_Absyn_Switch_clause{ struct Cyc_Absyn_Pat* pattern; struct
Cyc_Core_Opt* pat_vars; struct Cyc_Absyn_Exp* where_clause; struct Cyc_Absyn_Stmt*
body; struct Cyc_Position_Segment* loc; } ; struct Cyc_Absyn_SwitchC_clause{
struct Cyc_Absyn_Exp* cnst_exp; struct Cyc_Absyn_Stmt* body; struct Cyc_Position_Segment*
loc; } ; static const int Cyc_Absyn_Unresolved_b= 0; static const int Cyc_Absyn_Global_b=
0; struct Cyc_Absyn_Global_b_struct{ int tag; struct Cyc_Absyn_Vardecl* f1; } ;
static const int Cyc_Absyn_Funname_b= 1; struct Cyc_Absyn_Funname_b_struct{ int
tag; struct Cyc_Absyn_Fndecl* f1; } ; static const int Cyc_Absyn_Param_b= 2;
struct Cyc_Absyn_Param_b_struct{ int tag; struct Cyc_Absyn_Vardecl* f1; } ;
static const int Cyc_Absyn_Local_b= 3; struct Cyc_Absyn_Local_b_struct{ int tag;
struct Cyc_Absyn_Vardecl* f1; } ; static const int Cyc_Absyn_Pat_b= 4; struct
Cyc_Absyn_Pat_b_struct{ int tag; struct Cyc_Absyn_Vardecl* f1; } ; struct Cyc_Absyn_Vardecl{
void* sc; struct _tuple0* name; struct Cyc_Absyn_Tqual tq; void* type; struct
Cyc_Absyn_Exp* initializer; struct Cyc_Core_Opt* rgn; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Fndecl{ void* sc; int is_inline; struct _tuple0*
name; struct Cyc_List_List* tvs; struct Cyc_Core_Opt* effect; void* ret_type;
struct Cyc_List_List* args; int c_varargs; struct Cyc_Absyn_VarargInfo*
cyc_varargs; struct Cyc_List_List* rgn_po; struct Cyc_Absyn_Stmt* body; struct
Cyc_Core_Opt* cached_typ; struct Cyc_Core_Opt* param_vardecls; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Structfield{ struct _tagged_arr* name; struct
Cyc_Absyn_Tqual tq; void* type; struct Cyc_Core_Opt* width; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Structdecl{ void* sc; struct Cyc_Core_Opt* name;
struct Cyc_List_List* tvs; struct Cyc_Core_Opt* fields; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Uniondecl{ void* sc; struct Cyc_Core_Opt* name;
struct Cyc_List_List* tvs; struct Cyc_Core_Opt* fields; struct Cyc_List_List*
attributes; } ; struct Cyc_Absyn_Tunionfield{ struct _tuple0* name; struct Cyc_List_List*
tvs; struct Cyc_List_List* typs; struct Cyc_Position_Segment* loc; void* sc; } ;
struct Cyc_Absyn_Tuniondecl{ void* sc; struct _tuple0* name; struct Cyc_List_List*
tvs; struct Cyc_Core_Opt* fields; int is_xtunion; } ; struct Cyc_Absyn_Enumfield{
struct _tuple0* name; struct Cyc_Absyn_Exp* tag; struct Cyc_Position_Segment*
loc; } ; struct Cyc_Absyn_Enumdecl{ void* sc; struct _tuple0* name; struct Cyc_Core_Opt*
fields; } ; struct Cyc_Absyn_Typedefdecl{ struct _tuple0* name; struct Cyc_List_List*
tvs; void* defn; } ; static const int Cyc_Absyn_Var_d= 0; struct Cyc_Absyn_Var_d_struct{
int tag; struct Cyc_Absyn_Vardecl* f1; } ; static const int Cyc_Absyn_Fn_d= 1;
struct Cyc_Absyn_Fn_d_struct{ int tag; struct Cyc_Absyn_Fndecl* f1; } ; static
const int Cyc_Absyn_Let_d= 2; struct Cyc_Absyn_Let_d_struct{ int tag; struct Cyc_Absyn_Pat*
f1; struct Cyc_Core_Opt* f2; struct Cyc_Core_Opt* f3; struct Cyc_Absyn_Exp* f4;
int f5; } ; static const int Cyc_Absyn_Letv_d= 3; struct Cyc_Absyn_Letv_d_struct{
int tag; struct Cyc_List_List* f1; } ; static const int Cyc_Absyn_Struct_d= 4;
struct Cyc_Absyn_Struct_d_struct{ int tag; struct Cyc_Absyn_Structdecl* f1; } ;
static const int Cyc_Absyn_Union_d= 5; struct Cyc_Absyn_Union_d_struct{ int tag;
struct Cyc_Absyn_Uniondecl* f1; } ; static const int Cyc_Absyn_Tunion_d= 6;
struct Cyc_Absyn_Tunion_d_struct{ int tag; struct Cyc_Absyn_Tuniondecl* f1; } ;
static const int Cyc_Absyn_Enum_d= 7; struct Cyc_Absyn_Enum_d_struct{ int tag;
struct Cyc_Absyn_Enumdecl* f1; } ; static const int Cyc_Absyn_Typedef_d= 8;
struct Cyc_Absyn_Typedef_d_struct{ int tag; struct Cyc_Absyn_Typedefdecl* f1; }
; static const int Cyc_Absyn_Namespace_d= 9; struct Cyc_Absyn_Namespace_d_struct{
int tag; struct _tagged_arr* f1; struct Cyc_List_List* f2; } ; static const int
Cyc_Absyn_Using_d= 10; struct Cyc_Absyn_Using_d_struct{ int tag; struct _tuple0*
f1; struct Cyc_List_List* f2; } ; static const int Cyc_Absyn_ExternC_d= 11;
struct Cyc_Absyn_ExternC_d_struct{ int tag; struct Cyc_List_List* f1; } ; struct
Cyc_Absyn_Decl{ void* r; struct Cyc_Position_Segment* loc; } ; static const int
Cyc_Absyn_ArrayElement= 0; struct Cyc_Absyn_ArrayElement_struct{ int tag; struct
Cyc_Absyn_Exp* f1; } ; static const int Cyc_Absyn_FieldName= 1; struct Cyc_Absyn_FieldName_struct{
int tag; struct _tagged_arr* f1; } ; extern unsigned char Cyc_Absyn_EmptyAnnot[
15u]; extern struct Cyc_Absyn_Conref* Cyc_Absyn_compress_conref( struct Cyc_Absyn_Conref*
x); extern struct _tagged_arr Cyc_Absyn_attribute2string( void*); struct Cyc_PP_Ppstate;
struct Cyc_PP_Out; struct Cyc_PP_Doc; struct Cyc_Absynpp_Params{ int
expand_typedefs: 1; int qvar_to_Cids: 1; int add_cyc_prefix: 1; int to_VC: 1;
int decls_first: 1; int rewrite_temp_tvars: 1; int print_all_tvars: 1; int
print_all_kinds: 1; int print_using_stmts: 1; int print_externC_stmts: 1; int
print_full_evars: 1; int use_curr_namespace: 1; struct Cyc_List_List*
curr_namespace; } ; extern void Cyc_Absynpp_set_params( struct Cyc_Absynpp_Params*
fs); extern struct _tagged_arr* Cyc_Absynpp_cyc_stringptr; extern int Cyc_Absynpp_exp_prec(
struct Cyc_Absyn_Exp*); extern struct _tagged_arr Cyc_Absynpp_char_escape(
unsigned char); extern struct _tagged_arr Cyc_Absynpp_string_escape( struct
_tagged_arr); extern struct _tagged_arr Cyc_Absynpp_prim2str( void* p); extern
int Cyc_Absynpp_is_declaration( struct Cyc_Absyn_Stmt* s); struct _tuple3{
struct _tagged_arr* f1; struct Cyc_Absyn_Tqual f2; void* f3; } ; extern struct
_tuple1* Cyc_Absynpp_arg_mk_opt( struct _tuple3* arg); struct _tuple4{ struct
Cyc_Absyn_Tqual f1; void* f2; struct Cyc_List_List* f3; } ; extern struct
_tuple4 Cyc_Absynpp_to_tms( struct Cyc_Absyn_Tqual tq, void* t); extern
unsigned int Cyc_Evexp_eval_const_uint_exp( struct Cyc_Absyn_Exp* e); struct Cyc_Set_Set;
extern unsigned char Cyc_Set_Absent[ 11u]; struct Cyc_Dict_Dict; extern
unsigned char Cyc_Dict_Present[ 12u]; extern unsigned char Cyc_Dict_Absent[ 11u];
static const int Cyc_Tcenv_VarRes= 0; struct Cyc_Tcenv_VarRes_struct{ int tag;
void* f1; } ; static const int Cyc_Tcenv_StructRes= 1; struct Cyc_Tcenv_StructRes_struct{
int tag; struct Cyc_Absyn_Structdecl* f1; } ; static const int Cyc_Tcenv_TunionRes=
2; struct Cyc_Tcenv_TunionRes_struct{ int tag; struct Cyc_Absyn_Tuniondecl* f1;
struct Cyc_Absyn_Tunionfield* f2; } ; static const int Cyc_Tcenv_EnumRes= 3;
struct Cyc_Tcenv_EnumRes_struct{ int tag; struct Cyc_Absyn_Enumdecl* f1; struct
Cyc_Absyn_Enumfield* f2; } ; struct Cyc_Tcenv_Genv{ struct Cyc_Set_Set*
namespaces; struct Cyc_Dict_Dict* structdecls; struct Cyc_Dict_Dict* uniondecls;
struct Cyc_Dict_Dict* tuniondecls; struct Cyc_Dict_Dict* enumdecls; struct Cyc_Dict_Dict*
typedefs; struct Cyc_Dict_Dict* ordinaries; struct Cyc_List_List* availables; }
; struct Cyc_Tcenv_Fenv; static const int Cyc_Tcenv_NotLoop_j= 0; static const
int Cyc_Tcenv_CaseEnd_j= 1; static const int Cyc_Tcenv_FnEnd_j= 2; static const
int Cyc_Tcenv_Stmt_j= 0; struct Cyc_Tcenv_Stmt_j_struct{ int tag; struct Cyc_Absyn_Stmt*
f1; } ; static const int Cyc_Tcenv_Outermost= 0; struct Cyc_Tcenv_Outermost_struct{
int tag; void* f1; } ; static const int Cyc_Tcenv_Frame= 1; struct Cyc_Tcenv_Frame_struct{
int tag; void* f1; void* f2; } ; static const int Cyc_Tcenv_Hidden= 2; struct
Cyc_Tcenv_Hidden_struct{ int tag; void* f1; void* f2; } ; struct Cyc_Tcenv_Tenv{
struct Cyc_List_List* ns; struct Cyc_Dict_Dict* ae; struct Cyc_Core_Opt* le; } ;
extern unsigned char Cyc_Tcutil_TypeErr[ 12u]; extern void* Cyc_Tcutil_compress(
void* t); static int Cyc_Absyndump_expand_typedefs; static int Cyc_Absyndump_qvar_to_Cids;
static int Cyc_Absyndump_add_cyc_prefix; static int Cyc_Absyndump_to_VC; void
Cyc_Absyndump_set_params( struct Cyc_Absynpp_Params* fs){ Cyc_Absyndump_expand_typedefs=
fs->expand_typedefs; Cyc_Absyndump_qvar_to_Cids= fs->qvar_to_Cids; Cyc_Absyndump_add_cyc_prefix=
fs->add_cyc_prefix; Cyc_Absyndump_to_VC= fs->to_VC; Cyc_Absynpp_set_params( fs);}
void Cyc_Absyndump_dumptyp( void*); void Cyc_Absyndump_dumpntyp( void* t); void
Cyc_Absyndump_dumpexp( struct Cyc_Absyn_Exp*); void Cyc_Absyndump_dumpexp_prec(
int, struct Cyc_Absyn_Exp*); void Cyc_Absyndump_dumppat( struct Cyc_Absyn_Pat*);
void Cyc_Absyndump_dumpstmt( struct Cyc_Absyn_Stmt*); void Cyc_Absyndump_dumpdecl(
struct Cyc_Absyn_Decl*); void Cyc_Absyndump_dumptms( struct Cyc_List_List* tms,
void(* f)( void*), void* a); void Cyc_Absyndump_dumptqtd( struct Cyc_Absyn_Tqual,
void*, void(* f)( void*), void*); void Cyc_Absyndump_dumpstructfields( struct
Cyc_List_List* fields); struct Cyc_Stdio___sFILE** Cyc_Absyndump_dump_file=& Cyc_Stdio_stdout;
void Cyc_Absyndump_ignore( void* x){ return;} static unsigned int Cyc_Absyndump_pos=
0; void Cyc_Absyndump_dump( struct _tagged_arr s){ int sz=( int) _get_arr_size(
s, sizeof( unsigned char)); if( !(( int)*(( const unsigned char*)
_check_unknown_subscript( s, sizeof( unsigned char), sz - 1)))){ -- sz;} Cyc_Absyndump_pos
+= sz + 1; if( Cyc_Absyndump_pos > 80){ Cyc_Absyndump_pos=( unsigned int) sz;
Cyc_Stdio_fputc(( int)'\n',* Cyc_Absyndump_dump_file);} else{ Cyc_Stdio_fputc((
int)' ',* Cyc_Absyndump_dump_file);} Cyc_Stdio_file_string_write(* Cyc_Absyndump_dump_file,
s, 0, sz);} void Cyc_Absyndump_dump_nospace( struct _tagged_arr s){ int sz=( int)
_get_arr_size( s, sizeof( unsigned char)); if( !(( int)*(( const unsigned char*)
_check_unknown_subscript( s, sizeof( unsigned char), sz - 1)))){ -- sz;} Cyc_Absyndump_pos
+= sz; Cyc_Stdio_file_string_write(* Cyc_Absyndump_dump_file, s, 0, sz);} void
Cyc_Absyndump_dump_char( int c){ ++ Cyc_Absyndump_pos; Cyc_Stdio_fputc( c,* Cyc_Absyndump_dump_file);}
void Cyc_Absyndump_dump_str( struct _tagged_arr* s){ Cyc_Absyndump_dump(* s);}
void Cyc_Absyndump_dump_semi(){ Cyc_Absyndump_dump_char(( int)';');} void Cyc_Absyndump_dump_sep(
void(* f)( void*), struct Cyc_List_List* l, struct _tagged_arr sep){ if( l == 0){
return;} for( 0;(( struct Cyc_List_List*) _check_null( l))->tl != 0; l=(( struct
Cyc_List_List*) _check_null( l))->tl){ f(( void*)(( struct Cyc_List_List*)
_check_null( l))->hd); Cyc_Absyndump_dump_nospace( sep);} f(( void*)(( struct
Cyc_List_List*) _check_null( l))->hd);} void Cyc_Absyndump_dump_sep_c( void(* f)(
void*, void*), void* env, struct Cyc_List_List* l, struct _tagged_arr sep){ if(
l == 0){ return;} for( 0;(( struct Cyc_List_List*) _check_null( l))->tl != 0; l=((
struct Cyc_List_List*) _check_null( l))->tl){ f( env,( void*)(( struct Cyc_List_List*)
_check_null( l))->hd); Cyc_Absyndump_dump_nospace( sep);} f( env,( void*)((
struct Cyc_List_List*) _check_null( l))->hd);} void Cyc_Absyndump_group( void(*
f)( void*), struct Cyc_List_List* l, struct _tagged_arr start, struct
_tagged_arr end, struct _tagged_arr sep){ Cyc_Absyndump_dump_nospace( start);
Cyc_Absyndump_dump_sep( f, l, sep); Cyc_Absyndump_dump_nospace( end);} void Cyc_Absyndump_group_c(
void(* f)( void*, void*), void* env, struct Cyc_List_List* l, struct _tagged_arr
start, struct _tagged_arr end, struct _tagged_arr sep){ Cyc_Absyndump_dump_nospace(
start); Cyc_Absyndump_dump_sep_c( f, env, l, sep); Cyc_Absyndump_dump_nospace(
end);} void Cyc_Absyndump_egroup( void(* f)( void*), struct Cyc_List_List* l,
struct _tagged_arr start, struct _tagged_arr end, struct _tagged_arr sep){ if( l
!= 0){ Cyc_Absyndump_group( f, l, start, end, sep);}} void Cyc_Absyndump_dumpqvar(
struct _tuple0* v){ struct Cyc_List_List* _temp0= 0;{ void* _temp1=(* v).f1;
struct Cyc_List_List* _temp9; struct Cyc_List_List* _temp11; _LL3: if( _temp1 ==(
void*) Cyc_Absyn_Loc_n){ goto _LL4;} else{ goto _LL5;} _LL5: if(( unsigned int)
_temp1 > 1u?*(( int*) _temp1) == Cyc_Absyn_Rel_n: 0){ _LL10: _temp9=(( struct
Cyc_Absyn_Rel_n_struct*) _temp1)->f1; goto _LL6;} else{ goto _LL7;} _LL7: if((
unsigned int) _temp1 > 1u?*(( int*) _temp1) == Cyc_Absyn_Abs_n: 0){ _LL12:
_temp11=(( struct Cyc_Absyn_Abs_n_struct*) _temp1)->f1; goto _LL8;} else{ goto
_LL2;} _LL4: _temp9= 0; goto _LL6; _LL6: _temp0= _temp9; goto _LL2; _LL8: _temp0=(
Cyc_Absyndump_qvar_to_Cids? Cyc_Absyndump_add_cyc_prefix: 0)?({ struct Cyc_List_List*
_temp13=( struct Cyc_List_List*) GC_malloc( sizeof( struct Cyc_List_List));
_temp13->hd=( void*) Cyc_Absynpp_cyc_stringptr; _temp13->tl= _temp11; _temp13;}):
_temp11; goto _LL2; _LL2:;} if( _temp0 != 0){ Cyc_Absyndump_dump_str(( struct
_tagged_arr*)(( struct Cyc_List_List*) _check_null( _temp0))->hd); for( _temp0=((
struct Cyc_List_List*) _check_null( _temp0))->tl; _temp0 != 0; _temp0=(( struct
Cyc_List_List*) _check_null( _temp0))->tl){ if( Cyc_Absyndump_qvar_to_Cids){ Cyc_Absyndump_dump_char((
int)'_');} else{ Cyc_Absyndump_dump_nospace( _tag_arr("::", sizeof(
unsigned char), 3u));} Cyc_Absyndump_dump_nospace(*(( struct _tagged_arr*)((
struct Cyc_List_List*) _check_null( _temp0))->hd));} if( Cyc_Absyndump_qvar_to_Cids){
Cyc_Absyndump_dump_nospace( _tag_arr("_", sizeof( unsigned char), 2u));} else{
Cyc_Absyndump_dump_nospace( _tag_arr("::", sizeof( unsigned char), 3u));} Cyc_Absyndump_dump_nospace(*(*
v).f2);} else{ Cyc_Absyndump_dump_str((* v).f2);}} void Cyc_Absyndump_dumptq(
struct Cyc_Absyn_Tqual tq){ if( tq.q_restrict){ Cyc_Absyndump_dump( _tag_arr("restrict",
sizeof( unsigned char), 9u));} if( tq.q_volatile){ Cyc_Absyndump_dump( _tag_arr("volatile",
sizeof( unsigned char), 9u));} if( tq.q_const){ Cyc_Absyndump_dump( _tag_arr("const",
sizeof( unsigned char), 6u));}} void Cyc_Absyndump_dumpscope( void* sc){ void*
_temp14= sc; _LL16: if( _temp14 ==( void*) Cyc_Absyn_Static){ goto _LL17;} else{
goto _LL18;} _LL18: if( _temp14 ==( void*) Cyc_Absyn_Public){ goto _LL19;} else{
goto _LL20;} _LL20: if( _temp14 ==( void*) Cyc_Absyn_Extern){ goto _LL21;} else{
goto _LL22;} _LL22: if( _temp14 ==( void*) Cyc_Absyn_ExternC){ goto _LL23;}
else{ goto _LL24;} _LL24: if( _temp14 ==( void*) Cyc_Absyn_Abstract){ goto _LL25;}
else{ goto _LL15;} _LL17: Cyc_Absyndump_dump( _tag_arr("static", sizeof(
unsigned char), 7u)); return; _LL19: return; _LL21: Cyc_Absyndump_dump( _tag_arr("extern",
sizeof( unsigned char), 7u)); return; _LL23: Cyc_Absyndump_dump( _tag_arr("extern \"C\"",
sizeof( unsigned char), 11u)); return; _LL25: Cyc_Absyndump_dump( _tag_arr("abstract",
sizeof( unsigned char), 9u)); return; _LL15:;} void Cyc_Absyndump_dumpkind( void*
k){ void* _temp26= k; _LL28: if( _temp26 ==( void*) Cyc_Absyn_AnyKind){ goto
_LL29;} else{ goto _LL30;} _LL30: if( _temp26 ==( void*) Cyc_Absyn_MemKind){
goto _LL31;} else{ goto _LL32;} _LL32: if( _temp26 ==( void*) Cyc_Absyn_BoxKind){
goto _LL33;} else{ goto _LL34;} _LL34: if( _temp26 ==( void*) Cyc_Absyn_RgnKind){
goto _LL35;} else{ goto _LL36;} _LL36: if( _temp26 ==( void*) Cyc_Absyn_EffKind){
goto _LL37;} else{ goto _LL27;} _LL29: Cyc_Absyndump_dump( _tag_arr("A", sizeof(
unsigned char), 2u)); return; _LL31: Cyc_Absyndump_dump( _tag_arr("M", sizeof(
unsigned char), 2u)); return; _LL33: Cyc_Absyndump_dump( _tag_arr("B", sizeof(
unsigned char), 2u)); return; _LL35: Cyc_Absyndump_dump( _tag_arr("R", sizeof(
unsigned char), 2u)); return; _LL37: Cyc_Absyndump_dump( _tag_arr("E", sizeof(
unsigned char), 2u)); return; _LL27:;} void Cyc_Absyndump_dumptps( struct Cyc_List_List*
ts){ Cyc_Absyndump_egroup( Cyc_Absyndump_dumptyp, ts, _tag_arr("<", sizeof(
unsigned char), 2u), _tag_arr(">", sizeof( unsigned char), 2u), _tag_arr(",",
sizeof( unsigned char), 2u));} void Cyc_Absyndump_dumptvar( struct Cyc_Absyn_Tvar*
tv){ Cyc_Absyndump_dump_str( tv->name);} void Cyc_Absyndump_dumpkindedtvar(
struct Cyc_Absyn_Tvar* tv){ Cyc_Absyndump_dump_str( tv->name);{ void* _temp38=(
void*)( Cyc_Absyn_compress_conref( tv->kind))->v; void* _temp46; void* _temp48;
_LL40: if(( unsigned int) _temp38 > 1u?*(( int*) _temp38) == Cyc_Absyn_Eq_constr:
0){ _LL47: _temp46=( void*)(( struct Cyc_Absyn_Eq_constr_struct*) _temp38)->f1;
if( _temp46 ==( void*) Cyc_Absyn_BoxKind){ goto _LL41;} else{ goto _LL42;}}
else{ goto _LL42;} _LL42: if(( unsigned int) _temp38 > 1u?*(( int*) _temp38) ==
Cyc_Absyn_Eq_constr: 0){ _LL49: _temp48=( void*)(( struct Cyc_Absyn_Eq_constr_struct*)
_temp38)->f1; goto _LL43;} else{ goto _LL44;} _LL44: goto _LL45; _LL41: goto
_LL39; _LL43: Cyc_Absyndump_dump( _tag_arr("::", sizeof( unsigned char), 3u));
Cyc_Absyndump_dumpkind( _temp48); goto _LL39; _LL45: Cyc_Absyndump_dump(
_tag_arr("::?", sizeof( unsigned char), 4u)); goto _LL39; _LL39:;}} void Cyc_Absyndump_dumptvars(
struct Cyc_List_List* tvs){(( void(*)( void(* f)( struct Cyc_Absyn_Tvar*),
struct Cyc_List_List* l, struct _tagged_arr start, struct _tagged_arr end,
struct _tagged_arr sep)) Cyc_Absyndump_egroup)( Cyc_Absyndump_dumptvar, tvs,
_tag_arr("<", sizeof( unsigned char), 2u), _tag_arr(">", sizeof( unsigned char),
2u), _tag_arr(",", sizeof( unsigned char), 2u));} void Cyc_Absyndump_dumpkindedtvars(
struct Cyc_List_List* tvs){(( void(*)( void(* f)( struct Cyc_Absyn_Tvar*),
struct Cyc_List_List* l, struct _tagged_arr start, struct _tagged_arr end,
struct _tagged_arr sep)) Cyc_Absyndump_egroup)( Cyc_Absyndump_dumpkindedtvar,
tvs, _tag_arr("<", sizeof( unsigned char), 2u), _tag_arr(">", sizeof(
unsigned char), 2u), _tag_arr(",", sizeof( unsigned char), 2u));} struct _tuple5{
struct Cyc_Absyn_Tqual f1; void* f2; } ; void Cyc_Absyndump_dumparg( struct
_tuple5* pr){(( void(*)( struct Cyc_Absyn_Tqual, void*, void(* f)( int), int))
Cyc_Absyndump_dumptqtd)((* pr).f1,(* pr).f2,( void(*)( int x)) Cyc_Absyndump_ignore,
0);} void Cyc_Absyndump_dumpargs( struct Cyc_List_List* ts){(( void(*)( void(* f)(
struct _tuple5*), struct Cyc_List_List* l, struct _tagged_arr start, struct
_tagged_arr end, struct _tagged_arr sep)) Cyc_Absyndump_group)( Cyc_Absyndump_dumparg,
ts, _tag_arr("(", sizeof( unsigned char), 2u), _tag_arr(")", sizeof(
unsigned char), 2u), _tag_arr(",", sizeof( unsigned char), 2u));} void Cyc_Absyndump_dump_callconv(
struct Cyc_List_List* atts){ for( 0; atts != 0; atts=(( struct Cyc_List_List*)
_check_null( atts))->tl){ void* _temp50=( void*)(( struct Cyc_List_List*)
_check_null( atts))->hd; _LL52: if( _temp50 ==( void*) Cyc_Absyn_Stdcall_att){
goto _LL53;} else{ goto _LL54;} _LL54: if( _temp50 ==( void*) Cyc_Absyn_Cdecl_att){
goto _LL55;} else{ goto _LL56;} _LL56: if( _temp50 ==( void*) Cyc_Absyn_Fastcall_att){
goto _LL57;} else{ goto _LL58;} _LL58: goto _LL59; _LL53: Cyc_Absyndump_dump(
_tag_arr("_stdcall", sizeof( unsigned char), 9u)); return; _LL55: Cyc_Absyndump_dump(
_tag_arr("_cdecl", sizeof( unsigned char), 7u)); return; _LL57: Cyc_Absyndump_dump(
_tag_arr("_fastcall", sizeof( unsigned char), 10u)); return; _LL59: goto _LL51;
_LL51:;}} void Cyc_Absyndump_dump_noncallconv( struct Cyc_List_List* atts){ int
hasatt= 0;{ struct Cyc_List_List* atts2= atts; for( 0; atts2 != 0; atts2=((
struct Cyc_List_List*) _check_null( atts2))->tl){ void* _temp60=( void*)((
struct Cyc_List_List*) _check_null( atts))->hd; _LL62: if( _temp60 ==( void*)
Cyc_Absyn_Stdcall_att){ goto _LL63;} else{ goto _LL64;} _LL64: if( _temp60 ==(
void*) Cyc_Absyn_Cdecl_att){ goto _LL65;} else{ goto _LL66;} _LL66: if( _temp60
==( void*) Cyc_Absyn_Fastcall_att){ goto _LL67;} else{ goto _LL68;} _LL68: goto
_LL69; _LL63: goto _LL61; _LL65: goto _LL61; _LL67: goto _LL61; _LL69: hasatt= 1;
goto _LL61; _LL61:;}} if( ! hasatt){ return;} Cyc_Absyndump_dump( _tag_arr("__declspec(",
sizeof( unsigned char), 12u)); for( 0; atts != 0; atts=(( struct Cyc_List_List*)
_check_null( atts))->tl){ void* _temp70=( void*)(( struct Cyc_List_List*)
_check_null( atts))->hd; _LL72: if( _temp70 ==( void*) Cyc_Absyn_Stdcall_att){
goto _LL73;} else{ goto _LL74;} _LL74: if( _temp70 ==( void*) Cyc_Absyn_Cdecl_att){
goto _LL75;} else{ goto _LL76;} _LL76: if( _temp70 ==( void*) Cyc_Absyn_Fastcall_att){
goto _LL77;} else{ goto _LL78;} _LL78: goto _LL79; _LL73: goto _LL71; _LL75:
goto _LL71; _LL77: goto _LL71; _LL79: Cyc_Absyndump_dump( Cyc_Absyn_attribute2string((
void*)(( struct Cyc_List_List*) _check_null( atts))->hd)); goto _LL71; _LL71:;}
Cyc_Absyndump_dump_char(( int)')');} void Cyc_Absyndump_dumpatts( struct Cyc_List_List*
atts){ if( atts == 0){ return;} if( Cyc_Absyndump_to_VC){ Cyc_Absyndump_dump_noncallconv(
atts); return;} Cyc_Absyndump_dump( _tag_arr(" __attribute__((", sizeof(
unsigned char), 17u)); for( 0; atts != 0; atts=(( struct Cyc_List_List*)
_check_null( atts))->tl){ Cyc_Absyndump_dump( Cyc_Absyn_attribute2string(( void*)((
struct Cyc_List_List*) _check_null( atts))->hd)); if((( struct Cyc_List_List*)
_check_null( atts))->tl != 0){ Cyc_Absyndump_dump( _tag_arr(",", sizeof(
unsigned char), 2u));}} Cyc_Absyndump_dump( _tag_arr(")) ", sizeof(
unsigned char), 4u));} int Cyc_Absyndump_next_is_pointer( struct Cyc_List_List*
tms){ if( tms == 0){ return 0;}{ void* _temp80=( void*)(( struct Cyc_List_List*)
_check_null( tms))->hd; _LL82: if(( unsigned int) _temp80 > 1u?*(( int*) _temp80)
== Cyc_Absyn_Pointer_mod: 0){ goto _LL83;} else{ goto _LL84;} _LL84: goto _LL85;
_LL83: return 1; _LL85: return 0; _LL81:;}} static void Cyc_Absyndump_dumprgn(
void* t){ void* _temp86= Cyc_Tcutil_compress( t); _LL88: if( _temp86 ==( void*)
Cyc_Absyn_HeapRgn){ goto _LL89;} else{ goto _LL90;} _LL90: goto _LL91; _LL89:
Cyc_Absyndump_dump( _tag_arr("`H", sizeof( unsigned char), 3u)); goto _LL87;
_LL91: Cyc_Absyndump_dumpntyp( t); goto _LL87; _LL87:;} struct _tuple6{ struct
Cyc_List_List* f1; struct Cyc_List_List* f2; } ; static struct _tuple6 Cyc_Absyndump_effects_split(
void* t){ struct Cyc_List_List* regions= 0; struct Cyc_List_List* effects= 0;{
void* _temp92= Cyc_Tcutil_compress( t); void* _temp100; struct Cyc_List_List*
_temp102; _LL94: if(( unsigned int) _temp92 > 4u?*(( int*) _temp92) == Cyc_Absyn_AccessEff:
0){ _LL101: _temp100=( void*)(( struct Cyc_Absyn_AccessEff_struct*) _temp92)->f1;
goto _LL95;} else{ goto _LL96;} _LL96: if(( unsigned int) _temp92 > 4u?*(( int*)
_temp92) == Cyc_Absyn_JoinEff: 0){ _LL103: _temp102=(( struct Cyc_Absyn_JoinEff_struct*)
_temp92)->f1; goto _LL97;} else{ goto _LL98;} _LL98: goto _LL99; _LL95: regions=({
struct Cyc_List_List* _temp104=( struct Cyc_List_List*) GC_malloc( sizeof(
struct Cyc_List_List)); _temp104->hd=( void*) _temp100; _temp104->tl= regions;
_temp104;}); goto _LL93; _LL97: for( 0; _temp102 != 0; _temp102=(( struct Cyc_List_List*)
_check_null( _temp102))->tl){ struct Cyc_List_List* _temp107; struct Cyc_List_List*
_temp109; struct _tuple6 _temp105= Cyc_Absyndump_effects_split(( void*)(( struct
Cyc_List_List*) _check_null( _temp102))->hd); _LL110: _temp109= _temp105.f1;
goto _LL108; _LL108: _temp107= _temp105.f2; goto _LL106; _LL106: regions= Cyc_List_imp_append(
_temp109, regions); effects= Cyc_List_imp_append( _temp107, effects);} goto
_LL93; _LL99: effects=({ struct Cyc_List_List* _temp111=( struct Cyc_List_List*)
GC_malloc( sizeof( struct Cyc_List_List)); _temp111->hd=( void*) t; _temp111->tl=
effects; _temp111;}); goto _LL93; _LL93:;} return({ struct _tuple6 _temp112;
_temp112.f1= regions; _temp112.f2= effects; _temp112;});} static void Cyc_Absyndump_dumpeff(
void* t){ struct Cyc_List_List* _temp115; struct Cyc_List_List* _temp117; struct
_tuple6 _temp113= Cyc_Absyndump_effects_split( t); _LL118: _temp117= _temp113.f1;
goto _LL116; _LL116: _temp115= _temp113.f2; goto _LL114; _LL114: _temp117= Cyc_List_imp_rev(
_temp117); _temp115= Cyc_List_imp_rev( _temp115); for( 0; _temp115 != 0;
_temp115=(( struct Cyc_List_List*) _check_null( _temp115))->tl){ Cyc_Absyndump_dumpntyp((
void*)(( struct Cyc_List_List*) _check_null( _temp115))->hd); Cyc_Absyndump_dump_char((
int)'+');} Cyc_Absyndump_dump_char(( int)'{'); for( 0; _temp117 != 0; _temp117=((
struct Cyc_List_List*) _check_null( _temp117))->tl){ Cyc_Absyndump_dumprgn((
void*)(( struct Cyc_List_List*) _check_null( _temp117))->hd); if((( struct Cyc_List_List*)
_check_null( _temp117))->tl != 0){ Cyc_Absyndump_dump_char(( int)',');}} Cyc_Absyndump_dump_char((
int)'}');} void Cyc_Absyndump_dumpntyp( void* t){ void* _temp119= t; struct Cyc_Absyn_Tvar*
_temp189; int _temp191; struct Cyc_Core_Opt* _temp193; struct Cyc_Core_Opt*
_temp195; int _temp197; struct Cyc_Core_Opt* _temp199; struct Cyc_Core_Opt
_temp201; void* _temp202; struct Cyc_Core_Opt* _temp204; struct Cyc_Absyn_TunionInfo
_temp206; void* _temp208; struct Cyc_List_List* _temp210; void* _temp212; struct
Cyc_Absyn_TunionFieldInfo _temp214; struct Cyc_List_List* _temp216; void*
_temp218; struct _tuple0* _temp220; void* _temp222; void* _temp224; void*
_temp226; void* _temp228; void* _temp230; void* _temp232; void* _temp234; void*
_temp236; void* _temp238; void* _temp240; void* _temp242; void* _temp244; void*
_temp247; void* _temp249; void* _temp251; void* _temp253; void* _temp256; void*
_temp258; void* _temp260; void* _temp262; struct Cyc_List_List* _temp264; struct
Cyc_List_List* _temp266; struct _tuple0* _temp268; struct Cyc_List_List*
_temp270; struct _tuple0* _temp272; struct Cyc_List_List* _temp274; struct
_tuple0* _temp276; struct Cyc_List_List* _temp278; struct _tuple0* _temp280;
struct Cyc_List_List* _temp282; struct Cyc_List_List* _temp284; struct Cyc_Core_Opt*
_temp286; struct Cyc_List_List* _temp288; struct _tuple0* _temp290; void*
_temp292; _LL121: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_ArrayType:
0){ goto _LL122;} else{ goto _LL123;} _LL123: if(( unsigned int) _temp119 > 4u?*((
int*) _temp119) == Cyc_Absyn_FnType: 0){ goto _LL124;} else{ goto _LL125;}
_LL125: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_PointerType:
0){ goto _LL126;} else{ goto _LL127;} _LL127: if( _temp119 ==( void*) Cyc_Absyn_VoidType){
goto _LL128;} else{ goto _LL129;} _LL129: if(( unsigned int) _temp119 > 4u?*((
int*) _temp119) == Cyc_Absyn_VarType: 0){ _LL190: _temp189=(( struct Cyc_Absyn_VarType_struct*)
_temp119)->f1; goto _LL130;} else{ goto _LL131;} _LL131: if(( unsigned int)
_temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_Evar: 0){ _LL196: _temp195=((
struct Cyc_Absyn_Evar_struct*) _temp119)->f1; goto _LL194; _LL194: _temp193=((
struct Cyc_Absyn_Evar_struct*) _temp119)->f2; if( _temp193 == 0){ goto _LL192;}
else{ goto _LL133;} _LL192: _temp191=(( struct Cyc_Absyn_Evar_struct*) _temp119)->f3;
goto _LL132;} else{ goto _LL133;} _LL133: if(( unsigned int) _temp119 > 4u?*((
int*) _temp119) == Cyc_Absyn_Evar: 0){ _LL205: _temp204=(( struct Cyc_Absyn_Evar_struct*)
_temp119)->f1; goto _LL200; _LL200: _temp199=(( struct Cyc_Absyn_Evar_struct*)
_temp119)->f2; if( _temp199 == 0){ goto _LL135;} else{ _temp201=* _temp199;
_LL203: _temp202=( void*) _temp201.v; goto _LL198;} _LL198: _temp197=(( struct
Cyc_Absyn_Evar_struct*) _temp119)->f3; goto _LL134;} else{ goto _LL135;} _LL135:
if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_TunionType: 0){
_LL207: _temp206=(( struct Cyc_Absyn_TunionType_struct*) _temp119)->f1; _LL213:
_temp212=( void*) _temp206.tunion_info; goto _LL211; _LL211: _temp210= _temp206.targs;
goto _LL209; _LL209: _temp208=( void*) _temp206.rgn; goto _LL136;} else{ goto
_LL137;} _LL137: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_TunionFieldType:
0){ _LL215: _temp214=(( struct Cyc_Absyn_TunionFieldType_struct*) _temp119)->f1;
_LL219: _temp218=( void*) _temp214.field_info; goto _LL217; _LL217: _temp216=
_temp214.targs; goto _LL138;} else{ goto _LL139;} _LL139: if(( unsigned int)
_temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_EnumType: 0){ _LL221: _temp220=((
struct Cyc_Absyn_EnumType_struct*) _temp119)->f1; goto _LL140;} else{ goto
_LL141;} _LL141: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL225: _temp224=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp224 ==( void*) Cyc_Absyn_Signed){ goto _LL223;} else{ goto _LL143;}
_LL223: _temp222=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp222 ==( void*) Cyc_Absyn_B4){ goto _LL142;} else{ goto _LL143;}} else{ goto
_LL143;} _LL143: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL229: _temp228=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp228 ==( void*) Cyc_Absyn_Signed){ goto _LL227;} else{ goto _LL145;}
_LL227: _temp226=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp226 ==( void*) Cyc_Absyn_B1){ goto _LL144;} else{ goto _LL145;}} else{ goto
_LL145;} _LL145: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL233: _temp232=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp232 ==( void*) Cyc_Absyn_Unsigned){ goto _LL231;} else{ goto _LL147;}
_LL231: _temp230=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp230 ==( void*) Cyc_Absyn_B1){ goto _LL146;} else{ goto _LL147;}} else{ goto
_LL147;} _LL147: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL237: _temp236=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp236 ==( void*) Cyc_Absyn_Signed){ goto _LL235;} else{ goto _LL149;}
_LL235: _temp234=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp234 ==( void*) Cyc_Absyn_B2){ goto _LL148;} else{ goto _LL149;}} else{ goto
_LL149;} _LL149: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL241: _temp240=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp240 ==( void*) Cyc_Absyn_Unsigned){ goto _LL239;} else{ goto _LL151;}
_LL239: _temp238=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp238 ==( void*) Cyc_Absyn_B2){ goto _LL150;} else{ goto _LL151;}} else{ goto
_LL151;} _LL151: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL245: _temp244=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp244 ==( void*) Cyc_Absyn_Unsigned){ goto _LL243;} else{ goto _LL153;}
_LL243: _temp242=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp242 ==( void*) Cyc_Absyn_B4){ goto _LL152;} else{ goto _LL153;}} else{ goto
_LL153;} _LL153: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL250: _temp249=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp249 ==( void*) Cyc_Absyn_Signed){ goto _LL248;} else{ goto _LL155;}
_LL248: _temp247=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp247 ==( void*) Cyc_Absyn_B8){ goto _LL246;} else{ goto _LL155;}} else{ goto
_LL155;} _LL246: if( Cyc_Absyndump_to_VC){ goto _LL154;} else{ goto _LL155;}
_LL155: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL254: _temp253=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp253 ==( void*) Cyc_Absyn_Signed){ goto _LL252;} else{ goto _LL157;}
_LL252: _temp251=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp251 ==( void*) Cyc_Absyn_B8){ goto _LL156;} else{ goto _LL157;}} else{ goto
_LL157;} _LL157: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL259: _temp258=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp258 ==( void*) Cyc_Absyn_Unsigned){ goto _LL257;} else{ goto _LL159;}
_LL257: _temp256=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp256 ==( void*) Cyc_Absyn_B8){ goto _LL255;} else{ goto _LL159;}} else{ goto
_LL159;} _LL255: if( Cyc_Absyndump_to_VC){ goto _LL158;} else{ goto _LL159;}
_LL159: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_IntType:
0){ _LL263: _temp262=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f1;
if( _temp262 ==( void*) Cyc_Absyn_Unsigned){ goto _LL261;} else{ goto _LL161;}
_LL261: _temp260=( void*)(( struct Cyc_Absyn_IntType_struct*) _temp119)->f2; if(
_temp260 ==( void*) Cyc_Absyn_B8){ goto _LL160;} else{ goto _LL161;}} else{ goto
_LL161;} _LL161: if( _temp119 ==( void*) Cyc_Absyn_FloatType){ goto _LL162;}
else{ goto _LL163;} _LL163: if( _temp119 ==( void*) Cyc_Absyn_DoubleType){ goto
_LL164;} else{ goto _LL165;} _LL165: if(( unsigned int) _temp119 > 4u?*(( int*)
_temp119) == Cyc_Absyn_TupleType: 0){ _LL265: _temp264=(( struct Cyc_Absyn_TupleType_struct*)
_temp119)->f1; goto _LL166;} else{ goto _LL167;} _LL167: if(( unsigned int)
_temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_StructType: 0){ _LL269: _temp268=((
struct Cyc_Absyn_StructType_struct*) _temp119)->f1; if( _temp268 == 0){ goto
_LL267;} else{ goto _LL169;} _LL267: _temp266=(( struct Cyc_Absyn_StructType_struct*)
_temp119)->f2; goto _LL168;} else{ goto _LL169;} _LL169: if(( unsigned int)
_temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_StructType: 0){ _LL273: _temp272=((
struct Cyc_Absyn_StructType_struct*) _temp119)->f1; goto _LL271; _LL271:
_temp270=(( struct Cyc_Absyn_StructType_struct*) _temp119)->f2; goto _LL170;}
else{ goto _LL171;} _LL171: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119)
== Cyc_Absyn_UnionType: 0){ _LL277: _temp276=(( struct Cyc_Absyn_UnionType_struct*)
_temp119)->f1; if( _temp276 == 0){ goto _LL275;} else{ goto _LL173;} _LL275:
_temp274=(( struct Cyc_Absyn_UnionType_struct*) _temp119)->f2; goto _LL172;}
else{ goto _LL173;} _LL173: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119)
== Cyc_Absyn_UnionType: 0){ _LL281: _temp280=(( struct Cyc_Absyn_UnionType_struct*)
_temp119)->f1; goto _LL279; _LL279: _temp278=(( struct Cyc_Absyn_UnionType_struct*)
_temp119)->f2; goto _LL174;} else{ goto _LL175;} _LL175: if(( unsigned int)
_temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_AnonStructType: 0){ _LL283:
_temp282=(( struct Cyc_Absyn_AnonStructType_struct*) _temp119)->f1; goto _LL176;}
else{ goto _LL177;} _LL177: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119)
== Cyc_Absyn_AnonUnionType: 0){ _LL285: _temp284=(( struct Cyc_Absyn_AnonUnionType_struct*)
_temp119)->f1; goto _LL178;} else{ goto _LL179;} _LL179: if(( unsigned int)
_temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_TypedefType: 0){ _LL291: _temp290=((
struct Cyc_Absyn_TypedefType_struct*) _temp119)->f1; goto _LL289; _LL289:
_temp288=(( struct Cyc_Absyn_TypedefType_struct*) _temp119)->f2; goto _LL287;
_LL287: _temp286=(( struct Cyc_Absyn_TypedefType_struct*) _temp119)->f3; goto
_LL180;} else{ goto _LL181;} _LL181: if(( unsigned int) _temp119 > 4u?*(( int*)
_temp119) == Cyc_Absyn_RgnHandleType: 0){ _LL293: _temp292=( void*)(( struct Cyc_Absyn_RgnHandleType_struct*)
_temp119)->f1; goto _LL182;} else{ goto _LL183;} _LL183: if( _temp119 ==( void*)
Cyc_Absyn_HeapRgn){ goto _LL184;} else{ goto _LL185;} _LL185: if(( unsigned int)
_temp119 > 4u?*(( int*) _temp119) == Cyc_Absyn_AccessEff: 0){ goto _LL186;}
else{ goto _LL187;} _LL187: if(( unsigned int) _temp119 > 4u?*(( int*) _temp119)
== Cyc_Absyn_JoinEff: 0){ goto _LL188;} else{ goto _LL120;} _LL122: return;
_LL124: return; _LL126: return; _LL128: Cyc_Absyndump_dump( _tag_arr("void",
sizeof( unsigned char), 5u)); return; _LL130: Cyc_Absyndump_dump_str( _temp189->name);
return; _LL132: Cyc_Absyndump_dump( _tag_arr("%", sizeof( unsigned char), 2u));
if( _temp195 == 0){ Cyc_Absyndump_dump( _tag_arr("?", sizeof( unsigned char), 2u));}
else{ Cyc_Absyndump_dumpkind(( void*)(( struct Cyc_Core_Opt*) _check_null(
_temp195))->v);} Cyc_Absyndump_dump(( struct _tagged_arr)({ struct _tagged_arr(*
_temp294)( struct _tagged_arr fmt, struct _tagged_arr)= Cyc_Stdio_aprintf;
struct _tagged_arr _temp298= _tag_arr("(%d)", sizeof( unsigned char), 5u);
struct Cyc_Stdio_Int_pa_struct _temp299; _temp299.tag= Cyc_Stdio_Int_pa;
_temp299.f1=( int)(( unsigned int) _temp191);{ void* _temp297=( void*)& _temp299;
void* _temp295[ 1u]={ _temp297}; struct _tagged_arr _temp296={( void*) _temp295,(
void*) _temp295,( void*)( _temp295 + 1u)}; _temp294( _temp298, _temp296);}}));
return; _LL134: Cyc_Absyndump_dumpntyp( _temp202); return; _LL136:{ void*
_temp300= _temp212; struct Cyc_Absyn_UnknownTunionInfo _temp306; int _temp308;
struct _tuple0* _temp310; struct Cyc_Absyn_Tuniondecl* _temp312; struct Cyc_Absyn_Tuniondecl
_temp314; int _temp315; struct _tuple0* _temp317; _LL302: if(*(( int*) _temp300)
== Cyc_Absyn_UnknownTunion){ _LL307: _temp306=(( struct Cyc_Absyn_UnknownTunion_struct*)
_temp300)->f1; _LL311: _temp310= _temp306.name; goto _LL309; _LL309: _temp308=
_temp306.is_xtunion; goto _LL303;} else{ goto _LL304;} _LL304: if(*(( int*)
_temp300) == Cyc_Absyn_KnownTunion){ _LL313: _temp312=(( struct Cyc_Absyn_KnownTunion_struct*)
_temp300)->f1; _temp314=* _temp312; _LL318: _temp317= _temp314.name; goto _LL316;
_LL316: _temp315= _temp314.is_xtunion; goto _LL305;} else{ goto _LL301;} _LL303:
_temp317= _temp310; _temp315= _temp308; goto _LL305; _LL305: if( _temp315){ Cyc_Absyndump_dump(
_tag_arr("xtunion ", sizeof( unsigned char), 9u));} else{ Cyc_Absyndump_dump(
_tag_arr("tunion ", sizeof( unsigned char), 8u));}{ void* _temp319= Cyc_Tcutil_compress(
_temp208); _LL321: if( _temp319 ==( void*) Cyc_Absyn_HeapRgn){ goto _LL322;}
else{ goto _LL323;} _LL323: goto _LL324; _LL322: goto _LL320; _LL324: Cyc_Absyndump_dumptyp(
_temp208); Cyc_Absyndump_dump( _tag_arr(" ", sizeof( unsigned char), 2u)); goto
_LL320; _LL320:;} Cyc_Absyndump_dumpqvar( _temp317); Cyc_Absyndump_dumptps(
_temp210); goto _LL301; _LL301:;} goto _LL120; _LL138:{ void* _temp325= _temp218;
struct Cyc_Absyn_UnknownTunionFieldInfo _temp331; int _temp333; struct _tuple0*
_temp335; struct _tuple0* _temp337; struct Cyc_Absyn_Tunionfield* _temp339;
struct Cyc_Absyn_Tunionfield _temp341; struct _tuple0* _temp342; struct Cyc_Absyn_Tuniondecl*
_temp344; struct Cyc_Absyn_Tuniondecl _temp346; int _temp347; struct _tuple0*
_temp349; _LL327: if(*(( int*) _temp325) == Cyc_Absyn_UnknownTunionfield){
_LL332: _temp331=(( struct Cyc_Absyn_UnknownTunionfield_struct*) _temp325)->f1;
_LL338: _temp337= _temp331.tunion_name; goto _LL336; _LL336: _temp335= _temp331.field_name;
goto _LL334; _LL334: _temp333= _temp331.is_xtunion; goto _LL328;} else{ goto
_LL329;} _LL329: if(*(( int*) _temp325) == Cyc_Absyn_KnownTunionfield){ _LL345:
_temp344=(( struct Cyc_Absyn_KnownTunionfield_struct*) _temp325)->f1; _temp346=*
_temp344; _LL350: _temp349= _temp346.name; goto _LL348; _LL348: _temp347=
_temp346.is_xtunion; goto _LL340; _LL340: _temp339=(( struct Cyc_Absyn_KnownTunionfield_struct*)
_temp325)->f2; _temp341=* _temp339; _LL343: _temp342= _temp341.name; goto _LL330;}
else{ goto _LL326;} _LL328: _temp349= _temp337; _temp347= _temp333; _temp342=
_temp335; goto _LL330; _LL330: if( _temp347){ Cyc_Absyndump_dump( _tag_arr("xtunion ",
sizeof( unsigned char), 9u));} else{ Cyc_Absyndump_dump( _tag_arr("tunion ",
sizeof( unsigned char), 8u));} Cyc_Absyndump_dumpqvar( _temp349); Cyc_Absyndump_dump(
_tag_arr(".", sizeof( unsigned char), 2u)); Cyc_Absyndump_dumpqvar( _temp342);
Cyc_Absyndump_dumptps( _temp216); goto _LL326; _LL326:;} goto _LL120; _LL140:
Cyc_Absyndump_dump( _tag_arr("enum ", sizeof( unsigned char), 6u)); Cyc_Absyndump_dumpqvar(
_temp220); return; _LL142: Cyc_Absyndump_dump( _tag_arr("int", sizeof(
unsigned char), 4u)); return; _LL144: Cyc_Absyndump_dump( _tag_arr("signed char",
sizeof( unsigned char), 12u)); return; _LL146: Cyc_Absyndump_dump( _tag_arr("unsigned char",
sizeof( unsigned char), 14u)); return; _LL148: Cyc_Absyndump_dump( _tag_arr("short",
sizeof( unsigned char), 6u)); return; _LL150: Cyc_Absyndump_dump( _tag_arr("unsigned short",
sizeof( unsigned char), 15u)); return; _LL152: Cyc_Absyndump_dump( _tag_arr("unsigned int",
sizeof( unsigned char), 13u)); return; _LL154: Cyc_Absyndump_dump( _tag_arr("__int64",
sizeof( unsigned char), 8u)); return; _LL156: Cyc_Absyndump_dump( _tag_arr("long long",
sizeof( unsigned char), 10u)); return; _LL158: Cyc_Absyndump_dump( _tag_arr("unsigned __int64",
sizeof( unsigned char), 17u)); return; _LL160: Cyc_Absyndump_dump( _tag_arr("unsigned long long",
sizeof( unsigned char), 19u)); return; _LL162: Cyc_Absyndump_dump( _tag_arr("float",
sizeof( unsigned char), 6u)); return; _LL164: Cyc_Absyndump_dump( _tag_arr("double",
sizeof( unsigned char), 7u)); return; _LL166: Cyc_Absyndump_dump_char(( int)'$');
Cyc_Absyndump_dumpargs( _temp264); return; _LL168: Cyc_Absyndump_dump( _tag_arr("struct",
sizeof( unsigned char), 7u)); Cyc_Absyndump_dumptps( _temp266); return; _LL170:
Cyc_Absyndump_dump( _tag_arr("struct", sizeof( unsigned char), 7u)); Cyc_Absyndump_dumpqvar((
struct _tuple0*) _check_null( _temp272)); Cyc_Absyndump_dumptps( _temp270);
return; _LL172: Cyc_Absyndump_dump( _tag_arr("union", sizeof( unsigned char), 6u));
Cyc_Absyndump_dumptps( _temp274); return; _LL174: Cyc_Absyndump_dump( _tag_arr("union",
sizeof( unsigned char), 6u)); Cyc_Absyndump_dumpqvar(( struct _tuple0*)
_check_null( _temp280)); Cyc_Absyndump_dumptps( _temp278); return; _LL176: Cyc_Absyndump_dump(
_tag_arr("struct {", sizeof( unsigned char), 9u)); Cyc_Absyndump_dumpstructfields(
_temp282); Cyc_Absyndump_dump( _tag_arr("}", sizeof( unsigned char), 2u));
return; _LL178: Cyc_Absyndump_dump( _tag_arr("union {", sizeof( unsigned char),
8u)); Cyc_Absyndump_dumpstructfields( _temp284); Cyc_Absyndump_dump( _tag_arr("}",
sizeof( unsigned char), 2u)); return; _LL180:( Cyc_Absyndump_dumpqvar( _temp290),
Cyc_Absyndump_dumptps( _temp288)); return; _LL182: Cyc_Absyndump_dumprgn(
_temp292); return; _LL184: return; _LL186: return; _LL188: return; _LL120:;}
void Cyc_Absyndump_dumpvaropt( struct Cyc_Core_Opt* vo){ if( vo != 0){ Cyc_Absyndump_dump_str((
struct _tagged_arr*)(( struct Cyc_Core_Opt*) _check_null( vo))->v);}} void Cyc_Absyndump_dumpfunarg(
struct _tuple1* t){(( void(*)( struct Cyc_Absyn_Tqual, void*, void(* f)( struct
Cyc_Core_Opt*), struct Cyc_Core_Opt*)) Cyc_Absyndump_dumptqtd)((* t).f2,(* t).f3,
Cyc_Absyndump_dumpvaropt,(* t).f1);} struct _tuple7{ void* f1; void* f2; } ;
void Cyc_Absyndump_dump_rgncmp( struct _tuple7* cmp){ struct _tuple7 _temp353;
void* _temp354; void* _temp356; struct _tuple7* _temp351= cmp; _temp353=*
_temp351; _LL357: _temp356= _temp353.f1; goto _LL355; _LL355: _temp354= _temp353.f2;
goto _LL352; _LL352: Cyc_Absyndump_dumptyp( _temp356); Cyc_Absyndump_dump_char((
int)'<'); Cyc_Absyndump_dumptyp( _temp354);} void Cyc_Absyndump_dump_rgnpo(
struct Cyc_List_List* rgn_po){(( void(*)( void(* f)( struct _tuple7*), struct
Cyc_List_List* l, struct _tagged_arr sep)) Cyc_Absyndump_dump_sep)( Cyc_Absyndump_dump_rgncmp,
rgn_po, _tag_arr(",", sizeof( unsigned char), 2u));} void Cyc_Absyndump_dumpfunargs(
struct Cyc_List_List* args, int c_varargs, struct Cyc_Absyn_VarargInfo*
cyc_varargs, struct Cyc_Core_Opt* effopt, struct Cyc_List_List* rgn_po){ Cyc_Absyndump_dump_char((
int)'('); for( 0; args != 0; args=(( struct Cyc_List_List*) _check_null( args))->tl){
Cyc_Absyndump_dumpfunarg(( struct _tuple1*)(( struct Cyc_List_List*) _check_null(
args))->hd); if(((( struct Cyc_List_List*) _check_null( args))->tl != 0? 1:
c_varargs)? 1: cyc_varargs != 0){ Cyc_Absyndump_dump_char(( int)',');}} if(
c_varargs){ Cyc_Absyndump_dump( _tag_arr("...", sizeof( unsigned char), 4u));}
else{ if( cyc_varargs != 0){ struct _tuple1* _temp358=({ struct _tuple1*
_temp359=( struct _tuple1*) GC_malloc( sizeof( struct _tuple1)); _temp359->f1=((
struct Cyc_Absyn_VarargInfo*) _check_null( cyc_varargs))->name; _temp359->f2=((
struct Cyc_Absyn_VarargInfo*) _check_null( cyc_varargs))->tq; _temp359->f3=(
void*)(( struct Cyc_Absyn_VarargInfo*) _check_null( cyc_varargs))->type;
_temp359;}); Cyc_Absyndump_dump( _tag_arr("...", sizeof( unsigned char), 4u));
Cyc_Absyndump_dumptyp(( void*)(( struct Cyc_Absyn_VarargInfo*) _check_null(
cyc_varargs))->rgn); if((( struct Cyc_Absyn_VarargInfo*) _check_null(
cyc_varargs))->inject){ Cyc_Absyndump_dump( _tag_arr(" inject ", sizeof(
unsigned char), 9u));} Cyc_Absyndump_dumpfunarg( _temp358);}} if( effopt != 0){
Cyc_Absyndump_dump_semi(); Cyc_Absyndump_dumpeff(( void*)(( struct Cyc_Core_Opt*)
_check_null( effopt))->v);} if( rgn_po != 0){ Cyc_Absyndump_dump_char(( int)':');
Cyc_Absyndump_dump_rgnpo( rgn_po);} Cyc_Absyndump_dump_char(( int)')');} void
Cyc_Absyndump_dumptyp( void* t){(( void(*)( struct Cyc_Absyn_Tqual, void*, void(*
f)( int), int)) Cyc_Absyndump_dumptqtd)(({ struct Cyc_Absyn_Tqual _temp360;
_temp360.q_const= 0; _temp360.q_volatile= 0; _temp360.q_restrict= 0; _temp360;}),
t,( void(*)( int x)) Cyc_Absyndump_ignore, 0);} void Cyc_Absyndump_dumpdesignator(
void* d){ void* _temp361= d; struct Cyc_Absyn_Exp* _temp367; struct _tagged_arr*
_temp369; _LL363: if(*(( int*) _temp361) == Cyc_Absyn_ArrayElement){ _LL368:
_temp367=(( struct Cyc_Absyn_ArrayElement_struct*) _temp361)->f1; goto _LL364;}
else{ goto _LL365;} _LL365: if(*(( int*) _temp361) == Cyc_Absyn_FieldName){
_LL370: _temp369=(( struct Cyc_Absyn_FieldName_struct*) _temp361)->f1; goto
_LL366;} else{ goto _LL362;} _LL364: Cyc_Absyndump_dump( _tag_arr(".[", sizeof(
unsigned char), 3u)); Cyc_Absyndump_dumpexp( _temp367); Cyc_Absyndump_dump_char((
int)']'); goto _LL362; _LL366: Cyc_Absyndump_dump_char(( int)'.'); Cyc_Absyndump_dump_nospace(*
_temp369); goto _LL362; _LL362:;} struct _tuple8{ struct Cyc_List_List* f1;
struct Cyc_Absyn_Exp* f2; } ; void Cyc_Absyndump_dumpde( struct _tuple8* de){
Cyc_Absyndump_egroup( Cyc_Absyndump_dumpdesignator,(* de).f1, _tag_arr("",
sizeof( unsigned char), 1u), _tag_arr("=", sizeof( unsigned char), 2u), _tag_arr("=",
sizeof( unsigned char), 2u)); Cyc_Absyndump_dumpexp((* de).f2);} void Cyc_Absyndump_dumpexps_prec(
int inprec, struct Cyc_List_List* es){(( void(*)( void(* f)( int, struct Cyc_Absyn_Exp*),
int env, struct Cyc_List_List* l, struct _tagged_arr start, struct _tagged_arr
end, struct _tagged_arr sep)) Cyc_Absyndump_group_c)( Cyc_Absyndump_dumpexp_prec,
inprec, es, _tag_arr("", sizeof( unsigned char), 1u), _tag_arr("", sizeof(
unsigned char), 1u), _tag_arr(",", sizeof( unsigned char), 2u));} void Cyc_Absyndump_dumpexp_prec(
int inprec, struct Cyc_Absyn_Exp* e){ int myprec= Cyc_Absynpp_exp_prec( e); if(
inprec >= myprec){ Cyc_Absyndump_dump_nospace( _tag_arr("(", sizeof(
unsigned char), 2u));}{ void* _temp371=( void*) e->r; void* _temp465;
unsigned char _temp467; void* _temp469; void* _temp471; short _temp473; void*
_temp475; void* _temp477; int _temp479; void* _temp481; void* _temp483; int
_temp485; void* _temp487; void* _temp489; long long _temp491; void* _temp493;
void* _temp495; struct _tagged_arr _temp497; void* _temp499; void* _temp501;
struct _tagged_arr _temp503; struct _tuple0* _temp505; struct _tuple0* _temp507;
struct Cyc_List_List* _temp509; void* _temp511; struct Cyc_Absyn_Exp* _temp513;
struct Cyc_Core_Opt* _temp515; struct Cyc_Absyn_Exp* _temp517; void* _temp519;
struct Cyc_Absyn_Exp* _temp521; void* _temp523; struct Cyc_Absyn_Exp* _temp525;
void* _temp527; struct Cyc_Absyn_Exp* _temp529; void* _temp531; struct Cyc_Absyn_Exp*
_temp533; struct Cyc_Absyn_Exp* _temp535; struct Cyc_Absyn_Exp* _temp537; struct
Cyc_Absyn_Exp* _temp539; struct Cyc_Absyn_Exp* _temp541; struct Cyc_Absyn_Exp*
_temp543; struct Cyc_List_List* _temp545; struct Cyc_Absyn_Exp* _temp547; struct
Cyc_List_List* _temp549; struct Cyc_Absyn_Exp* _temp551; struct Cyc_Absyn_Exp*
_temp553; struct Cyc_Absyn_Exp* _temp555; struct Cyc_Absyn_Exp* _temp557; struct
Cyc_Absyn_Exp* _temp559; void* _temp561; struct Cyc_Absyn_Exp* _temp563; struct
Cyc_Absyn_Exp* _temp565; struct Cyc_Absyn_Exp* _temp567; void* _temp569; struct
Cyc_Absyn_Exp* _temp571; struct _tagged_arr* _temp573; void* _temp575; struct
Cyc_Absyn_Exp* _temp577; struct _tagged_arr* _temp579; struct Cyc_Absyn_Exp*
_temp581; struct _tagged_arr* _temp583; struct Cyc_Absyn_Exp* _temp585; struct
Cyc_Absyn_Exp* _temp587; struct Cyc_Absyn_Exp* _temp589; struct Cyc_List_List*
_temp591; struct Cyc_List_List* _temp593; struct _tuple1* _temp595; struct Cyc_List_List*
_temp597; struct Cyc_Absyn_Exp* _temp599; struct Cyc_Absyn_Exp* _temp601; struct
Cyc_Absyn_Vardecl* _temp603; struct Cyc_List_List* _temp605; struct _tuple0*
_temp607; struct Cyc_List_List* _temp609; struct Cyc_Absyn_Tunionfield* _temp611;
struct Cyc_List_List* _temp613; struct _tuple0* _temp615; void* _temp617; struct
Cyc_Absyn_Exp* _temp619; struct Cyc_List_List* _temp621; struct Cyc_Core_Opt*
_temp623; struct Cyc_Absyn_Stmt* _temp625; struct Cyc_Absyn_Fndecl* _temp627;
struct Cyc_Absyn_Exp* _temp629; _LL373: if(*(( int*) _temp371) == Cyc_Absyn_Const_e){
_LL466: _temp465=( void*)(( struct Cyc_Absyn_Const_e_struct*) _temp371)->f1; if((
unsigned int) _temp465 > 1u?*(( int*) _temp465) == Cyc_Absyn_Char_c: 0){ _LL470:
_temp469=( void*)(( struct Cyc_Absyn_Char_c_struct*) _temp465)->f1; goto _LL468;
_LL468: _temp467=(( struct Cyc_Absyn_Char_c_struct*) _temp465)->f2; goto _LL374;}
else{ goto _LL375;}} else{ goto _LL375;} _LL375: if(*(( int*) _temp371) == Cyc_Absyn_Const_e){
_LL472: _temp471=( void*)(( struct Cyc_Absyn_Const_e_struct*) _temp371)->f1; if((
unsigned int) _temp471 > 1u?*(( int*) _temp471) == Cyc_Absyn_Short_c: 0){ _LL476:
_temp475=( void*)(( struct Cyc_Absyn_Short_c_struct*) _temp471)->f1; goto _LL474;
_LL474: _temp473=(( struct Cyc_Absyn_Short_c_struct*) _temp471)->f2; goto _LL376;}
else{ goto _LL377;}} else{ goto _LL377;} _LL377: if(*(( int*) _temp371) == Cyc_Absyn_Const_e){
_LL478: _temp477=( void*)(( struct Cyc_Absyn_Const_e_struct*) _temp371)->f1; if((
unsigned int) _temp477 > 1u?*(( int*) _temp477) == Cyc_Absyn_Int_c: 0){ _LL482:
_temp481=( void*)(( struct Cyc_Absyn_Int_c_struct*) _temp477)->f1; if( _temp481
==( void*) Cyc_Absyn_Signed){ goto _LL480;} else{ goto _LL379;} _LL480: _temp479=((
struct Cyc_Absyn_Int_c_struct*) _temp477)->f2; goto _LL378;} else{ goto _LL379;}}
else{ goto _LL379;} _LL379: if(*(( int*) _temp371) == Cyc_Absyn_Const_e){ _LL484:
_temp483=( void*)(( struct Cyc_Absyn_Const_e_struct*) _temp371)->f1; if((
unsigned int) _temp483 > 1u?*(( int*) _temp483) == Cyc_Absyn_Int_c: 0){ _LL488:
_temp487=( void*)(( struct Cyc_Absyn_Int_c_struct*) _temp483)->f1; if( _temp487
==( void*) Cyc_Absyn_Unsigned){ goto _LL486;} else{ goto _LL381;} _LL486:
_temp485=(( struct Cyc_Absyn_Int_c_struct*) _temp483)->f2; goto _LL380;} else{
goto _LL381;}} else{ goto _LL381;} _LL381: if(*(( int*) _temp371) == Cyc_Absyn_Const_e){
_LL490: _temp489=( void*)(( struct Cyc_Absyn_Const_e_struct*) _temp371)->f1; if((
unsigned int) _temp489 > 1u?*(( int*) _temp489) == Cyc_Absyn_LongLong_c: 0){
_LL494: _temp493=( void*)(( struct Cyc_Absyn_LongLong_c_struct*) _temp489)->f1;
goto _LL492; _LL492: _temp491=(( struct Cyc_Absyn_LongLong_c_struct*) _temp489)->f2;
goto _LL382;} else{ goto _LL383;}} else{ goto _LL383;} _LL383: if(*(( int*)
_temp371) == Cyc_Absyn_Const_e){ _LL496: _temp495=( void*)(( struct Cyc_Absyn_Const_e_struct*)
_temp371)->f1; if(( unsigned int) _temp495 > 1u?*(( int*) _temp495) == Cyc_Absyn_Float_c:
0){ _LL498: _temp497=(( struct Cyc_Absyn_Float_c_struct*) _temp495)->f1; goto
_LL384;} else{ goto _LL385;}} else{ goto _LL385;} _LL385: if(*(( int*) _temp371)
== Cyc_Absyn_Const_e){ _LL500: _temp499=( void*)(( struct Cyc_Absyn_Const_e_struct*)
_temp371)->f1; if( _temp499 ==( void*) Cyc_Absyn_Null_c){ goto _LL386;} else{
goto _LL387;}} else{ goto _LL387;} _LL387: if(*(( int*) _temp371) == Cyc_Absyn_Const_e){
_LL502: _temp501=( void*)(( struct Cyc_Absyn_Const_e_struct*) _temp371)->f1; if((
unsigned int) _temp501 > 1u?*(( int*) _temp501) == Cyc_Absyn_String_c: 0){
_LL504: _temp503=(( struct Cyc_Absyn_String_c_struct*) _temp501)->f1; goto
_LL388;} else{ goto _LL389;}} else{ goto _LL389;} _LL389: if(*(( int*) _temp371)
== Cyc_Absyn_UnknownId_e){ _LL506: _temp505=(( struct Cyc_Absyn_UnknownId_e_struct*)
_temp371)->f1; goto _LL390;} else{ goto _LL391;} _LL391: if(*(( int*) _temp371)
== Cyc_Absyn_Var_e){ _LL508: _temp507=(( struct Cyc_Absyn_Var_e_struct*)
_temp371)->f1; goto _LL392;} else{ goto _LL393;} _LL393: if(*(( int*) _temp371)
== Cyc_Absyn_Primop_e){ _LL512: _temp511=( void*)(( struct Cyc_Absyn_Primop_e_struct*)
_temp371)->f1; goto _LL510; _LL510: _temp509=(( struct Cyc_Absyn_Primop_e_struct*)
_temp371)->f2; goto _LL394;} else{ goto _LL395;} _LL395: if(*(( int*) _temp371)
== Cyc_Absyn_AssignOp_e){ _LL518: _temp517=(( struct Cyc_Absyn_AssignOp_e_struct*)
_temp371)->f1; goto _LL516; _LL516: _temp515=(( struct Cyc_Absyn_AssignOp_e_struct*)
_temp371)->f2; goto _LL514; _LL514: _temp513=(( struct Cyc_Absyn_AssignOp_e_struct*)
_temp371)->f3; goto _LL396;} else{ goto _LL397;} _LL397: if(*(( int*) _temp371)
== Cyc_Absyn_Increment_e){ _LL522: _temp521=(( struct Cyc_Absyn_Increment_e_struct*)
_temp371)->f1; goto _LL520; _LL520: _temp519=( void*)(( struct Cyc_Absyn_Increment_e_struct*)
_temp371)->f2; if( _temp519 ==( void*) Cyc_Absyn_PreInc){ goto _LL398;} else{
goto _LL399;}} else{ goto _LL399;} _LL399: if(*(( int*) _temp371) == Cyc_Absyn_Increment_e){
_LL526: _temp525=(( struct Cyc_Absyn_Increment_e_struct*) _temp371)->f1; goto
_LL524; _LL524: _temp523=( void*)(( struct Cyc_Absyn_Increment_e_struct*)
_temp371)->f2; if( _temp523 ==( void*) Cyc_Absyn_PreDec){ goto _LL400;} else{
goto _LL401;}} else{ goto _LL401;} _LL401: if(*(( int*) _temp371) == Cyc_Absyn_Increment_e){
_LL530: _temp529=(( struct Cyc_Absyn_Increment_e_struct*) _temp371)->f1; goto
_LL528; _LL528: _temp527=( void*)(( struct Cyc_Absyn_Increment_e_struct*)
_temp371)->f2; if( _temp527 ==( void*) Cyc_Absyn_PostInc){ goto _LL402;} else{
goto _LL403;}} else{ goto _LL403;} _LL403: if(*(( int*) _temp371) == Cyc_Absyn_Increment_e){
_LL534: _temp533=(( struct Cyc_Absyn_Increment_e_struct*) _temp371)->f1; goto
_LL532; _LL532: _temp531=( void*)(( struct Cyc_Absyn_Increment_e_struct*)
_temp371)->f2; if( _temp531 ==( void*) Cyc_Absyn_PostDec){ goto _LL404;} else{
goto _LL405;}} else{ goto _LL405;} _LL405: if(*(( int*) _temp371) == Cyc_Absyn_Conditional_e){
_LL540: _temp539=(( struct Cyc_Absyn_Conditional_e_struct*) _temp371)->f1; goto
_LL538; _LL538: _temp537=(( struct Cyc_Absyn_Conditional_e_struct*) _temp371)->f2;
goto _LL536; _LL536: _temp535=(( struct Cyc_Absyn_Conditional_e_struct*)
_temp371)->f3; goto _LL406;} else{ goto _LL407;} _LL407: if(*(( int*) _temp371)
== Cyc_Absyn_SeqExp_e){ _LL544: _temp543=(( struct Cyc_Absyn_SeqExp_e_struct*)
_temp371)->f1; goto _LL542; _LL542: _temp541=(( struct Cyc_Absyn_SeqExp_e_struct*)
_temp371)->f2; goto _LL408;} else{ goto _LL409;} _LL409: if(*(( int*) _temp371)
== Cyc_Absyn_UnknownCall_e){ _LL548: _temp547=(( struct Cyc_Absyn_UnknownCall_e_struct*)
_temp371)->f1; goto _LL546; _LL546: _temp545=(( struct Cyc_Absyn_UnknownCall_e_struct*)
_temp371)->f2; goto _LL410;} else{ goto _LL411;} _LL411: if(*(( int*) _temp371)
== Cyc_Absyn_FnCall_e){ _LL552: _temp551=(( struct Cyc_Absyn_FnCall_e_struct*)
_temp371)->f1; goto _LL550; _LL550: _temp549=(( struct Cyc_Absyn_FnCall_e_struct*)
_temp371)->f2; goto _LL412;} else{ goto _LL413;} _LL413: if(*(( int*) _temp371)
== Cyc_Absyn_Throw_e){ _LL554: _temp553=(( struct Cyc_Absyn_Throw_e_struct*)
_temp371)->f1; goto _LL414;} else{ goto _LL415;} _LL415: if(*(( int*) _temp371)
== Cyc_Absyn_NoInstantiate_e){ _LL556: _temp555=(( struct Cyc_Absyn_NoInstantiate_e_struct*)
_temp371)->f1; goto _LL416;} else{ goto _LL417;} _LL417: if(*(( int*) _temp371)
== Cyc_Absyn_Instantiate_e){ _LL558: _temp557=(( struct Cyc_Absyn_Instantiate_e_struct*)
_temp371)->f1; goto _LL418;} else{ goto _LL419;} _LL419: if(*(( int*) _temp371)
== Cyc_Absyn_Cast_e){ _LL562: _temp561=( void*)(( struct Cyc_Absyn_Cast_e_struct*)
_temp371)->f1; goto _LL560; _LL560: _temp559=(( struct Cyc_Absyn_Cast_e_struct*)
_temp371)->f2; goto _LL420;} else{ goto _LL421;} _LL421: if(*(( int*) _temp371)
== Cyc_Absyn_Address_e){ _LL564: _temp563=(( struct Cyc_Absyn_Address_e_struct*)
_temp371)->f1; goto _LL422;} else{ goto _LL423;} _LL423: if(*(( int*) _temp371)
== Cyc_Absyn_New_e){ _LL568: _temp567=(( struct Cyc_Absyn_New_e_struct*)
_temp371)->f1; goto _LL566; _LL566: _temp565=(( struct Cyc_Absyn_New_e_struct*)
_temp371)->f2; goto _LL424;} else{ goto _LL425;} _LL425: if(*(( int*) _temp371)
== Cyc_Absyn_Sizeoftyp_e){ _LL570: _temp569=( void*)(( struct Cyc_Absyn_Sizeoftyp_e_struct*)
_temp371)->f1; goto _LL426;} else{ goto _LL427;} _LL427: if(*(( int*) _temp371)
== Cyc_Absyn_Sizeofexp_e){ _LL572: _temp571=(( struct Cyc_Absyn_Sizeofexp_e_struct*)
_temp371)->f1; goto _LL428;} else{ goto _LL429;} _LL429: if(*(( int*) _temp371)
== Cyc_Absyn_Offsetof_e){ _LL576: _temp575=( void*)(( struct Cyc_Absyn_Offsetof_e_struct*)
_temp371)->f1; goto _LL574; _LL574: _temp573=(( struct Cyc_Absyn_Offsetof_e_struct*)
_temp371)->f2; goto _LL430;} else{ goto _LL431;} _LL431: if(*(( int*) _temp371)
== Cyc_Absyn_Deref_e){ _LL578: _temp577=(( struct Cyc_Absyn_Deref_e_struct*)
_temp371)->f1; goto _LL432;} else{ goto _LL433;} _LL433: if(*(( int*) _temp371)
== Cyc_Absyn_StructMember_e){ _LL582: _temp581=(( struct Cyc_Absyn_StructMember_e_struct*)
_temp371)->f1; goto _LL580; _LL580: _temp579=(( struct Cyc_Absyn_StructMember_e_struct*)
_temp371)->f2; goto _LL434;} else{ goto _LL435;} _LL435: if(*(( int*) _temp371)
== Cyc_Absyn_StructArrow_e){ _LL586: _temp585=(( struct Cyc_Absyn_StructArrow_e_struct*)
_temp371)->f1; goto _LL584; _LL584: _temp583=(( struct Cyc_Absyn_StructArrow_e_struct*)
_temp371)->f2; goto _LL436;} else{ goto _LL437;} _LL437: if(*(( int*) _temp371)
== Cyc_Absyn_Subscript_e){ _LL590: _temp589=(( struct Cyc_Absyn_Subscript_e_struct*)
_temp371)->f1; goto _LL588; _LL588: _temp587=(( struct Cyc_Absyn_Subscript_e_struct*)
_temp371)->f2; goto _LL438;} else{ goto _LL439;} _LL439: if(*(( int*) _temp371)
== Cyc_Absyn_Tuple_e){ _LL592: _temp591=(( struct Cyc_Absyn_Tuple_e_struct*)
_temp371)->f1; goto _LL440;} else{ goto _LL441;} _LL441: if(*(( int*) _temp371)
== Cyc_Absyn_CompoundLit_e){ _LL596: _temp595=(( struct Cyc_Absyn_CompoundLit_e_struct*)
_temp371)->f1; goto _LL594; _LL594: _temp593=(( struct Cyc_Absyn_CompoundLit_e_struct*)
_temp371)->f2; goto _LL442;} else{ goto _LL443;} _LL443: if(*(( int*) _temp371)
== Cyc_Absyn_Array_e){ _LL598: _temp597=(( struct Cyc_Absyn_Array_e_struct*)
_temp371)->f1; goto _LL444;} else{ goto _LL445;} _LL445: if(*(( int*) _temp371)
== Cyc_Absyn_Comprehension_e){ _LL604: _temp603=(( struct Cyc_Absyn_Comprehension_e_struct*)
_temp371)->f1; goto _LL602; _LL602: _temp601=(( struct Cyc_Absyn_Comprehension_e_struct*)
_temp371)->f2; goto _LL600; _LL600: _temp599=(( struct Cyc_Absyn_Comprehension_e_struct*)
_temp371)->f3; goto _LL446;} else{ goto _LL447;} _LL447: if(*(( int*) _temp371)
== Cyc_Absyn_Struct_e){ _LL608: _temp607=(( struct Cyc_Absyn_Struct_e_struct*)
_temp371)->f1; goto _LL606; _LL606: _temp605=(( struct Cyc_Absyn_Struct_e_struct*)
_temp371)->f3; goto _LL448;} else{ goto _LL449;} _LL449: if(*(( int*) _temp371)
== Cyc_Absyn_AnonStruct_e){ _LL610: _temp609=(( struct Cyc_Absyn_AnonStruct_e_struct*)
_temp371)->f2; goto _LL450;} else{ goto _LL451;} _LL451: if(*(( int*) _temp371)
== Cyc_Absyn_Tunion_e){ _LL614: _temp613=(( struct Cyc_Absyn_Tunion_e_struct*)
_temp371)->f3; goto _LL612; _LL612: _temp611=(( struct Cyc_Absyn_Tunion_e_struct*)
_temp371)->f5; goto _LL452;} else{ goto _LL453;} _LL453: if(*(( int*) _temp371)
== Cyc_Absyn_Enum_e){ _LL616: _temp615=(( struct Cyc_Absyn_Enum_e_struct*)
_temp371)->f1; goto _LL454;} else{ goto _LL455;} _LL455: if(*(( int*) _temp371)
== Cyc_Absyn_Malloc_e){ _LL620: _temp619=(( struct Cyc_Absyn_Malloc_e_struct*)
_temp371)->f1; goto _LL618; _LL618: _temp617=( void*)(( struct Cyc_Absyn_Malloc_e_struct*)
_temp371)->f2; goto _LL456;} else{ goto _LL457;} _LL457: if(*(( int*) _temp371)
== Cyc_Absyn_UnresolvedMem_e){ _LL624: _temp623=(( struct Cyc_Absyn_UnresolvedMem_e_struct*)
_temp371)->f1; goto _LL622; _LL622: _temp621=(( struct Cyc_Absyn_UnresolvedMem_e_struct*)
_temp371)->f2; goto _LL458;} else{ goto _LL459;} _LL459: if(*(( int*) _temp371)
== Cyc_Absyn_StmtExp_e){ _LL626: _temp625=(( struct Cyc_Absyn_StmtExp_e_struct*)
_temp371)->f1; goto _LL460;} else{ goto _LL461;} _LL461: if(*(( int*) _temp371)
== Cyc_Absyn_Codegen_e){ _LL628: _temp627=(( struct Cyc_Absyn_Codegen_e_struct*)
_temp371)->f1; goto _LL462;} else{ goto _LL463;} _LL463: if(*(( int*) _temp371)
== Cyc_Absyn_Fill_e){ _LL630: _temp629=(( struct Cyc_Absyn_Fill_e_struct*)
_temp371)->f1; goto _LL464;} else{ goto _LL372;} _LL374: Cyc_Absyndump_dump_char((
int)'\''); Cyc_Absyndump_dump_nospace( Cyc_Absynpp_char_escape( _temp467)); Cyc_Absyndump_dump_char((
int)'\''); goto _LL372; _LL376: Cyc_Absyndump_dump(( struct _tagged_arr) Cyc_Core_string_of_int((
int) _temp473)); goto _LL372; _LL378: Cyc_Absyndump_dump(( struct _tagged_arr)
Cyc_Core_string_of_int( _temp479)); goto _LL372; _LL380: Cyc_Absyndump_dump((
struct _tagged_arr) Cyc_Core_string_of_int( _temp485)); Cyc_Absyndump_dump_nospace(
_tag_arr("u", sizeof( unsigned char), 2u)); goto _LL372; _LL382: Cyc_Absyndump_dump(
_tag_arr("<<FIX LONG LONG CONSTANT>>", sizeof( unsigned char), 27u)); goto
_LL372; _LL384: Cyc_Absyndump_dump( _temp497); goto _LL372; _LL386: Cyc_Absyndump_dump(
_tag_arr("null", sizeof( unsigned char), 5u)); goto _LL372; _LL388: Cyc_Absyndump_dump_char((
int)'"'); Cyc_Absyndump_dump_nospace( Cyc_Absynpp_string_escape( _temp503)); Cyc_Absyndump_dump_char((
int)'"'); goto _LL372; _LL390: _temp507= _temp505; goto _LL392; _LL392: Cyc_Absyndump_dumpqvar(
_temp507); goto _LL372; _LL394: { struct _tagged_arr _temp631= Cyc_Absynpp_prim2str(
_temp511); switch((( int(*)( struct Cyc_List_List* x)) Cyc_List_length)(
_temp509)){ case 1: _LL632: if( _temp511 ==( void*) Cyc_Absyn_Size){ Cyc_Absyndump_dumpexp_prec(
myprec,( struct Cyc_Absyn_Exp*)(( struct Cyc_List_List*) _check_null( _temp509))->hd);
Cyc_Absyndump_dump( _tag_arr(".size", sizeof( unsigned char), 6u));} else{ Cyc_Absyndump_dump(
_temp631); Cyc_Absyndump_dumpexp_prec( myprec,( struct Cyc_Absyn_Exp*)(( struct
Cyc_List_List*) _check_null( _temp509))->hd);} break; case 2: _LL633: Cyc_Absyndump_dumpexp_prec(
myprec,( struct Cyc_Absyn_Exp*)(( struct Cyc_List_List*) _check_null( _temp509))->hd);
Cyc_Absyndump_dump( _temp631); Cyc_Absyndump_dumpexp_prec( myprec,( struct Cyc_Absyn_Exp*)((
struct Cyc_List_List*) _check_null((( struct Cyc_List_List*) _check_null(
_temp509))->tl))->hd); break; default: _LL634:( int) _throw(( void*)({ struct
Cyc_Core_Failure_struct* _temp636=( struct Cyc_Core_Failure_struct*) GC_malloc(
sizeof( struct Cyc_Core_Failure_struct)); _temp636[ 0]=({ struct Cyc_Core_Failure_struct
_temp637; _temp637.tag= Cyc_Core_Failure; _temp637.f1= _tag_arr("Absyndump -- Bad number of arguments to primop",
sizeof( unsigned char), 47u); _temp637;}); _temp636;}));} goto _LL372;} _LL396:
Cyc_Absyndump_dumpexp_prec( myprec, _temp517); if( _temp515 != 0){ Cyc_Absyndump_dump(
Cyc_Absynpp_prim2str(( void*)(( struct Cyc_Core_Opt*) _check_null( _temp515))->v));}
Cyc_Absyndump_dump_nospace( _tag_arr("=", sizeof( unsigned char), 2u)); Cyc_Absyndump_dumpexp_prec(
myprec, _temp513); goto _LL372; _LL398: Cyc_Absyndump_dump( _tag_arr("++",
sizeof( unsigned char), 3u)); Cyc_Absyndump_dumpexp_prec( myprec, _temp521);
goto _LL372; _LL400: Cyc_Absyndump_dump( _tag_arr("--", sizeof( unsigned char),
3u)); Cyc_Absyndump_dumpexp_prec( myprec, _temp525); goto _LL372; _LL402: Cyc_Absyndump_dumpexp_prec(
myprec, _temp529); Cyc_Absyndump_dump( _tag_arr("++", sizeof( unsigned char), 3u));
goto _LL372; _LL404: Cyc_Absyndump_dumpexp_prec( myprec, _temp533); Cyc_Absyndump_dump(
_tag_arr("--", sizeof( unsigned char), 3u)); goto _LL372; _LL406: Cyc_Absyndump_dumpexp_prec(
myprec, _temp539); Cyc_Absyndump_dump_char(( int)'?'); Cyc_Absyndump_dumpexp_prec(
0, _temp537); Cyc_Absyndump_dump_char(( int)':'); Cyc_Absyndump_dumpexp_prec(
myprec, _temp535); goto _LL372; _LL408: Cyc_Absyndump_dump_char(( int)'('); Cyc_Absyndump_dumpexp_prec(
myprec, _temp543); Cyc_Absyndump_dump_char(( int)','); Cyc_Absyndump_dumpexp_prec(
myprec, _temp541); Cyc_Absyndump_dump_char(( int)')'); goto _LL372; _LL410:
_temp551= _temp547; _temp549= _temp545; goto _LL412; _LL412: Cyc_Absyndump_dumpexp_prec(
myprec, _temp551); Cyc_Absyndump_dump_nospace( _tag_arr("(", sizeof(
unsigned char), 2u)); Cyc_Absyndump_dumpexps_prec( 20, _temp549); Cyc_Absyndump_dump_nospace(
_tag_arr(")", sizeof( unsigned char), 2u)); goto _LL372; _LL414: Cyc_Absyndump_dump(
_tag_arr("throw", sizeof( unsigned char), 6u)); Cyc_Absyndump_dumpexp_prec(
myprec, _temp553); goto _LL372; _LL416: _temp557= _temp555; goto _LL418; _LL418:
Cyc_Absyndump_dumpexp_prec( inprec, _temp557); goto _LL372; _LL420: Cyc_Absyndump_dump_char((
int)'('); Cyc_Absyndump_dumptyp( _temp561); Cyc_Absyndump_dump_char(( int)')');
Cyc_Absyndump_dumpexp_prec( myprec, _temp559); goto _LL372; _LL422: Cyc_Absyndump_dump_char((
int)'&'); Cyc_Absyndump_dumpexp_prec( myprec, _temp563); goto _LL372; _LL424:
Cyc_Absyndump_dump( _tag_arr("new ", sizeof( unsigned char), 5u)); Cyc_Absyndump_dumpexp_prec(
myprec, _temp565); goto _LL372; _LL426: Cyc_Absyndump_dump( _tag_arr("sizeof(",
sizeof( unsigned char), 8u)); Cyc_Absyndump_dumptyp( _temp569); Cyc_Absyndump_dump_char((
int)')'); goto _LL372; _LL428: Cyc_Absyndump_dump( _tag_arr("sizeof(", sizeof(
unsigned char), 8u)); Cyc_Absyndump_dumpexp_prec( myprec, _temp571); Cyc_Absyndump_dump_char((
int)')'); goto _LL372; _LL430: Cyc_Absyndump_dump( _tag_arr("offsetof(", sizeof(
unsigned char), 10u)); Cyc_Absyndump_dumptyp( _temp575); Cyc_Absyndump_dump_char((
int)','); Cyc_Absyndump_dump_nospace(* _temp573); Cyc_Absyndump_dump_char(( int)')');
goto _LL372; _LL432: Cyc_Absyndump_dump_char(( int)'*'); Cyc_Absyndump_dumpexp_prec(
myprec, _temp577); goto _LL372; _LL434: Cyc_Absyndump_dumpexp_prec( myprec,
_temp581); Cyc_Absyndump_dump_char(( int)'.'); Cyc_Absyndump_dump_nospace(*
_temp579); goto _LL372; _LL436: Cyc_Absyndump_dumpexp_prec( myprec, _temp585);
Cyc_Absyndump_dump_nospace( _tag_arr("->", sizeof( unsigned char), 3u)); Cyc_Absyndump_dump_nospace(*
_temp583); goto _LL372; _LL438: Cyc_Absyndump_dumpexp_prec( myprec, _temp589);
Cyc_Absyndump_dump_char(( int)'['); Cyc_Absyndump_dumpexp( _temp587); Cyc_Absyndump_dump_char((
int)']'); goto _LL372; _LL440: Cyc_Absyndump_dump( _tag_arr("$(", sizeof(
unsigned char), 3u)); Cyc_Absyndump_dumpexps_prec( 20, _temp591); Cyc_Absyndump_dump_char((
int)')'); goto _LL372; _LL442: Cyc_Absyndump_dump_char(( int)'('); Cyc_Absyndump_dumptyp((*
_temp595).f3); Cyc_Absyndump_dump_char(( int)')');(( void(*)( void(* f)( struct
_tuple8*), struct Cyc_List_List* l, struct _tagged_arr start, struct _tagged_arr
end, struct _tagged_arr sep)) Cyc_Absyndump_group)( Cyc_Absyndump_dumpde,
_temp593, _tag_arr("{", sizeof( unsigned char), 2u), _tag_arr("}", sizeof(
unsigned char), 2u), _tag_arr(",", sizeof( unsigned char), 2u)); goto _LL372;
_LL444:(( void(*)( void(* f)( struct _tuple8*), struct Cyc_List_List* l, struct
_tagged_arr start, struct _tagged_arr end, struct _tagged_arr sep)) Cyc_Absyndump_group)(
Cyc_Absyndump_dumpde, _temp597, _tag_arr("{", sizeof( unsigned char), 2u),
_tag_arr("}", sizeof( unsigned char), 2u), _tag_arr(",", sizeof( unsigned char),
2u)); goto _LL372; _LL446: Cyc_Absyndump_dump( _tag_arr("new {for", sizeof(
unsigned char), 9u)); Cyc_Absyndump_dump_str((* _temp603->name).f2); Cyc_Absyndump_dump_char((
int)'<'); Cyc_Absyndump_dumpexp( _temp601); Cyc_Absyndump_dump_char(( int)':');
Cyc_Absyndump_dumpexp( _temp599); Cyc_Absyndump_dump_char(( int)'}'); goto
_LL372; _LL448: Cyc_Absyndump_dumpqvar( _temp607);(( void(*)( void(* f)( struct
_tuple8*), struct Cyc_List_List* l, struct _tagged_arr start, struct _tagged_arr
end, struct _tagged_arr sep)) Cyc_Absyndump_group)( Cyc_Absyndump_dumpde,
_temp605, _tag_arr("{", sizeof( unsigned char), 2u), _tag_arr("}", sizeof(
unsigned char), 2u), _tag_arr(",", sizeof( unsigned char), 2u)); goto _LL372;
_LL450:(( void(*)( void(* f)( struct _tuple8*), struct Cyc_List_List* l, struct
_tagged_arr start, struct _tagged_arr end, struct _tagged_arr sep)) Cyc_Absyndump_group)(
Cyc_Absyndump_dumpde, _temp609, _tag_arr("{", sizeof( unsigned char), 2u),
_tag_arr("}", sizeof( unsigned char), 2u), _tag_arr(",", sizeof( unsigned char),
2u)); goto _LL372; _LL452: Cyc_Absyndump_dumpqvar( _temp611->name); if( _temp613
!= 0){(( void(*)( void(* f)( struct Cyc_Absyn_Exp*), struct Cyc_List_List* l,
struct _tagged_arr start, struct _tagged_arr end, struct _tagged_arr sep)) Cyc_Absyndump_group)(
Cyc_Absyndump_dumpexp, _temp613, _tag_arr("(", sizeof( unsigned char), 2u),
_tag_arr(")", sizeof( unsigned char), 2u), _tag_arr(",", sizeof( unsigned char),
2u));} goto _LL372; _LL454: Cyc_Absyndump_dumpqvar( _temp615); goto _LL372;
_LL456: if( _temp619 != 0){ Cyc_Absyndump_dump( _tag_arr("rmalloc(", sizeof(
unsigned char), 9u)); Cyc_Absyndump_dumpexp(( struct Cyc_Absyn_Exp*) _check_null(
_temp619)); Cyc_Absyndump_dump( _tag_arr(",", sizeof( unsigned char), 2u));}
else{ Cyc_Absyndump_dump( _tag_arr("malloc(", sizeof( unsigned char), 8u));} Cyc_Absyndump_dump(
_tag_arr("sizeof(", sizeof( unsigned char), 8u)); Cyc_Absyndump_dumptyp(
_temp617); Cyc_Absyndump_dump( _tag_arr("))", sizeof( unsigned char), 3u)); goto
_LL372; _LL458:(( void(*)( void(* f)( struct _tuple8*), struct Cyc_List_List* l,
struct _tagged_arr start, struct _tagged_arr end, struct _tagged_arr sep)) Cyc_Absyndump_group)(
Cyc_Absyndump_dumpde, _temp621, _tag_arr("{", sizeof( unsigned char), 2u),
_tag_arr("}", sizeof( unsigned char), 2u), _tag_arr(",", sizeof( unsigned char),
2u)); goto _LL372; _LL460: Cyc_Absyndump_dump_nospace( _tag_arr("({", sizeof(
unsigned char), 3u)); Cyc_Absyndump_dumpstmt( _temp625); Cyc_Absyndump_dump_nospace(
_tag_arr("})", sizeof( unsigned char), 3u)); goto _LL372; _LL462: Cyc_Absyndump_dump(
_tag_arr("codegen(", sizeof( unsigned char), 9u)); Cyc_Absyndump_dumpdecl(({
struct Cyc_Absyn_Decl* _temp638=( struct Cyc_Absyn_Decl*) GC_malloc( sizeof(
struct Cyc_Absyn_Decl)); _temp638->r=( void*)(( void*)({ struct Cyc_Absyn_Fn_d_struct*
_temp639=( struct Cyc_Absyn_Fn_d_struct*) GC_malloc( sizeof( struct Cyc_Absyn_Fn_d_struct));
_temp639[ 0]=({ struct Cyc_Absyn_Fn_d_struct _temp640; _temp640.tag= Cyc_Absyn_Fn_d;
_temp640.f1= _temp627; _temp640;}); _temp639;})); _temp638->loc= e->loc;
_temp638;})); Cyc_Absyndump_dump( _tag_arr(")", sizeof( unsigned char), 2u));
goto _LL372; _LL464: Cyc_Absyndump_dump( _tag_arr("fill(", sizeof( unsigned char),
6u)); Cyc_Absyndump_dumpexp( _temp629); Cyc_Absyndump_dump( _tag_arr(")",
sizeof( unsigned char), 2u)); goto _LL372; _LL372:;} if( inprec >= myprec){ Cyc_Absyndump_dump_char((
int)')');}} void Cyc_Absyndump_dumpexp( struct Cyc_Absyn_Exp* e){ Cyc_Absyndump_dumpexp_prec(
0, e);} void Cyc_Absyndump_dumpswitchclauses( struct Cyc_List_List* scs){ for( 0;
scs != 0; scs=(( struct Cyc_List_List*) _check_null( scs))->tl){ struct Cyc_Absyn_Switch_clause*
_temp641=( struct Cyc_Absyn_Switch_clause*)(( struct Cyc_List_List*) _check_null(
scs))->hd; if( _temp641->where_clause == 0?( void*)( _temp641->pattern)->r ==(
void*) Cyc_Absyn_Wild_p: 0){ Cyc_Absyndump_dump( _tag_arr("default:", sizeof(
unsigned char), 9u));} else{ Cyc_Absyndump_dump( _tag_arr("case", sizeof(
unsigned char), 5u)); Cyc_Absyndump_dumppat( _temp641->pattern); if( _temp641->where_clause
!= 0){ Cyc_Absyndump_dump( _tag_arr("&&", sizeof( unsigned char), 3u)); Cyc_Absyndump_dumpexp((
struct Cyc_Absyn_Exp*) _check_null( _temp641->where_clause));} Cyc_Absyndump_dump_nospace(
_tag_arr(":", sizeof( unsigned char), 2u));} Cyc_Absyndump_dumpstmt( _temp641->body);}}
void Cyc_Absyndump_dumpstmt( struct Cyc_Absyn_Stmt* s){ void* _temp642=( void*)
s->r; struct Cyc_Absyn_Exp* _temp688; struct Cyc_Absyn_Stmt* _temp690; struct
Cyc_Absyn_Stmt* _temp692; struct Cyc_Absyn_Exp* _temp694; struct Cyc_Absyn_Exp*
_temp696; struct Cyc_Absyn_Stmt* _temp698; struct Cyc_Absyn_Stmt* _temp700;
struct Cyc_Absyn_Exp* _temp702; struct Cyc_Absyn_Stmt* _temp704; struct _tuple2
_temp706; struct Cyc_Absyn_Exp* _temp708; struct _tagged_arr* _temp710; struct
Cyc_Absyn_Stmt* _temp712; struct _tuple2 _temp714; struct Cyc_Absyn_Exp*
_temp716; struct _tuple2 _temp718; struct Cyc_Absyn_Exp* _temp720; struct Cyc_Absyn_Exp*
_temp722; struct Cyc_List_List* _temp724; struct Cyc_Absyn_Exp* _temp726; struct
Cyc_Absyn_Stmt* _temp728; struct Cyc_Absyn_Decl* _temp730; struct Cyc_Absyn_Stmt*
_temp732; struct _tagged_arr* _temp734; struct _tuple2 _temp736; struct Cyc_Absyn_Exp*
_temp738; struct Cyc_Absyn_Stmt* _temp740; struct Cyc_List_List* _temp742;
struct Cyc_Absyn_Exp* _temp744; struct Cyc_List_List* _temp746; struct Cyc_List_List*
_temp748; struct Cyc_List_List* _temp750; struct Cyc_Absyn_Stmt* _temp752;
struct Cyc_Absyn_Stmt* _temp754; struct Cyc_Absyn_Vardecl* _temp756; struct Cyc_Absyn_Tvar*
_temp758; struct Cyc_Absyn_Stmt* _temp760; struct Cyc_Absyn_Stmt* _temp762;
_LL644: if( _temp642 ==( void*) Cyc_Absyn_Skip_s){ goto _LL645;} else{ goto
_LL646;} _LL646: if(( unsigned int) _temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Exp_s:
0){ _LL689: _temp688=(( struct Cyc_Absyn_Exp_s_struct*) _temp642)->f1; goto
_LL647;} else{ goto _LL648;} _LL648: if(( unsigned int) _temp642 > 1u?*(( int*)
_temp642) == Cyc_Absyn_Seq_s: 0){ _LL693: _temp692=(( struct Cyc_Absyn_Seq_s_struct*)
_temp642)->f1; goto _LL691; _LL691: _temp690=(( struct Cyc_Absyn_Seq_s_struct*)
_temp642)->f2; goto _LL649;} else{ goto _LL650;} _LL650: if(( unsigned int)
_temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Return_s: 0){ _LL695: _temp694=((
struct Cyc_Absyn_Return_s_struct*) _temp642)->f1; if( _temp694 == 0){ goto
_LL651;} else{ goto _LL652;}} else{ goto _LL652;} _LL652: if(( unsigned int)
_temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Return_s: 0){ _LL697: _temp696=((
struct Cyc_Absyn_Return_s_struct*) _temp642)->f1; goto _LL653;} else{ goto
_LL654;} _LL654: if(( unsigned int) _temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_IfThenElse_s:
0){ _LL703: _temp702=(( struct Cyc_Absyn_IfThenElse_s_struct*) _temp642)->f1;
goto _LL701; _LL701: _temp700=(( struct Cyc_Absyn_IfThenElse_s_struct*) _temp642)->f2;
goto _LL699; _LL699: _temp698=(( struct Cyc_Absyn_IfThenElse_s_struct*) _temp642)->f3;
goto _LL655;} else{ goto _LL656;} _LL656: if(( unsigned int) _temp642 > 1u?*((
int*) _temp642) == Cyc_Absyn_While_s: 0){ _LL707: _temp706=(( struct Cyc_Absyn_While_s_struct*)
_temp642)->f1; _LL709: _temp708= _temp706.f1; goto _LL705; _LL705: _temp704=((
struct Cyc_Absyn_While_s_struct*) _temp642)->f2; goto _LL657;} else{ goto _LL658;}
_LL658: if(( unsigned int) _temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Break_s:
0){ goto _LL659;} else{ goto _LL660;} _LL660: if(( unsigned int) _temp642 > 1u?*((
int*) _temp642) == Cyc_Absyn_Continue_s: 0){ goto _LL661;} else{ goto _LL662;}
_LL662: if(( unsigned int) _temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Goto_s:
0){ _LL711: _temp710=(( struct Cyc_Absyn_Goto_s_struct*) _temp642)->f1; goto
_LL663;} else{ goto _LL664;} _LL664: if(( unsigned int) _temp642 > 1u?*(( int*)
_temp642) == Cyc_Absyn_For_s: 0){ _LL723: _temp722=(( struct Cyc_Absyn_For_s_struct*)
_temp642)->f1; goto _LL719; _LL719: _temp718=(( struct Cyc_Absyn_For_s_struct*)
_temp642)->f2; _LL721: _temp720= _temp718.f1; goto _LL715; _LL715: _temp714=((
struct Cyc_Absyn_For_s_struct*) _temp642)->f3; _LL717: _temp716= _temp714.f1;
goto _LL713; _LL713: _temp712=(( struct Cyc_Absyn_For_s_struct*) _temp642)->f4;
goto _LL665;} else{ goto _LL666;} _LL666: if(( unsigned int) _temp642 > 1u?*((
int*) _temp642) == Cyc_Absyn_Switch_s: 0){ _LL727: _temp726=(( struct Cyc_Absyn_Switch_s_struct*)
_temp642)->f1; goto _LL725; _LL725: _temp724=(( struct Cyc_Absyn_Switch_s_struct*)
_temp642)->f2; goto _LL667;} else{ goto _LL668;} _LL668: if(( unsigned int)
_temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Decl_s: 0){ _LL731: _temp730=((
struct Cyc_Absyn_Decl_s_struct*) _temp642)->f1; goto _LL729; _LL729: _temp728=((
struct Cyc_Absyn_Decl_s_struct*) _temp642)->f2; goto _LL669;} else{ goto _LL670;}
_LL670: if(( unsigned int) _temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Label_s:
0){ _LL735: _temp734=(( struct Cyc_Absyn_Label_s_struct*) _temp642)->f1; goto
_LL733; _LL733: _temp732=(( struct Cyc_Absyn_Label_s_struct*) _temp642)->f2;
goto _LL671;} else{ goto _LL672;} _LL672: if(( unsigned int) _temp642 > 1u?*((
int*) _temp642) == Cyc_Absyn_Do_s: 0){ _LL741: _temp740=(( struct Cyc_Absyn_Do_s_struct*)
_temp642)->f1; goto _LL737; _LL737: _temp736=(( struct Cyc_Absyn_Do_s_struct*)
_temp642)->f2; _LL739: _temp738= _temp736.f1; goto _LL673;} else{ goto _LL674;}
_LL674: if(( unsigned int) _temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_SwitchC_s:
0){ _LL745: _temp744=(( struct Cyc_Absyn_SwitchC_s_struct*) _temp642)->f1; goto
_LL743; _LL743: _temp742=(( struct Cyc_Absyn_SwitchC_s_struct*) _temp642)->f2;
goto _LL675;} else{ goto _LL676;} _LL676: if(( unsigned int) _temp642 > 1u?*((
int*) _temp642) == Cyc_Absyn_Fallthru_s: 0){ _LL747: _temp746=(( struct Cyc_Absyn_Fallthru_s_struct*)
_temp642)->f1; if( _temp746 == 0){ goto _LL677;} else{ goto _LL678;}} else{ goto
_LL678;} _LL678: if(( unsigned int) _temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Fallthru_s:
0){ _LL749: _temp748=(( struct Cyc_Absyn_Fallthru_s_struct*) _temp642)->f1; goto
_LL679;} else{ goto _LL680;} _LL680: if(( unsigned int) _temp642 > 1u?*(( int*)
_temp642) == Cyc_Absyn_TryCatch_s: 0){ _LL753: _temp752=(( struct Cyc_Absyn_TryCatch_s_struct*)
_temp642)->f1; goto _LL751; _LL751: _temp750=(( struct Cyc_Absyn_TryCatch_s_struct*)
_temp642)->f2; goto _LL681;} else{ goto _LL682;} _LL682: if(( unsigned int)
_temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Region_s: 0){ _LL759: _temp758=((
struct Cyc_Absyn_Region_s_struct*) _temp642)->f1; goto _LL757; _LL757: _temp756=((
struct Cyc_Absyn_Region_s_struct*) _temp642)->f2; goto _LL755; _LL755: _temp754=((
struct Cyc_Absyn_Region_s_struct*) _temp642)->f3; goto _LL683;} else{ goto
_LL684;} _LL684: if(( unsigned int) _temp642 > 1u?*(( int*) _temp642) == Cyc_Absyn_Cut_s:
0){ _LL761: _temp760=(( struct Cyc_Absyn_Cut_s_struct*) _temp642)->f1; goto
_LL685;} else{ goto _LL686;} _LL686: if(( unsigned int) _temp642 > 1u?*(( int*)
_temp642) == Cyc_Absyn_Splice_s: 0){ _LL763: _temp762=(( struct Cyc_Absyn_Splice_s_struct*)
_temp642)->f1; goto _LL687;} else{ goto _LL643;} _LL645: Cyc_Absyndump_dump_semi();
goto _LL643; _LL647: Cyc_Absyndump_dumpexp( _temp688); Cyc_Absyndump_dump_semi();
goto _LL643; _LL649: if( Cyc_Absynpp_is_declaration( _temp692)){ Cyc_Absyndump_dump_char((
int)'{'); Cyc_Absyndump_dumpstmt( _temp692); Cyc_Absyndump_dump_char(( int)'}');}
else{ Cyc_Absyndump_dumpstmt( _temp692);} if( Cyc_Absynpp_is_declaration(
_temp690)){ Cyc_Absyndump_dump_char(( int)'{'); Cyc_Absyndump_dumpstmt( _temp690);
Cyc_Absyndump_dump_char(( int)'}');} else{ Cyc_Absyndump_dumpstmt( _temp690);}
goto _LL643; _LL651: Cyc_Absyndump_dump( _tag_arr("return;", sizeof(
unsigned char), 8u)); goto _LL643; _LL653: Cyc_Absyndump_dump( _tag_arr("return",
sizeof( unsigned char), 7u)); Cyc_Absyndump_dumpexp(( struct Cyc_Absyn_Exp*)
_check_null( _temp696)); Cyc_Absyndump_dump_semi(); goto _LL643; _LL655: Cyc_Absyndump_dump(
_tag_arr("if(", sizeof( unsigned char), 4u)); Cyc_Absyndump_dumpexp( _temp702);
Cyc_Absyndump_dump_nospace( _tag_arr("){", sizeof( unsigned char), 3u)); Cyc_Absyndump_dumpstmt(
_temp700); Cyc_Absyndump_dump_char(( int)'}');{ void* _temp764=( void*) _temp698->r;
_LL766: if( _temp764 ==( void*) Cyc_Absyn_Skip_s){ goto _LL767;} else{ goto
_LL768;} _LL768: goto _LL769; _LL767: goto _LL765; _LL769: Cyc_Absyndump_dump(
_tag_arr("else{", sizeof( unsigned char), 6u)); Cyc_Absyndump_dumpstmt( _temp698);
Cyc_Absyndump_dump_char(( int)'}'); goto _LL765; _LL765:;} goto _LL643; _LL657:
Cyc_Absyndump_dump( _tag_arr("while(", sizeof( unsigned char), 7u)); Cyc_Absyndump_dumpexp(
_temp708); Cyc_Absyndump_dump_nospace( _tag_arr(") {", sizeof( unsigned char), 4u));
Cyc_Absyndump_dumpstmt( _temp704); Cyc_Absyndump_dump_char(( int)'}'); goto
_LL643; _LL659: Cyc_Absyndump_dump( _tag_arr("break;", sizeof( unsigned char), 7u));
goto _LL643; _LL661: Cyc_Absyndump_dump( _tag_arr("continue;", sizeof(
unsigned char), 10u)); goto _LL643; _LL663: Cyc_Absyndump_dump( _tag_arr("goto",
sizeof( unsigned char), 5u)); Cyc_Absyndump_dump_str( _temp710); Cyc_Absyndump_dump_semi();
goto _LL643; _LL665: Cyc_Absyndump_dump( _tag_arr("for(", sizeof( unsigned char),
5u)); Cyc_Absyndump_dumpexp( _temp722); Cyc_Absyndump_dump_semi(); Cyc_Absyndump_dumpexp(
_temp720); Cyc_Absyndump_dump_semi(); Cyc_Absyndump_dumpexp( _temp716); Cyc_Absyndump_dump_nospace(
_tag_arr("){", sizeof( unsigned char), 3u)); Cyc_Absyndump_dumpstmt( _temp712);
Cyc_Absyndump_dump_char(( int)'}'); goto _LL643; _LL667: Cyc_Absyndump_dump(
_tag_arr("switch(", sizeof( unsigned char), 8u)); Cyc_Absyndump_dumpexp(
_temp726); Cyc_Absyndump_dump_nospace( _tag_arr("){", sizeof( unsigned char), 3u));
Cyc_Absyndump_dumpswitchclauses( _temp724); Cyc_Absyndump_dump_char(( int)'}');
goto _LL643; _LL669: Cyc_Absyndump_dumpdecl( _temp730); Cyc_Absyndump_dumpstmt(
_temp728); goto _LL643; _LL671: if( Cyc_Absynpp_is_declaration( _temp732)){ Cyc_Absyndump_dump_str(
_temp734); Cyc_Absyndump_dump_nospace( _tag_arr(": {", sizeof( unsigned char), 4u));
Cyc_Absyndump_dumpstmt( _temp732); Cyc_Absyndump_dump_char(( int)'}');} else{
Cyc_Absyndump_dump_str( _temp734); Cyc_Absyndump_dump_char(( int)':'); Cyc_Absyndump_dumpstmt(
_temp732);} goto _LL643; _LL673: Cyc_Absyndump_dump( _tag_arr("do {", sizeof(
unsigned char), 5u)); Cyc_Absyndump_dumpstmt( _temp740); Cyc_Absyndump_dump_nospace(
_tag_arr("} while (", sizeof( unsigned char), 10u)); Cyc_Absyndump_dumpexp(
_temp738); Cyc_Absyndump_dump_nospace( _tag_arr(");", sizeof( unsigned char), 3u));
goto _LL643; _LL675: Cyc_Absyndump_dump( _tag_arr("switch \"C\" (", sizeof(
unsigned char), 13u)); Cyc_Absyndump_dumpexp( _temp744); Cyc_Absyndump_dump_nospace(
_tag_arr("){", sizeof( unsigned char), 3u)); for( 0; _temp742 != 0; _temp742=((
struct Cyc_List_List*) _check_null( _temp742))->tl){ struct Cyc_Absyn_SwitchC_clause
_temp772; struct Cyc_Absyn_Stmt* _temp773; struct Cyc_Absyn_Exp* _temp775;
struct Cyc_Absyn_SwitchC_clause* _temp770=( struct Cyc_Absyn_SwitchC_clause*)((
struct Cyc_List_List*) _check_null( _temp742))->hd; _temp772=* _temp770; _LL776:
_temp775= _temp772.cnst_exp; goto _LL774; _LL774: _temp773= _temp772.body; goto
_LL771; _LL771: if( _temp775 == 0){ Cyc_Absyndump_dump( _tag_arr("default: ",
sizeof( unsigned char), 10u));} else{ Cyc_Absyndump_dump( _tag_arr("case ",
sizeof( unsigned char), 6u)); Cyc_Absyndump_dumpexp(( struct Cyc_Absyn_Exp*)
_check_null( _temp775)); Cyc_Absyndump_dump_char(( int)':');} Cyc_Absyndump_dumpstmt(
_temp773);} Cyc_Absyndump_dump_char(( int)'}'); goto _LL643; _LL677: Cyc_Absyndump_dump(
_tag_arr("fallthru;", sizeof( unsigned char), 10u)); goto _LL643; _LL679: Cyc_Absyndump_dump(
_tag_arr("fallthru(", sizeof( unsigned char), 10u)); Cyc_Absyndump_dumpexps_prec(
20, _temp748); Cyc_Absyndump_dump_nospace( _tag_arr(");", sizeof( unsigned char),
3u)); goto _LL643; _LL681: Cyc_Absyndump_dump( _tag_arr("try", sizeof(
unsigned char), 4u)); Cyc_Absyndump_dumpstmt( _temp752); Cyc_Absyndump_dump(
_tag_arr("catch {", sizeof( unsigned char), 8u)); Cyc_Absyndump_dumpswitchclauses(
_temp750); Cyc_Absyndump_dump_char(( int)'}'); goto _LL643; _LL683: Cyc_Absyndump_dump(
_tag_arr("region<", sizeof( unsigned char), 8u)); Cyc_Absyndump_dumptvar(
_temp758); Cyc_Absyndump_dump( _tag_arr("> ", sizeof( unsigned char), 3u)); Cyc_Absyndump_dumpqvar(
_temp756->name); Cyc_Absyndump_dump( _tag_arr("{", sizeof( unsigned char), 2u));
Cyc_Absyndump_dumpstmt( _temp754); Cyc_Absyndump_dump( _tag_arr("}", sizeof(
unsigned char), 2u)); goto _LL643; _LL685: Cyc_Absyndump_dump( _tag_arr("cut",
sizeof( unsigned char), 4u)); Cyc_Absyndump_dumpstmt( _temp760); goto _LL643;
_LL687: Cyc_Absyndump_dump( _tag_arr("splice", sizeof( unsigned char), 7u)); Cyc_Absyndump_dumpstmt(
_temp762); goto _LL643; _LL643:;} struct _tuple9{ struct Cyc_List_List* f1;
struct Cyc_Absyn_Pat* f2; } ; void Cyc_Absyndump_dumpdp( struct _tuple9* dp){
Cyc_Absyndump_egroup( Cyc_Absyndump_dumpdesignator,(* dp).f1, _tag_arr("",
sizeof( unsigned char), 1u), _tag_arr("=", sizeof( unsigned char), 2u), _tag_arr("=",
sizeof( unsigned char), 2u)); Cyc_Absyndump_dumppat((* dp).f2);} void Cyc_Absyndump_dumppat(
struct Cyc_Absyn_Pat* p){ void* _temp777=( void*) p->r; int _temp811; void*
_temp813; int _temp815; void* _temp817; unsigned char _temp819; struct
_tagged_arr _temp821; struct Cyc_Absyn_Vardecl* _temp823; struct Cyc_List_List*
_temp825; struct Cyc_Absyn_Pat* _temp827; struct Cyc_Absyn_Vardecl* _temp829;
struct _tuple0* _temp831; struct Cyc_List_List* _temp833; struct Cyc_List_List*
_temp835; struct _tuple0* _temp837; struct Cyc_List_List* _temp839; struct Cyc_List_List*
_temp841; struct _tuple0* _temp843; struct Cyc_List_List* _temp845; struct Cyc_List_List*
_temp847; struct Cyc_Absyn_Structdecl* _temp849; struct Cyc_List_List* _temp851;
struct Cyc_List_List* _temp853; struct Cyc_Absyn_Tunionfield* _temp855; struct
Cyc_Absyn_Enumfield* _temp857; _LL779: if( _temp777 ==( void*) Cyc_Absyn_Wild_p){
goto _LL780;} else{ goto _LL781;} _LL781: if( _temp777 ==( void*) Cyc_Absyn_Null_p){
goto _LL782;} else{ goto _LL783;} _LL783: if(( unsigned int) _temp777 > 2u?*((
int*) _temp777) == Cyc_Absyn_Int_p: 0){ _LL814: _temp813=( void*)(( struct Cyc_Absyn_Int_p_struct*)
_temp777)->f1; if( _temp813 ==( void*) Cyc_Absyn_Signed){ goto _LL812;} else{
goto _LL785;} _LL812: _temp811=(( struct Cyc_Absyn_Int_p_struct*) _temp777)->f2;
goto _LL784;} else{ goto _LL785;} _LL785: if(( unsigned int) _temp777 > 2u?*((
int*) _temp777) == Cyc_Absyn_Int_p: 0){ _LL818: _temp817=( void*)(( struct Cyc_Absyn_Int_p_struct*)
_temp777)->f1; if( _temp817 ==( void*) Cyc_Absyn_Unsigned){ goto _LL816;} else{
goto _LL787;} _LL816: _temp815=(( struct Cyc_Absyn_Int_p_struct*) _temp777)->f2;
goto _LL786;} else{ goto _LL787;} _LL787: if(( unsigned int) _temp777 > 2u?*((
int*) _temp777) == Cyc_Absyn_Char_p: 0){ _LL820: _temp819=(( struct Cyc_Absyn_Char_p_struct*)
_temp777)->f1; goto _LL788;} else{ goto _LL789;} _LL789: if(( unsigned int)
_temp777 > 2u?*(( int*) _temp777) == Cyc_Absyn_Float_p: 0){ _LL822: _temp821=((
struct Cyc_Absyn_Float_p_struct*) _temp777)->f1; goto _LL790;} else{ goto _LL791;}
_LL791: if(( unsigned int) _temp777 > 2u?*(( int*) _temp777) == Cyc_Absyn_Var_p:
0){ _LL824: _temp823=(( struct Cyc_Absyn_Var_p_struct*) _temp777)->f1; goto
_LL792;} else{ goto _LL793;} _LL793: if(( unsigned int) _temp777 > 2u?*(( int*)
_temp777) == Cyc_Absyn_Tuple_p: 0){ _LL826: _temp825=(( struct Cyc_Absyn_Tuple_p_struct*)
_temp777)->f1; goto _LL794;} else{ goto _LL795;} _LL795: if(( unsigned int)
_temp777 > 2u?*(( int*) _temp777) == Cyc_Absyn_Pointer_p: 0){ _LL828: _temp827=((
struct Cyc_Absyn_Pointer_p_struct*) _temp777)->f1; goto _LL796;} else{ goto
_LL797;} _LL797: if(( unsigned int) _temp777 > 2u?*(( int*) _temp777) == Cyc_Absyn_Reference_p:
0){ _LL830: _temp829=(( struct Cyc_Absyn_Reference_p_struct*) _temp777)->f1;
goto _LL798;} else{ goto _LL799;} _LL799: if(( unsigned int) _temp777 > 2u?*((
int*) _temp777) == Cyc_Absyn_UnknownId_p: 0){ _LL832: _temp831=(( struct Cyc_Absyn_UnknownId_p_struct*)
_temp777)->f1; goto _LL800;} else{ goto _LL801;} _LL801: if(( unsigned int)
_temp777 > 2u?*(( int*) _temp777) == Cyc_Absyn_UnknownCall_p: 0){ _LL838:
_temp837=(( struct Cyc_Absyn_UnknownCall_p_struct*) _temp777)->f1; goto _LL836;
_LL836: _temp835=(( struct Cyc_Absyn_UnknownCall_p_struct*) _temp777)->f2; goto
_LL834; _LL834: _temp833=(( struct Cyc_Absyn_UnknownCall_p_struct*) _temp777)->f3;
goto _LL802;} else{ goto _LL803;} _LL803: if(( unsigned int) _temp777 > 2u?*((
int*) _temp777) == Cyc_Absyn_UnknownFields_p: 0){ _LL844: _temp843=(( struct Cyc_Absyn_UnknownFields_p_struct*)
_temp777)->f1; goto _LL842; _LL842: _temp841=(( struct Cyc_Absyn_UnknownFields_p_struct*)
_temp777)->f2; goto _LL840; _LL840: _temp839=(( struct Cyc_Absyn_UnknownFields_p_struct*)
_temp777)->f3; goto _LL804;} else{ goto _LL805;} _LL805: if(( unsigned int)
_temp777 > 2u?*(( int*) _temp777) == Cyc_Absyn_Struct_p: 0){ _LL850: _temp849=((
struct Cyc_Absyn_Struct_p_struct*) _temp777)->f1; goto _LL848; _LL848: _temp847=((
struct Cyc_Absyn_Struct_p_struct*) _temp777)->f3; goto _LL846; _LL846: _temp845=((
struct Cyc_Absyn_Struct_p_struct*) _temp777)->f4; goto _LL806;} else{ goto
_LL807;} _LL807: if(( unsigned int) _temp777 > 2u?*(( int*) _temp777) == Cyc_Absyn_Tunion_p:
0){ _LL856: _temp855=(( struct Cyc_Absyn_Tunion_p_struct*) _temp777)->f2; goto
_LL854; _LL854: _temp853=(( struct Cyc_Absyn_Tunion_p_struct*) _temp777)->f3;
goto _LL852; _LL852: _temp851=(( struct Cyc_Absyn_Tunion_p_struct*) _temp777)->f4;
goto _LL808;} else{ goto _LL809;} _LL809: if(( unsigned int) _temp777 > 2u?*((
int*) _temp777) == Cyc_Absyn_Enum_p: 0){ _LL858: _temp857=(( struct Cyc_Absyn_Enum_p_struct*)
_temp777)->f2; goto _LL810;} else{ goto _LL778;} _LL780: Cyc_Absyndump_dump_char((
int)'_'); goto _LL778; _LL782: Cyc_Absyndump_dump( _tag_arr("null", sizeof(
unsigned char), 5u)); goto _LL778; _LL784: Cyc_Absyndump_dump(( struct
_tagged_arr)({ struct _tagged_arr(* _temp859)( struct _tagged_arr fmt, struct
_tagged_arr)= Cyc_Stdio_aprintf; struct _tagged_arr _temp863= _tag_arr("%d",
sizeof( unsigned char), 3u); struct Cyc_Stdio_Int_pa_struct _temp864; _temp864.tag=
Cyc_Stdio_Int_pa; _temp864.f1=( int)(( unsigned int) _temp811);{ void* _temp862=(
void*)& _temp864; void* _temp860[ 1u]={ _temp862}; struct _tagged_arr _temp861={(
void*) _temp860,( void*) _temp860,( void*)( _temp860 + 1u)}; _temp859( _temp863,
_temp861);}})); goto _LL778; _LL786: Cyc_Absyndump_dump(( struct _tagged_arr)({
struct _tagged_arr(* _temp865)( struct _tagged_arr fmt, struct _tagged_arr)= Cyc_Stdio_aprintf;
struct _tagged_arr _temp869= _tag_arr("%u", sizeof( unsigned char), 3u); struct
Cyc_Stdio_Int_pa_struct _temp870; _temp870.tag= Cyc_Stdio_Int_pa; _temp870.f1=(
unsigned int) _temp815;{ void* _temp868=( void*)& _temp870; void* _temp866[ 1u]={
_temp868}; struct _tagged_arr _temp867={( void*) _temp866,( void*) _temp866,(
void*)( _temp866 + 1u)}; _temp865( _temp869, _temp867);}})); goto _LL778; _LL788:
Cyc_Absyndump_dump( _tag_arr("'", sizeof( unsigned char), 2u)); Cyc_Absyndump_dump_nospace(
Cyc_Absynpp_char_escape( _temp819)); Cyc_Absyndump_dump_nospace( _tag_arr("'",
sizeof( unsigned char), 2u)); goto _LL778; _LL790: Cyc_Absyndump_dump( _temp821);
goto _LL778; _LL792: Cyc_Absyndump_dumpqvar( _temp823->name); goto _LL778;
_LL794:(( void(*)( void(* f)( struct Cyc_Absyn_Pat*), struct Cyc_List_List* l,
struct _tagged_arr start, struct _tagged_arr end, struct _tagged_arr sep)) Cyc_Absyndump_group)(
Cyc_Absyndump_dumppat, _temp825, _tag_arr("$(", sizeof( unsigned char), 3u),
_tag_arr(")", sizeof( unsigned char), 2u), _tag_arr(",", sizeof( unsigned char),
2u)); goto _LL778; _LL796: Cyc_Absyndump_dump( _tag_arr("&", sizeof(
unsigned char), 2u)); Cyc_Absyndump_dumppat( _temp827); goto _LL778; _LL798: Cyc_Absyndump_dump(
_tag_arr("*", sizeof( unsigned char), 2u)); Cyc_Absyndump_dumpqvar( _temp829->name);
goto _LL778; _LL800: Cyc_Absyndump_dumpqvar( _temp831); goto _LL778; _LL802: Cyc_Absyndump_dumpqvar(
_temp837); Cyc_Absyndump_dumptvars( _temp835);(( void(*)( void(* f)( struct Cyc_Absyn_Pat*),
struct Cyc_List_List* l, struct _tagged_arr start, struct _tagged_arr end,
struct _tagged_arr sep)) Cyc_Absyndump_group)( Cyc_Absyndump_dumppat, _temp833,
_tag_arr("(", sizeof( unsigned char), 2u), _tag_arr(")", sizeof( unsigned char),
2u), _tag_arr(",", sizeof( unsigned char), 2u)); goto _LL778; _LL804: Cyc_Absyndump_dumpqvar(
_temp843); Cyc_Absyndump_dumptvars( _temp841);(( void(*)( void(* f)( struct
_tuple9*), struct Cyc_List_List* l, struct _tagged_arr start, struct _tagged_arr
end, struct _tagged_arr sep)) Cyc_Absyndump_group)( Cyc_Absyndump_dumpdp,
_temp839, _tag_arr("{", sizeof( unsigned char), 2u), _tag_arr("}", sizeof(
unsigned char), 2u), _tag_arr(",", sizeof( unsigned char), 2u)); goto _LL778;
_LL806: if( _temp849->name != 0){ Cyc_Absyndump_dumpqvar(( struct _tuple0*)((
struct Cyc_Core_Opt*) _check_null( _temp849->name))->v);} Cyc_Absyndump_dumptvars(
_temp847);(( void(*)( void(* f)( struct _tuple9*), struct Cyc_List_List* l,
struct _tagged_arr start, struct _tagged_arr end, struct _tagged_arr sep)) Cyc_Absyndump_group)(
Cyc_Absyndump_dumpdp, _temp845, _tag_arr("{", sizeof( unsigned char), 2u),
_tag_arr("}", sizeof( unsigned char), 2u), _tag_arr(",", sizeof( unsigned char),
2u)); goto _LL778; _LL808: Cyc_Absyndump_dumpqvar( _temp855->name); Cyc_Absyndump_dumptvars(
_temp853); if( _temp851 != 0){(( void(*)( void(* f)( struct Cyc_Absyn_Pat*),
struct Cyc_List_List* l, struct _tagged_arr start, struct _tagged_arr end,
struct _tagged_arr sep)) Cyc_Absyndump_group)( Cyc_Absyndump_dumppat, _temp851,
_tag_arr("(", sizeof( unsigned char), 2u), _tag_arr(")", sizeof( unsigned char),
2u), _tag_arr(",", sizeof( unsigned char), 2u));} goto _LL778; _LL810: Cyc_Absyndump_dumpqvar(
_temp857->name); goto _LL778; _LL778:;} void Cyc_Absyndump_dumptunionfield(
struct Cyc_Absyn_Tunionfield* ef){ Cyc_Absyndump_dumpqvar( ef->name); if( ef->typs
!= 0){ Cyc_Absyndump_dumpargs( ef->typs);}} void Cyc_Absyndump_dumptunionfields(
struct Cyc_List_List* fields){(( void(*)( void(* f)( struct Cyc_Absyn_Tunionfield*),
struct Cyc_List_List* l, struct _tagged_arr sep)) Cyc_Absyndump_dump_sep)( Cyc_Absyndump_dumptunionfield,
fields, _tag_arr(",", sizeof( unsigned char), 2u));} void Cyc_Absyndump_dumpenumfield(
struct Cyc_Absyn_Enumfield* ef){ Cyc_Absyndump_dumpqvar( ef->name); if( ef->tag
!= 0){ Cyc_Absyndump_dump( _tag_arr(" = ", sizeof( unsigned char), 4u)); Cyc_Absyndump_dumpexp((
struct Cyc_Absyn_Exp*) _check_null( ef->tag));}} void Cyc_Absyndump_dumpenumfields(
struct Cyc_List_List* fields){(( void(*)( void(* f)( struct Cyc_Absyn_Enumfield*),
struct Cyc_List_List* l, struct _tagged_arr sep)) Cyc_Absyndump_dump_sep)( Cyc_Absyndump_dumpenumfield,
fields, _tag_arr(",", sizeof( unsigned char), 2u));} void Cyc_Absyndump_dumpstructfields(
struct Cyc_List_List* fields){ for( 0; fields != 0; fields=(( struct Cyc_List_List*)
_check_null( fields))->tl){ struct Cyc_Absyn_Structfield _temp873; struct Cyc_List_List*
_temp874; struct Cyc_Core_Opt* _temp876; void* _temp878; struct Cyc_Absyn_Tqual
_temp880; struct _tagged_arr* _temp882; struct Cyc_Absyn_Structfield* _temp871=(
struct Cyc_Absyn_Structfield*)(( struct Cyc_List_List*) _check_null( fields))->hd;
_temp873=* _temp871; _LL883: _temp882= _temp873.name; goto _LL881; _LL881:
_temp880= _temp873.tq; goto _LL879; _LL879: _temp878=( void*) _temp873.type;
goto _LL877; _LL877: _temp876= _temp873.width; goto _LL875; _LL875: _temp874=
_temp873.attributes; goto _LL872; _LL872:(( void(*)( struct Cyc_Absyn_Tqual,
void*, void(* f)( struct _tagged_arr*), struct _tagged_arr*)) Cyc_Absyndump_dumptqtd)(
_temp880, _temp878, Cyc_Absyndump_dump_str, _temp882); Cyc_Absyndump_dumpatts(
_temp874); if( _temp876 != 0){ Cyc_Absyndump_dump_char(( int)':'); Cyc_Absyndump_dumpexp((
struct Cyc_Absyn_Exp*)(( struct Cyc_Core_Opt*) _check_null( _temp876))->v);} Cyc_Absyndump_dump_semi();}}
void Cyc_Absyndump_dumptypedefname( struct Cyc_Absyn_Typedefdecl* td){ Cyc_Absyndump_dumpqvar(
td->name); Cyc_Absyndump_dumptvars( td->tvs);} static void Cyc_Absyndump_dump_atts_qvar(
struct Cyc_Absyn_Fndecl* fd){ Cyc_Absyndump_dumpatts( fd->attributes); Cyc_Absyndump_dumpqvar(
fd->name);} struct _tuple10{ void* f1; struct _tuple0* f2; } ; static void Cyc_Absyndump_dump_callconv_qvar(
struct _tuple10* pr){{ void* _temp884=(* pr).f1; _LL886: if( _temp884 ==( void*)
Cyc_Absyn_Unused_att){ goto _LL887;} else{ goto _LL888;} _LL888: if( _temp884 ==(
void*) Cyc_Absyn_Stdcall_att){ goto _LL889;} else{ goto _LL890;} _LL890: if(
_temp884 ==( void*) Cyc_Absyn_Cdecl_att){ goto _LL891;} else{ goto _LL892;}
_LL892: if( _temp884 ==( void*) Cyc_Absyn_Fastcall_att){ goto _LL893;} else{
goto _LL894;} _LL894: goto _LL895; _LL887: goto _LL885; _LL889: Cyc_Absyndump_dump(
_tag_arr("_stdcall", sizeof( unsigned char), 9u)); goto _LL885; _LL891: Cyc_Absyndump_dump(
_tag_arr("_cdecl", sizeof( unsigned char), 7u)); goto _LL885; _LL893: Cyc_Absyndump_dump(
_tag_arr("_fastcall", sizeof( unsigned char), 10u)); goto _LL885; _LL895: goto
_LL885; _LL885:;} Cyc_Absyndump_dumpqvar((* pr).f2);} static void Cyc_Absyndump_dump_callconv_fdqvar(
struct Cyc_Absyn_Fndecl* fd){ Cyc_Absyndump_dump_callconv( fd->attributes); Cyc_Absyndump_dumpqvar(
fd->name);} static void Cyc_Absyndump_dumpids( struct Cyc_List_List* vds){ for(
0; vds != 0; vds=(( struct Cyc_List_List*) _check_null( vds))->tl){ Cyc_Absyndump_dumpqvar(((
struct Cyc_Absyn_Vardecl*)(( struct Cyc_List_List*) _check_null( vds))->hd)->name);
if((( struct Cyc_List_List*) _check_null( vds))->tl != 0){ Cyc_Absyndump_dump_char((
int)',');}}} void Cyc_Absyndump_dumpdecl( struct Cyc_Absyn_Decl* d){ void*
_temp896=( void*) d->r; struct Cyc_Absyn_Fndecl* _temp922; struct Cyc_Absyn_Structdecl*
_temp924; struct Cyc_Absyn_Uniondecl* _temp926; struct Cyc_Absyn_Vardecl*
_temp928; struct Cyc_Absyn_Vardecl _temp930; struct Cyc_List_List* _temp931;
struct Cyc_Absyn_Exp* _temp933; void* _temp935; struct Cyc_Absyn_Tqual _temp937;
struct _tuple0* _temp939; void* _temp941; struct Cyc_Absyn_Tuniondecl* _temp943;
struct Cyc_Absyn_Tuniondecl _temp945; int _temp946; struct Cyc_Core_Opt*
_temp948; struct Cyc_List_List* _temp950; struct _tuple0* _temp952; void*
_temp954; struct Cyc_Absyn_Enumdecl* _temp956; struct Cyc_Absyn_Enumdecl
_temp958; struct Cyc_Core_Opt* _temp959; struct _tuple0* _temp961; void*
_temp963; struct Cyc_Absyn_Exp* _temp965; struct Cyc_Absyn_Pat* _temp967; struct
Cyc_List_List* _temp969; struct Cyc_Absyn_Typedefdecl* _temp971; struct Cyc_List_List*
_temp973; struct _tagged_arr* _temp975; struct Cyc_List_List* _temp977; struct
_tuple0* _temp979; struct Cyc_List_List* _temp981; _LL898: if(*(( int*) _temp896)
== Cyc_Absyn_Fn_d){ _LL923: _temp922=(( struct Cyc_Absyn_Fn_d_struct*) _temp896)->f1;
goto _LL899;} else{ goto _LL900;} _LL900: if(*(( int*) _temp896) == Cyc_Absyn_Struct_d){
_LL925: _temp924=(( struct Cyc_Absyn_Struct_d_struct*) _temp896)->f1; goto
_LL901;} else{ goto _LL902;} _LL902: if(*(( int*) _temp896) == Cyc_Absyn_Union_d){
_LL927: _temp926=(( struct Cyc_Absyn_Union_d_struct*) _temp896)->f1; goto _LL903;}
else{ goto _LL904;} _LL904: if(*(( int*) _temp896) == Cyc_Absyn_Var_d){ _LL929:
_temp928=(( struct Cyc_Absyn_Var_d_struct*) _temp896)->f1; _temp930=* _temp928;
_LL942: _temp941=( void*) _temp930.sc; goto _LL940; _LL940: _temp939= _temp930.name;
goto _LL938; _LL938: _temp937= _temp930.tq; goto _LL936; _LL936: _temp935=( void*)
_temp930.type; goto _LL934; _LL934: _temp933= _temp930.initializer; goto _LL932;
_LL932: _temp931= _temp930.attributes; goto _LL905;} else{ goto _LL906;} _LL906:
if(*(( int*) _temp896) == Cyc_Absyn_Tunion_d){ _LL944: _temp943=(( struct Cyc_Absyn_Tunion_d_struct*)
_temp896)->f1; _temp945=* _temp943; _LL955: _temp954=( void*) _temp945.sc; goto
_LL953; _LL953: _temp952= _temp945.name; goto _LL951; _LL951: _temp950= _temp945.tvs;
goto _LL949; _LL949: _temp948= _temp945.fields; goto _LL947; _LL947: _temp946=
_temp945.is_xtunion; goto _LL907;} else{ goto _LL908;} _LL908: if(*(( int*)
_temp896) == Cyc_Absyn_Enum_d){ _LL957: _temp956=(( struct Cyc_Absyn_Enum_d_struct*)
_temp896)->f1; _temp958=* _temp956; _LL964: _temp963=( void*) _temp958.sc; goto
_LL962; _LL962: _temp961= _temp958.name; goto _LL960; _LL960: _temp959= _temp958.fields;
goto _LL909;} else{ goto _LL910;} _LL910: if(*(( int*) _temp896) == Cyc_Absyn_Let_d){
_LL968: _temp967=(( struct Cyc_Absyn_Let_d_struct*) _temp896)->f1; goto _LL966;
_LL966: _temp965=(( struct Cyc_Absyn_Let_d_struct*) _temp896)->f4; goto _LL911;}
else{ goto _LL912;} _LL912: if(*(( int*) _temp896) == Cyc_Absyn_Letv_d){ _LL970:
_temp969=(( struct Cyc_Absyn_Letv_d_struct*) _temp896)->f1; goto _LL913;} else{
goto _LL914;} _LL914: if(*(( int*) _temp896) == Cyc_Absyn_Typedef_d){ _LL972:
_temp971=(( struct Cyc_Absyn_Typedef_d_struct*) _temp896)->f1; goto _LL915;}
else{ goto _LL916;} _LL916: if(*(( int*) _temp896) == Cyc_Absyn_Namespace_d){
_LL976: _temp975=(( struct Cyc_Absyn_Namespace_d_struct*) _temp896)->f1; goto
_LL974; _LL974: _temp973=(( struct Cyc_Absyn_Namespace_d_struct*) _temp896)->f2;
goto _LL917;} else{ goto _LL918;} _LL918: if(*(( int*) _temp896) == Cyc_Absyn_Using_d){
_LL980: _temp979=(( struct Cyc_Absyn_Using_d_struct*) _temp896)->f1; goto _LL978;
_LL978: _temp977=(( struct Cyc_Absyn_Using_d_struct*) _temp896)->f2; goto _LL919;}
else{ goto _LL920;} _LL920: if(*(( int*) _temp896) == Cyc_Absyn_ExternC_d){
_LL982: _temp981=(( struct Cyc_Absyn_ExternC_d_struct*) _temp896)->f1; goto
_LL921;} else{ goto _LL897;} _LL899: if( Cyc_Absyndump_to_VC){ Cyc_Absyndump_dumpatts(
_temp922->attributes);} if( _temp922->is_inline){ if( Cyc_Absyndump_to_VC){ Cyc_Absyndump_dump(
_tag_arr("__inline", sizeof( unsigned char), 9u));} else{ Cyc_Absyndump_dump(
_tag_arr("inline", sizeof( unsigned char), 7u));}} Cyc_Absyndump_dumpscope((
void*) _temp922->sc);{ void* t=( void*)({ struct Cyc_Absyn_FnType_struct*
_temp984=( struct Cyc_Absyn_FnType_struct*) GC_malloc( sizeof( struct Cyc_Absyn_FnType_struct));
_temp984[ 0]=({ struct Cyc_Absyn_FnType_struct _temp985; _temp985.tag= Cyc_Absyn_FnType;
_temp985.f1=({ struct Cyc_Absyn_FnInfo _temp986; _temp986.tvars= _temp922->tvs;
_temp986.effect= _temp922->effect; _temp986.ret_typ=( void*)(( void*) _temp922->ret_type);
_temp986.args=(( struct Cyc_List_List*(*)( struct _tuple1*(* f)( struct _tuple3*),
struct Cyc_List_List* x)) Cyc_List_map)( Cyc_Absynpp_arg_mk_opt, _temp922->args);
_temp986.c_varargs= _temp922->c_varargs; _temp986.cyc_varargs= _temp922->cyc_varargs;
_temp986.rgn_po= _temp922->rgn_po; _temp986.attributes= 0; _temp986;}); _temp985;});
_temp984;});(( void(*)( struct Cyc_Absyn_Tqual, void*, void(* f)( struct Cyc_Absyn_Fndecl*),
struct Cyc_Absyn_Fndecl*)) Cyc_Absyndump_dumptqtd)(({ struct Cyc_Absyn_Tqual
_temp983; _temp983.q_const= 0; _temp983.q_volatile= 0; _temp983.q_restrict= 0;
_temp983;}), t, Cyc_Absyndump_to_VC? Cyc_Absyndump_dump_callconv_fdqvar: Cyc_Absyndump_dump_atts_qvar,
_temp922); Cyc_Absyndump_dump_char(( int)'{'); Cyc_Absyndump_dumpstmt( _temp922->body);
Cyc_Absyndump_dump_char(( int)'}'); goto _LL897;} _LL901: Cyc_Absyndump_dumpscope((
void*) _temp924->sc); Cyc_Absyndump_dump( _tag_arr("struct", sizeof(
unsigned char), 7u)); if( _temp924->name != 0){ Cyc_Absyndump_dumpqvar(( struct
_tuple0*)(( struct Cyc_Core_Opt*) _check_null( _temp924->name))->v);} Cyc_Absyndump_dumptvars(
_temp924->tvs); if( _temp924->fields == 0){ Cyc_Absyndump_dump_semi();} else{
Cyc_Absyndump_dump_char(( int)'{'); Cyc_Absyndump_dumpstructfields(( struct Cyc_List_List*)((
struct Cyc_Core_Opt*) _check_null( _temp924->fields))->v); Cyc_Absyndump_dump(
_tag_arr("}", sizeof( unsigned char), 2u)); Cyc_Absyndump_dumpatts( _temp924->attributes);
Cyc_Absyndump_dump( _tag_arr(";", sizeof( unsigned char), 2u));} goto _LL897;
_LL903: Cyc_Absyndump_dumpscope(( void*) _temp926->sc); Cyc_Absyndump_dump(
_tag_arr("union", sizeof( unsigned char), 6u)); if( _temp926->name != 0){ Cyc_Absyndump_dumpqvar((
struct _tuple0*)(( struct Cyc_Core_Opt*) _check_null( _temp926->name))->v);} Cyc_Absyndump_dumptvars(
_temp926->tvs); if( _temp926->fields == 0){ Cyc_Absyndump_dump_semi();} else{
Cyc_Absyndump_dump_char(( int)'{'); Cyc_Absyndump_dumpstructfields(( struct Cyc_List_List*)((
struct Cyc_Core_Opt*) _check_null( _temp926->fields))->v); Cyc_Absyndump_dump(
_tag_arr("}", sizeof( unsigned char), 2u)); Cyc_Absyndump_dumpatts( _temp926->attributes);
Cyc_Absyndump_dump( _tag_arr(";", sizeof( unsigned char), 2u));} goto _LL897;
_LL905: if( Cyc_Absyndump_to_VC){ Cyc_Absyndump_dumpatts( _temp931); Cyc_Absyndump_dumpscope(
_temp941);{ struct Cyc_List_List* _temp989; void* _temp991; struct Cyc_Absyn_Tqual
_temp993; struct _tuple4 _temp987= Cyc_Absynpp_to_tms( _temp937, _temp935);
_LL994: _temp993= _temp987.f1; goto _LL992; _LL992: _temp991= _temp987.f2; goto
_LL990; _LL990: _temp989= _temp987.f3; goto _LL988; _LL988: { void* call_conv=(
void*) Cyc_Absyn_Unused_att;{ struct Cyc_List_List* tms2= _temp989; for( 0; tms2
!= 0; tms2=(( struct Cyc_List_List*) _check_null( tms2))->tl){ void* _temp995=(
void*)(( struct Cyc_List_List*) _check_null( tms2))->hd; struct Cyc_List_List*
_temp1001; _LL997: if(( unsigned int) _temp995 > 1u?*(( int*) _temp995) == Cyc_Absyn_Attributes_mod:
0){ _LL1002: _temp1001=(( struct Cyc_Absyn_Attributes_mod_struct*) _temp995)->f2;
goto _LL998;} else{ goto _LL999;} _LL999: goto _LL1000; _LL998: for( 0;
_temp1001 != 0; _temp1001=(( struct Cyc_List_List*) _check_null( _temp1001))->tl){
void* _temp1003=( void*)(( struct Cyc_List_List*) _check_null( _temp1001))->hd;
_LL1005: if( _temp1003 ==( void*) Cyc_Absyn_Stdcall_att){ goto _LL1006;} else{
goto _LL1007;} _LL1007: if( _temp1003 ==( void*) Cyc_Absyn_Cdecl_att){ goto
_LL1008;} else{ goto _LL1009;} _LL1009: if( _temp1003 ==( void*) Cyc_Absyn_Fastcall_att){
goto _LL1010;} else{ goto _LL1011;} _LL1011: goto _LL1012; _LL1006: call_conv=(
void*) Cyc_Absyn_Stdcall_att; goto _LL1004; _LL1008: call_conv=( void*) Cyc_Absyn_Cdecl_att;
goto _LL1004; _LL1010: call_conv=( void*) Cyc_Absyn_Fastcall_att; goto _LL1004;
_LL1012: goto _LL1004; _LL1004:;} goto _LL996; _LL1000: goto _LL996; _LL996:;}}
Cyc_Absyndump_dumptq( _temp993); Cyc_Absyndump_dumpntyp( _temp991);{ struct
_tuple10 _temp1013=({ struct _tuple10 _temp1014; _temp1014.f1= call_conv;
_temp1014.f2= _temp939; _temp1014;});(( void(*)( struct Cyc_List_List* tms, void(*
f)( struct _tuple10*), struct _tuple10* a)) Cyc_Absyndump_dumptms)( Cyc_List_imp_rev(
_temp989), Cyc_Absyndump_dump_callconv_qvar,& _temp1013);}}}} else{ Cyc_Absyndump_dumpscope(
_temp941);(( void(*)( struct Cyc_Absyn_Tqual, void*, void(* f)( struct _tuple0*),
struct _tuple0*)) Cyc_Absyndump_dumptqtd)( _temp937, _temp935, Cyc_Absyndump_dumpqvar,
_temp939); Cyc_Absyndump_dumpatts( _temp931);} if( _temp933 != 0){ Cyc_Absyndump_dump_char((
int)'='); Cyc_Absyndump_dumpexp(( struct Cyc_Absyn_Exp*) _check_null( _temp933));}
Cyc_Absyndump_dump_semi(); goto _LL897; _LL907: Cyc_Absyndump_dumpscope(
_temp954); if( _temp946){ Cyc_Absyndump_dump( _tag_arr("xtunion ", sizeof(
unsigned char), 9u));} else{ Cyc_Absyndump_dump( _tag_arr("tunion ", sizeof(
unsigned char), 8u));} Cyc_Absyndump_dumpqvar( _temp952); Cyc_Absyndump_dumptvars(
_temp950); if( _temp948 == 0){ Cyc_Absyndump_dump_semi();} else{ Cyc_Absyndump_dump_char((
int)'{'); Cyc_Absyndump_dumptunionfields(( struct Cyc_List_List*)(( struct Cyc_Core_Opt*)
_check_null( _temp948))->v); Cyc_Absyndump_dump_nospace( _tag_arr("};", sizeof(
unsigned char), 3u));} goto _LL897; _LL909: Cyc_Absyndump_dumpscope( _temp963);
Cyc_Absyndump_dump( _tag_arr("enum ", sizeof( unsigned char), 6u)); Cyc_Absyndump_dumpqvar(
_temp961); if( _temp959 == 0){ Cyc_Absyndump_dump_semi();} else{ Cyc_Absyndump_dump_char((
int)'{'); Cyc_Absyndump_dumpenumfields(( struct Cyc_List_List*)(( struct Cyc_Core_Opt*)
_check_null( _temp959))->v); Cyc_Absyndump_dump_nospace( _tag_arr("};", sizeof(
unsigned char), 3u));} return; _LL911: Cyc_Absyndump_dump( _tag_arr("let",
sizeof( unsigned char), 4u)); Cyc_Absyndump_dumppat( _temp967); Cyc_Absyndump_dump_char((
int)'='); Cyc_Absyndump_dumpexp( _temp965); Cyc_Absyndump_dump_semi(); goto
_LL897; _LL913: Cyc_Absyndump_dump( _tag_arr("let ", sizeof( unsigned char), 5u));
Cyc_Absyndump_dumpids( _temp969); Cyc_Absyndump_dump_semi(); goto _LL897; _LL915:
if( ! Cyc_Absyndump_expand_typedefs){ Cyc_Absyndump_dump( _tag_arr("typedef",
sizeof( unsigned char), 8u));(( void(*)( struct Cyc_Absyn_Tqual, void*, void(* f)(
struct Cyc_Absyn_Typedefdecl*), struct Cyc_Absyn_Typedefdecl*)) Cyc_Absyndump_dumptqtd)(({
struct Cyc_Absyn_Tqual _temp1015; _temp1015.q_const= 0; _temp1015.q_volatile= 0;
_temp1015.q_restrict= 0; _temp1015;}),( void*) _temp971->defn, Cyc_Absyndump_dumptypedefname,
_temp971); Cyc_Absyndump_dump_semi();} goto _LL897; _LL917: Cyc_Absyndump_dump(
_tag_arr("namespace", sizeof( unsigned char), 10u)); Cyc_Absyndump_dump_str(
_temp975); Cyc_Absyndump_dump_char(( int)'{'); for( 0; _temp973 != 0; _temp973=((
struct Cyc_List_List*) _check_null( _temp973))->tl){ Cyc_Absyndump_dumpdecl((
struct Cyc_Absyn_Decl*)(( struct Cyc_List_List*) _check_null( _temp973))->hd);}
Cyc_Absyndump_dump_char(( int)'}'); goto _LL897; _LL919: Cyc_Absyndump_dump(
_tag_arr("using", sizeof( unsigned char), 6u)); Cyc_Absyndump_dumpqvar( _temp979);
Cyc_Absyndump_dump_char(( int)'{'); for( 0; _temp977 != 0; _temp977=(( struct
Cyc_List_List*) _check_null( _temp977))->tl){ Cyc_Absyndump_dumpdecl(( struct
Cyc_Absyn_Decl*)(( struct Cyc_List_List*) _check_null( _temp977))->hd);} Cyc_Absyndump_dump_char((
int)'}'); goto _LL897; _LL921: Cyc_Absyndump_dump( _tag_arr("extern \"C\" {",
sizeof( unsigned char), 13u)); for( 0; _temp981 != 0; _temp981=(( struct Cyc_List_List*)
_check_null( _temp981))->tl){ Cyc_Absyndump_dumpdecl(( struct Cyc_Absyn_Decl*)((
struct Cyc_List_List*) _check_null( _temp981))->hd);} Cyc_Absyndump_dump_char((
int)'}'); goto _LL897; _LL897:;} static void Cyc_Absyndump_dump_upperbound(
struct Cyc_Absyn_Exp* e){ unsigned int i= Cyc_Evexp_eval_const_uint_exp( e); if(
i != 1){ Cyc_Absyndump_dump_char(( int)'{'); Cyc_Absyndump_dumpexp( e); Cyc_Absyndump_dump_char((
int)'}');}} void Cyc_Absyndump_dumptms( struct Cyc_List_List* tms, void(* f)(
void*), void* a){ if( tms == 0){ f( a); return;}{ void* _temp1016=( void*)((
struct Cyc_List_List*) _check_null( tms))->hd; struct Cyc_Absyn_Tqual _temp1034;
void* _temp1036; void* _temp1038; struct Cyc_Absyn_Exp* _temp1040; struct Cyc_Absyn_Tqual
_temp1042; void* _temp1044; void* _temp1046; struct Cyc_Absyn_Exp* _temp1048;
struct Cyc_Absyn_Tqual _temp1050; void* _temp1052; void* _temp1054; struct Cyc_Absyn_Tqual
_temp1056; void* _temp1058; struct Cyc_Absyn_Tvar* _temp1060; void* _temp1062;
struct Cyc_Absyn_Exp* _temp1064; struct Cyc_Absyn_Tqual _temp1066; void*
_temp1068; struct Cyc_Absyn_Tvar* _temp1070; void* _temp1072; struct Cyc_Absyn_Exp*
_temp1074; struct Cyc_Absyn_Tqual _temp1076; void* _temp1078; struct Cyc_Absyn_Tvar*
_temp1080; void* _temp1082; _LL1018: if(( unsigned int) _temp1016 > 1u?*(( int*)
_temp1016) == Cyc_Absyn_Pointer_mod: 0){ _LL1039: _temp1038=( void*)(( struct
Cyc_Absyn_Pointer_mod_struct*) _temp1016)->f1; if(( unsigned int) _temp1038 > 1u?*((
int*) _temp1038) == Cyc_Absyn_Nullable_ps: 0){ _LL1041: _temp1040=(( struct Cyc_Absyn_Nullable_ps_struct*)
_temp1038)->f1; goto _LL1037;} else{ goto _LL1020;} _LL1037: _temp1036=( void*)((
struct Cyc_Absyn_Pointer_mod_struct*) _temp1016)->f2; if( _temp1036 ==( void*)
Cyc_Absyn_HeapRgn){ goto _LL1035;} else{ goto _LL1020;} _LL1035: _temp1034=((
struct Cyc_Absyn_Pointer_mod_struct*) _temp1016)->f3; goto _LL1019;} else{ goto
_LL1020;} _LL1020: if(( unsigned int) _temp1016 > 1u?*(( int*) _temp1016) == Cyc_Absyn_Pointer_mod:
0){ _LL1047: _temp1046=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f1; if(( unsigned int) _temp1046 > 1u?*(( int*) _temp1046) == Cyc_Absyn_NonNullable_ps:
0){ _LL1049: _temp1048=(( struct Cyc_Absyn_NonNullable_ps_struct*) _temp1046)->f1;
goto _LL1045;} else{ goto _LL1022;} _LL1045: _temp1044=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f2; if( _temp1044 ==( void*) Cyc_Absyn_HeapRgn){ goto _LL1043;}
else{ goto _LL1022;} _LL1043: _temp1042=(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f3; goto _LL1021;} else{ goto _LL1022;} _LL1022: if(( unsigned int)
_temp1016 > 1u?*(( int*) _temp1016) == Cyc_Absyn_Pointer_mod: 0){ _LL1055:
_temp1054=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*) _temp1016)->f1; if(
_temp1054 ==( void*) Cyc_Absyn_TaggedArray_ps){ goto _LL1053;} else{ goto
_LL1024;} _LL1053: _temp1052=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f2; if( _temp1052 ==( void*) Cyc_Absyn_HeapRgn){ goto _LL1051;}
else{ goto _LL1024;} _LL1051: _temp1050=(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f3; goto _LL1023;} else{ goto _LL1024;} _LL1024: if(( unsigned int)
_temp1016 > 1u?*(( int*) _temp1016) == Cyc_Absyn_Pointer_mod: 0){ _LL1063:
_temp1062=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*) _temp1016)->f1; if((
unsigned int) _temp1062 > 1u?*(( int*) _temp1062) == Cyc_Absyn_Nullable_ps: 0){
_LL1065: _temp1064=(( struct Cyc_Absyn_Nullable_ps_struct*) _temp1062)->f1; goto
_LL1059;} else{ goto _LL1026;} _LL1059: _temp1058=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f2; if(( unsigned int) _temp1058 > 4u?*(( int*) _temp1058) == Cyc_Absyn_VarType:
0){ _LL1061: _temp1060=(( struct Cyc_Absyn_VarType_struct*) _temp1058)->f1; goto
_LL1057;} else{ goto _LL1026;} _LL1057: _temp1056=(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f3; goto _LL1025;} else{ goto _LL1026;} _LL1026: if(( unsigned int)
_temp1016 > 1u?*(( int*) _temp1016) == Cyc_Absyn_Pointer_mod: 0){ _LL1073:
_temp1072=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*) _temp1016)->f1; if((
unsigned int) _temp1072 > 1u?*(( int*) _temp1072) == Cyc_Absyn_NonNullable_ps: 0){
_LL1075: _temp1074=(( struct Cyc_Absyn_NonNullable_ps_struct*) _temp1072)->f1;
goto _LL1069;} else{ goto _LL1028;} _LL1069: _temp1068=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f2; if(( unsigned int) _temp1068 > 4u?*(( int*) _temp1068) == Cyc_Absyn_VarType:
0){ _LL1071: _temp1070=(( struct Cyc_Absyn_VarType_struct*) _temp1068)->f1; goto
_LL1067;} else{ goto _LL1028;} _LL1067: _temp1066=(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f3; goto _LL1027;} else{ goto _LL1028;} _LL1028: if(( unsigned int)
_temp1016 > 1u?*(( int*) _temp1016) == Cyc_Absyn_Pointer_mod: 0){ _LL1083:
_temp1082=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*) _temp1016)->f1; if(
_temp1082 ==( void*) Cyc_Absyn_TaggedArray_ps){ goto _LL1079;} else{ goto
_LL1030;} _LL1079: _temp1078=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f2; if(( unsigned int) _temp1078 > 4u?*(( int*) _temp1078) == Cyc_Absyn_VarType:
0){ _LL1081: _temp1080=(( struct Cyc_Absyn_VarType_struct*) _temp1078)->f1; goto
_LL1077;} else{ goto _LL1030;} _LL1077: _temp1076=(( struct Cyc_Absyn_Pointer_mod_struct*)
_temp1016)->f3; goto _LL1029;} else{ goto _LL1030;} _LL1030: if(( unsigned int)
_temp1016 > 1u?*(( int*) _temp1016) == Cyc_Absyn_Pointer_mod: 0){ goto _LL1031;}
else{ goto _LL1032;} _LL1032: goto _LL1033; _LL1019: Cyc_Absyndump_dump_char((
int)'*'); Cyc_Absyndump_dump_upperbound( _temp1040); Cyc_Absyndump_dumptms(((
struct Cyc_List_List*) _check_null( tms))->tl, f, a); return; _LL1021: Cyc_Absyndump_dump_char((
int)'@'); Cyc_Absyndump_dump_upperbound( _temp1048); Cyc_Absyndump_dumptms(((
struct Cyc_List_List*) _check_null( tms))->tl, f, a); return; _LL1023: Cyc_Absyndump_dump_char((
int)'?'); Cyc_Absyndump_dumptms((( struct Cyc_List_List*) _check_null( tms))->tl,
f, a); return; _LL1025: Cyc_Absyndump_dump_char(( int)'*'); Cyc_Absyndump_dump_upperbound(
_temp1064); Cyc_Absyndump_dump_str( _temp1060->name); Cyc_Absyndump_dumptms(((
struct Cyc_List_List*) _check_null( tms))->tl, f, a); return; _LL1027: Cyc_Absyndump_dump_char((
int)'@'); Cyc_Absyndump_dump_upperbound( _temp1074); Cyc_Absyndump_dump_str(
_temp1070->name); Cyc_Absyndump_dumptms((( struct Cyc_List_List*) _check_null(
tms))->tl, f, a); return; _LL1029: Cyc_Absyndump_dump_char(( int)'?'); Cyc_Absyndump_dump_str(
_temp1080->name); Cyc_Absyndump_dumptms((( struct Cyc_List_List*) _check_null(
tms))->tl, f, a); return; _LL1031:( int) _throw(( void*)({ struct Cyc_Core_Impossible_struct*
_temp1084=( struct Cyc_Core_Impossible_struct*) GC_malloc( sizeof( struct Cyc_Core_Impossible_struct));
_temp1084[ 0]=({ struct Cyc_Core_Impossible_struct _temp1085; _temp1085.tag= Cyc_Core_Impossible;
_temp1085.f1= _tag_arr("dumptms: bad Pointer_mod", sizeof( unsigned char), 25u);
_temp1085;}); _temp1084;})); _LL1033: { int next_is_pointer= 0; if((( struct Cyc_List_List*)
_check_null( tms))->tl != 0){ void* _temp1086=( void*)(( struct Cyc_List_List*)
_check_null((( struct Cyc_List_List*) _check_null( tms))->tl))->hd; _LL1088: if((
unsigned int) _temp1086 > 1u?*(( int*) _temp1086) == Cyc_Absyn_Pointer_mod: 0){
goto _LL1089;} else{ goto _LL1090;} _LL1090: goto _LL1091; _LL1089:
next_is_pointer= 1; goto _LL1087; _LL1091: goto _LL1087; _LL1087:;} if(
next_is_pointer){ Cyc_Absyndump_dump_char(( int)'(');} Cyc_Absyndump_dumptms(((
struct Cyc_List_List*) _check_null( tms))->tl, f, a); if( next_is_pointer){ Cyc_Absyndump_dump_char((
int)')');}{ void* _temp1092=( void*)(( struct Cyc_List_List*) _check_null( tms))->hd;
struct Cyc_Absyn_Exp* _temp1108; void* _temp1110; struct Cyc_List_List*
_temp1112; struct Cyc_Core_Opt* _temp1114; struct Cyc_Absyn_VarargInfo*
_temp1116; int _temp1118; struct Cyc_List_List* _temp1120; void* _temp1122;
struct Cyc_Position_Segment* _temp1124; struct Cyc_List_List* _temp1126; int
_temp1128; struct Cyc_Position_Segment* _temp1130; struct Cyc_List_List*
_temp1132; struct Cyc_List_List* _temp1134; void* _temp1136; void* _temp1138;
_LL1094: if( _temp1092 ==( void*) Cyc_Absyn_Carray_mod){ goto _LL1095;} else{
goto _LL1096;} _LL1096: if(( unsigned int) _temp1092 > 1u?*(( int*) _temp1092)
== Cyc_Absyn_ConstArray_mod: 0){ _LL1109: _temp1108=(( struct Cyc_Absyn_ConstArray_mod_struct*)
_temp1092)->f1; goto _LL1097;} else{ goto _LL1098;} _LL1098: if(( unsigned int)
_temp1092 > 1u?*(( int*) _temp1092) == Cyc_Absyn_Function_mod: 0){ _LL1111:
_temp1110=( void*)(( struct Cyc_Absyn_Function_mod_struct*) _temp1092)->f1; if(*((
int*) _temp1110) == Cyc_Absyn_WithTypes){ _LL1121: _temp1120=(( struct Cyc_Absyn_WithTypes_struct*)
_temp1110)->f1; goto _LL1119; _LL1119: _temp1118=(( struct Cyc_Absyn_WithTypes_struct*)
_temp1110)->f2; goto _LL1117; _LL1117: _temp1116=(( struct Cyc_Absyn_WithTypes_struct*)
_temp1110)->f3; goto _LL1115; _LL1115: _temp1114=(( struct Cyc_Absyn_WithTypes_struct*)
_temp1110)->f4; goto _LL1113; _LL1113: _temp1112=(( struct Cyc_Absyn_WithTypes_struct*)
_temp1110)->f5; goto _LL1099;} else{ goto _LL1100;}} else{ goto _LL1100;}
_LL1100: if(( unsigned int) _temp1092 > 1u?*(( int*) _temp1092) == Cyc_Absyn_Function_mod:
0){ _LL1123: _temp1122=( void*)(( struct Cyc_Absyn_Function_mod_struct*)
_temp1092)->f1; if(*(( int*) _temp1122) == Cyc_Absyn_NoTypes){ _LL1127:
_temp1126=(( struct Cyc_Absyn_NoTypes_struct*) _temp1122)->f1; goto _LL1125;
_LL1125: _temp1124=(( struct Cyc_Absyn_NoTypes_struct*) _temp1122)->f2; goto
_LL1101;} else{ goto _LL1102;}} else{ goto _LL1102;} _LL1102: if(( unsigned int)
_temp1092 > 1u?*(( int*) _temp1092) == Cyc_Absyn_TypeParams_mod: 0){ _LL1133:
_temp1132=(( struct Cyc_Absyn_TypeParams_mod_struct*) _temp1092)->f1; goto
_LL1131; _LL1131: _temp1130=(( struct Cyc_Absyn_TypeParams_mod_struct*)
_temp1092)->f2; goto _LL1129; _LL1129: _temp1128=(( struct Cyc_Absyn_TypeParams_mod_struct*)
_temp1092)->f3; goto _LL1103;} else{ goto _LL1104;} _LL1104: if(( unsigned int)
_temp1092 > 1u?*(( int*) _temp1092) == Cyc_Absyn_Attributes_mod: 0){ _LL1135:
_temp1134=(( struct Cyc_Absyn_Attributes_mod_struct*) _temp1092)->f2; goto
_LL1105;} else{ goto _LL1106;} _LL1106: if(( unsigned int) _temp1092 > 1u?*((
int*) _temp1092) == Cyc_Absyn_Pointer_mod: 0){ _LL1139: _temp1138=( void*)((
struct Cyc_Absyn_Pointer_mod_struct*) _temp1092)->f1; goto _LL1137; _LL1137:
_temp1136=( void*)(( struct Cyc_Absyn_Pointer_mod_struct*) _temp1092)->f2; goto
_LL1107;} else{ goto _LL1093;} _LL1095: Cyc_Absyndump_dump( _tag_arr("[]",
sizeof( unsigned char), 3u)); goto _LL1093; _LL1097: Cyc_Absyndump_dump_char((
int)'['); Cyc_Absyndump_dumpexp( _temp1108); Cyc_Absyndump_dump_char(( int)']');
goto _LL1093; _LL1099: Cyc_Absyndump_dumpfunargs( _temp1120, _temp1118,
_temp1116, _temp1114, _temp1112); goto _LL1093; _LL1101:(( void(*)( void(* f)(
struct _tagged_arr*), struct Cyc_List_List* l, struct _tagged_arr start, struct
_tagged_arr end, struct _tagged_arr sep)) Cyc_Absyndump_group)( Cyc_Absyndump_dump_str,
_temp1126, _tag_arr("(", sizeof( unsigned char), 2u), _tag_arr(")", sizeof(
unsigned char), 2u), _tag_arr(",", sizeof( unsigned char), 2u)); goto _LL1093;
_LL1103: if( _temp1128){ Cyc_Absyndump_dumpkindedtvars( _temp1132);} else{ Cyc_Absyndump_dumptvars(
_temp1132);} goto _LL1093; _LL1105: Cyc_Absyndump_dumpatts( _temp1134); goto
_LL1093; _LL1107:( int) _throw(( void*)({ struct Cyc_Core_Impossible_struct*
_temp1140=( struct Cyc_Core_Impossible_struct*) GC_malloc( sizeof( struct Cyc_Core_Impossible_struct));
_temp1140[ 0]=({ struct Cyc_Core_Impossible_struct _temp1141; _temp1141.tag= Cyc_Core_Impossible;
_temp1141.f1= _tag_arr("dumptms", sizeof( unsigned char), 8u); _temp1141;});
_temp1140;})); _LL1093:;} return;} _LL1017:;}} void Cyc_Absyndump_dumptqtd(
struct Cyc_Absyn_Tqual tq, void* t, void(* f)( void*), void* a){ struct Cyc_List_List*
_temp1144; void* _temp1146; struct Cyc_Absyn_Tqual _temp1148; struct _tuple4
_temp1142= Cyc_Absynpp_to_tms( tq, t); _LL1149: _temp1148= _temp1142.f1; goto
_LL1147; _LL1147: _temp1146= _temp1142.f2; goto _LL1145; _LL1145: _temp1144=
_temp1142.f3; goto _LL1143; _LL1143: Cyc_Absyndump_dumptq( _temp1148); Cyc_Absyndump_dumpntyp(
_temp1146); Cyc_Absyndump_dumptms( Cyc_List_imp_rev( _temp1144), f, a);} void
Cyc_Absyndump_dumpdecllist2file( struct Cyc_List_List* tdl, struct Cyc_Stdio___sFILE*
f){ Cyc_Absyndump_pos= 0;* Cyc_Absyndump_dump_file= f; for( 0; tdl != 0; tdl=((
struct Cyc_List_List*) _check_null( tdl))->tl){ Cyc_Absyndump_dumpdecl(( struct
Cyc_Absyn_Decl*)(( struct Cyc_List_List*) _check_null( tdl))->hd);}({ int(*
_temp1150)( struct Cyc_Stdio___sFILE*, struct _tagged_arr fmt, struct
_tagged_arr)= Cyc_Stdio_fprintf; struct Cyc_Stdio___sFILE* _temp1153= f; struct
_tagged_arr _temp1154= _tag_arr("\n", sizeof( unsigned char), 2u); void*
_temp1151[ 0u]={}; struct _tagged_arr _temp1152={( void*) _temp1151,( void*)
_temp1151,( void*)( _temp1151 + 0u)}; _temp1150( _temp1153, _temp1154, _temp1152);});}
