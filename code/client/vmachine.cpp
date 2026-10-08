// vmachine.cpp -- wrapper to fake virtual machine for client

#include "vmachine.h"
#pragma warning (disable : 4514)
/*
==============================================================

VIRTUAL MACHINE

==============================================================
*/
int	VM_Call( int callnum, ... )
{
#ifdef _WIN32
	return cgvm.entryPoint( (&callnum)[0], (&callnum)[1], (&callnum)[2], (&callnum)[3],
		(&callnum)[4], (&callnum)[5], (&callnum)[6], (&callnum)[7],
		(&callnum)[8],  (&callnum)[9] );
#else
	// the portable way to the same nine int arguments
	int		args[9];
	va_list	ap;
	int		i;

	va_start( ap, callnum );
	for ( i = 0 ; i < 9 ; i++ ) {
		args[i] = va_arg( ap, int );
	}
	va_end( ap );

	return cgvm.entryPoint( callnum, args[0], args[1], args[2], args[3],
		args[4], args[5], args[6], args[7], args[8] );
#endif
}

/*
============
VM_DllSyscall

we pass this to the cgame dll to call back into the client
============
*/
extern int CL_CgameSystemCalls( int *args );
extern int CL_UISystemCalls( int *args );

int VM_DllSyscall( int arg, ... ) {
//	return cgvm->systemCall( &arg );
#ifdef _WIN32
	return CL_CgameSystemCalls( &arg );
#else
	// &arg need not point into the caller's argument list: gcc for the 68k
	// passed the address of a local copy, so every cgame call read garbage
	// for its arguments (the game hung entering the first map). The calls
	// use args[0..10]; floats go as ints (PASSFLOAT).
	int		args[16];
	va_list	ap;
	int		i;

	args[0] = arg;
	va_start( ap, arg );
	for ( i = 1 ; i < 16 ; i++ ) {
		args[i] = va_arg( ap, int );
	}
	va_end( ap );

	return CL_CgameSystemCalls( args );
#endif
}
