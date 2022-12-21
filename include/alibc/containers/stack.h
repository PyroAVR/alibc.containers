#pragma once
#include <alibc/containers/dynabuf.h>

#include <stddef.h>
#include <stdbool.h>

typedef struct _stack_impl stack_t;

/**
 * Allocate memory for, and initialize a stack datastructure.
 */
stack_t *create_stack(size_t size, int unit);

/**
 * Initialize a stack with pre-allocated memory at self.
 */
stack_t *init_stack(stack_t *self, size_t size, int unit);

/**
 * Push a value onto the stack
 */
bool stack_push(stack_t *self, void *value);

/**
 * Pop a value from the stack
 */
void **stack_pop(stack_t *self);

/**
 * Duplicate the top value on the stack
 */
int stack_dup(stack_t *self);

/**
 * Duplicate the top two values on the stack
 */
int stack_dup2(stack_t *self);

/**
 * Rotate the top three values on the stack
 */
int stack_rot(stack_t *self);

/**
 * Drop the top value from the stack
 */
int stack_drop(stack_t *self);

/**
 * Drop the top two values from the stack
 */
int stack_drop2(stack_t *self);

/**
 * Swap the top two values on the stack
 */
int stack_swap(stack_t *self);

/**
 * Swap the top two pairs of values on the stack
 */
int stack_swap2(stack_t *self);

/**
 * Get the size of the stack, in elements
 */
size_t stack_size(stack_t *self);

/**
 * Get the capacity of the stack in elements. This value may change based on the
 * excess space allocated for the stack when applicable.
 */
size_t stack_capacity(stack_t *self);

/**
 * Free memory associated with the stack
 */
void stack_free(stack_t *self);

/**
 * Obtain status information about the most recent stack operation
 */
int stack_status(stack_t *self);
