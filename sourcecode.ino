#include <FreeRTOSConfig.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Khai báo LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Cảm biến hồng ngoại
#define sensor_1_on  4  // Cảm biến trên băng chuyền 1 (trên)
#define sensor_1_bottom 14 // Cảm biến dưới băng chuyền 1 (dưới)
#define sensor_2 15   // Cảm biến băng chuyền 2

//buzzer
#define buzzer_1 32// buzzer cho băng truyền 1
#define buzzer_2 33// buzzer cho băng truyền 2

// LED
#define LED_sem_1 25  // LED cho băng chuyền 1
#define LED_sem_2 26  // LED cho băng chuyền 2

// Biến đếm số lượng ô vuông
int count_1 = 0;
int count_2 = 0;

// Biến đếm số lượng thùng
int box_count_1 = 0;
int box_count_2 = 0;
int box_soluong_1 = 0;
int box_soluong_2 = 0;

// để nháy LED
int led_soluong_1 = 0;
int led_soluong_2 = 0;


SemaphoreHandle_t Sem_Handle;

// Khai báo các hàm tác vụ
void countTaskBelt1(void *pvParameters);
void countTaskBelt2(void *pvParameters);
void LEDTask(void *pvParameters);
void displayTask(void *pvParameters);
void buzzerTask(void *pvParameters);

void setup() {
  // Khởi tạo LCD
  lcd.init();
  //lcd.backlight();

  // Cấu hình cảm biến
  Serial.begin(9600);
  pinMode(sensor_1_on, INPUT_PULLUP);
  pinMode(sensor_1_bottom, INPUT_PULLUP);
  pinMode(sensor_2, INPUT_PULLUP);
  pinMode(buzzer_1, OUTPUT);
  pinMode(buzzer_2, OUTPUT);

  // Cấu hình LED
  pinMode(LED_sem_1, OUTPUT);
  pinMode(LED_sem_2, OUTPUT);

  // Khởi tạo semaphore
  Sem_Handle = xSemaphoreCreateMutex();
  if (Sem_Handle == NULL) {
    Serial.println("Semaphore creation failed!");
  }

  // Tạo các tác vụ RTOS
  xTaskCreate(countTaskBelt1, "Task1", 4096, NULL, 3, NULL);
  xTaskCreate(countTaskBelt2, "Task2", 4096, NULL, 3, NULL);
  xTaskCreate(LEDTask, "Task3", 4096, NULL, 2, NULL);
  xTaskCreate(displayTask, "Task4", 4096, NULL, 4, NULL);
  xTaskCreate(buzzerTask, "Task5", 4096, NULL, 2 , NULL);

  // Hiển thị giá trị ban đầu lên LCD
  lcd.setCursor(0, 0);
  lcd.print("Belt1:00");
  lcd.setCursor(0, 1);
  lcd.print("Belt2:00");
  lcd.setCursor(8, 0);
  lcd.print("Boxes:00");
  lcd.setCursor(8, 1);
  lcd.print("Boxes:00");
}

void loop() {
  // Không cần nội dung trong vòng lặp chính
}


// Tác vụ đếm cho băng chuyền 1
void countTaskBelt1(void *pvParameters) {
  while (1) {
    if (digitalRead(sensor_1_bottom) == HIGH) { // Dưới bị kích hoạt
      if (digitalRead(sensor_1_on) == HIGH) { // Trên cũng bị kích hoạt
        if (xSemaphoreTake(Sem_Handle, pdMS_TO_TICKS(100)) == pdTRUE) {
          count_1 += 2; // Ô vuông xếp chồng
          led_soluong_1++;
          xSemaphoreGive(Sem_Handle);
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Chờ tránh đếm lại
      } else {
        if (xSemaphoreTake(Sem_Handle, pdMS_TO_TICKS(100)) == pdTRUE) {
          count_1++; // Ô vuông đơn
          xSemaphoreGive(Sem_Handle);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
      }
    }

    // Kiểm tra và tăng số lượng thùng
    if (xSemaphoreTake(Sem_Handle, pdMS_TO_TICKS(100)) == pdTRUE) {
      int newBoxCount_1 = count_1 / 2; // Tính số lượng thùng mới
      if (newBoxCount_1 > box_count_1) { // Nếu có thêm thùng
        box_count_1 = newBoxCount_1;
        box_soluong_1++; // Tăng số lượng thùng cho buzzer
      }
      xSemaphoreGive(Sem_Handle);
    }

    vTaskDelay(pdMS_TO_TICKS(2000)); 
  }
}


// Tác vụ đếm cho băng chuyền 2
void countTaskBelt2(void *pvParameters) {
  unsigned long starttime = 0;
  bool Counting = false;

  while (1) {
    if (digitalRead(sensor_2) == HIGH) { // Cảm biến bị kích hoạt
      if (!Counting) {
        Counting = true;
        starttime = millis();
      }
    } else {
      if (Counting) {
        unsigned long duration = millis() - starttime;
        if (xSemaphoreTake(Sem_Handle, pdMS_TO_TICKS(100)) == pdTRUE) {
          if (duration > 3000) {
            count_2 += 2; // Hai ô vuông dính liền
            led_soluong_2++;
          } else {
            count_2++; // Một ô vuông
            led_soluong_2++;
          }

          // Kiểm tra và tăng số lượng thùng
          int newBoxCount_2 = count_2 / 2; // Tính số lượng thùng mới
          if (newBoxCount_2 > box_count_2) { // Nếu có thùng mới
            box_count_2 = newBoxCount_2;
            box_soluong_2++; // Tăng số lượng thùng cho buzzer
          }
          xSemaphoreGive(Sem_Handle);
        }
        Counting = false;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}


// Tác vụ nháy LED
void LEDTask(void *pvParameters) {
   while (1) { 
  // Nháy LED băng chuyền 1 
   if (xSemaphoreTake(Sem_Handle, pdMS_TO_TICKS(portMAX_DELAY)) == pdTRUE) {
     if (led_soluong_1 > 0) {
       digitalWrite(LED_sem_1, HIGH);
       vTaskDelay(pdMS_TO_TICKS(500)); // LED sáng 500ms 
       digitalWrite(LED_sem_1, LOW);
       vTaskDelay(pdMS_TO_TICKS(500)); // LED tắt 500ms
       led_soluong_1--; } 
  // Nháy LED băng chuyền 2 
     if (led_soluong_2 > 0) {
        digitalWrite(LED_sem_2, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500)); // LED sáng 500ms 
        digitalWrite(LED_sem_2, LOW); 
        vTaskDelay(pdMS_TO_TICKS(500)); // LED tắt 500ms
        led_soluong_2--; } 

        xSemaphoreGive(Sem_Handle); } 
        vTaskDelay(pdMS_TO_TICKS(100)); }
}

// Tác vụ buzzer
void buzzerTask(void *pvParameters) {
   while (1) { 
    // buzzer băng chuyền 1 
   if (xSemaphoreTake(Sem_Handle, pdMS_TO_TICKS(portMAX_DELAY)) == pdTRUE) {
     if (box_soluong_1 > 0) {
        digitalWrite(buzzer_1, HIGH);
        vTaskDelay(pdMS_TO_TICKS(200)); // buzzer kêu 200ms 
        digitalWrite(buzzer_1, LOW); 
        vTaskDelay(pdMS_TO_TICKS(200)); // buzzer tắt 200ms
        box_soluong_1--; 
        } 
    // buzzer băng chuyền 2 
     if (box_soluong_2 > 0) {
        tone(buzzer_2, 2500); // Phát tín hiệu 2.5 kHz
        vTaskDelay(pdMS_TO_TICKS(200)); // buzzer kêu 200ms 
        noTone(buzzer_2); 
        vTaskDelay(pdMS_TO_TICKS(200)); // buzzer tắt 200ms
        box_soluong_2--; 
        } 
        xSemaphoreGive(Sem_Handle); 
        } 
        vTaskDelay(pdMS_TO_TICKS(100)); }
}

// Tác vụ hiển thị lên LCD
void displayTask(void *pvParameters) {
  static int lastCount1 = -1, lastCount2 = -1, lastBoxCount1 = -1, lastBoxCount2 = -1;

  while (1) {
    if (xSemaphoreTake(Sem_Handle, pdMS_TO_TICKS(portMAX_DELAY)) == pdTRUE) {
      // Cập nhật LCD chỉ khi có thay đổi
      if (count_1 != lastCount1) {
        lcd.setCursor(0, 0);
        lcd.print("Belt1:"); // In lại tiêu đề
        lcd.print(count_1 / 10);  // Hiển thị hàng chục
        lcd.print(count_1 % 10);  // Hiển thị hàng đơn vị
        lastCount1 = count_1; // Cập nhật giá trị đã in ra
      }

      if (count_2 != lastCount2) {
        lcd.setCursor(0, 1);
        lcd.print("Belt2:"); // In lại tiêu đề
        lcd.print(count_2 / 10);  // Hiển thị hàng chục
        lcd.print(count_2 % 10);  // Hiển thị hàng đơn vị
        lastCount2 = count_2; // Cập nhật giá trị đã in ra
      }

      if (box_count_1 != lastBoxCount1) {
        lcd.setCursor(8, 0);
        lcd.print("Boxes:"); // In lại tiêu đề
        lcd.print(box_count_1 / 10);  // Hiển thị hàng chục
        lcd.print(box_count_1 % 10);  // Hiển thị hàng đơn vị
        lastBoxCount1 = box_count_1; // Cập nhật giá trị đã in ra
      }

      if (box_count_2 != lastBoxCount2) {
        lcd.setCursor(8, 1);
        lcd.print("Boxes:"); // In lại tiêu đề
        lcd.print(box_count_2 / 10);  // Hiển thị hàng chục
        lcd.print(box_count_2 % 10);  // Hiển thị hàng đơn vị
        lastBoxCount2 = box_count_2; // Cập nhật giá trị đã in ra
      }

      xSemaphoreGive(Sem_Handle);
    }

    vTaskDelay(pdMS_TO_TICKS(100)); // Thời gian cập nhật lại LCD
  }
}


