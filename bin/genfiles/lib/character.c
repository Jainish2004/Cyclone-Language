#include "cyc_include.h"

 typedef int Cyc_ptrdiff_t; typedef unsigned int Cyc_size_t; typedef
unsigned short Cyc_wchar_t; typedef unsigned int Cyc_wint_t; typedef char Cyc_u_char;
typedef unsigned short Cyc_u_short; typedef unsigned int Cyc_u_int; typedef
unsigned int Cyc_u_long; typedef unsigned short Cyc_ushort; typedef unsigned int
Cyc_uint; typedef unsigned int Cyc_clock_t; typedef int Cyc_time_t; struct Cyc_timespec{
int tv_sec; int tv_nsec; } ; struct Cyc_itimerspec{ struct Cyc_timespec
it_interval; struct Cyc_timespec it_value; } ; typedef int Cyc_daddr_t; typedef
char* Cyc_caddr_t; typedef unsigned int Cyc_ino_t; typedef unsigned int Cyc_vm_offset_t;
typedef unsigned int Cyc_vm_size_t; typedef char Cyc_int8_t; typedef char Cyc_u_int8_t;
typedef short Cyc_int16_t; typedef unsigned short Cyc_u_int16_t; typedef int Cyc_int32_t;
typedef unsigned int Cyc_u_int32_t; typedef long long Cyc_int64_t; typedef
unsigned long long Cyc_u_int64_t; typedef int Cyc_register_t; typedef short Cyc_dev_t;
typedef int Cyc_off_t; typedef unsigned short Cyc_uid_t; typedef unsigned short
Cyc_gid_t; typedef int Cyc_pid_t; typedef int Cyc_key_t; typedef int Cyc_ssize_t;
typedef char* Cyc_addr_t; typedef int Cyc_mode_t; typedef unsigned short Cyc_nlink_t;
typedef int Cyc_fd_mask; struct Cyc__types_fd_set{ int fds_bits[ 8u]; } ;
typedef struct Cyc__types_fd_set Cyc__types_fd_set; typedef char* Cyc_Cstring;
typedef struct _tagged_string Cyc_string; typedef struct _tagged_string Cyc_string_t;
typedef struct _tagged_string* Cyc_stringptr; typedef int Cyc_bool; extern void*
exit( int); extern void* abort(); struct Cyc_Core_Opt{ void* v; } ; typedef
struct Cyc_Core_Opt* Cyc_Core_opt_t; extern char Cyc_Core_InvalidArg_tag[ 11u];
struct Cyc_Core_InvalidArg_struct{ char* tag; struct _tagged_string f1; } ;
extern char Cyc_Core_Failure_tag[ 8u]; struct Cyc_Core_Failure_struct{ char* tag;
struct _tagged_string f1; } ; extern char Cyc_Core_Impossible_tag[ 11u]; struct
Cyc_Core_Impossible_struct{ char* tag; struct _tagged_string f1; } ; extern char
Cyc_Core_Not_found_tag[ 10u]; struct Cyc_Core_Not_found_struct{ char* tag; } ;
extern char Cyc_Core_Unreachable_tag[ 12u]; struct Cyc_Core_Unreachable_struct{
char* tag; struct _tagged_string f1; } ; extern char* string_to_Cstring( struct
_tagged_string); extern char* underlying_Cstring( struct _tagged_string); extern
struct _tagged_string Cstring_to_string( char*); extern int system( char*); int
Cyc_Character_isupper( char c){ return( int) c >=( int)'A'?( int) c <=( int)'Z':
0;} int Cyc_Character_islower( char c){ return( int) c >=( int)'a'?( int) c <=(
int)'z': 0;} int Cyc_Character_isalpha( char c){ return Cyc_Character_islower( c)?
1: Cyc_Character_isupper( c);} int Cyc_Character_isdigit( char c){ return( int)
c >=( int)'0'?( int) c <=( int)'9': 0;} int Cyc_Character_isprint( char c){
return( int) c ==( int)' '? 1:(( int) c >=( int)'!'?( int) c <=( int)'~': 0);}
int Cyc_Character_isgraph( char c){ return( int) c >=( int)'!'?( int) c <=( int)'~':
0;} char Cyc_Character_toupper( char c){ if( Cyc_Character_islower( c)){( int)(
c +=( int)'A' -( int)'a');} return c;} int Cyc_Character_isxdigit( char c){ if(
Cyc_Character_isdigit( c)){ return 1;} else{ char d= Cyc_Character_toupper( c);
return( int) d >=( int)'A'?( int) d <=( int)'F': 0;}} int Cyc_Character_isspace(
char c){ return((((( int) c ==( int)' '? 1:( int) c ==( int)'\t')? 1:( int) c ==(
int)'\n')? 1:( int) c ==( int)'\r')? 1:( int) c ==( int)'\f')? 1:( int) c ==(
int)'\v';} char Cyc_Character_tolower( char c){ if( Cyc_Character_isupper( c)){(
int)( c +=( int)'a' -( int)'A');} return c;}