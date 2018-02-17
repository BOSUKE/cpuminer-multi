#ifndef BITCOIN_FPGA_H_
#define BITCOIN_FPGA_H_
#include <stdint.h>

typedef enum {
  BITCOIN_FPGA_OK = 0,
  BITCOIN_FPGA_ERR = -1,
  BITCOIN_FPGA_FOUND = 1,
} BITCOIN_FPGA_RESULT;

BITCOIN_FPGA_RESULT bitcoin_fpga_initialize(uint32_t phys_addr);
BITCOIN_FPGA_RESULT bitcoin_fpga_mining(const void *block_header,
                                        const void *target,
                                        uint32_t max_nonce,
                                        uint32_t *nonce);

#endif
