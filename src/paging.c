#include "paging.h"

#include "assert.h"
#include "interrupts.h"
#include "memmgnt.h"
#include "processes/process.h"
#include "processes/scheduler/scheduler.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PTEs_BASE 0xD0000000
#define PDT 0xD0400000

static PageDirectoryEntry kernel_pde;

void setup_kernel_paging(void) {
  kernel_pde = (PageDirectoryEntry){
      .page = {
          .present   = true,
          .writable  = true,
          .user      = false,
          .page_size = true,
          .address   = 0x0,
      },
  };
}

bool is_mounted(PageDirectoryEntry* pdt, void* virtual) {
  return pdt[(size_t) virtual >> 22].unmapped.present &&
         (pdt[(size_t) virtual >> 22].page.page_size ||
          ((PageTableEntry*) ((size_t) pdt[(size_t) virtual >> 22].directory.address << 12))[((size_t) virtual >> 12) & 0x3ff].mapped.present);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"

void mount_page(PageDirectoryEntry* pdt, void* virtual, void* physical, bool writable, bool user, bool freeable) {
  if (!pdt[(size_t) virtual >> 22].unmapped.present) {
    pdt[(size_t) virtual >> 22] = (PageDirectoryEntry){
        .directory = {
            .present  = true,
            .writable = true,
            .user     = true,
            .address  = (size_t) calloc_page() >> 12,
        }};
  }

  if (!pdt[(size_t) virtual >> 22].directory.page_size) {
    PageTableEntry* pt                   = (PageTableEntry*) ((size_t) pdt[(size_t) virtual >> 22].directory.address << 12);
    pt[((size_t) virtual >> 12) & 0x3ff] = (PageTableEntry){
        .mapped = {
            .present            = true,
            .writable           = writable,
            .user               = user,
            .available_freeable = freeable,
            .address            = (uint32_t) physical >> 12,
        }};
  }
}

#pragma GCC diagnostic pop

PageDirectoryEntry* create_VAS(void) {
  PageDirectoryEntry* pdt = calloc_page();

  pdt[0] = kernel_pde;


  mount_page(pdt, (void*) 0x7ff000, malloc_page(), true, true, true);

  // technically every write to pdt should be followed with write to ptpt (at exact same coordinates)
  // PageTableEntry* page_table_page_table = calloc_page();
  //
  // page_table_page_table[0] = (PageTableEntry){
  //     .mapped = {
  //         .present  = true,
  //         .writable = true,
  //         .user     = false,
  //         .address  = (size_t) kernel_pt >> 12,
  //     }};
  // page_table_page_table[1] = (PageTableEntry){
  //     .mapped = {
  //         .present  = true,
  //         .writable = true,
  //         .user     = false,
  //         .address  = (size_t) stack_pt >> 12,
  //     }};
  // // now it's getting freaky
  // page_table_page_table[PTEs_BASE >> 22] = (PageTableEntry){
  //     .mapped = {
  //         .present  = true,
  //         .writable = true,
  //         .user     = false,
  //         .address  = (size_t) page_table_page_table >> 12,
  //     }};
  //
  // pdt[PTEs_BASE >> 22] = (PageDirectoryEntry){
  //     .directory = {
  //         .present  = true,
  //         .writable = true,
  //         .user     = false,
  //         .address  = (size_t) page_table_page_table >> 12,
  //     }};
  //
  // PageTableEntry* page_directory_page_table = calloc_page();
  //
  // page_directory_page_table[0] = (PageTableEntry){
  //     .mapped = {
  //         .present  = true,
  //         .writable = true,
  //         .user     = false,
  //         .address  = (size_t) pdt >> 12,
  //     }};
  //
  // pdt[PDT >> 22] = (PageDirectoryEntry){
  //     .directory = {
  //         .present  = true,
  //         .writable = true,
  //         .user     = false,
  //         .address  = (size_t) page_directory_page_table >> 12,
  //     }};
  // page_table_page_table[PDT >> 22] = (PageTableEntry){
  //     .mapped = {
  //         .present  = true,
  //         .writable = true,
  //         .user     = false,
  //         .address  = (size_t) page_directory_page_table >> 12,
  //     }};

  return pdt;
}

static void free_pagetable(PageTableEntry* pt) {
  for (size_t i = 0; i < 1024; i++) {
    if (pt[i].mapped.present && pt[i].mapped.available_freeable) {
      free_page((void*) ((size_t) pt[i].mapped.address << 12));
    }
  }
}

void free_VAS(PageDirectoryEntry* pdt) {
  for (size_t i = 1; i < 1024; i++) {
    if (pdt[i].directory.present) {
      if (!pdt[i].directory.page_size) {
        PageTableEntry* pt = (PageTableEntry*) ((size_t) pdt[i].directory.address << 12);
        free_pagetable(pt);
        free_page(pt);
      }
    }
  }
  free_page(pdt);
}

typedef enum {
  PRESENT_FLAG        = 1 << 0,
  WRITE_FLAG          = 1 << 1,
  USER_FLAG           = 1 << 2,
  RESERVED_BIT_FLAG   = 1 << 3,
  INSTRUCTION_FLAG    = 1 << 4,
  PROTECTION_KEY_FLAG = 1 << 5,
  SHADOW_STACK_FLAG   = 1 << 6,
  HLAT_FLAG           = 1 << 7,
  SGX_FLAG            = 1 << 8,
} PageFaultErrorCode;

extern void collect_cr(CRContext* crctx);

void pagefault_handler(const Context* const ctx) {
  CRContext crctx;
  collect_cr(&crctx);
  if (!(ctx->error_code & USER_FLAG)) {
    kernel_panic(PANIC_MESSAGE "\n" ERROR_CODE_MESSAGE CR_MESSAGE, UNPACK_CONTEXT(ctx), ctx->error_code, UNPACK_CR(crctx));
  }

  disable_paging();
  Process* process = scheduler_current_process();

  if (crctx.cr2 < 0x200000) {
    // NULL
    cprintf(&process->console, "Process NPEd\n");
  } else if (0x200000 <= crctx.cr2 && crctx.cr2 < 0x400000) {
    // Stack Overflow
    cprintf(&process->console, "Process SOEd\n");
  } else if (0x400000 <= crctx.cr2 && crctx.cr2 < 0x800000) {
    for (size_t i = (size_t) process->lowest_stack_page; i > crctx.cr2;) {
      i -= 0x1000;
      if (!is_mounted(process->pdt, (void*) i)) {
        mount_page(process->pdt, (void*) (i & (size_t) ~0xfff), malloc_page(), true, true, true);
      }
      process->lowest_stack_page = (void*) i;
    }
    enable_paging();
    return;
  } else {
    // UB
    cprintf(&process->console, "Process UBd\n");
  }
  kill_process(process);
  switch_process();
}
