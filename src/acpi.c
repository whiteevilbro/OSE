#include "acpi.h"

#include "memmgnt.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define EBDA_end 0xA0000

#define BIOS_ROM_START 0x0E0000
#define BIOS_ROM_END 0x0FFFFF

const RSDP_t* RSDP = NULL;
const RSDT_t* RSDT = NULL;
const FADT_t* FADT = NULL;

static bool validate_RSDP(const RSDP_t* ptr) {
  if (memcmp(ptr->signature, "RSD PTR ", 8)) {
    return false;
  }
  uint8_t checksum = 0;
  for (size_t i = 0; i < sizeof(RSDP_t); i++) {
    checksum += ((const uint8_t*) ptr)[i];
  }
  return !checksum;
}

static inline const RSDP_t* find_RSDP(const void* EBDA_start) {
  size_t ebda_p = (size_t) EBDA_start;

  ebda_p = (ebda_p + 15) & (size_t) ~0xf;
  while (ebda_p < 0xA0000) {
    if (validate_RSDP((const RSDP_t*) ebda_p)) {
      return (const RSDP_t*) ebda_p;
    }
    ebda_p += 0x10;
  }

  ebda_p = (BIOS_ROM_START + 15) & (size_t) ~0xf;
  while (ebda_p < BIOS_ROM_END) {
    if (validate_RSDP((const RSDP_t*) ebda_p)) {
      return (const RSDP_t*) ebda_p;
    }
    ebda_p += 0x10;
  }

  return NULL;
}

static bool validate_checksum(const DescriptionTableHeader* table) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < table->length; i++) {
    checksum += ((const uint8_t*) table)[i];
  }
  return !checksum;
}

void init_acpi(const void* EBDA_start) {
  // find valid RSDP
  RSDP = find_RSDP(EBDA_start);

  if (validate_checksum(&RSDP->RSDT->header))
    RSDT = RSDP->RSDT;

  if (!RSDT)
    return;

  for (size_t i = 0; i < (RSDT->header.length - sizeof(DescriptionTableHeader)) / sizeof(DescriptionTableHeader*); i++) {
    if (!FADT && !memcmp(RSDT->entry[i]->signature, "FACP", 4)) {
      if (validate_checksum(RSDT->entry[i])) {
        FADT = (const FADT_t*) RSDT->entry[i];
      }
    }
  }
}
