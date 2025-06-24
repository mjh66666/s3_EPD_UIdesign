/***
 * @Author: mojionghao
 * @Date: 2025-06-17 15:36:00
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-06-24 11:33:17
 * @FilePath: \s3_EPD_UIdesign\src\button\button.cpp
 * @Description:
 */
#include "button.h"

Button::Button(uint8_t pin, void (*shortPressCallback)(), void (*longPressCallback)(), unsigned long debounceDelay, unsigned long longPressDelay)
	: _pin(pin), _shortPressCallback(shortPressCallback), _longPressCallback(longPressCallback), _debounceDelay(debounceDelay), _longPressDelay(longPressDelay)
{
	Serial.printf("Button initialized on pin %d\n", _pin);
	pinMode(_pin, INPUT_PULLUP);
	_queue = xQueueCreate(10, sizeof(uint8_t));
	xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 2048, this, 1, NULL, 0);
	xTaskCreatePinnedToCore(eventHandlerTask, "ButtonEventHandler", 2048, this, 1, NULL, 0);
}

Button::~Button()
{
	vQueueDelete(_queue);
}

void Button::buttonTask(void *param)
{
	Button *button = static_cast<Button *>(param);
	unsigned long lastDebounceTime = 0;
	unsigned long pressStartTime = 0;
	int lastState = HIGH;
	bool longPressSent = false; // 新增标志
	Serial.printf("Button task started on pin %d\n", button->_pin);
	while (true) {
		int currentState = digitalRead(button->_pin); // 读取当前按钮状态

		if (currentState != lastState) { //电平发生变化
			lastDebounceTime = millis(); //记录电平变化时间
			if (currentState == LOW) {
				pressStartTime = millis();//记录按下的时间
				longPressSent = false; // 按下时重置
			}
			// 松开时检测短按（且未触发过长按）
			if (currentState == HIGH && lastState == LOW) {
				if (!longPressSent && (millis() - pressStartTime) >= button->_debounceDelay) {
					uint8_t event = 1;
					Serial.println("Button short press detected");
					xQueueSend(button->_queue, &event, portMAX_DELAY);
				}
			}
		}
		// 按下且超过长按时间，且未触发过长按
		if (currentState == LOW && !longPressSent && (millis() - pressStartTime) > button->_longPressDelay) {
			uint8_t event = 2;
			Serial.println("Button long press detected");
			xQueueSend(button->_queue, &event, portMAX_DELAY);
			longPressSent = true;
		}
		lastState = currentState;
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void Button::eventHandlerTask(void *param)
{
	Button *button = static_cast<Button *>(param);
	uint8_t event;
	Serial.printf("Button event handler task started on pin %d\n", button->_pin);
	while (true) {
		if (xQueueReceive(button->_queue, &event, portMAX_DELAY)) {
			if (event == 1 && button->_shortPressCallback) {
				button->_shortPressCallback();
			}
			else if (event == 2 && button->_longPressCallback) {
				button->_longPressCallback();
			}
		}
	}
}