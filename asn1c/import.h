#ifndef IMPORT_H
#define IMPORT_H

#include <stdbool.h>

/**
 * @brief Performs dependency analysis, validation, and file modification.
 * 
 * This function orchestrates the entire process:
 * 1. Collects module definitions from all input files.
 * 2. Prints a summary of the found modules.
 * 3. For each file, verifies the compatibility of its dependencies.
 * 4. For each file, cleans up "-- WITH SUCCESSORS" clauses.
 * 5. Saves the files (modified or not) to the specified output directory.
 * 
 * @param num_input_files The number of files in the input_files array.
 * @param input_files An array of strings containing the paths to the input files.
 * @param output_dir The path to the directory where processed files will be saved.
 * @param processed_files A pointer to an array of strings that will be populated with the paths of the processed files.
 * @param processed_files_count A pointer to an integer that will be populated with the number of processed files.
 */
void run_dependency_check_and_modify(int num_input_files, const char **input_files, const char *output_dir, char ***processed_files, int *processed_files_count);

#endif // IMPORT_H