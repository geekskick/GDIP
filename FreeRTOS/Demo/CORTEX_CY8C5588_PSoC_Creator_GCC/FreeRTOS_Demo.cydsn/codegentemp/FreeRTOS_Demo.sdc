# THIS FILE IS AUTOMATICALLY GENERATED
# Project: C:\Users\NGGMLGM\Documents\UWE\GDIP\GDIP\FreeRTOS\Demo\CORTEX_CY8C5588_PSoC_Creator_GCC\FreeRTOS_Demo.cydsn\FreeRTOS_Demo.cyprj
# Date: Sun, 06 Nov 2016 08:37:51 GMT
#set_units -time ns
create_clock -name {CyILO} -period 1000000 -waveform {0 500000} [list [get_pins {ClockBlock/ilo}] [get_pins {ClockBlock/clk_100k}] [get_pins {ClockBlock/clk_1k}] [get_pins {ClockBlock/clk_32k}]]
create_clock -name {CyIMO} -period 333.33333333333331 -waveform {0 166.666666666667} [list [get_pins {ClockBlock/imo}]]
create_clock -name {CyPLL_OUT} -period 20.833333333333332 -waveform {0 10.4166666666667} [list [get_pins {ClockBlock/pllout}]]
create_clock -name {CyMASTER_CLK} -period 20.833333333333332 -waveform {0 10.4166666666667} [list [get_pins {ClockBlock/clk_sync}]]
create_generated_clock -name {CyBUS_CLK} -source [get_pins {ClockBlock/clk_sync}] -edges {1 2 3} [list [get_pins {ClockBlock/clk_bus_glb}]]
create_clock -name {CyBUS_CLK(fixed-function)} -period 20.833333333333332 -waveform {0 10.4166666666667} [get_pins {ClockBlock/clk_bus_glb_ff}]
create_generated_clock -name {Clock_1} -source [get_pins {ClockBlock/clk_sync}] -edges {1 49 97} [list [get_pins {ClockBlock/dclk_glb_0}]]
create_generated_clock -name {UART_1_IntClock} -source [get_pins {ClockBlock/clk_sync}] -edges {1 105 209} -nominal_period 2166.6666666666665 [list [get_pins {ClockBlock/dclk_glb_1}]]

set_false_path -from [get_pins {__ONE__/q}]

# Component constraints for C:\Users\NGGMLGM\Documents\UWE\GDIP\GDIP\FreeRTOS\Demo\CORTEX_CY8C5588_PSoC_Creator_GCC\FreeRTOS_Demo.cydsn\TopDesign\TopDesign.cysch
# Project: C:\Users\NGGMLGM\Documents\UWE\GDIP\GDIP\FreeRTOS\Demo\CORTEX_CY8C5588_PSoC_Creator_GCC\FreeRTOS_Demo.cydsn\FreeRTOS_Demo.cyprj
# Date: Sun, 06 Nov 2016 08:37:43 GMT
