#ifndef IMPORT_H
#define IMPORT_H

#include <stdbool.h>

/**
 * @brief Esegue l'analisi delle dipendenze, la validazione e la modifica dei file.
 * 
 * Questa funzione orchestra l'intero processo:
 * 1. Raccoglie le definizioni di tutti i moduli dai file di input.
 * 2. Stampa un riepilogo dei moduli trovati.
 * 3. Per ogni file, verifica la compatibilità delle sue dipendenze.
 * 4. Per ogni file, esegue la pulizia delle clausole "-- WITH SUCCESSORS".
 * 5. Salva i file (modificati o meno) nella directory di output specificata.
 * 
 * @param num_input_files Il numero di file nell'array input_files.
 * @param input_files Un array di stringhe contenenti i percorsi ai file di input.
 * @param output_dir Il percorso della directory dove salvare i file processati.
 * @param processed_files Un puntatore a un array di stringhe che verrà popolato con i percorsi dei file processati.
 * @param processed_files_count Un puntatore a un intero che verrà popolato con il numero di file processati.
 */
void run_dependency_check_and_modify(int num_input_files, const char **input_files, const char *output_dir, char ***processed_files, int *processed_files_count);

#endif // IMPORT_H