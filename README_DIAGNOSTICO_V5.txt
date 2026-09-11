POCKETBOARD HW1 - DIAGNOSTICO V5

Objetivo: ver por USB si el MCP23017/I2C se inicializa o falla.

Hardware esperado:
- SDA: D1 / P0.06
- SCL: D0 / P0.08
- MCP23017: 0x20
- GPA0..GPA5: columnas
- GPB0..GPB3: filas

Esta version habilita ZMK USB logging (CDC ACM).
Despues de flashear, conectar por USB y abrir el nuevo puerto COM con Arduino Serial Monitor o PuTTY.
Buscar mensajes que contengan: i2c, mcp230, gpio, error, failed, ready.
