#ifndef ACPI_H_
#define ACPI_H_

#include <stdint.h>

#pragma pack(push, 1)

typedef struct {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  uint8_t oemid[6];
  uint8_t oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} DescriptionTableHeader;

typedef struct {
  // signature: RSDT
  DescriptionTableHeader header;
  const DescriptionTableHeader* entry[]; // size according to the header `length` field
} RSDT_t;

typedef struct {
  char signature[8];
  uint8_t checksum;
  uint8_t oemid[6];
  uint8_t revision;
  RSDT_t* RSDT;
} RSDP_t;

typedef struct {
  char signature[8];
  uint8_t checksum;
  uint8_t oemid[6];
  uint8_t revision;
  RSDT_t* RSDT;

  uint32_t length;
  void* XSDT;
  uint8_t xchecksum;
  uint8_t reserved[3];
} XSDP_t;

typedef struct {
  // signature: FACP
  DescriptionTableHeader header;

  uint32_t firmware_ctrl;
  uint32_t dsdt;

  uint8_t int_model;
  uint8_t preferred_pm_profile;

  uint16_t sci_int;
  uint32_t smi_cmd;

  uint8_t acpi_enable;
  uint8_t acpi_disable;

  uint8_t s4bios_req;
  uint8_t pstate_cnt;

  uint32_t pm1a_evt_blk;
  uint32_t pm1b_evt_blk;

  uint32_t pm1a_cnt_blk;
  uint32_t pm1b_cnt_blk;

  uint32_t pm2_cnt_blk;

  uint32_t pm_tmr_blk;

  uint32_t gpe0_blk;
  uint32_t gpe1_blk;

  uint8_t pm1_evt_len;
  uint8_t pm1_cnt_len;
  uint8_t pm2_cnt_len;

  uint8_t pm_tm_len;

  uint8_t gpe0_blk_len;
  uint8_t gpe1_blk_len;

  uint8_t gpe1_base;

  uint8_t cst_cnt;

  uint16_t p_lvl2_lat;
  uint16_t p_lvl3_lat;

  uint16_t flush_size;
  uint16_t flush_stride;

  uint8_t duty_offset;
  uint8_t duty_width;

  uint8_t day_alrm;
  uint8_t mon_alrm;
  uint8_t century;

  uint16_t iapc_boot_arch;
  uint8_t reserved;

  uint32_t flags;
} FADT_t;

#pragma pack(pop)

extern const RSDP_t* RSDP;
extern const RSDT_t* RSDT;
extern const FADT_t* FADT;

void init_acpi(const void* EBDA_start);

#endif
