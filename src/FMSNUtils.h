/*
   The MIT License (MIT)

   Copyright (C) 2017 Hong-She Liang <starofrainnight@gmail.com>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
 */

#ifndef __INCLUDED_895A8BE057E211E7AA6EA088B4D1658C
#define __INCLUDED_895A8BE057E211E7AA6EA088B4D1658C

#include "FMSNTypes.h"

#define fmsnSafeCopyText(dest, src, size) \
  do \
  { \
    strncpy(dest, src, (size)); \
    (dest)[(size) - 1] = 0; \
  } while(0);

///
/// Get respond type from a reqeuest type.
///
/// @arg requestType
FMSNMsgType
fmsnGetRespondType(FMSNMsgType requestType);

///
/// @brief Check if qos level is high qos
/// @param qos
/// @return
///
bool
fmsnIsHighQos(uint8_t qos);

#endif // __INCLUDED_895A8BE057E211E7AA6EA088B4D1658C
