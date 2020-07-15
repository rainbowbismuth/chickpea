#include "chickpea.h"

void halt()
{
	__asm__ volatile("swi 0x2");
}