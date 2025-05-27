#include "asn1c_internal.h"
#include "asn1c_constraint.h"
#include "asn1c_misc.h"
#include "asn1c_out.h"
#include "asn1c_naming.h"
#include "asn1print.h"
#include <asn1fix_crange.h>	/* constraint groker from libasn1fix */
#include <asn1fix_export.h>	/* other exportables from libasn1fix */

static int asn1c_emit_constraint_tables(arg_t *arg, int got_size);
static int emit_alphabet_check_loop(arg_t *arg, asn1cnst_range_t *range);
static int emit_value_determination_code(arg_t *arg, asn1p_expr_type_e etype, asn1cnst_range_t *r_value);
static int emit_size_determination_code(arg_t *arg, asn1p_expr_type_e etype);
static asn1p_expr_type_e _find_terminal_type(arg_t *arg);
static abuf *emit_range_comparison_code(asn1cnst_range_t *range,
                                          const char *varname,
                                          asn1c_integer_t natural_start,
                                          asn1c_integer_t natural_stop);
static int native_long_sign(arg_t *arg, asn1cnst_range_t *r);	/* -1, 0, 1 */
static void emit_component_constraint_checks(arg_t *arg, asn1p_constraint_t *comp_ct, char *component_name);
static int
ulong_optimization(arg_t *arg, asn1p_expr_type_e etype, asn1cnst_range_t *r_size,
						asn1cnst_range_t *r_value)
{
	return (!r_size && r_value
		&& (etype == ASN_BASIC_INTEGER
		|| etype == ASN_BASIC_ENUMERATED)
		&& native_long_sign(arg, r_value) == 0);
}


char *escape_for_c_string(const char *input) {
    if (input == NULL) return NULL;

    size_t len = strlen(input);
    // Worst case: ogni carattere va escapato → 2x spazio
    char *escaped = malloc(len * 2 + 1);
    if (!escaped) return NULL;

    char *dst = escaped;
    for (const char *src = input; *src; ++src) {
        if (*src == '\\') {
            *dst++ = '\\';
            *dst++ = '\\';
        } else if (*src == '"') {
            *dst++ = '\\';
            *dst++ = '"';
        } else {
            *dst++ = *src;
        }
    }
    *dst = '\0';
    return escaped;
}


static void
emit_pattern_constraint(arg_t *arg, asn1p_constraint_t *ct, int i) {
    //OUT("printf(\"Sono dentro\");\n");

    // if(ct->elements[i]->value->value.string.buf != NULL) {
    //     //Possibile warning qua per cast non esplicito
    //     OUT("const char *c_string = strndup((const char *)st->buf, st->size);\n");
    //     const char *pattern = ct->elements[i]->value->value.string.buf;
    //     OUT("const char *string_pattern =  \"%s\";\n", pattern);
    //     //Probabile errore per le ""
    //
    //     OUT("regex_t regex;\n");
    //     OUT("int ret = regcomp(&regex, string_pattern , REG_EXTENDED);\n");
    //     OUT("if (ret) {\n");
    //     OUT("    return -1;\n");
    //     OUT("}\n");
    //     OUT("\n");
    //     OUT("ret = regexec(&regex, c_string, 0, NULL, 0);\n");
    //     OUT("regfree(&regex);\n");
    //     OUT("if (ret) return -1;\n");
    // }
    if (ct->elements[i]->value->value.string.buf != NULL) {
        OUT("// --- PCRE2 MATCH START ---\n");

        // Cast esplicito e copia della stringa
        OUT("const char *c_string = strndup((const char *)st->buf, st->size);\n");

        // Inserisci il pattern direttamente come stringa C corretta
        const char *pattern = (const char *)ct->elements[i]->value->value.string.buf;
        char *escaped_pattern = escape_for_c_string(pattern);
        OUT("PCRE2_SPTR pattern = (PCRE2_SPTR)\"%s\";\n", escaped_pattern);
        OUT("PCRE2_SPTR subject = (PCRE2_SPTR)c_string;\n");

        OUT("int errorcode;\n");
        OUT("PCRE2_SIZE erroffset;\n");
        OUT("pcre2_code *re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);\n");
        OUT("if (!re) {\n");
        OUT("    PCRE2_UCHAR buffer[256];\n");
        OUT("    pcre2_get_error_message(errorcode, buffer, sizeof(buffer));\n");
        OUT("    fprintf(stderr, \"Regex compilation error: %%s\\n\", buffer);\n");
        OUT("    return -1;\n");
        OUT("}\n");

        OUT("pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(re, NULL);\n");

        OUT("int rc = pcre2_match(re, subject, strlen((char *)subject), 0, 0, match_data, NULL);\n");
        OUT("pcre2_match_data_free(match_data);\n");
        OUT("pcre2_code_free(re);\n");

        OUT("if (rc < 0) return -1;\n");

        OUT("// --- PCRE2 MATCH END ---\n");
    }

}

static void
emit_pattern_constraint_union(arg_t *arg, asn1p_constraint_t *ct, int i, int j ,int first_pattern) {
    //OUT("printf(\"Sono dentro\");\n");

    // if(ct->elements[i]->elements[j]->value->value.string.buf != NULL) {
    //     //Possibile warning qua per cast non esplicito
    //     if (first_pattern == 0) {
    //         OUT("const char *c_string = strndup((const char *)st->buf, st->size);\n");
    //     }
    //
    //     const char *pattern = ct->elements[i]->elements[j]->value->value.string.buf;
    //     if (first_pattern == 0) {
    //         OUT("char *string_pattern =  \"%s\";\n", pattern);
    //         OUT("regex_t regex;\n");
    //         OUT("int ret = regcomp(&regex, string_pattern , REG_EXTENDED);\n");
    //     }else {
    //         OUT("string_pattern =  \"%s\";\n", pattern);
    //         OUT("ret = regcomp(&regex, string_pattern , REG_EXTENDED);\n");
    //     }
    //
    //     //Probabile errore per le ""
    //
    //
    //
    //     OUT("if (ret==0) {\n");
    //     OUT("    union_contains = 1;\n");
    //     OUT("}\n");
    //     OUT("\n");
    //     OUT("ret = regexec(&regex, c_string, 0, NULL, 0);\n");
    //     OUT("regfree(&regex);\n");
    //     OUT("if (ret==0) union_contains = 1;\n");
    // }
    if(ct->elements[i]->elements[j]->value->value.string.buf != NULL) {
        // Possibile warning qua per cast non esplicito
        if (first_pattern == 0) {
            OUT("const char *c_string = strndup((const char *)st->buf, st->size);\n");
            OUT("PCRE2_SPTR subject = (PCRE2_SPTR)c_string;\n");
            OUT("int errorcode;\n");
            OUT("PCRE2_SIZE erroffset;\n");
            OUT("pcre2_code *re;\n");
            OUT("pcre2_match_data *match_data;\n");
        }

        const char *pattern = ct->elements[i]->elements[j]->value->value.string.buf;
        char *escaped_pattern = escape_for_c_string(pattern);
        if (first_pattern == 0) {
            OUT("PCRE2_SPTR pattern = (PCRE2_SPTR)\"%s\";\n", escaped_pattern);
            OUT("re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);\n");
        } else {
            OUT("pattern = (PCRE2_SPTR)\"%s\";\n", escaped_pattern);
            OUT("re = pcre2_compile(pattern, PCRE2_ZERO_TERMINATED, 0, &errorcode, &erroffset, NULL);\n");
        }

        OUT("if (re) {\n");
        OUT("    match_data = pcre2_match_data_create_from_pattern(re, NULL);\n");
        OUT("    int rc = pcre2_match(re, subject, strlen((char *)subject), 0, 0, match_data, NULL);\n");
        OUT("    pcre2_match_data_free(match_data);\n");
        OUT("    pcre2_code_free(re);\n");
        OUT("    if (rc >= 0) union_contains = 1;\n");
        OUT("}\n");
    }

}


// static void
// emit_single_value_string_constraint(arg_t *arg, asn1p_constraint_t *ct, int i) {
//     if(ct->elements[i]->value->value.string.buf != NULL) {
//         //Possibile warning qua per cast non esplicito
//
//         OUT("const char *c_string = strndup((const char *)st->buf, st->size);\n");
//         const char *single_value = ct->elements[i]->value->value.string.buf;
//         OUT("const char *single_value =  \"%s\";\n", single_value);
//
//         OUT("if (strcmp(c_string, single_value) != 0) {\n");
//         INDENT(+1);
//         OUT("\t return -1;\n");
//         OUT("}\n");
//         INDENT(-1);
//     }
// }


static void
emit_single_value_string_constraint(arg_t *arg, asn1p_constraint_t *ct, int i) {
    if(ct->elements[i]->value->value.string.buf != NULL) {
        const char *raw_constraint_value = (const char *)ct->elements[i]->value->value.string.buf;
        char *escaped_constraint_value = escape_for_c_string(raw_constraint_value);
        // La free di escaped_constraint_value (variabile C di questa funzione)
        // deve essere fatta alla fine di questa funzione.

        // Nome del campo per i messaggi di errore (se disponibile)
        const char *field_name_for_error = (arg->expr && arg->expr->Identifier) ? arg->expr->Identifier : "field";

        // Genera codice C per la validazione
        OUT("            char *actual_runtime_value = strndup((const char *)st->buf, st->size);\n");
        OUT("            if(!actual_runtime_value) {\n");
        INDENT(+1);
        OUT("                ASN__CTFAIL(app_key, td, sptr, \"%%%%s: strndup failed for component '%s' (%%%%s:%%%%d)\", td->name, __FILE__, __LINE__);\n", field_name_for_error);

        OUT("                return -1;\n");
        INDENT(-1);
        OUT("            }\n");

        if (escaped_constraint_value) {
            OUT("            const char *expected_constraint_literal = \"%s\";\n", escaped_constraint_value);
            OUT("            if (strcmp(actual_runtime_value, expected_constraint_literal) != 0) {\n");
            INDENT(+1);
            OUT("                ASN__CTFAIL(app_key, td, sptr, \"%%%%s: component '%s' value ('%%s') does not match constraint '%%s' (%%%%s:%%%%d)\",\n", field_name_for_error);
            OUT("                    td->name, actual_runtime_value, expected_constraint_literal, __FILE__, __LINE__);\n");
            OUT("                free(actual_runtime_value);\n");
            OUT("                return -1;\n");
            INDENT(-1);
            OUT("            }\n");
            OUT("            free(actual_runtime_value);\n");
        } else {
            // Errore durante l'escape del valore del vincolo. Questo è un problema di asn1c.
            // Genera codice per liberare actual_runtime_value e fallire.
            OUT("            ASN__CTFAIL(app_key, td, sptr, \"%%%%s: internal error escaping constraint value for component '%s' (%%%%s:%%%%d)\", td->name, __FILE__, __LINE__);\n", field_name_for_error);
            OUT("            free(actual_runtime_value);\n");
            OUT("            return -1;\n");
        }

        // Libera la memoria allocata da escape_for_c_string in questa funzione C (emit_single_value_string_constraint)
        if(escaped_constraint_value) {
            free(escaped_constraint_value);
        }
    }
}

static void
emit_single_value_string_constraint_union(arg_t *arg, asn1p_constraint_t *ct, int i, int j, int first_string) {
    if(ct->elements[i]->elements[j]->value->value.string.buf != NULL) {
        //Possibile warning qua per cast non esplicito
        if (first_string == 0) {
            OUT("const char *c_string = strndup((const char *)st->buf, st->size);\n");
        }
        char *single_value = ct->elements[i]->elements[j]->value->value.string.buf;
        if (first_string == 0) {
            OUT("char *single_value =  \"%s\";\n", single_value);
        }else {
            OUT(" single_value =  \"%s\";\n", single_value);
        }

        OUT("if (strcmp(c_string, single_value) == 0) {\n");
        INDENT(+1);
        OUT("\t union_contains = 1;\n");
        OUT("}\n");
        INDENT(-1);

    }
}

static void
emit_regex_include(arg_t *arg) {
    int saved_target = arg->target->target;
    printf("Debug: saved_target = %d, OT_INCLUDES = %d\n", saved_target, OT_INCLUDES);
    REDIR(8);
    //OUT("#include <regex.h>\n");
    OUT("#define PCRE2_CODE_UNIT_WIDTH 8\n");
    OUT("#include <pcre2.h>\n");
    REDIR(saved_target);
    printf("Include regex\n");

}

static void
emit_component_constraint_checks(arg_t *arg, asn1p_constraint_t *comp_ct, char *component_name) {
    // Cerca l'espressione del componente nella definizione del tipo
    asn1p_expr_t *comp_expr = NULL;
    arg_t comp_arg = *arg;  // Clona l'argomento attuale

    // Cerca il componente nella definizione del tipo corrente
    if (1/*arg->expr->expr_type == ASN_CONSTR_SEQUENCE ||
        arg->expr->expr_type == ASN_CONSTR_SET ||
        arg->expr->expr_type == ASN_CONSTR_CHOICE*/) {
        asn1p_expr_t *child_expr;
        TQ_FOR(child_expr, &arg->asn->modules.tq_head->members.tq_head->members, next) {
            int component_exists = child_expr->Identifier && strcmp(child_expr->Identifier, component_name) == 0;
            if (component_exists){
                comp_expr = child_expr;
                break;
            }
        }
    }

    if (!comp_expr) {
        OUT("    /* Componente '%s' non trovato nella definizione */\n", component_name);
        return;
    }

    comp_arg.expr = comp_expr; // Imposta l'espressione per il componente specifico
    OUT("    /* Controlli specifici per il componente %s */\n", component_name);

    // Itera attraverso i vincoli specifici di questo componente
    for (int k = 0; k < comp_ct->el_count; k++) {
        asn1p_constraint_t *constraint = comp_ct->elements[k];

        // L'errore era qui: comp_ct->type è un enum, non un puntatore a enum
        // Eseguiamo lo switch su constraint->type (non su comp_ct->type)

        // La variabile 'constraint' (che è comp_ct->elements[k]) è già definita dal ciclo for esterno.

    if (constraint->type == ACT_CA_SET) {
        OUT("    /* Vincolo SET per '%s': controllo dei sotto-vincoli */\n", component_name);
        // Itera attraverso gli elementi del SET (che sono essi stessi vincoli)
        for (int m = 0; m < constraint->el_count; m++) {
            asn1p_constraint_t *sub_constraint = constraint->elements[m];
            // Applica lo switch al sotto-vincolo
            OUT("        /* Sotto-vincolo di tipo '%s' per '%s' */\n", asn1p_constraint_type2str(sub_constraint->type), component_name);
            switch (sub_constraint->type) {
                case ACT_CT_SIZE:
                {
                    OUT("            /* Controllo del vincolo SIZE (sotto-vincolo) per %s */\n", component_name);
                    asn1p_expr_type_e comp_etype = _find_terminal_type(&comp_arg);
                    asn1cnst_range_t *comp_s_value = asn1constraint_compute_constraint_range(
                        comp_arg.expr->Identifier, comp_etype, sub_constraint, ACT_CT_SIZE, 0, 0, 0);

                    if (comp_s_value && !comp_s_value->incompatible && !comp_s_value->empty_constraint) {
                        char size_var_name_str[128];
                        sprintf(size_var_name_str, "comp_size_%s_sval", component_name);
                        const char* size_var_name_ptr = size_var_name_str;
                        int type_processed_for_size = 1; // Flag per tracciare se il tipo è stato gestito

                        if (comp_arg.expr->marker.flags & EM_OPTIONAL) {
                            OUT("            if(typed_struct->%s) {\n", component_name); INDENT(+1);
                            OUT("                long %s;\n", size_var_name_ptr);
                            switch(comp_etype) {
                                case ASN_BASIC_BIT_STRING:
                                    OUT("                if(typed_struct->%s->size > 0) {\n", component_name);
                                    OUT("                    %s = 8 * typed_struct->%s->size - (typed_struct->%s->bits_unused & 0x07);\n", size_var_name_ptr, component_name, component_name);
                                    OUT("                } else {\n");
                                    OUT("                    %s = 0;\n", size_var_name_ptr);
                                    OUT("                }\n");
                                    break;
                                case ASN_STRING_UniversalString:
                                    OUT("                %s = typed_struct->%s->size >> 2; /* 4 byte per character */\n", size_var_name_ptr, component_name);
                                    break;
                                case ASN_STRING_BMPString:
                                    OUT("                %s = typed_struct->%s->size >> 1; /* 2 byte per character */\n", size_var_name_ptr, component_name);
                                    break;
                                case ASN_STRING_UTF8String:
                                    OUT("                %s = UTF8String_length(typed_struct->%s);\n", size_var_name_ptr, component_name);
                                    OUT("                if((ssize_t)%s < 0) {\n", size_var_name_ptr);
                                    OUT("                    ASN__CTFAIL(app_key, td, sptr, \"%%%%s: component '%s' UTF-8: broken encoding (%%%%s:%%%%d)\",\n", component_name);
                                    OUT("                        td->name, __FILE__, __LINE__);\n");
                                    OUT("                    return -1;\n");
                                    OUT("                }\n");
                                    break;
                                case ASN_CONSTR_SET_OF:
                                case ASN_CONSTR_SEQUENCE_OF:
                                    OUT("                %s = typed_struct->%s->count;\n", size_var_name_ptr, component_name);
                                    break;
                                case ASN_BASIC_OCTET_STRING:
                                default:
                                    if (comp_etype & ASN_STRING_MASK) {
                                        OUT("                %s = typed_struct->%s->size;\n", size_var_name_ptr, component_name);
                                    } else {
                                        OUT("                /* SIZE check for component %s of type %s not implemented (optional) */\n", component_name, asn1p_expr_type2str[comp_etype]);
                                        type_processed_for_size = 0;
                                    }
                                    break;
                            }

                            if(type_processed_for_size) {
                                abuf *comp_ab_s = emit_range_comparison_code(comp_s_value, size_var_name_ptr, 0, 0);
                                if (comp_ab_s && comp_ab_s->buffer && comp_ab_s->length > 0) {
                                    OUT("                if (!(%s)) {\n", comp_ab_s->buffer);
                                    OUT("                    ASN__CTFAIL(app_key, td, sptr,\n");
                                    OUT("                        \"%%%%s: component '%s' size constraint (sub) violated (%%%%s:%%%%d)\",\n", component_name);
                                    OUT("                        td->name, __FILE__, __LINE__);\n");
                                    OUT("                    return -1;\n");
                                    OUT("                }\n");
                                }
                                if (comp_ab_s) abuf_free(comp_ab_s);
                            }
                            INDENT(-1); OUT("            }\n");
                        } else { // Componente non opzionale
                            OUT("            long %s;\n", size_var_name_ptr);
                            switch(comp_etype) {
                                case ASN_BASIC_BIT_STRING:
                                    OUT("            if(typed_struct->%s.size > 0) {\n", component_name);
                                    OUT("                %s = 8 * typed_struct->%s.size - (typed_struct->%s.bits_unused & 0x07);\n", size_var_name_ptr, component_name, component_name);
                                    OUT("            } else {\n");
                                    OUT("                %s = 0;\n", size_var_name_ptr);
                                    OUT("            }\n");
                                    break;
                                case ASN_STRING_UniversalString:
                                    OUT("            %s = typed_struct->%s.size >> 2; /* 4 byte per character */\n", size_var_name_ptr, component_name);
                                    break;
                                case ASN_STRING_BMPString:
                                    OUT("            %s = typed_struct->%s.size >> 1; /* 2 byte per character */\n", size_var_name_ptr, component_name);
                                    break;
                                case ASN_STRING_UTF8String:
                                    OUT("            %s = UTF8String_length(&typed_struct->%s);\n", size_var_name_ptr, component_name);
                                    OUT("            if((ssize_t)%s < 0) {\n", size_var_name_ptr);
                                    OUT("                ASN__CTFAIL(app_key, td, sptr, \"%%%%s: component '%s' UTF-8: broken encoding (%%%%s:%%%%d)\",\n", component_name);
                                    OUT("                    td->name, __FILE__, __LINE__);\n");
                                    OUT("                return -1;\n");
                                    OUT("            }\n");
                                    break;
                                case ASN_CONSTR_SET_OF:
                                case ASN_CONSTR_SEQUENCE_OF:
                                    OUT("            %s = typed_struct->%s.count;\n", size_var_name_ptr, component_name);
                                    break;
                                case ASN_BASIC_OCTET_STRING:
                                default:
                                    if (comp_etype & ASN_STRING_MASK) {
                                        OUT("            %s = typed_struct->%s.size;\n", size_var_name_ptr, component_name);
                                    } else {
                                        OUT("            /* SIZE check for component %s of type %s not implemented (non-optional) */\n", component_name, asn1p_expr_type2str[comp_etype]);
                                        type_processed_for_size = 0;
                                    }
                                    break;
                            }

                            if(type_processed_for_size) {
                                abuf *comp_ab_s = emit_range_comparison_code(comp_s_value, size_var_name_ptr, 0, 0);
                                if (comp_ab_s && comp_ab_s->buffer && comp_ab_s->length > 0) {
                                    OUT("            if (!(%s)) {\n", comp_ab_s->buffer);
                                    OUT("                ASN__CTFAIL(app_key, td, sptr,\n");
                                    OUT("                    \"%%%%s: component '%s' size constraint (sub) violated (%%%%s:%%%%d)\",\n", component_name);
                                    OUT("                    td->name, __FILE__, __LINE__);\n");
                                    OUT("                return -1;\n");
                                    OUT("            }\n");
                                }
                                if (comp_ab_s) abuf_free(comp_ab_s);
                            }
                        }
                    }
                    if (comp_s_value) asn1constraint_range_free(comp_s_value);
                    break;
                }


                case ACT_EL_RANGE:
            {
                OUT("            /* Controllo del vincolo RANGE (sotto-vincolo) per %s */\n", component_name);
                asn1p_expr_type_e comp_etype = _find_terminal_type(&comp_arg);
                asn1cnst_range_t *comp_r_value = asn1constraint_compute_constraint_range(
                    comp_arg.expr->Identifier, comp_etype, sub_constraint, ACT_EL_RANGE, 0, 0, 0);

                if (comp_r_value && !comp_r_value->incompatible && !comp_r_value->empty_constraint) {
                    asn1cnst_range_t *type_intrinsic_range = NULL;
                    if (comp_r_value->left.type == ARE_MIN || comp_r_value->right.type == ARE_MAX) {
                        type_intrinsic_range = asn1constraint_compute_constraint_range(
                            comp_arg.expr->Identifier, comp_etype, comp_arg.expr->constraints, ACT_EL_RANGE, 0, 0, 0);

                        if (type_intrinsic_range && !type_intrinsic_range->incompatible && !type_intrinsic_range->empty_constraint) {
                            if (comp_r_value->left.type == ARE_MIN && type_intrinsic_range->left.type == ARE_VALUE) {
                                asn1c_integer_t type_left_val = type_intrinsic_range->left.value;
                                if (comp_r_value->right.type == ARE_MAX ||
                                    (comp_r_value->right.type == ARE_VALUE && type_left_val <= comp_r_value->right.value)) {
                                    comp_r_value->left.value = type_left_val;
                                    comp_r_value->left.type = ARE_VALUE;
                                }
                            }
                            if (comp_r_value->right.type == ARE_MAX && type_intrinsic_range->right.type == ARE_VALUE) {
                                asn1c_integer_t type_right_val = type_intrinsic_range->right.value;
                                if (comp_r_value->left.type == ARE_MIN ||
                                    (comp_r_value->left.type == ARE_VALUE && comp_r_value->left.value <= type_right_val)) {
                                    comp_r_value->right.value = type_right_val;
                                    comp_r_value->right.type = ARE_VALUE;
                                }
                            }
                        }
                    }

                    char val_var_name_str[128];
                    const char* val_var_name_ptr = NULL;

                    if (comp_etype == ASN_BASIC_INTEGER || comp_etype == ASN_BASIC_ENUMERATED) {
                        sprintf(val_var_name_str, "val_comp_%s_srange", component_name);
                        val_var_name_ptr = val_var_name_str;
                        enum asn1c_fitslong_e fits = asn1c_type_fits_long(&comp_arg, comp_arg.expr);
                        if (comp_arg.expr->marker.flags & EM_OPTIONAL) {
                            OUT("            if(typed_struct->%s) {\n", component_name); INDENT(+1);
                        }
                        if (fits == FL_FITS_UNSIGN) {
                            OUT("            unsigned long %s = typed_struct->%s;\n", val_var_name_ptr, component_name);
                        } else if (fits == FL_FITS_SIGNED) {
                            OUT("            long %s = typed_struct->%s;\n", val_var_name_ptr, component_name);
                        } else { /* FL_NOTFIT */
                            OUT("            long %s;\n", val_var_name_ptr);
                            OUT("            if(asn_INTEGER2long(&typed_struct->%s, &%s) != 0) {\n", component_name, val_var_name_ptr);
                            OUT("                ASN__CTFAIL(app_key, td, sptr, \"%%%%s: value of component '%s' too large for range check (%%%%s:%%%%d)\",\n", component_name);
                            OUT("                    td->name, __FILE__, __LINE__);\n");
                            OUT("                return -1;\n");
                            OUT("            }\n");
                        }
                    } else if (comp_etype == ASN_BASIC_REAL) {
                        sprintf(val_var_name_str, "val_comp_%s_srange", component_name);
                        val_var_name_ptr = val_var_name_str;
                        if (comp_arg.expr->marker.flags & EM_OPTIONAL) {
                            OUT("            if(typed_struct->%s) {\n", component_name); INDENT(+1);
                        }
                        OUT("            double %s = typed_struct->%s;\n", val_var_name_ptr, component_name);
                    } else {
                        OUT("            // Range check for component %s of type %s not yet implemented for sub-constraint.\n", component_name, asn1p_expr_type2str[comp_etype]);
                        if (type_intrinsic_range) asn1constraint_range_free(type_intrinsic_range);
                        if (comp_r_value) asn1constraint_range_free(comp_r_value);
                        break;
                    }

                    if (val_var_name_ptr) {
                        abuf *comp_ab = emit_range_comparison_code(comp_r_value, val_var_name_ptr,
                                                                 (comp_etype == ASN_BASIC_INTEGER || comp_etype == ASN_BASIC_ENUMERATED)
                                                                 && native_long_sign(&comp_arg, comp_r_value) >= 0,
                                                                 0);

                        if (comp_ab && comp_ab->buffer && comp_ab->length > 0) {
                            OUT("            if (!(%s)) {\n", comp_ab->buffer);
                            OUT("                ASN__CTFAIL(app_key, td, sptr,\n");
                            OUT("                    \"%%%%s: component '%s' range constraint (sub) violated (%%%%s:%%%%d)\",\n", component_name);
                            OUT("                    td->name, __FILE__, __LINE__);\n");
                            OUT("                return -1;\n");
                            OUT("            }\n");
                        }
                        if (comp_ab) abuf_free(comp_ab);
                    }
                    if (comp_arg.expr->marker.flags & EM_OPTIONAL && (comp_etype == ASN_BASIC_INTEGER || comp_etype == ASN_BASIC_ENUMERATED || comp_etype == ASN_BASIC_REAL)) {
                        INDENT(-1); OUT("            }\n");
                    }
                    if (type_intrinsic_range) asn1constraint_range_free(type_intrinsic_range);
                }
                if (comp_r_value) asn1constraint_range_free(comp_r_value);
                break;
            }

                case ACT_EL_VALUE:
                {
                    OUT("            /* Controllo del vincolo VALUE (sotto-vincolo) per %s */\n", component_name);
                    asn1p_expr_type_e comp_etype = _find_terminal_type(&comp_arg);
                    asn1p_value_t *constraint_value = sub_constraint->value;

                    if (!constraint_value) {
                        OUT("            /* Valore del vincolo non presente per %s nel sotto-vincolo VALUE */\n", component_name);
                        break;
                    }

                    int is_optional = (comp_arg.expr->marker.flags & EM_OPTIONAL);

                    switch (comp_etype) {
                        case ASN_BASIC_INTEGER:
                            if (constraint_value->type == ATV_INTEGER) {
                                char actual_value_var_name[128];
                                sprintf(actual_value_var_name, "actual_comp_%s_svalue", component_name);

                                if (is_optional) {
                                    OUT("            if(typed_struct->%s) {\n", component_name); INDENT(+1);
                                }

                                OUT("            long %s;\n", actual_value_var_name);

                                char component_access_for_conversion[256];
                                if (is_optional) {
                                    // Assumendo che se is_optional, typed_struct->component_name sia un puntatore a INTEGER_t
                                    sprintf(component_access_for_conversion, "typed_struct->%s", component_name);
                                } else {
                                    sprintf(component_access_for_conversion, "&typed_struct->%s", component_name);
                                }

                                OUT("            if(asn_INTEGER2long(%s, &%s) != 0) {\n",
                                    component_access_for_conversion, actual_value_var_name);
                                INDENT(+1);
                                OUT("                ASN__CTFAIL(app_key, td, sptr,\n");
                                OUT("                    \"%%%%s: component '%s' (INTEGER) value too large for constraint check (%%%%s:%%%%d)\",\n",
                                    component_name);
                                OUT("                    td->name, __FILE__, __LINE__);\n");
                                OUT("                return -1;\n");
                                INDENT(-1);
                                OUT("            }\n");

                                OUT("            if (%s != (long long)&typed_struct->%s) {\n", actual_value_var_name, component_name);
                                INDENT(+1);
                                OUT("                ASN__CTFAIL(app_key, td, sptr,\n");
                                OUT("                    \"%%%%s: component '%s' (INTEGER) value constraint violated (expected %%lld, got %%ld) (%%%%s:%%%%d)\",\n",
                                    component_name,
                                    (long long)constraint_value->value.v_integer,
                                    actual_value_var_name);
                                OUT("                    td->name, __FILE__, __LINE__);\n");
                                OUT("                return -1;\n");
                                INDENT(-1);
                                OUT("            }\n");

                                if (is_optional) {
                                    INDENT(-1); OUT("            }\n");
                                }
                            } else {
                                OUT("            /* Tipo di valore del vincolo (enum: %d) non corrispondente per INTEGER per %s */\n",
                                    (int)constraint_value->type, component_name);
                            }
                            break; // Fine case ASN_BASIC_INTEGER


                        case ASN_BASIC_OCTET_STRING:
                        case ASN_STRING_UTF8String:
                        case ASN_STRING_PrintableString:
                        case ASN_STRING_VisibleString: /* alias ISO646String */
                        case ASN_STRING_IA5String:
                        case ASN_STRING_NumericString:
                            if (constraint_value->type == ATV_STRING) {
                                if (is_optional) {
                                    OUT("            if(typed_struct->%s) {\n", component_name); INDENT(+1);
                                    OUT("                const OCTET_STRING_t *st = (const OCTET_STRING_t *)typed_struct->%s;\n", component_name);
                                    OUT("                /* ATTENZIONE: La seguente chiamata usa emit_single_value_string_constraint. */\n");
                                    OUT("                /* Questa funzione usa strndup (richiede free per 'c_string'), strcmp (non sicuro per binari), */\n");
                                    OUT("                /* e ritorna -1 direttamente senza ASN__CTFAIL. */\n");
                                    emit_single_value_string_constraint(&comp_arg, constraint, m); // constraint è ct, m è l'indice
                                    OUT("                if(st && st->buf) free((char *)c_string); /* Liberare la memoria allocata da strndup in emit_single_value_string_constraint */\n");
                                    INDENT(-1); OUT("            }\n");
                                } else {
                                    OUT("            const OCTET_STRING_t *st = (const OCTET_STRING_t *)&typed_struct->%s;\n", component_name);
                                    OUT("            /* ATTENZIONE: La seguente chiamata usa emit_single_value_string_constraint. */\n");
                                    OUT("            /* Questa funzione usa strndup (richiede free per 'c_string'), strcmp (non sicuro per binari), */\n");
                                    OUT("            /* e ritorna -1 direttamente senza ASN__CTFAIL. */\n");
                                    emit_single_value_string_constraint(&comp_arg, constraint, m); // constraint è ct, m è l'indice
                                    OUT("            if(st && st->buf) free((char *)c_string); /* Liberare la memoria allocata da strndup in emit_single_value_string_constraint */\n");
                                }
                            } else {
                                OUT("            /* Tipo di valore del vincolo (enum: %d) non corrispondente per %s per %s */\n",
                                    (int)constraint_value->type, asn1p_expr_type2str[comp_etype], component_name);
                            }
                            break; // Fine case STRING


                        default:
                            OUT("            /* Controllo del vincolo VALUE (sotto-vincolo) non implementato per il tipo %s del componente %s */\n",
                                asn1p_expr_type2str[comp_etype], component_name);
                            break;
                    }
                    break; // Fine case ACT_EL_VALUE
                }
                case ACT_CT_PATTERN:
                    // TODO: Implementare il controllo del vincolo PATTERN (sotto-vincolo)
                    OUT("            // TODO: Implementare il controllo del vincolo PATTERN (sotto-vincolo) per %s\n", component_name);
                    break;
                default:
                    // Tipo di sotto-vincolo non supportato
                    OUT("            // Tipo di sotto-vincolo '%s' non gestito per %s\n", asn1p_constraint_type2str(sub_constraint->type), component_name);
                    break;
            }
        }
    } else {
        // Comportamento originale: lo switch si applica direttamente a 'constraint'
        OUT("    /* Controllo del vincolo di tipo '%s' per il componente '%s' */\n", asn1p_constraint_type2str(constraint->type), component_name);
        switch (constraint->type) {
            case ACT_CT_SIZE:
                // TODO: Implementare il controllo del vincolo SIZE
                OUT("        // TODO: Implementare il controllo del vincolo SIZE per %s\n", component_name);
                break;
            case ACT_EL_RANGE:
                // TODO: Implementare il controllo del vincolo RANGE
                OUT("        // TODO: Implementare il controllo del vincolo RANGE per %s\n", component_name);
                break;
            case ACT_EL_VALUE:
                // TODO: Implementare il controllo del vincolo VALUE
                OUT("        // TODO: Implementare il controllo del vincolo VALUE per %s\n", component_name);
                break;
            case ACT_CT_PATTERN:
                // TODO: Implementare il controllo del vincolo PATTERN
                OUT("        // TODO: Implementare il controllo del vincolo PATTERN per %s\n", component_name);
                break;
            case ACT_CT_CTDBY:
                // TODO: Implementare il controllo del vincolo CONSTRAINED BY
                OUT("        // TODO: Implementare il controllo del vincolo CONSTRAINED BY per %s\n", component_name);
                break;
            default:
                // Tipo di vincolo non supportato
                OUT("        // Tipo di vincolo '%s' non gestito per %s\n", asn1p_constraint_type2str(constraint->type), component_name);
                break;
        }
    }
    }
}


int
asn1c_emit_constraint_checking_code(arg_t *arg) {
	asn1cnst_range_t *r_size;
	asn1cnst_range_t *r_value;
	asn1p_expr_t *expr = arg->expr;
	asn1p_expr_type_e etype;
	asn1p_constraint_t *ct;
	int alphabet_table_compiled;
	int produce_st = 0;
	int ulong_optimize = 0;
	int value_unsigned = 0;
	int ret = 0;

	ct = expr->combined_constraints;

	if(ct == NULL)
		return 1;	/* No additional constraints defined */\


/*
	if (ct != NULL) {
		printf("Debug info:\n");
		printf("ct address: %p\n", (void*)ct);
		printf("ct->type raw value: %d\n", ct->type);
		printf("ct->type as enum: %s\n", asn1p_constraint_type2str(ct->type));

		// Se è un SET, controlla i suoi elementi
		if (ct->type == ACT_CA_SET && ct->elements != NULL) {
			printf("Number of elements in SET: %d\n", ct->el_count);
			for (unsigned int i = 0; i < ct->el_count; i++) {
				printf("Element %d type: %s\n", i,
					asn1p_constraint_type2str(ct->elements[i]->type));
			}
		}
}
*/

	//Gives back the base type on which the constraint is applied
	etype = _find_terminal_type(arg);
	 if (etype & ASN_STRING_MASK) {
        //printf("Expression is a string type\n");
		}
	r_value=asn1constraint_compute_constraint_range(expr->Identifier, etype, ct, ACT_EL_RANGE,0,0,0);
	r_size =asn1constraint_compute_constraint_range(expr->Identifier, etype, ct, ACT_CT_SIZE, 0,0,0);
	if(r_value) {
		if(r_value->incompatible
		|| r_value->empty_constraint
		|| (r_value->left.type == ARE_MIN
			&& r_value->right.type == ARE_MAX)
		|| (etype == ASN_BASIC_BOOLEAN
			&& r_value->left.value == 0
			&& r_value->right.value == 1)
		) {
			asn1constraint_range_free(r_value);
			r_value = 0;
		}
	}
	if(r_size) {
		if(r_size->incompatible
		|| r_size->empty_constraint
		|| (r_size->left.value == 0	/* or .type == MIN */
			&& r_size->right.type == ARE_MAX)
		) {
			asn1constraint_range_free(r_size);
			r_size = 0;
		}
	}

	/*
	 * Do we really need an "*st = sptr" pointer?
	 */
	switch(etype) {
	case ASN_BASIC_INTEGER:
	case ASN_BASIC_ENUMERATED:
		if(asn1c_type_fits_long(arg, arg->expr) == FL_NOTFIT)
			produce_st = 1;
		break;
	case ASN_BASIC_REAL:
        if((arg->flags & A1C_USE_WIDE_TYPES)
           && asn1c_REAL_fits(arg, arg->expr) == RL_NOTFIT)
            produce_st = 1;
		break;
	case ASN_BASIC_BIT_STRING:
	case ASN_BASIC_OCTET_STRING:
		produce_st = 1;
		break;
	default:
		if(etype & ASN_STRING_MASK){
			produce_st = 1;
			}
		break;
	}
	if(produce_st) {
		const char *tname = asn1c_type_name(arg, arg->expr, TNF_SAFE);
		OUT("const %s_t *st = (const %s_t *)sptr;\n", tname, tname);
	}

	if(r_size || r_value) {
		if(r_size) {
			OUT("size_t size;\n");
		}
		if(r_value)
			switch(etype) {
			case ASN_BASIC_INTEGER:
			case ASN_BASIC_ENUMERATED:
				if(native_long_sign(arg, r_value) >= 0) {
					ulong_optimize = ulong_optimization(arg, etype, r_size, r_value);
					if(!ulong_optimize) {
						value_unsigned = 1;
						OUT("unsigned long value;\n");
					}
				} else {
					OUT("long value;\n");
				}
				break;
			case ASN_BASIC_REAL:
				OUT("%s value;\n", c_name(arg).type.constrained_c_name);
				break;
			case ASN_BASIC_BOOLEAN:
				OUT("BOOLEAN_t value;\n");
				break;
			default:
				break;
		}
	}

	OUT("\n");

	/*
	 * Protection against null input.
	 */
	OUT("if(!sptr) {\n");
		INDENT(+1);
		OUT("ASN__CTFAIL(app_key, td, sptr,\n");
		OUT("\t\"%%s: value not given (%%s:%%d)\",\n");
		OUT("//Sto provando a modificare i Costrain,\n");
		OUT("\ttd->name, __FILE__, __LINE__);\n");
		OUT("return -1;\n");
		INDENT(-1);
	OUT("}\n");
	OUT("\n");

    //Parte aggiunta
	if (ct->type == ACT_CA_SET && ct->elements != NULL) {
        printf("Number of elements in SET: %d\n", ct->el_count);
        for (unsigned int i = 0; i < ct->el_count; i++) {
            //Implementazione del Vincolo pattern
			if(ct->elements[i]->type == ACT_CT_PATTERN) {
				 // Reindirizza agli include
			    emit_regex_include(arg);
                emit_pattern_constraint(arg, ct, i);
			}

            //Implementazione del Vincolo  per single value delle Stringhe
            if(ct->elements[i]->type == ACT_EL_VALUE && etype & ASN_STRING_MASK) {
               printf("Im in\n");
                emit_single_value_string_constraint(arg, ct,i);
            }
            if(ct->elements[i]->type == ACT_CT_WCOMPS) {
                printf("Im in WITH COMPONETS\n");
                //Gestione del constraint WCOMPONENTS
                OUT("//PROVA WITH COMPONENTS \n");
                asn1p_constraint_t *new_ct = ct->elements[i];
                // Recupero il nome del tipo (AdultPerson)
                const char *type_name = arg->expr->Identifier;
                OUT("// Tipo con vincolo: %s\n", type_name);
                // Creo un puntatore tipizzato alla struttura per accesso diretto ai campi
                OUT("// Accesso alla struttura tipizzata\n");
                OUT("const %s_t *typed_struct = (const %s_t *)sptr;\n", type_name, type_name);

                // Recupero la struttura del tipo corrente per estrarne i membri
                asn1p_expr_t *type_def = arg->expr;
                for (unsigned int j = 0; j < new_ct->el_count; j++) {
                        // char *component_name = new_ct->elements[j]->value->value.reference->components->name;
                        // if(component_name != NULL) {
                        //     printf("Componente con vincolo: %s\n", component_name);
                        //
                        // }
                    if(new_ct->elements[j]->value && new_ct->elements[j]->value->type == ATV_REFERENCED) {
            // Estrai il nome del componente
            char *component_name = NULL;
            if(new_ct->elements[j]->value->value.reference &&
               new_ct->elements[j]->value->value.reference->components) {
                component_name = new_ct->elements[j]->value->value.reference->components->name;
            }

            if(component_name) {
                printf("Componente con vincolo: %s\n", component_name);

                // Genera codice per l'accesso e la verifica del componente
                OUT("// Verifica del componente: %s\n", component_name);
                // Controlla la presenza del campo (PRESENT o ABSENT)
                if(new_ct->elements[j]->presence == ACPRES_PRESENT) {
                    OUT("if(!typed_struct->%s) {\n", component_name);
                    OUT("    ASN__CTFAIL(app_key, td, sptr,\n");
                    OUT("        \"%%s: Component '%s' deve essere presente\",\n", component_name);
                    OUT("        td->name);\n");
                    OUT("    return -1;\n");
                    OUT("}\n");
                } else if(new_ct->elements[j]->presence == ACPRES_ABSENT) {
                    OUT("if(typed_struct->%s) {\n", component_name);
                    OUT("    ASN__CTFAIL(app_key, td, sptr,\n");
                    OUT("        \"%%s: Component '%s' deve essere assente\",\n", component_name);
                    OUT("        td->name);\n");
                    OUT("    return -1;\n");
                    OUT("}\n");
                }

                // Se ci sono ulteriori vincoli sul componente
                if(new_ct->elements[j]->el_count > 0) {
                    OUT("// Vincoli aggiuntivi sul componente %s\n", component_name);
                    OUT("if(typed_struct->%s) {\n", component_name);

                    // Chiamata alla funzione per generare i controlli dei vincoli
                    emit_component_constraint_checks(arg, new_ct->elements[j], component_name);

                    OUT("}\n");
                }
            }
        }
                    printf("Element %d type: %s\n", j,
                           asn1p_constraint_type2str(new_ct->elements[j]->type));
                }
                // Aggiungi una variabile per facilitare l'elaborazione successiva
                OUT("// Memorizzazione del tipo per uso successivo\n");
                OUT("const char *constraint_type = \"%s\";\n", type_name);
            }
            int value_found = 0;
            int first_string = 0;
            int first_pattern = 0;
            //Implementazione del Vincolo per single value con Union
            if(ct->elements[i]->type == ACT_CA_UNI) {
                // Gestione del constraint UNION
                printf("Constraint UNION found\n");
                for (unsigned int j = 0; j < ct->elements[i]->el_count; j++) {
                    printf("Element %d type: %s\n", j,
                           asn1p_constraint_type2str(ct->elements[i]->elements[j]->type));
                    if(ct->elements[i]->elements[j]->type == ACT_EL_VALUE && etype & ASN_STRING_MASK) {
                        if (value_found == 0) {
                            value_found = 1;
                            OUT("int union_contains = 0;\n");
                        }
                        emit_single_value_string_constraint_union(arg, ct,i,j, first_string);
                        first_string++;
                    }

                    if(ct->elements[i]->elements[j]->type == ACT_CT_PATTERN) {
                        printf("DEBUG\n");
                        if (value_found == 0) {
                            value_found = 1;
                            OUT("int union_contains = 0;\n");
                        }
                        emit_regex_include(arg);
                        emit_pattern_constraint_union(arg, ct,i,j, first_pattern);
                        first_pattern++;
                    }
                    if (j == ct->elements[i]->el_count - 1) {
                        OUT("if (union_contains == 0) {\n");
                        INDENT(+1);
                        OUT("\t return -1;\n");
                        OUT("}\n");
                        INDENT(-1);
                    }
                }

            }


            // printf("Element %d type: %s\n", i,
            //        asn1p_constraint_type2str(ct->elements[i]->type));
            // printf("Constrain %d type: %d\n", i,
            //        ct->elements[i]->type);

        }
    }


	if((r_value) && (!ulong_optimize))
		emit_value_determination_code(arg, etype, r_value);
	if(r_size)
		emit_size_determination_code(arg, etype);

	INDENT(-1);
	REDIR(OT_CTABLES);
	/* Emit FROM() tables */
	alphabet_table_compiled =
		(asn1c_emit_constraint_tables(arg, r_size?1:0) == 1);
	REDIR(OT_CODE);
	INDENT(+1);

	/*
	 * Optimization for unsigned longs.
	 */
	if(ulong_optimize) {
		OUT("\n");
		OUT("/* Constraint check succeeded */\n");
		OUT("return 0;\n");
		goto end;
	}

	/*
	 * Here is an if() {} else {} consrtaint checking code.
	 */
	int got_something = 0;
    int value_unused = 0;
	OUT("\n");
	OUT("if(");
	INDENT(+1);
		if(r_size) {
            abuf *ab = emit_range_comparison_code(r_size, "size", 0, -1);
            if(ab->length)  {
                OUT("(%s)", ab->buffer);
                got_something++;
            }
            abuf_free(ab);
		}
		if(r_value) {
			if(got_something) { OUT("\n"); OUT(" && "); }
            abuf *ab;
            if(etype == ASN_BASIC_BOOLEAN)
                ab = emit_range_comparison_code(r_value, "value", 0, 1);
            else
                ab = emit_range_comparison_code(r_value, "value",
                                                value_unsigned ? 0 : -1, -1);
            if(ab->length)  {
                OUT("(%s)", ab->buffer);
                got_something++;
            } else {
                value_unused = 1;
            }
            abuf_free(ab);
		}
		if(alphabet_table_compiled) {
			if(got_something) { OUT("\n"); OUT(" && "); }
			OUT("!check_permitted_alphabet_%d(%s)",
				arg->expr->_type_unique_index,
				produce_st ? "st" : "sptr");
            got_something++;
        }
		if(!got_something) {
			OUT("1 /* No applicable constraints whatsoever */");
			OUT(") {\n");
			INDENT(-1);
			if(produce_st) {
				INDENTED(OUT("(void)st; /* Unused variable */\n"));
			}
			if(value_unused) {
				INDENTED(OUT("(void)value; /* Unused variable */\n"));
			}
			INDENTED(OUT("/* Nothing is here. See below */\n"));
			OUT("}\n");
			OUT("\n");
			ret = 1;
			goto end;
		}
	INDENT(-1);
	OUT(") {\n");
		INDENT(+1);
		switch(etype) {
		case ASN_CONSTR_SEQUENCE_OF:
			OUT("/* Perform validation of the inner elements */\n");
			OUT("return SEQUENCE_OF_constraint(td, sptr, ctfailcb, app_key);\n");
			break;
		case ASN_CONSTR_SET_OF:
			OUT("/* Perform validation of the inner elements */\n");
			OUT("return SET_OF_constraint(td, sptr, ctfailcb, app_key);\n");
			break;
		default:
			OUT("/* Constraint check succeeded */\n");
			OUT("return 0;\n");
		}
		INDENT(-1);
	OUT("} else {\n");
		INDENT(+1);
			OUT("ASN__CTFAIL(app_key, td, sptr,\n");
			OUT("\t\"%%s: constraint failed (%%s:%%d)\",\n");
			OUT("\ttd->name, __FILE__, __LINE__);\n");
			OUT("return -1;\n");
		INDENT(-1);
	OUT("}\n");
	end:
	if (r_value) asn1constraint_range_free(r_value);
	if (r_size) asn1constraint_range_free(r_size);

	return ret;
}

static int
asn1c_emit_constraint_tables(arg_t *arg, int got_size) {
	asn1c_integer_t range_start;
	asn1c_integer_t range_stop;
	asn1p_expr_type_e etype;
	asn1cnst_range_t *range;
	asn1p_constraint_t *ct;
	int utf8_full_alphabet_check = 0;
	int max_table_size = 256;
	int table[256];
	int use_table;

	ct = arg->expr->combined_constraints;
	if(!ct) return 0;

	etype = _find_terminal_type(arg);

	range = asn1constraint_compute_constraint_range(arg->expr->Identifier, etype, ct, ACT_CT_FROM, 0,0,0);
	if(!range) return 0;

	if(range->incompatible
	|| range->empty_constraint) {
		asn1constraint_range_free(range);
		return 0;
	}

	if(range->left.type == ARE_MIN
	&& range->right.type == ARE_MAX) {
		/*
		 * The permitted alphabet constraint checker code guarantees
		 * that either both bounds (left/right) are present, or
		 * they're absent simultaneously. Thus, this assertion
		 * legitimately holds true.
		 */
		assert(range->el_count == 0);
		/* The full range is specified. Ignore it. */
		asn1constraint_range_free(range);
		return 0;
	}

	range_start = range->left.value;
	range_stop = range->right.value;
	assert(range->left.type == ARE_VALUE);
	assert(range->right.type == ARE_VALUE);
	assert(range_start <= range_stop);

	range_start = 0;	/* Force old behavior */

	/*
	 * Check if we need a test table to check the alphabet.
	 */
	use_table = 1;
	if(range->el_count == 0) {
		/*
		 * It's better to have a short if() check
		 * than waste 1k of table space
		 */
		use_table = 0;
	}
	if((range_stop - range_start) > 255)
		use_table = 0;
	if(etype == ASN_STRING_UTF8String) {
		if(range_stop >= 0x80)
			use_table = 0;
		else
			max_table_size = 128;
	}

	if(use_table) {
		int cardinal = 0;
		int i, n = 0;
		int untl;
		memset(table, 0, sizeof(table));
		for(i = -1; i < range->el_count; i++) {
			asn1cnst_range_t *r;
			asn1c_integer_t v;
			if(i == -1) {
				if(range->el_count) continue;
				r = range;
			} else {
				r = range->elements[i];
			}
			for(v = r->left.value; v <= r->right.value; v++) {
				assert((v - range_start) >= 0);
				assert((v - range_start) < max_table_size);
				table[v - range_start] = ++n;
			}
		}

		untl = (range_stop - range_start) + 1;
		untl += (untl % 16)?16 - (untl % 16):0;
		OUT("static const int permitted_alphabet_table_%d[%d] = {\n",
			arg->expr->_type_unique_index, max_table_size);
		for(n = 0; n < untl; n++) {
			cardinal += table[n] ? 1 : 0;
			OUT("%2d,", table[n]);
			if(!((n+1) % 16)) {
				int c;
				if(!n || (n-15) + range_start >= 0x80) {
					OUT("\n");
					continue;
				}
				OUT("\t/* ");
				for(c = n - 15; c <= n; c++) {
					if(table[c]) {
						int a = c + range_start;
						if(a > 0x20 && a < 0x80)
							OUT("%c", a);
						else
							OUT(".");
					} else {
						OUT(" ");
					}
				}
				OUT(" */");
				OUT("\n");
			}
		}
		OUT("};\n");

		if((arg->flags & (A1C_GEN_UPER | A1C_GEN_APER))
		&& (etype & ASN_STRING_KM_MASK)) {
		    int c;
		    OUT("static const int permitted_alphabet_code2value_%d[%d] = {\n",
			arg->expr->_type_unique_index, cardinal);
		    for(n = c = 0; c < max_table_size; c++) {
			if(table[c]) {
				OUT("%d,", c);
				if(!((++n) % 16)) OUT("\n");
			}
		    }
		    OUT("};\n");
		    OUT("\n");
		    DEBUG("code2value map gen for %s", arg->expr->Identifier);
		    arg->expr->_mark |= TM_PERFROMCT;
		}

		OUT("\n");
	} else if(etype == ASN_STRING_UTF8String) {
		/*
		 * UTF8String type is a special case in many respects.
		 */
		if(got_size) {
			/*
			 * Size has been already determined.
			 * The UTF8String length checker also checks
			 * for the syntax validity, so we don't have
			 * to repeat this process twice.
			 */
			asn1constraint_range_free(range);
			return 0;
		} else {
			utf8_full_alphabet_check = 1;
		}
	} else {
		/*
		 * This permitted alphabet check will be
		 * expressed using conditional statements
		 * instead of table lookups. Table would be
		 * to large or otherwise inappropriate (too sparse?).
		 */
	}

	OUT("static int check_permitted_alphabet_%d(const void *sptr) {\n",
			arg->expr->_type_unique_index);
	INDENT(+1);
	if(utf8_full_alphabet_check) {
		OUT("if(UTF8String_length((const UTF8String_t *)sptr) < 0)\n");
		OUT("\treturn -1; /* Alphabet (sic!) test failed. */\n");
		OUT("\n");
	} else {
		if(use_table) {
			OUT("const int *table = permitted_alphabet_table_%d;\n",
				arg->expr->_type_unique_index);
			emit_alphabet_check_loop(arg, 0);
		} else {
			emit_alphabet_check_loop(arg, range);
		}
	}
	OUT("return 0;\n");
	INDENT(-1);
	OUT("}\n");
	OUT("\n");

	asn1constraint_range_free(range);

	return 1;
}

static int
emit_alphabet_check_loop(arg_t *arg, asn1cnst_range_t *range) {
	asn1c_integer_t natural_stop;
	asn1p_expr_t *terminal;
	const char *tname;

	terminal = asn1f_find_terminal_type_ex(arg->asn, arg->ns, arg->expr);
	if(terminal) {
		OUT("/* The underlying type is %s */\n",
			ASN_EXPR_TYPE2STR(terminal->expr_type));
	} else {
		terminal = arg->expr;
	}
	tname = asn1c_type_name(arg, terminal, TNF_SAFE);
	OUT("const %s_t *st = (const %s_t *)sptr;\n", tname, tname);

	switch(terminal->expr_type) {
	case ASN_STRING_UTF8String:
		OUT("const uint8_t *ch = st->buf;\n");
		OUT("const uint8_t *end = ch + st->size;\n");
		OUT("\n");
		OUT("for(; ch < end; ch++) {\n");
			INDENT(+1);
			OUT("uint8_t cv = *ch;\n");
			if(!range) OUT("if(cv >= 0x80) return -1;\n");
		natural_stop = 0xffffffffUL;
		break;
	case ASN_STRING_UniversalString:
		OUT("const uint8_t *ch = st->buf;\n");
		OUT("const uint8_t *end = ch + st->size;\n");
		OUT("\n");
		OUT("if(st->size %% 4) return -1; /* (size%%4)! */\n");
		OUT("for(; ch < end; ch += 4) {\n");
			INDENT(+1);
			OUT("uint32_t cv = (ch[0] << 24)\n");
			OUT("\t\t| (ch[1] << 16)\n");
			OUT("\t\t| (ch[2] << 8)\n");
			OUT("\t\t|  ch[3];\n");
			if(!range) OUT("if(cv > 255) return -1;\n");
		natural_stop = 0xffffffffUL;
		break;
	case ASN_STRING_BMPString:
		OUT("const uint8_t *ch = st->buf;\n");
		OUT("const uint8_t *end = ch + st->size;\n");
		OUT("\n");
		OUT("if(st->size %% 2) return -1; /* (size%%2)! */\n");
		OUT("for(; ch < end; ch += 2) {\n");
			INDENT(+1);
			OUT("uint16_t cv = (ch[0] << 8)\n");
			OUT("\t\t| ch[1];\n");
			if(!range) OUT("if(cv > 255) return -1;\n");
		natural_stop = 0xffff;
		break;
	case ASN_BASIC_OCTET_STRING:
	default:
		OUT("const uint8_t *ch = st->buf;\n");
		OUT("const uint8_t *end = ch + st->size;\n");
		OUT("\n");
		OUT("for(; ch < end; ch++) {\n");
			INDENT(+1);
			OUT("uint8_t cv = *ch;\n");
		natural_stop = 0xff;
		break;
	}

	if(range) {
        abuf *ab = emit_range_comparison_code(range, "cv", 0, natural_stop);
        if(ab->length) {
            OUT("if(!(%s)) return -1;\n", ab->buffer);
        } else {
            OUT("(void)cv; /* Unused variable */\n");
        }
	} else {
		OUT("if(!table[cv]) return -1;\n");
	}

	INDENT(-1);
	OUT("}\n");

	return 0;
}

static void
abuf_oint(abuf *ab, asn1c_integer_t v, asn1c_integer_t natural_start) {
    if(v == (-2147483647L - 1)) {
        abuf_printf(ab, "(-2147483647L - 1)");
    } else {
        abuf_printf(ab, "%s%s", asn1p_itoa(v), natural_start ? "L" : "UL");
    }
}

static abuf *
emit_range_comparison_code(asn1cnst_range_t *range, const char *varname,
                           asn1c_integer_t natural_start,
                           asn1c_integer_t natural_stop) {
    abuf *ab = abuf_new();
	//OUT("Sto provando a modificare i Costrain dei numeri,\n");
    if(range->el_count == 0) {
        int ignore_left =
            (range->left.type == ARE_MIN)
            || (natural_start != -1 && range->left.value <= natural_start);
        int ignore_right =
            (range->right.type == ARE_MAX)
            || (natural_stop != -1 && range->right.value >= natural_stop);

        if(ignore_left && ignore_right) {
            /* Empty constraint comparison */
        } else if(ignore_left) {
            abuf_printf(ab, "%s <= ", varname);
            abuf_oint(ab, range->right.value, natural_start);
        } else if(ignore_right) {
            abuf_printf(ab, "%s >= ", varname);
            abuf_oint(ab, range->left.value, natural_start);
        } else if(range->left.value == range->right.value) {
            abuf_printf(ab, "%s == ", varname);
            abuf_oint(ab, range->right.value, natural_start);
        } else {
            abuf_printf(ab, "%s >= ", varname);
            abuf_oint(ab, range->left.value, natural_start);
            abuf_printf(ab, " && ");
            abuf_printf(ab, "%s <= ", varname);
            abuf_oint(ab, range->right.value, natural_start);
        }
    } else {
        for(int i = 0; i < range->el_count; i++) {
            asn1cnst_range_t *r = range->elements[i];

            abuf *rec = emit_range_comparison_code(r, varname, natural_start,
                                                   natural_stop);
            if(rec->length) {
                if(ab->length) {
                    abuf_str(ab, " || ");
                }
                abuf_str(ab, "(");
                abuf_buf(ab, rec);
                abuf_str(ab, ")");
            } else {
                /* Ignore this part */
            }
            abuf_free(rec);
        }
    }

    return ab;
}

static int
emit_size_determination_code(arg_t *arg, asn1p_expr_type_e etype) {

	switch(etype) {
	case ASN_BASIC_BIT_STRING:
		OUT("if(st->size > 0) {\n");
		OUT("\t/* Size in bits */\n");
		OUT("\tsize = 8 * st->size - (st->bits_unused & 0x07);\n");
		OUT("} else {\n");
		OUT("\tsize = 0;\n");
		OUT("}\n");
		break;
	case ASN_STRING_UniversalString:
		OUT("size = st->size >> 2;\t/* 4 byte per character */\n");
		break;
	case ASN_STRING_BMPString:
		OUT("size = st->size >> 1;\t/* 2 byte per character */\n");
		break;
	case ASN_STRING_UTF8String:
		OUT("size = UTF8String_length(st);\n");
		OUT("if((ssize_t)size < 0) {\n");
		OUT("\tASN__CTFAIL(app_key, td, sptr,\n");
		OUT("\t\t\"%%s: UTF-8: broken encoding (%%s:%%d)\",\n");
		OUT("\t\ttd->name, __FILE__, __LINE__);\n");
		OUT("\treturn -1;\n");
		OUT("}\n");
		break;
	case ASN_CONSTR_SET_OF:
	case ASN_CONSTR_SEQUENCE_OF:
		OUT("/* Determine the number of elements */\n");
		OUT("size = _A_C%s_FROM_VOID(sptr)->count;\n",
			etype==ASN_CONSTR_SET_OF?"SET":"SEQUENCE");
		break;
	case ASN_BASIC_OCTET_STRING:
		OUT("size = st->size;\n");
		break;
	default:
		if(etype & ASN_STRING_MASK) {
			OUT("size = st->size;\n");
			break;
		} else {
			const char *type_name = ASN_EXPR_TYPE2STR(etype);
			if(!type_name) type_name = arg->expr->Identifier;
			WARNING("SizeConstraint is not defined for %s",
				type_name);
			OUT_NOINDENT("#warning SizeConstraint "
				"is not defined for %s!\n", type_name);
			OUT("size = st->size;\n");
		}
		return -1;
	}

	return 0;
}

static int
emit_value_determination_code(arg_t *arg, asn1p_expr_type_e etype, asn1cnst_range_t *r_value) {

	switch(etype) {
	case ASN_BASIC_INTEGER:
	case ASN_BASIC_ENUMERATED:
		OUT("//Sto provando a modificare i Costrain dei numeri,\n");
		if(asn1c_type_fits_long(arg, arg->expr) == FL_FITS_UNSIGN) {
			OUT("value = *(const unsigned long *)sptr;\n");

		} else if(asn1c_type_fits_long(arg, arg->expr) != FL_NOTFIT) {
			OUT("value = *(const long *)sptr;\n");
		} else {
			/*
			 * In some cases we can explore our knowledge of
			 * underlying INTEGER_t->buf format.
			 */
			if(r_value->el_count == 0
			&& (
				/* Speed-up common case: (0..MAX) */
				(r_value->left.type == ARE_VALUE
				&& r_value->left.value == 0
				&& r_value->right.type == ARE_MAX)
			    ||
				/* Speed-up common case: (MIN..-1) */
				(r_value->left.type == ARE_MIN
				&& r_value->right.type == ARE_VALUE
				&& r_value->right.value == -1)
			)) {
				OUT("/* Check if the sign bit is present */\n");
				OUT("value = st->buf ? ((st->buf[0] & 0x80) ? -1 : 1) : 0;\n");
				break;
			}

			if(native_long_sign(arg, r_value) >= 0) {
				/* Special case for treating unsigned longs */
				OUT("if(asn_INTEGER2ulong(st, &value)) {\n");
				INDENT(+1);
				OUT("ASN__CTFAIL(app_key, td, sptr,\n");
				OUT("\t\"%%s: value too large (%%s:%%d)\",\n");
				OUT("\ttd->name, __FILE__, __LINE__);\n");
				OUT("return -1;\n");
				INDENT(-1);
				OUT("}\n");
			} else {
				OUT("if(asn_INTEGER2long(st, &value)) {\n");
				INDENT(+1);
				OUT("ASN__CTFAIL(app_key, td, sptr,\n");
				OUT("\t\"%%s: value too large (%%s:%%d)\",\n");
				OUT("\ttd->name, __FILE__, __LINE__);\n");
				OUT("return -1;\n");
				INDENT(-1);
				OUT("}\n");
			}
		}
		break;
	case ASN_BASIC_REAL:
		if(arg->flags & A1C_USE_WIDE_TYPES) {
			OUT("if(asn_REAL2double(st, &value)) {\n");
				INDENT(+1);
				OUT("ASN__CTFAIL(app_key, td, sptr,\n");
				OUT("\t\"%%s: value too large (%%s:%%d)\",\n");
				OUT("\ttd->name, __FILE__, __LINE__);\n");
				OUT("return -1;\n");
				INDENT(-1);
			OUT("}\n");
		} else {
			OUT("value = *(const %s *)sptr;\n", c_name(arg).type.c_name);
		}
		break;
	case ASN_BASIC_BOOLEAN:
		OUT("value = (*(const long *)sptr) ? 1 : 0;\n");
		break;
	default:
		WARNING("%s:%d: Value cannot be determined "
			"for constraint check for %s",
			arg->expr->module->source_file_name,
			arg->expr->_lineno,
			arg->expr->Identifier
		);
		OUT_NOINDENT(
			"#error %s:%d: Value of %s cannot be determined\n",
			arg->expr->module->source_file_name,
			arg->expr->_lineno,
			arg->expr->Identifier
		);
		break;
	}

	return 0;
}

static asn1p_expr_type_e
_find_terminal_type(arg_t *arg) {
	asn1p_expr_t *expr;
	expr = asn1f_find_terminal_type_ex(arg->asn, arg->ns, arg->expr);
	if(expr) return expr->expr_type;
	return A1TC_INVALID;
}

static int
native_long_sign(arg_t *arg, asn1cnst_range_t *r) {
    if(!(arg->flags & A1C_USE_WIDE_TYPES) && r->left.type == ARE_VALUE
       && r->left.value >= 0 && r->left.value <= 2147483647
        && r->right.type == ARE_MAX) {
        return 1;
    }
	if(r->left.type == ARE_VALUE
	&& r->left.value >= 0
	&& r->right.type == ARE_VALUE
	&& r->right.value > 2147483647
	&& r->right.value <= (asn1c_integer_t)(4294967295UL)) {
		if(r->el_count == 0
		&& r->left.value == 0
		&& r->right.value == 4294967295UL)
			return 0;
		else
			return 1;
	} else {
		return -1;
	}
}
