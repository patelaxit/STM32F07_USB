# List of all available vendor requests
requests = (
    'CTRL_LOOPBACK_WRITE',
    'CTRL_LOOPBACK_READ',
    'LED_TOGGLE',
    'LED_ON',
    'LED_OFF',
)

# Creates string constants with their respective index as value
def _create_module_level_constants():
    this_module = globals()
    for i, name in enumerate(this_module['requests']):
        this_module[name] = i

_create_module_level_constants()