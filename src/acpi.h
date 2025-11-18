#ifndef ACPI_H_
#define ACPI_H_

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
  char signature[8];
  uint8_t checksum;
  uint8_t oemid[6];
  uint8_t revision;
  void* RSDT;
} RSDP_t;

typedef struct {
  char signature[8];
  uint8_t checksum;
  uint8_t oemid[6];
  uint8_t revision;
  void* RSDT_address;

  uint32_t length;
  void* XSDT;
  uint8_t xchecksum;
  uint8_t reserved[3];
} XSDP_t;

#pragma pack(pop)

void init_acpi(const void* EBDA_start);

#endif
