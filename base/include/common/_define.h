#ifndef __DEFINE_H__
#define __DEFINE_H__


#define SUCCESS       0
#define FAILURE       (-1)

/****************************************************
  Date types(Compiler specific) 数据类型（和编译器相关）   
*****************************************************/
#define _NULL              0L
typedef void               _VOID;
typedef char               _CHAR;    /* 8 bit quantity  */
typedef unsigned char      _U8;      /* Unsigned 8 bit quantity  */
typedef signed char        _S8;      /* Signed 8 bit quantity    */
typedef unsigned short     _U16;     /* Unsigned 16 bit quantity */
typedef signed short       _S16;     /* Signed 16 bit quantity   */
typedef unsigned int       _U32;     /* Unsigned 32 bit quantity */
typedef signed int         _S32;     /* Signed 32 bit quantity   */
# if __WORDSIZE == 64
typedef unsigned long      _U64;     /* Unsigned 32 bit quantity */
typedef long               _S64;     /* Signed 32 bit quantity   */
#else
typedef unsigned long long _U64;     /* Unsigned 32 bit quantity */
typedef long long          _S64;     /* Signed 32 bit quantity   */
#endif
typedef float              _FP32;    /* Single precision         */
                                     /* floating point           */
typedef double             _FP64;    /* Double precision         */
                                     /* floating point           */
#endif