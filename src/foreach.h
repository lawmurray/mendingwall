#pragma once

#define foreach_with_iterator(value, values, iterator) \
    typeof(values) iterator = values; \
    for (typeof(*iterator) value = *iterator; value; value = *++iterator)
#define foreach_iterator(line) \
    iter_ ## line ## _
#define foreach_with_line(value, values, line) \
    foreach_with_iterator(value, values, foreach_iterator(line))

/**
 * A tidier way to loop over null-terminated arrays of pointers.
 * 
 * ```c
 * foreach(value, values) {
 *   //
 * }
 * ```
 */
#define foreach(value, values) \
    foreach_with_line(value, values, __LINE__)
