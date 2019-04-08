#include "PmodWIFI.h"
#include "xil_cache.h"
#include <xparameters.h> // for addresses of memory-mapped devices

#ifdef __MICROBLAZE__
#define PMODWIFI_VEC_ID XPAR_INTC_0_PMODWIFI_0_VEC_ID
#else
#define PMODWIFI_VEC_ID XPAR_FABRIC_PMODWIFI_0_WF_INTERRUPT_INTR
#endif

/************************************************************************/
/*                                                                      */
/*              SET THESE VALUES FOR YOUR NETWORK                       */
/*                                                                      */
/************************************************************************/

// Internet assigned Static IP
//IPv4 ipServer = {192,168,1,190}; // {0,0,0,0} for DHCP
IPv4 ipServer = {0,0,0,0}; // for DHCP

unsigned short portServer = 44400;

// memory-mapped addresses for the two servo controllers and the Pmod camera
unsigned int* SERVO_CONTROLLER_0 = (unsigned int*) XPAR_SERVO_CONTROLLER_0_SERVO_AXI_BASEADDR;
unsigned int* SERVO_CONTROLLER_1 = (unsigned int*) XPAR_SERVO_CONTROLLER_1_SERVO_AXI_BASEADDR;

short int* frame_buffer_base_addr = (short int*) XPAR_PMOD_CAMERA_0_S00_AXI_BASEADDR;

// Specify the SSID
const char *szSsid = "YOUR_SSID"

// Select 1 for the security you want, or none for no security
#define USE_WPA2_PASSPHRASE
//#define USE_WPA2_KEY
//#define USE_WEP40
//#define USE_WEP104
//#define USE_WF_CONFIG_H

// modify the security key to what you have.
#if defined(USE_WPA2_PASSPHRASE)

   const char *szPassPhrase = "YOUR_PASSPHRASE";
   #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, szPassPhrase, &status)

#elif defined(USE_WPA2_KEY)

   WPA2KEY key = { 0x27, 0x2C, 0x89, 0xCC, 0xE9, 0x56, 0x31, 0x1E,
                   0x3B, 0xAD, 0x79, 0xF7, 0x1D, 0xC4, 0xB9, 0x05,
                   0x7A, 0x34, 0x4C, 0x3E, 0xB5, 0xFA, 0x38, 0xC2,
                   0x0F, 0x0A, 0xB0, 0x90, 0xDC, 0x62, 0xAD, 0x58 };
   #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, key, &status)

#elif defined(USE_WEP40)

   const int iWEPKey = 0;
   WEP40KEY keySet = { 0xBE, 0xC9, 0x58, 0x06, 0x97,   // Key 0
                       0x00, 0x00, 0x00, 0x00, 0x00,   // Key 1
                       0x00, 0x00, 0x00, 0x00, 0x00,   // Key 2
                       0x00, 0x00, 0x00, 0x00, 0x00 }; // Key 3
   #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, keySet, iWEPKey, &status)

#elif defined(USE_WEP104)

   const int iWEPKey = 0;
   WEP104KEY keySet = { 0x3E, 0xCD, 0x30, 0xB2, 0x55, 0x2D, 0x3C, 0x50, 0x52, 0x71, 0xE8, 0x83, 0x91,   // Key 0
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 1
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Key 2
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // Key 3
   #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, keySet, iWEPKey, &status)

#elif defined(USE_WF_CONFIG_H)

   #define WiFiConnectMacro() deIPcK.wfConnect(0, &status)

#else // No security - OPEN

   #define WiFiConnectMacro() deIPcK.wfConnect(szSsid, &status)

#endif

//******************************************************************************************
//******************************************************************************************
//***************************** END OF CONFIGURATION ***************************************
//******************************************************************************************
//******************************************************************************************

typedef enum {
   NONE = 0,
   CONNECT,
   LISTEN,
   ISLISTENING,
   AVAILABLECLIENT,
   ACCEPTCLIENT,
   READ,
   STORE,
   WRITE,
   CLOSE,
   EXIT,
   DONE
} STATE;

STATE state = CONNECT;

unsigned tStart = 0;
unsigned tWait = 5000;

// Remember to give the UDP client a datagram cache
#define cUDPSockets 3
static UDPSocket rgUDPClient[cUDPSockets];
static UDPSocket *  pUdpSocket = NULL;
static UDPServer udpServer;

IPSTATUS status;

// A read buffer
byte rgbRead[1024];
int cbRead = 0;
int count = 0;
int i = 0;

// for parsing the 32-bit servo register value out of the byte array
unsigned int servo_val;

// image to write
#define IMAGE_SIZE 57600 // 160 x 120 pixels times 3 colors
#define IMAGE_SLICE_SIZE 960 // dividing the image into 60 datagrams (2 rows at a time, 3 colors per row)
#define NUM_SLICES 60

byte frameBuffer[IMAGE_SIZE];
byte toWrite[IMAGE_SLICE_SIZE];

long int write_status;
int cur_slice = 0;

int baseIdx;
int baseIdxFrame;

int tmp_coord;
int x_coord;
int y_coord;

// temporary variables for decoding
short int tmp_pixel;
byte red, green, blue;

// set to true to read from the frame buffer for the response, else send a checkerboard pattern
bool camera_frame = false;

void DemoInitialize();
void DemoRun();


int main(void) {
   Xil_ICacheEnable();
   Xil_DCacheEnable();

   xil_printf("camera_wifi\r\nConnecting to network...\r\n");
   DemoInitialize();
   DemoRun();
   return 0;
}

void DemoInitialize() {
   setPmodWifiAddresses(
      XPAR_PMODWIFI_0_AXI_LITE_SPI_BASEADDR,
      XPAR_PMODWIFI_0_AXI_LITE_WFGPIO_BASEADDR,
      XPAR_PMODWIFI_0_AXI_LITE_WFCS_BASEADDR,
      XPAR_PMODWIFI_0_S_AXI_TIMER_BASEADDR
   );
   setPmodWifiIntVector(PMODWIFI_VEC_ID);
}

void DemoRun() {
   while (1) {
      switch (state) {
      case CONNECT:
         if (WiFiConnectMacro()) {
            xil_printf("Connection Created\r\n");
            deIPcK.begin(ipServer);
            state = LISTEN;
         } else if (IsIPStatusAnError(status)) {
            xil_printf("Unable to connection, status: %d\r\n", status);
            state = CLOSE;
         }
         break;

      // Say to listen on the port
      case LISTEN:
         if (deIPcK.udpStartListening(portServer, udpServer, &status)) {
            // Add our sockets
            for (i = 0; i < cUDPSockets; i++) {
               udpServer.addSocket(rgUDPClient[i]);
            }
            xil_printf("Started Listening\r\n");
            state = ISLISTENING;
         } else if (IsIPStatusAnError(status)) {
            xil_printf("Listening Error: 0x%X\r\n", status);
            state = EXIT;
         }
         break;

      // Not specifically needed, we could go right to AVAILABLECLIENT
      // but this is a nice way to print to the serial monitor that we are
      // actively listening.
      // Remember, this can have non-fatal failures, so check the status
      case ISLISTENING:
         if ((i = udpServer.isListening(&status)) > 0) {
            deIPcK.getMyIP(ipServer);
            xil_printf("Server started on %d.%d.%d.%d:%d\r\n", ipServer.u8[0],
                  ipServer.u8[1], ipServer.u8[2], ipServer.u8[3], portServer);
            xil_printf("%d Sockets listening on port: %d\r\n", i, portServer);
            state = AVAILABLECLIENT;
         } else if (IsIPStatusAnError(status)) {
            xil_printf("Lost IP connectivity, error: 0x%X\r\n", status);
            state = EXIT;
         }
         break;

      // Wait for a connection
      case AVAILABLECLIENT:
         if ((count = udpServer.availableClients()) > 0) {
            xil_printf("Got %d clients pending\r\n", count);
            state = ACCEPTCLIENT;
         }
         break;

      // Accept the connection
      case ACCEPTCLIENT:
         // Accept the client
         if ((pUdpSocket = udpServer.acceptClient()) != NULL) {
            xil_printf("Got a Connection\r\n");
            state = READ;
            tStart = (unsigned) SYSGetMilliSecond();
         }

         // This probably won't happen unless the connection is dropped
         // If it is, just release our socket and go back to listening
         else {
            state = CLOSE;
         }
         break;

      // Wait for the read, but if too much time elapses (5 seconds)
      // we will just close the udpClient and go back to listening
      case READ:
         // See if we got anything to read
         if ((cbRead = pUdpSocket->available()) > 0) {
            cbRead = cbRead < (int) sizeof(rgbRead) ? cbRead : sizeof(rgbRead);
            cbRead = pUdpSocket->readDatagram(rgbRead, cbRead);

            // parse command
            if (rgbRead[0] == 0) { // request frame
                //cur_slice = rgbRead[1]; // second frame is the requested slice number
            	// for burst transmission
            	cur_slice = 0;
            	camera_frame = true;
            	state = STORE;
            }
            else if (rgbRead[0] > 0 && rgbRead[0] <= 9) { // writing to a servo
            	// parse the value to write to the servo
            	servo_val = 0;
			    for (int j=1; j<5; j++) {
				   servo_val = servo_val << 8;
				   servo_val += (unsigned int) rgbRead[j];
			    }
			    xil_printf("Writing value %d to servo %d\r\n", servo_val, rgbRead[0]);

            	if (rgbRead[0] > 0 && rgbRead[0] <= 4) { // write to servo bank 0
				   *(SERVO_CONTROLLER_0 + 4*(rgbRead[0] - 1)) = servo_val;
				}
				else if (rgbRead[0] > 4 && rgbRead[0] <= 8) { // write to servo bank 1
				   *(SERVO_CONTROLLER_1 + 4*(rgbRead[0] - 5)) = servo_val;
				}
            	state = READ;
            }
            else if (rgbRead[0] == 0xA) { // writing target coordinates
            	// coordinates are located at the end of the address space
            	tmp_coord = *(frame_buffer_base_addr + 76800);
            	x_coord = tmp_coord && 0xFFFF;
            	y_coord = (tmp_coord >> 16) && 0xFFFF;

            	xil_printf("Sending coordinates (%d,%d) to client\r\n", x_coord, y_coord);
            	toWrite[0] = x_coord >> 7; // most significant half of x coord
            	toWrite[1] = x_coord & 0x7F; // least significant half of x coord
            	toWrite[2] = y_coord >> 7; // most significant half of x coord
				toWrite[3] = y_coord & 0x7F; // least significant half of x coord

				write_status = 0;
				while (!write_status) {
					 write_status = pUdpSocket->writeDatagram(toWrite, 4);
					 if (!write_status) {
						 xil_printf("Write status for sending coordinates = %d\n", write_status);
					 }
				}
				state = READ;
            }
            else if (rgbRead[0] == 0xF) { // writing checkerboard pattern
            	cur_slice = 0;
            	camera_frame = false;
            	state = WRITE;
            }
            else {
               xil_printf("Invalid command:\r\n%s\r\n", rgbRead);
               state = READ;
            }
         }

         // If too much time elapsed between reads, close the connection
         else if ((((unsigned) SYSGetMilliSecond()) - tStart) > tWait) {
            state = CLOSE;
         }
         break;

      case STORE:
    	  for (int i=0; i<120; i++) {
    		  for (int j=0; j<160; j++) {
    			    // read from camera
					tmp_pixel = *(frame_buffer_base_addr + (320*i + j));
					// mask out the colors
					blue = tmp_pixel & 0xF;
					green = (tmp_pixel >> 4) & 0xF;
					red = (tmp_pixel >> 8) & 0xF;
					frameBuffer[3*(160*i + j)] = red;
					frameBuffer[3*(160*i + j) + 1] = green;
					frameBuffer[3*(160*i + j) + 2] = blue;
    		  }
    	  }
    	  state = WRITE;
    	  break;

      // Write the frame slice
      case WRITE:
    	 if (camera_frame) { // send frame from camera
    		 for (int i=0; i<2; i++) {
				for (int j=0; j<160; j++) {
					baseIdx = 3*(160*i + j);
					baseIdxFrame = 3*(160*(i + 2*cur_slice) + j);
					toWrite[baseIdx] = frameBuffer[baseIdxFrame];
					toWrite[baseIdx + 1] = frameBuffer[baseIdxFrame + 1];
					toWrite[baseIdx + 2] = frameBuffer[baseIdxFrame + 2];
				}
			 }
    	 }
    	 else { // otherwise, send checkerboard for testing purposes
			 for (int i=0; i<6; i++) {
				for (int j=0; j<160; j++) {
					if ((i % 2 == 0) ^ (j % 2 == 1)) {
					   toWrite[160*i + j] = 5*(cur_slice+1); // dark gray
					}
					else {
					   toWrite[160*i + j] = 0; // light gray
					}
				}
			 }
    	 }

		 write_status = 0;
		 while (!write_status) {
			 write_status = pUdpSocket->writeDatagram(toWrite, IMAGE_SLICE_SIZE);
			 if (!write_status) {
				 xil_printf("Write status (slice %d/%d) = %d\n", cur_slice+1, NUM_SLICES, write_status);
			 }
		 }

		 cur_slice++;

		 // if there are slices left to write, return to this state, else go back to reading
		 if (cur_slice < NUM_SLICES) {
			 state = WRITE;
		 }
		 else {
			 state = READ;
		 }

		 tStart = (unsigned) SYSGetMilliSecond();
         break;

      // Close our udpClient and go back to listening
      case CLOSE:
         if (pUdpSocket != NULL) {
            pUdpSocket->close();
            udpServer.addSocket(*pUdpSocket);
         }
         xil_printf("Closing UdpClient and re-adding it to the server\r\n");
         state = ISLISTENING;
         break;

      // Something bad happen, just exit out of the program
      case EXIT:
         udpServer.close();
         for (i = 0; i < cUDPSockets; i++) {
            rgUDPClient[i].close();
         }
         xil_printf("Something went wrong, sketch is done.\r\n");
         state = DONE;
         break;

      // Do nothing in the loop
      case DONE:

      default:
         break;
      }

      // Every pass through loop(), keep the stack alive
      DEIPcK::periodicTasks();
   }
}
