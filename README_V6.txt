PocketBoard-Corne-HW1 - Firmware diagnostico v6

OBJETIVO
- No cambia el cableado ni la matriz.
- D1/P0.06 queda como SDA.
- D0/P0.08 queda como SCL.
- UART0 permanece deshabilitado.
- I2C permanece a 100 kHz.
- Se agregan pull-ups INTERNOS del nRF52840 a SDA y SCL mediante bias-pull-up.
- MCP23017 permanece en 0x20.
- Combo bootloader: teclas 36 + 38.

PRUEBA DESPUES DE FLASHEAR
1. Sin TRRS, conectar solo mitad izquierda al USB.
2. Multimetro en V DC.
3. Medir D0/P0.08 contra GND.
4. Luego medir D1/P0.06 contra GND.
5. Ambos deberian subir claramente respecto a las mediciones anteriores.

Si compila y las lineas suben, conectar TRRS con USB DESCONECTADO y probar el lado derecho.
