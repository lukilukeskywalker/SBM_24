### Project configuration

1. Press "options for target"
2. In "Target"-> Code Generation, Select "Use default compiler version 5"
3. In "C/C++" in "Define" include "HSE_VALUE=8000000", select "C99 Mode" and "GNU extensions"
4. In "Debug" Select "Use: ST-Link Debugger". Press "Settings"
- In "Trace" Set "Core Clock" to 168.000000 MHz
- Enable "Trace Enable"
- Enable "Use Core Clock"
- Disable "Set max" and Set "SWO Clock Prescaler" to 2624, SWO Clock should be automaticaly configured to "0.064024"
- All ITM Stimulus Ports should be enabled
