#include "tim_param_calc.h"

#include <math.h>


typedef struct {
    uint32_t prescaler;
    uint32_t period;
    uint32_t actual_frequency;
    float frequency_error;
} tim_calc_result_t;

// Function to find the best prescaler and period combination
tim_calc_result_t tim_calculate_optimal_params(uint32_t system_clock, uint32_t desired_freq, uint16_t max_counter_depth) {
    tim_calc_result_t result = {0};
    float best_error = 100.0f; // Start with 100% error
    uint32_t best_prescaler = 1;
    uint32_t best_period = 1;
    uint32_t best_actual_freq = 0;
    
    // Calculate the total division needed
    uint32_t total_division = system_clock / desired_freq;
    
    // Try different prescaler values
    for (uint32_t prescaler = 1; prescaler <= 65536; prescaler++) {
        // Calculate required period for this prescaler
        uint32_t period = total_division / prescaler;
        
        // Check if period is within valid range
        if (period > 0 && period <= max_counter_depth) {
            // Calculate actual frequency with this combination
            uint32_t actual_freq = system_clock / (prescaler * period);
            
            // Calculate error percentage
            float error = fabsf((float)(actual_freq - desired_freq)) / desired_freq * 100.0f;
            
            // Update best combination if this is better
            if (error < best_error) {
                best_error = error;
                best_prescaler = prescaler;
                best_period = period;
                best_actual_freq = actual_freq;
                
                // If we found exact match, break early
                if (error < 0.01f) break;
            }
        }
        
        // Early termination if prescaler is getting too large
        if (total_division / prescaler < 1) break;
    }
    
    result.prescaler = best_prescaler;
    result.period = best_period;
    result.actual_frequency = best_actual_freq;
    result.frequency_error = best_error;
    
    return result;
}

// Enhanced timer configuration function
int tim_config(TIM_HandleTypeDef *htim, TIM_TypeDef *instance, uint32_t desired_tick_hz, uint16_t counter_depth) {
    if (!htim || !instance || desired_tick_hz == 0) {
        return -1; // Invalid parameters
    }
    
    // Calculate optimal parameters
    tim_calc_result_t calc = tim_calculate_optimal_params(SystemCoreClock, desired_tick_hz, counter_depth);
    
    // Check if we found a valid solution
    if (calc.prescaler == 0 || calc.period == 0) {
        return -2; // No valid configuration found
    }
    
    // Configure the timer
    htim->Instance = instance;
    htim->Init.Prescaler = calc.prescaler - 1;  // STM32 uses 0-based prescaler
    htim->Init.Period = calc.period - 1;        // STM32 uses 0-based period
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.RepetitionCounter = 0;
    
    return 0; // Success
}

// Alternative simpler approach using mathematical optimization
/*
int tim_config(TIM_HandleTypeDef *htim, TIM_TypeDef *instance, uint32_t desired_tick_hz, uint16_t counter_depth) {
    if (!htim || !instance || desired_tick_hz == 0) {
        return -1;
    }
    
    uint32_t total_division = SystemCoreClock / desired_tick_hz;
    
    // Find the best prescaler that gives us a period within range
    uint32_t prescaler = 1;
    uint32_t period = total_division;
    
    // Adjust prescaler to bring period within valid range
    while (period > counter_depth && prescaler <= 65536) {
        prescaler++;
        period = total_division / prescaler;
    }
    
    // If we still can't fit, use maximum values
    if (period > counter_depth) {
        prescaler = 65536;
        period = counter_depth;
    }
    
    // Ensure minimum values
    if (prescaler < 1) prescaler = 1;
    if (period < 1) period = 1;
    
    htim->Instance = instance;
    htim->Init.Prescaler = prescaler - 1;
    htim->Init.Period = period - 1;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.RepetitionCounter = 0;
    
    return 0;
}
*/