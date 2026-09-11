# PocketBoard-Corne-HW1 - cambios manuales

Esta versión reemplaza el cableado ficticio que Shield Wizard asignó a las 21 teclas derechas.

## Hardware usado

- Único controlador: nice!nano v2 / nRF52840, lado izquierdo.
- Lado izquierdo: matriz 4x6 directa al nRF52840.
- Lado derecho: matriz 4x6 mediante MCP23017 por I2C, dirección 0x20.
- SDA: D1 / P0.06.
- SCL: D0 / P0.08.
- MCP columnas C1-C6: GPA0-GPA5.
- MCP filas R1-R4: GPB0-GPB3.
- B0-B3 tienen pull-down físicos externos de 10 kOhm.
- Diodos: col2row.
- Pulgares físicos: C4, C5 y C6 en R4.

## Kscan

Se usan dos matrices y un `zmk,kscan-composite`:

- izquierda -> offset fila 0 / columna 0
- derecha -> offset fila 4 / columna 6

Esto conserva el transform 8x12 y el orden de 42 teclas que produjo Shield Wizard.

## Bootloader por dos teclas

Presionar simultáneamente las posiciones 36 y 38 (las dos teclas exteriores del grupo de 3 pulgares de la mitad izquierda) ejecuta `&bootloader`.

Esto permite entrar al modo UF2 sin tocar RESET una vez que el firmware esté funcionando.


## Build fix for Zephyr 3.5 / ZMK v0.3
The Zephyr 3.5 tree used by this ZMK build exposes the MCP23017 family through the generic devicetree compatible `microchip,mcp230xx` (not the newer chip-specific `microchip,mcp23017`). The overlay was updated accordingly.
