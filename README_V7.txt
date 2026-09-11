PocketBoard-Corne-HW1 v7

Cambio principal:
- Se deja de usar TWIM hardware para el enlace al MCP23017.
- Se usa I2C por GPIO (bit-bang) exactamente en los pines físicos ya cableados:
  SDA = D1 / P0.06
  SCL = D0 / P0.08
- Se solicitan pull-up internos del nRF52840 en SDA y SCL.
- MCP23017 permanece en 0x20.
- No se cambia la matriz izquierda ni el keymap.
- Bootloader 36 + 38 se conserva en el keymap existente.

Objetivo: aislar cualquier conflicto de pinctrl/TWIM/UART del firmware anterior.
