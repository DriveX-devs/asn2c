#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
#include <stdbool.h>
#include <glob.h>

#include "import.h" // Includiamo la nostra nuova libreria

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

void generate_alias(const char *component, char *alias, size_t alias_size) {
    snprintf(alias, alias_size, "Alias%c%s", toupper(component[0]), component + 1);
}

// NOTA: La funzione preprocess_imports non è più necessaria qui,
// perché la sua logica è ora gestita da generate_modified_content in Testimport.c

bool process_file_for_choice(const char *filename, const char *output_dir, bool debug_mode) {
    entry_count = 0;

    FILE *in = fopen(filename, "r");
    if (!in) {
        perror("Error opening input file for CHOICE processing");
        return false;
    }

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

    printf("[CHOICE] Pass 1: Scanning for a CHOICE block with WITH COMPONENTS...\n");
    while(true) {
        const char *block_start_pattern = "([A-Za-z0-9-]+)[[:space:]]*::=[[:space:]]*([A-Za-z0-9-]+)[[:space:]]*\\(";
        if (!match_line(search_start_pos, block_start_pattern, block_match, 3)) {
            break;
        }

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
            search_start_pos += block_match[0].rm_eo;
            continue;
        }

        size_t block_len = block_content_end - block_content_start;
        char *block_content_str = (char *)malloc(block_len + 1);
        if (!block_content_str) {
            perror("Failed to allocate memory for block content");
            break;
        }
        strncpy(block_content_str, block_content_start, block_len);
        block_content_str[block_len] = '\0';
        
        if (strstr(block_content_str, "WITH COMPONENTS") && strchr(block_content_str, '|')) {
            printf("  [OK] Found target block to transform.\n");
            snprintf(main_type_name, sizeof(main_type_name), "%.*s", (int)(block_match[1].rm_eo - block_match[1].rm_so), search_start_pos + block_match[1].rm_so);
            snprintf(parent_type_name, sizeof(parent_type_name), "%.*s", (int)(block_match[2].rm_eo - block_match[2].rm_so), search_start_pos + block_match[2].rm_so);
            target_block_start_ptr = search_start_pos + block_match[0].rm_so;
            target_block_end_ptr = block_content_end;
            
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
            
            if (entry_count > 0) transform_needed = true;
            else printf("[WARN] Target block was found, but no component entries inside. File will be copied as is.\n");
            
            free(block_content_str);
            break; 
        }
        
        free(block_content_str);
        search_start_pos += block_match[0].rm_eo;
    }

    if (!transform_needed) {
        printf("[INFO] No CHOICE transformation needed for this file.\n");
        free(content);
        return false; // Nessuna modifica per il CHOICE
    }

    // --- SECONDO PASSAGGIO: Scrivere il nuovo file ---
    const char *base_name = strrchr(filename, '/');
    if (base_name) base_name++;
    else base_name = filename;

    char output_path[1024];
    snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, base_name);

    FILE *out = fopen(output_path, "w");
    if (!out) {
        perror("Error opening output file for CHOICE transformation");
        free(content);
        return false;
    }

    printf("[INFO] Pass 2: Writing file transformed for CHOICE with WITH COMPONENTS.\n");
    fwrite(content, 1, target_block_start_ptr - content, out);

    fprintf(out, "-- Scoped type aliases, generated automatically\n");
    for (int i = 0; i < entry_count; i++) {
        fprintf(out, "%s ::= %s {\n", entries[i].alias, entries[i].scoped_type);
        fprintf(out, "  %s (WITH COMPONENTS {\n", entries[i].inner_type);
        fprintf(out, "    %s\n", entries[i].component);
        fprintf(out, "  })\n");
        fprintf(out, "}\n\n");
    }

    fprintf(out, "-- Restructured main type\n");
    fprintf(out, "%s ::= %s (\n", main_type_name, parent_type_name);
    for (int i = 0; i < entry_count; i++) {
        fprintf(out, "  %s%s\n", entries[i].alias, (i < entry_count - 1) ? " |" : "");
    }
    fprintf(out, ")\n");

    fwrite(target_block_end_ptr, 1, strlen(target_block_end_ptr), out);
    printf("[INFO] Transformation complete. Output written to %s\n", output_path);

    fclose(out);
    free(content);
    return true;
}

int run_preprocessor(int num_input_files, const char **input_files, const char *output_dir, bool debug_mode) {
    int modified_files_count = 0;
    char **processed_files = NULL;
    int processed_files_count = 0;

    if (num_input_files == 0) {
        fprintf(stderr, "[PREPROCESSOR] Error: No input files provided.\n");
        return 0;
    }

    // --- FASE 1: Esegui il controllo delle dipendenze e la pulizia degli import ---
    // Questa funzione ora legge i file originali, esegue l'analisi, scrive
    // i file nella directory di output e popola l'array processed_files.
    run_dependency_check_and_modify(num_input_files, input_files, output_dir, &processed_files, &processed_files_count);

    // --- FASE 2: Esegui le trasformazioni del CHOICE sui file appena generati ---
    printf("\n--- [CHOICE TRANSFORM] Starting second preprocessing phase on generated files ---\n");
    
    for (int i = 0; i < processed_files_count; i++) {
        printf("\n--- Processing for CHOICE: %s ---\n", processed_files[i]);
        if (process_file_for_choice(processed_files[i], output_dir, debug_mode)) {
            modified_files_count++;
        }
        printf("--- Finished CHOICE processing: %s ---\n", processed_files[i]);
    }
    
    // Stampa il riepilogo finale
    printf("\n--- PREPROCESSOR SUMMARY ---\n");
    printf("Total files analyzed and processed: %d\n", processed_files_count);
    printf("Total files modified by CHOICE transform: %d\n", modified_files_count);
    printf("--------------------------\n\n");

    // Cleanup
    for (int i = 0; i < processed_files_count; i++) {
        free(processed_files[i]);
    }
    free(processed_files);

    return modified_files_count;
}