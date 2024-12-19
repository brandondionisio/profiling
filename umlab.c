/*
 * umlab.c
 *
 * Functions to generate UM unit tests. Once complete, this module
 * should be augmented and then linked against umlabwrite.c to produce
 * a unit test writing program.
 *  
 * A unit test is a stream of UM instructions, represented as a Hanson
 * Seq_T of 32-bit words adhering to the UM's instruction format.  
 * 
 * Any additional functions and unit tests written for the lab go
 * here. 
 *  
 */
 

#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <seq.h>
#include <bitpack.h>


typedef uint32_t Um_instruction;
typedef enum Um_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, ACTIVATE, INACTIVATE, OUT, IN, LOADP, LV
} Um_opcode;


/* Functions that return the two instruction types */

Um_instruction three_register(Um_opcode op, int ra, int rb, int rc) {
        Um_instruction instruction = 0;

        instruction = Bitpack_newu(instruction, 4, 28, op);
        instruction = Bitpack_newu(instruction, 3, 6, ra);
        instruction = Bitpack_newu(instruction, 3, 3, rb);
        instruction = Bitpack_newu(instruction, 3, 0, rc);
        
        return instruction;
}
        
Um_instruction loadval(unsigned ra, unsigned val) {
        Um_instruction instruction = 0;

        uint32_t op = 13;
        
        instruction = Bitpack_newu(instruction, 4, 28, op);
        instruction = Bitpack_newu(instruction, 3, 25, ra);
        instruction = Bitpack_newu(instruction, 25, 0, val);

        return instruction;
}

/* Wrapper functions for each of the instructions */

typedef enum Um_register { r0 = 0, r1, r2, r3, r4, r5, r6, r7 } Um_register;

static inline Um_instruction add(Um_register a, Um_register b, Um_register c) 
{
        return three_register(ADD, a, b, c);
}

Um_instruction multiply(Um_register a, Um_register b, Um_register c) 
{
        return three_register(MUL, a, b, c);
}

Um_instruction divide(Um_register a, Um_register b, Um_register c) 
{
        return three_register(DIV, a, b, c);
}

Um_instruction nand(Um_register a, Um_register b, Um_register c) 
{
        return three_register(NAND, a, b, c);
}

static inline Um_instruction halt(void) 
{
        return three_register(HALT, 0, 0, 0);
}

Um_instruction cmov(Um_register a, Um_register b, Um_register c) 
{
        return three_register(CMOV, a, b, c);
}

Um_instruction activate(Um_register b, Um_register c) 
{
        return three_register(ACTIVATE, 0, b, c);
}

Um_instruction inactivate(Um_register c) 
{
        return three_register(INACTIVATE, 0, 0, c);
}

Um_instruction input(Um_register c) {
        return three_register(IN, 0, 0, c);
}

Um_instruction output(Um_register c) 
{
        return three_register(OUT, 0, 0, c);
}

Um_instruction sload(Um_register a, Um_register b, Um_register c) 
{
        return three_register(SLOAD, a, b, c);
}

Um_instruction sstore(Um_register a, Um_register b, Um_register c) 
{
        return three_register(SSTORE, a, b, c);
}

Um_instruction loadp(Um_instruction b, Um_instruction c)
{
        return three_register(LOADP, 0, b, c);
}

/* Functions for working with streams */

static inline void append(Seq_T stream, Um_instruction inst)
{
        assert(sizeof(inst) <= sizeof(uintptr_t));
        Seq_addhi(stream, (void *)(uintptr_t)inst);
}

const uint32_t Um_word_width = 32;

void Um_write_sequence(FILE *output, Seq_T stream)
{
        assert(output != NULL && stream != NULL);
        int stream_length = Seq_length(stream);
        for (int i = 0; i < stream_length; i++) {
                Um_instruction inst = (uintptr_t)Seq_remlo(stream);
                for (int lsb = Um_word_width - 8; lsb >= 0; lsb -= 8) {
                        fputc(Bitpack_getu(inst, 8, lsb), output);
                }
        }     
}

/* Unit tests for the UM */

void build_halt_test(Seq_T stream)
{
        append(stream, halt());
}

void build_verbose_halt_test(Seq_T stream)
{
        append(stream, halt());
        append(stream, loadval(r1, 'B'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'a'));
        append(stream, output(r1));
        append(stream, loadval(r1, 'd'));
        append(stream, output(r1));
        append(stream, loadval(r1, '!'));
        append(stream, output(r1));
        append(stream, loadval(r1, '\n'));
        append(stream, output(r1));
}

void build_output_test(Seq_T stream)
{
        append(stream, output(r1));
        append(stream, halt());
}

/* expected output: < */
void build_output_load_test(Seq_T stream)
{
        append(stream, loadval(r1, 60));
        append(stream, output(r1));
        append(stream, halt());
}

/* Tests that the add instruction works */
void build_print_six(Seq_T stream)
{
        append(stream, loadval(r1, 48));
        append(stream, loadval(r2, 6));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* Tests that the add instruction works with non-adjacent registers */
void add_non_adjacent_test(Seq_T stream) 
{
        append(stream, loadval(r4, 17));
        append(stream, loadval(r1, 73));
        append(stream, add(r7, r1, r4));
        append(stream, output(r7));
        append(stream, halt());
}

// void add_nonprintable_test(Seq_T stream)
// {
//         append(stream, loadval(r4, 17));
//         append(stream, loadval(r1, 173));
//         append(stream, add(r7, r1, r4));
//         append(stream, output(r7));
//         append(stream, halt());
// }

/* Tests input and output are same */
void in_and_out_test(Seq_T stream)
{
        append(stream, input(r1));
        append(stream, output(r1));
        append(stream, halt());
}

/* Tests add with an input */
void add_input_test(Seq_T stream)
{
        append(stream, input(r1));
        append(stream, loadval(r2, 2));
        append(stream, add(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* Multiply test with hardcoded values */
/* expected output: lol'\n' */
void multiply_test(Seq_T stream)
{
        append(stream, loadval(r1, 111));
        append(stream, loadval(r2, 2));
        append(stream, loadval(r3, 54));
        append(stream, multiply(r4, r2, r3));
        append(stream, output(r4));
        append(stream, output(r1));
        append(stream, output(r4));
        append(stream, halt());
}

/* Multiply test that uses user input */
void multiply_test_input(Seq_T stream)
{
        append(stream, input(r1));
        append(stream, loadval(r2, 3));
        append(stream, multiply(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* Multiply by 0 test */
/* expected output: NUL */
void multiply_by_zero_test(Seq_T stream)
{
        append(stream, loadval(r1, 0));
        append(stream, loadval(r2, 2));
        append(stream, multiply(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* Multiply by 1 test */
/* expected output: $ */
void multiply_by_one_test(Seq_T stream)
{
        append(stream, loadval(r1, 1));
        append(stream, loadval(r2, 36));
        append(stream, multiply(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* Multiply by self test */
/* expected output: 1 */
void multiply_by_self_test(Seq_T stream)
{
        append(stream, loadval(r1, 7));
        append(stream, multiply(r3, r1, r1));
        append(stream, output(r3));
        append(stream, halt());
}

/* expected output: +o+'\n' */
void divide_test(Seq_T stream)
{
        append(stream, loadval(r1, 111));
        append(stream, loadval(r2, 129));
        append(stream, loadval(r3, 3));
        append(stream, divide(r4, r2, r3));
        append(stream, output(r4));
        append(stream, output(r1));
        append(stream, output(r4));
        append(stream, halt());
}

/* Tests divide by 1 */
/* expected value: > */
void divide_by_one_test(Seq_T stream)
{
        append(stream, loadval(r1, 1));
        append(stream, loadval(r2, 62));
        append(stream, divide(r3, r2, r1));
        append(stream, output(r3));
        append(stream, halt());
}


/* Test divide by self hardcoded */
/* expected: SOH */
void divide_by_self_test(Seq_T stream)
{
        append(stream, loadval(r1, 5));
        append(stream, divide(r3, r1, r1));
        append(stream, output(r3));
        append(stream, halt());
}

/* Test divide by the input */
void divide_by_input_test(Seq_T stream)
{
        append(stream, input(r1));
        append(stream, loadval(r2, 2));
        append(stream, divide(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* Test divide by self from input */
/* expected: SOH except when 0*/
void divide_by_self_input_test(Seq_T stream)
{
        append(stream, input(r1));
        append(stream, divide(r3, r1, r1));
        append(stream, output(r3));
        append(stream, halt());
}

/* Test bitwise NAND */
/* expected output: ? */
void nand_test(Seq_T stream)
{
        append(stream, loadval(r0, 2));
        append(stream, loadval(r1, 33554431));
        for (int i = 0; i < 7; i++) {
                append(stream, multiply(r1, r1, r0));
        }
        append(stream, nand(r3, r1, r1));
        append(stream, divide(r3, r3, r0));
        append(stream, output(r3));
        append(stream, halt());
}

/* Test bitwise NAND with input */
void nand_input_test(Seq_T stream)
{
        append(stream, input(r1));
        append(stream, nand(r3, r1, r1));
        append(stream, nand(r1, r3, r3));
        append(stream, output(r1));
        append(stream, halt());
}

/* Test cmov */
/* expected output: V */
void cmov_test_0(Seq_T stream)
{
        append(stream, loadval(r3, 86));
        append(stream, loadval(r1, 85));
        append(stream, loadval(r2, 0));
        append(stream, cmov(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* Test cmov */
/* expected output: U */
void cmov_test_1(Seq_T stream)
{
        append(stream, loadval(r3, 86));
        append(stream, loadval(r1, 85));
        append(stream, loadval(r2, 1));
        append(stream, cmov(r3, r1, r2));
        append(stream, output(r3));
        append(stream, halt());
}

/* expected output: GO */
void map_test(Seq_T stream)
{
        append(stream, loadval(r3, 79));

        /* mapping new segment with 40 length
         * r2 has the index */
        for (int i = 0; i < 71; i++) {
                append(stream, activate(r2, r3));
        }
        
        append(stream, output(r2));
        
        append(stream, output(r3));
        append(stream, halt());
}

/* expected output: qK */
void unmap_test_1(Seq_T stream)
{
        append(stream, loadval(r3, 80));

        for (int i = 0; i < 113; i++) {
                append(stream, activate(r2, r3));
        }

        append(stream, output(r2));

        append(stream, loadval(r0, 75));
        append(stream, inactivate(r0));

        append(stream, activate(r4, r3));
        append(stream, output(r4));

        append(stream, halt());
}

/* expected output: qp */
void unmap_test_2(Seq_T stream)
{
        append(stream, loadval(r3, 80));

        for (int i = 0; i < 113; i++) {
                append(stream, activate(r2, r3));
        }

        append(stream, output(r2));

        for (int i = 1; i < 113; i++) {
                append(stream, loadval(r0, i));
                append(stream, inactivate(r0));
        }

        append(stream, activate(r2, r3));
        append(stream, output(r2));

        append(stream, halt());
}

void segment_store_test(Seq_T stream)
{
        append(stream, loadval(r3, 87));
        append(stream, activate(r2, r3));
        
        append(stream, loadval(r0, 36));
        
        append(stream, divide(r1, r0, r0));

        append(stream, loadval(r2, 80));
        append(stream, sstore(r1, r2, r0));

        append(stream, loadval(r0, 89));
        append(stream, loadval(r1, 65));

        append(stream, output(r0));
        append(stream, output(r1));
        append(stream, output(r0));

        append(stream, halt());
}

/* expected output: q$ */
void segment_sl_test(Seq_T stream)
{
        append(stream, loadval(r3, 87));

        for (int i = 0; i < 113; i++) {
                append(stream, activate(r2, r3));
        }

        append(stream, output(r2));

        /* segment stuff */
        append(stream, loadval(r0, 36));
        append(stream, loadval(r1, 50));
        append(stream, loadval(r2, 80));
        append(stream, sstore(r1, r2, r0));

        append(stream, sload(r5, r1, r2));
        append(stream, output(r5));

        append(stream, halt());
}



/* expected output: WWWWWWWWWWWWWWWWWWWWWWWWWWWWW */
void load_test_not_0(Seq_T stream)
{
        /* load registers 3 and 6 */
        append(stream, loadval(r3, 40)); /* loads char for length of uarray */
        append(stream, loadval(r2, 1)); /* loads 1 */
        append(stream, loadval(r7, 0)); /* loads 0 */

        /* map a new segment thats 40 long */
        append(stream, activate(r2, r3)); /* map 40-long segment */

        append(stream, loadval(r5, OUT));
        append(stream, loadval(r6, 2));
        for (int i = 0; i < 28; i++) {
                append(stream, multiply(r5, r5, r6));
        }

        for (int i = 0; i < 39; i++) {
                append(stream, loadval(r4, i)); /* loading index */
                append(stream, sstore(r2, r4, r5)); /* store "output reg 0"
                at all indices */
        }

        append(stream, loadval(r5, HALT));
        append(stream, loadval(r6, 2));
        for (int i = 0; i < 28; i++) {
                append(stream, multiply(r5, r5, r6));
        }
        append(stream, loadval(r4, 39)); /* loading index */
        append(stream, sstore(r2, r4, r5));

        append(stream, loadval(r0, 87));
        append(stream, loadp(r2, r7)); /* load 0 segment at 0th word */
}