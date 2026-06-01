#define	BPI_M2M_V11_01	-1
#define	BPI_M2M_V11_03	GPIO_PH05
#define	BPI_M2M_V11_05	GPIO_PH04
#define	BPI_M2M_V11_07	GPIO_PH01
#define	BPI_M2M_V11_09	-1
#define	BPI_M2M_V11_11	GPIO_PH06
#define	BPI_M2M_V11_13	GPIO_PH07
#define	BPI_M2M_V11_15	GPIO_PG12
#define	BPI_M2M_V11_17	-1
#define	BPI_M2M_V11_19	GPIO_PC00
#define	BPI_M2M_V11_21	GPIO_PC01
#define	BPI_M2M_V11_23	GPIO_PC02
#define	BPI_M2M_V11_25	-1
#define	BPI_M2M_V11_27	GPIO_PD19
#define	BPI_M2M_V11_29	GPIO_PD18
#define	BPI_M2M_V11_31	GPIO_PD21
#define	BPI_M2M_V11_33	GPIO_PD20
#define	BPI_M2M_V11_35	GPIO_PD25
#define	BPI_M2M_V11_37	GPIO_PD24
#define	BPI_M2M_V11_39	-1

#define	BPI_M2M_V11_02	-1
#define	BPI_M2M_V11_04	-1
#define	BPI_M2M_V11_06	-1
#define	BPI_M2M_V11_08	GPIO_PB00
#define	BPI_M2M_V11_10	GPIO_PB01
//#define	BPI_M2M_V11_08	-1
//#define	BPI_M2M_V11_10	-1
#define	BPI_M2M_V11_12	GPIO_PB03
#define	BPI_M2M_V11_14	-1
#define	BPI_M2M_V11_16	GPIO_PB02
#define	BPI_M2M_V11_18	GPIO_PG11
#define	BPI_M2M_V11_20	-1
#define	BPI_M2M_V11_22	GPIO_PG10
#define	BPI_M2M_V11_24	GPIO_PC03
#define	BPI_M2M_V11_26	GPIO_PG13
#define	BPI_M2M_V11_28	GPIO_PL11
#define	BPI_M2M_V11_30	-1
#define	BPI_M2M_V11_32	GPIO_PD23
#define	BPI_M2M_V11_34	-1
#define	BPI_M2M_V11_36	GPIO_PD22
#define	BPI_M2M_V11_38	GPIO_PD27
#define	BPI_M2M_V11_40	GPIO_PD26

//map wpi gpio_num(index) to bp bpio_num(element)
const int pinToGpio_BPI_M2M_V11 [64] =
{
   BPI_M2M_V11_11, BPI_M2M_V11_12,        //0, 1
   BPI_M2M_V11_13, BPI_M2M_V11_15,        //2, 3
   BPI_M2M_V11_16, BPI_M2M_V11_18,        //4, 5
   BPI_M2M_V11_22, BPI_M2M_V11_07,        //6, 7
   BPI_M2M_V11_03, BPI_M2M_V11_05,        //8, 9
   BPI_M2M_V11_24, BPI_M2M_V11_26,        //10, 11
   BPI_M2M_V11_19, BPI_M2M_V11_21,        //12, 13
   BPI_M2M_V11_23, BPI_M2M_V11_08,        //14, 15
   BPI_M2M_V11_10,        -1,        //16, 17
          -1,        -1,        //18, 19
          -1, BPI_M2M_V11_29,        //20, 21
   BPI_M2M_V11_31, BPI_M2M_V11_33,        //22, 23
   BPI_M2M_V11_35, BPI_M2M_V11_37,        //24, 25
   BPI_M2M_V11_32, BPI_M2M_V11_36,        //26, 27
   BPI_M2M_V11_38, BPI_M2M_V11_40,        //28. 29
   BPI_M2M_V11_27, BPI_M2M_V11_28,        //30, 31
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // ... 47
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // ... 63
} ;

//map bcm gpio_num(index) to bp gpio_num(element)
const int pinTobcm_BPI_M2M_V11 [64] =
{
  BPI_M2M_V11_27, BPI_M2M_V11_28,  //0, 1
  BPI_M2M_V11_03, BPI_M2M_V11_05,  //2, 3
  BPI_M2M_V11_07, BPI_M2M_V11_29,  //4, 5
  BPI_M2M_V11_31, BPI_M2M_V11_26,  //6, 7
  BPI_M2M_V11_24, BPI_M2M_V11_21,  //8, 9
  BPI_M2M_V11_19, BPI_M2M_V11_23,  //10, 11
  BPI_M2M_V11_32, BPI_M2M_V11_33,  //12, 13
  BPI_M2M_V11_08, BPI_M2M_V11_10,  //14, 15
  BPI_M2M_V11_36, BPI_M2M_V11_11,  //16, 17
  BPI_M2M_V11_12, BPI_M2M_V11_35,	 //18, 19
  BPI_M2M_V11_38, BPI_M2M_V11_40,  //20, 21
  BPI_M2M_V11_15, BPI_M2M_V11_16,  //22, 23
  BPI_M2M_V11_18, BPI_M2M_V11_22,  //24, 25
  BPI_M2M_V11_37, BPI_M2M_V11_13,  //26, 27
  -1, -1,
  -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // ... 47
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // ... 63
} ;

//map phys_num(index) to bp gpio_num(element)
const int physToGpio_BPI_M2M_V11 [64] =
{
          -1,                //0
          -1,        -1,     //1, 2
   BPI_M2M_V11_03,        -1,     //3, 4
   BPI_M2M_V11_05,        -1,     //5, 6
   BPI_M2M_V11_07, BPI_M2M_V11_08,     //7, 8
          -1, BPI_M2M_V11_10,     //9, 10
   BPI_M2M_V11_11, BPI_M2M_V11_12,     //11, 12
   BPI_M2M_V11_13,        -1,     //13, 14
   BPI_M2M_V11_15, BPI_M2M_V11_16,     //15, 16
          -1, BPI_M2M_V11_18,     //17, 18
   BPI_M2M_V11_19,        -1,     //19, 20
   BPI_M2M_V11_21, BPI_M2M_V11_22,     //21, 22
   BPI_M2M_V11_23, BPI_M2M_V11_24,     //23, 24
          -1, BPI_M2M_V11_26,     //25, 26
   BPI_M2M_V11_27, BPI_M2M_V11_28,     //27, 28
   BPI_M2M_V11_29,        -1,     //29, 30
   BPI_M2M_V11_31, BPI_M2M_V11_32,     //31, 32      
   BPI_M2M_V11_33,        -1,     //33, 34
   BPI_M2M_V11_35, BPI_M2M_V11_36,     //35, 36
   BPI_M2M_V11_37, BPI_M2M_V11_38,     //37, 38
          -1, BPI_M2M_V11_40,     //39, 40
   -1,   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //41-> 55
   -1,   -1, -1, -1, -1, -1, -1, -1 // 56-> 63
} ;
