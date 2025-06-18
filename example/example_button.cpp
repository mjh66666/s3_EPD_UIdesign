#include <Button.h>

#define BUTTON_PIN 0 // 定义按钮引脚

// 短按回调函数
void onShortPress() {
    Serial.println("短按事件触发！");
}

// 长按回调函数
void onLongPress() {
    Serial.println("长按事件触发！");
}

// 创建按钮对象，启用短按和长按功能
Button button(BUTTON_PIN, onShortPress, onLongPress, 50, 1000);

void setup() {
    Serial.begin(115200);
}

void loop() {
    // 主循环无需处理按钮逻辑，FreeRTOS 任务会自动处理
}