#!/bin/bash

# List of files to process
files=(
    "halt.um"
    "halt-verbose.um"
    "output_test.um"
    "output_load_test.um"
    "print-six.um"
    "add_non_adjacent_test.um"
    "in_and_out_test.um"
    "add_input_test.um"
    "multiply_test.um"
    "multiply_test_input.um"
    "multiply_by_zero_test.um"
    "multiply_by_one_test.um"
    "multiply_by_self_test.um"
    "divide_test.um"
    "divide_by_one_test.um"
    "divide_by_self_test.um"
    "divide_by_input_test.um"
    "divide_by_self_input_test.um"
    "nand_input_test.um"
    "cmov_test_0.um"
    "cmov_test_1.um"
    "map_test.um"
    "unmap_test_1.um"
    "unmap_test_2.um"
    "segment_store_test.um"
    "segment_sl_test.um"
    "load_test_not_0.um"
    "load_test_0.um"
)

# Iterate through each file and run the `./um` executable
for file in "${files[@]}"; do
    # Extract the base file name without the extension
    base_name="${file%.um}"

    # Check if the base name contains "input" or is "in_and_out_test"
    if [[ $base_name == *"input"* || $base_name == "in_and_out_test" ]]; then
        # File name ends with .0 for input if it contains "input" or is "in_and_out_test"
        input_file="${base_name}.0"
        # Run the `./um` executable with the current file and input file, and save the output to a .out file
        um "$file" < "$input_file" > "${base_name}.out"
        echo "Processed $file with input from $input_file and saved output to ${base_name}.out"
    else
        # Run the `./um` executable with the current file and save the output to a .out file
        um "$file" > "${base_name}.out"
        echo "Processed $file and saved output to ${base_name}.out"
    fi

    # Perform a diff test if a .1 file exists for the current test
    expected_output_file="${base_name}.1"
    if [[ -f "$expected_output_file" ]]; then
        echo "Comparing ${base_name}.out with ${expected_output_file} using diff..."
        diff "${base_name}.out" "$expected_output_file"
        # Check the exit status of diff
        if [[ $? -ne 0 ]]; then
            # Raise an obvious error if the files are not identical
            echo "Error: ${base_name}.out and ${expected_output_file} are not identical!"
            exit 1 # You can exit the script with a specific status code (e.g., 1)
        fi
    fi
done
