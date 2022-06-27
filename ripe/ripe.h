
#ifndef SHELLCODE_H
#define SHELLCODE_H

#include <stdbool.h>

#define POINTER_LEN 4
#define SHELLCODE_LEN 6
#define PADDING_LEN 14

struct payload_ret_t
{
    char shellcode[SHELLCODE_LEN];
    char padding[PADDING_LEN];
    struct payload_t *address;
};

struct payload_struct_t
{
    char shellcode[SHELLCODE_LEN];
    struct payload_t *address;
};

struct payload_fun_t
{
    void *address;
};

enum param_shellcode_t
{
    NONOP,
    NOP,
    SHELLCODE_NUM
};

enum param_location_t
{
    STACK,
    BSS,
    DATA,
    LOCATION_NUM
};

enum param_code_pointer_t
{
    RETURN_ADDRESS,
    STRUCT,
    FUNCTION_POINTER,
    POINTER_NUM
};

enum param_function_abused_t
{
    MEMCPY,
    STRCPY,
    STRNCPY,
    MEMMOVE,
    HOMEBREW,
    FUNCTION_NUM
};

struct param_t
{
    enum param_shellcode_t shellcode;
    enum param_location_t location;
    enum param_code_pointer_t pointer;
    enum param_function_abused_t function;
};

// -------------------------------------------------------
// GETTER
// -------------------------------------------------------

int get_payload_len(struct param_t *attack_params);

// -------------------------------------------------------
// ATTACK DOMAIN SPACE EXPLORATION
// -------------------------------------------------------

/**
 * @brief Given a progressive integer number, sets the parameters structure
 *
 * As an example, passing n as attack number, it sets:
 * - 0 -> NONOP, STACK, RETURN_ADDRESS, MEMCPY
 * - 1 -> NOP, STACK, RETURN_ADDRESS, MEMCPY
 * - 2 -> NONOP, BSS, RETURN_ADDRESS, MEMCPY
 * ...
 *
 * @param number The attack progressive number
 * @param attack_params The structure hosting the attack params
 */

void number_to_attack_params(int number, struct param_t *attack_params);

/**
 * @brief Exclude non-possible attacks due to conflicting option
 *
 * @param attack_params The structure hosting the attack params
 */

bool attack_possible(struct param_t *attak_params);

// -------------------------------------------------------
// VULNERABLE FUNCTIONS / TYPEDEFS / DUMMY DATA
// -------------------------------------------------------

/**
 * @brief A loop-based equivalent of memcpy
 */
void homebrew(char *dest, char *src, unsigned int len);

#define DUMMY_CHAR 'A'
#define DUMMY_BUFF "ABCDEF"
#define DUMMY_STRUCT                                 \
    {                                                \
        .buffer = DUMMY_BUFF, .fun_point = dummy_fun \
    }

/**
 * @brief Just a placeholder function
 *
 * Needed to initialize variables (and let them be placed in .data)
 *
 */
void dummy_fun();

/**
 * @brief A function that contains a sensitive instruction
 *
 * The hijacked pointer to function calls this function or part of it
 *
 */
void fake_fun();

#define FAKE_FUN_SVC_OFFSET 40

/**
 * @brief Fill the provided buffer 'buffer' using DUMMY_CHAR for 'n' bytes
 *
 * @param buffer The buffer to be filled
 * @param n The number of bytes written
 */
void fill_padding(char *buffer, int n);

typedef char vulnerable_buffer_t[SHELLCODE_LEN];

typedef void (*vulnerable_function_t)();

typedef struct
{
    char buffer[SHELLCODE_LEN];
    void (*fun_point)();
} vulnerable_struct_t;

// -------------------------------------------------------
// BENCHMARK
// -------------------------------------------------------

void attack(struct param_t *attack_params);
void build_payload(struct param_t *attack_params, char *payload, int *target_location);

#endif