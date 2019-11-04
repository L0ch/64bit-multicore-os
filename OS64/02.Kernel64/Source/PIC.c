#include "PIC.h"

void kInitializePIC(void){

	// Initialize Master PIC
	// ICW1(0x20), IC4 bit = 1
	kOutPortByte(PIC_MASTER_PORT1, 0x11);

	// ICW2(0x21), Interrupt vector(0x20)
	kOutPortByte(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR);

	// ICW3(0x21), slave PIC connect pin
	// pin 2 -> 0x04 (bit 2)
	kOutPortByte(PIC_MASTER_PORT2, 0x04);

	// ICW4(0x21). uPM (bit 0) = 1
	kOutPortByte(PIC_MASTER_PORT2, 0x01);

	// Initialize Slave PIC
	// ICW1(0xA0), IC4 (bit 0) = 1
	kOutPortByte(PIC_SLAVE_PORT1, 0x11);

	// ICW2(0xA1), Interrupt vector(0x20 + 8)
	kOutPortByte(PIC_SLAVE_PORT2, PIC_IRQSTARTVECTOR +8);

	// ICW3(0xA1), Master PIC connect pin
	// pin 2 -> 0x02 (integer)
	kOutPortByte(PIC_SLAVE_PORT2, 0x02);

	// ICW4(0xA1), uPM (bit 0) = 1
	kOutPortByte(PIC_SLAVE_PORT2, 0x01);

}

// Set IMR
void kMaskPICInterrupt(WORD wIRQBitmask){
	// Master PIC
	// OCW1(0x21), IRQ 0~7
	kOutPortByte(PIC_MASTER_PORT2, (BYTE)wIRQBitmask);

	// Slave PIC
	// OCW1(0xA1), IRQ 8~15
	kOutPortByte(PIC_SLAVE_PORT2, (BYTE) (wIRQBitmask >> 8));
}

// Send Interrupt processing is complete (EOI)
// If Master PIC, Send EOI to Master PIC
// If Slave PIC, Send EOI to both of PIC
void kSendEOIToPIC(int iIRQNumber){
	// Send EOI to Master PIC
	// OCW2(0x20), EOI(bit 5) = 1
	kOutPortByte(PIC_MASTER_PORT1, 0x20);

	// If Slave PIC interrupt
	if(iIRQNumber >= 8){
		//OCW2(0xA0), EOI
		kOutPortByte(PIC_SLAVE_PORT1, 0x20);
	}
}
