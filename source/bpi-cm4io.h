#ifndef BPI_CM4IO_H
#define BPI_CM4IO_H

#ifndef BPI_MESON_GPIO_PIN_BASE
#define BPI_MESON_GPIO_PIN_BASE 410
#define BPI_MESON_GPIOH(n) (BPI_MESON_GPIO_PIN_BASE + 17 + (n))
#define BPI_MESON_GPIOA(n) (BPI_MESON_GPIO_PIN_BASE + 50 + (n))
#define BPI_MESON_GPIOX(n) (BPI_MESON_GPIO_PIN_BASE + 66 + (n))
#define BPI_MESON_GPIOAO(n) (BPI_MESON_GPIO_PIN_BASE + 86 + (n))
#endif

const int pinToGpio_BPI_CM4IO[64] =
{
   BPI_MESON_GPIOAO(10), BPI_MESON_GPIOA(1),
   BPI_MESON_GPIOH(4),   BPI_MESON_GPIOAO(5),
   BPI_MESON_GPIOA(0),   BPI_MESON_GPIOA(2),
   BPI_MESON_GPIOA(7),   BPI_MESON_GPIOH(5),
   BPI_MESON_GPIOX(17),  BPI_MESON_GPIOX(18),
   BPI_MESON_GPIOX(10),  BPI_MESON_GPIOA(3),
   BPI_MESON_GPIOX(8),   BPI_MESON_GPIOX(9),
   BPI_MESON_GPIOX(11),  BPI_MESON_GPIOX(6),
   BPI_MESON_GPIOX(7),   -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
};

const int pinTobcm_BPI_CM4IO[64] =
{
   -1, -1, BPI_MESON_GPIOX(17), BPI_MESON_GPIOX(18),
   BPI_MESON_GPIOAO(10), -1, -1, BPI_MESON_GPIOA(3),
   BPI_MESON_GPIOX(10),  BPI_MESON_GPIOX(9),
   BPI_MESON_GPIOX(8),   BPI_MESON_GPIOX(11),
   -1, -1, BPI_MESON_GPIOX(6), BPI_MESON_GPIOX(7),
   -1, BPI_MESON_GPIOH(5), BPI_MESON_GPIOA(1), -1,
   -1, -1, BPI_MESON_GPIOAO(5), BPI_MESON_GPIOA(0),
   BPI_MESON_GPIOA(2), BPI_MESON_GPIOA(7), -1, BPI_MESON_GPIOH(4),
   -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
};

const int physToGpio_BPI_CM4IO[64] =
{
   -1,
   -1, -1,
   BPI_MESON_GPIOX(17), -1,
   BPI_MESON_GPIOX(18), -1,
   BPI_MESON_GPIOH(5),  BPI_MESON_GPIOX(6),
   -1, BPI_MESON_GPIOX(7),
   BPI_MESON_GPIOAO(10), BPI_MESON_GPIOA(1),
   BPI_MESON_GPIOH(4), -1,
   BPI_MESON_GPIOAO(5), BPI_MESON_GPIOA(0),
   -1, BPI_MESON_GPIOA(2),
   BPI_MESON_GPIOX(8), -1,
   BPI_MESON_GPIOX(9), BPI_MESON_GPIOA(7),
   BPI_MESON_GPIOX(11), BPI_MESON_GPIOX(10),
   -1, BPI_MESON_GPIOA(3),
   -1, -1,
   -1, -1,
   -1, -1,
   -1, -1,
   -1, -1,
   -1, -1,
   -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1, -1,
   -1, -1, -1, -1, -1, -1, -1,
};

#endif
