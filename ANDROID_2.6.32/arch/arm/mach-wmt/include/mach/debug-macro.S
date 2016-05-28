/*++
	linux/include/asm-arm/arch-wmt/debug-macro.S
	
	Debugging macro include header

	Some descriptions of such software. Copyright (c) 2008  WonderMedia Technologies, Inc.

	This program is free software: you can redistribute it and/or modify it under the
	terms of the GNU General Public License as published by the Free Software Foundation,
	either version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful, but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
	PARTICULAR PURPOSE.  See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with
	this program.  If not, see <http://www.gnu.org/licenses/>.

	WonderMedia Technologies, Inc.
	10F, 529, Chung-Cheng Road, Hsin-Tien, Taipei 231, R.O.C.
--*/

	@Current kernel I/O mmap design is phys = virt

	.macro	addruart,rx
	mov	\rx, #0xd8000000
	add     \rx, \rx, #0x00200000   @ UART0_BASE = 0xd8200000
	.endm

	.macro	senduart,rd,rx
	ldr	r4,  [\rx, #0x20]           @ FIFO control register
	tst	r4,  #0x01                  @ Check FIFOEN bit
	strne   \rd, [\rx, #0x1000]     @ TX FIFO
	streq	\rd, [\rx]              @ Transmit data register
	.endm

	.macro	waituart,rd,rx
1001:
	ldr	\rd, [\rx, #0x1C]       @ Status register
	tst	\rd, #2                 @ Transmit busy
	bne	1001b                   @ If busy then wait
	.endm

	.macro	busyuart,rd,rx
1001:
	ldr	\rd, [\rx, #0x1C]       @ Status register
	tst	\rd, #2                 @ Transmit busy
	bne	1001b
	.endm

