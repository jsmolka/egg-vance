arithmetic:
        ; Tests for arithmetic operations

t100:
        ; Carry flag addition
        imm32   r0, 0xFFFFFFFE

        add     r0, 1
        bcs     t100f

        add     r0, 1
        bcc     t100f

        b       t101

t100f:
        Failed 100

t101:
        ; Carry flag subtraction
        mov     r0, 1

        sub     r0, 2
        bcs     t101f

        mov     r0, 3

        sub     r0, 2
        bcc     t101f

        sub     r0, 1
        bcc     t101f

        b       t102

t101f:
        Failed 101

t102:
        ; Overflow flag addition
        imm32   r0, 0x7FFFFFFE

        add     r0, 1
        bvs     t102f

        add     r0, 1
        bvc     t102f

        b       t103

t102f:
        Failed 102

t103:
        ; Overflow flag subtraction
        imm32   r0, 0x80000001

        sub     r0, 1
        bvs     t103f

        sub     r0, 1
        bvc     t103f

        b       t104

t103f:
        Failed 103

t104:
        ; Thumb 2: add rd, rs, imm3
        mov     r0, #0

        add     r0, r0, #3
        add     r0, r0, 7
        cmp     r0, 10
        bne     t104f

        b       t105

t104f:
        Failed 104

t105:
        ; Thumb 3: add rd, imm8
        mov     r0, 0

        add     r0, 128
        add     r0, 127
        cmp     r0, 255
        bne     t105f

        b       t106

t105f:
        Failed 105

t106:
        ; Thumb 5: add rd, rs (high registers)
        mov     r0, 1
        mov     r1, 0
        mov     r8, r1
        mov     r9, r1

        add     r8, r0
        add     r9, r8
        add     r0, r9
        cmp     r0, 2
        bne     t106f

        b       t107

t106f:
        Failed 106

t107:
        ; Thumb 12: add rd, sp, imm8 << 2
        mov     r0, sp
        add     r0, 128
        mov     r1, 0

        add     r1, sp, 128
        cmp     r1, r0
        bne     t107f

        b       t108

t107f:
        Failed 107

t108:
        ; Thumb 12: add rd, pc, imm8 << 2
        mov     r0, pc
        add     r0, 10
        mov     r1, 0

        add     r1, pc, 4
        cmp     r1, r0
        bne     t108f

        b       t109

t108f:
        Failed 108

t109:
        ; Thumb 13: add sp, imm7 << 2
        mov     r0, sp

        add     sp, 128
        add     sp, -128
        cmp     sp, r0
        bne     t109f

        b       t110

t109f:
        Failed 109

t110:
        ; Thumb 4: adc rd, rs
        mov     r0, 8
        cmn     r0, r0

        adc     r0, r0
        cmp     r0, 16
        bne     t110f

        cmp     r0, r0

        adc     r0, r0
        cmp     r0, 33
        bne     t110f

        b       t111

t110f:
        Failed 110

t111:
        ; Thumb 2: sub rd, rs, imm3
        mov     r0, 10

        sub     r0, r0, 3
        sub     r0, r0, 7
        bne     t111f

        b       t112

t111f:
        Failed 111

t112:
        ; Thumb 3: sub rd, imm8
        mov     r0, 255
        add     r0, 10

        sub     r0, 255
        sub     r0, 10
        bne     t112f

        b       t113

t112f:
        Failed 112

t113:
        ; Thumb 2: sub rd, rs, rn
        mov     r0, 255
        add     r0, 10

        mov     r1, 255
        mov     r2, 10

        sub     r0, r1
        sub     r0, r2
        bne     t113f

        b       t114

t113f:
        Failed 113

t114:
        ; Thumb 4: sbc rd, rs
        mov     r0, 64
        mov     r1, 16
        cmn     r0, r0

        sbc     r0, r1
        cmp     r0, 47
        bne     t114f

        cmp     r0, r0

        sbc     r0, r1
        cmp     r0, 31
        bne     t114f

        b       t115

t114f:
        Failed 114

t115:
        ; Thumb 4: neg rd, rs
        mov     r0, 4
        mov     r1, 0
        sub     r1, r0

        neg     r0, r0
        cmp     r0, r1
        bne     t115f

        b       t116

t115f:
        Failed 115

t116:
        ; Thumb 3: cmp rd, imm8
        mov     r0, 128

        cmp     r0, 128
        bne     t116f
        bmi     t116f
        bcc     t116f
        bvs     t116f

        cmp     r0, 255
        beq     t116f
        bpl     t116f
        bcs     t116f

        mov     r0, 1
        lsl     r0, 31

        cmp     r0, 255
        bvc     t116f

        b       t117

t116f:
        Failed 116

t117:
        ; Thumb 4: cmp rd, rs
        mov     r0, 128
        mov     r1, 255

        cmp     r0, r0
        bne     t117f
        bmi     t117f
        bcc     t117f
        bvs     t117f

        cmp     r0, r1
        beq     t117f
        bpl     t117f
        bcs     t117f

        mov     r0, 1
        lsl     r0, 31

        cmp     r0, r1
        bvc     t117f

        b       t118

t117f:
        Failed 117

t118:
        ; Thumb 5: cmp rd, rs (high registers)
        mov     r0, 128
        mov     r1, 255
        mov     r8, r0
        mov     r9, r1

        cmp     r8, r8
        bne     t118f
        bmi     t118f
        bcc     t118f
        bvs     t118f

        cmp     r8, r9
        beq     t118f
        bpl     t118f
        bcs     t118f

        mov     r0, 1
        lsl     r0, 31
        mov     r8, r0

        cmp     r8, r9
        bvc     t118f

        b       t119

t118f:
        Failed 118

t119:
        ; Thumb 4: cmn rd, rs
        mov     r0, 128

        cmn     r0, r0
        beq     t119f
        bmi     t119f
        bcs     t119f
        bvs     t119f

        mov     r0, 0

        cmn     r0, r0
        bne     t119f

        mvn     r0, r0

        cmn     r0, r0
        bpl     t119f
        bcc     t119f

        mov     r0, 1
        lsl     r0, 31
        sub     r0, 1
        mov     r1, 1

        cmn     r0, r1
        bvc     t119f

        b       t120

t119f:
        Failed 119

t120:
        ; Thumb 4: mul rd, rs
        mov     r0, 2

        mul     r0, r0
        mul     r0, r0
        cmp     r0, 16
        bne     t120f

        ; Branch to branches.asm
        b       branches

t120f:
        Failed 120
