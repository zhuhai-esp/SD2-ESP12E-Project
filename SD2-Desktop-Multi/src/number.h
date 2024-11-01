#ifndef NUMBER_H
#define NUMBER_H

#include <TFT_eSPI.h>

#include "font/O_3660_i0.h"
#include "font/O_3660_i1.h"
#include "font/O_3660_i2.h"
#include "font/O_3660_i3.h"
#include "font/O_3660_i4.h"
#include "font/O_3660_i5.h"
#include "font/O_3660_i6.h"
#include "font/O_3660_i7.h"
#include "font/O_3660_i8.h"
#include "font/O_3660_i9.h"

#include "font/W_3660_i0.h"
#include "font/W_3660_i1.h"
#include "font/W_3660_i2.h"
#include "font/W_3660_i3.h"
#include "font/W_3660_i4.h"
#include "font/W_3660_i5.h"
#include "font/W_3660_i6.h"
#include "font/W_3660_i7.h"
#include "font/W_3660_i8.h"
#include "font/W_3660_i9.h"

#include "font/W_1830_i0.h"
#include "font/W_1830_i1.h"
#include "font/W_1830_i2.h"
#include "font/W_1830_i3.h"
#include "font/W_1830_i4.h"
#include "font/W_1830_i5.h"
#include "font/W_1830_i6.h"
#include "font/W_1830_i7.h"
#include "font/W_1830_i8.h"
#include "font/W_1830_i9.h"

#include "font/O_40120_colon.h"

#include "font/O_80120_i0.h"
#include "font/O_80120_i1.h"
#include "font/O_80120_i2.h"
#include "font/O_80120_i3.h"
#include "font/O_80120_i4.h"
#include "font/O_80120_i5.h"
#include "font/O_80120_i6.h"
#include "font/O_80120_i7.h"
#include "font/O_80120_i8.h"
#include "font/O_80120_i9.h"

#include "font/B_80120_i0.h"
#include "font/B_80120_i1.h"
#include "font/B_80120_i2.h"
#include "font/B_80120_i3.h"
#include "font/B_80120_i4.h"
#include "font/B_80120_i5.h"
#include "font/B_80120_i6.h"
#include "font/B_80120_i7.h"
#include "font/B_80120_i8.h"
#include "font/B_80120_i9.h"

class Number {
private:
public:
  void printfB80120(int numx, int numy, int numn);
  void printfO80120(int numx, int numy, int numn);
  void printfW3660(int numx, int numy, int numn);
  void printfO3660(int numx, int numy, int numn);
  void printfW1830(int numx, int numy, int numn);
};

#endif
