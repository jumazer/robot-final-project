[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/_AmpSwBJ)
[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-2e0aaae1b6195c2367325f4f02e2d04e9abb55f0b24a779b69b11b9e10269abc.svg)](https://classroom.github.com/online_ide?assignment_repo_id=23406162&assignment_repo_type=AssignmentRepo)
# ecen_5813_assignment7_s26

# PES Assignment 7: Waveform Generation and Analysis

This project implements Assignment 7 for ECEN 5813: Principles of Embedded Software on the STM32 Nucleo-F091RC.

The program generates analog waveforms with the DAC using DMA and TIMER6, samples the output with the ADC using TIMER1 as a hardware trigger, captures 1024 ADC samples, and analyzes the captured waveform for minimum, maximum, average, period, and observed frequency. The project cycles through four repeating waveforms: 450 Hz triangle, 550 Hz sine, 650 Hz triangle, and 750 Hz sine. Each waveform uses 512 samples per period and plays for two seconds before advancing to the next waveform.

## Features

- Generates four repeating output waveforms:
  - 450 Hz triangle
  - 550 Hz sine
  - 650 Hz triangle
  - 750 Hz sine
- Uses a 512-sample lookup buffer for each waveform
- Uses DMA + DAC + TIMER6 for continuous waveform playback
- Uses ADC + TIMER1 hardware triggering for 48 kHz waveform capture
- Captures 1024 ADC samples for each analysis pass
- Computes:
  - minimum
  - maximum
  - average
  - detected period using autocorrelation
  - observed input frequency
- Prints waveform start messages and waveform analysis results over UART/debug output

## Project Structure

- `main.c`  
  Main application loop. Initializes analog output and analog input, captures ADC samples, and runs waveform analysis.

- `analog_out.c`  
  Controls waveform playback state, waveform switching, DMA playback startup, and the DMA interrupt handler used to advance to the next waveform every two seconds.

- `analog_in.c`  
  Starts ADC capture, stores 1024 samples into a local buffer, and analyzes the captured waveform.

- `tone_to_samples.c`  
  Generates the lookup-table sample buffers for the triangle and sine waveforms.

- `timer.c`  
  Configures:
  - TIMER6 for DAC output timing
  - TIMER1 for ADC triggering at 48 kHz

- `dma.c`  
  Configures DMA2 Channel 3 for circular memory-to-DAC transfers.

- `dac.c`  
  Initializes DAC channel 1 on PA4 for analog output.

- `adc.c`  
  Initializes ADC1 for 10-bit sampling on channel 4 using TIM1_TRGO as the external trigger.

## How It Works

### Waveform Output
The output module precomputes one period of each waveform into 512-sample buffers. TIMER6 generates periodic update events, DMA moves waveform samples from memory to the DAC, and the DAC outputs the analog signal on PA4. The waveform changes every two seconds.

### Waveform Input
The input module configures ADC1 to sample channel 4 and uses TIMER1_TRGO to trigger conversions at 48 kHz. A polling loop collects 1024 ADC samples into a buffer.

### Waveform Analysis
After capture, the program computes the minimum, maximum, and average ADC value. It then calls the provided autocorrelation function to estimate the waveform period in samples and converts that into an observed frequency in Hz.

## Expected UART / Debug Output

When a waveform starts, the program prints a message such as:

Starting 450 Hz waveform generation with 512 samples

After each ADC capture and analysis pass, the program prints a line similar to:

min=... max=... avg=... period=... observed samples frequency=... Hz

## Hardware Notes

- Board: STM32 Nucleo-F091RC
- DAC output pin: `PA4`
- ADC input channel: channel 4

## Oscilloscope Screenshots

Oscilloscope screenshots for the four generated waveforms are included in the root folder:

`Oscilloscope_Screenshots`

These screenshots were taken to verify waveform shape, frequency, and that there is no visible tearing at the waveform wrap point.

## Notes

- The waveform buffers are scaled to match the required voltage range for the assignment.
- The autocorrelation code used for period detection was provided and integrated into the waveform analysis stage.
- The project is organized into separate modules for analog output, analog input, timers, DMA, ADC, DAC, and waveform generation.