# gbmtest
Test program to see which formats can be used by libgbm.

This program is also used to demonstrate the difference between libgbm of mesa, and minigbm that is bundled with chromium.

If the libminigbm.so is preloaded, more formats are supported, including NV12.

For example:
```
$ LD_PRELOAD=/usr/src/minigbm/libminigbm.so.1.0.0 ./nv12test
```

Will list this on my machine:
```
Using backend i915
C8  : 
R8  : 
R16 : 
GR88: 
RG32: 
GR32: 
RGB8: 
BGR8: 
XR12: 
XB12: 
RX12: 
BX12: 
AR12: 
AB12: 
RA12: 
BA12: 
XR15: 
XB15: 
RX15: 
BX15: 
AR15: 
AB15: 
RA15: 
BA15: 
RG16: [SUPPORTED]
BG16: 
RG24: 
BG24: 
XR24: [SUPPORTED]
XB24: [SUPPORTED]
RX24: 
BX24: 
AR24: [SUPPORTED]
AB24: [SUPPORTED]
RA24: 
BA24: 
XR30: [SUPPORTED]
XB30: [SUPPORTED]
RX30: 
BX30: 
AR30: [SUPPORTED]
AB30: [SUPPORTED]
RA30: 
BA30: 
XB48: 
AB48: 
XB4H: 
AB4H: 
YUYV: 
YVYU: 
UYVY: 
VYUY: 
AYUV: 
NV12: [SUPPORTED]
NV21: 
NV16: 
NV61: 
YUV9: 
YVU9: 
YU11: 
YV11: 
YU12: 
YV12: 
YU16: 
YV16: 
YU24: 
YV24: 
Created GBM buffer object of format NV12 and dimension 1280x720
Bpp: 1
Stride for plane 0: 1280
Stride for plane 1: 1280
m = 0x7fd678ca7000
mapping = 0x55d21c5c4270
```

