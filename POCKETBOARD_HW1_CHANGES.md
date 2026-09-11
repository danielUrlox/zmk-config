# PocketBoard HW1 - Firmware v3

Cambios frente a v2:

- Mantiene un solo nice!nano/nRF52840 en el lado izquierdo.
- Mantiene MCP23017 en I2C 0x20 para el lado derecho.
- SDA = D1 / P0.06 y SCL = D0 / P0.08.
- Matriz derecha: GPA0..GPA5 = columnas y GPB0..GPB3 = filas.
- Reduce I2C de 400 kHz a 100 kHz para mayor margen con TRRS y resistencias serie.
- Agrega 1 tick de espera antes de leer inputs y entre columnas para el MCP23017.
- Usa polling estable de 10 ms y debounce 5 ms.
- Conserva el combo de bootloader en las teclas 36 + 38.

Objetivo de esta versión: priorizar estabilidad del lado derecho antes que velocidad máxima.


## v4
- UART0 explicitly disabled because nice!nano v2 defaults UART RX/TX to P0.08/P0.06, the exact pins PocketBoard uses for SCL/SDA.
- I2C remains P0.06 SDA / P0.08 SCL at 100 kHz.
- MCP23017 remains 0x20, GPA0-GPA5 columns, GPB0-GPB3 rows.
- Left matrix and bootloader combo 36+38 unchanged.


## v6 - prueba I2C con pull-ups internos
- Se mantienen SDA=P0.06/D1 y SCL=P0.08/D0.
- UART0 sigue deshabilitado.
- I2C a 100 kHz.
- Se agrega `bias-pull-up;` al grupo pinctrl TWIM para usar los pull-ups internos del nRF52840 en SDA/SCL durante la prueba.

## v9 - prueba directa MCP23017 @ 0x20
- Agrega un módulo C de diagnóstico.
- Lee el registro IODIRA (0x00) del MCP23017 en 0x20 mediante i2c_write_read().
- Informa FOUND / NO RESPONSE por USB logging cada 3 segundos.
- Mantiene el hardware/pinout de v6: SDA=P0.06/D1, SCL=P0.08/D0.
