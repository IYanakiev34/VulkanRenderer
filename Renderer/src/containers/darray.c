#include "darray.h"
#include "core/vmemory.h"
#include "core/logger.h"

void* _darray_create(u64 length, u64 stride) {
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 array_size = length * stride;
    u64* new_array = vallocate(header_size + array_size, MEMORY_TAG_DARRAY);
    new_array[DARRAY_CAPACITY] = length;
    new_array[DARRAY_LENGTH] = 0;
    new_array[DARRAY_STRIDE] = stride;

    return (void*)(new_array + DARRAY_FIELD_LENGTH);

}

void _darray_destroy(void* array) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
    u64 total_size = header_size + header[DARRAY_CAPACITY] * header[DARRAY_STRIDE];
    vfree(header, total_size, MEMORY_TAG_DARRAY);
}

u64 _darray_field_get(void* array, u64 field) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    return header[field];
}

void _darray_field_set(void* array, u64 field, u64 value) {
    u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
    header[field] = value;
}

void* _darray_resize(void* array) {
    // Get properties
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    // Reallocate array
    void* temp = _darray_create(DARRAY_RESIZE_FACTOR * darray_capacity(array), stride);
    vcopy_memory(temp, array, length * stride);
    _darray_field_set(temp, DARRAY_LENGTH, length);
    // Free previous array
    _darray_destroy(array);

    return temp;
}

void* _darray_push(void* array, const void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    if (length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 address = (u64)array;
    address += length * stride;
    vcopy_memory((void *)address, value_ptr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}

void _darray_pop(void* array, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    u64 address = (u64)array;
    address += (length - 1) * stride;
    vcopy_memory(dest, (void*)address, stride);
    _darray_field_set(array, DARRAY_LENGTH, length - 1);
}

void* _darray_insert_at(void* array, u64 index, const void* value_ptr) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);

    if (index >= length) {
        VERROR("Index outside of the bound of this array! Length: %i, index: %i", length, index);
        return array;
    }

    if (length >= darray_capacity(array)) {
        array = _darray_resize(array);
    }

    u64 address = (u64)array;

    if (index != length - 1) {
        vcopy_memory((void*)(address + (index + 1) * stride),
            (void*)(address + index * stride),
            stride * (length - index));
    }

    vcopy_memory((void*)(address + index * stride), value_ptr, stride);
    _darray_field_set(array, DARRAY_LENGTH, length + 1);
    return array;
}

void* _darray_pop_at(void* array, u64 index, void* dest) {
    u64 length = darray_length(array);
    u64 stride = darray_stride(array);
    
    if (index >= length)
    {
        VERROR("Index outside of the bound of this array! Length: %i, index: %i", length, index);
        return array;
    }
    
    u64 address = (u64)array;
    vcopy_memory(dest, (void*)(address + index * stride), stride);

    // Move elements from index + 1 inward
    // if 1, 2, 3, 4, 5 and we want to remove 3 -> 1, 2, 4, 5 (4, 5) override memory of 3
    if (index != length - 1) {
        vcopy_memory((void*)(address + (stride * index)),
            (void*)(address + (stride * (index + 1))),
            stride * (length - index));
    }

    _darray_field_set(array, DARRAY_LENGTH, length - 1);
    return array;
}