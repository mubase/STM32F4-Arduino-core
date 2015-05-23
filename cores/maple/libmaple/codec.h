
// I2S and I2C setup for the STM32F4 discovery board.
// For use with Roger Clark's STM32 Arduino Project.
// S.Scutt - 2015.   


#include <HardWire.h>
#include <Wire.h>
#include <WireBase.h>

void codec_reg_setup();
uint16 codec_send_data(uint16 data);
void codec_16_bit_setup()
{

rcc_clk_enable(RCC_SPI3); //Turn on SPI3 (I2S clock)
//i2c pin modes
gpio_set_mode(GPIOB,6,GPIO_AF_OUTPUT_OD);  // i2c clk
gpio_set_mode(GPIOB,9,GPIO_AF_OUTPUT_OD);  // i2c data
// i2c1 remap to port B
gpio_set_af_mode(GPIOB,6,4);   //I2C clock (SCL) remap to codec pins
gpio_set_af_mode(GPIOB,9,4);   //I2C data (SDA)  remap to codec pins

i2s_pin_setup();  // Sets up pins for I2S SCK, MCLK, SD and LRCK
plli2son();  // I2S PLL clock on (defined in rccF2.c)
SPI3_BASE->I2SPR |= (0x204);   // configure I2S frequency for 48KHz with MCLK ON
 i2s_peripheral_16_bit_master(SPI3); // set up I2S for 16 bit, phillips, master TX, CPOL low.
i2s_init(SPI3); // set i2s enable bit

codec_reg_setup(); // I2C commands to set up the codec as slave
}

// I2C Codec setup functions. Code adapted from http://www.openmusiclabs.com/projects/codec-shield/

#define ADDR 0x94 // i2c address and write bit
#define SDA_PIN 25 // i2c data line
#define SCL_PIN 22 // i2c clock line
#define LED_PIN 60 // board led pin

void mydelay(unsigned char t) {
  while((t << 4) > 0) {
    togglePin(LED_PIN);
    t--;
  }
}

// i2c start condition
char i2cbb_start(void) {
  if (digitalRead(SDA_PIN) == 0) { // check if data line released
    return -1; // end with failure if not released
  }
  else if (digitalRead(SCL_PIN) == 0) { // check if clock line released
    return -2; // end with failure if not released
  }
  else { // send start condition
    digitalWrite(SDA_PIN, LOW);  // data low
    mydelay(10); // delay
    digitalWrite(SCL_PIN, LOW); // clock low
    mydelay(10); // delay
    return 1; // set state to success
  }
}


// i2c stop condition
void i2cbb_stop(void) {
  digitalWrite(SDA_PIN, LOW); // pull data low
  mydelay(10); // delay
  digitalWrite(SCL_PIN, HIGH); // release clock line
  mydelay(10); // delay
  digitalWrite(SDA_PIN, HIGH); // release data line
  mydelay(40); // delay to make sure a new data transfer doesnt occur too quickly
}


// i2c data send
char i2cbb_send(unsigned char data) {  // clock out data
  unsigned char state = 0; // initialize return state
  unsigned char i;
  for(i = 8 ; i > 0  ; i--) {
    digitalWrite(SDA_PIN, (data & (1 << (i - 1))));
    mydelay(10);
    digitalWrite(SCL_PIN, HIGH);
    mydelay(10);
    digitalWrite(SCL_PIN, LOW);
  }
  // check for ack
  digitalWrite(SDA_PIN, HIGH); // release line
  mydelay(10); // wait a bit
  unsigned char d = 30; // initialize timeout
  while(digitalRead(SDA_PIN) == 1){ // wait for ack
    // timeout in case of nack
    togglePin(LED_PIN);
    d--;
    if (d == 0) {
      state = 2; // set i2c state to nack
      break;
    }
  }
  // clock the ack or nack
  digitalWrite(SCL_PIN, HIGH);
  mydelay(10);
  digitalWrite(SCL_PIN, LOW);
  // make sure line is released
  d = 30;
  while(digitalRead(SDA_PIN) == 0){
    // timeout in case of failure
    togglePin(LED_PIN);
    d--;
    if (d == 0) {
      state = 3; // set i2c state to no line release
      break;
    }
  }
  if (state > 1) { // send stop if failure
    i2cbb_stop();
  }
  else { // set state to success
    state = 1;
  }
  return state;
}

// full i2c protocol for 3 byte transfer
unsigned char i2cbb(unsigned char reg, unsigned char data) {
  if (i2cbb_start() != 1) { // send start condition
    return 2;
  }
  else if (i2cbb_send(ADDR) != 1) { // send address and write bit
    return 3;
  }
  else if (i2cbb_send(reg) != 1) { // send register to write to
    return 4;
  }
  else if (i2cbb_send(data) != 1) { // write data to register
    return 5;
  }
  else {
  i2cbb_stop(); // send stop condition
  return 1;
  }
}

// each register retries until success
// if communication fails the device will hang
void codec_reg_setup(void) {
  Wire.begin();
  gpio_write_bit(GPIOD,4,1);
  
  while (i2cbb(0x02, 0x01) != 1) { // power save registers -> all on
    delay(10);
  }
  
   while (i2cbb(0x00, 0x99) != 1) { // 
    delay(10);
  }
  
  while (i2cbb(0x47, 0x80) != 1) { // inits
    delay(10);
  }
  
  while (i2cbb(0x0d, 0x03) != 1) { // playback ctrl
    delay(10);
  }
 
  
  while (i2cbb(0x32, (1<<7)) != 1) { // vol
    delay(10);
  }
  while (i2cbb(0x32, (0<<7)) != 1) { // vol
    delay(10);
  }
  while (i2cbb(0x00, 0x00) != 1) { // inits
    delay(10);
  }
  while (i2cbb(0x04, 0xaf) != 1) { // power ctl
    delay(10);
  }
  
  while (i2cbb(0x0d, 0x70) != 1) { 
    delay(10);
  }
  
  while (i2cbb(0x05, 0x81) != 1) { // clocking
    delay(10);
  }
 
  while (i2cbb(0x06, 0x07) != 1) { 
    delay(10);
  }
  
  while (i2cbb(0x0a, 0x00) != 1) { 
    delay(10);
  }
  
  while (i2cbb(0x27, 0x00) != 1) { 
    delay(10);
  }
  
  while (i2cbb(0x1a| 0x80, 0x0a) != 1) { // both channels on
    delay(10);
  }
  

  while (i2cbb(0x1f, 0x0f) != 1) { 
    delay(10);
  }
  while (i2cbb(0x02, 0x9e) != 1) { 
    delay(10);
  }
     
}

uint16 codec_send_data(uint16 data)
{
 while (!(SPI3_BASE->SR&SPI_SR_TXE));
SPI3_BASE->DR=(data); 
}