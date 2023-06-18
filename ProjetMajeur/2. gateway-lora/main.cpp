/**
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>


#include "mbed.h"
#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"

#include "platform\ATCmdParser.h"
 

// Application helpers
#include "trace_helper.h"
#include "lora_radio_helper.h"
#include "CayenneLPP.h"

using namespace events;

// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
#define TX_TIMER                        500

/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS            10

/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER     3

/**
 * Dummy pin for dummy sensor
 */
#define PC_9                            0


#define size             32

#define TARGET_TX_PIN                   PA_9
#define TARGET_RX_PIN                   PA_10

#define   ESP8266_DEFAULT_BAUD_RATE   115200


/**
* This event queue is the global event queue for both the
* application and stack. To conserve memory, the stack is designed to run
* in the same thread as the application and the application is responsible for
* providing an event queue to the stack that will be used for ISR deferment as
* well as application information event queuing.
*/
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS *EVENTS_EVENT_SIZE);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */

static unsigned int my_sleep(unsigned int seconds);

static void lora_event_handler(lorawan_event_t event);

static DigitalOut led(LED1);
/**
 * Constructing Mbed LoRaWANInterface and passing it the radio object from lora_radio_helper.
 */
static LoRaWANInterface lorawan(radio);
/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;


BufferedSerial *_serial;
ATCmdParser *_parser;

BufferedSerial pc(USBTX, USBRX, 115200);


int ResetAT()
{
    //Now get the FW version number of ESP8266 by sending an AT command
    pc.write("\nATCmdParser: Begin Reset",size-8);
    _parser->send("AT+RST");
    if (_parser->recv("OK")) {
        pc.write("\nATCmdParser: Reset successful",size);
        
    } else {
        pc.write("\nATCmdParser: Reset Failed",size);
        return -1;
    }

    return 0;
}

int selectModeAT()
{
    //Now get the FW version number of ESP8266 by sending an AT command
    pc.write("\nATCmdParser: Begin Mode Selection : AP",size+8);
    _parser->send("AT+CWMODE=2");
    int version;
    if (_parser->recv("ready")) {
        pc.write("\nATCmdParser: AP init successful",size);
    } else if (_parser->recv("AT(Timeout)")){
        _parser->send("AT+CWMODE=2");
        pc.write("\nATCmdParser: Timeout Retriwing",size);
    }
    else{
        pc.write("\nATCmdParser: AP init Failed",size);
        return -1;
    }

    return 0;
}


int createSSID()
{
    //Now get the FW version number of ESP8266 by sending an AT command
    pc.write("\nATCmdParser: Begin SSID Config",size+8);
    _parser->send("AT+CWSAP=\"WifiGRP1\",\"soleil01\",1,3\r\n");
    if (_parser->recv("OK")) {
        pc.write("\nATCmdParser: SSID Created Succefully",size);
    } else if (_parser->recv("AT(Timeout)")){
        _parser->send("AT+CWSAP=\"WifiGRP1\",\"soleil01\",1,3\r\n");
        pc.write("\nATCmdParser: Timeout Retriwing",size);
    }
    else{
        pc.write("\nATCmdParser: SSID Creation failed",size);
        return -1;
    }
    return 0;
}

int createTCPServer()
{
    
    //Now get the FW version number of ESP8266 by sending an AT command
    pc.write("\nATCmdParser: Begin Server Config",size+8);

    _parser->send("AT+CIPMUX=1\r\n");

    if (_parser->recv("OK")) {
            pc.write("\nATCmdParser: Multiple selection OK",size);
            _parser->send("AT+CIPSERVER=1\r\n");
        if (_parser->recv("OK")) {
            pc.write("\nATCmdParser: Server creation OK",size);
        }
    }
    else{
        pc.write("\nATCmdParser: Server Creation failed",size);
        return -1;
    }
    return 0;
}

int receiveATData(char data[11])
{
    //Now get the FW version number of ESP8266 by sending an AT command
    pc.write("\nATCmdParser: Begin Receiving Data\n",size+4);
    //_parser->send("AT+CIPRECVMODE=1\r\n");

//    if (_parser->recv("OK")) {
        _parser->send("AT+CIPRECVDATA=0,11\r\n");
        

        //my_sleep(1);
        //_parser->flush();
            
        if (_parser->recv("+IPD,0,11:")) {
            
            _parser->read(data,11);
            printf("\nATCmdParser: data received : %s",data);
        }
        else{
            data = "";
            return -1;
        }
//    }
    return 0;
}



/**
 * Entry point for application
 */
int main(void)
{
    _serial = new BufferedSerial(TARGET_TX_PIN, TARGET_RX_PIN, 115200);
    _parser = new ATCmdParser(_serial, "\r\n");

        //_parser->debug_on(true);

    // setup tracing
    setup_trace();

    ResetAT();

    selectModeAT();

    createSSID();

    if(createTCPServer() == -1)
    {
        return -1;
    }

    my_sleep(1);

    // stores the status of a call to LoRaWAN protocol
    lorawan_status_t retcode;

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK) {
        printf("\r\n LoRa initialization failed! \r\n");
        return -1;
    }

    printf("\r\n Mbed LoRaWANStack initialized \r\n");

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER)
            != LORAWAN_STATUS_OK) {
        printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
        return -1;
    }

    printf("\r\n CONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\r\n enable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");

    retcode = lorawan.connect();

    if (retcode == LORAWAN_STATUS_OK ||
            retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
    } else {
        printf("\r\n Connection error, code = %d \r\n", retcode);
        return -1;
    }

    printf("\r\n Connection - In Progress ...\r\n");

    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();

    return 0;

}


/**
 * Sends a message to the Network Server
 */
static void send_message(char data[11])
{
    uint16_t packet_len;
    int16_t retcode;

    printf("\r\n Send Value = %s \r\n", data);


    packet_len = sprintf((char *) tx_buffer, "%s",
                         data);

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                           MSG_CONFIRMED_FLAG);
        
    if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
        : printf("\r\n send() - Error code %d \r\n", retcode);

        if (retcode == LORAWAN_STATUS_WOULD_BLOCK) {
            //retry in 2 seconds
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) 
            {
                ev_queue.call_in(2000, send_message, data);
            }
        }
        return;
    }

    printf("\r\n %d bytes scheduled for transmission \r\n", retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    uint8_t port;
    int flags;
    int16_t retcode = lorawan.receive(rx_buffer, sizeof(rx_buffer), port, flags);

    if (retcode < 0) {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" RX Data on port %u (%d bytes): ", port, retcode);
    for (uint8_t i = 0; i < retcode; i++) {
        printf("%02x ", rx_buffer[i]);
    }
    printf("\r\n");
    
    memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{
    char data[11];

    receiveATData(data);
    printf("\n\nDATA TO SEND : %s", data);

    switch (event) {
        case CONNECTED:
            printf("\r\n Connection - Successful \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message(data);
            } else {
                ev_queue.call_every(TX_TIMER, send_message, data);
            }

            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            printf("\r\n Disconnected Successfully \r\n");
            break;
        case TX_DONE:
            printf("\r\n Message Sent to Network Server \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message(data);
            }
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("\r\n Transmission Error - EventCode = %d \r\n", event);
            // try again
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message(data);
            }
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            receive_message();
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("\r\n Error in reception - Code = %d \r\n", event);
            break;
        case JOIN_FAILURE:
            printf("\r\n OTAA Failed - Check Keys \r\n");
            break;
        case UPLINK_REQUIRED:
            printf("\r\n Uplink required by NS \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message(data);
            }
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}


unsigned int my_sleep(unsigned int seconds) {
   HAL_Delay(seconds * 1000);
   return 0;
}

// EOF
