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
