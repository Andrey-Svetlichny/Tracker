EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L myLib:BlackPill U?
U 1 1 5FBB3C61
P 5000 3550
F 0 "U?" H 5000 4765 50  0000 C CNN
F 1 "STM32F401 BlackPill" H 5000 4674 50  0000 C CNN
F 2 "" H 5100 4650 50  0001 C CNN
F 3 "" H 5100 4650 50  0001 C CNN
	1    5000 3550
	1    0    0    -1  
$EndComp
$Comp
L myLib:SIM800L U?
U 1 1 5FBB5448
P 7100 3750
F 0 "U?" H 7378 3796 50  0000 L CNN
F 1 "SIM800L" H 7378 3705 50  0000 L CNN
F 2 "" H 7100 4150 50  0001 C CNN
F 3 "" H 7100 4150 50  0001 C CNN
	1    7100 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	6150 3700 6250 3900
Wire Wire Line
	6750 3600 6600 3600
$Comp
L Device:R R?
U 1 1 5FBC7BFE
P 5600 2850
F 0 "R?" H 5450 2800 50  0000 C CNN
F 1 "3k3" H 5450 2900 50  0000 C CNN
F 2 "" V 5530 2850 50  0001 C CNN
F 3 "~" H 5600 2850 50  0001 C CNN
	1    5600 2850
	-1   0    0    1   
$EndComp
$Comp
L Device:CP C?
U 1 1 5FBC947C
P 6300 4650
F 0 "C?" H 6550 4700 50  0000 C CNN
F 1 "470mF" H 6550 4600 50  0000 C CNN
F 2 "" H 6338 4500 50  0001 C CNN
F 3 "~" H 6300 4650 50  0001 C CNN
	1    6300 4650
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 4400 4550 4400
Wire Wire Line
	4250 3700 4250 4300
Wire Wire Line
	4250 4300 4550 4300
Wire Wire Line
	4150 3800 4150 4400
Wire Wire Line
	4050 3700 4250 3700
Wire Wire Line
	4550 4000 4050 4000
Wire Wire Line
	4050 3900 4550 3900
Wire Wire Line
	4050 3800 4150 3800
$Comp
L myLib:OLED_SSD1306 U?
U 1 1 5FBCCE2D
P 3650 3850
F 0 "U?" H 3650 3500 50  0000 C CNN
F 1 "OLED_SSD1306" H 3650 3400 50  0000 C CNN
F 2 "" H 3650 4250 50  0001 C CNN
F 3 "" H 3650 4250 50  0001 C CNN
	1    3650 3850
	1    0    0    -1  
$EndComp
Connection ~ 4250 3700
Wire Wire Line
	4150 3400 4150 3800
Connection ~ 4150 3800
$Comp
L Device:R R?
U 1 1 5FBD5AD0
P 6350 3450
F 0 "R?" H 6420 3496 50  0000 L CNN
F 1 "10" H 6420 3405 50  0000 L CNN
F 2 "" V 6280 3450 50  0001 C CNN
F 3 "~" H 6350 3450 50  0001 C CNN
	1    6350 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 4400 4150 4800
Connection ~ 4150 4400
Wire Wire Line
	5450 3700 6150 3700
Wire Wire Line
	4250 3600 4250 3700
$Comp
L Device:Battery_Cell BT?
U 1 1 5FBD92D9
P 5750 4700
F 0 "BT?" H 6000 4800 50  0000 C CNN
F 1 "250mAh" H 6000 4700 50  0000 C CNN
F 2 "" V 5750 4760 50  0001 C CNN
F 3 "~" V 5750 4760 50  0001 C CNN
	1    5750 4700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4150 4800 5600 4800
Connection ~ 5750 4500
Wire Wire Line
	5750 4500 5450 4500
Connection ~ 5750 4800
Wire Wire Line
	5750 4800 6300 4800
Wire Wire Line
	6750 4000 6750 4800
Connection ~ 6300 4500
Connection ~ 6300 4800
Wire Wire Line
	6300 4800 6750 4800
Wire Wire Line
	5450 3800 6750 3800
Wire Wire Line
	5600 3900 5600 4100
Wire Wire Line
	5450 3900 5600 3900
Wire Wire Line
	5600 4400 5600 4800
Connection ~ 5600 4800
Wire Wire Line
	5600 4800 5750 4800
Connection ~ 5600 3900
Wire Wire Line
	6750 3900 6250 3900
Wire Wire Line
	6350 2600 6350 2900
Wire Wire Line
	5450 2600 5600 2600
$Comp
L Device:LED D?
U 1 1 5FC2FAC7
P 5850 3100
F 0 "D?" H 5700 2950 50  0000 C CNN
F 1 "LED" H 5700 2850 50  0000 C CNN
F 2 "" H 5850 3100 50  0001 C CNN
F 3 "~" H 5850 3100 50  0001 C CNN
	1    5850 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5450 3100 5700 3100
Connection ~ 6600 3600
Wire Wire Line
	6600 3600 6350 3600
$Comp
L Device:R R?
U 1 1 5FBC87D2
P 5600 4250
F 0 "R?" H 5750 4300 50  0000 C CNN
F 1 "10k" H 5750 4200 50  0000 C CNN
F 2 "" V 5530 4250 50  0001 C CNN
F 3 "~" H 5600 4250 50  0001 C CNN
	1    5600 4250
	1    0    0    -1  
$EndComp
Text Notes 4250 3200 0    50   ~ 0
UART1 TX
Text Notes 4250 3100 0    50   ~ 0
UART1 RX\n
Wire Wire Line
	5600 2700 5600 2600
Connection ~ 5600 2600
Wire Wire Line
	5600 2600 6000 2600
Wire Wire Line
	5600 3000 5600 3900
Wire Wire Line
	5750 4500 6300 4500
Text Notes 3700 3100 0    50   ~ 0
TX
Text Notes 3700 3200 0    50   ~ 0
RX
Text Notes 3700 3300 0    50   ~ 0
5V
Text Notes 3700 3400 0    50   ~ 0
G
Connection ~ 6350 2600
Wire Wire Line
	6000 2700 6000 2600
Connection ~ 6000 2600
Wire Wire Line
	6000 2600 6350 2600
Wire Wire Line
	6000 3000 6000 3100
Wire Wire Line
	6350 2600 6600 2600
Wire Wire Line
	6600 3000 6600 3600
Wire Wire Line
	6600 2700 6600 2600
$Comp
L Device:D D?
U 1 1 5FC34600
P 6600 2850
F 0 "D?" V 6554 2930 50  0000 L CNN
F 1 "1N5819" V 6645 2930 50  0000 L CNN
F 2 "" H 6600 2850 50  0001 C CNN
F 3 "~" H 6600 2850 50  0001 C CNN
	1    6600 2850
	0    1    1    0   
$EndComp
$Comp
L Device:R R?
U 1 1 5FC1A3F2
P 6000 2850
F 0 "R?" H 5850 2800 50  0000 C CNN
F 1 "10k" H 5850 2900 50  0000 C CNN
F 2 "" V 5930 2850 50  0001 C CNN
F 3 "~" H 6000 2850 50  0001 C CNN
	1    6000 2850
	-1   0    0    1   
$EndComp
$Comp
L Transistor_FET:AO3401A Q?
U 1 1 60218619
P 6250 3100
F 0 "Q?" H 6000 3250 50  0000 L CNN
F 1 "AO3401A" H 5950 3350 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 6450 3025 50  0001 L CIN
F 3 "http://www.aosmd.com/pdfs/datasheet/AO3401A.pdf" H 6250 3100 50  0001 L CNN
	1    6250 3100
	1    0    0    1   
$EndComp
Wire Wire Line
	6300 4500 6600 4500
Wire Wire Line
	6600 4500 6600 3600
Wire Wire Line
	6000 3100 6050 3100
Connection ~ 6000 3100
Text Notes 5400 3800 0    50   ~ 0
UART2 TX
Text Notes 5400 3700 0    50   ~ 0
UART2 RX\n
Wire Wire Line
	4050 3400 4150 3400
Wire Wire Line
	4050 3300 4250 3300
Wire Wire Line
	4550 3200 4050 3200
Wire Wire Line
	4050 3100 4550 3100
$Comp
L Connector:Conn_01x04_Male J?
U 1 1 5FBCF7C1
P 3850 3200
F 0 "J?" H 3950 3550 50  0000 C CNN
F 1 "FC UART" H 3950 3450 50  0000 C CNN
F 2 "" H 3850 3200 50  0001 C CNN
F 3 "~" H 3850 3200 50  0001 C CNN
	1    3850 3200
	1    0    0    -1  
$EndComp
$Comp
L Device:D D?
U 1 1 5FC14E46
P 4250 3450
F 0 "D?" V 4350 3400 50  0000 R CNN
F 1 "1N5819" V 4250 3400 50  0000 R CNN
F 2 "" H 4250 3450 50  0001 C CNN
F 3 "~" H 4250 3450 50  0001 C CNN
	1    4250 3450
	0    -1   -1   0   
$EndComp
$EndSCHEMATC
