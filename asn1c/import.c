#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include <ctype.h>
#include <glob.h>

#include "import.h" // include our new dependency management library

#define MAX_MODULES 100

// --- Globals for Conflict Reporting ---
static bool conflict_found = false;
static char first_conflicting_file[1024];
static char first_conflicting_module[256];
static char first_conflict_reason[256];
// ------------------------------------

typedef struct {
    char name[256];
    int major_version;
    int minor_version;
} ModuleInfo;

int match_and_capture_int(const char *text, const char *pattern) {
    regex_t regex;
    regmatch_t matches[2];
    int value = -1;
    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NEWLINE) != 0) return -1;
    if (regexec(&regex, text, 2, matches, 0) == 0) {
        char number_str[32];
        int len = matches[1].rm_eo - matches[1].rm_so;
        snprintf(number_str, sizeof(number_str), "%.*s", len, text + matches[1].rm_so);
        value = atoi(number_str);
    }
    regfree(&regex);
    return value;
}

ModuleInfo analyze_module_definition(const char* file_content) {
    ModuleInfo def_info = { .name = "N/A", .major_version = -1, .minor_version = -1 };
    const char *p = file_content;

    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (*p == '-' && *(p + 1) == '-') {
            p += 2;
            while (*p && *p != '\n') p++;
            continue;
        }
        if (*p == '/' && *(p + 1) == '*') {
            p += 2;
            while (*p && !(*p == '*' && *(p + 1) == '/')) p++;
            if (*p) p += 2;
            continue;
        }
        break;
    }

    const char *name_start = p;
    while (*p && !isspace((unsigned char)*p) && *p != '{') p++;
    const char *name_end = p;

    int name_len = name_end - name_start;
    if (name_len > 0) {
        snprintf(def_info.name, sizeof(def_info.name), "%.*s", name_len, name_start);
    } else {
        return def_info;
    }

    const char *oid_start = strchr(name_start, '{');
    const char *imports_block = strstr(name_start, "IMPORTS");
    if (oid_start && (!imports_block || oid_start < imports_block)) {
        def_info.major_version = match_and_capture_int(oid_start, "major-version-([0-9]+)");
        if (def_info.major_version != -1) {
            def_info.minor_version = match_and_capture_int(oid_start, "minor-version-([0-9]+)");
        } else {
            def_info.major_version = match_and_capture_int(oid_start, "version-([0-9]+)");
            if (def_info.major_version != -1) {
                def_info.minor_version = 0;
            } else {
                def_info.major_version = match_and_capture_int(oid_start, "version[[:space:]]*\\(([0-9]+)\\)");
                if (def_info.major_version != -1) {
                    def_info.minor_version = 0;
                }
            }
        }
    }
    return def_info;
}

void analyze_imports(const char *file_content) {
    const char *imports_start = strstr(file_content, "IMPORTS");
    if (!imports_start) return;
    const char *imports_end = strstr(imports_start, ";");
    if (!imports_end) return;

    printf("\n    [+] Import Analysis (Dependencies):\n");
    printf("    ---------------------------------------------------\n");
    printf("    | %-25s | Major | Minor |\n", "Module Name");
    printf("    ---------------------------------------------------\n");
    const char *current_pos = imports_start;
    while (current_pos < imports_end) {
        const char *from_keyword = strstr(current_pos, "FROM");
        if (!from_keyword || from_keyword > imports_end) break;
        const char *next_from = strstr(from_keyword + 4, "FROM");
        const char *clause_end = (next_from && next_from < imports_end) ? next_from : imports_end;
        ModuleInfo info = { .name = "N/A", .major_version = -1, .minor_version = -1 };
        const char *module_name_start = from_keyword + 4;
        while (*module_name_start && isspace((unsigned char)*module_name_start)) module_name_start++;
        const char *module_name_end = module_name_start;
        while (*module_name_end && !isspace((unsigned char)*module_name_end) && *module_name_end != '{') module_name_end++;
        snprintf(info.name, sizeof(info.name), "%.*s", (int)(module_name_end - module_name_start), module_name_start);
        const char *oid_start = strchr(from_keyword, '{');
        if (oid_start && oid_start < clause_end) {
            info.major_version = match_and_capture_int(oid_start, "major-version-([0-9]+)");
            if (info.major_version != -1) {
                info.minor_version = match_and_capture_int(oid_start, "minor-version-([0-9]+)");
            } else {
                info.major_version = match_and_capture_int(oid_start, "version-([0-9]+)");
                if (info.major_version != -1) {
                    info.minor_version = 0;
                } else {
                    info.major_version = match_and_capture_int(oid_start, "version[[:space:]]*\\(([0-9]+)\\)");
                    if (info.major_version != -1) {
                        info.minor_version = 0;
                    }
                }
            }
        }
        printf("    | %-25s | %-5d | %-5d |\n", info.name, info.major_version, info.minor_version);
        current_pos = clause_end;
    }
    printf("    ---------------------------------------------------\n");
}

void verify_imports(const char *current_filename, const char *file_content, const ModuleInfo *database, int db_count) {
    const char *imports_start = strstr(file_content, "IMPORTS");
    if (!imports_start) return;
    const char *imports_end = strstr(imports_start, ";");
    if (!imports_end) return;

    printf("\n    [+] Verifying Dependencies:\n");
    printf("    --------------------------------------------------------------------------------------------------------------------\n");
    printf("    | %-25s | %-8s | %-10s | %-10s | %-45s |\n", "Module Name", "Status", "Found", "Requested", "Details");
    printf("    --------------------------------------------------------------------------------------------------------------------\n");
    const char *current_pos = imports_start;
    while (current_pos < imports_end) {
        const char *from_keyword = strstr(current_pos, "FROM");
        if (!from_keyword || from_keyword > imports_end) break;
        const char *next_from = strstr(from_keyword + 4, "FROM");
        const char *clause_end = (next_from && next_from < imports_end) ? next_from : imports_end;
        ModuleInfo requested = { .name = "N/A", .major_version = -1, .minor_version = -1 };
        const char *module_name_start = from_keyword + 4;
        while (*module_name_start && isspace((unsigned char)*module_name_start)) module_name_start++;
        const char *module_name_end = module_name_start;
        while (*module_name_end && !isspace((unsigned char)*module_name_end) && *module_name_end != '{') module_name_end++;
        snprintf(requested.name, sizeof(requested.name), "%.*s", (int)(module_name_end - module_name_start), module_name_start);
        const char *oid_start = strchr(from_keyword, '{');
        if (oid_start && oid_start < clause_end) {
            requested.major_version = match_and_capture_int(oid_start, "major-version-([0-9]+)");
            if (requested.major_version != -1) {
                requested.minor_version = match_and_capture_int(oid_start, "minor-version-([0-9]+)");
            } else {
                requested.major_version = match_and_capture_int(oid_start, "version-([0-9]+)");
                if (requested.major_version != -1) {
                    requested.minor_version = 0;
                } else {
                    requested.major_version = match_and_capture_int(oid_start, "version[[:space:]]*\\(([0-9]+)\\)");
                    if (requested.major_version != -1) {
                        requested.minor_version = 0;
                    }
                }
            }
        }
        char requested_ver_str[20];
        snprintf(requested_ver_str, sizeof(requested_ver_str), "v%d.%d", requested.major_version, requested.minor_version);
        bool found_in_db = false;
        for (int i = 0; i < db_count; i++) {
            if (strcmp(requested.name, database[i].name) == 0) {
                found_in_db = true;
                const ModuleInfo *found = &database[i];
                char found_ver_str[20];
                snprintf(found_ver_str, sizeof(found_ver_str), "v%d.%d", found->major_version, found->minor_version);
                if (requested.major_version == found->major_version) {
                    if (found->minor_version >= requested.minor_version) {
                        printf("    | %-25s | %-8s | %-10s | %-10s | %-45s |\n", requested.name, "[OK]", found_ver_str, requested_ver_str, "Compatible version found");
                    } else {
                        printf("    | %-25s | %-8s | %-10s | %-10s | %-45s |\n", requested.name, "[ERROR]", found_ver_str, requested_ver_str, "Minor mismatch: found is older than requested");
                        if (!conflict_found) {
                            conflict_found = true;
                            snprintf(first_conflicting_file, sizeof(first_conflicting_file), "%s", current_filename);
                            snprintf(first_conflicting_module, sizeof(first_conflicting_module), "%s", requested.name);
                            snprintf(first_conflict_reason, sizeof(first_conflict_reason), "Minor mismatch (found %s, requested %s)", found_ver_str, requested_ver_str);
                        }
                    }
                } else {
                    printf("    | %-25s | %-8s | %-10s | %-10s | %-45s |\n", requested.name, "[ERROR]", found_ver_str, requested_ver_str, "Major version mismatch");
                    if (!conflict_found) {
                        conflict_found = true;
                        snprintf(first_conflicting_file, sizeof(first_conflicting_file), "%s", current_filename);
                        snprintf(first_conflicting_module, sizeof(first_conflicting_module), "%s", requested.name);
                        snprintf(first_conflict_reason, sizeof(first_conflict_reason), "Major version mismatch (found %s, requested %s)", found_ver_str, requested_ver_str);
                    }
                }
                break;
            }
        }
        if (!found_in_db) {
            printf("    | %-25s | %-8s | %-10s | %-10s | %-45s |\n", requested.name, "[ERROR]", "N/A", requested_ver_str, "Module not found in provided files");
            if (!conflict_found) {
                conflict_found = true;
                snprintf(first_conflicting_file, sizeof(first_conflicting_file), "%s", current_filename);
                snprintf(first_conflicting_module, sizeof(first_conflicting_module), "%s", requested.name);
                snprintf(first_conflict_reason, sizeof(first_conflict_reason), "Module not found in provided files");
            }
        }
        current_pos = clause_end;
    }
    printf("    --------------------------------------------------------------------------------------------------------------------\n");
}

bool generate_modified_content(const char *file_content, char **output_content) {
    const char *imports_start = strstr(file_content, "IMPORTS");
    if (!imports_start) { *output_content = strdup(file_content); return false; }
    const char *imports_end = strstr(imports_start, ";");
    if (!imports_end) { *output_content = strdup(file_content); return false; }
    char *result = (char *)malloc(strlen(file_content) + 1);
    if (!result) { *output_content = strdup(file_content); return false; }
    char *writer = result;
    bool modified = false;
    int prefix_len = imports_start - file_content;
    strncpy(writer, file_content, prefix_len);
    writer += prefix_len;
    const char *current_pos = imports_start;
    while (current_pos < imports_end) {
        const char *next_from = strstr(current_pos + 1, "FROM");
        const char *clause_end = (next_from && next_from < imports_end) ? next_from : imports_end;
        const char *successors = strstr(current_pos, "-- WITH SUCCESSORS");
        if (successors && successors < clause_end) {
            modified = true;
            const char *from_keyword = strstr(current_pos, "FROM");
            int clause_prefix_len = from_keyword - current_pos;
            strncpy(writer, current_pos, clause_prefix_len);
            writer += clause_prefix_len;
            const char *module_name_start = from_keyword + 4;
            while (*module_name_start && isspace((unsigned char)*module_name_start)) module_name_start++;
            const char *module_name_end = module_name_start;
            while (*module_name_end && !isspace((unsigned char)*module_name_end) && *module_name_end != '{') module_name_end++;
            int module_name_len = module_name_end - module_name_start;
            writer += sprintf(writer, "FROM %.*s", module_name_len, module_name_start);
            current_pos = successors + strlen("-- WITH SUCCESSORS");
        } else {
            int clause_len = clause_end - current_pos;
            strncpy(writer, current_pos, clause_len);
            writer += clause_len;
            current_pos = clause_end;
        }
    }
    *writer++ = ';';
    strcpy(writer, imports_end + 1);
    if (modified) { *output_content = result; } 
    else { free(result); *output_content = strdup(file_content); }
    return modified;
}

char* read_file_content(const char *filename) {
    FILE *in = fopen(filename, "r");
    if (!in) {
        perror("    [!] Error opening input file");
        return NULL;
    }
    fseek(in, 0, SEEK_END);
    long fsize = ftell(in);
    fseek(in, 0, SEEK_SET);
    char *content = (char *)malloc(fsize + 1);
    if (!content) {
        perror("    [!] Failed to allocate memory");
        fclose(in);
        return NULL;
    }
    fread(content, 1, fsize, in);
    fclose(in);
    content[fsize] = 0;
    return content;
}

void run_dependency_check_and_modify(int num_input_files, const char **input_files, const char *output_dir, char ***processed_files, int *processed_files_count) {
    ModuleInfo module_database[MAX_MODULES];
    char* file_contents[MAX_MODULES];
    char* original_filenames[MAX_MODULES];
    int db_count = 0;

    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    int flags = GLOB_TILDE | GLOB_NOCHECK;
    for (int i = 0; i < num_input_files; i++) {
        glob(input_files[i], flags, NULL, &glob_result);
        flags |= GLOB_APPEND;
    }

    printf("--- [DEPENDENCY CHECK] Step 1: Collecting module definitions ---\n");
    for (size_t i = 0; i < glob_result.gl_pathc && i < MAX_MODULES; i++) {
        file_contents[db_count] = read_file_content(glob_result.gl_pathv[i]);
        if (file_contents[db_count]) {
            original_filenames[db_count] = strdup(glob_result.gl_pathv[i]);
            module_database[db_count] = analyze_module_definition(file_contents[db_count]);
            db_count++;
        }
    }

    printf("\n[+] Found %d module definitions in provided files:\n", db_count);
    printf("---------------------------------------------------\n");
    printf("| %-25s | Major | Minor |\n", "Module Name");
    printf("---------------------------------------------------\n");
    for (int i = 0; i < db_count; i++) {
        printf("| %-25s | %-5d | %-5d |\n", module_database[i].name, module_database[i].major_version, module_database[i].minor_version);
    }
    printf("---------------------------------------------------\n");

    printf("\n--- [DEPENDENCY CHECK] Step 2: Verifying dependencies ---\n");
    for (int i = 0; i < db_count; i++) {
        printf("\n--- Verifying file: %s ---\n", original_filenames[i]);
        analyze_imports(file_contents[i]);
        verify_imports(original_filenames[i], file_contents[i], module_database, db_count);
    }

    if (conflict_found) {
        printf("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        printf("!!                      WARNING: CONFLICT DETECTED                      !!\n");
        printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
        printf("A dependency conflict was found. This may lead to compilation errors or unexpected behavior.\n\n");
        printf("  - File with conflict: %s\n", first_conflicting_file);
        printf("  - Conflicting module: %s\n", first_conflicting_module);
        printf("  - Reason: %s\n\n", first_conflict_reason);
        printf("Do you want to continue with the preprocessing anyway? (y/n): ");
        
        char answer = getchar();
        while (getchar() != '\n'); // Clear input buffer
        
        if (answer != 'y' && answer != 'Y') {
            printf("\nPreprocessing aborted by user.\n");
            exit(1);
        }
        printf("\nUser chose to continue despite warnings.\n");
    }

    *processed_files = malloc(db_count * sizeof(char*));
    *processed_files_count = db_count;

    printf("\n--- [DEPENDENCY CHECK] Step 3: Generating modified files ---\n");
    for (int i = 0; i < db_count; i++) {
        char *modified_content = NULL;
        bool modified = generate_modified_content(file_contents[i], &modified_content);
        
        const char *base_name = strrchr(original_filenames[i], '/');
        if (base_name) base_name++;
        else base_name = original_filenames[i];
        
        char output_path[1024];
        snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, base_name);
        
        (*processed_files)[i] = strdup(output_path);

        FILE *out = fopen(output_path, "w");
        if (!out) {
            perror("    [!] Error opening output file for dependency check");
        } else {
            fwrite(modified_content, 1, strlen(modified_content), out);
            fclose(out);
            if(modified) {
                printf("    [+] Saved simplified version of %s to: %s\n", base_name, output_path);
            } else {
                printf("    [i] Copied %s as is to: %s\n", base_name, output_path);
            }
        }
        free(modified_content);
    }

    for (int i = 0; i < db_count; i++) {
        free(file_contents[i]);
        free(original_filenames[i]);
    }
    globfree(&glob_result);
}