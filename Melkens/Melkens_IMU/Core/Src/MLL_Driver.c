/*
 * MLL_Driver.c
 *
 *MLL (Melkens Link Lite) Driver
 *MLL V1.1
 *
 *  Created on: Sep 19, 2022
 *      Author: Jacek Dobija
 *
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "IMU_func.h"

const unsigned char MY_ADDRESS = 0x08; //Change this for your custom peripheral address
unsigned char sof1;
unsigned char sof2;
unsigned char dest_addr;
unsigned char src_addr;
unsigned char frame_size;
unsigned char parity1;
unsigned char eof1;
unsigned counter=0;
unsigned char *ptr;

enum state_codes {    wait, get_sof1, check_get_sof2, check_get_dest, check_get_src, get_size, setup_count, get_byte_dec, get_parity, get_eof, check_parity, write_data, discard_data } current_state, next_state;
enum event_code { no_event, byte_received, error, timeout } current_event;

void event_set(enum event_code new_event){
    current_event=new_event;
}
void event_clear(void){
    current_event=no_event;
}
enum event_code event_get(void){
    return current_event;
}

unsigned char get_serial_buff(){
    // return getch(); // for console testing on a hardware with a display.

//    return SBUF; // modify according to your hardware.
    return 0; // modify according to your hardware.
}

void my_protocol_machine(){
    switch(current_state){
        case wait:
            switch(event_get()){
                case byte_received:
                    next_state=get_sof1;
                    break;
                case no_event:
                    next_state=wait;
                    event_set(no_event);
                    break;
                case error:
                	  next_state=wait;
                	  event_set(no_event);
                	  break;
                case timeout:
              	  next_state=wait;
              	  event_set(no_event);
              	  break;
            }
            break;
        case get_sof1:
            sof1 = get_serial_buff();
            next_state = check_get_sof2;
            break;
        case check_get_sof2:
            if(sof1 != ':'){ // in case of error
                next_state = wait;
                break;
            }
            sof2=get_serial_buff();
            next_state = check_get_dest;
            break;
        case check_get_dest:
            if( sof2 != '{' ){ // in case of error
                next_state = wait;
                break;
            }
            dest_addr = get_serial_buff();
            //next_state = get_src; UNCOMMENT
            break;
        case check_get_src:
            if (dest_addr != MY_ADDRESS ){// wrong address?
                next_state = wait;
                break;
            }
            src_addr = get_serial_buff();
            break;
        case get_size:
            frame_size = get_serial_buff();
            next_state = setup_count;
            break;
        case setup_count:
            if( frame_size<=7 ){
                next_state = wait;
                break;
            }
            ptr=malloc( frame_size-7 );
            counter = 0;
            next_state = get_byte_dec;
            break;
        case get_byte_dec:
            *(ptr+counter) = get_serial_buff();
            counter++;
            if ( counter>(frame_size-7) ){
                next_state= get_parity;
            } else {
                next_state = get_byte_dec;
            }
            break;
        case get_parity:
            parity1 = get_serial_buff();
            next_state = get_eof;
            break;
        case get_eof:
             eof1 = get_serial_buff();
            next_state = check_parity;
            break;
        case check_parity:
        	/* Commented out due to not exisitng function */
        	/*            if( checkParity() > 0 ){
                next_state = write_data;
            } else {
                next_state = discard_data;
            }*/
            break;
        case write_data:
            // do something here
            next_state = wait;
            break;
        case discard_data:
            // check and deallocate memory here
            if ( ptr != NULL){
                free(ptr);
            }
            ptr = NULL;
            next_state = wait;
            break;
        default:
        // check and deallocate memory here
           if ( ptr != NULL){
        	  free(ptr);
            }
        	ptr = NULL;
        	next_state = wait;
        	break;
    }

    current_state = next_state; //is added to the end of the code along with
    if(next_state == wait){ // no need for a timer
    	/* Commented out due to not existing function */
        /*timer_stop();*/
    } else {        // a valid state is next so restart the timing.
        /*reset_timer();*/
    }

    if( event_get() == timeout ){
        event_set(no_event);
        next_state = discard_data;
    }
    // do everything else here after the switch case including check for events
}
