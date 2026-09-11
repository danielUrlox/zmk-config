PocketBoard HW1 - diagnostico I2C v9

Objetivo:
- Comprobar directamente si el nRF52840 puede hablar con el MCP23017 en 0x20.
- No requiere cambiar el cableado.

Resultado por USB serial (se repite cada 3 s):
  POCKETBOARD: MCP23017 FOUND @ 0x20 (...)
  o
  POCKETBOARD: NO RESPONSE @ 0x20 (...)

Uso:
1. Compilar en GitHub Actions.
2. Flashear pocketboard_hw1_i2c_diagnostic*.uf2 (NO settings_reset).
3. Conectar TRRS antes de conectar USB.
4. Abrir monitor serie del puerto USB del nice!nano a 115200.
5. Esperar ~7 segundos.
6. Copiar aquí las líneas que comiencen con POCKETBOARD:.
