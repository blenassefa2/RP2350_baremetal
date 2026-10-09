// core1.c
// SPDX-License-Identifier: GPL-3.0-or-later

#include "debug.h"

void core1_main(void)
{
  debug_blink(1);       // core 1: blink four times

  while (1);            // park
}