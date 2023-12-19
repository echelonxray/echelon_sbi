#ifndef _insertion_csr_h
#define _insertion_csr_h

#define CSR_SSTATUS        0x100
#define CSR_SEDELEG        0x102
#define CSR_SIDELEG        0x103
#define CSR_SIE            0x104
#define CSR_STVEC          0x105
#define CSR_SCOUNTEREN     0x106
#define CSR_SSCRATCH       0x140
#define CSR_SEPC           0x141
#define CSR_SCAUSE         0x142
#define CSR_STVAL          0x143
#define CSR_SIP            0x144
#define CSR_SATP           0x180

#define CSR_MVENDORID      0xF11
#define CSR_MARCHID        0xF12
#define CSR_MIMPID         0xF13
#define CSR_MHARTID        0xF14
#define CSR_MSTATUS        0x300
#define CSR_MISA           0x301
#define CSR_MEDELEG        0x302
#define CSR_MIDELEG        0x303
#define CSR_MIE            0x304
#define CSR_MTVEC          0x305
#define CSR_MCOUNTEREN     0x306
#define CSR_MSCRATCH       0x340
#define CSR_MEPC           0x341
#define CSR_MCAUSE         0x342
#define CSR_MTVAL          0x343
#define CSR_MIP            0x344

#define CSR_PMPCFG0        0x3A0
#define CSR_PMPCFG1        0x3A1 // 32-Bit Only
#define CSR_PMPCFG2        0x3A2
#define CSR_PMPCFG3        0x3A3 // 32-Bit Only

#define CSR_PMPADDR0       0x3B0
#define CSR_PMPADDR1       0x3B1
#define CSR_PMPADDR2       0x3B2
#define CSR_PMPADDR3       0x3B3
#define CSR_PMPADDR4       0x3B4
#define CSR_PMPADDR5       0x3B5
#define CSR_PMPADDR6       0x3B6
#define CSR_PMPADDR7       0x3B7
#define CSR_PMPADDR8       0x3B8
#define CSR_PMPADDR9       0x3B9
#define CSR_PMPADDR10      0x3BA
#define CSR_PMPADDR11      0x3BB
#define CSR_PMPADDR12      0x3BC
#define CSR_PMPADDR13      0x3BD
#define CSR_PMPADDR14      0x3BE
#define CSR_PMPADDR15      0x3BF

#define CSR_MCYCLE         0xB00
#define CSR_MINSTRET       0xB02
#define CSR_MHPMCOUNTER3   0xB03
#define CSR_MHPMCOUNTER4   0xB04
#define CSR_MHPMCOUNTER5   0xB05
#define CSR_MHPMCOUNTER6   0xB06
#define CSR_MHPMCOUNTER7   0xB07
#define CSR_MHPMCOUNTER8   0xB08
#define CSR_MHPMCOUNTER9   0xB09
#define CSR_MHPMCOUNTER10  0xB0A
#define CSR_MHPMCOUNTER11  0xB0B
#define CSR_MHPMCOUNTER12  0xB0C
#define CSR_MHPMCOUNTER13  0xB0D
#define CSR_MHPMCOUNTER14  0xB0E
#define CSR_MHPMCOUNTER15  0xB0F
#define CSR_MHPMCOUNTER16  0xB10
#define CSR_MHPMCOUNTER17  0xB11
#define CSR_MHPMCOUNTER18  0xB12
#define CSR_MHPMCOUNTER19  0xB13
#define CSR_MHPMCOUNTER20  0xB14
#define CSR_MHPMCOUNTER21  0xB15
#define CSR_MHPMCOUNTER22  0xB16
#define CSR_MHPMCOUNTER23  0xB17
#define CSR_MHPMCOUNTER24  0xB18
#define CSR_MHPMCOUNTER25  0xB19
#define CSR_MHPMCOUNTER26  0xB1A
#define CSR_MHPMCOUNTER27  0xB1B
#define CSR_MHPMCOUNTER28  0xB1C
#define CSR_MHPMCOUNTER29  0xB1D
#define CSR_MHPMCOUNTER30  0xB1E
#define CSR_MHPMCOUNTER31  0xB1F
#define CSR_MCYCLEH        0xB80
#define CSR_MINSTRETH      0xB82
#define CSR_MHPMCOUNTER3H  0xB83
#define CSR_MHPMCOUNTER4H  0xB84
#define CSR_MHPMCOUNTER5H  0xB85
#define CSR_MHPMCOUNTER6H  0xB86
#define CSR_MHPMCOUNTER7H  0xB87
#define CSR_MHPMCOUNTER8H  0xB88
#define CSR_MHPMCOUNTER9H  0xB89
#define CSR_MHPMCOUNTER10H 0xB8A
#define CSR_MHPMCOUNTER11H 0xB8B
#define CSR_MHPMCOUNTER12H 0xB8C
#define CSR_MHPMCOUNTER13H 0xB8D
#define CSR_MHPMCOUNTER14H 0xB8E
#define CSR_MHPMCOUNTER15H 0xB8F
#define CSR_MHPMCOUNTER16H 0xB90
#define CSR_MHPMCOUNTER17H 0xB91
#define CSR_MHPMCOUNTER18H 0xB92
#define CSR_MHPMCOUNTER19H 0xB93
#define CSR_MHPMCOUNTER20H 0xB94
#define CSR_MHPMCOUNTER21H 0xB95
#define CSR_MHPMCOUNTER22H 0xB96
#define CSR_MHPMCOUNTER23H 0xB97
#define CSR_MHPMCOUNTER24H 0xB98
#define CSR_MHPMCOUNTER25H 0xB99
#define CSR_MHPMCOUNTER26H 0xB9A
#define CSR_MHPMCOUNTER27H 0xB9B
#define CSR_MHPMCOUNTER28H 0xB9C
#define CSR_MHPMCOUNTER29H 0xB9D
#define CSR_MHPMCOUNTER30H 0xB9E
#define CSR_MHPMCOUNTER31H 0xB9F

#define CSR_MCOUNTINHIBIT  0x320
#define CSR_MHPMEVENT3     0x323
#define CSR_MHPMEVENT4     0x324
#define CSR_MHPMEVENT5     0x325
#define CSR_MHPMEVENT6     0x326
#define CSR_MHPMEVENT7     0x327
#define CSR_MHPMEVENT8     0x328
#define CSR_MHPMEVENT9     0x329
#define CSR_MHPMEVENT10    0x32A
#define CSR_MHPMEVENT11    0x32B
#define CSR_MHPMEVENT12    0x32C
#define CSR_MHPMEVENT13    0x32D
#define CSR_MHPMEVENT14    0x32E
#define CSR_MHPMEVENT15    0x32F
#define CSR_MHPMEVENT16    0x330
#define CSR_MHPMEVENT17    0x331
#define CSR_MHPMEVENT18    0x332
#define CSR_MHPMEVENT19    0x333
#define CSR_MHPMEVENT20    0x334
#define CSR_MHPMEVENT21    0x335
#define CSR_MHPMEVENT22    0x336
#define CSR_MHPMEVENT23    0x337
#define CSR_MHPMEVENT24    0x338
#define CSR_MHPMEVENT25    0x339
#define CSR_MHPMEVENT26    0x33A
#define CSR_MHPMEVENT27    0x33B
#define CSR_MHPMEVENT28    0x33C
#define CSR_MHPMEVENT29    0x33D
#define CSR_MHPMEVENT30    0x33E
#define CSR_MHPMEVENT31    0x33F

#define CSR_TSELECT        0x7A0
#define CSR_TDATA1         0x7A1
#define CSR_TDATA2         0x7A2
#define CSR_TDATA3         0x7A3

#define CSR_DCSR           0x7B0
#define CSR_DPC            0x7B1
#define CSR_DSCRATCH0      0x7B2
#define CSR_DSCRATCH1      0x7B3

#define CSRI_WRITE(csr, value) \
	{ \
		unsigned long out; \
		__asm__ __volatile__ ("csrrw %0, %1, %2 \n" : "=r" (out), "i" (csr) : "r" (value) ); \
		out; \
	}
#define CSRI_BITSET(csr, value) \
	{ \
		unsigned long out; \
		__asm__ __volatile__ ("csrrs %0, %1, %2 \n" : "=r" (out), "i" (csr) : "r" (value) ); \
		out; \
	}
#define CSRI_BITCLR(csr, value) \
	{ \
		unsigned long out; \
		__asm__ __volatile__ ("csrrc %0, %1, %2 \n" : "=r" (out), "i" (csr) : "r" (value) ); \
		out; \
	}

#endif
