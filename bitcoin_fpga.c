#include "bitcoin_fpga.h"
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <openssl/sha.h>

static int m_dev_mem_fd = -1;
static uint8_t *m_reg_base = NULL;

static const uint32_t REG_FIRST_BLOCK_HASH = 0x00;
static const uint32_t REG_SECOND_BLOCK = 0x20;
static const uint32_t REG_TARGET = 0x30;
static const uint32_t REG_MAX_NONCE = 0x50;
static const uint32_t REG_CTRL_STATUS = 0x54;
static const uint32_t REG_NONCE = 0x58;

static const uint32_t CTRL_STATUS_RUNNING = 0x01;
static const uint32_t CTRL_STATUS_FOUND = 0x02;

static void write_reg(uint32_t offset, uint32_t value)
{
  *(volatile uint32_t*)(m_reg_base + offset) = value;
}

static uint32_t read_reg(uint32_t offset)
{
  return *(volatile uint32_t*)(m_reg_base + offset);
}

static void write_words(uint32_t offset, const uint32_t *words, size_t count)
{
  const uint32_t *src = words;
  volatile uint32_t *dst = (volatile uint32_t*)(m_reg_base + offset);
  for (size_t i = 0; i < count; i++) {
      *dst = htonl(*src);
      dst++;
      src++;
    }
}

void sha256_calc_h(const void* src, size_t length, uint32_t *h) {
  SHA256_CTX c;
  SHA256_Init(&c);
  SHA256_Update(&c, src, length);
  for (size_t i = 0; i < 8; i++) h[i] = htonl(c.h[i]);
}

BITCOIN_FPGA_RESULT bitcoin_fpga_initialize(uint32_t phys_addr)
{
  if (m_dev_mem_fd >= 0) {
    return BITCOIN_FPGA_OK;
  }
  m_dev_mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (m_dev_mem_fd < 0) {
    return BITCOIN_FPGA_ERR;
  }
  m_reg_base = mmap(NULL, 0x10000, PROT_READ | PROT_WRITE, MAP_SHARED, m_dev_mem_fd, phys_addr);
  if (m_reg_base == NULL) {
    close(m_dev_mem_fd);
    m_dev_mem_fd = -1;
    return BITCOIN_FPGA_ERR;
  }
  return BITCOIN_FPGA_OK;
}

void wait_stop(void)
{
  while(1) {
    uint32_t status = read_reg(REG_CTRL_STATUS);
    if (!(status & CTRL_STATUS_RUNNING)) {
      break;
    }
    // printf("Nonce: %08xh\n", read_reg(REG_NONCE));
  }
}

BITCOIN_FPGA_RESULT bitcoin_fpga_mining(const void *block_header,
                                        const void *target,
                                        uint32_t max_nonce,
                                        uint32_t *nonce)
{
  uint32_t first_block_hash[8];

  sha256_calc_h(block_header, 64, first_block_hash);

  write_words(REG_FIRST_BLOCK_HASH, first_block_hash, 8);
  write_words(REG_SECOND_BLOCK, ((uint32_t*)block_header) + 64/4, 4);
  write_words(REG_TARGET, target, 8);
  write_reg(REG_MAX_NONCE, max_nonce);

  write_reg(REG_CTRL_STATUS, CTRL_STATUS_RUNNING);
  write_reg(REG_CTRL_STATUS, 0);

  wait_stop();

  *nonce = read_reg(REG_NONCE);
  if (read_reg(REG_CTRL_STATUS) & CTRL_STATUS_FOUND) {
    return BITCOIN_FPGA_FOUND;
  }
  return BITCOIN_FPGA_OK;
}

#if 0
int main(void)
{
  uint8_t block_header[80];
  uint8_t target[32] = {  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x21,0x03,0xe1,0x19,0xdf,0x59,0x1d,
  0x66,0x79,0x26,0x50,0xfc,0xb4,0x33,0x4f,
  0x1f,0x6d,0xd1,0x56,0x41,0x26,0x17,0x30};
  uint32_t max_nonce = 0xFFFFFFFF;
  uint32_t nonce = 0;

  FILE *fp = fopen("block_header.bin", "rb");
  fread(block_header, 1, 80, fp);
  fclose(fp);

  ((uint32_t*)block_header)[19] = 0; // nonce = 0

  bitcoin_fpga_initialize(0x43C00000);

  BITCOIN_FPGA_RESULT result = 	(block_header, target, max_nonce, &nonce);
  printf("Result: %d\n", result);
  printf("Nonce: 0x%08x\n", nonce);
}
#endif
