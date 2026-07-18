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

int setup(void);
void setup_gpio(int gpio, int direction, int pud);
int gpio_function(int gpio);
void output_gpio(int gpio, int value);
int input_gpio(int gpio);
void set_rising_event(int gpio, int enable);
void set_falling_event(int gpio, int enable);
void set_high_event(int gpio, int enable);
void set_low_event(int gpio, int enable);
int eventdetected(int gpio);
void cleanup(void);

#ifndef BPI
#define BPI
#endif

#ifdef BPI
int sunxi_setup(void);
void sunxi_setup_gpio(int gpio, int direction, int pud);
int sunxi_gpio_function(int gpio);
void sunxi_output_gpio(int gpio, int value);
int sunxi_input_gpio(int gpio);
void sunxi_set_pullupdn(int gpio, int pud);
int mtk_setup(void);
int mtk_set_gpio_out(unsigned int pin, unsigned int output);
int mtk_set_gpio_dir(unsigned int pin, unsigned int dir);
int mtk_set_gpio_mode(unsigned int pin, unsigned int mode);
int mtk_set_pullupdn(unsigned int pin, int pud);
int mtk_gpio_function(unsigned int pin);
int mtk_input_gpio(unsigned int pin);
int mtk_v2_setup(void);
void mtk_v2_setup_gpio(int gpio, int direction, int pud);
int mtk_v2_gpio_function(int gpio);
void mtk_v2_output_gpio(int gpio, int value);
int mtk_v2_input_gpio(int gpio);
void mtk_v2_set_pullupdn(int gpio, int pud);
int mtk_mt7622_setup(void);
void mtk_mt7622_setup_gpio(int gpio, int direction, int pud);
int mtk_mt7622_gpio_function(int gpio);
void mtk_mt7622_output_gpio(int gpio, int value);
int mtk_mt7622_input_gpio(int gpio);
void mtk_mt7622_set_pullupdn(int gpio, int pud);
int meson_setup(void);
void meson_setup_gpio(int gpio, int direction, int pud);
int meson_gpio_function(int gpio);
void meson_output_gpio(int gpio, int value);
int meson_input_gpio(int gpio);
void meson_set_pullupdn(int gpio, int pud);
int spacemit_setup(void);
void spacemit_setup_gpio(int gpio, int direction, int pud);
int spacemit_gpio_function(int gpio);
void spacemit_output_gpio(int gpio, int value);
int spacemit_input_gpio(int gpio);
void spacemit_set_pullupdn(int gpio, int pud);
int renesas_setup(void);
void renesas_setup_gpio(int gpio, int direction, int pud);
int renesas_gpio_function(int gpio);
void renesas_output_gpio(int gpio, int value);
int renesas_input_gpio(int gpio);
void renesas_set_pullupdn(int gpio, int pud);
int rockchip_setup(void);
void rockchip_setup_gpio(int gpio, int direction, int pud);
int rockchip_gpio_function(int gpio);
void rockchip_output_gpio(int gpio, int value);
int rockchip_input_gpio(int gpio);
void rockchip_set_pullupdn(int gpio, int pud);
int realtek_setup(void);
void realtek_setup_gpio(int gpio, int direction, int pud);
int realtek_gpio_function(int gpio);
void realtek_output_gpio(int gpio, int value);
int realtek_input_gpio(int gpio);
void realtek_set_pullupdn(int gpio, int pud);
int vs680_setup(void);
void vs680_setup_gpio(int gpio, int direction, int pud);
int vs680_gpio_function(int gpio);
void vs680_output_gpio(int gpio, int value);
int vs680_input_gpio(int gpio);
void vs680_set_pullupdn(int gpio, int pud);
int sp7021_setup(void);
void sp7021_setup_gpio(int gpio, int direction, int pud);
int sp7021_gpio_function(int gpio);
void sp7021_output_gpio(int gpio, int value);
int sp7021_input_gpio(int gpio);
void sp7021_set_pullupdn(int gpio, int pud);
int sp7350_setup(void);
void sp7350_setup_gpio(int gpio, int direction, int pud);
int sp7350_gpio_function(int gpio);
void sp7350_output_gpio(int gpio, int value);
int sp7350_input_gpio(int gpio);
void sp7350_set_pullupdn(int gpio, int pud);
int k230_setup(void);
void k230_setup_gpio(int gpio, int direction, int pud);
int k230_gpio_function(int gpio);
void k230_output_gpio(int gpio, int value);
int k230_input_gpio(int gpio);
void k230_set_pullupdn(int gpio, int pud);
void bpi_cleanup(void);
#endif

#define SETUP_OK           0
#define SETUP_DEVMEM_FAIL  1
#define SETUP_MALLOC_FAIL  2
#define SETUP_MMAP_FAIL    3
#define SETUP_CPUINFO_FAIL 4
#define SETUP_NO_PERI_ADDR 5

#define INPUT  1 // is really 0 for control register!
#define OUTPUT 0 // is really 1 for control register!
#define ALT0   4

#define HIGH 1
#define LOW  0

#define PUD_OFF  0
#define PUD_DOWN 1
#define PUD_UP   2
