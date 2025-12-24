#ifndef PAGING_H_
#define PAGING_H_

#include "interrupts.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#pragma pack(push, 1)

// C99 allows read/write operations in different union fields with compatible types
typedef union {
  struct {
    bool present         : 1;
    bool writable        : 1;
    bool user            : 1;
    bool write_through   : 1;
    bool disable_caching : 1;
    bool accessed        : 1;
    bool dirty           : 1;
    bool pat             : 1;
    bool global          : 1;
    uint8_t available    : 3;
    size_t address       : 20;
  } mapped;

  struct {
    bool present       : 1;
    uint32_t available : 31;
  } unmapped;
} PageTableEntry;

typedef union {
  struct {
    bool present         : 1;
    bool writable        : 1;
    bool user            : 1;
    bool write_through   : 1;
    bool disable_caching : 1;
    bool accessed        : 1;
    bool available0      : 1;
    bool page_size       : 1;
    uint8_t available1   : 4;
    size_t address       : 20;
  } directory;

  struct {
    bool present         : 1;
    bool writable        : 1;
    bool user            : 1;
    bool write_through   : 1;
    bool disable_caching : 1;
    bool accessed        : 1;
    bool dirty           : 1;
    bool page_size       : 1;
    bool global          : 1;
    uint8_t available    : 3;
    bool pat             : 1;
    size_t reserved      : 9;
    size_t address       : 10;
  } page;

  struct {
    bool present       : 1;
    uint32_t available : 31;
  } unmapped;
} PageDirectoryEntry;

#pragma pack(pop)

extern void enable_paging(void);
extern void disable_paging(void);
extern void set_cr3(void* cr3);


PageDirectoryEntry* create_VAS(void);
void free_VAS(PageDirectoryEntry* pdt);
void setup_kernel_paging(void);

void pagefault_handler(const Context* const ctx);

#endif
