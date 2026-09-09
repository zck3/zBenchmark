;============================================================================
; zBenchmark
; 64-bit x86 (x86_64) routines for zBenchmark.
;
; Bignum and memory bandwidth routines: 
; Copyright (C) 2024, 2026 by Zack T Smith.
;
; Primes checker:
; Copyright (C) 2012, 2017, 2021 by Zack T Smith.
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
;
; The author may be reached at 3 at zs3 dot me.
;=============================================================================

; Unix ABI says integer param are put in these registers in this order:
;       rdi, rsi, rdx, rcx, r8, r9

bits    64
cpu     x64
default rel

global bignum_add
global _bignum_add

global seq_memory_reader
global _seq_memory_reader

global seq_memory_writer
global _seq_memory_writer

global determine_primes64
global _determine_primes64

        section .text

;-----------------------------------------------------------------------------
; Name: 	bignum_add
; Purpose:	Adds two huge numbers efficiently.
; Params:
;	rdi = address of first
;	rsi = address of second
;	rdx = address of sum
; 	rcx = number of quadwords
;-----------------------------------------------------------------------------
align 16
bignum_add:
_bignum_add:
	push	r8
	push	r9
	push	r10
	push	r11
	push	r12
	push	r13
	push	r14
	push	r15

	sar	rcx, 2

	clc

.L0:
	; 256 bits per iteration
	mov	r8, [rdi]
	mov	r9, [rdi+8]
	mov	r10, [rdi+16]
	mov	r11, [rdi+24]
	lea	rdi, [rdi + 32]
	
	mov	r12, [rsi]
	mov	r13, [rsi+8]
	mov	r14, [rsi+16]
	mov	r15, [rsi+24]
	lea	rsi, [rsi + 32]
	
	adc	r8, r12
	adc	r9, r13
	adc	r10, r14
	adc	r11, r15

	mov	[rdx], r8
	mov	[rdx+8], r9
	mov	[rdx+16], r10
	mov	[rdx+24], r11
	lea	rdx, [rdx + 32]

	; Decrement counter & leave carry intact.
	loop	.L0

	; Return the carry bit in RAX.
	mov	rax, 0
	adc	rax, 0

	pop	r15
	pop	r14
	pop	r13
	pop	r12
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	ret

;-----------------------------------------------------------------------------
; Name: 	seq_memory_reader
; Purpose:	Rapidly reads from memory.
; Params:
;	rdi = pointer
;	rsi = length (multiple of 128)
;	rdx = loops
;-----------------------------------------------------------------------------
align 16
_seq_memory_reader:
seq_memory_reader:

	mfence 

	push	r8
	push	r9
	push	r10
	push	r11

	shr	rsi, 7

	mov	r10, rsi
	mov	r11, rdi
.L0:
	mov	rsi, r10
	mov	rdi, r11
.L1:
	mov	r8, [rdi]
	or	r9, [rdi+8]
	mov	r9, [rdi+16]
	and	r8, [rdi+24]

	and	r8, [rdi+32]
	mov	r9, [rdi+40]
	xor	r9, [rdi+48]
	mov	r8, [rdi+56]

	mov	r8, [rdi+64]
	sub	r9, [rdi+72]
	mov	r9, [rdi+80]
	sub	r8, [rdi+88]

	mov	r8, [rdi+96]
	or	r9, [rdi+104]
	mov	r9, [rdi+112]
	or	r8, [rdi+120]

	add	rdi, 128
	dec	rsi
	jnz	.L1

	dec	rdx
	jnz	.L0

	pop	r11
	pop	r10
	pop	r9
	pop	r8
	
	ret

;-----------------------------------------------------------------------------
; Name: 	seq_memory_writer
; Purpose:	Rapidly writes to memory.
; Params:
;	rdi = pointer
;	rsi = length (multiple of 128)
;	rdx = loops
;-----------------------------------------------------------------------------
align 16
_seq_memory_writer:
seq_memory_writer:

	sfence

	push	r10
	push	r11

	shr	rsi, 7

	mov	r10, rsi
	mov	r11, rdi
.L0:
	mov	rsi, r10
	mov	rdi, r11
.L2:
	mov	[rdi], r8
	mov	[rdi+8], r9
	mov	[rdi+16], r10
	mov	[rdi+24], r11

	mov	[rdi+32], r12
	mov	[rdi+40], r13
	mov	[rdi+48], r14
	mov	[rdi+56], r15

	mov	[rdi+64], rax
	mov	[rdi+72], rbx
	mov	[rdi+80], rcx
	mov	[rdi+88], rdx

	mov	[rdi+96], rsi
	mov	[rdi+104], rdi
	mov	[rdi+112], rbp
	mov	[rdi+120], rbp

	add	rdi, 128

	dec	rsi
	jnz	.L2

	dec	rdx
	jnz	.L0

	pop	r11
	pop	r10
	ret

;------------------------------------------------------------------------------
; Name:		is_prime
; Purpose:	Progressively divides value by known lesser primes
;		to determine whether value is prime.
; Params:	rbp = n_primes found so far
; 		rdi = max_primes
; 		rsi = ptr to memory area having 64-bit primes
; 		r15 = value (dividend)
;------------------------------------------------------------------------------
align 16
is_prime:
	push	rbx
	push	rdx
	push	r8	
	push	r9

	;------------------------------------------------
	; Get 1 + the square root of the candidate prime.
	; This is the upper limit of primes that 
	; we need to divide into the candidate prime.
	push	r15
	fild	qword [rsp]
	fsqrt
	fistp	qword [rsp]
	pop	r9
	inc	r9

	mov	rbx, 14	; Skip first 14 primes

.LprimeCheckerLoop:
	cmp	rbx, rbp	; Done all primes? If done our value is prime.
	jae	.LreturnTrue

	mov	r8, [8*rbx + rsi]	; Get next preexisting prime#.
	cmp	r8, r9		; Only test primes < 1+sqrt(n).
	jae	.LreturnTrue
	inc	rbx

	mov	rdx, r15
	shr	rdx, 32
	jz	.L32bitDiv

	; Number is not 32-bit.
	; Do a 128{rdx:rax}-by-64{r8} division, putting zeroes in high-order qword RDX.
	xor	rdx, rdx
	mov	rax, r15
	div	r8 	; Remainder will be in RDX.

	; If remainder == 0, number was factorable by a smaller number that is prime .'. it is not prime.
	or	rdx, rdx	
	jz	.LreturnFalse
	jmp	.LprimeCheckerLoop

.L32bitDiv:
	; Perform a 64-by-32 bit division.
	mov	rax, r15
	xor	rdx, rdx
	div	r8d
	test	edx, edx	
	jz	.LreturnFalse
	jmp	.LprimeCheckerLoop

.LreturnTrue:
	mov	al, 1	; Return nonzero.
	jmp 	.Lreturn

.LreturnFalse:
	xor	rax, rax
.Lreturn:
	pop	r9
	pop	r8
	pop	rdx
	pop	rbx
	ret

.Lerror:
	jmp	.Lreturn

;------------------------------------------------------------------------------
; Name:		determine_primes64
; Purpose:	This is the main primes test.
; Params:
;	rdi = n_primes
;	rsi = primes pointer
; Returns:
;	rax = total primes found
;------------------------------------------------------------------------------
align 16
determine_primes64:
_determine_primes64:
	push	rbx	; counters 3 & 5
	push	rcx	; counters 7 & 11
	push	rdx	; counters 13 & 17
	push	rsi	; pointer to primes array
	push	rdi	; max_primes
	push	rbp	; n_primes found so far
	push	r8	; counter 19
	push 	r9	; counter 23
	push 	r10	; counter 29
	push 	r11	; counter 31
	push 	r12	; counter 37
	push 	r13	; counter 41
	push 	r14	; counter 43
	push 	r15	; candidate number

	;--------------------------------------------------
	; Initialize the prime number and primes counters.
	;
	mov	r15, 2

	mov	bl, 2	; 3 counter
	mov	bh, 2	; 5 counter
	mov	cl, 2	; 7 counter
	mov 	ch, 2	; 11 counter
	mov 	dl, 2	; 13 counter
	mov	dh, 2	; 17 counter
	mov	r8, 2	; 19 counter
	mov	r9, 2	; 23 counter
	mov	r10, 2	; 29 counter
	mov	r11, 2	; 31 counter
	mov	r12, 2	; 37 counter
	mov	r13, 2	; 41 counter
	mov	r14, 2	; 43 counter

	;----------------------
	; Set up preset primes.
	;
	mov	qword [rsi], 2
	mov	qword [8 + rsi], 3
	mov	qword [16 + rsi], 5
	mov	qword [24 + rsi], 7
	mov	qword [32 + rsi], 11
	mov	qword [40 + rsi], 13
	mov	qword [48 + rsi], 17
	mov	qword [56 + rsi], 19
	mov	qword [64 + rsi], 23
	mov	qword [72 + rsi], 29
	mov	qword [80 + rsi], 31
	mov	qword [88 + rsi], 37
	mov	qword [96 + rsi], 41
	mov	qword [104 + rsi], 43
	mov	rbp, 14	; # primes in the array so far.

.LmainLoop:
	;----------------------------------------
	; Go to the next number 
	; Use counters to approximate something like the Sieve of Eratosthenes.
	; 0 1 2 3 4 5 6 7 8 9 10 11	value
	; 0     3     6     9		counter3
	; 0         5         10	counter5
	; 0             7     		counter7
	; 0                      11	counter11
	; etc.

	inc	r15	; value++

	inc	bl	; counter3++
	inc	bh	; counter5++
	inc 	cl	; counter7++
	inc	ch	; counter11++
	inc	dl	; counter13++
	inc	dh	; counter17++
	inc	r8	; counter19++
	inc	r9	; counter23++
	inc	r10	; counter29++
	inc	r11	; counter31++
	inc	r12	; counter37++
	inc	r13	; counter41++
	inc	r14	; counter43++

	; Tricky bit of code
	; AL = 0
	; AH = the skip flag
	xor	rax, rax	; RULE: ah != 0 ? therefore skip

	cmp	bl, 3
	jb	.Lbh
	mov	bl, al
	mov	ah, 1

.Lbh:
	cmp	bh, 5
	jb	.Lcl
	mov	bh, al
	mov	ah, 1

.Lcl:
	cmp	cl, 7
	jb	.Lch
	mov	cl, al
	mov	ah, 1

.Lch:
	cmp	ch, 11
	jb	.Ldl
	mov	ch, al
	mov	ah, 1

.Ldl:
	cmp	dl, 13
	jb	.Ldh
	mov	dl, al
	mov	ah, 1

.Ldh:
	cmp	dh, 17
	jb	.Lr8
	mov	dh, al
	mov	ah, 1

.Lr8:
	cmp	r8b, 19
	jb	.Lr9
	xor	r8, r8
	mov	ah, 1

.Lr9:
	cmp	r9b, 23
	jb	.Lr10
	xor	r9, r9
	mov	ah, 1

.Lr10:
	cmp	r10b, 29
	jb	.Lr11
	xor	r10, r10
	mov	ah, 1

.Lr11:
	cmp	r11b, 31
	jb	.Lr12
	xor	r11, r11
	mov	ah, 1

.Lr12:
	cmp	r12b, 37
	jb	.Lr13
	xor	r12, r12
	mov	ah, 1

.Lr13:
	cmp	r13b, 41
	jb	.Lr14
	xor	r13, r13
	mov	ah, 1

.Lr14:
	cmp	r14b, 43
	jb	.LdoneComparing
	xor	r14, r14
	mov	ah, 1

.LdoneComparing:
	;----------------------------------------
	; If ah != 0, value is multiple of prime.
	;
	test	ah, ah
	jne	.LmainLoop

	;--------------------
	; Skip if even.
	;
	test	r15, 1	; Ignore even values.
	jz	.LmainLoop

	;----------------------
	; Skip counters' primes.
	;
	cmp	r15, 44
	jb	.LmainLoop

	;------------------------------
	; Test whether r15 is prime.
	;
	call	is_prime
	test	rax, rax
	jz	.LmainLoop	; Not prime, try another.

	;--------------------
	; Prime, store it.
	;
	mov	qword [rbp*8 + rsi], r15
	inc	rbp
	cmp	rbp, rdi	; Array full? 
	jae	.Ldone

	jmp .LmainLoop

.Ldone:
	;--------------------
	; Return # of primes.
	;
	mov	rax, rbp

	pop	r15
	pop	r14
	pop	r13
	pop	r12
	pop	r11
	pop	r10
	pop	r9
	pop	r8
	pop	rbp
	pop	rdi
	pop	rsi
	pop	rdx
	pop	rcx
	pop	rbx
	ret

%ifidn __OUTPUT_FORMAT__, elf64
        ; Not compatible with macOS:
        section .note.GNU-stack
%endif

