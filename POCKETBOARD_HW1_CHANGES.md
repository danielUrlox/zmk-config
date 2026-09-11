# PocketBoard-Corne-HW1 v10

- Base: v6 funcional del usuario.
- MCP23017 configurado en 0x20.
- Supuesto físico requerido: A0/A1/A2 de DIRECCIÓN unidos a GND.
- Conservado: SDA P0.06, SCL P0.08, UART0 deshabilitado, I2C 100 kHz.
- Conservado: izquierda C1..C6 = D4,D5,D6,D10,D8,D9; R1..R4 = D14,D15,D16,D18.
- Conservado: derecha GPA0..5 columnas y GPB0..3 filas.
- Eliminado del build principal: logging USB de diagnóstico.
