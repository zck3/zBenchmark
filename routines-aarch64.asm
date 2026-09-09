#============================================================================
# zBenchmark
# 64-bit ARM (aarch64) routines for zBenchmark.
#
# Copyright (C) 2021, 2026 by Zack T Smith.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# The author may be reached at 3 at zs3 dot me.
#=============================================================================

# 64 bit ISA, application processor
.arch armv8-a

.global bignum_add
.global _macos_bignum_add

.global seq_memory_reader
.global _macos_seq_memory_reader

.global seq_memory_writer
.global _macos_seq_memory_writer

.text

#-----------------------------------------------------------------------------
# Name: 	bignum_add
# Purpose:	Adds two huge numbers efficiently.
# Params:
#	x0 = address of first
#	x1 = address of second
#	x2 = address of sum
# 	x3 = number of quadwords
#-----------------------------------------------------------------------------
.align 16
bignum_add:
_macos_bignum_add:
	stp	x5, x6, [sp, -16]!
	stp	x7, x8, [sp, -16]!
	stp	x9, x10, [sp, -16]!
	stp	x11, x12, [sp, -16]!

	lsr	x3, x3, 2

	# Clear carry bit
	adds	xzr, xzr, xzr

.L0:
	# Add 256 bits per loop iteration
	ldp	x5, x6, [x0], 16
	ldp	x7, x8, [x0], 16

	ldp	x9, x10, [x1], 16
	ldp	x11, x12, [x1], 16

	adcs	x5, x5, x9
	adcs	x6, x6, x10
	adcs	x7, x7, x11
	adcs	x8, x8, x12

	stp	x5, x6, [x2], 16
	stp	x7, x8, [x2], 16

	# Decrement counter & leave carry intact.
	sub	x3, x3, 1

	cbnz	x3, .L0

	# Return 1 if carry is set, else 0.
	adc	x0, xzr, xzr
	
	ldp	x12, x11, [sp], 16
	ldp	x10, x9, [sp], 16
	ldp	x8, x7, [sp], 16
	ldp	x6, x5, [sp], 16
	ret

#-----------------------------------------------------------------------------
# Name: 	seq_memory_reader
# Purpose:	Rapidly reads from memory.
# Params:
#	x0 = pointer
#	x1 = length (multiple of 128)
#	x2 = loops
#-----------------------------------------------------------------------------
.align 16
seq_memory_reader:
_macos_seq_memory_reader:
	stp	x5, x6, [sp, -16]!
	stp	x7, x8, [sp, -16]!

	mov	x7, x0
	mov	x8, x1
.L0:
	mov	x0, x7
	mov	x1, x8
.L1:
	ldp	x5, x6, [x0], 16
	ldp	x5, x6, [x0], 16
	ldp	x5, x6, [x0], 16
	ldp	x5, x6, [x0], 16
	ldp	x5, x6, [x0], 16
	ldp	x5, x6, [x0], 16
	ldp	x5, x6, [x0], 16
	ldp	x5, x6, [x0], 16
	subs	x1, x1, 128
	bne	.L1

	subs	x2, x2, 1
	bne	.L0

	ldp	x8, x7, [sp], 16
	ldp	x6, x5, [sp], 16
	ret

#-----------------------------------------------------------------------------
# Name: 	seq_memory_writer
# Purpose:	Rapidly writes to memory.
# Params:
#	x0 = pointer
#	x1 = length (multiple of 128)
#	x2 = loops
#-----------------------------------------------------------------------------
.align 16
seq_memory_writer:
_macos_seq_memory_writer:
	stp	x5, x6, [sp, -16]!
	stp	x7, x8, [sp, -16]!

	mov	x5, 12345
	mov	x6, 54321

	mov	x7, x0
	mov	x8, x1
.L0:
	mov	x0, x7
	mov	x1, x8
.L2:
	stp	x5, x6, [x0], 16
	stp	x5, x6, [x0], 16
	stp	x5, x6, [x0], 16
	stp	x5, x6, [x0], 16
	stp	x5, x6, [x0], 16
	stp	x5, x6, [x0], 16
	stp	x5, x6, [x0], 16
	stp	x5, x6, [x0], 16
	subs	x1, x1, 128
	bne	.L2

	subs	x2, x2, 1
	bne	.L0

	ldp	x8, x7, [sp], 16
	ldp	x6, x5, [sp], 16
	ret

