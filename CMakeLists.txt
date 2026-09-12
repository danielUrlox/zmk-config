zephyr_library()
zephyr_library_sources_ifdef(CONFIG_POCKETBOARD_MCP_DIAG src/pocketboard_mcp_diag.c)

# ZMK public headers live in zmk/app/include. Extra Zephyr modules do not
# automatically inherit this application include path.
zephyr_library_include_directories(${APPLICATION_SOURCE_DIR}/include)
