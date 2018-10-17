#include <stdlib.h>
#include <stdio.h>

int main(char**argv)
{
  for(int i = 0; i < 1000000000; i++)
    {
      /*
	// pop counting takes 2ns
	// take approx 2s
	mov	eax, DWORD PTR -8[rbp]
	mov	eax, eax
	mov	rdi, rax
	call	__popcountdi2@PLT
	mov	DWORD PTR -4[rbp], eax
       */
      //int z = __builtin_popcount(i);
    }
}
