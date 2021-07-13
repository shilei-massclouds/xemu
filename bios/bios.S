/*
 * BIOS
 */

#include "bios.h"

.global _start
_start:

    csrr    a0, mhartid
    li      a1, DTB_LOAD_ADDR

    li      t5, PAYLOAD_LOAD_ADDR
    li      t6, PAYLOAD_LINK_ADDR
    call    _relocate

    li      t5, SBI_LOAD_ADDR
    li      t6, SBI_LINK_ADDR
    call    _relocate

    jr      t6

_relocate:
    mv      t2, t5              /* start of load addr   */
    ld      t1, -8(t2)          /* size of sbi          */
    addi    t1, t1, 7
    andi    t1, t1, ~7
    add     t3, t2, t1          /* end of load addr     */

    mv      t0, t6              /* start of link addr   */
    add     t1, t0, t1          /* end of link end      */

_loop:
	add	    t3, t3, -8
	add	    t1, t1, -8
	ld	    t2, 0(t3)
	sd	    t2, 0(t1)
	blt	    t0, t1, _loop
    ret
