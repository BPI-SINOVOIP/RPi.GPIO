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
#define MTK_GPIO_DOUT				0x500
#define MTK_GPIO_DIN				0x630
#define MTK_GPIO_MODE				0x760

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

int *pinToGpio_BP ;
int *physToGpio_BP ;
int *pinTobcm_BP ;


static volatile uint32_t *r_gpio_map;

char *piModelNames [64] =
{
  "Model A",	//  0
  "Model B",	//  1
  "Model A+",	//  2
  "Model B+",	//  3
  "Pi 2",	//  4
  "Alpha",	//  5
  "CM",		//  6
  "Unknown07",	// 07
  "Pi 3",	// 08
  "Pi Zero",	// 09
  "CM3",	// 10
  "Unknown11",	// 11
  "Pi Zero-W",	// 12
  "Unknown13",	// 13
  "Unknown14",	// 14
  "Unknown15",	// 15
#ifdef BPI
  "Banana Pi[New]",	// 16
  "Banana Pi[X86]",	// 17
  "Raspbery Pi[RPI]",	// 18
  "Raspbery Pi[RPI2]",	// 19
  "Raspbery Pi[RPI3]",	// 20
  "Banana Pi M1[A20]",	// 21	
  "Banana Pi M1+[A20]",	// 22
  "Banana Pi R1[A20]",	// 23
  "Banana Pi M2[A31s]",	// 24
  "Banana Pi M3[A83T]",	// 25
  "Banana Pi M2+[H3]",	// 26
  "Banana Pi M64[A64]",	// 27
  "Banana Pi M2 Ultra[R40]",	// 28
  "Banana Pi M2 Magic[R16]",	// 29
  "Banana Pi M2+[H2+]",	// 30
  "Banana Pi M2+[H5]",	// 31
  "Banana Pi M2 Ultra[V40]",	// 32
  "Banana Pi M2 Zero[H2+/H3]",	// 33
  "Banana Pi R2[MT7623]", //34
  NULL,
#endif
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
#ifdef BPI
  "BPI-Sinovoip",	//	 5
#else
  "Unknown05",	//	 5
#endif
  "Unknown06",	//	 6
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
  int *pinToGpio;
  int *physToGpio;
  int *pinTobcm;
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
  { "bpi-m1",	   10001, 21, 1, 2, 5, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bpi-m1p",	   10001, 22, 1, 2, 5, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bpi-r1",	   10001, 23, 1, 2, 5, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bpi-m2",	   10101, 24, 1, 2, 5, 0, pinToGpio_BPI_M2, physToGpio_BPI_M2, pinTobcm_BPI_M2 	},
  { "bpi-m3",	   10201, 25, 1, 3, 5, 0, pinToGpio_BPI_M3, physToGpio_BPI_M3, pinTobcm_BPI_M3 	},
  { "bpi-m2p",	   10301, 26, 1, 2, 5, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m64",	   10401, 27, 1, 3, 5, 0, pinToGpio_BPI_M64, physToGpio_BPI_M64, pinTobcm_BPI_M64 	},
  { "bpi-m2u",	   10501, 28, 1, 3, 5, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-m2m",	   10601, 29, 1, 1, 5, 0, pinToGpio_BPI_M2M, physToGpio_BPI_M2M, pinTobcm_BPI_M2M 	},
  { "bpi-m2p_H2+", 10701, 30, 1, 2, 5, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p_H5",  10801, 31, 1, 2, 5, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2u_V40", 10901, 32, 1, 3, 5, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-m2z",	   11001, 33, 1, 1, 5, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-r2",      11101, 34, 1, 3, 5, 0, pinToGpio_BPI_R2,  physToGpio_BPI_R2,  pinTobcm_BPI_R2    },
  { NULL,		0, 0, 1, 2, 5, 0, NULL, NULL, NULL 	},
} ;



static uint8_t* gpio_mmap_reg = NULL;

int mtk_set_gpio_out(unsigned int pin, unsigned int output)
{
    uint32_t tmp;
    uint32_t position = 0;

    position = gpio_mmap_reg + MTK_GPIO_DOUT + (pin / 16) * 16;
    printf("pin=%d, output = %d, positon = %X\n", pin, output, position);
    tmp = *(volatile uint32_t*)(position);
    printf("tmp = %X\n", tmp);
    if(output == 1){
	    tmp |= (1u << (pin % 16));
    }else{
	    tmp &= ~(1u << (pin % 16));
    }
    printf("tmp = %X\n", tmp);
    *(volatile uint32_t*)(position) = tmp;
    printf("finish mtk_set_gpio_out\n");
    return 1;

}

int mtk_set_gpio_dir(unsigned int pin, unsigned int dir)
{
    uint32_t tmp;
    uint32_t position = 0;

    if(pin < 199){
        position = gpio_mmap_reg + (pin / 16) * 16;
    }else{
        position = gpio_mmap_reg + (pin / 16) * 16 + 0x10;
    }
    printf("pin=%d, dir=%d, positon = %X\n", pin, dir, position);
    tmp = *(volatile uint32_t*)(position);
    printf("tmp = %X\n", tmp);
    if(dir == 1){
        tmp |= (1u << (pin % 16));
    }else{
	tmp &= ~(1u << (pin % 16));
    }
    printf("tmp = %X\n", tmp);
    *(volatile uint32_t*)(position) = tmp;
    return 0;   

}

int mtk_set_gpio_mode(unsigned int pin, unsigned int mode){
    uint32_t tmp;
    uint32_t position = 0;
    position = gpio_mmap_reg + MTK_GPIO_MODE + (pin / 5) * 16;

    printf("pin=%d, mode=%d, positon = %X\n", pin, mode, position);
    tmp = *(volatile uint32_t*)(position);

    printf("tmp = %X\n", tmp);
    tmp &= ~(1u << ((pin % 5) * 3));
    printf("tmp = %X\n", tmp);

    *(volatile uint32_t*)(position) = tmp;
    return ;

}

int mtk_setup(void)
{
    int gpio_mmap_fd = 0;
    if ((gpio_mmap_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0) {
        fprintf(stderr, "unable to open mmap file");
        return -1;
    }
    
      gpio_mmap_reg = (uint8_t*)mmap(NULL, 8 * 1024, PROT_READ | PROT_WRITE,
        MAP_FILE | MAP_SHARED, gpio_mmap_fd, 0x10005000);
    if (gpio_mmap_reg == MAP_FAILED) {
        perror("foo");
        fprintf(stderr, "failed to mmap");
        gpio_mmap_reg = NULL;
        close(gpio_mmap_fd);
        return -1;
    }
    printf("gpio_mmap_fd=%d, gpio_map=%x", gpio_mmap_fd, gpio_mmap_reg);

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

    set_pullupdn(gpio, pud);

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
        if(gpioLayout >= 21) {
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
  char ram[64];
  char manufacturer[64];
  char processor[64];
  char type[64];

  gpioLayout = bpi_piGpioLayout () ;
  printf("BPI: gpioLayout(%d)\n", gpioLayout);
  if(bpi_found == 1) {
    board = &bpiboard[gpioLayout];
    printf("BPI: name[%s] gpioLayout(%d)\n",board->name, gpioLayout);
    sprintf(ram, "%dMB", piMemorySize [board->mem]);
    sprintf(type, "%s", piModelNames [board->model]);
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
