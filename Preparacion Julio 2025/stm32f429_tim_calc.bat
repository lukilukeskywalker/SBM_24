@echo off
setlocal enabledelayedexpansion

echo ========================================
echo STM32F429 Timer Frequency Calculator
echo ========================================
echo.

:main_menu
echo Select calculation mode:
echo 1. Quick calculation (default clocks)
echo 2. Custom clock configuration
echo 3. Exit
echo.
set /p mode="Enter choice (1-3): "

if "%mode%"=="1" goto quick_calc
if "%mode%"=="2" goto custom_calc
if "%mode%"=="3" goto end
echo Invalid choice. Please try again.
echo.
goto main_menu

:quick_calc
echo.
echo === Quick Calculation Mode ===
echo Using default clock configuration:
echo - HSE = 8 MHz
echo - PLL: M=8, N=336, P=2
echo - SYSCLK = 168 MHz
echo - AHB Prescaler = 1 (HCLK = 168 MHz)
echo - APB1 Prescaler = 4 (PCLK1 = 42 MHz)
echo - APB2 Prescaler = 2 (PCLK2 = 84 MHz)
echo.

set HSE=8000000
set M=8
set N=336
set P=2
set AHB_PRESCALER=1
set APB1_PRESCALER=4
set APB2_PRESCALER=2

goto get_timer_params

:custom_calc
echo.
echo === Custom Clock Configuration ===
echo.

:get_hse
set /p HSE="Enter HSE frequency (Hz, default 8000000): "
if "%HSE%"=="" set HSE=8000000

:get_pll_m
set /p M="Enter PLL M divider (2-63, default 8): "
if "%M%"=="" set M=8

:get_pll_n
set /p N="Enter PLL N multiplier (50-432, default 336): "
if "%N%"=="" set N=336

:get_pll_p
set /p P="Enter PLL P divider (2,4,6,8, default 2): "
if "%P%"=="" set P=2

:get_ahb
echo.
echo AHB Prescaler options: 1, 2, 4, 8, 16, 64, 128, 256, 512
set /p AHB_PRESCALER="Enter AHB prescaler (default 1): "
if "%AHB_PRESCALER%"=="" set AHB_PRESCALER=1

:get_apb1
echo.
echo APB1 Prescaler options: 1, 2, 4, 8, 16
set /p APB1_PRESCALER="Enter APB1 prescaler (default 4): "
if "%APB1_PRESCALER%"=="" set APB1_PRESCALER=4

:get_apb2
echo.
echo APB2 Prescaler options: 1, 2, 4, 8, 16
set /p APB2_PRESCALER="Enter APB2 prescaler (default 2): "
if "%APB2_PRESCALER%"=="" set APB2_PRESCALER=2

:get_timer_params
echo.
echo === Timer Configuration ===
echo.

:get_timer_num
set /p TIMER_NUM="Enter Timer number (1-14): "
if "%TIMER_NUM%"=="" (
    echo Timer number cannot be empty!
    goto get_timer_num
)

:get_prescaler
set /p PRESCALER="Enter Timer Prescaler (0-65535): "
if "%PRESCALER%"=="" (
    echo Prescaler cannot be empty!
    goto get_prescaler
)

:get_period
set /p PERIOD="Enter Timer Period/ARR (1-65535 for 16-bit, 1-4294967295 for 32-bit): "
if "%PERIOD%"=="" (
    echo Period cannot be empty!
    goto get_period
)

echo.
echo === Calculation Results ===
echo.

rem Calculate PLL VCO frequency
set /a VCO_FREQ=%HSE% * %N% / %M%

rem Calculate SYSCLK
set /a SYSCLK=%VCO_FREQ% / %P%

rem Calculate HCLK (AHB clock)
set /a HCLK=%SYSCLK% / %AHB_PRESCALER%

rem Calculate PCLK1 (APB1 clock)
set /a PCLK1=%HCLK% / %APB1_PRESCALER%

rem Calculate PCLK2 (APB2 clock)
set /a PCLK2=%HCLK% / %APB2_PRESCALER%

echo Clock Configuration:
echo - HSE: %HSE% Hz
echo - VCO: %VCO_FREQ% Hz
echo - SYSCLK: %SYSCLK% Hz
echo - HCLK: %HCLK% Hz
echo - PCLK1: %PCLK1% Hz
echo - PCLK2: %PCLK2% Hz
echo.

rem Determine which APB bus the timer is on
set TIMER_CLOCK=0

rem APB1 timers (TIM2, TIM3, TIM4, TIM5, TIM6, TIM7, TIM12, TIM13, TIM14)
if %TIMER_NUM%==2 set TIMER_CLOCK=%PCLK1%
if %TIMER_NUM%==3 set TIMER_CLOCK=%PCLK1%
if %TIMER_NUM%==4 set TIMER_CLOCK=%PCLK1%
if %TIMER_NUM%==5 set TIMER_CLOCK=%PCLK1%
if %TIMER_NUM%==6 set TIMER_CLOCK=%PCLK1%
if %TIMER_NUM%==7 set TIMER_CLOCK=%PCLK1%
if %TIMER_NUM%==12 set TIMER_CLOCK=%PCLK1%
if %TIMER_NUM%==13 set TIMER_CLOCK=%PCLK1%
if %TIMER_NUM%==14 set TIMER_CLOCK=%PCLK1%

rem APB2 timers (TIM1, TIM8, TIM9, TIM10, TIM11)
if %TIMER_NUM%==1 set TIMER_CLOCK=%PCLK2%
if %TIMER_NUM%==8 set TIMER_CLOCK=%PCLK2%
if %TIMER_NUM%==9 set TIMER_CLOCK=%PCLK2%
if %TIMER_NUM%==10 set TIMER_CLOCK=%PCLK2%
if %TIMER_NUM%==11 set TIMER_CLOCK=%PCLK2%

if %TIMER_CLOCK%==0 (
    echo Error: Invalid timer number or timer not supported!
    echo Supported timers: 1-14
    goto get_timer_params
)

rem Apply timer clock multiplier rule
rem If APBx prescaler != 1, timer clock = 2 * PCLKx
set TIMER_INPUT_CLOCK=%TIMER_CLOCK%

if %TIMER_NUM%==2 if %APB1_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK1% * 2
if %TIMER_NUM%==3 if %APB1_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK1% * 2
if %TIMER_NUM%==4 if %APB1_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK1% * 2
if %TIMER_NUM%==5 if %APB1_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK1% * 2
if %TIMER_NUM%==6 if %APB1_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK1% * 2
if %TIMER_NUM%==7 if %APB1_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK1% * 2
if %TIMER_NUM%==12 if %APB1_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK1% * 2
if %TIMER_NUM%==13 if %APB1_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK1% * 2
if %TIMER_NUM%==14 if %APB1_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK1% * 2

if %TIMER_NUM%==1 if %APB2_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK2% * 2
if %TIMER_NUM%==8 if %APB2_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK2% * 2
if %TIMER_NUM%==9 if %APB2_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK2% * 2
if %TIMER_NUM%==10 if %APB2_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK2% * 2
if %TIMER_NUM%==11 if %APB2_PRESCALER% NEQ 1 set /a TIMER_INPUT_CLOCK=%PCLK2% * 2

rem Determine timer bus
set TIMER_BUS=APB1
if %TIMER_NUM%==1 set TIMER_BUS=APB2
if %TIMER_NUM%==8 set TIMER_BUS=APB2
if %TIMER_NUM%==9 set TIMER_BUS=APB2
if %TIMER_NUM%==10 set TIMER_BUS=APB2
if %TIMER_NUM%==11 set TIMER_BUS=APB2

echo Timer TIM%TIMER_NUM% Configuration:
echo - Bus: %TIMER_BUS%
echo - Input Clock: %TIMER_INPUT_CLOCK% Hz
echo - Prescaler: %PRESCALER%
echo - Period (ARR): %PERIOD%
echo.

rem Calculate timer frequency and period
set /a PRESCALER_PLUS_ONE=%PRESCALER% + 1
set /a PERIOD_PLUS_ONE=%PERIOD% + 1
set /a TIMER_FREQ=%TIMER_INPUT_CLOCK% / %PRESCALER_PLUS_ONE%
set /a OUTPUT_FREQ=%TIMER_FREQ% / %PERIOD_PLUS_ONE%

rem Calculate periods in microseconds (approximate)
set /a TIMER_PERIOD_US=1000000 / (%TIMER_FREQ% / 1000000) 2>nul
set /a OUTPUT_PERIOD_US=1000000 / (%OUTPUT_FREQ% / 1000000) 2>nul

echo === Results ===
echo Timer Clock Frequency: %TIMER_FREQ% Hz
echo Output Frequency: %OUTPUT_FREQ% Hz
if %OUTPUT_PERIOD_US% GTR 0 echo Output Period: ~%OUTPUT_PERIOD_US% microseconds

rem Convert to more readable units
if %OUTPUT_FREQ% LSS 1000 (
    echo Output Frequency: %OUTPUT_FREQ% Hz
) else if %OUTPUT_FREQ% LSS 1000000 (
    set /a FREQ_KHZ=%OUTPUT_FREQ% / 1000
    echo Output Frequency: ~%FREQ_KHZ% kHz
) else (
    set /a FREQ_MHZ=%OUTPUT_FREQ% / 1000000
    echo Output Frequency: ~%FREQ_MHZ% MHz
)

echo.
echo Formula used:
echo Timer_Freq = Timer_Input_Clock / (Prescaler + 1)
echo Output_Freq = Timer_Freq / (Period + 1)
echo.

:ask_continue
set /p continue="Calculate another timer? (y/n): "
if /i "%continue%"=="y" goto main_menu
if /i "%continue%"=="yes" goto main_menu
if /i "%continue%"=="n" goto end
if /i "%continue%"=="no" goto end
echo Please enter y or n.
goto ask_continue

:end
echo.
echo Thank you for using STM32F429 Timer Calculator!
pause
