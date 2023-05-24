#pragma once

// Unsigned int types
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef int b32;
typedef char b8;

// Ensure all types are of correct size
static_assert(sizeof(u8) == 1, "Expected u8 to be 1 byte");
static_assert(sizeof(u16) == 2, "Expected u16 to be 2 bytes");
static_assert(sizeof(u32) == 4, "Expected u32 to be 4 bytes");
static_assert(sizeof(u64) == 8, "Expected u64 to be 8 bytes");

static_assert(sizeof(i8) == 1, "Expected i8 to be 1 byte");
static_assert(sizeof(i16) == 2, "Expected i16 to be 2 bytes");
static_assert(sizeof(i32) == 4, "Expected i32 to be 4 bytes");
static_assert(sizeof(i64) == 8, "Expected i64 to be 8 bytes");

static_assert(sizeof(f32) == 4, "Expected f32 to be 4 bytes");
static_assert(sizeof(f64) == 8, "Expected f64 to be 8 bytes");

static_assert(sizeof(b32) == 4, "Expected b32 to be 4 bytes");
static_assert(sizeof(b8) == 1, "Expected b8 to be 1 bytes");

#define TRUE 1
#define FALSE 0
#define VCLAMP(value,min,max) (value <= min) ? min : (value >= max) ? max : value;

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define R_PLATFORM_WINDOWS 1
#ifndef _WIN64 // x64 architecture we do not build for x86
#error "64-bit is required on Windows"
#endif
#elif defined(__linux__) || defined(__gnu_linux__)
#define R_PLATFORM_LINUX 1
#if defined(__ANDROID__)
#define R_PLATFORM_ANDROID 1
#endif
#elif defined(__unix__)
#define R_PLATFORM_POSIX 1
#elif __APPLE__
#define R_PLATFORM_APPLE 1
#endif

// Export control
#ifdef VNEXPORT
#ifdef _MSC_VER
#define VAPI __declspec(dllexport)
#else
#define VAPI __attribute__((visibility("default")))
#endif
#else
// Import control
#ifdef _MSC_VER
#define VAPI __declspec(dllimport)
#else
#define VAPI 
#endif
#endif