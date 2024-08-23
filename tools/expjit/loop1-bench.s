	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 14, 0	sdk_version 14, 2

	.globl	__1_bench_jit
	.p2align	2

// typedef struct SHLocals
//    0: struct SHLocals *prev;
//    8: unsigned count;
//   12: gap
//   16: SHUnit *unit;
//   24: uint32_t src_location_idx;
//   28: gap
//   32: locals

__1_bench_jit:
    sub sp, sp, #10*8
    //  0-3: SHLocals
    //  4: x22
    //  5: x21
    //  6: x20
    //  7: x19
    //  8: x29 <- new x29 points here
    //  9: x30
    stp	x22, x21, [sp, #4*8]
    stp	x20, x19, [sp, #6*8]
    stp	x29, x30, [sp, #8*8]
    add	x29, sp, #8*8

	mov	x19, x0 ; x19 is runtime

    // _sh_check_native_stack_overflow(shr);
	bl	__sh_check_native_stack_overflow

    // SHLegacyValue *frame = _sh_enter(shr, &locals.head, 6);
	mov	x1, sp
	mov	x0, x19
	mov	w2, #6
	bl	__sh_enter
	mov x20, x0     ; x20 is frame

    // locals.head.coount = 0;
    mov w1, #0
    str w1, [sp, #1*8]

    // frame[0] = _sh_ljs_param(frame, 1);
	; x0 is still frame
	mov w1, #1      ; param 1
	bl  __sh_ljs_param
	str x0, [x20]   ; frame[0] = param 1

    // frame[3] = _sh_ljs_dec_rjs(shr, frame + 0);
	mov x0, x19     ; runtime
	mov x1, x20     ; frame + 0
	bl  __sh_ljs_dec_rjs
	str x0, [x20, 3*8]  ; frame[3] =

    // frame[2] = _sh_ljs_double(1);
    fmov d0, 1
    str d0, [x20, 2*8]

    // frame[1] = frame[0];
    ldr x0, [x20]
    str x0, [x20, 1*8]
    // frame[0] = frame[1];
    ldr x0, [x20, 1*8]
    str x0, [x20]

    // if (!_sh_ljs_greater_rjs(shr, frame + 3, frame + 2)) goto L1;
    mov x0, x19
    add x1, x20, 3*8
    add x2, x20, 2*8
    bl __sh_ljs_greater_rjs
    cbz w0, L1

L2:
    // frame[1] = _sh_ljs_mul_rjs(shr, frame + 1, frame + 3);
    mov x0, x19
    add x1, x20, 1*8
    add x2, x20, 3*8
    bl __sh_ljs_mul_rjs
    str x0, [x20, 1*8]

    // frame[3] = _sh_ljs_dec_rjs(shr, frame + 3);
    mov x0, x19
    add x1, x20, 3*8
    bl __sh_ljs_dec_rjs
    str x0, [x20, 3*8]

    // frame[0] = frame[1];
    ldr x0, [x20, 1*8]
    str x0, [x20]

    // if (_sh_ljs_greater_rjs(shr, frame + 3, frame + 2)) goto L2;
    mov x0, x19
    add x1, x20, 3*8
    add x2, x20, 2*8
    bl __sh_ljs_greater_rjs
    cbnz w0, L2

L1:
    // SHLegacyValue tmp = frame[0];
    ldr x21, [x20]

    // _sh_leave(shr, &locals.head, frame);
	mov	x0, x19
	mov	x1, sp
    mov	x2, x20
	bl	__sh_leave

	mov	x0, x21

    ldp	x29, x30, [sp, #8*8]
    ldp	x20, x19, [sp, #6*8]
    ldp	x22, x21, [sp, #4*8]
	add	sp, sp, #10*8
	ret
