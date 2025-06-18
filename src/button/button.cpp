/***
 * @Author: mojionghao
 * @Date: 2025-06-17 15:36:00
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-06-17 19:52:05
 * @FilePath: \s3_EPD_UIdesign\src\button.cpp
 * @Description:
 */
/***
 * @Author: mojionghao
 * @Date: 2025-06-17 15:36:00
 * @LastEditors: mojionghao
 * @LastEditTime: 2025-06-17 16:03:04
 * @FilePath: \s3_EPD_UIdesign\src\button.cpp
 * @Description:
 */
#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

class Button {
public:
	// 构造函数，初始化按钮引脚、短按和长按回调函数
	Button(uint8_t pin, void (*shortPressCallback)(), void (*longPressCallback)() = nullptr, unsigned long debounceDelay = 50, unsigned long longPressDelay = 1000)
		: _pin(pin), _shortPressCallback(shortPressCallback), _longPressCallback(longPressCallback), _debounceDelay(debounceDelay), _longPressDelay(longPressDelay)
	{
		pinMode(_pin, INPUT_PULLUP); // 设置为输入模式，使用内部上拉电阻
		_queue = xQueueCreate(10, sizeof(uint8_t)); // 创建队列
		xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 2048, this, 1, NULL, 0); // 创建任务
	}

	// 析构函数，删除队列
	~Button()
	{
		vQueueDelete(_queue); // 删除队列
	}

private:
	uint8_t _pin;                  // 按钮引脚
	void (*_shortPressCallback)(); // 短按回调函数
	void (*_longPressCallback)();  // 长按回调函数
	unsigned long _debounceDelay;  // 去抖时间（毫秒）
	unsigned long _longPressDelay; // 长按时间（毫秒）
	QueueHandle_t _queue;          // FreeRTOS 队列

	// 按钮任务
	static void buttonTask(void *param)
	{
		Button *button = static_cast<Button *>(param);
		unsigned long lastDebounceTime = 0;
		unsigned long pressStartTime = 0;
		int lastState = HIGH;

		while (true) {
			int currentState = digitalRead(button->_pin);

			// 检查状态是否发生变化
			if (currentState != lastState) {
				lastDebounceTime = millis(); // 记录状态变化的时间
				if (currentState == LOW) {
					pressStartTime = millis(); // 记录按下的开始时间
				}
			}

			// 如果状态稳定超过去抖时间
			if ((millis() - lastDebounceTime) > button->_debounceDelay) {
				if (currentState == LOW && lastState == HIGH) { // 短按事件
					uint8_t event = 1; // 短按事件
					xQueueSend(button->_queue, &event, portMAX_DELAY);
				}

				if (currentState == LOW && (millis() - pressStartTime) > button->_longPressDelay) { // 长按事件
					uint8_t event = 2; // 长按事件
					xQueueSend(button->_queue, &event, portMAX_DELAY);
				}
			}

			lastState = currentState; // 更新按钮状态
			vTaskDelay(pdMS_TO_TICKS(10)); // 延时 10 毫秒，避免占用过多 CPU
		}
	}

	// 按钮事件处理任务
	static void eventHandlerTask(void *param)
	{
		Button *button = static_cast<Button *>(param);
		uint8_t event;

		while (true) {
			if (xQueueReceive(button->_queue, &event, portMAX_DELAY)) {
				if (event == 1 && button->_shortPressCallback) {
					button->_shortPressCallback(); // 调用短按回调函数
				}
				else if (event == 2 && button->_longPressCallback) {
					button->_longPressCallback(); // 调用长按回调函数
				}
			}
		}
	}
};

#endif