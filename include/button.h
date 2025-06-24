#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

class Button {
public:
    Button(uint8_t pin, void (*shortPressCallback)(), void (*longPressCallback)() = nullptr, unsigned long debounceDelay = 50, unsigned long longPressDelay = 1000);
    ~Button();

private:
    uint8_t _pin;
    void (*_shortPressCallback)();
    void (*_longPressCallback)();
    unsigned long _debounceDelay;
    unsigned long _longPressDelay;
    QueueHandle_t _queue;

    static void buttonTask(void *param);
    static void eventHandlerTask(void *param);
};

#endif