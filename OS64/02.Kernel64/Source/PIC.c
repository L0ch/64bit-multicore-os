#include "PIC.h"
#include "AssemblyUtility.h"

void InitializePIC(void){

	// Initialize Master PIC
	// ICW1(0x20), IC4 bit = 1
	OutPortByte(PIC_MASTER_PORT1, 0x11);

	// ICW2(0x21), Interrupt vector(0x20)
	OutPortByte(PIC_MASTER_PORT2, PIC_IRQSTARTVECTOR);

	// ICW3(0x21), slave PIC connect pin
	// pin 2 -> 0x04 (bit 2)
	OutPortByte(PIC_MASTER_PORT2, 0x04);

	// ICW4(0x21). uPM (bit 0) = 1
	OutPortByte(PIC_MASTER_PORT2, 0x01);

	// Initialize Slave PIC
	// ICW1(0xA0), IC4 (bit 0) = 1
	OutPortByte(PIC_SLAVE_PORT1, 0x11);

	// ICW2(0xA1), Interrupt vector(0x20 + 8)
	OutPortByte(PIC_SLAVE_PORT2, PIC_IRQSTARTVECTOR +8);

	// ICW3(0xA1), Master PIC connect pin
	// pin 2 -> 0x02 (integer)
	OutPortByte(PIC_SLAVE_PORT2, 0x02);

	// ICW4(0xA1), uPM (bit 0) = 1
	OutPortByte(PIC_SLAVE_PORT2, 0x01);

}

// Set IMR
void MaskPICInterrupt(WORD wIRQBitmask){
	// Master PIC
	// OCW1(0x21), IRQ 0~7
	OutPortByte(PIC_MASTER_PORT2, (BYTE)wIRQBitmask);

	// Slave PIC
	// OCW1(0xA1), IRQ 8~15
	OutPortByte(PIC_SLAVE_PORT2, (BYTE) (wIRQBitmask >> 8));
}

// Send Interrupt processing is complete (EOI)
// If Master PIC, Send EOI to Master PIC
// If Slave PIC, Send EOI to both of PIC
void SendEOIToPIC(int iIRQNumber){
	// Send EOI to Master PIC
	// OCW2(0x20), EOI(bit 5) = 1
	OutPortByte(PIC_MASTER_PORT1, 0x20);

	// If Slave PIC interrupt
	if(iIRQNumber >= 8){
		//OCW2(0xA0), EOI
		OutPortByte(PIC_SLAVE_PORT1, 0x20);
	}
}
