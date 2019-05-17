#include"i2c_setup.h"

// I2C Master utilities, 100 kHz, using polling rather than interrupts
// The functions must be callled in the correct order as per the I2C protocol
// Change I2C1 to the I2C channel you are using
// I2C pins need pull-up resistors, 2k-10k

void i2c_master_setup(void) {
  I2C2BRG = 53;            // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2 
                                    // look up PGD for your PIC32
  I2C2CONbits.ON = 1;               // turn on the I2C1 module
}

// Start a transmission on the I2C bus
void i2c_master_start(void) {
    I2C2CONbits.SEN = 1;            // send the start bit
    while(I2C2CONbits.SEN) { ; }    // wait for the start bit to be sent
}

void i2c_master_restart(void) {     
    I2C2CONbits.RSEN = 1;           // send a restart 
    while(I2C2CONbits.RSEN) { ; }   // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
  I2C2TRN = byte;                   // if an address, bit 0 = 0 for write, 1 for read
  while(I2C2STATbits.TRSTAT) { ; }  // wait for the transmission to finish
  if(I2C2STATbits.ACKSTAT) {        // if this is high, slave has not acknowledged
    // ("I2C2 Master: failed to receive ACK\r\n");
  }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C2CONbits.RCEN = 1;             // start receiving data
    while(!I2C2STATbits.RBF) { ; }    // wait to receive the data
    return I2C2RCV;                   // read and return the data
}

void i2c_master_ack(int val) {        // sends ACK = 0 (slave should send another byte)
                                      // or NACK = 1 (no more bytes requested from slave)
    I2C2CONbits.ACKDT = val;          // store ACK/NACK in ACKDT
    I2C2CONbits.ACKEN = 1;            // send ACKDT
    while(I2C2CONbits.ACKEN) { ; }    // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) {          // send a STOP:
  I2C2CONbits.PEN = 1;                // comm is complete and master relinquishes bus
  while(I2C2CONbits.PEN) { ; }        // wait for STOP to complete
}

void initExpander(){
    i2c_master_setup();
    i2c_master_start(); // make the start bit
    i2c_master_send(0b01000000); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(0x00); // the register to write to
    i2c_master_send(0xF0); // the value to put in the register
    i2c_master_stop(); // make the stop bit
    
    i2c_master_start(); // make the start bit
    i2c_master_send(0b01000000); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(0x06); // the register to write to
    i2c_master_send(0xF0); // the value to put in the register
    i2c_master_stop(); // make the stop bit
    
    i2c_master_start(); // make the start bit
    i2c_master_send(0b01000000); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(0x0A); // the register to write to
    i2c_master_send(0x01); // the value to put in the register
    i2c_master_stop(); // make the stop bit
}

void setExpander(char pin, char level){
    i2c_master_start(); // make the start bit
    i2c_master_send(0b01000000); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(0x0A); // the register to write to
    i2c_master_send((0b1<<pin) & level); // the value to put in the register
    i2c_master_stop(); // make the stop bit
}

char getExpander(){
    i2c_master_start(); // make the start bit
    i2c_master_send(0b01000000); // write the address, shifted left by 1, or'ed with a 0 to indicate writin
    i2c_master_send(0x09); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(0b01000001); // write the address, shifted left by 1, or'ed with a 1 to indicate reading
    char r = i2c_master_recv(); // save the value returned
    i2c_master_ack(1); // make the ack so the slave knows we got it
    i2c_master_stop(); // make the stop bit
    return r;
}


void initIMU(){
    i2c_master_setup();
    i2c_master_start(); // make the start bit
    i2c_master_send(0b11010110); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(0x10); // the register to write to
    i2c_master_send(0b10000010); // the value to put in the register
    i2c_master_stop(); // make the stop bit
    
    i2c_master_start(); // make the start bit
    i2c_master_send(0b11010110); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(0x011); // the register to write to
    i2c_master_send(0b10001000); // the value to put in the register
    i2c_master_stop(); // make the stop bit
    
    i2c_master_start(); // make the start bit
    i2c_master_send(0b11010110); // write the address, shifted left by 1, or'ed with a 0 to indicate writin
    i2c_master_send(0x012); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(0b11010111); // write the address, shifted left by 1, or'ed with a 1 to indicate reading
    char r = i2c_master_recv(); // save the value returned
    i2c_master_ack(1); // make the ack so the slave knows we got it
    i2c_master_stop(); // make the stop bit
    
    i2c_master_start(); // make the start bit
    i2c_master_send(0b11010110); // write the address, shifted left by 1, or'ed with a 0 to indicate writing
    i2c_master_send(0x12); // the register to write to
    i2c_master_send((r|(0b00000100))); // the value to put in the register
    i2c_master_stop(); // make the stop bit
}


char getIMU(){
    i2c_master_start(); // make the start bit
    i2c_master_send(0b11010110); // write the address, shifted left by 1, or'ed with a 0 to indicate writin
    i2c_master_send(0x0F); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(0b11010111); // write the address, shifted left by 1, or'ed with a 1 to indicate reading
    char r = i2c_master_recv(); // save the value returned
    i2c_master_ack(1); // make the ack so the slave knows we got it
    i2c_master_stop(); // make the stop bit
    return r;
}

void I2C_read_multiple(unsigned char address, unsigned char register1, unsigned char * data, int length){
    i2c_master_start(); // make the start bit
    i2c_master_send(address<<1); // write the address, shifted left by 1, or'ed with a 0 to indicate writin
    i2c_master_send(register1); // the register to read from
    i2c_master_restart(); // make the restart bit
    i2c_master_send(address<<1|1); // write the address, shifted left by 1, or'ed with a 1 to indicate reading
    int i = 0;
    for(i = 0; i<length; i++){
        data[i] = i2c_master_recv(); // save the value returned
        if(i<length-1){
            i2c_master_ack(0);
        }
        else{
            i2c_master_ack(1);
        }
    } // make the ack so the slave knows we got it
    i2c_master_stop(); // make the stop bit
}