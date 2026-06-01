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
#define BPI_MODEL_M4BERRY    79
#define BPI_MODEL_M4ZERO     80
#define BPI_MODEL_M2S        81
#define BPI_MODEL_CM4IO      82
#define BPI_MODEL_M5         83
#define BPI_MODEL_M2PRO      84
#define BPI_MODEL_F3         85
#define BPI_MODEL_AI2N       86
#define BPI_MODEL_R2PRO      87
#define BPI_MODEL_M5PRO      88
#define BPI_MODEL_CM5PRO     89
#define BPI_MODEL_M7         90
#define BPI_MODEL_W3         91
#define BPI_MODEL_AIM7       92
#define BPI_MODEL_M4SUPER    93
#define BPI_MODEL_M1SUPER    94
#define BPI_MODEL_FORGE1     95
#define BPI_MODEL_P2PRO      96
#define BPI_MODEL_W2         97
#define BPI_MODEL_M4         98
#define BPI_MODEL_M6         99
#define BPI_MODELS_MAX       100

#define BPI_MAKER_SINOVOIP    6

#define SUNXI_R_GPIO_BASE	0x01F02000
#define SUNXI_R_GPIO_REG_OFFSET   0xC00
#define SUNXI_GPIO_BASE		0x01C20000
#define SUNXI_GPIO_REG_OFFSET   0x800
#define SUN50IW9_GPIO_BASE	0x0300B000
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

#define MESON_GPIO_BASE_ADDR			0xFF634000
#define MESON_GPIO_AO_BASE_ADDR			0xFF800000
#define MESON_GPIO_PIN_BASE			410
#define MESON_GPIOH_PIN_START			(MESON_GPIO_PIN_BASE + 17)
#define MESON_GPIOH_PIN_END			(MESON_GPIO_PIN_BASE + 25)
#define MESON_GPIOA_PIN_START			(MESON_GPIO_PIN_BASE + 50)
#define MESON_GPIOA_PIN_END			(MESON_GPIO_PIN_BASE + 65)
#define MESON_GPIOX_PIN_START			(MESON_GPIO_PIN_BASE + 66)
#define MESON_GPIOX_PIN_MID			(MESON_GPIO_PIN_BASE + 81)
#define MESON_GPIOX_PIN_END			(MESON_GPIO_PIN_BASE + 85)
#define MESON_GPIOAO_PIN_START			(MESON_GPIO_PIN_BASE + 86)
#define MESON_GPIOAO_PIN_END			(MESON_GPIO_PIN_BASE + 97)

#define MESON_GPIOH_FSEL_REG_OFFSET		0x119
#define MESON_GPIOH_OUTP_REG_OFFSET		0x11A
#define MESON_GPIOH_INP_REG_OFFSET		0x11B
#define MESON_GPIOH_PUPD_REG_OFFSET		0x13D
#define MESON_GPIOH_PUEN_REG_OFFSET		0x14B
#define MESON_GPIOH_MUX_B_REG_OFFSET		0x1BB

#define MESON_GPIOA_FSEL_REG_OFFSET		0x120
#define MESON_GPIOA_OUTP_REG_OFFSET		0x121
#define MESON_GPIOA_INP_REG_OFFSET		0x122
#define MESON_GPIOA_PUPD_REG_OFFSET		0x13F
#define MESON_GPIOA_PUEN_REG_OFFSET		0x14D
#define MESON_GPIOA_MUX_D_REG_OFFSET		0x1BD
#define MESON_GPIOA_MUX_E_REG_OFFSET		0x1BE

#define MESON_GPIOX_FSEL_REG_OFFSET		0x116
#define MESON_GPIOX_OUTP_REG_OFFSET		0x117
#define MESON_GPIOX_INP_REG_OFFSET		0x118
#define MESON_GPIOX_PUPD_REG_OFFSET		0x13C
#define MESON_GPIOX_PUEN_REG_OFFSET		0x14A
#define MESON_GPIOX_MUX_3_REG_OFFSET		0x1B3
#define MESON_GPIOX_MUX_4_REG_OFFSET		0x1B4
#define MESON_GPIOX_MUX_5_REG_OFFSET		0x1B5

#define MESON_GPIOAO_FSEL_REG_OFFSET		0x109
#define MESON_GPIOAO_OUTP_REG_OFFSET		0x10D
#define MESON_GPIOAO_INP_REG_OFFSET		0x10A
#define MESON_GPIOAO_PUPD_REG_OFFSET		0x10B
#define MESON_GPIOAO_PUEN_REG_OFFSET		0x10C
#define MESON_GPIOAO_MUX_REG0_OFFSET		0x105
#define MESON_GPIOAO_MUX_REG1_OFFSET		0x106

#define SPACEMIT_GPIO_BASE_ADDR		0xD4019000
#define SPACEMIT_PINCTRL_BASE_ADDR		0xD401E000
#define SPACEMIT_GPIO_PIN_BASE			0
#define SPACEMIT_GPIO_PIN_END			127

#define SPACEMIT_BANK012_OFFSET(x)		((x) << 2)
#define SPACEMIT_BANK3_OFFSET			0x100

#define SPACEMIT_GPLR				0x0
#define SPACEMIT_GPDR				0xC
#define SPACEMIT_GPSR				0x18
#define SPACEMIT_GPCR				0x24
#define SPACEMIT_GSDR				0x54
#define SPACEMIT_GCDR				0x60

#define SPACEMIT_AF_SEL_OFFSET			0
#define SPACEMIT_AF_SEL_MASK			(7 << 0)
#define SPACEMIT_PULL_DIS			0
#define SPACEMIT_PULL_UP			6
#define SPACEMIT_PULL_DOWN			5
#define SPACEMIT_PULL_OFFSET			13
#define SPACEMIT_PULL_MASK			(7 << 13)

#define RENESAS_GPIO_BASE_ADDR			0x10410000
#define RENESAS_GPIO_PIN_BASE			416
#define RENESAS_GPIO_PIN_END			511
#define RENESAS_GPIO_MAP_SIZE			(8 * 1024)

#define RENESAS_PINS_PER_PORT			8
#define RENESAS_EXTENDED_REG_OFFSET		0x10
#define RENESAS_PIN_OFFSET(pin)			((pin) - RENESAS_GPIO_PIN_BASE)
#define RENESAS_PIN_ID_TO_PORT(n)		((n) / RENESAS_PINS_PER_PORT)
#define RENESAS_PIN_ID_TO_PORT_OFFSET(n)	(RENESAS_PIN_ID_TO_PORT(n) + RENESAS_EXTENDED_REG_OFFSET)
#define RENESAS_PIN_ID_TO_PIN(n)		((n) % RENESAS_PINS_PER_PORT)

#define RENESAS_P(n)				(0x0000 + 0x10 + (n))
#define RENESAS_PM(n)				(0x0100 + 0x20 + (n) * 2)
#define RENESAS_PMC(n)				(0x0200 + 0x10 + (n))
#define RENESAS_PFC(n)				(0x0400 + 0x40 + (n) * 4)
#define RENESAS_PIN(n)				(0x0800 + 0x10 + (n))
#define RENESAS_PUPD(n)			(0x1C00 + (n) * 8)

#define RENESAS_PM_INPUT			0x1
#define RENESAS_PM_OUTPUT			0x2

#define RENESAS_PULL_DIS			0x0
#define RENESAS_PULL_UP			0x3
#define RENESAS_PULL_DOWN			0x2

#define ROCKCHIP_GPIO_BANKS			5
#define ROCKCHIP_GPIO_PIN_BASE			0
#define ROCKCHIP_GPIO_PIN_END			159
#define ROCKCHIP_GPIO_MAP_SIZE_RK3308		0x100
#define ROCKCHIP_GPIO_MAP_SIZE_RK3568		0x100
#define ROCKCHIP_GPIO_MAP_SIZE_RK3528		0x200
#define ROCKCHIP_GPIO_MAP_SIZE_RK3506		0x200
#define ROCKCHIP_GPIO_MAP_SIZE_RK3576		0x200
#define ROCKCHIP_GPIO_MAP_SIZE_RK3588		0x100

#define ROCKCHIP_GPIO_SWPORT_DR_V1		0x00
#define ROCKCHIP_GPIO_SWPORT_DDR_V1		0x04
#define ROCKCHIP_GPIO_EXT_PORT_V1		0x50
#define ROCKCHIP_GPIO_SWPORT_DR_V2		0x00
#define ROCKCHIP_GPIO_SWPORT_DDR_V2		0x08
#define ROCKCHIP_GPIO_EXT_PORT_V2		0x70

#define REALTEK_GPIO_GROUPS			2
#define REALTEK_GPIO_MAP_SIZE			0x100
#define REALTEK_RTD129X_MISC_BASE		0x9801b100
#define REALTEK_RTD129X_ISO_BASE		0x98007100
#define REALTEK_RTD139X_ISO_BASE		0x98007100
#define REALTEK_RTD129X_MISC_PIN_BASE		0
#define REALTEK_RTD129X_MISC_PIN_END		100
#define REALTEK_RTD129X_ISO_PIN_BASE		101
#define REALTEK_RTD129X_ISO_PIN_END		135
#define REALTEK_RTD139X_ISO_PIN_BASE		0
#define REALTEK_RTD139X_ISO_PIN_END		56

#define VS680_GPIO_BANKS			4
#define VS680_GPIO_MAP_SIZE			0x400
#define VS680_GPIO_SOC_PIN_BASE		0
#define VS680_GPIO_SOC_PIN_END			95
#define VS680_GPIO_SM_PIN_BASE			96
#define VS680_GPIO_SM_PIN_END			127
#define VS680_GPIO_SWPORT_DR			0x00
#define VS680_GPIO_SWPORT_DDR			0x04
#define VS680_GPIO_EXT_PORT			0x50

struct realtek_gpio_group {
    int pin_base;
    int pin_end;
    int map_index;
    const int *dir_offset;
    const int *dato_offset;
    const int *dati_offset;
};

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
int bpi_found_sun50iw9 = 0;
int bpi_found_meson = 0;
int bpi_found_spacemit = 0;
int bpi_found_renesas = 0;
int bpi_found_rockchip = 0;
int bpi_found_realtek = 0;
int bpi_found_vs680 = 0;

const int *pinToGpio_BP ;
const int *physToGpio_BP ;
const int *pinTobcm_BP ;


static volatile uint32_t *r_gpio_map;
static volatile uint32_t *spacemit_gpio_map;
static volatile uint32_t *spacemit_pinctrl_map;
static volatile uint32_t *renesas_gpio_map;
static volatile uint32_t *rockchip_gpio_map[ROCKCHIP_GPIO_BANKS] = { NULL };
static volatile uint32_t *realtek_gpio_map[REALTEK_GPIO_GROUPS] = { NULL };
static volatile uint32_t *vs680_gpio_map[VS680_GPIO_BANKS] = { NULL };
static const off_t rockchip_gpio_base_rk3308[ROCKCHIP_GPIO_BANKS] = {
  0xff220000,
  0xff230000,
  0xff240000,
  0xff250000,
  0xff260000,
};
static const off_t rockchip_gpio_base_rk3568[ROCKCHIP_GPIO_BANKS] = {
  0xfdd60000,
  0xfe740000,
  0xfe750000,
  0xfe760000,
  0xfe770000,
};
static const off_t rockchip_gpio_base_rk3528[ROCKCHIP_GPIO_BANKS] = {
  0xff610000,
  0xffaf0000,
  0xffb00000,
  0xffb10000,
  0xffb20000,
};
static const off_t rockchip_gpio_base_rk3506[ROCKCHIP_GPIO_BANKS] = {
  0xff940000,
  0xff870000,
  0xff1c0000,
  0xff1d0000,
  0xff1e0000,
};
static const off_t rockchip_gpio_base_rk3576[ROCKCHIP_GPIO_BANKS] = {
  0x27320000,
  0x2ae10000,
  0x2ae20000,
  0x2ae30000,
  0x2ae40000,
};
static const off_t rockchip_gpio_base_rk3588[ROCKCHIP_GPIO_BANKS] = {
  0xfd8a0000,
  0xfec20000,
  0xfec30000,
  0xfec40000,
  0xfec50000,
};
static const off_t *rockchip_gpio_base = rockchip_gpio_base_rk3568;
static size_t rockchip_gpio_map_size = ROCKCHIP_GPIO_MAP_SIZE_RK3568;
static int rockchip_gpio_v2 = 1;
static int rockchip_gpio_swport_dr = ROCKCHIP_GPIO_SWPORT_DR_V2;
static int rockchip_gpio_swport_ddr = ROCKCHIP_GPIO_SWPORT_DDR_V2;
static int rockchip_gpio_ext_port = ROCKCHIP_GPIO_EXT_PORT_V2;
static const int realtek_rtd129x_misc_dir[4] = { 0x00, 0x04, 0x08, 0x0c };
static const int realtek_rtd129x_misc_dato[4] = { 0x10, 0x14, 0x18, 0x1c };
static const int realtek_rtd129x_misc_dati[4] = { 0x20, 0x24, 0x28, 0x2c };
static const int realtek_rtd129x_iso_dir[4] = { 0x00, 0x18, 0x00, 0x00 };
static const int realtek_rtd129x_iso_dato[4] = { 0x04, 0x1c, 0x00, 0x00 };
static const int realtek_rtd129x_iso_dati[4] = { 0x08, 0x20, 0x00, 0x00 };
static const struct realtek_gpio_group realtek_rtd129x_groups[REALTEK_GPIO_GROUPS] = {
    {
        REALTEK_RTD129X_MISC_PIN_BASE,
        REALTEK_RTD129X_MISC_PIN_END,
        0,
        realtek_rtd129x_misc_dir,
        realtek_rtd129x_misc_dato,
        realtek_rtd129x_misc_dati,
    },
    {
        REALTEK_RTD129X_ISO_PIN_BASE,
        REALTEK_RTD129X_ISO_PIN_END,
        1,
        realtek_rtd129x_iso_dir,
        realtek_rtd129x_iso_dato,
        realtek_rtd129x_iso_dati,
    },
};
static const off_t realtek_gpio_base_rtd129x[REALTEK_GPIO_GROUPS] = {
    REALTEK_RTD129X_MISC_BASE,
    REALTEK_RTD129X_ISO_BASE,
};
static const struct realtek_gpio_group realtek_rtd139x_groups[1] = {
    {
        REALTEK_RTD139X_ISO_PIN_BASE,
        REALTEK_RTD139X_ISO_PIN_END,
        0,
        realtek_rtd129x_iso_dir,
        realtek_rtd129x_iso_dato,
        realtek_rtd129x_iso_dati,
    },
};
static const off_t realtek_gpio_base_rtd139x[REALTEK_GPIO_GROUPS] = {
    REALTEK_RTD139X_ISO_BASE,
    0,
};
static const off_t vs680_gpio_base[VS680_GPIO_BANKS] = {
    0xf7e82400,
    0xf7e80800,
    0xf7e80c00,
    0xf7fc8000,
};
static const struct realtek_gpio_group *realtek_gpio_groups = realtek_rtd129x_groups;
static const off_t *realtek_gpio_base = realtek_gpio_base_rtd129x;
static int realtek_gpio_group_count = REALTEK_GPIO_GROUPS;
static size_t realtek_gpio_map_size = REALTEK_GPIO_MAP_SIZE;

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
  [BPI_MODEL_M4BERRY] = "Banana Pi M4 Berry[H618]",
  [BPI_MODEL_M4ZERO]  = "Banana Pi M4 Zero[H618]",
  [BPI_MODEL_M2S]     = "Banana Pi M2S[Amlogic G12B]",
  [BPI_MODEL_CM4IO]   = "Banana Pi CM4IO[Amlogic G12B]",
  [BPI_MODEL_M5]      = "Banana Pi M5[Amlogic SM1]",
  [BPI_MODEL_M2PRO]   = "Banana Pi M2 Pro[Amlogic SM1]",
  [BPI_MODEL_F3]      = "Banana Pi F3[SpacemiT K1]",
  [BPI_MODEL_AI2N]    = "Banana Pi AI2N[Renesas RZ/V2N]",
  [BPI_MODEL_R2PRO]   = "Banana Pi R2 Pro[RK3568]",
  [BPI_MODEL_M5PRO]   = "Banana Pi M5 Pro[RK3576]",
  [BPI_MODEL_CM5PRO]  = "Banana Pi CM5 Pro[RK3576]",
  [BPI_MODEL_M7]      = "Banana Pi M7[RK3588]",
  [BPI_MODEL_W3]      = "Banana Pi W3[RK3588]",
  [BPI_MODEL_AIM7]    = "Banana Pi AIM7[RK3588]",
  [BPI_MODEL_M4SUPER] = "Banana Pi M4 Super[RK3568]",
  [BPI_MODEL_M1SUPER] = "Banana Pi M1 Super[RK3528]",
  [BPI_MODEL_FORGE1]  = "Banana Pi Forge1[RK3506J]",
  [BPI_MODEL_P2PRO]   = "Banana Pi P2 Pro[RK3308]",
  [BPI_MODEL_W2]      = "Banana Pi W2[RTD1296]",
  [BPI_MODEL_M4]      = "Banana Pi M4[RTD1395]",
  [BPI_MODEL_M6]      = "Banana Pi M6[Synaptics VS680]",
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
  4096,		//	 4
  8192,		//	 5
#else
     0,		//	 3
     0,		//	 4
     0,		//	 5
#endif
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
  { "bpi-pro",	   10001, BPI_MODEL_M1P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "banana-pro", 10001, BPI_MODEL_M1P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bananapro",  10001, BPI_MODEL_M1P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bananapi-pro", 10001, BPI_MODEL_M1P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
  { "bananapipro", 10001, BPI_MODEL_M1P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1P, physToGpio_BPI_M1P, pinTobcm_BPI_M1P 	},
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
  { "bpi-m2-ultra", 10501, BPI_MODEL_M2U, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-m2m",	   10601, BPI_MODEL_M2M, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2M, physToGpio_BPI_M2M, pinTobcm_BPI_M2M 	},
  { "bpi-m2-magic", 10601, BPI_MODEL_M2M, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2M, physToGpio_BPI_M2M, pinTobcm_BPI_M2M 	},
  { "bpi-m2m-v1.1", 10601, BPI_MODEL_M2M_V11, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2M_V11, physToGpio_BPI_M2M_V11, pinTobcm_BPI_M2M_V11 	},
  { "bpi-m2m-v11", 10601, BPI_MODEL_M2M_V11, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2M_V11, physToGpio_BPI_M2M_V11, pinTobcm_BPI_M2M_V11 	},
  { "bpi-m2-magic-v1.1", 10601, BPI_MODEL_M2M_V11, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2M_V11, physToGpio_BPI_M2M_V11, pinTobcm_BPI_M2M_V11 	},
  { "bpi-m2-magic-v11", 10601, BPI_MODEL_M2M_V11, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2M_V11, physToGpio_BPI_M2M_V11, pinTobcm_BPI_M2M_V11 	},
  { "bpi-m2p_H2+", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p-h2+", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p-h2p", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2-plus-h2+", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2-plus-h2p", 10701, BPI_MODEL_M2P_H2P, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p_H5",  10801, BPI_MODEL_M2P_H5, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2p-h5",  10801, BPI_MODEL_M2P_H5, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2-plus-h5", 10801, BPI_MODEL_M2P_H5, 1, 2, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2u_V40", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-m2u-v40", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-m2b", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-m2-berry", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-6204", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-cs6204", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-cs-6204", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-6202", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-cs6202", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-cs-6202", 10901, BPI_MODEL_M2U_V40, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2U, physToGpio_BPI_M2U, pinTobcm_BPI_M2U 	},
  { "bpi-m2z",	   11001, BPI_MODEL_M2Z, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m2-zero", 11001, BPI_MODEL_M2Z, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-p2z",	   11001, BPI_MODEL_M2Z, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-p2-zero", 11001, BPI_MODEL_M2Z, 1, 1, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2P, physToGpio_BPI_M2P, pinTobcm_BPI_M2P 	},
  { "bpi-m4berry", 11201, BPI_MODEL_M4BERRY, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4BERRY, physToGpio_BPI_M4BERRY, pinTobcm_BPI_M4BERRY 	},
  { "bpi-m4-berry", 11201, BPI_MODEL_M4BERRY, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4BERRY, physToGpio_BPI_M4BERRY, pinTobcm_BPI_M4BERRY 	},
  { "bananapim4berry", 11201, BPI_MODEL_M4BERRY, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4BERRY, physToGpio_BPI_M4BERRY, pinTobcm_BPI_M4BERRY 	},
  { "bpi-m4zero", 11301, BPI_MODEL_M4ZERO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4ZERO, physToGpio_BPI_M4ZERO, pinTobcm_BPI_M4ZERO 	},
  { "bpi-m4-zero", 11301, BPI_MODEL_M4ZERO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4ZERO, physToGpio_BPI_M4ZERO, pinTobcm_BPI_M4ZERO 	},
  { "bananapim4zero", 11301, BPI_MODEL_M4ZERO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4ZERO, physToGpio_BPI_M4ZERO, pinTobcm_BPI_M4ZERO 	},
  { "bpi-m2s",     11401, BPI_MODEL_M2S, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2S, physToGpio_BPI_M2S, pinTobcm_BPI_M2S 	},
  { "bananapim2s", 11401, BPI_MODEL_M2S, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2S, physToGpio_BPI_M2S, pinTobcm_BPI_M2S 	},
  { "banana-pi-m2s", 11401, BPI_MODEL_M2S, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2S, physToGpio_BPI_M2S, pinTobcm_BPI_M2S 	},
  { "bananapi-m2s", 11401, BPI_MODEL_M2S, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M2S, physToGpio_BPI_M2S, pinTobcm_BPI_M2S 	},
  { "bpi-cm4io",   11501, BPI_MODEL_CM4IO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM4IO, physToGpio_BPI_CM4IO, pinTobcm_BPI_CM4IO 	},
  { "bpi-cm4-io",  11501, BPI_MODEL_CM4IO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM4IO, physToGpio_BPI_CM4IO, pinTobcm_BPI_CM4IO 	},
  { "bananapicm4io", 11501, BPI_MODEL_CM4IO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM4IO, physToGpio_BPI_CM4IO, pinTobcm_BPI_CM4IO 	},
  { "banana-pi-cm4io", 11501, BPI_MODEL_CM4IO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM4IO, physToGpio_BPI_CM4IO, pinTobcm_BPI_CM4IO 	},
  { "bpi-cm4",     11501, BPI_MODEL_CM4IO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM4IO, physToGpio_BPI_CM4IO, pinTobcm_BPI_CM4IO 	},
  { "bananapicm4", 11501, BPI_MODEL_CM4IO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM4IO, physToGpio_BPI_CM4IO, pinTobcm_BPI_CM4IO 	},
  { "bpi-cm5pro",  12101, BPI_MODEL_CM5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM5PRO, physToGpio_BPI_CM5PRO, pinTobcm_BPI_CM5PRO 	},
  { "bpi-cm5-pro", 12101, BPI_MODEL_CM5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM5PRO, physToGpio_BPI_CM5PRO, pinTobcm_BPI_CM5PRO 	},
  { "bpi-cm5pro-io", 12101, BPI_MODEL_CM5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM5PRO, physToGpio_BPI_CM5PRO, pinTobcm_BPI_CM5PRO 	},
  { "bpi-cm5-pro-io", 12101, BPI_MODEL_CM5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM5PRO, physToGpio_BPI_CM5PRO, pinTobcm_BPI_CM5PRO 	},
  { "bananapicm5pro", 12101, BPI_MODEL_CM5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM5PRO, physToGpio_BPI_CM5PRO, pinTobcm_BPI_CM5PRO 	},
  { "bananapi-cm5pro", 12101, BPI_MODEL_CM5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM5PRO, physToGpio_BPI_CM5PRO, pinTobcm_BPI_CM5PRO 	},
  { "bananapi-cm5-pro", 12101, BPI_MODEL_CM5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM5PRO, physToGpio_BPI_CM5PRO, pinTobcm_BPI_CM5PRO 	},
  { "banana-pi-cm5-pro", 12101, BPI_MODEL_CM5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_CM5PRO, physToGpio_BPI_CM5PRO, pinTobcm_BPI_CM5PRO 	},
  { "bpi-m5",      11601, BPI_MODEL_M5, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "bananapim5",  11601, BPI_MODEL_M5, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "banana-pi-m5", 11601, BPI_MODEL_M5, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "bananapi-m5", 11601, BPI_MODEL_M5, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "bpi-m5pro",   12201, BPI_MODEL_M5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5PRO, physToGpio_BPI_M5PRO, pinTobcm_BPI_M5PRO 	},
  { "bpi-m5-pro",  12201, BPI_MODEL_M5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5PRO, physToGpio_BPI_M5PRO, pinTobcm_BPI_M5PRO 	},
  { "bananapim5pro", 12201, BPI_MODEL_M5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5PRO, physToGpio_BPI_M5PRO, pinTobcm_BPI_M5PRO 	},
  { "bananapi-m5pro", 12201, BPI_MODEL_M5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5PRO, physToGpio_BPI_M5PRO, pinTobcm_BPI_M5PRO 	},
  { "bananapi-m5-pro", 12201, BPI_MODEL_M5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5PRO, physToGpio_BPI_M5PRO, pinTobcm_BPI_M5PRO 	},
  { "banana-pi-m5-pro", 12201, BPI_MODEL_M5PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5PRO, physToGpio_BPI_M5PRO, pinTobcm_BPI_M5PRO 	},
  { "bpi-m2pro",   11701, BPI_MODEL_M2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "bpi-m2-pro",  11701, BPI_MODEL_M2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "bananapim2pro", 11701, BPI_MODEL_M2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "bananapi-m2pro", 11701, BPI_MODEL_M2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "bananapi-m2-pro", 11701, BPI_MODEL_M2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "banana-pi-m2pro", 11701, BPI_MODEL_M2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "banana-pi-m2-pro", 11701, BPI_MODEL_M2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M5, physToGpio_BPI_M5, pinTobcm_BPI_M5 	},
  { "bpi-f3",      11801, BPI_MODEL_F3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_F3, physToGpio_BPI_F3, pinTobcm_BPI_F3 	},
  { "bananapif3",  11801, BPI_MODEL_F3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_F3, physToGpio_BPI_F3, pinTobcm_BPI_F3 	},
  { "banana-pi-f3", 11801, BPI_MODEL_F3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_F3, physToGpio_BPI_F3, pinTobcm_BPI_F3 	},
  { "bananapi-f3", 11801, BPI_MODEL_F3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_F3, physToGpio_BPI_F3, pinTobcm_BPI_F3 	},
  { "bpi-ai2n",    11901, BPI_MODEL_AI2N, 1, 5, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_AI2N, physToGpio_BPI_AI2N, pinTobcm_BPI_AI2N 	},
  { "bpi-ai2-n",   11901, BPI_MODEL_AI2N, 1, 5, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_AI2N, physToGpio_BPI_AI2N, pinTobcm_BPI_AI2N 	},
  { "bananapiai2n", 11901, BPI_MODEL_AI2N, 1, 5, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_AI2N, physToGpio_BPI_AI2N, pinTobcm_BPI_AI2N 	},
  { "banana-pi-ai2n", 11901, BPI_MODEL_AI2N, 1, 5, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_AI2N, physToGpio_BPI_AI2N, pinTobcm_BPI_AI2N 	},
  { "bananapi-ai2n", 11901, BPI_MODEL_AI2N, 1, 5, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_AI2N, physToGpio_BPI_AI2N, pinTobcm_BPI_AI2N 	},
  { "bpi-r2pro",   12001, BPI_MODEL_R2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_R2PRO, physToGpio_BPI_R2PRO, pinTobcm_BPI_R2PRO 	},
  { "bpi-r2-pro",  12001, BPI_MODEL_R2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_R2PRO, physToGpio_BPI_R2PRO, pinTobcm_BPI_R2PRO 	},
  { "bananapir2pro", 12001, BPI_MODEL_R2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_R2PRO, physToGpio_BPI_R2PRO, pinTobcm_BPI_R2PRO 	},
  { "bananapi-r2pro", 12001, BPI_MODEL_R2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_R2PRO, physToGpio_BPI_R2PRO, pinTobcm_BPI_R2PRO 	},
  { "bananapi-r2-pro", 12001, BPI_MODEL_R2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_R2PRO, physToGpio_BPI_R2PRO, pinTobcm_BPI_R2PRO 	},
  { "banana-pi-r2-pro", 12001, BPI_MODEL_R2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_R2PRO, physToGpio_BPI_R2PRO, pinTobcm_BPI_R2PRO 	},
  { "bpi-m7",      12301, BPI_MODEL_M7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "bananapim7",  12301, BPI_MODEL_M7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "banana-pi-m7", 12301, BPI_MODEL_M7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "bananapi-m7", 12301, BPI_MODEL_M7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "bpi-w3",      12401, BPI_MODEL_W3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_W3, physToGpio_BPI_W3, pinTobcm_BPI_W3 	},
  { "bananapiw3",  12401, BPI_MODEL_W3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_W3, physToGpio_BPI_W3, pinTobcm_BPI_W3 	},
  { "banana-pi-w3", 12401, BPI_MODEL_W3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_W3, physToGpio_BPI_W3, pinTobcm_BPI_W3 	},
  { "bananapi-w3", 12401, BPI_MODEL_W3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_W3, physToGpio_BPI_W3, pinTobcm_BPI_W3 	},
  { "armsom-w3",   12401, BPI_MODEL_W3, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_W3, physToGpio_BPI_W3, pinTobcm_BPI_W3 	},
  { "bpi-aim7",    12501, BPI_MODEL_AIM7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "bananapiaim7", 12501, BPI_MODEL_AIM7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "banana-pi-aim7", 12501, BPI_MODEL_AIM7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "bananapi-aim7", 12501, BPI_MODEL_AIM7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "armsom-aim7", 12501, BPI_MODEL_AIM7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "armsom-aim7-io", 12501, BPI_MODEL_AIM7, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M7, physToGpio_BPI_M7, pinTobcm_BPI_M7 	},
  { "bpi-m4super", 12601, BPI_MODEL_M4SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4SUPER, physToGpio_BPI_M4SUPER, pinTobcm_BPI_M4SUPER 	},
  { "bpi-m4-super", 12601, BPI_MODEL_M4SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4SUPER, physToGpio_BPI_M4SUPER, pinTobcm_BPI_M4SUPER 	},
  { "bananapim4super", 12601, BPI_MODEL_M4SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4SUPER, physToGpio_BPI_M4SUPER, pinTobcm_BPI_M4SUPER 	},
  { "banana-pi-m4-super", 12601, BPI_MODEL_M4SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4SUPER, physToGpio_BPI_M4SUPER, pinTobcm_BPI_M4SUPER 	},
  { "bananapi-m4super", 12601, BPI_MODEL_M4SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4SUPER, physToGpio_BPI_M4SUPER, pinTobcm_BPI_M4SUPER 	},
  { "bananapi-m4-super", 12601, BPI_MODEL_M4SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4SUPER, physToGpio_BPI_M4SUPER, pinTobcm_BPI_M4SUPER 	},
  { "armsom-sige3", 12601, BPI_MODEL_M4SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4SUPER, physToGpio_BPI_M4SUPER, pinTobcm_BPI_M4SUPER 	},
  { "bpi-m1super", 12701, BPI_MODEL_M1SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bpi-m1-super", 12701, BPI_MODEL_M1SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bpi-m1s", 12701, BPI_MODEL_M1SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bananapim1super", 12701, BPI_MODEL_M1SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "banana-pi-m1-super", 12701, BPI_MODEL_M1SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bananapi-m1super", 12701, BPI_MODEL_M1SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bananapi-m1-super", 12701, BPI_MODEL_M1SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bananapi-m1s", 12701, BPI_MODEL_M1SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "armsom-sige1", 12701, BPI_MODEL_M1SUPER, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bpi-forge1", 12801, BPI_MODEL_FORGE1, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bananapiforge1", 12801, BPI_MODEL_FORGE1, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "banana-pi-forge1", 12801, BPI_MODEL_FORGE1, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bananapi-forge1", 12801, BPI_MODEL_FORGE1, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "armsom-forge1", 12801, BPI_MODEL_FORGE1, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M1SUPER, physToGpio_BPI_M1SUPER, pinTobcm_BPI_M1SUPER 	},
  { "bpi-p2pro",   12901, BPI_MODEL_P2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_P2PRO, physToGpio_BPI_P2PRO, pinTobcm_BPI_P2PRO 	},
  { "bpi-p2-pro",  12901, BPI_MODEL_P2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_P2PRO, physToGpio_BPI_P2PRO, pinTobcm_BPI_P2PRO 	},
  { "bananapip2pro", 12901, BPI_MODEL_P2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_P2PRO, physToGpio_BPI_P2PRO, pinTobcm_BPI_P2PRO 	},
  { "bananapi-p2pro", 12901, BPI_MODEL_P2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_P2PRO, physToGpio_BPI_P2PRO, pinTobcm_BPI_P2PRO 	},
  { "bananapi-p2-pro", 12901, BPI_MODEL_P2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_P2PRO, physToGpio_BPI_P2PRO, pinTobcm_BPI_P2PRO 	},
  { "banana-pi-p2-pro", 12901, BPI_MODEL_P2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_P2PRO, physToGpio_BPI_P2PRO, pinTobcm_BPI_P2PRO 	},
  { "armsom-p2pro", 12901, BPI_MODEL_P2PRO, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_P2PRO, physToGpio_BPI_P2PRO, pinTobcm_BPI_P2PRO 	},
  { "bpi-w2",      13001, BPI_MODEL_W2, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_W2, physToGpio_BPI_W2, pinTobcm_BPI_W2 	},
  { "bananapiw2",  13001, BPI_MODEL_W2, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_W2, physToGpio_BPI_W2, pinTobcm_BPI_W2 	},
  { "bananapi-w2", 13001, BPI_MODEL_W2, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_W2, physToGpio_BPI_W2, pinTobcm_BPI_W2 	},
  { "banana-pi-w2", 13001, BPI_MODEL_W2, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_W2, physToGpio_BPI_W2, pinTobcm_BPI_W2 	},
  { "bpi-m4",      13101, BPI_MODEL_M4, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4, physToGpio_BPI_M4, pinTobcm_BPI_M4 	},
  { "bananapim4",  13101, BPI_MODEL_M4, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4, physToGpio_BPI_M4, pinTobcm_BPI_M4 	},
  { "bananapi-m4", 13101, BPI_MODEL_M4, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4, physToGpio_BPI_M4, pinTobcm_BPI_M4 	},
  { "banana-pi-m4", 13101, BPI_MODEL_M4, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M4, physToGpio_BPI_M4, pinTobcm_BPI_M4 	},
  { "bpi-m6",      13201, BPI_MODEL_M6, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M6, physToGpio_BPI_M6, pinTobcm_BPI_M6 	},
  { "bananapim6",  13201, BPI_MODEL_M6, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M6, physToGpio_BPI_M6, pinTobcm_BPI_M6 	},
  { "bananapi-m6", 13201, BPI_MODEL_M6, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M6, physToGpio_BPI_M6, pinTobcm_BPI_M6 	},
  { "banana-pi-m6", 13201, BPI_MODEL_M6, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_M6, physToGpio_BPI_M6, pinTobcm_BPI_M6 	},
  { "bpi-r2",      11101, BPI_MODEL_R2, 1, 3, BPI_MAKER_SINOVOIP, 0, pinToGpio_BPI_R2,  physToGpio_BPI_R2,  pinTobcm_BPI_R2    },
  { NULL,		0, 0, 1, 2, BPI_MAKER_SINOVOIP, 0, NULL, NULL, NULL 	},
} ;

static struct BPIBoards *bpi_find_board_by_name(const char *hardware)
{
  struct BPIBoards *board;

  for (board = bpiboard ; board->name != NULL ; ++board)
    if (strcmp(board->name, hardware) == 0)
      return board;

  return NULL;
}

static int bpi_model_is_rk3576(int model)
{
  return model == BPI_MODEL_M5PRO || model == BPI_MODEL_CM5PRO;
}

static int bpi_model_is_rk3528(int model)
{
  return model == BPI_MODEL_M1SUPER;
}

static int bpi_model_is_rk3506(int model)
{
  return model == BPI_MODEL_FORGE1;
}

static int bpi_model_is_rk3308(int model)
{
  return model == BPI_MODEL_P2PRO;
}

static int bpi_model_is_rk3588(int model)
{
  return model == BPI_MODEL_M7 ||
      model == BPI_MODEL_W3 ||
      model == BPI_MODEL_AIM7;
}

static int bpi_model_is_rockchip(int model)
{
  return model == BPI_MODEL_R2PRO ||
      model == BPI_MODEL_M4SUPER ||
      bpi_model_is_rk3308(model) ||
      bpi_model_is_rk3506(model) ||
      bpi_model_is_rk3528(model) ||
      bpi_model_is_rk3576(model) ||
      bpi_model_is_rk3588(model);
}

static int bpi_model_is_realtek(int model)
{
  return model == BPI_MODEL_W2 ||
      model == BPI_MODEL_M4;
}

static int bpi_model_is_vs680(int model)
{
  return model == BPI_MODEL_M6;
}

static void bpi_select_realtek_backend(int model)
{
  if (model == BPI_MODEL_M4) {
    realtek_gpio_base = realtek_gpio_base_rtd139x;
    realtek_gpio_groups = realtek_rtd139x_groups;
    realtek_gpio_group_count = 1;
    realtek_gpio_map_size = REALTEK_GPIO_MAP_SIZE;
    return;
  }

  realtek_gpio_base = realtek_gpio_base_rtd129x;
  realtek_gpio_groups = realtek_rtd129x_groups;
  realtek_gpio_group_count = REALTEK_GPIO_GROUPS;
  realtek_gpio_map_size = REALTEK_GPIO_MAP_SIZE;
}

static void rockchip_select_gpio_v1_regs(void)
{
  rockchip_gpio_v2 = 0;
  rockchip_gpio_swport_dr = ROCKCHIP_GPIO_SWPORT_DR_V1;
  rockchip_gpio_swport_ddr = ROCKCHIP_GPIO_SWPORT_DDR_V1;
  rockchip_gpio_ext_port = ROCKCHIP_GPIO_EXT_PORT_V1;
}

static void rockchip_select_gpio_v2_regs(void)
{
  rockchip_gpio_v2 = 1;
  rockchip_gpio_swport_dr = ROCKCHIP_GPIO_SWPORT_DR_V2;
  rockchip_gpio_swport_ddr = ROCKCHIP_GPIO_SWPORT_DDR_V2;
  rockchip_gpio_ext_port = ROCKCHIP_GPIO_EXT_PORT_V2;
}

static void bpi_select_rockchip_backend(int model)
{
  rockchip_select_gpio_v2_regs();

  if (bpi_model_is_rk3308(model)) {
    rockchip_gpio_base = rockchip_gpio_base_rk3308;
    rockchip_gpio_map_size = ROCKCHIP_GPIO_MAP_SIZE_RK3308;
    rockchip_select_gpio_v1_regs();
    return;
  }

  if (bpi_model_is_rk3506(model)) {
    rockchip_gpio_base = rockchip_gpio_base_rk3506;
    rockchip_gpio_map_size = ROCKCHIP_GPIO_MAP_SIZE_RK3506;
    return;
  }

  if (bpi_model_is_rk3528(model)) {
    rockchip_gpio_base = rockchip_gpio_base_rk3528;
    rockchip_gpio_map_size = ROCKCHIP_GPIO_MAP_SIZE_RK3528;
    return;
  }

  if (bpi_model_is_rk3576(model)) {
    rockchip_gpio_base = rockchip_gpio_base_rk3576;
    rockchip_gpio_map_size = ROCKCHIP_GPIO_MAP_SIZE_RK3576;
    return;
  }

  if (bpi_model_is_rk3588(model)) {
    rockchip_gpio_base = rockchip_gpio_base_rk3588;
    rockchip_gpio_map_size = ROCKCHIP_GPIO_MAP_SIZE_RK3588;
    return;
  }

  rockchip_gpio_base = rockchip_gpio_base_rk3568;
  rockchip_gpio_map_size = ROCKCHIP_GPIO_MAP_SIZE_RK3568;
}

static struct BPIBoards *bpi_find_board_by_model_string(const char *hardware)
{
  if (strstr(hardware, "BananaPi M4 Berry") ||
      strstr(hardware, "Banana Pi BPI-M4 Berry") ||
      strstr(hardware, "BPI-M4Berry"))
    return bpi_find_board_by_name("bpi-m4berry");

  if (strstr(hardware, "Banana Pi BPI-W2") ||
      strstr(hardware, "BananaPi BPI-W2") ||
      strstr(hardware, "Banana Pi W2") ||
      strstr(hardware, "BananaPi W2") ||
      strstr(hardware, "BPI-W2") ||
      strstr(hardware, "rtd-1296-bananapi-w2") ||
      strstr(hardware, "Realtek_RTD1296"))
    return bpi_find_board_by_name("bpi-w2");

  if (strstr(hardware, "BananaPi BPI-M4-Zero") ||
      strstr(hardware, "Banana Pi BPI-M4-Zero") ||
      strstr(hardware, "BananaPi M4 Zero") ||
      strstr(hardware, "BPI-M4Zero"))
    return bpi_find_board_by_name("bpi-m4zero");

  if (strstr(hardware, "BananaPi M2S") ||
      strstr(hardware, "BananaPi BPI-M2S") ||
      strstr(hardware, "Banana Pi BPI-M2S") ||
      strstr(hardware, "Banana Pi M2S") ||
      strstr(hardware, "BPI-M2S"))
    return bpi_find_board_by_name("bpi-m2s");

  if (strstr(hardware, "Bananapi BPI-CM4") ||
      strstr(hardware, "BananaPi BPI-CM4") ||
      strstr(hardware, "Banana Pi BPI-CM4") ||
      strstr(hardware, "BananaPi BPI-CM4IO") ||
      strstr(hardware, "Banana Pi BPI-CM4IO") ||
      strstr(hardware, "BPI-CM4IO") ||
      strstr(hardware, "BPI-CM4"))
    return bpi_find_board_by_name("bpi-cm4io");

  if (strstr(hardware, "Banana Pi BPI-CM5 Pro") ||
      strstr(hardware, "BananaPi BPI-CM5 Pro") ||
      strstr(hardware, "Banana Pi CM5 Pro") ||
      strstr(hardware, "BananaPi CM5 Pro") ||
      strstr(hardware, "BPI-CM5 Pro") ||
      strstr(hardware, "ArmSoM CM5 IO") ||
      strstr(hardware, "armsom,cm5-io") ||
      strstr(hardware, "rk3576-armsom-cm5-io"))
    return bpi_find_board_by_name("bpi-cm5-pro");

  if (strstr(hardware, "Banana Pi BPI-M5 Pro") ||
      strstr(hardware, "BananaPi BPI-M5 Pro") ||
      strstr(hardware, "Banana Pi M5 Pro") ||
      strstr(hardware, "BananaPi M5 Pro") ||
      strstr(hardware, "BPI-M5 Pro") ||
      strstr(hardware, "rk3576-bananapi-m5-pro"))
    return bpi_find_board_by_name("bpi-m5-pro");

  if (strstr(hardware, "Banana Pi BPI-M7") ||
      strstr(hardware, "BananaPi BPI-M7") ||
      strstr(hardware, "Banana Pi M7") ||
      strstr(hardware, "BananaPi M7") ||
      strstr(hardware, "BPI-M7") ||
      strstr(hardware, "bananapi,m7") ||
      strstr(hardware, "rk3588-bananapi-m7"))
    return bpi_find_board_by_name("bpi-m7");

  if (strstr(hardware, "Banana Pi BPI-W3") ||
      strstr(hardware, "BananaPi BPI-W3") ||
      strstr(hardware, "Banana Pi W3") ||
      strstr(hardware, "BananaPi W3") ||
      strstr(hardware, "BPI-W3") ||
      strstr(hardware, "ArmSoM W3") ||
      strstr(hardware, "armsom w3") ||
      strstr(hardware, "bananapi,bpi-w3") ||
      strstr(hardware, "armsom,w3") ||
      strstr(hardware, "rk3588-bananapi-w3") ||
      strstr(hardware, "rk3588-armsom-w3"))
    return bpi_find_board_by_name("bpi-w3");

  if (strstr(hardware, "Banana Pi BPI-AIM7") ||
      strstr(hardware, "BananaPi BPI-AIM7") ||
      strstr(hardware, "Banana Pi AIM7") ||
      strstr(hardware, "BananaPi AIM7") ||
      strstr(hardware, "BPI-AIM7") ||
      strstr(hardware, "ArmSoM AIM7 IO") ||
      strstr(hardware, "ArmSoM AIM7") ||
      strstr(hardware, "armsom,aim7-io") ||
      strstr(hardware, "armsom,aim7") ||
      strstr(hardware, "rk3588-armsom-aim7-io"))
    return bpi_find_board_by_name("bpi-aim7");

  if (strstr(hardware, "Banana Pi BPI-M4 Super") ||
      strstr(hardware, "BananaPi BPI-M4 Super") ||
      strstr(hardware, "Banana Pi M4 Super") ||
      strstr(hardware, "BananaPi M4 Super") ||
      strstr(hardware, "BPI-M4 Super") ||
      strstr(hardware, "ArmSom Sige3") ||
      strstr(hardware, "ArmSoM Sige3") ||
      strstr(hardware, "armsom,sige3") ||
      strstr(hardware, "rk3568-armsom-sige3"))
    return bpi_find_board_by_name("bpi-m4-super");

  if (strstr(hardware, "Sinovoip_Bananapi_M4") ||
      strstr(hardware, "Banana Pi BPI-M4") ||
      strstr(hardware, "BananaPi BPI-M4") ||
      strstr(hardware, "Banana Pi M4") ||
      strstr(hardware, "BananaPi M4") ||
      strstr(hardware, "BPI-M4") ||
      strstr(hardware, "rtd-1395-bananapi-m4"))
    return bpi_find_board_by_name("bpi-m4");

  if (strstr(hardware, "Banana Pi BPI-M6") ||
      strstr(hardware, "BananaPi BPI-M6") ||
      strstr(hardware, "Banana Pi M6") ||
      strstr(hardware, "BananaPi M6") ||
      strstr(hardware, "BPI-M6") ||
      strstr(hardware, "Synaptics VS680 EVK") ||
      strstr(hardware, "vs680-a0-bananapi-m6"))
    return bpi_find_board_by_name("bpi-m6");

  if (strstr(hardware, "Banana Pi BPI-M1 Super") ||
      strstr(hardware, "BananaPi BPI-M1 Super") ||
      strstr(hardware, "Banana Pi M1 Super") ||
      strstr(hardware, "BananaPi M1 Super") ||
      strstr(hardware, "BPI-M1 Super") ||
      strstr(hardware, "Banana Pi BPI-M1S") ||
      strstr(hardware, "BananaPi BPI-M1S") ||
      strstr(hardware, "Banana Pi M1S") ||
      strstr(hardware, "BananaPi M1S") ||
      strstr(hardware, "BPI-M1S") ||
      strstr(hardware, "ArmSom Sige1") ||
      strstr(hardware, "ArmSoM Sige1") ||
      strstr(hardware, "armsom,sige1") ||
      strstr(hardware, "rk3528-armsom-sige1"))
    return bpi_find_board_by_name("bpi-m1-super");

  if (strstr(hardware, "Banana Pi BPI-Forge1") ||
      strstr(hardware, "BananaPi BPI-Forge1") ||
      strstr(hardware, "Banana Pi Forge1") ||
      strstr(hardware, "BananaPi Forge1") ||
      strstr(hardware, "BPI-Forge1") ||
      strstr(hardware, "ArmSom Forge1") ||
      strstr(hardware, "ArmSoM Forge1") ||
      strstr(hardware, "armsom,forge1") ||
      strstr(hardware, "rockchip,rk3506J-armsom-forge1") ||
      strstr(hardware, "rockchip,rk3506j-armsom-forge1") ||
      strstr(hardware, "rk3506b-armsom-forge1"))
    return bpi_find_board_by_name("bpi-forge1");

  if (strstr(hardware, "Banana Pi BPI-P2 Pro") ||
      strstr(hardware, "BananaPi BPI-P2 Pro") ||
      strstr(hardware, "Banana Pi P2 Pro") ||
      strstr(hardware, "BananaPi P2 Pro") ||
      strstr(hardware, "BPI-P2 Pro") ||
      strstr(hardware, "ArmSom P2 Pro") ||
      strstr(hardware, "ArmSoM P2 Pro") ||
      strstr(hardware, "armsom,p2pro") ||
      strstr(hardware, "sinovoip,rk3308-bpi-p2pro") ||
      strstr(hardware, "rk3308-bpi-p2-pro"))
    return bpi_find_board_by_name("bpi-p2-pro");

  if (strstr(hardware, "Banana Pi BPI-M5") ||
      strstr(hardware, "BananaPi BPI-M5") ||
      strstr(hardware, "Banana Pi M5") ||
      strstr(hardware, "BananaPi M5") ||
      strstr(hardware, "BPI-M5"))
    return bpi_find_board_by_name("bpi-m5");

  if (strstr(hardware, "Banana Pi BPI-M2-PRO") ||
      strstr(hardware, "Banana Pi BPI-M2 Pro") ||
      strstr(hardware, "BananaPi BPI-M2-PRO") ||
      strstr(hardware, "BananaPi BPI-M2 Pro") ||
      strstr(hardware, "Banana Pi M2Pro") ||
      strstr(hardware, "Banana Pi M2 Pro") ||
      strstr(hardware, "BananaPi M2Pro") ||
      strstr(hardware, "BPI-M2-PRO") ||
      strstr(hardware, "BPI-M2-Pro") ||
      strstr(hardware, "BPI-M2 Pro"))
    return bpi_find_board_by_name("bpi-m2pro");

  if (strstr(hardware, "BananaPi BPI-F3") ||
      strstr(hardware, "Banana Pi BPI-F3") ||
      strstr(hardware, "BananaPi F3") ||
      strstr(hardware, "Banana Pi F3") ||
      strstr(hardware, "BPI-F3") ||
      strstr(hardware, "k1-x deb1"))
    return bpi_find_board_by_name("bpi-f3");

  if (strstr(hardware, "BananaPi BPI-AI2N") ||
      strstr(hardware, "Banana Pi BPI-AI2N") ||
      strstr(hardware, "BananaPi AI2N") ||
      strstr(hardware, "Banana Pi AI2N") ||
      strstr(hardware, "BPI-AI2N"))
    return bpi_find_board_by_name("bpi-ai2n");

  if (strstr(hardware, "Bananapi-R2 Pro") ||
      strstr(hardware, "BananaPi BPI-R2 Pro") ||
      strstr(hardware, "Banana Pi BPI-R2 Pro") ||
      strstr(hardware, "BananaPi R2 Pro") ||
      strstr(hardware, "Banana Pi R2 Pro") ||
      strstr(hardware, "BPI-R2 Pro") ||
      strstr(hardware, "rk3568-bpi-r2pro"))
    return bpi_find_board_by_name("bpi-r2-pro");

  return NULL;
}

static int bpi_set_layout_from_board(struct BPIBoards *board, int *gpioLayout)
{
  if (board == NULL)
    return 0;

  *gpioLayout = board->model;
  if (*gpioLayout >= BPI_MODEL_MIN) {
    bpi_found = 1;
    return 1;
  }

  return 0;
}



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

static volatile uint32_t *meson_gpio_map = NULL;
static volatile uint32_t *meson_gpioao_map = NULL;

static int meson_gpio_mapped(void)
{
    return meson_gpio_map != NULL && meson_gpioao_map != NULL;
}

static int meson_is_ao_pin(int pin)
{
    return pin >= MESON_GPIOAO_PIN_START && pin <= MESON_GPIOAO_PIN_END;
}

static volatile uint32_t *meson_gpio_regs(int pin)
{
    return meson_is_ao_pin(pin) ? meson_gpioao_map : meson_gpio_map;
}

static int meson_gpio_shift(int pin)
{
    if (pin >= MESON_GPIOH_PIN_START && pin <= MESON_GPIOH_PIN_END)
        return pin - MESON_GPIOH_PIN_START;
    if (pin >= MESON_GPIOA_PIN_START && pin <= MESON_GPIOA_PIN_END)
        return pin - MESON_GPIOA_PIN_START;
    if (pin >= MESON_GPIOX_PIN_START && pin <= MESON_GPIOX_PIN_END)
        return pin - MESON_GPIOX_PIN_START;
    if (pin >= MESON_GPIOAO_PIN_START && pin <= MESON_GPIOAO_PIN_END)
        return pin - MESON_GPIOAO_PIN_START;

    return -1;
}

static int meson_gpio_fsel_offset(int pin)
{
    if (pin >= MESON_GPIOH_PIN_START && pin <= MESON_GPIOH_PIN_END)
        return MESON_GPIOH_FSEL_REG_OFFSET;
    if (pin >= MESON_GPIOA_PIN_START && pin <= MESON_GPIOA_PIN_END)
        return MESON_GPIOA_FSEL_REG_OFFSET;
    if (pin >= MESON_GPIOX_PIN_START && pin <= MESON_GPIOX_PIN_END)
        return MESON_GPIOX_FSEL_REG_OFFSET;
    if (pin >= MESON_GPIOAO_PIN_START && pin <= MESON_GPIOAO_PIN_END)
        return MESON_GPIOAO_FSEL_REG_OFFSET;

    return -1;
}

static int meson_gpio_out_offset(int pin)
{
    if (pin >= MESON_GPIOH_PIN_START && pin <= MESON_GPIOH_PIN_END)
        return MESON_GPIOH_OUTP_REG_OFFSET;
    if (pin >= MESON_GPIOA_PIN_START && pin <= MESON_GPIOA_PIN_END)
        return MESON_GPIOA_OUTP_REG_OFFSET;
    if (pin >= MESON_GPIOX_PIN_START && pin <= MESON_GPIOX_PIN_END)
        return MESON_GPIOX_OUTP_REG_OFFSET;
    if (pin >= MESON_GPIOAO_PIN_START && pin <= MESON_GPIOAO_PIN_END)
        return MESON_GPIOAO_OUTP_REG_OFFSET;

    return -1;
}

static int meson_gpio_in_offset(int pin)
{
    if (pin >= MESON_GPIOH_PIN_START && pin <= MESON_GPIOH_PIN_END)
        return MESON_GPIOH_INP_REG_OFFSET;
    if (pin >= MESON_GPIOA_PIN_START && pin <= MESON_GPIOA_PIN_END)
        return MESON_GPIOA_INP_REG_OFFSET;
    if (pin >= MESON_GPIOX_PIN_START && pin <= MESON_GPIOX_PIN_END)
        return MESON_GPIOX_INP_REG_OFFSET;
    if (pin >= MESON_GPIOAO_PIN_START && pin <= MESON_GPIOAO_PIN_END)
        return MESON_GPIOAO_INP_REG_OFFSET;

    return -1;
}

static int meson_gpio_puen_offset(int pin)
{
    if (pin >= MESON_GPIOH_PIN_START && pin <= MESON_GPIOH_PIN_END)
        return MESON_GPIOH_PUEN_REG_OFFSET;
    if (pin >= MESON_GPIOA_PIN_START && pin <= MESON_GPIOA_PIN_END)
        return MESON_GPIOA_PUEN_REG_OFFSET;
    if (pin >= MESON_GPIOX_PIN_START && pin <= MESON_GPIOX_PIN_END)
        return MESON_GPIOX_PUEN_REG_OFFSET;
    if (pin >= MESON_GPIOAO_PIN_START && pin <= MESON_GPIOAO_PIN_END)
        return MESON_GPIOAO_PUEN_REG_OFFSET;

    return -1;
}

static int meson_gpio_pupd_offset(int pin)
{
    if (pin >= MESON_GPIOH_PIN_START && pin <= MESON_GPIOH_PIN_END)
        return MESON_GPIOH_PUPD_REG_OFFSET;
    if (pin >= MESON_GPIOA_PIN_START && pin <= MESON_GPIOA_PIN_END)
        return MESON_GPIOA_PUPD_REG_OFFSET;
    if (pin >= MESON_GPIOX_PIN_START && pin <= MESON_GPIOX_PIN_END)
        return MESON_GPIOX_PUPD_REG_OFFSET;
    if (pin >= MESON_GPIOAO_PIN_START && pin <= MESON_GPIOAO_PIN_END)
        return MESON_GPIOAO_PUPD_REG_OFFSET;

    return -1;
}

static int meson_gpio_mux_offset(int pin)
{
    if (pin >= MESON_GPIOH_PIN_START && pin <= MESON_GPIOH_PIN_END)
        return MESON_GPIOH_MUX_B_REG_OFFSET;
    if (pin >= MESON_GPIOA_PIN_START && pin <= MESON_GPIOA_PIN_START + 7)
        return MESON_GPIOA_MUX_D_REG_OFFSET;
    if (pin >= MESON_GPIOA_PIN_START + 8 && pin <= MESON_GPIOA_PIN_END)
        return MESON_GPIOA_MUX_E_REG_OFFSET;
    if (pin >= MESON_GPIOX_PIN_START && pin <= MESON_GPIOX_PIN_START + 7)
        return MESON_GPIOX_MUX_3_REG_OFFSET;
    if (pin >= MESON_GPIOX_PIN_START + 8 && pin <= MESON_GPIOX_PIN_MID)
        return MESON_GPIOX_MUX_4_REG_OFFSET;
    if (pin > MESON_GPIOX_PIN_MID && pin <= MESON_GPIOX_PIN_END)
        return MESON_GPIOX_MUX_5_REG_OFFSET;
    if (pin >= MESON_GPIOAO_PIN_START && pin <= MESON_GPIOAO_PIN_START + 7)
        return MESON_GPIOAO_MUX_REG0_OFFSET;
    if (pin >= MESON_GPIOAO_PIN_START + 8 && pin <= MESON_GPIOAO_PIN_END)
        return MESON_GPIOAO_MUX_REG1_OFFSET;

    return -1;
}

static void meson_update_reg(int pin, int offset, uint32_t clear, uint32_t set)
{
    uint32_t regval;
    volatile uint32_t *reg;

    if (!meson_gpio_mapped() || offset < 0)
        return;

    reg = meson_gpio_regs(pin) + offset;
    regval = *reg;
    regval &= ~clear;
    regval |= set;
    *reg = regval;
}

static void meson_set_gpio_mode(int pin, int direction)
{
    int shift = meson_gpio_shift(pin);
    int mux = meson_gpio_mux_offset(pin);
    int fsel = meson_gpio_fsel_offset(pin);
    unsigned int mux_shift;

    if (!meson_gpio_mapped() || shift < 0 || mux < 0 || fsel < 0)
        return;

    mux_shift = (unsigned int)(shift & 0x7) * 4;
    meson_update_reg(pin, mux, 0xFu << mux_shift, 0);
    if (direction == INPUT)
        meson_update_reg(pin, fsel, 0, 1u << shift);
    else if (direction == OUTPUT)
        meson_update_reg(pin, fsel, 1u << shift, 0);
}

int meson_gpio_function(int pin)
{
    int shift = meson_gpio_shift(pin);
    int mux = meson_gpio_mux_offset(pin);
    int fsel = meson_gpio_fsel_offset(pin);
    unsigned int mux_shift;
    uint32_t mode;

    if (!meson_gpio_mapped() || shift < 0 || mux < 0 || fsel < 0)
        return 0;

    mux_shift = (unsigned int)(shift & 0x7) * 4;
    mode = (*(meson_gpio_regs(pin) + mux) >> mux_shift) & 0xFu;
    if (mode != 0)
        return (int)mode + 1;

    return (*(meson_gpio_regs(pin) + fsel) & (1u << shift)) ? 0 : 1;
}

void meson_set_pullupdn(int pin, int pud)
{
    int shift = meson_gpio_shift(pin);
    int puen = meson_gpio_puen_offset(pin);
    int pupd = meson_gpio_pupd_offset(pin);

    if (!meson_gpio_mapped() || shift < 0 || puen < 0 || pupd < 0)
        return;

    if (pud == PUD_OFF) {
        meson_update_reg(pin, puen, 1u << shift, 0);
        return;
    }

    meson_update_reg(pin, pupd, 1u << shift, pud == PUD_UP ? 1u << shift : 0);
    meson_update_reg(pin, puen, 0, 1u << shift);
}

void meson_setup_gpio(int pin, int direction, int pud)
{
    meson_set_pullupdn(pin, pud);
    meson_set_gpio_mode(pin, direction);
}

void meson_output_gpio(int pin, int value)
{
    int shift = meson_gpio_shift(pin);
    int offset = meson_gpio_out_offset(pin);

    if (!meson_gpio_mapped() || shift < 0 || offset < 0)
        return;

    meson_update_reg(pin, offset, value == 0 ? 1u << shift : 0, value == 0 ? 0 : 1u << shift);
}

int meson_input_gpio(int pin)
{
    int shift = meson_gpio_shift(pin);
    int offset = meson_gpio_in_offset(pin);

    if (!meson_gpio_mapped() || shift < 0 || offset < 0)
        return 0;

    return (*(meson_gpio_regs(pin) + offset) & (1u << shift)) ? 1 : 0;
}

int meson_setup(void)
{
    int mem_fd;

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0)
        return SETUP_DEVMEM_FAIL;

    meson_gpio_map = (uint32_t *)mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE,
                                      MAP_SHARED, mem_fd, MESON_GPIO_BASE_ADDR);
    meson_gpioao_map = (uint32_t *)mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE,
                                        MAP_SHARED, mem_fd, MESON_GPIO_AO_BASE_ADDR);
    close(mem_fd);

    if (meson_gpio_map == MAP_FAILED || meson_gpioao_map == MAP_FAILED) {
        meson_gpio_map = NULL;
        meson_gpioao_map = NULL;
        return SETUP_MMAP_FAIL;
    }

    return SETUP_OK;
}

static int spacemit_gpio_mapped(void)
{
    return spacemit_gpio_map != NULL && spacemit_pinctrl_map != NULL;
}

static int spacemit_is_pin(int pin)
{
    return pin >= SPACEMIT_GPIO_PIN_BASE && pin <= SPACEMIT_GPIO_PIN_END;
}

static int spacemit_mfpr_offset(int pin)
{
    if (pin < SPACEMIT_GPIO_PIN_BASE || pin > SPACEMIT_GPIO_PIN_END)
        return -1;
    if (pin <= 85)
        return (pin + 1) << 2;
    if (pin <= 92)
        return ((pin + 1) << 2) + 0x90;

    return ((pin + 1) << 2) + 0x4C;
}

static int spacemit_gpio_alt(int pin)
{
    if ((pin >= 70 && pin <= 73) || (pin >= 93 && pin <= 103))
        return 1;
    if (pin >= 104 && pin <= 109)
        return 4;

    return 0;
}

static int spacemit_bank_offset(int pin)
{
    int bank = pin >> 5;

    return bank == 3 ? SPACEMIT_BANK3_OFFSET : SPACEMIT_BANK012_OFFSET(bank);
}

static int spacemit_pin_shift(int pin)
{
    return pin & 0x1F;
}

static void spacemit_update_reg(volatile uint32_t *base, int offset, uint32_t clear, uint32_t set)
{
    volatile uint32_t *reg;
    uint32_t regval;

    if (!spacemit_gpio_mapped() || offset < 0)
        return;

    reg = base + (offset >> 2);
    regval = *reg;
    regval &= ~clear;
    regval |= set;
    *reg = regval;
}

static void spacemit_set_gpio_mode(int pin, int direction)
{
    int mfpr = spacemit_mfpr_offset(pin);
    int bank = spacemit_bank_offset(pin);
    int shift = spacemit_pin_shift(pin);
    int dir_offset;

    if (!spacemit_is_pin(pin) || mfpr < 0)
        return;

    spacemit_update_reg(spacemit_pinctrl_map, mfpr, SPACEMIT_AF_SEL_MASK,
                        (uint32_t)spacemit_gpio_alt(pin) << SPACEMIT_AF_SEL_OFFSET);

    if (direction == INPUT)
        dir_offset = bank + SPACEMIT_GCDR;
    else if (direction == OUTPUT)
        dir_offset = bank + SPACEMIT_GSDR;
    else
        return;

    spacemit_update_reg(spacemit_gpio_map, dir_offset, 0, 1u << shift);
}

int spacemit_gpio_function(int pin)
{
    int mfpr = spacemit_mfpr_offset(pin);
    int bank = spacemit_bank_offset(pin);
    int shift = spacemit_pin_shift(pin);
    uint32_t af_sel;

    if (!spacemit_is_pin(pin) || mfpr < 0 || !spacemit_gpio_mapped())
        return INPUT;

    af_sel = (*(spacemit_pinctrl_map + (mfpr >> 2))) & SPACEMIT_AF_SEL_MASK;
    if (af_sel != (uint32_t)spacemit_gpio_alt(pin))
        return (int)af_sel + 2;

    return (*(spacemit_gpio_map + ((bank + SPACEMIT_GPDR) >> 2)) & (1u << shift)) ? OUTPUT : INPUT;
}

void spacemit_set_pullupdn(int pin, int pud)
{
    int mfpr = spacemit_mfpr_offset(pin);
    uint32_t pull = SPACEMIT_PULL_DIS;

    if (!spacemit_is_pin(pin) || mfpr < 0)
        return;

    if (pud == PUD_UP)
        pull = SPACEMIT_PULL_UP;
    else if (pud == PUD_DOWN)
        pull = SPACEMIT_PULL_DOWN;

    spacemit_update_reg(spacemit_pinctrl_map, mfpr, SPACEMIT_PULL_MASK,
                        (pull & 0x7) << SPACEMIT_PULL_OFFSET);
}

void spacemit_setup_gpio(int pin, int direction, int pud)
{
    spacemit_set_pullupdn(pin, pud);
    spacemit_set_gpio_mode(pin, direction);
}

void spacemit_output_gpio(int pin, int value)
{
    int bank = spacemit_bank_offset(pin);
    int shift = spacemit_pin_shift(pin);
    int offset;

    if (!spacemit_is_pin(pin) || !spacemit_gpio_mapped())
        return;

    offset = bank + (value == 0 ? SPACEMIT_GPCR : SPACEMIT_GPSR);
    spacemit_update_reg(spacemit_gpio_map, offset, 0, 1u << shift);
}

int spacemit_input_gpio(int pin)
{
    int bank = spacemit_bank_offset(pin);
    int shift = spacemit_pin_shift(pin);

    if (!spacemit_is_pin(pin) || !spacemit_gpio_mapped())
        return 0;

    return (*(spacemit_gpio_map + ((bank + SPACEMIT_GPLR) >> 2)) & (1u << shift)) ? 1 : 0;
}

int spacemit_setup(void)
{
    int mem_fd;

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0)
        return SETUP_DEVMEM_FAIL;

    spacemit_gpio_map = (uint32_t *)mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE,
                                         MAP_SHARED, mem_fd, SPACEMIT_GPIO_BASE_ADDR);
    spacemit_pinctrl_map = (uint32_t *)mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE,
                                            MAP_SHARED, mem_fd, SPACEMIT_PINCTRL_BASE_ADDR);
    close(mem_fd);

    if (spacemit_gpio_map == MAP_FAILED || spacemit_pinctrl_map == MAP_FAILED) {
        spacemit_gpio_map = NULL;
        spacemit_pinctrl_map = NULL;
        return SETUP_MMAP_FAIL;
    }

    return SETUP_OK;
}

static int renesas_gpio_mapped(void)
{
    return renesas_gpio_map != NULL;
}

static int renesas_is_pin(int pin)
{
    return pin >= RENESAS_GPIO_PIN_BASE && pin <= RENESAS_GPIO_PIN_END;
}

static void renesas_update_reg(int offset, uint32_t clear, uint32_t set)
{
    volatile uint32_t *reg;
    uint32_t regval;

    if (!renesas_gpio_mapped() || offset < 0)
        return;

    reg = renesas_gpio_map + (offset >> 2);
    regval = *reg;
    regval &= ~clear;
    regval |= set;
    *reg = regval;
}

static void renesas_set_gpio_mode(int pin, int direction)
{
    int offset, port, bit, pmc_phyaddr, pmc_shift, pm_phyaddr, pm_shift;
    uint32_t pmc_mask, pm_mask, pm_value;

    if (!renesas_is_pin(pin))
        return;

    offset = RENESAS_PIN_OFFSET(pin);
    port = RENESAS_PIN_ID_TO_PORT_OFFSET(offset);
    bit = RENESAS_PIN_ID_TO_PIN(offset);
    pmc_phyaddr = RENESAS_PMC(port);
    pmc_shift = (pmc_phyaddr % 4) * 8;
    pm_phyaddr = RENESAS_PM(port);
    pm_shift = (pm_phyaddr % 4) * 8;

    pmc_mask = (1u << bit) << pmc_shift;
    pm_mask = (0x3u << (bit * 2)) << pm_shift;

    if (direction == INPUT)
        pm_value = ((uint32_t)RENESAS_PM_INPUT << (bit * 2)) << pm_shift;
    else if (direction == OUTPUT)
        pm_value = ((uint32_t)RENESAS_PM_OUTPUT << (bit * 2)) << pm_shift;
    else
        return;

    renesas_update_reg(pmc_phyaddr, pmc_mask, 0);
    renesas_update_reg(pm_phyaddr, pm_mask, pm_value);
}

int renesas_gpio_function(int pin)
{
    int offset, port, bit, pmc_phyaddr, pmc_shift, pm_phyaddr, pm_shift;
    uint32_t mode, gpiomode;

    if (!renesas_is_pin(pin) || !renesas_gpio_mapped())
        return INPUT;

    offset = RENESAS_PIN_OFFSET(pin);
    port = RENESAS_PIN_ID_TO_PORT_OFFSET(offset);
    bit = RENESAS_PIN_ID_TO_PIN(offset);
    pmc_phyaddr = RENESAS_PMC(port);
    pmc_shift = (pmc_phyaddr % 4) * 8;
    pm_phyaddr = RENESAS_PM(port);
    pm_shift = (pm_phyaddr % 4) * 8;

    mode = (*(renesas_gpio_map + (pmc_phyaddr >> 2)) >> pmc_shift) & (1u << bit);
    if (!mode) {
        gpiomode = *(renesas_gpio_map + (pm_phyaddr >> 2)) >> pm_shift;
        gpiomode = (gpiomode >> (bit * 2)) & 0x3;
        if (gpiomode == RENESAS_PM_OUTPUT)
            return OUTPUT;
        if (gpiomode == RENESAS_PM_INPUT)
            return INPUT;
        return INPUT;
    }

    mode = *(renesas_gpio_map + (RENESAS_PFC(port) >> 2));
    mode = (mode >> (bit * 4)) & 0xf;
    return (int)mode + 2;
}

void renesas_set_pullupdn(int pin, int pud)
{
    int offset, port, port_offset, bit, pupd_phyaddr;
    uint32_t pull = RENESAS_PULL_DIS;

    if (!renesas_is_pin(pin))
        return;

    offset = RENESAS_PIN_OFFSET(pin);
    port = RENESAS_PIN_ID_TO_PORT_OFFSET(offset);
    port_offset = port + RENESAS_EXTENDED_REG_OFFSET;
    bit = RENESAS_PIN_ID_TO_PIN(offset);
    pupd_phyaddr = RENESAS_PUPD(port_offset);

    if (bit >= 4) {
        bit -= 4;
        pupd_phyaddr += 4;
    }

    if (pud == PUD_UP)
        pull = RENESAS_PULL_UP;
    else if (pud == PUD_DOWN)
        pull = RENESAS_PULL_DOWN;

    renesas_update_reg(pupd_phyaddr, 0x3u << (bit * 8), pull << (bit * 8));
}

void renesas_setup_gpio(int pin, int direction, int pud)
{
    renesas_set_pullupdn(pin, pud);
    renesas_set_gpio_mode(pin, direction);
}

void renesas_output_gpio(int pin, int value)
{
    int offset, port, bit, p_phyaddr, p_shift;
    uint32_t bit_mask;

    if (!renesas_is_pin(pin))
        return;

    offset = RENESAS_PIN_OFFSET(pin);
    port = RENESAS_PIN_ID_TO_PORT_OFFSET(offset);
    bit = RENESAS_PIN_ID_TO_PIN(offset);
    p_phyaddr = RENESAS_P(port);
    p_shift = (p_phyaddr % 4) * 8;
    bit_mask = (1u << bit) << p_shift;

    renesas_update_reg(p_phyaddr, bit_mask, value == 0 ? 0 : bit_mask);
}

int renesas_input_gpio(int pin)
{
    int offset, port, bit, p_phyaddr, p_shift, pm_phyaddr, pm_shift;
    int pin_phyaddr, pin_shift;
    uint32_t gpiomode;

    if (!renesas_is_pin(pin) || !renesas_gpio_mapped())
        return 0;

    offset = RENESAS_PIN_OFFSET(pin);
    port = RENESAS_PIN_ID_TO_PORT_OFFSET(offset);
    bit = RENESAS_PIN_ID_TO_PIN(offset);

    p_phyaddr = RENESAS_P(port);
    p_shift = (p_phyaddr % 4) * 8;
    pm_phyaddr = RENESAS_PM(port);
    pm_shift = (pm_phyaddr % 4) * 8;
    pin_phyaddr = RENESAS_PIN(port);
    pin_shift = (pin_phyaddr % 4) * 8;

    gpiomode = *(renesas_gpio_map + (pm_phyaddr >> 2)) >> pm_shift;
    gpiomode = (gpiomode >> (bit * 2)) & 0x3;

    if (gpiomode == RENESAS_PM_INPUT)
        return ((*(renesas_gpio_map + (pin_phyaddr >> 2)) >> pin_shift) & (1u << bit)) ? 1 : 0;
    if (gpiomode == RENESAS_PM_OUTPUT)
        return ((*(renesas_gpio_map + (p_phyaddr >> 2)) >> p_shift) & (1u << bit)) ? 1 : 0;

    return 0;
}

int renesas_setup(void)
{
    int mem_fd;

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0)
        return SETUP_DEVMEM_FAIL;

    renesas_gpio_map = (uint32_t *)mmap(NULL, RENESAS_GPIO_MAP_SIZE, PROT_READ|PROT_WRITE,
                                        MAP_SHARED, mem_fd, RENESAS_GPIO_BASE_ADDR);
    close(mem_fd);

    if (renesas_gpio_map == MAP_FAILED) {
        renesas_gpio_map = NULL;
        return SETUP_MMAP_FAIL;
    }

    return SETUP_OK;
}

static int rockchip_is_pin(int pin)
{
    return pin >= ROCKCHIP_GPIO_PIN_BASE && pin <= ROCKCHIP_GPIO_PIN_END;
}

static volatile uint32_t *rockchip_bank_regs(int bank)
{
    if (bank < 0 || bank >= ROCKCHIP_GPIO_BANKS)
        return NULL;

    return rockchip_gpio_map[bank];
}

static int rockchip_gpio_mapped(void)
{
    int i;

    for (i = 0; i < ROCKCHIP_GPIO_BANKS; ++i)
        if (rockchip_gpio_map[i] == NULL)
            return 0;

    return 1;
}

static uint32_t rockchip_read_reg(int bank, int offset)
{
    volatile uint32_t *regs = rockchip_bank_regs(bank);

    if (regs == NULL)
        return 0;

    if (rockchip_gpio_v2)
        return regs[offset >> 2] | (regs[(offset + 4) >> 2] << 16);

    return regs[offset >> 2];
}

static void rockchip_write_bit(int bank, int offset, int bit, int value)
{
    volatile uint32_t *regs = rockchip_bank_regs(bank);
    int half_bit;
    uint32_t data;

    if (regs == NULL)
        return;

    if (!rockchip_gpio_v2) {
        data = regs[offset >> 2];
        if (value)
            data |= (1u << bit);
        else
            data &= ~(1u << bit);
        regs[offset >> 2] = data;
        return;
    }

    half_bit = bit & 0xf;
    data = (value ? (1u << half_bit) : 0) | (1u << (half_bit + 16));
    regs[(offset + (bit >= 16 ? 4 : 0)) >> 2] = data;
}

void rockchip_set_pullupdn(int pin, int pud)
{
    (void)pin;
    (void)pud;
}

void rockchip_setup_gpio(int pin, int direction, int pud)
{
    int bank, bit;

    rockchip_set_pullupdn(pin, pud);

    if (!rockchip_is_pin(pin) || !rockchip_gpio_mapped())
        return;

    if (direction != INPUT && direction != OUTPUT)
        return;

    bank = pin >> 5;
    bit = pin & 0x1f;
    rockchip_write_bit(bank, rockchip_gpio_swport_ddr, bit, direction == OUTPUT);
}

int rockchip_gpio_function(int pin)
{
    int bank, bit;

    if (!rockchip_is_pin(pin) || !rockchip_gpio_mapped())
        return INPUT;

    bank = pin >> 5;
    bit = pin & 0x1f;

    return (rockchip_read_reg(bank, rockchip_gpio_swport_ddr) & (1u << bit)) ? OUTPUT : INPUT;
}

void rockchip_output_gpio(int pin, int value)
{
    int bank, bit;

    if (!rockchip_is_pin(pin) || !rockchip_gpio_mapped())
        return;

    bank = pin >> 5;
    bit = pin & 0x1f;
    rockchip_write_bit(bank, rockchip_gpio_swport_dr, bit, value != 0);
}

int rockchip_input_gpio(int pin)
{
    int bank, bit;

    if (!rockchip_is_pin(pin) || !rockchip_gpio_mapped())
        return 0;

    bank = pin >> 5;
    bit = pin & 0x1f;

    return (rockchip_read_reg(bank, rockchip_gpio_ext_port) & (1u << bit)) ? 1 : 0;
}

int rockchip_setup(void)
{
    int mem_fd;
    int i;

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0)
        return SETUP_DEVMEM_FAIL;

    for (i = 0; i < ROCKCHIP_GPIO_BANKS; ++i) {
        rockchip_gpio_map[i] = (uint32_t *)mmap(NULL, rockchip_gpio_map_size,
                                                PROT_READ|PROT_WRITE,
                                                MAP_SHARED, mem_fd,
                                                rockchip_gpio_base[i]);
        if (rockchip_gpio_map[i] == MAP_FAILED) {
            int j;

            rockchip_gpio_map[i] = NULL;
            for (j = 0; j < i; ++j) {
                if (rockchip_gpio_map[j] != NULL) {
                    munmap((void *)rockchip_gpio_map[j], rockchip_gpio_map_size);
                    rockchip_gpio_map[j] = NULL;
                }
            }
            close(mem_fd);
            return SETUP_MMAP_FAIL;
        }
    }

    close(mem_fd);
    return SETUP_OK;
}

static const struct realtek_gpio_group *realtek_group_for_pin(int pin, int *local_pin)
{
    int i;

    for (i = 0; i < realtek_gpio_group_count; ++i) {
        const struct realtek_gpio_group *group = &realtek_gpio_groups[i];

        if (pin < group->pin_base || pin > group->pin_end)
            continue;

        if (local_pin != NULL)
            *local_pin = pin - group->pin_base;
        return group;
    }

    return NULL;
}

static volatile uint32_t *realtek_group_regs(const struct realtek_gpio_group *group)
{
    if (group == NULL || group->map_index < 0 || group->map_index >= REALTEK_GPIO_GROUPS)
        return NULL;

    return realtek_gpio_map[group->map_index];
}

static int realtek_gpio_mapped(void)
{
    int i;

    for (i = 0; i < realtek_gpio_group_count; ++i)
        if (realtek_gpio_map[i] == NULL)
            return 0;

    return 1;
}

static uint32_t realtek_read_reg(const struct realtek_gpio_group *group, int offset)
{
    volatile uint32_t *regs = realtek_group_regs(group);

    if (regs == NULL)
        return 0;

    return regs[offset >> 2];
}

static void realtek_write_bit(const struct realtek_gpio_group *group, int offset, int bit, int value)
{
    volatile uint32_t *regs = realtek_group_regs(group);
    uint32_t data;

    if (regs == NULL)
        return;

    data = regs[offset >> 2];
    if (value)
        data |= (1u << bit);
    else
        data &= ~(1u << bit);
    regs[offset >> 2] = data;
}

void realtek_set_pullupdn(int pin, int pud)
{
    (void)pin;
    (void)pud;
}

void realtek_setup_gpio(int pin, int direction, int pud)
{
    const struct realtek_gpio_group *group;
    int local_pin, index, bit;

    realtek_set_pullupdn(pin, pud);

    if (!realtek_gpio_mapped())
        return;

    group = realtek_group_for_pin(pin, &local_pin);
    if (group == NULL)
        return;

    if (direction != INPUT && direction != OUTPUT)
        return;

    index = local_pin >> 5;
    bit = local_pin & 0x1f;
    realtek_write_bit(group, group->dir_offset[index], bit, direction == OUTPUT);
}

int realtek_gpio_function(int pin)
{
    const struct realtek_gpio_group *group;
    int local_pin, index, bit;

    if (!realtek_gpio_mapped())
        return INPUT;

    group = realtek_group_for_pin(pin, &local_pin);
    if (group == NULL)
        return INPUT;

    index = local_pin >> 5;
    bit = local_pin & 0x1f;
    return (realtek_read_reg(group, group->dir_offset[index]) & (1u << bit)) ? OUTPUT : INPUT;
}

void realtek_output_gpio(int pin, int value)
{
    const struct realtek_gpio_group *group;
    int local_pin, index, bit;

    if (!realtek_gpio_mapped())
        return;

    group = realtek_group_for_pin(pin, &local_pin);
    if (group == NULL)
        return;

    index = local_pin >> 5;
    bit = local_pin & 0x1f;
    realtek_write_bit(group, group->dato_offset[index], bit, value != 0);
}

int realtek_input_gpio(int pin)
{
    const struct realtek_gpio_group *group;
    int local_pin, index, bit, offset;

    if (!realtek_gpio_mapped())
        return 0;

    group = realtek_group_for_pin(pin, &local_pin);
    if (group == NULL)
        return 0;

    index = local_pin >> 5;
    bit = local_pin & 0x1f;
    offset = realtek_gpio_function(pin) == OUTPUT ? group->dato_offset[index] : group->dati_offset[index];
    return (realtek_read_reg(group, offset) & (1u << bit)) ? 1 : 0;
}

int realtek_setup(void)
{
    int mem_fd;
    int i;

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0)
        return SETUP_DEVMEM_FAIL;

    for (i = 0; i < realtek_gpio_group_count; ++i) {
        realtek_gpio_map[i] = (uint32_t *)mmap(NULL, realtek_gpio_map_size,
                                               PROT_READ|PROT_WRITE,
                                               MAP_SHARED, mem_fd,
                                               realtek_gpio_base[i]);
        if (realtek_gpio_map[i] == MAP_FAILED) {
            int j;

            realtek_gpio_map[i] = NULL;
            for (j = 0; j < i; ++j) {
                if (realtek_gpio_map[j] != NULL) {
                    munmap((void *)realtek_gpio_map[j], realtek_gpio_map_size);
                    realtek_gpio_map[j] = NULL;
                }
            }
            close(mem_fd);
            return SETUP_MMAP_FAIL;
        }
    }

    close(mem_fd);
    return SETUP_OK;
}

static int vs680_is_pin(int pin)
{
    return (pin >= VS680_GPIO_SOC_PIN_BASE && pin <= VS680_GPIO_SOC_PIN_END) ||
        (pin >= VS680_GPIO_SM_PIN_BASE && pin <= VS680_GPIO_SM_PIN_END);
}

static int vs680_pin_bank(int pin)
{
    if (pin >= VS680_GPIO_SOC_PIN_BASE && pin <= VS680_GPIO_SOC_PIN_END)
        return pin >> 5;

    if (pin >= VS680_GPIO_SM_PIN_BASE && pin <= VS680_GPIO_SM_PIN_END)
        return 3;

    return -1;
}

static int vs680_pin_bit(int pin)
{
    if (pin >= VS680_GPIO_SM_PIN_BASE)
        return pin - VS680_GPIO_SM_PIN_BASE;

    return pin & 0x1f;
}

static int vs680_gpio_mapped(void)
{
    int i;

    for (i = 0; i < VS680_GPIO_BANKS; ++i)
        if (vs680_gpio_map[i] == NULL)
            return 0;

    return 1;
}

static uint32_t vs680_read_reg(int bank, int offset)
{
    volatile uint32_t *regs;

    if (bank < 0 || bank >= VS680_GPIO_BANKS)
        return 0;

    regs = vs680_gpio_map[bank];
    if (regs == NULL)
        return 0;

    return regs[offset >> 2];
}

static void vs680_write_bit(int bank, int offset, int bit, int value)
{
    volatile uint32_t *regs;
    uint32_t data;

    if (bank < 0 || bank >= VS680_GPIO_BANKS)
        return;

    regs = vs680_gpio_map[bank];
    if (regs == NULL)
        return;

    data = regs[offset >> 2];
    if (value)
        data |= (1u << bit);
    else
        data &= ~(1u << bit);
    regs[offset >> 2] = data;
}

void vs680_set_pullupdn(int pin, int pud)
{
    (void)pin;
    (void)pud;
}

void vs680_setup_gpio(int pin, int direction, int pud)
{
    int bank, bit;

    vs680_set_pullupdn(pin, pud);

    if (!vs680_is_pin(pin) || !vs680_gpio_mapped())
        return;

    bank = vs680_pin_bank(pin);
    bit = vs680_pin_bit(pin);
    vs680_write_bit(bank, VS680_GPIO_SWPORT_DDR, bit, direction == OUTPUT);
}

int vs680_gpio_function(int pin)
{
    int bank, bit;

    if (!vs680_is_pin(pin) || !vs680_gpio_mapped())
        return INPUT;

    bank = vs680_pin_bank(pin);
    bit = vs680_pin_bit(pin);
    return (vs680_read_reg(bank, VS680_GPIO_SWPORT_DDR) & (1u << bit)) ? OUTPUT : INPUT;
}

void vs680_output_gpio(int pin, int value)
{
    int bank, bit;

    if (!vs680_is_pin(pin) || !vs680_gpio_mapped())
        return;

    bank = vs680_pin_bank(pin);
    bit = vs680_pin_bit(pin);
    vs680_write_bit(bank, VS680_GPIO_SWPORT_DR, bit, value != 0);
}

int vs680_input_gpio(int pin)
{
    int bank, bit;

    if (!vs680_is_pin(pin) || !vs680_gpio_mapped())
        return 0;

    bank = vs680_pin_bank(pin);
    bit = vs680_pin_bit(pin);
    return (vs680_read_reg(bank, VS680_GPIO_EXT_PORT) & (1u << bit)) ? 1 : 0;
}

int vs680_setup(void)
{
    int mem_fd;
    int i;

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0)
        return SETUP_DEVMEM_FAIL;

    for (i = 0; i < VS680_GPIO_BANKS; ++i) {
        vs680_gpio_map[i] = (uint32_t *)mmap(NULL, VS680_GPIO_MAP_SIZE,
                                             PROT_READ|PROT_WRITE,
                                             MAP_SHARED, mem_fd,
                                             vs680_gpio_base[i]);
        if (vs680_gpio_map[i] == MAP_FAILED) {
            int j;

            vs680_gpio_map[i] = NULL;
            for (j = 0; j < i; ++j) {
                if (vs680_gpio_map[j] != NULL) {
                    munmap((void *)vs680_gpio_map[j], VS680_GPIO_MAP_SIZE);
                    vs680_gpio_map[j] = NULL;
                }
            }
            close(mem_fd);
            return SETUP_MMAP_FAIL;
        }
    }

    close(mem_fd);
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

    gpio_map = (uint32_t *)mmap((caddr_t)gpio_mem, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, mem_fd,
                                bpi_found_sun50iw9 ? SUN50IW9_GPIO_BASE : SUNXI_GPIO_BASE);
    pio_map = bpi_found_sun50iw9 ? gpio_map : gpio_map + (SUNXI_GPIO_REG_OFFSET>>2);
//printf("gpio_mem[%x] gpio_map[%x] pio_map[%x]\n", gpio_mem, gpio_map, pio_map);
//R_PIO GPIO LMN
    if (!bpi_found_sun50iw9) {
        r_gpio_map = (uint32_t *)mmap( (caddr_t)0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, SUNXI_R_GPIO_BASE);
        r_pio_map = r_gpio_map + (SUNXI_R_GPIO_REG_OFFSET>>2);
    } else {
        r_gpio_map = NULL;
        r_pio_map = NULL;
    }
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
    if(!bpi_found_sun50iw9 && bank >= 11) {
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
    if(!bpi_found_sun50iw9 && bank >= 11) {
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
    if(!bpi_found_sun50iw9 && bank >= 11) {
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
    if(!bpi_found_sun50iw9 && bank >= 11) {
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
    if(!bpi_found_sun50iw9 && bank >= 11) {
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

    if (bpi_found_meson == 1) {
        if (meson_gpio_map != NULL) {
            munmap((void *)meson_gpio_map, BLOCK_SIZE);
            meson_gpio_map = NULL;
        }
        if (meson_gpioao_map != NULL) {
            munmap((void *)meson_gpioao_map, BLOCK_SIZE);
            meson_gpioao_map = NULL;
        }
        return;
    }

    if (bpi_found_spacemit == 1) {
        if (spacemit_gpio_map != NULL) {
            munmap((void *)spacemit_gpio_map, BLOCK_SIZE);
            spacemit_gpio_map = NULL;
        }
        if (spacemit_pinctrl_map != NULL) {
            munmap((void *)spacemit_pinctrl_map, BLOCK_SIZE);
            spacemit_pinctrl_map = NULL;
        }
        return;
    }

    if (bpi_found_renesas == 1) {
        if (renesas_gpio_map != NULL) {
            munmap((void *)renesas_gpio_map, RENESAS_GPIO_MAP_SIZE);
            renesas_gpio_map = NULL;
        }
        return;
    }

    if (bpi_found_rockchip == 1) {
        int i;

        for (i = 0; i < ROCKCHIP_GPIO_BANKS; ++i) {
            if (rockchip_gpio_map[i] != NULL) {
                munmap((void *)rockchip_gpio_map[i], rockchip_gpio_map_size);
                rockchip_gpio_map[i] = NULL;
            }
        }
        return;
    }

    if (bpi_found_realtek == 1) {
        int i;

        for (i = 0; i < realtek_gpio_group_count; ++i) {
            if (realtek_gpio_map[i] != NULL) {
                munmap((void *)realtek_gpio_map[i], realtek_gpio_map_size);
                realtek_gpio_map[i] = NULL;
            }
        }
        return;
    }

    if (bpi_found_vs680 == 1) {
        int i;

        for (i = 0; i < VS680_GPIO_BANKS; ++i) {
            if (vs680_gpio_map[i] != NULL) {
                munmap((void *)vs680_gpio_map[i], VS680_GPIO_MAP_SIZE);
                vs680_gpio_map[i] = NULL;
            }
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
  bpi_found_mtk = 0;
  bpi_found_sun50iw9 = 0;
  bpi_found_meson = 0;
  bpi_found_spacemit = 0;
  bpi_found_renesas = 0;
  bpi_found_rockchip = 0;
  bpi_found_realtek = 0;
  bpi_found_vs680 = 0;
  if ((bpiFd = fopen("/var/lib/bananapi/board.sh", "r")) != NULL) {
    while(fgets(buffer, sizeof(buffer), bpiFd) != NULL) {
      if (sscanf(buffer, "BOARD=%1023s", hardware) != 1)
        continue;

      board = bpi_find_board_by_name(hardware);
      if (bpi_set_layout_from_board(board, &gpioLayout))
        break;
    }
    fclose(bpiFd);
  }

  if (bpi_found != 1 && (bpiFd = fopen("/proc/device-tree/model", "r")) != NULL) {
    if (fgets(hardware, sizeof(hardware), bpiFd) != NULL) {
      board = bpi_find_board_by_model_string(hardware);
      bpi_set_layout_from_board(board, &gpioLayout);
    }
    fclose(bpiFd);
  }

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
    bpi_found_sun50iw9 = (board->model == BPI_MODEL_M4BERRY || board->model == BPI_MODEL_M4ZERO);
    bpi_found_meson = (board->model == BPI_MODEL_M2S ||
                       board->model == BPI_MODEL_CM4IO ||
                       board->model == BPI_MODEL_M5 ||
                       board->model == BPI_MODEL_M2PRO);
    bpi_found_spacemit = (board->model == BPI_MODEL_F3);
    bpi_found_renesas = (board->model == BPI_MODEL_AI2N);
    bpi_found_rockchip = bpi_model_is_rockchip(board->model);
    if (bpi_found_rockchip == 1)
        bpi_select_rockchip_backend(board->model);
    bpi_found_realtek = bpi_model_is_realtek(board->model);
    if (bpi_found_realtek == 1)
        bpi_select_realtek_backend(board->model);
    bpi_found_vs680 = bpi_model_is_vs680(board->model);
    sprintf(manufacturer, "%s", piMakerNames [board->maker]);
    info->p1_revision = 3;
    info->type = type;
    info->ram  = ram;
    info->manufacturer = manufacturer;
    if(bpi_found_mtk == 1){
        info->processor = "MTK";
    }else if (bpi_found_meson == 1) {
	info->processor = "Amlogic Meson";
    }else if (bpi_found_spacemit == 1) {
	info->processor = "SpacemiT K1";
    }else if (bpi_found_renesas == 1) {
	info->processor = "Renesas RZ/V2N";
    }else if (bpi_found_rockchip == 1) {
	if (bpi_model_is_rk3576(board->model))
	    info->processor = "Rockchip RK3576";
	else if (bpi_model_is_rk3308(board->model))
	    info->processor = "Rockchip RK3308";
	else if (bpi_model_is_rk3506(board->model))
	    info->processor = "Rockchip RK3506J";
	else if (bpi_model_is_rk3528(board->model))
	    info->processor = "Rockchip RK3528";
	else if (bpi_model_is_rk3588(board->model))
	    info->processor = "Rockchip RK3588";
	else
	    info->processor = "Rockchip RK3568";
    }else if (bpi_found_realtek == 1) {
	if (board->model == BPI_MODEL_M4)
	    info->processor = "Realtek RTD1395";
	else
	    info->processor = "Realtek RTD1296";
    }else if (bpi_found_vs680 == 1) {
	info->processor = "Synaptics VS680";
    }else if (bpi_found_sun50iw9 == 1) {
	info->processor = "AW SUN50IW9";
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
