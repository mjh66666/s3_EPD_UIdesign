#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

class Button {
public:
	// 构造函数，初始化按钮引脚、回调函数和队列
	Button(uint8_t pin, void (*callback)(), unsigned long debounceDelay = 50)
		: _pin(pin), _callback(callback), _debounceDelay(debounceDelay)
	{
		pinMode(_pin, INPUT_PULLUP); // 设置为输入模式，使用内部上拉电阻
		_queue = xQueueCreate(10, sizeof(uint8_t)); // 创建队列
		xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 2048, this, 1, NULL, 0); // 创建任务
	}

	~Button()
	{
		vQueueDelete(_queue); // 删除队列
	}

private:
	uint8_t _pin;                  // 按钮引脚
	void (*_callback)();           // 回调函数
	unsigned long _debounceDelay;  // 去抖时间（毫秒）
	QueueHandle_t _queue;          // FreeRTOS 队列

	// 按钮任务
	static void buttonTask(void *param)
	{
		Button *button = static_cast<Button *>(param);
		unsigned long lastDebounceTime = 0;
		int lastState = HIGH;

		while (true) {
			int currentState = digitalRead(button->_pin);

			// 检查状态是否发生变化
			if (currentState != lastState) {
				lastDebounceTime = millis(); // 记录状态变化的时间
			}

			// 如果状态稳定超过去抖时间，并且是低电平触发
			if ((millis() - lastDebounceTime) > button->_debounceDelay && currentState == LOW) {
				if (lastState == HIGH) { // 从高电平变为低电平
					uint8_t event = 1;   // 按钮事件
					xQueueSend(button->_queue, &event, portMAX_DELAY); // 将事件发送到队列
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
				if (event == 1) {
					button->_callback(); // 调用回调函数
				}
			}
		}
	}
};

#endif