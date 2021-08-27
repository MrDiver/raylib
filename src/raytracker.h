
#if !defined(RAYTRACKER_H)
#define RAYTRACKER_H

#define RTRACK_API

#include <dlfcn.h> // For finding malloc and so on

#include <stdio.h> // for fprintf
#include <libgen.h>
#include <stdbool.h> // for using booleans

#ifndef RTLD_NEXT
#define RTLD_NEXT ((void *)-1)
#endif

#define PRINTFUNCTION(format, ...)      fprintf(stderr, format, __VA_ARGS__)

#define MEM_TAG "[MEM %s:%d]\t"
#define EXTERN_TAG "[MEM extern]\t\t"

// #define ALWAYS_INTERN

#ifndef DISABLE_EXTERN
#define EXTERN_PRINT 1
#else
#define EXTERN_PRINT 0
#endif

#ifndef DISABLE_LOG
    #define TRACK_LOG_INTERN(file,line,message, args...) PRINTFUNCTION(MEM_TAG message, file, line, ## args)
    #define TRACK_LOG_EXTERN(message, args...) if(EXTERN_PRINT&&print_enabled) PRINTFUNCTION(EXTERN_TAG message, ## args)
#else
    #define TRACK_LOG_INTERN(file,message, args...) //
    #define TRACK_LOG_EXTERN(message, args...) //
#endif

#if defined(ALWAYS_INTERN)
    #define EXEC_INTERN(func) func;
#else
    #define EXEC_INTERN(func) print_enabled = false; func; print_enabled = true;
#endif

//GLOBALS INTERN
static bool print_enabled = true;
static int current_alloc = 0;
static unsigned char buffer[8192];
static bool searching = false;
//TYPES
typedef void* (*MallocFn)(size_t size);
typedef void* (*CallocFn)(size_t n, size_t size);
typedef void* (*ReallocFn)(void* ptr, size_t size);
typedef void  (*FreeFn)(void* ptr);
//FN TYPES
static MallocFn malloc_ptr;
static CallocFn calloc_ptr;
static ReallocFn realloc_ptr;
static FreeFn free_ptr;

// Memory Interposition
void *malloc(size_t size)
{
    if(malloc_ptr == NULL)
        malloc_ptr = (MallocFn)dlsym(RTLD_NEXT, "malloc");

    void* p = malloc_ptr(size);
    TRACK_LOG_EXTERN("MALLOC \t %p : %zu \n",p,size);
    return p;
}

void *calloc(size_t n, size_t size)
{
    if(searching) // The case when calloc is called in dlsym
    {
        TRACK_LOG_EXTERN("CALLOC \t BUFFER %p: %zu \n",buffer,size);
        return buffer; // return a temporary buffer for dlsym
    }
    if(calloc_ptr == NULL){
        searching = true;
        calloc_ptr = (CallocFn)dlsym(RTLD_NEXT, "calloc");
        searching = false;
    }

    void* p = calloc_ptr(n, size);
    TRACK_LOG_EXTERN("CALLOC \t %p : %zu \n",p,size);
    return p;
}

void* realloc(void* ptr, size_t size){
    if(realloc_ptr == NULL)
        realloc_ptr = (ReallocFn)dlsym(RTLD_NEXT, "realloc");

    if(ptr == buffer){
        TRACK_LOG_EXTERN("REALLOC BUFFER\t %p : %zu \n",ptr,size);
        return ptr;
    }

    void* p = realloc_ptr(ptr, size);
    TRACK_LOG_EXTERN("REALLOC\t %p : %zu \n",p,size);
    return p;
}

void free(void* ptr){
    if(free_ptr == NULL)
        free_ptr = (FreeFn)dlsym(RTLD_NEXT, "free");

    if(ptr == buffer)
    {
        TRACK_LOG_EXTERN("FREE BUFFER %p",ptr);
        return;
    }

    free_ptr(ptr);
    TRACK_LOG_EXTERN("FREE \t %p\n",ptr);
}

// Raylib Trackers

const char* rootname(const char* filename){
    unsigned int lastSlash = 0;
    for(int i = 0; filename[i] != '\0'; i++){
        if(filename[i] == '/')
            lastSlash = i+1;
    }
    if(lastSlash==0)
        return filename;
    return filename+lastSlash;
}

void* tracker_malloc(const char* file, int line, size_t size){
    EXEC_INTERN(void* p = malloc(size));
    TRACK_LOG_INTERN(rootname(file),line,"RL_MALLOC \t%p : %zu\n",p,size);
    return p;
}

void* tracker_calloc(const char* file, int line,size_t n, size_t size){
    EXEC_INTERN(void *p = calloc(n, size));
    TRACK_LOG_INTERN(rootname(file),line,"RL_CALLOC \t%p : %zu\n",p,size);
    return p;
}

void* tracker_realloc(const char* file, int line,void* ptr, size_t size){
    EXEC_INTERN(void* p = realloc(ptr, size));
    TRACK_LOG_INTERN(rootname(file),line,"RL_REALLOC \t%p to %p : %zu\n",ptr,p,size);
    return p;
}

void tracker_free(const char* file, int line,void* ptr){
    EXEC_INTERN(free(ptr));
    TRACK_LOG_INTERN(rootname(file),line,"RL_FREE    \t%p\n",ptr);
}

// in raylib.h
// #define RL_MALLOC(sz)       tracker_malloc(__FILE__,__LINE__,sz)
// #define RL_CALLOC(n,sz)     tracker_calloc(__FILE__,__LINE__,n,sz)
// #define RL_REALLOC(ptr,sz)  tracker_realloc(__FILE__,__LINE__,ptr,sz)
// #define RL_FREE(ptr)        tracker_free(__FILE__,__LINE__,ptr)

#endif


