#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <SPI.h>              // Arduino SPI library#include <Adafruit_GFX.h>    // Core graphics library
#include <Wire.h>
#define TFT_CS 9    // define chip select pin
#define TFT_DC 8    // define data/command pin
#define TFT_RST 12  // define reset pin, or set to -1 and connect to Arduino RESET pin
#define TFT_MOSI 11
#define TFT_SCLK 10
#define TFT_BL 25
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
/* Select the frequency we want to tune to by way
     * of selecting the channel for the desired frequency 
     */
uint16_t channel = 150;
/*
     * предполагаем что диапазон начинается с частоты 87.0MHz (для приведенных ниже настроек)
     * расстояние между каналами составляет 100kHz (0.1MHz) (per settings below)
     * тогда номер канала можно вычислить с помощью следующего выражения:
     *  
     * channel = (<desired freq in MHz> - 87.0) / 0.1 
     *
     * или можно записать это выражение в другом виде:
     * <10 x desired freq in MHz> - 870
     */
#define RDA5807FP_ADDRESS 0b0010000  // 0x10
#define BOOT_CONFIG_LEN 12
#define TUNE_CONFIG_LEN 4
/* 
 *  Эти байты устанавливают нашу первоначальную конфигурацию
 *  На этом этапе мы не беспокоимся о настройке приемника на нужный канал.
 *  Здесь мы производим сброс модуля.
 */
uint8_t boot_config[] = {
  /* register 0x02 */
  0b11000001,
  /* 
     * DHIZ audio output high-z disable
     * 1 = normal operation
     *
     * DMUTE mute disable 
     * 1 = normal operation
     *
     * MONO mono select
     * 0 = stereo
     *
     * BASS bass boost
     * 0 = disabled
     *
     * RCLK NON-CALIBRATE MODE 
     * 0 = RCLK is always supplied
     *
     * RCLK DIRECT INPUT MODE 
     * 0 = ??? not certain what this does
     *
     * SEEKUP
     * 0 = seek in down direction
     *
     * SEEK
     * 0 = disable / stop seek (i.e. don't seek)
     */
  0b00000111,
  /* 
     * SKMODE seek mode: 
     * 0 = wrap at upper or lower band limit and contiue seeking
     *
     * CLK_MODE clock mode
     *  000 = 32.768kHZ clock rate (match the watch cystal on the module) 
     *
     * RDS_EN radio data system enable
     * 0 = disable radio data system
     *
     * NEW_METHOD use new demodulate method for improved sensitivity
     * 0 = presumably disabled 
     *
     * SOFT_RESET
     * 1 = perform a reset
     *
     * ENABLE power up enable: 
     * 1 = enabled
     */
  /* register 0x03 */
  /* Don't bother to tune to a channel at this stage*/
  0b00000000,
  /* 
     * CHAN channel select 8 most significant bits of 10 in total
     * 0000 0000 = don't boher to program a channel at this time
     */
  0b00000000,
  /* 
     * CHAN two least significant bits of 10 in total 
     * 00 = don't bother to program a channel at this time
     *
     * DIRECT MODE used only when test
     * 0 = presumably disabled
     *
     * TUNE commence tune operation 
     * 0 = disable (i.e. don't tune to selected channel)
     *
     * BAND band select
     * 00 = select the 87-108MHz band
     *
     * SPACE channel spacing
     * 00 = select spacing of 100kHz between channels
     */
  /* register 0x04 */
  0b00001010,
  /* 
     * RESERVED 15
     * 0
     *
     * PRESUMABLY RESERVED 14
     * 0
     *
     * RESERVED 13:12
     * 00
     *
     * DE de-emphasis: 
     * 1 = 50us de-emphasis as used in Australia
     *
     * RESERVED
     * 0
     *
     * SOFTMUTE_EN
     * 1 = soft mute enabled
     *
     * AFCD AFC disable
     * 0 = AFC enabled
     */
  0b00000000,
  /* 
     *  Bits 7-0 are not specified, so assume all 0's
     * 0000 0000
     */
  /* register 0x05 */
  0b10001000,
  /* 
     * INT_MODE
     * 1 = interrupt last until read reg 0x0C
     *
     * RESERVED 14:12 
     * 000
     *
     * SEEKTH seek signal to noise ratio threshold
     * 1000 = suggested default
     */
  0b00000001,
  /* 
     * PRESUMABLY RESERVED 7:6
     * 00
     *
     * RESERVED 5:4
     * 00
     *
     * VOLUME
     * 1111 = loudest volume
     */
  /* register 0x06 */
  0b00000000,
  /* 
     * RESERVED 15
     * 0
     *
     * OPEN_MODE open reserved registers mode
     * 00 = suggested default
     *
     * Bits 12:8 are not specified, so assume all 0's
     * 00000
     */
  0b00000000,
  /* 
     *  Bits 7:0 are not specified, so assume all 0's
     *  00000000
     */
  /* register 0x07 */
  0b10000010,
  /* 
     *  RESERVED 15 
     * 0
     *
     * TH_SOFRBLEND threshhold for noise soft blend setting
     * 10000 = using default value
     *
     * 65M_50M MODE 
     * 1 = only applies to BAND setting of 0b11, so could probably use 0 here too
     *
     * RESERVED 8
     * 0
     */
  0b00000010,
  /*   
     *  SEEK_TH_OLD seek threshold for old seek mode
     * 000000
     *
     * SOFTBLEND_EN soft blend enable
     * 1 = using default value
     *
     * FREQ_MODE
     * 0 = используем значение по умолчанию
     */
};
/* After reset, we can tune the device
 * We only need program the first 4 bytes in order to do this
 */
uint8_t tune_config[] = {
  /* register 0x02 */
  0b11000000,
  /* 
     * DHIZ audio output high-z disable
     * 1 = normal operation
     *
     * DMUTE mute disable 
     * 1 = normal operation
     *
     * MONO mono select
     * 0 = mono
     *
     * BASS bass boost
     * 0 = disabled
     *
     * RCLK NON-CALIBRATE MODE 
     * 0 = RCLK is always supplied
     *
     * RCLK DIRECT INPUT MODE 
     * 0 = ??? not certain what this does
     *
     * SEEKUP
     * 0 = seek in down direction
     *
     * SEEK
     * 0 = disable / stop seek (i.e. don't seek)
     */
  0b00000101,
  /* 
     * SKMODE seek mode: 
     * 0 = wrap at upper or lower band limit and contiue seeking
     *
     * CLK_MODE clock mode
     * 000 = 32.768kHZ clock rate (match the watch cystal on the module) 
     *
     * RDS_EN radio data system enable
     * 0 = disable radio data system
     *
     * NEW_METHOD use new demodulate method for improved sensitivity
     * 0 = presumably disabled 
     *
     * SOFT_RESET
     * 0 = don't reset this time around
     *
     * ENABLE power up enable: 
     * 1 = enabled
     */
  /* register 0x03 */
  /* Here's where we set the frequency we want to tune to */
  (channel >> 2),
  /* CHAN channel select 8 most significant bits of 10 in total   */
  ((channel & 0b11) << 6) | 0b0000000
  /* 
     *  CHAN two least significant bits of 10 in total 
     *
     * DIRECT MODE used only when test
     * 0 = presumably disabled
     *
     * TUNE commence tune operation 
     * 1 = enable (i.e. tune to selected channel)
     *
     * BAND band select
     * 00 = select the 87-108MHz band
     *
     * SPACE channel spacing
     * 00 = select spacing of 100kHz between channels
     */
};
void setup() {
  Serial.begin(115200);
  tft.setSPISpeed(40000000);
  tft.initR(INITR_MINI160x80);
  tft.setRotation(3);
  tft.invertDisplay(true);

  tft.fillScreen(ST77XX_BLACK);
  // tft.fillRoundRect(0, 0, 170, 90, 0, ST77XX_BLACK);

  Wire.begin();
  Wire.beginTransmission(RDA5807FP_ADDRESS);
  Wire.write(boot_config, BOOT_CONFIG_LEN);
  Wire.endTransmission();
  Wire.beginTransmission(RDA5807FP_ADDRESS);
  Wire.write(tune_config, TUNE_CONFIG_LEN);
  Wire.endTransmission();
}  //setup end
int channel1 = 90;
int Freq = 5000;
String cmd = "";

void waiting_4_command() {
  int Freq_VAL, Freq_VALhigest, Freq_VALH, Freq_VALL, Freq_VALlowest;
  cmd = "";
  if (Serial.available()) {
    //    cmd = Serial.readStringUntil('\n');
    cmd = Serial.readString();
    cmd.trim();
  }


  if (cmd.substring(0, 1) == "F") {
    Freq_VALhigest = cmd[1] - '0';
    Freq_VALH = cmd[2] - '0';
    Freq_VALL = cmd[3] - '0';
    Freq_VALlowest = cmd[4] - '0';
    Freq_VAL = (Freq_VALhigest * 1000) + (Freq_VALH * 100) + (Freq_VALL * 10) + (Freq_VALlowest * 1);
    Freq = Freq_VAL;
    float FreqFloat = (float)Freq;
    float channelFloat = ((FreqFloat / 10.0) - 76) / 0.025;
    Serial.println("Freq has been changed");
    Serial.println(channelFloat);
    int channelInt = (int)channelFloat;
    Serial.println(channelInt);
    myChangeChannel(channelInt);
    Serial.println(String(FreqFloat/10.0));
    tft.fillRoundRect(0, 0, 100, 40, 0, ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(5, 5);
    tft.setTextSize(3);
    tft.print(String(FreqFloat/10.0));
  }
}

void loop() {
  waiting_4_command();

  delay(20);
}  //конец цикла loop
/*
 * функция для изменения канала в модуле RDA5807
 * Example: channel = 191 
 */
void myChangeChannel(int channel) { /* void if nothing is returned else int */
                                    /*
   * первым делом записываем новый канал в массив tune_config, потом передаем его значение модулю RDA5807
   */
  tune_config[2] = (channel >> 2);
  tune_config[3] = ((channel & 0b11) << 6) | 0b00011011;
  Wire.begin();
  Wire.beginTransmission(RDA5807FP_ADDRESS);
  Wire.write(tune_config, TUNE_CONFIG_LEN);
  Wire.endTransmission();
}