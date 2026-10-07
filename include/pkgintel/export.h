#ifndef PKGINTEL_EXPORT_H
#define PKGINTEL_EXPORT_H

#if defined(_WIN32) && defined(PKGINTEL_BUILDING_LIBRARY)
#define PKGINTEL_API __declspec(dllexport)
#elif defined(_WIN32)
#define PKGINTEL_API __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#define PKGINTEL_API __attribute__((visibility("default")))
#else
#define PKGINTEL_API
#endif

#endif
