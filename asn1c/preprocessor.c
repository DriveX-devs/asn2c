#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <stdbool.h> // Inclusione per usare bool, true, false in C

#define MAX_LINE 1024
#define MAX_ENTRIES 64

typedef struct {
    char scoped_type[1024];
    char inner_type[1024];
    char component[1024];
    char alias[1024];
} ScopedEntry;

ScopedEntry entries[MAX_ENTRIES];
int entry_count = 0;

// Funzione per eseguire una regex su una riga
int match_line(const char *line, const char *pattern, regmatch_t *matches, int nmatch) {
    regex_t regex;
    int result;

    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE) != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }
    result = regexec(&regex, line, nmatch, matches, 0);
    regfree(&regex);
    return result == 0;
}

// Capitalizza la prima lettera e aggiunge un prefisso generico
void generate_alias(const char *component, char *alias, size_t alias_size) {
    snprintf(alias, alias_size, "Alias-%c%s", toupper(component[0]), component + 1);
}

// Processa il file e restituisce true se è stato modificato
bool process_file(const char *filename, const char *output_dir, bool debug_mode) {
    // Resetta lo stato globale per ogni file processato
    entry_count = 0;

    FILE *in = fopen(filename, "r");
    if (!in) {
        perror("Error opening input file");
        return false;
    }

    // Leggi l'intero file in un buffer
    fseek(in, 0, SEEK_END);
    long fsize = ftell(in);
    fseek(in, 0, SEEK_SET);
    char *content = (char *)malloc(fsize + 1);
    if (!content) {
        perror("Failed to allocate memory for file content");
        fclose(in);
        return false;
    }
    fread(content, 1, fsize, in);
    fclose(in);
    content[fsize] = 0;

    char *search_start_pos = content;
    const char *target_block_start_ptr = NULL;
    const char *target_block_end_ptr = NULL;
    regmatch_t block_match[3];
    char main_type_name[128] = {0};
    char parent_type_name[128] = {0};
    bool transform_needed = false;

    // --- PRIMO PASSAGGIO: Cerca il blocco corretto da trasformare ---
    printf("[INFO] Pass 1: Scanning for a CHOICE block with WITH COMPONENTS...\n");
    while(true) {
        const char *block_start_pattern = "([A-Za-z0-9-]+)[[:space:]]*::=[[:space:]]*([A-Za-z0-9-]+)[[:space:]]*\\(";
        if (!match_line(search_start_pos, block_start_pattern, block_match, 3)) {
            break; // Nessun altro blocco trovato
        }

        // Trovato un potenziale blocco, determiniamo i suoi limiti
        const char *block_content_start = search_start_pos + block_match[0].rm_eo;
        int open_parens = 1;
        const char *block_content_end = block_content_start;
        while (*block_content_end && open_parens > 0) {
            if (*block_content_end == '(') open_parens++;
            if (*block_content_end == ')') open_parens--;
            block_content_end++;
        }

        if (open_parens != 0) {
            fprintf(stderr, "Warning: Unbalanced parentheses found, skipping block.\n");
            search_start_pos += block_match[0].rm_eo; // Avanza e continua la ricerca
            continue;
        }

        // Controlla se il contenuto del blocco è quello che vogliamo
        size_t block_len = block_content_end - block_content_start;
        // Sostituzione di strndup con malloc + strncpy per compatibilità C standard
        char *block_content_str = (char *)malloc(block_len + 1);
        if (!block_content_str) {
            perror("Failed to allocate memory for block content");
            break;
        }
        strncpy(block_content_str, block_content_start, block_len);
        block_content_str[block_len] = '\0';
        
        if (strstr(block_content_str, "WITH COMPONENTS") && strchr(block_content_str, '|')) {
            printf("  [OK] Found target block to transform.\n");
            // Trovato! Salviamo le informazioni ed usciamo dal ciclo di ricerca.
            snprintf(main_type_name, sizeof(main_type_name), "%.*s", (int)(block_match[1].rm_eo - block_match[1].rm_so), search_start_pos + block_match[1].rm_so);
            snprintf(parent_type_name, sizeof(parent_type_name), "%.*s", (int)(block_match[2].rm_eo - block_match[2].rm_so), search_start_pos + block_match[2].rm_so);
            target_block_start_ptr = search_start_pos + block_match[0].rm_so;
            target_block_end_ptr = block_content_end;
            
            // --- Estrai i componenti dal blocco trovato ---
            const char *component_search_ptr = target_block_start_ptr + (block_match[0].rm_eo - block_match[0].rm_so);
            const char *pattern = "([A-Za-z0-9-]+)[[:space:]]*\\{[[:space:]]*([A-Za-z0-9-]+)[[:space:]]*\\(WITH COMPONENTS[[:space:]]*\\{[[:space:]]*([A-Za-z0-9-]+)[[:space:]]*\\}\\)[[:space:]]*\\}";
            regmatch_t m[4];

            while (component_search_ptr < target_block_end_ptr && match_line(component_search_ptr, pattern, m, 4)) {
                if (entry_count >= MAX_ENTRIES) {
                    fprintf(stderr, "Error: Too many entries found.\n");
                    break;
                }
                ScopedEntry *e = &entries[entry_count++];
                snprintf(e->scoped_type, sizeof(e->scoped_type), "%.*s", (int)(m[1].rm_eo - m[1].rm_so), component_search_ptr + m[1].rm_so);
                snprintf(e->inner_type, sizeof(e->inner_type), "%.*s", (int)(m[2].rm_eo - m[2].rm_so), component_search_ptr + m[2].rm_so);
                snprintf(e->component, sizeof(e->component), "%.*s", (int)(m[3].rm_eo - m[3].rm_so), component_search_ptr + m[3].rm_so);
                generate_alias(e->component, e->alias, sizeof(e->alias));
                printf("    Found component: %s (%s) -> %s\n", e->component, e->scoped_type, e->alias);
                component_search_ptr += m[0].rm_eo;
            }
            
            if (entry_count > 0) {
                transform_needed = true;
            } else {
                printf("[WARN] Target block was found, but no component entries inside. File will be copied as is.\n");
            }
            
            free(block_content_str);
            break; 
        }
        
        free(block_content_str);
        // Non è il blocco giusto, continuiamo la ricerca da dopo questo
        search_start_pos += block_match[0].rm_eo;
    }

    if (!transform_needed) {
        printf("[INFO] No transformation needed. File will be copied as is.\n");
    }

    // --- DEBUG MODE CHECK ---
    if (debug_mode && transform_needed) {
        printf("\n--- DEBUG MODE: ANALYSIS COMPLETE ---\n");
        printf("File: %s\n", filename);
        printf("Found block to transform: '%s'\n", main_type_name);
        printf("Found %d components to create aliases for:\n", entry_count);
        for (int i = 0; i < entry_count; i++) {
            printf("  - Component: '%s' -> Alias: '%s'\n", entries[i].component, entries[i].alias);
        }
        printf("\nContinue with writing the file? (y/N): ");
        
        int response = getchar();
        // Consuma il resto della riga (es. il carattere newline)
        while(getchar() != '\n' && getchar() != EOF);

        if (response != 'y' && response != 'Y') {
            printf("--- ABORTED BY USER. NO FILE WILL BE WRITTEN. ---\n");
            free(content);
            return false; // Interrompe l'esecuzione, file non modificato
        }
        printf("--- CONTINUING. ---\n\n");
    }

    // --- SECONDO PASSAGGIO: Scrivere il nuovo file ---
    const char *base_name = strrchr(filename, '/');
    if (base_name) {
        base_name++;
    } else {
        base_name = filename;
    }

    char output_path[1024];
    snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, base_name);

    FILE *out = fopen(output_path, "w");
    if (!out) {
        perror("Error opening output file");
        free(content);
        return false;
    }

    if (transform_needed) {
        printf("[INFO] Pass 2: Writing TRANSFORMED file to %s\n", output_path);
        // Scrivi la parte del file prima del blocco
        fwrite(content, 1, target_block_start_ptr - content, out);

        // Scrivi i nuovi alias generati
        fprintf(out, "-- Scoped type aliases, generated automatically\n");
        for (int i = 0; i < entry_count; i++) {
            fprintf(out, "%s ::= %s {\n", entries[i].alias, entries[i].scoped_type);
            fprintf(out, "  %s (WITH COMPONENTS {\n", entries[i].inner_type);
            fprintf(out, "    %s\n", entries[i].component);
            fprintf(out, "  })\n");
            fprintf(out, "}\n\n");
        }

        // E la nuova definizione principale
        fprintf(out, "-- Restructured main type\n");
        fprintf(out, "%s ::= %s (\n", main_type_name, parent_type_name);
        for (int i = 0; i < entry_count; i++) {
            fprintf(out, "  %s%s\n", entries[i].alias, (i < entry_count - 1) ? " |" : "");
        }
        fprintf(out, ")\n");

        // Scrivi la parte del file dopo il blocco
        fwrite(target_block_end_ptr, 1, strlen(target_block_end_ptr), out);
        printf("[INFO] Transformation complete. Output written to %s\n", output_path);
    } else {
        printf("[INFO] Pass 2: Writing UNMODIFIED file to %s\n", output_path);
        fwrite(content, 1, fsize, out);
        printf("[INFO] File copy complete. Output written to %s\n", output_path);
    }

    fclose(out);
    free(content);
    return transform_needed;
}

/**
 * @brief Esegue la pre-elaborazione su una lista di file ASN.1.
 * 
 * Questa funzione itera su una lista di file di input, li processa
 * (modificandoli o copiandoli) in una directory di output e stampa
 * un riepilogo finale.
 * 
 * @param num_input_files Il numero di file nell'array input_files.
 * @param input_files Un array di stringhe contenenti i percorsi ai file di input.
 * @param output_dir Il percorso della directory dove salvare i file processati.
 * @param debug_mode Se true, attiva la modalità di debug con output verboso.
 * @return Il numero di file che sono stati effettivamente modificati.
 */
int run_preprocessor(int num_input_files, const char **input_files, const char *output_dir, bool debug_mode) {
    int modified_files_count = 0;

    if (num_input_files == 0) {
        fprintf(stderr, "[PREPROCESSOR] Error: No input files provided.\n");
        return 0;
    }

    // Processa ogni file di input
    for (int i = 0; i < num_input_files; i++) {
        printf("--- Preprocessing file: %s ---\n", input_files[i]);
        // Controlla il valore di ritorno e incrementa il contatore se true
        if (process_file(input_files[i], output_dir, debug_mode)) {
            modified_files_count++;
        }
        printf("--- Finished preprocessing: %s ---\n\n", input_files[i]);
    }

    // Stampa il riepilogo finale
    printf("--- PREPROCESSOR SUMMARY ---\n");
    printf("Total files analyzed: %d\n", num_input_files);
    printf("Total files modified: %d\n", modified_files_count);
    printf("--------------------------\n\n");

    return modified_files_count;
}