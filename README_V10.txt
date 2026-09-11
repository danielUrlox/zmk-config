PocketBoard-Corne-HW1 v10 — firmware funcional

BASE:
- Basado directamente en la v6 que mantenía funcionando el lado izquierdo.
- Sin el diagnóstico LED ni logging USB experimental.

REQUISITO DE HARDWARE ANTES DE PROBAR:
- MCP23017 A0 (dirección) -> GND
- MCP23017 A1 (dirección) -> GND
- MCP23017 A2 (dirección) -> GND
Esto fija la dirección I2C en 0x20.

MAPEO DERECHO:
- GPA0..GPA5 -> C1..C6
- GPB0..GPB3 -> R1..R4
- SDA -> nRF P0.06 / D1
- SCL -> nRF P0.08 / D0
- VCC -> 3.3 V
- GND -> GND

IMPORTANTE:
- No conectar/desconectar TRRS con USB energizado.
- Flashear el artefacto pocketboard_hw1_v10_mcp_0x20, NO settings_reset para la prueba normal.
