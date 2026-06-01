/*
Copyright (c) 2012-2015 Ben Croston

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include "cpuinfo.h"
#include "c_gpio.h"
#include "bpi_gpio.h"

#define BCM2708_PERI_BASE_DEFAULT   0x20000000
#define BCM2709_PERI_BASE_DEFAULT   0x3f000000
#define GPIO_BASE_OFFSET            0x200000
#define FSEL_OFFSET                 0   // 0x0000
#define SET_OFFSET                  7   // 0x001c / 4
#define CLR_OFFSET                  10  // 0x0028 / 4
#define PINLEVEL_OFFSET             13  // 0x0034 / 4
#define EVENT_DETECT_OFFSET         16  // 0x0040 / 4
#define RISING_ED_OFFSET            19  // 0x004c / 4
#define FALLING_ED_OFFSET           22  // 0x0058 / 4
#define HIGH_DETECT_OFFSET          25  // 0x0064 / 4
#define LOW_DETECT_OFFSET           28  // 0x0070 / 4
#define PULLUPDN_OFFSET             37  // 0x0094 / 4
#define PULLUPDNCLK_OFFSET          38  // 0x0098 / 4

#define PAGE_SIZE  (4*1024)
#define BLOCK_SIZE (4*1024)

static volatile uint32_t *gpio_map;

#ifndef BPI
#define BPI
#endif


#ifdef BPI
#define BPI_MODEL_MIN        64
#define BPI_MODEL_M1         64
#define BPI_MODEL_M1P        65
#define BPI_MODEL_R1         66
#define BPI_MODEL_M2         67
#define BPI_MODEL_M3         68
#define BPI_MODEL_M2P        69
#define BPI_MODEL_M64        70
#define BPI_MODEL_M2U        71
#define BPI_MODEL_M2M        72
#define BPI_MODEL_M2P_H2P    73
#define BPI_MODEL_M2P_H5     74
#define BPI_MODEL_M2U_V40    75
#define BPI_MODEL_M2Z        76
#define BPI_MODEL_R2         77
#define BPI_MODEL_M2M_V11    78
#define BPI_MODELS_MAX       81

#define BPI_MAKER_SINOVOIP    6

#define SUNXI_R_GPIO_BASE	0x01F02000
#define SUNXI_R_GPIO_REG_OFFSET   0xC00
#define SUNXI_GPIO_BASE		0x01C20000
#define SUNXI_GPIO_REG_OFFSET   0x800
#define SUNXI_CFG_OFFSET	0x00
#define SUNXI_DATA_OFFSET	0x10
#define SUNXI_PUD_OFFSET	0x1C
#define SUNXI_BANK_SIZE		0x24

#define MAP_SIZE        (4096*2)
#define MAP_MASK        (MAP_SIZE - 1)

#define MTK_GPIO_BASE_ADDR		0x10005000
#define MTK_GPIO_DIR				0x00
#define MTK_GPIO_PULLE          		0x150
#define MTK_GPIO_PULLSEL			0x280
#define MTK_GPIO_DOUT				0x500
#define MTK_GPIO_DIN				0x630
#define MTK_GPIO_MODE				0x760
#define MTK_GPIO_MAP_SIZE			(8 * 1024)
#define MTK_GPIO_MODE_PINS_PER_REG		5
#define MTK_GPIO_FIELD_PINS_PER_REG		16

typedef struct sunxi_gpio {
    unsigned int CFG[4];
    unsigned int DAT;
    unsigned int DRV[2];
    unsigned int PULL[2];
} sunxi_gpio_t;

/* gpio interrupt control */
typedef struct sunxi_gpio_int {
    unsigned int CFG[3];
    unsigned int CTL;
    unsigned int STA;
    unsigned int DEB;
} sunxi_gpio_int_t;

typedef struct sunxi_gpio_reg {
    struct sunxi_gpio gpio_bank[9];
    unsigned char res[0xbc];
    struct sunxi_gpio_int gpio_int;
} sunxi_gpio_reg_t;

#define GPIO_BANK(pin)  ((pin) >> 5)
#define GPIO_NUM(pin)   ((pin) & 0x1F)

#define GPIO_CFG_INDEX(pin)     (((pin) & 0x1F) >> 3)
#define GPIO_CFG_OFFSET(pin)    ((((pin) & 0x1F) & 0x7) << 2)

#define GPIO_PUL_INDEX(pin)     (((pin) & 0x1F )>> 4) 
#define GPIO_PUL_OFFSET(pin)    (((pin) & 0x0F) << 1)

static volatile uint32_t *pio_map;
static volatile uint32_t *r_pio_map;

int bpi_found=-1;
int bpi_found_mtk = 0;

const int *pinToGpio_BP ;
const int *physToGpio_BP ;
const int *pinTobcm_BP ;


static volatile uint32_t *r_gpio_map;

char *piModelNames [BPI_MODELS_MAX] =
{
  [0] = "Model A",
  [1] = "Model B",
  [2] = "Model A+",
  [3] = "Model B+",
  [4] = "Pi 2",
  [5] = "Alpha",
  [6] = "CM",
  [8] = "Pi 3",
  [9] = "Pi Zero",
  [10] = "CM3",
  [12] = "Pi Zero-W",
  [BPI_MODEL_M1]      = "Banana Pi M1[A20]",
  [BPI_MODEL_M1P]     = "Banana Pi M1+[A20]",
  [BPI_MODEL_R1]      = "Banana Pi R1[A20]",
  [BPI_MODEL_M2]      = "Banana Pi M2[A31s]",
  [BPI_MODEL_M3]      = "Banana Pi M3[A83T]",
  [BPI_MODEL_M2P]     = "Banana Pi M2+[H3]",
  [BPI_MODEL_M64]     = "Banana Pi M64[A64]",
  [BPI_MODEL_M2U]     = "Banana Pi M2 Ultra[R40]",
  [BPI_MODEL_M2M]     = "Banana Pi M2 Magic[R16]",
  [BPI_MODEL_M2P_H2P] = "Banana Pi M2+[H2+]",
  [BPI_MODEL_M2P_H5]  = "Banana Pi M2+[H5]",
  [BPI_MODEL_M2U_V40] = "Banana Pi M2 Ultra[V40]",
  [BPI_MODEL_M2Z]     = "Banana Pi M2 Zero[H2+/H3]",
  [BPI_MODEL_R2]      = "Banana Pi R2[MT7623]",
  [BPI_MODEL_M2M_V11] = "Banana Pi M2 Magic v1.1[R16]",
} ;

char *piRevisionNames [16] =
{
  "00",
  "01",
  "02",
  "03",
  "04",
  "05",
  "06",
  "07",
  "08",
  "09",
  "10",
  "11",
  "12",
  "13",
  "14",
  "15",
} ;

char *piMakerNames [16] =
{
  "Sony",	//	 0
  "Egoman",	//	 1
  "Embest",	//	 2
  "Unknown",	//	 3
  "Embest",	//	 4
  "Stadium",	//	 5
  "BPI-Sinovoip",	//	 6
  "Unknown07",	//	 7
  "Unknown08",	//	 8
  "Unknown09",	//	 9
  "Unknown10",	//	10
  "Unknown11",	//	11
  "Unknown12",	//	12
  "Unknown13",	//	13
  "Unknown14",	//	14
  "Unknown15",	//	15
} ;

int piMemorySize [8] =
{
   256,		//	 0
   512,		//	 1
  1024,		//	 2
#ifdef BPI
  2048,		//	 3
#else
     0,		//	 3
#endif
     0,		//	 4
     0,		//	 5
     0,		//	 6
     0,		//	 7
} ;

struct BPIBoards
{
  const char *name;
  int gpioLayout;
  int model;
  int rev;
  int mem;
  int maker;
  int warranty;
  const int *pinToGpio;
  const int *physToGpio;
  const int *pinTobcm;
} ;

/*
 * Board list
 *********************************************************************************
 */

struct BPIBoards bpiboard [] = 
{
  { "bpi-0",	      -1, 0, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-1",	      -1, 1, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-2",	      -1, 2, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-3",	      -1, 3, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-4",	      -1, 4, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-5",	      -1, 5, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-6",	      -1, 6, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-7",	      -1, 7, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-8",	      -1, 8, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-9",	      -1, 9, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-10",	      -1, 10, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-11",	      -1, 11, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-12",	      -1, 12, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-13",	      -1, 13, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-14",	      -1, 14, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-15",	      -1, 15, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-new",	      -1, 16, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-x86",	      -1, 17, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-rpi",	      -1, 18, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-rpi2",	      -1, 19, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-rpi3",	      -1, 20, 1, 2, 5, 0, NULL, NULL, NULL 	},
  { "bpi-m1",	   10001, BPI_MODEL_M1, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bpi-m1p",	   10001, BPI_MODEL_M1P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bpi-m1-plus", 10001, BPI_MODEL_M1P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bpi-r1",	   10001, BPI_MODEL_R1, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bpi-m2",	   10101, BPI_MODEL_M2, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2, physToGpio_BPI_M2, pinTobcm_BPI_M2 	},
  { "bpi-m3",	   10201, BPI_MODEL_M3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M3, physToGpio_BPI_M3, pinTobcm_BPI_M3 	},
  { "bpi-m2p",	   10301, BPI_MODEL_M2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2-plus", 10301, BPI_MODEL_M2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2-plus-h3", 10301, BPI_MODEL_M2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p-h3", 10301, BPI_MODEL_M2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p-480", 10301, BPI_MODEL_M2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m64",	   10401, BPI_MODEL_M64, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M64, physToGpio_BPI_M64, pinTobcm_BPI_M64 	},
  { "bpi-m2u",	   10501, BPI_MODEL_M2U, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-m2m",	   10601, BPI_MODEL_M2M, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2M, physToGpio_BPI_M2M, pinTobcm_BPI_M2M 	},
  { "bpi-m2m-v1.1", 10601, BPI_MODEL_M2M_V11, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2M_V11, physToGpio_BPI_M2M_V11, pinTobcm_BPI_M2M_V11 	},
  { "bpi-m2m-v11", 10601, BPI_MODEL_M2M_V11, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2M_V11, physToGpio_BPI_M2M_V11, pinTobcm_BPI_M2M_V11 	},
  { "bpi-m2p_H2+", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p-h2+", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p-h2p", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2-plus-h2+", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2-plus-h2p", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p_H5",  10801, BPI_MODEL_M2P_H5, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p-h5",  10801, BPI_MODEL_M2P_H5, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2-plus-h5", 10801, BPI_MODEL_M2P_H5, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2u_V40", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-m2z",	   11001, BPI_MODEL_M2Z, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-r2",      11101, BPI_MODEL_R2, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_R2,  physToGpio_BPI_R2,  pinTobcm_BPI_R2    },
  { NULL,		0, 0, 1, 2, BPI_MAKER_SINOVOIP, 0, NULL, NULL, NULL 	},
} ;



static uint8_t* gpio_mmap_reg = NULL;

static volatile uint32_t *mtk_gpio_reg(unsigned int offset)
{
    return (volatile uint32_t *)(gpio_mmap_reg + offset);
}

static int mtk_gpio_mapped(void)
{
    return gpio_mmap_reg != NULL;
}

static unsigned int mtk_gpio_field_offset(unsigned int base, unsigned int pin)
{
    return base + (pin / MTK_GPIO_FIELD_PINS_PER_REG) * 0x10;
}

static unsigned int mtk_gpio_field_shift(unsigned int pin)
{
    return pin % MTK_GPIO_FIELD_PINS_PER_REG;
}

static unsigned int mtk_gpio_dir_offset(unsigned int pin, unsigned int *shift)
{
    if (pin <= 175) {
        *shift = pin % MTK_GPIO_FIELD_PINS_PER_REG;
        return MTK_GPIO_DIR + (pin / MTK_GPIO_FIELD_PINS_PER_REG) * 0x10;
    }

    *shift = (pin - 176) % MTK_GPIO_FIELD_PINS_PER_REG;
    return 0xc0 + ((pin - 176) / MTK_GPIO_FIELD_PINS_PER_REG) * 0x10;
}

static int mtk_update_bit(unsigned int offset, unsigned int shift, unsigned int value)
{
    uint32_t tmp;
    volatile uint32_t *position = mtk_gpio_reg(offset);

    tmp = *position;
    if (value) {
        tmp |= (1u << shift);
    }else{
        tmp &= ~(1u << shift);
    }
    *position = tmp;
    return 0;
}

int mtk_set_gpio_out(unsigned int pin, unsigned int output)
{
    if (!mtk_gpio_mapped())
        return -1;

    return mtk_update_bit(mtk_gpio_field_offset(MTK_GPIO_DOUT, pin),
                          mtk_gpio_field_shift(pin), output);
}

int mtk_set_gpio_dir(unsigned int pin, unsigned int dir)
{
    unsigned int offset;
    unsigned int shift;

    if (!mtk_gpio_mapped())
        return -1;

    offset = mtk_gpio_dir_offset(pin, &shift);
    return mtk_update_bit(offset, shift, dir);

}

int mtk_set_gpio_mode(unsigned int pin, unsigned int mode){
    uint32_t tmp;
    volatile uint32_t *position;
    unsigned int shift;

    if (!mtk_gpio_mapped())
        return -1;

    position = mtk_gpio_reg(MTK_GPIO_MODE + (pin / MTK_GPIO_MODE_PINS_PER_REG) * 0x10);
    shift = (pin % MTK_GPIO_MODE_PINS_PER_REG) * 3;

    tmp = *position;
    tmp &= ~(0x7u << shift);
    tmp |= ((mode & 0x7u) << shift);
    *position = tmp;
    return 0;

}

int mtk_set_pullupdn(unsigned int pin, int pud)
{
    unsigned int shift = mtk_gpio_field_shift(pin);

    if (!mtk_gpio_mapped())
        return -1;

    if (pud == PUD_OFF)
        return mtk_update_bit(mtk_gpio_field_offset(MTK_GPIO_PULLE, pin), shift, 0);

    mtk_update_bit(mtk_gpio_field_offset(MTK_GPIO_PULLSEL, pin), shift, pud == PUD_UP);
    return mtk_update_bit(mtk_gpio_field_offset(MTK_GPIO_PULLE, pin), shift, 1);
}

int mtk_gpio_function(unsigned int pin)
{
    uint32_t mode;
    unsigned int offset;
    unsigned int shift;

    if (!mtk_gpio_mapped())
        return 0;

    offset = MTK_GPIO_MODE + (pin / MTK_GPIO_MODE_PINS_PER_REG) * 0x10;
    shift = (pin % MTK_GPIO_MODE_PINS_PER_REG) * 3;
    mode = (*mtk_gpio_reg(offset) >> shift) & 0x7u;
    if (mode != 0)
        return mode;

    offset = mtk_gpio_dir_offset(pin, &shift);
    return ((*mtk_gpio_reg(offset) >> shift) & 0x1u) ? 1 : 0;
}

int mtk_input_gpio(unsigned int pin)
{
    if (!mtk_gpio_mapped())
        return 0;

    return (*mtk_gpio_reg(mtk_gpio_field_offset(MTK_GPIO_DIN, pin)) >>
            mtk_gpio_field_shift(pin)) & 0x1u;
}

int mtk_setup(void)
{
    int gpio_mmap_fd = 0;
    if ((gpio_mmap_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0) {
        fprintf(stderr, "unable to open mmap file");
        return -1;
    }
    
      gpio_mmap_reg = (uint8_t*)mmap(NULL, MTK_GPIO_MAP_SIZE, PROT_READ | PROT_WRITE,
        MAP_FILE | MAP_SHARED, gpio_mmap_fd, 0x10005000);
    if (gpio_mmap_reg == MAP_FAILED) {
        perror("foo");
        fprintf(stderr, "failed to mmap");
        gpio_mmap_reg = NULL;
        close(gpio_mmap_fd);
        return -1;
    }
    close(gpio_mmap_fd);

    return SETUP_OK;

}


uint32_t sunxi_readl(volatile uint32_t *addr)
{
    printf("sunxi_readl\n");
    uint32_t val = 0;
    uint32_t mmap_base = (uint32_t)addr & (~MAP_MASK);
    uint32_t mmap_seek = ((uint32_t)addr - mmap_base) >> 2;
    val = *(gpio_map + mmap_seek);
    return val;
}   

void sunxi_writel(volatile uint32_t *addr, uint32_t val)
{
    printf("sunxi_writel\n");
    uint32_t mmap_base = (uint32_t)addr & (~MAP_MASK);
    uint32_t mmap_seek =( (uint32_t)addr - mmap_base) >> 2;
    *(gpio_map + mmap_seek) = val;
}

int sunxi_setup(void)
{
    int mem_fd;
    uint8_t *gpio_mem;
    uint8_t *r_gpio_mem;
    uint32_t peri_base;
    uint32_t gpio_base;
    unsigned char buf[4];
    FILE *fp;
    char buffer[1024];
    char hardware[1024];
    int found = 0;
	printf("enter to sunxi_setup\n");

    // mmap the GPIO memory registers
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
        return SETUP_DEVMEM_FAIL;

    if ((gpio_mem = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL)
        return SETUP_MALLOC_FAIL;

    if ((uint32_t)gpio_mem % PAGE_SIZE)
        gpio_mem += PAGE_SIZE - ((uint32_t)gpio_mem % PAGE_SIZE);

    gpio_map = (uint32_t *)mmap( (caddr_t)gpio_mem, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, mem_fd, SUNXI_GPIO_BASE);
    pio_map = gpio_map + (SUNXI_GPIO_REG_OFFSET>>2);
//printf("gpio_mem[%x] gpio_map[%x] pio_map[%x]\n", gpio_mem, gpio_map, pio_map);
//R_PIO GPIO LMN
    r_gpio_map = (uint32_t *)mmap( (caddr_t)0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, SUNXI_R_GPIO_BASE);
    r_pio_map = r_gpio_map + (SUNXI_R_GPIO_REG_OFFSET>>2);
//printf("r_gpio_map[%x] r_pio_map[%x]\n", r_gpio_map, r_pio_map);

    if ((uint32_t)gpio_map < 0)
        return SETUP_MMAP_FAIL;

    return SETUP_OK;
}

void sunxi_set_pullupdn(int gpio, int pud)
{
    uint32_t regval = 0;
    int bank = GPIO_BANK(gpio); //gpio >> 5
    int index = GPIO_PUL_INDEX(gpio); // (gpio & 0x1f) >> 4
    int offset = GPIO_PUL_OFFSET(gpio); // (gpio) & 0x0F) << 1
	printf("sunxi_set_pullupdn\n");

    sunxi_gpio_t *pio = &((sunxi_gpio_reg_t *) pio_map)->gpio_bank[bank];
/* DK, for PL and PM */
    if(bank >= 11) {
      bank -= 11;
      pio = &((sunxi_gpio_reg_t *) r_pio_map)->gpio_bank[bank];
    }

    regval = *(&pio->PULL[0] + index);
    regval &= ~(3 << offset);
    regval |= pud << offset;
    *(&pio->PULL[0] + index) = regval;
}

void sunxi_setup_gpio(int gpio, int direction, int pud)
{
    uint32_t regval = 0;
    int bank = GPIO_BANK(gpio); //gpio >> 5
    int index = GPIO_CFG_INDEX(gpio); // (gpio & 0x1F) >> 3
    int offset = GPIO_CFG_OFFSET(gpio); // ((gpio & 0x1F) & 0x7) << 2
    printf("sunxi_setup_gpio\n");
    sunxi_gpio_t *pio = &((sunxi_gpio_reg_t *) pio_map)->gpio_bank[bank];
/* DK, for PL and PM */
    if(bank >= 11) {
      bank -= 11;
      pio = &((sunxi_gpio_reg_t *) r_pio_map)->gpio_bank[bank];
    }

    sunxi_set_pullupdn(gpio, pud);

    regval = *(&pio->CFG[0] + index);
    regval &= ~(0x7 << offset); // 0xf?
    if (INPUT == direction) {
        *(&pio->CFG[0] + index) = regval;
    } else if (OUTPUT == direction) {
        regval |=  (1 << offset);
        *(&pio->CFG[0] + index) = regval;
    } else {
        printf("line:%dgpio number error\n",__LINE__);
    }
}

// Contribution by Eric Ptak <trouch@trouch.com>
int sunxi_gpio_function(int gpio)
{
    uint32_t regval = 0;
    int bank = GPIO_BANK(gpio); //gpio >> 5
    int index = GPIO_CFG_INDEX(gpio); // (gpio & 0x1F) >> 3
    int offset = GPIO_CFG_OFFSET(gpio); // ((gpio & 0x1F) & 0x7) << 2
     printf("sunxi_gpio_function\n");
    sunxi_gpio_t *pio = &((sunxi_gpio_reg_t *) pio_map)->gpio_bank[bank];
/* DK, for PL and PM */
    if(bank >= 11) {
      bank -= 11;
      pio = &((sunxi_gpio_reg_t *) r_pio_map)->gpio_bank[bank];
    }

    regval = *(&pio->CFG[0] + index);
    regval >>= offset;
    regval &= 7;
    return regval; // 0=input, 1=output, 4=alt0
}

void sunxi_output_gpio(int gpio, int value)
{
    int bank = GPIO_BANK(gpio); //gpio >> 5
    int num = GPIO_NUM(gpio); // gpio & 0x1F

 printf("gpio(%d) bank(%d) num(%d)\n", gpio, bank, num);
    sunxi_gpio_t *pio = &((sunxi_gpio_reg_t *) pio_map)->gpio_bank[bank];
/* DK, for PL and PM */
    if(bank >= 11) {
      bank -= 11;
      pio = &((sunxi_gpio_reg_t *) r_pio_map)->gpio_bank[bank];
    }

    if (value == 0)
        *(&pio->DAT) &= ~(1 << num);
    else
        *(&pio->DAT) |= (1 << num);
}

int sunxi_input_gpio(int gpio)
{
    uint32_t regval = 0;
    int bank = GPIO_BANK(gpio); //gpio >> 5
    int num = GPIO_NUM(gpio); // gpio & 0x1F

 printf("gpio(%d) bank(%d) num(%d)\n", gpio, bank, num);
    sunxi_gpio_t *pio = &((sunxi_gpio_reg_t *) pio_map)->gpio_bank[bank];
/* DK, for PL and PM */
    if(bank >= 11) {
      bank -= 11;
      pio = &((sunxi_gpio_reg_t *) r_pio_map)->gpio_bank[bank];
    }

    regval = *(&pio->DAT);
    regval = regval >> num;
    regval &= 1;
    return regval;
}

void bpi_cleanup(void)
{
    if (bpi_found_mtk == 1) {
        if (gpio_mmap_reg != NULL) {
            munmap((void *)gpio_mmap_reg, MTK_GPIO_MAP_SIZE);
            gpio_mmap_reg = NULL;
        }
        return;
    }

    if (gpio_map != MAP_FAILED && gpio_map != NULL) {
        munmap((void *)gpio_map, BLOCK_SIZE);
        gpio_map = NULL;
    }

    if (r_gpio_map != MAP_FAILED && r_gpio_map != NULL) {
        munmap((void *)r_gpio_map, BLOCK_SIZE);
        r_gpio_map = NULL;
    }
}

int bpi_piGpioLayout (void)
{
  FILE *bpiFd ;
  char buffer[1024];
  char hardware[1024];
  struct BPIBoards *board;
  static int  gpioLayout = -1 ;

  if (gpioLayout != -1)	// No point checking twice
    return gpioLayout ;

  bpi_found = 0; // -1: not init, 0: init but not found, 1: found
  if ((bpiFd = fopen("/var/lib/bananapi/board.sh", "r")) == NULL) {
    return -1;
  }
  while(!feof(bpiFd)) {
    fgets(buffer, sizeof(buffer), bpiFd);
    sscanf(buffer, "BOARD=%s", hardware);
    //printf("BPI: buffer[%s] hardware[%s]\n",buffer, hardware);
// Search for board:
    for (board = bpiboard ; board->name != NULL ; ++board) {
      //printf("BPI: name[%s] hardware[%s]\n",board->name, hardware);
      if (strcmp (board->name, hardware) == 0) {
        //gpioLayout = board->gpioLayout;
        gpioLayout = board->model; // BPI: use model to replace gpioLayout
        //printf("BPI: name[%s] gpioLayout(%d)\n",board->name, gpioLayout);
        if(gpioLayout >= BPI_MODEL_MIN) {
          bpi_found = 1;
          break;
        }
      }
    }
    if(bpi_found == 1) {
      break;
    }
  }
  fclose(bpiFd);
  //printf("BPI: name[%s] gpioLayout(%d)\n",board->name, gpioLayout);
  return gpioLayout ;
}

int bpi_get_rpi_info(rpi_info *info)
{
  struct BPIBoards *board=bpiboard;
  static int  gpioLayout = -1 ;
  static char ram[64];
  static char manufacturer[64];
  static char type[64];

  gpioLayout = bpi_piGpioLayout () ;
  printf("BPI: gpioLayout(%d)\n", gpioLayout);
  if(bpi_found == 1) {
    for (board = bpiboard ; board->name != NULL ; ++board) {
      if (board->model == gpioLayout)
        break;
    }
    if (board->name == NULL)
      return -1;
    printf("BPI: name[%s] gpioLayout(%d)\n",board->name, gpioLayout);
    sprintf(ram, "%dMB", piMemorySize [board->mem]);
    sprintf(type, "%s", piModelNames [board->model] ? piModelNames [board->model] : "Unknown");
     //add by jackzeng
     //jude mtk platform
    if(strcmp(board->name, "bpi-r2") == 0){
        bpi_found_mtk = 1;
	printf("found mtk board\n");
    }
    sprintf(manufacturer, "%s", piMakerNames [board->maker]);
    info->p1_revision = 3;
    info->type = type;
    info->ram  = ram;
    info->manufacturer = manufacturer;
    if(bpi_found_mtk == 1){
        info->processor = "MTK";
    }else{
	info->processor = "Allwinner";
    }
    
    strcpy(info->revision, "4001");
//    pin_to_gpio =  board->physToGpio ;
    pinToGpio_BP =  board->pinToGpio ;
    physToGpio_BP = board->physToGpio ;
    pinTobcm_BP = board->pinTobcm ;
    //printf("BPI: name[%s] bType(%d) model(%d)\n",board->name, bType, board->model);
    return 0;
  }
  return -1;
}

#endif //BPI
