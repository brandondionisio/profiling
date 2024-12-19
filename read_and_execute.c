
/**************************************************************
 *
 *                     read_and_execute.c
 *
 *     Assignment: HW 6: um
 *        Authors: Dan Glorioso & Brandon Dionisio (dglori02 & bdioni01)
 *           Date: 04/11/24
 *
 *     Summary: Implementation of the functions to read the instructions from
 *              an input file into the address space, execute the instructions
 *              in the address space appropriately by calling external  
 *              operations functions, and fully free the segments in the 
 *              address space after all instructions have been executed.
 * 
 **************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include "read_and_execute.h"
#include "seq.h"
#include "mem.h"

/****************** um_driver *******************
 * 
 * Function to call the appropriate functions to read the instructions from the
 * file into the address space, execute the instructions, and free the segments
 * in the address space. 
 *
 * Parameters:
 *            FILE *fp: pointer to the file that holds the instructions
 *     size_t num_inst: number of instructions in the file
 * Returns:
 *        None.
 * Expects:
 *      The file pointer is not NULL.
 *      The number of instructions is greater than 0.
 * Notes: 
 *      The function initializes 8 registers and sets each to 0. It then 
 *      creates a new address space, reads the instructions from the file into
 *      the address space, executes each instruction, and frees all the 
 *      segments in the address space.
 * 
 ********************************************/
extern void um_driver(FILE *fp, size_t num_inst) 
{
        /* Initialize 8 registers and set each to 0 */
        uint32_t registers[8] = { 0 };

        /* Initalize the fields of the Address_space struct */
        Seq_T unmapped = Seq_new(0);
        uint32_t *lengths = (uint32_t *)malloc(sizeof(uint32_t));
        uint32_t **in_use = (uint32_t **)malloc(sizeof(uint32_t *));

        lengths[0] = 0;
        in_use[0] = 0;

        uint32_t *zero_seg = (uint32_t *)malloc((int)num_inst * sizeof(uint32_t));
        
        /* Initialize variables to hold instruction and character inputted */
        uint32_t word;
        int c;

        /* Read the first character from the file */
        c = getc(fp);

        /* Read the intstructions from the file into 0 segment */
        for (size_t inst_count = 0; inst_count < num_inst; inst_count += 1) {
                /* Initialize word to 0 to start */
                word = 0;

                /* Read in the 32-bit word from the input file */
                for (int bit = 3; bit >= 0; bit -= 1) {
                        /* Add the character to the word */
                        word = (word >> ((bit * 8) + 8)) << ((bit * 8) + 8) 
                                | (word << (64 - (bit * 8))) >> (64 - (bit * 8)) 
                                | ((uint64_t)c << (bit * 8));

                        /* Read the next character from the file */
                        c = getc(fp);
                }

                /* Add the instruction to the end of the zero_seg array */
                zero_seg[inst_count] = word;
        }
        /* After reading in the instructions, close the file */
        fclose(fp);

        /* Initialize program counter */
        size_t prog_counter = 0;
        int seq_counter = 1;

        /* Initialize boolean to check if last instruction was a LOADP so that
         * the program does not increment new prog_counter at end of loop */
        bool last_loadp = false;

        /* Execute instructions until program counter reaches the end of the 
         * number of instructions there are */
        while (prog_counter < num_inst) {
                /* Get instruction from index of prog_counter from 0 segment */
                uint32_t instruction = zero_seg[prog_counter];

                /* Fetch register indices from instruction */
                uint32_t a_index = (instruction << 23) >> 29;
                uint32_t b_index = (instruction << 26) >> 29;
                uint32_t c_index =  (instruction << 29) >> 29;
                uint32_t a_lv_index = (instruction << 4) >> 29;
                uint32_t value = (instruction << 7) >> 7;

                /* Reset boolean to false */
                last_loadp = false;

                /* Execute instruction based on the opcode of instruction */
                uint32_t op = instruction >> 28;
                if (op == 13) {
                        registers[a_lv_index] = value;
                } else if (op == 1) {
                        if (registers[b_index] == 0) {
                                registers[a_index] = zero_seg[registers[c_index]];
                        } else {
                                /* Set the value in register a to the value of the word at the segment
                                * in register b and the offset in register c */
                                uint32_t ID = registers[b_index];

                                /* Get the UArray at the given ID */
                                uint32_t *arr = in_use[ID];

                                /* Get the word at the specified index from UArray */
                                registers[a_index] = (arr[registers[c_index]]);
                        }
                } else if (op == 2) {
                        if (registers[a_index] == 0) {
                                zero_seg[registers[b_index]] = registers[c_index];
                        } else {
                                /* Get the word at the address in register a and the offset of 
                                * register b and store value in register c to that word location */
                                uint32_t ID = registers[a_index];
                                uint32_t *arr = in_use[ID];

                                /* Get the word at the specified index from UArray */
                                uint32_t *word = &(arr[registers[b_index]]);

                                *word = registers[c_index];
                        }
                } else if (op == 12) {
                        /* Check if register b is not 0 */
                        if (registers[b_index] != 0) {
                                /* Fetch the length of the segment being duplicated */
                                int len = lengths[registers[b_index]];

                                /* Create a new segment of the length of the segment being 
                                * duplicated with each element being the size of a 32-bit
                                * word */
                                zero_seg = (uint32_t *)realloc(zero_seg, len * sizeof(uint32_t));
                                
                                uint32_t *arr = in_use[registers[b_index]];

                                /* Copy the contents of the segment being duplicated to the
                                * newly created segment */
                                for (int seg_index = 0; seg_index < len; seg_index += 1) {
                                        zero_seg[seg_index] = arr[seg_index];
                                }

                                /* Update the number of instructions to the length of the 
                                * newly duplicated segment that is now in the 0 segment */
                                num_inst = (size_t)len;
                        }
                        /* Update the program counter to the value in register c */
                        prog_counter = (size_t)(registers[c_index]);

                        /* Set bool to true to indicate LOADP was 
                         * last operation executed */
                        last_loadp = true;
                } else if (op == 3) {
                        registers[a_index] = (registers[b_index] + registers[c_index]);
                } else if (op == 6) {
                        registers[a_index] = ~(registers[b_index] & registers[c_index]);
                } else if (op == 0) {
                        if (registers[c_index] != 0) {
                                registers[a_index] = registers[b_index];
                        }
                } else if (op == 5) {
                        registers[a_index] = registers[b_index] / registers[c_index];
                } else if (op == 4) {
                        registers[a_index] = (registers[b_index] * registers[c_index]);
                } else if (op == 8) {
                        /* Get the length of the segment from register c */
                        int length = registers[c_index];

                        uint32_t *arr = (uint32_t *)malloc(length * sizeof(uint32_t));

                        /* Initalize all the words in the newly mapped segment to 0 */
                        for (int i = 0; i < length; i += 1) {
                                arr[i] = 0;
                        }

                        /* Check for unmapped segment */
                        if (Seq_length(unmapped) == 0) {
                                /* There are no unmapped segments, so save the length of the
                                * sequence of segments (non-zero) to register b */
                                registers[b_index] = seq_counter;

                                in_use = (uint32_t **)realloc(in_use, (seq_counter + 1) * sizeof(uint32_t *));
                                lengths = (uint32_t *)realloc(lengths, (seq_counter + 1) * sizeof(uint32_t));
                                in_use[seq_counter] = arr;
                                lengths[seq_counter] = length;

                                seq_counter += 1;
                        } else {
                                /* Reuse unmapped segments for new segment by fetching the 
                                 * index from the sequence of unmapped segments and removing it
                                 * from the unmapped sequence */
                                uint32_t unmap_index = (uint32_t)(uintptr_t)Seq_remhi(unmapped);

                                /* Save the index of the unmapped segment to register b, which
                                 * is not all zeros */
                                registers[b_index] = unmap_index;

                                /* Add the UArray to the index of first unmapped segment */
                                in_use[unmap_index] = arr;
                                lengths[unmap_index] = length;
                                
                        }

                } else if (op == 9) {
                        /* Get the ID of the segment to be unmapped */
                        uint32_t ID = registers[c_index];
                        
                        /* Free the segment at the given ID */
                        uint32_t *seg = in_use[ID];

                        /* Free the UArray if it is not NULL */
                        if (seg != NULL) {
                                free(seg);
                        }

                        /* free segment end */
                        in_use[ID] = 0;

                        /* Add ID of the unmapped segment to the sequence of unmapped IDs */
                        Seq_addhi(unmapped, (void *)(uintptr_t)ID);

                } else if (op == 10) {
                        /* Output the character in register c */
                        printf("%c", registers[c_index]);

                } else if (op == 11) {
                        /* Get input from stdin */
                        int input = getchar();

                        /* If EOF, set input to 32-bit word in which every bit is 1 */
                        if (input == EOF) {
                                input = 0xFFFFFFFF;
                        }
                        
                        /* Store input in register */
                        registers[c_index] = input;

                } else {
                        /* Free segment 0 */
                        free(zero_seg);

                        /* Free all the segments in the address space */
                        for (int i = 0; i < seq_counter; i += 1) {
                                uint32_t *seg = in_use[i];
                        
                                /* Free the UArray if it is not NULL */
                                if (seg != NULL) {
                                        free(seg);
                                }
                        }
                        
                        free(in_use);
                        free(lengths);

                        /* Free the sequence of unmapped IDs */
                        if (unmapped != NULL) {
                                Seq_free(&(unmapped));
                        }
                        
                        /* Exit the program */
                        exit(0);

                }
                /* Increment program counter to next instruction as long as 
                 * last instruction was not LOADP */
                if (!last_loadp) {
                        prog_counter += 1;
                }
        }

        /* Free all the segments in the address space */
        free(zero_seg);
        for (int i = 0; i < seq_counter; i += 1) {
                uint32_t *seg = in_use[i];
        
                /* Free the UArray if it is not NULL */
                if (seg != NULL) {
                        free(seg);
                }
        }

        free(in_use);
        free(lengths);

        /* Free the sequence of unmapped IDs */
        if (unmapped != NULL) {
                Seq_free(&(unmapped));
        }

}