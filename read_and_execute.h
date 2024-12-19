/**************************************************************
 *
 *                     read_and_execute.h
 *
 *     Assignment: HW 6: um
 *        Authors: Dan Glorioso & Brandon Dionisio (dglori02 & bdioni01)
 *           Date: 04/11/24
 *
 *     Summary: Function declarations for processes that read in and execute
 *              the instructions from the program file passed in as an argument
 *              to the Universal Machine. 
 * 
 **************************************************************/


#ifndef READ_AND_EXECUTE_H
#define READ_AND_EXECUTE_H

#include <stdint.h>
#include <stdbool.h>
#include <seq.h>

/*****************************************************************
 *                  Program Function Declarations
 *****************************************************************/
// extern void um_driver(FILE *fp, size_t num_inst);
extern void um_driver(FILE *fp, size_t num_inst);
extern void read_instructions(FILE *fp, size_t num_inst, uint32_t *zero_seg);
extern void execute_instructions(Seq_T in_use, Seq_T unmapped, Seq_T lengths, size_t num_inst,
                                     uint32_t *zero_seg);

#endif