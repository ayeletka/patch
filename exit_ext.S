#include "syscall.h"
#include "traps.h"

.text

    .global exit_start
    exit_start:

    pushl   %eax
    movl 	$2,%eax
    int 	$64

    .global exit_end
    exit_end:
    ret