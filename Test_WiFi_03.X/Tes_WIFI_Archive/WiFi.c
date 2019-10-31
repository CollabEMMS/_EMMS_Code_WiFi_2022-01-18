
#include <p24FV32KA302.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "test_LEDs.h"
#include "pic_frequency.h"
#include "limits.h"
#include <libpic30.h>


#define CHAR_NULL '\0'
#define CHAR_CR '\r'
#define CHAR_NEW_LINE '\n'

#define BUF_LEN 250

struct struct_buf
{
    char buf[BUF_LEN];
    unsigned char posWrite;
    unsigned char posRead;
};

struct struct_buf bufSendWiFi;
struct struct_buf bufRecvWiFi;

struct struct_buf bufSendSPI;
struct struct_buf bufRecvSPI;

struct struct_buf bufSendSPIHolding;


void UART2SendString( char *message );
extern unsigned long timer_ms;

void wifi( bool init );
void wifiComm( void );
void wifiCommandSet( char *command, bool addPrefix );
void wifiServer( void );
bool wifiResponseCheck( char* response, char* varOutput );
bool wifiResponseEnd( void );
void wifiCommSend( void );
bool wifiCommSendChar( char data );
bool wifiCommRecv( void );
bool wifiCommRecvChar( char *data );
void wifiInit( void );

bool spiCommRecvChar( char *data );
bool spiCommRecv( void );
bool spiCommSendChar( char data );
void spiCommSend( void );
void spiComm( void );
bool spiResponseEnd( void );
void spiCommandSet( char *command );
void spiCommandCopyBufferHolding( void );
void spiServer( void );
void spi( bool init );
void spiInit( void );
void remove_all_chars( char* str, char c );
int return_char_length( char* str );
// Wifi-related variables

char networkInfo[100] = "";
//char ipAddr[20] = "";
//char macAddr[20] = "";
//char ssidInfo[50] = "";
bool getBuffer = false;
int counter = 0;
int debugMode = 1;
char wifiInfo[100] = "";
char spiResponse[50] = "";
//bool spiSendYes = false;
int sendingCommand = 0;


void wifiInit( )
{
    ANSBbits.ANSB0 = 0;
    ANSBbits.ANSB1 = 0;
    ANSBbits.ANSB2 = 0;

    U1MODEbits.UARTEN = 0b0;

#define UART_BAUD 9600

    U1BRG = (((FCY / 16) / UART_BAUD) - 1);

    U1MODEbits.USIDL = 0b0;
    U1MODEbits.IREN = 0b0;
    U1MODEbits.RTSMD = 0b0;
    U1MODEbits.UEN = 0b00;
    U1MODEbits.WAKE = 0b0;
    U1MODEbits.LPBACK = 0b0;
    U1MODEbits.ABAUD = 0b0;
    U1MODEbits.RXINV = 0b0;
    U1MODEbits.BRGH = 0b0;
    U1MODEbits.PDSEL = 0b00;
    U1MODEbits.STSEL = 0b0;

    U1STAbits.URXISEL = 0b00;
    U1STAbits.UTXINV = 0b0;
    U1STAbits.UTXBRK = 0b0;
    U1STAbits.UTXEN = 0b1;
    U1STAbits.URXISEL = 0b00;
    U1STAbits.ADDEN = 0b0;

    U1MODEbits.UARTEN = 0b1;
    U1STAbits.UTXEN = 0b1;




    U2MODEbits.UARTEN = 0b0;

    U2BRG = (((FCY / 16) / UART_BAUD) - 1);

    U2MODEbits.USIDL = 0b0;
    U2MODEbits.IREN = 0b0;
    U2MODEbits.RTSMD = 0b0;
    U2MODEbits.UEN = 0b00;
    U2MODEbits.WAKE = 0b0;
    U2MODEbits.LPBACK = 0b0;
    U2MODEbits.ABAUD = 0b0;
    U2MODEbits.RXINV = 0b0;
    U2MODEbits.BRGH = 0b0;
    U2MODEbits.PDSEL = 0b00;
    U2MODEbits.STSEL = 0b0;

    U2STAbits.URXISEL = 0b00;
    U2STAbits.UTXINV = 0b0;
    U2STAbits.UTXBRK = 0b0;
    U2STAbits.UTXEN = 0b1;
    U2STAbits.URXISEL = 0b00;
    U2STAbits.ADDEN = 0b0;

    U2MODEbits.UARTEN = 0b1;
    U2STAbits.UTXEN = 0b1;

    return;
}

void wifi( bool init )
{
    // here we handle all things wifi

    static int stage = 0;
    if( init == true )
    {
	bufSendWiFi.posRead = 0;
	bufSendWiFi.posWrite = 0;
	bufRecvWiFi.posRead = 0;
	bufRecvWiFi.posWrite = 0;
	stage = 0;
	wifiInit( );
    }
    wifiComm( );

    //    volatile int currentCommand;// used to make sure specific data is sent in tcp response
    //    volatile int currentWiFiNetwork; // Used to quickly save the network that works, to prevent dropping off of wifi and long startup times

    switch( stage )
    {
    case 0:
    {

	// reset and init
	// set command
	wifiCommandSet( "RST", true );
	stage++;
	break;
    }
    case 1:
	//wait for response
	if( wifiResponseCheck( "ready", "null" ) == true )
	{
	    stage++;
	}

	break;
    case 2:
	wifiCommandSet( "RFPOWER=82", true );
	stage++;
	break;
    case 3:
	if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    stage++;
	}
	if( wifiResponseCheck( "FAIL", "null" ) == true )
	{
	    stage = 0;
	}
	break;
    case 4:
	wifiCommandSet( "CWMODE_CUR=1", true );
	stage++;
	break;
    case 5:
	if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    stage++;
	}
	if( wifiResponseCheck( "FAIL", "null" ) == true )
	{
	    stage = 0;
	}
	break;
    case 6:
	wifiCommandSet( "CWLAP", true );
	//wifiCommandSet( "ATE0", false );
	stage++;
	break;
    case 7:
	if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    stage++;
	}
	if( wifiResponseCheck( "FAIL", "null" ) == true )
	{
	    stage = 0;
	}
	break;
    case 8:
    {
//	wifiCommandSet( "CWJAP_CUR=\"EMMS-Zim2019\",\"emms-mahadaga\"", true );
	        wifiCommandSet( "CWJAP_CUR=\"EMMS-Meters\",\"emms_meters\"", true );

	//        wifiCommandSet( "CWJAP_CUR=\"AW2\",\"*****\"", true ); // Not sure what this does
	stage++;
	break;
    }
    case 9:
    {
	static int cautiousMuch = 0;
	static int currentWiFiNetwork;
	if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    stage++;
	}
	if( wifiResponseCheck( "FAIL", "null" ) == true || wifiResponseCheck( "ERROR", "null" ) == true )
	{
	    //        if( cautiousMuch == 1 ) {
	    //            stage = 0;
	    //        } else
	    if( cautiousMuch == 2 )
	    {
		stage = 0;
	    }
	    else
	    {
		cautiousMuch = 0;
		__delay_ms( 1000 );
		if( currentWiFiNetwork == 1 )
		{
		    currentWiFiNetwork = 0;
		}
		else
		{
		    currentWiFiNetwork++;
		}


		if( currentWiFiNetwork == 0 )
		{
		    stage = 8;
		}
		if( currentWiFiNetwork == 1 )
		{
		    stage = 20;
		}
	    }
	}
	if( wifiResponseCheck( "WIFI CONNECTED", "null" ) == true )
	{
	    cautiousMuch = 2;
	}
	if( wifiResponseCheck( "WIFI DISCONNECT", "null" ) == true )
	{
	    cautiousMuch = 1;
	}

	if( wifiResponseCheck( "WIFI CONNECTED", "null" ) == true && cautiousMuch == 1 )
	{
	    cautiousMuch = 0;
	    stage = 0;
	}
	break;
    }
    case 10:
	getBuffer = true;
	memset( &networkInfo[0], 0, sizeof (networkInfo) );
	counter = 0;
	wifiCommandSet( "CIFSR", true );
	//    wifiCommandSet( "CIPAP=\"192.167.0.101\"", true );

	stage++;
	break;
    case 11:

	if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    getBuffer = false;
	    stage++;
	}
	if( wifiResponseCheck( "FAIL", "null" ) == true )
	{
	    getBuffer = false;
	    stage = 0;
	}
	if( wifiResponseCheck( "ERROR", "null" ) == true )
	{
	    getBuffer = false;
	    stage = 0;
	}

	break;
    case 12:
	wifiCommandSet( "CIPMUX=1", true );
	stage++;
	break;
    case 13:
	if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    stage++;
	}
	if( wifiResponseCheck( "FAIL", "null" ) == true )
	{
	    stage = 0;
	}
	break;
    case 14:
	wifiCommandSet( "CIPSERVER=0", true );
	stage++;
	break;
    case 15:
	if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    stage++;
	}
	if( wifiResponseCheck( "FAIL", "null" ) == true )
	{
	    stage = 0;
	}
	break;
    case 16:
	wifiCommandSet( "CIPSERVER=1,80", true );
	stage++;
	break;
    case 17:
	if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    stage++;
	}
	if( wifiResponseCheck( "FAIL", "null" ) == true )
	{
	    stage = 0;
	}
	break;
    case 18:

	//	    spiCommandSet( "\r\nConnected!*" );
	//	    for( int inx = 0; inx < 40; inx++ )
	//	    {
	//		if( LED1READ == 0 )
	//		{
	//		    LED1SET = 1;
	//		    LED2SET = 1;
	//		}
	//		else
	//		{
	//		    LED1SET = 0;
	//		    LED2SET = 0;
	//		}
	//		__delay_ms( 10 );
	//		runone = true;
	//	    }
	//	    LED1SET = 0;
	//	    LED2SET = 0;

	//    FLASHES FAST TO LET USER KNOW ITS INITIALIZED
	//    This should be to Show WIFI is ready to connect!
	if( debugMode == 1 )
	{
	    for( int inx = 0; inx < 30; inx++ )
	    {
		if( LED1READ == 0 )
		{
		    LED1SET = 1;
		}
		else
		{
		    LED1SET = 0;
		}
		__delay_ms( 30 );
	    }
	    LED1SET = 1;
	    LED3SET = 0;
	}
	// only do the light pulse once
	stage++;
	break;
    case 19:
    {
	static int currentCommand = 0;
        wifiServer( );


	if( wifiResponseCheck( "0,CONNECT", "null" ) == true )
	{
	    wifiCommandSet( "CIPSEND=0,31", true );
	    currentCommand = 1;
	}
	else if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    if( currentCommand == 1 )
	    {
		wifiCommandSet( "MESSIAH COLLEGE COLLABORATORY\n\r", false );
		currentCommand = 0;
	    }
	}

	// prints network info from cifsr
	if( wifiResponseCheck( "+IPD,0,14:!MOD;NETWORK*", "null" ) == true )
	{
	    // have it send variable of IP Shtuff from cifsr
	    remove_all_chars( networkInfo, '\n' );
	    remove_all_chars( networkInfo, '\r' );
	    remove_all_chars( networkInfo, '+' );
	    remove_all_chars( networkInfo, '>' );
	    remove_all_chars( networkInfo, '"' );
        strcat( networkInfo, "\n" );
        int networkInfoLen = strlen( networkInfo );
	    char len[5];
	    itoa( len, networkInfoLen, 10 );
	    char sendAmount[15] = "CIPSEND=0,";
	    strcat( sendAmount, len );
	    wifiCommandSet( sendAmount, true );
//	    wifiCommandSet( "CIPSEND=0,70", true );
	    //        char str[2] = "\n";
	    //        strcat(networkInfo, str);
	    //        char strPrefix[] = "CIPSEND=0,";
	    //        char toCharr[10];
	    //        int old_array_len = return_char_length(networkInfo);
	    //        sprintf(toCharr, "%d", old_array_len);
	    //        int new_array_len = 10 + return_char_length(toCharr);
	    //
	    //        char sendCommand[new_array_len];
	    //        for(int i =0; i < new_array_len; i++) {
	    //            if(i < 10) {
	    //                sendCommand[i] = strPrefix[i];
	    //            }
	    //            if( i >= 10 ) {
	    //                sendCommand[i] = toCharr[i-10];
	    //            }
	    //        }
	    //
	    //         wifiCommandSet( sendCommand, true );
	    currentCommand = 3;
	}
	else if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    if( currentCommand == 3 )
	    {
		wifiCommandSet( networkInfo, false );
//		wifiCommandSet( "+++", false );
		currentCommand = 0;
	    }
	}
	//"+IPD,0,13:!MOD;CONFIG*"
	if( wifiResponseCheck( "+IPD,0,13:!MOD;CONFIG*", "null" ) == true )
	{
	    // have it send variable of IP Shtuff from cifsr
	    // set to 78
	    char debugStat[10] = "";

	    if( debugMode == 0 )
	    {
		debugStat[0] = 'O';
		debugStat[1] = 'F';
		debugStat[2] = 'F';
	    }
	    else
	    {
		debugStat[0] = 'O';
		debugStat[1] = 'N';
	    }

	    strcat( wifiInfo, "EMMS Collaboratory Team;Spring 2019;WiFi V1.5;Additional Installation;1;Unknown;" );
	    strcat( wifiInfo, debugStat );
	    strcat( wifiInfo, ";\n" );
	    int wifiInfoLen = strlen( wifiInfo );
	    char len[5];
	    itoa( len, wifiInfoLen, 10 );
	    char sendAmount[20] = "CIPSEND=0,";
	    strcat( sendAmount, len );
	    wifiCommandSet( sendAmount, true );
	    currentCommand = 2;
	}
	else if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    if( currentCommand == 2 )
	    {
		wifiCommandSet( wifiInfo, false );
		//        wifiCommandSet( "+++" , false );
		memset( wifiInfo, 0, sizeof (wifiInfo) );
		currentCommand = 0;
	    }
	}

	if( wifiResponseCheck( "+IPD,0,11:!MOD;CLOSE*", "null" ) == true )
	{
	    // have it send variable of IP Shtuff from cifsr
	    wifiCommandSet( "CIPCLOSE=0", true );
	}


	if( wifiResponseCheck( "+IPD,0,12:!MOD;RESET*", "null" ) == true )
	{
	    // have it send variable of IP Shtuff from cifsr
	    wifiCommandSet( "CIPSEND=0,7", true );
	    currentCommand = 4;
	}
	else if( wifiResponseCheck( "OK", "null" ) == true )
	{
	    if( currentCommand == 4 )
	    {
		wifiCommandSet( "Bye Bye", false );
		//        currentCommand = 0;
	    }
	}
	else if( wifiResponseCheck( "SEND OK", "null" ) == true )
	{
	    if( currentCommand == 4 )
	    {
		wifiCommandSet( "CIPCLOSE=0", true );
		currentCommand = 0;
		asm( "RESET" ); // Resets Pic
	    }

	}
	if( wifiResponseCheck( "+IPD,0,17:!Set;Lights;Off*", "null" ) == true )
	{
	    debugMode = 0;
	    LED1SET = 0;
	    LED2SET = 0;
	    LED3SET = 0;
//	    wifiCommandSet( "CIPCLOSE=0", true );
	}


	if( wifiResponseCheck( "+IPD,0,16:!Set;Lights;On*", "null" ) == true )
	{
	    debugMode = 1;
	    LED1SET = 1;
//	    wifiCommandSet( "CIPCLOSE=0", true );

	}

	if( wifiResponseCheck( "WIFI DISCONNECT", "null" ) == true )
	{
	    // Means it Lost the Network
	    LED1SET = 0;

	    U2TXREG = 'L';
	    __delay_ms( 50 );
	    U2TXREG = 'o';
	    __delay_ms( 50 );
	    U2TXREG = 's';
	    __delay_ms( 50 );
	    U2TXREG = 't';
	    __delay_ms( 50 );
	    U2TXREG = ' ';
	    __delay_ms( 50 );
	    U2TXREG = 'C';
	    __delay_ms( 50 );
	    U2TXREG = 'o';
	    __delay_ms( 50 );
	    U2TXREG = 'n';
	    __delay_ms( 50 );
	    U2TXREG = 'n';
	    __delay_ms( 50 );
	    U2TXREG = 'e';
	    __delay_ms( 50 );
	    U2TXREG = 'c';
	    __delay_ms( 50 );
	    U2TXREG = 't';
	    __delay_ms( 50 );
	    U2TXREG = 'i';
	    __delay_ms( 50 );
	    U2TXREG = 'o';
	    __delay_ms( 50 );
	    U2TXREG = 'n';
	    stage = 0;
	}
    if( wifiResponseCheck( "OK", "null" ) == true  && sendingCommand == 2)
	{    
        ////////// THIS IS FOR SPI DONT MESS WITH IT PLZ K THX ///////
        strcat(spiResponse, "\n");
        wifiCommandSet(spiResponse, false);
        sendingCommand = 3;
//        spiSendYes == true;
    }
//    spiComm( );
	break;
    }
    case 20:
	__delay_ms( 70 );
	wifiCommandSet( "CWJAP_CUR=\"Goodwin\",\"123goodwin7\"", true );
	//        wifiCommandSet( "CWJAP_CUR=\"EMMS-Meters\",\"emms_meters\"", true );

	stage = 9;
    }



    if( wifiResponseEnd( ) == true )
    {
	bufRecvWiFi.posRead = 0;
	bufRecvWiFi.posWrite = 0;
    }

}

void wifiServer( void )
{

    int inPort = 0;
    int inDataLen = 0;
//    static int inx = 0;

    // check the buffers and see if we have a command
    if( wifiResponseEnd( ) == true )
    {
	// check if data came in
	if( strncmp( "+IPD,", bufRecvWiFi.buf, 5 ) == 0 )
	{
	    // get the port
#define ESP_BUF_IPD_START_PORT 5
#define ESP_BUF_IPD_START_LEN 7
	    char tmpbuf[3];

	    tmpbuf[0] = bufRecvWiFi.buf[ESP_BUF_IPD_START_PORT];
	    tmpbuf[1] = CHAR_NULL;

	    inPort = atoi( tmpbuf );

	    tmpbuf[0] = bufRecvWiFi.buf[ESP_BUF_IPD_START_LEN];
//	    switch( bufRecvWiFi.buf[ESP_BUF_IPD_START_LEN + 1] )
//	    {
//	    case ':':
//		tmpbuf[1] = CHAR_NULL;
//		bufRecvWiFi.posRead = 9;
//		break;
//	    default:
//		tmpbuf[1] = bufRecvWiFi.buf[ESP_BUF_IPD_START_LEN + 1];
//		tmpbuf[2] = CHAR_NULL;
//		bufRecvWiFi.posRead = 10;
//		break;
//	    }

	    inDataLen = atoi( tmpbuf );

	    // we make things here static in case a command spans multiple data packets from the esp8266
	    static bool inCommand = false;
#define ESP_BUF_IPD_COMMAND_PROCESS_SIZE 200
	    static char bufRecvWiFiCommand[ ESP_BUF_IPD_COMMAND_PROCESS_SIZE ];
	    static int bufRecvWiFiCommandPos = 0;

	    bufRecvWiFiCommandPos = 0;


	    while( bufRecvWiFi.buf[bufRecvWiFi.posRead ] != CHAR_NULL )
	    {
		if( inCommand == false )
		{
		    if( bufRecvWiFi.buf[bufRecvWiFi.posRead] == '!' )
		    {
			inCommand = true;

			//			if( LED1READ == 0 )
			//			{
			//			    LED1SET = 1;
			//			}
			//			else
			//			{
			//			    LED1SET = 0;
			//			}
		    }
		}

		if( inCommand == true )
		{
		    bufRecvWiFiCommand[bufRecvWiFiCommandPos] = bufRecvWiFi.buf[bufRecvWiFi.posRead ];
		    bufRecvWiFiCommandPos++;
		    if( bufRecvWiFiCommandPos >= ESP_BUF_IPD_COMMAND_PROCESS_SIZE )
		    {
			bufRecvWiFiCommandPos = (ESP_BUF_IPD_COMMAND_PROCESS_SIZE - 1);
		    }

		    if( bufRecvWiFi.buf[bufRecvWiFi.posRead ] == '*' )
		    {
			//			if( LED2READ == 0 )
			//			{
			//			    LED2SET = 1;
			//			}
			//			else
			//			{
			//			    LED2SET = 0;
			//			}

			bufRecvWiFiCommand[bufRecvWiFiCommandPos] = CHAR_NULL;
			inCommand = false;

			spiCommandSet( bufRecvWiFiCommand );
			bufRecvWiFiCommandPos = 0;
		    }
		}
        
		bufRecvWiFi.posRead++;

	    }
	}
//	bufRecvWiFi.posRead = 0;
//	bufRecvWiFi.posWrite = 0;


    }
}

void wifiCommandAdd( char *addCommand )
{
    //AT+CIPSENDBUF=<link ID>,<length>

    const char *commandDataPrefix = "CIPSEND=";

    int inx;
    char temp[BUF_LEN];
    int inxtemp;

    inx = 0;
    inxtemp = 0;

    while( commandDataPrefix[inx] != CHAR_NULL )
    {
	temp[inxtemp] = commandDataPrefix[inx];
	inxtemp++;
	if( inxtemp >= BUF_LEN )
	{
	    inxtemp = (BUF_LEN - 1);
	}
	inx++;
    }

    temp[inxtemp] = '0';
    inxtemp++;
    temp[inxtemp] = ',';
    inxtemp++;


    // need the length of addCommand
    int dataLen = 0;

    while( addCommand[dataLen] != '*' )
    {
	dataLen++;
    }
    dataLen++; // we need to add one since this is a count, not index

    char dataLenChar[3];

    itoa( dataLenChar, dataLen, 10 );

    inx = 0;
    while( dataLenChar[inx] != CHAR_NULL )
    {
	temp[inxtemp] = dataLenChar[inx];
	inxtemp++;
	inx++;
    }

    temp[inxtemp] = CHAR_NULL;
    wifiCommandSet( temp, true );

    bufSendWiFi.buf[ bufSendWiFi.posWrite] = CHAR_NULL;
    bufSendWiFi.posWrite++;
    if( bufSendWiFi.posWrite >= BUF_LEN )
    {
	bufSendWiFi.posWrite = (BUF_LEN - 1);
    }

    inx = 0;
    while( addCommand[inx] != '*' )
    {
	bufSendWiFi.buf[ bufSendWiFi.posWrite] = addCommand[inx];
	bufSendWiFi.posWrite++;
	if( bufSendWiFi.posWrite >= BUF_LEN )
	{
	    bufSendWiFi.posWrite = (BUF_LEN - 1);
	}
	inx++;
    }

    bufSendWiFi.buf[ bufSendWiFi.posWrite] = '*';
    bufSendWiFi.posWrite++;
    if( bufSendWiFi.posWrite >= BUF_LEN )
    {
	bufSendWiFi.posWrite = (BUF_LEN - 1);
    }

    return;

}

void wifiCommandSet( char *command, bool addPrefix )
{
    int inx;

    const char *commandPrefix = "AT+";
    const char *commandPostfix = "\r\n";

    if( addPrefix == true )
    {
	inx = 0;
	while( commandPrefix [inx] != CHAR_NULL )
	{
	    bufSendWiFi.buf[ bufSendWiFi.posWrite] = commandPrefix[inx];
	    bufSendWiFi.posWrite++;
	    if( bufSendWiFi.posWrite >= BUF_LEN )
	    {
		bufSendWiFi.posWrite = (BUF_LEN - 1);
	    }
	    inx++;
	}
    }

    inx = 0;
    while( command[inx] != CHAR_NULL )
    {
	bufSendWiFi.buf[bufSendWiFi.posWrite] = command[inx];
	bufSendWiFi.posWrite++;
	if( bufSendWiFi.posWrite >= BUF_LEN )
	{
	    bufSendWiFi.posWrite = (BUF_LEN - 1);
	}
	inx++;
    }

    inx = 0;
    while( commandPostfix [inx] != CHAR_NULL )
    {
	bufSendWiFi.buf[ bufSendWiFi.posWrite] = commandPostfix[inx];
	bufSendWiFi.posWrite++;
	if( bufSendWiFi.posWrite >= BUF_LEN )
	{
	    bufSendWiFi.posWrite = (BUF_LEN - 1);
	}
	inx++;
    }

    return;

}

bool wifiResponseCheck( char* response, char* varOutput )
{
    bool match = false;
    bool varmatch = false;
    bool keepBuffer = false;

    char doNothingChar[] = "null";
    char ipAddr[] = "ipAddr";
    char networkInfo[] = "networkInfo";
    char macAddr[] = "macAddr";
    char ssidInfo[] = "ssidInfo";
    char flush[] = "flush";

    int i = strcmp( doNothingChar, varOutput );

    if( i != 0 )
    {
	keepBuffer = true;
    }


    if( wifiResponseEnd( ) == true )
    {
	match = true;
	varmatch = true;
	int inx = 0;
	while( (response[inx] != CHAR_NULL) && (match == true) )
	{

	    // Checks response against incoming wifi stream
	    if( response[inx] != bufRecvWiFi.buf[inx] )
	    {
		match = false;
	    }

	    if( keepBuffer == true )
	    {
		if( strcmp( ipAddr, varOutput ) == 0 || strcmp( macAddr, varOutput ) == 0 || strcmp( networkInfo, varOutput ) == 0 || strcmp( ssidInfo, varOutput ) == 0 )
		{
		    //            U2TXREG = '%';
		    //            networkInfo[inx] += bufRecvWiFi.buf[inx];
		}
		i = strcmp( flush, varOutput );
		if( i == 0 )
		{
		    UART2SendString( "Flushed" );

		    networkInfo[inx] = ' ';
		    ipAddr[inx] = ' ';
		    macAddr[inx] = ' ';
		    ssidInfo[inx] = ' ';
		}
	    }

	    inx++;
	}


    }

    return match;
}

bool wifiResponseEnd( void )
{
    bool responseEnd = false;


    if( bufRecvWiFi.posWrite >= 2 )
    {
	if( bufRecvWiFi.buf[bufRecvWiFi.posWrite - 2] == CHAR_CR )
	{
	    if( bufRecvWiFi.buf[bufRecvWiFi.posWrite - 1] == CHAR_NEW_LINE )
	    {
		responseEnd = true;
	    }


	}
    }

    return responseEnd;
}

void wifiComm( )
{
    wifiCommSend( );
    wifiCommRecv( );

    return;
}

void wifiCommSend( void )
{
    if( bufSendWiFi.posRead != bufSendWiFi.posWrite )
    {
	if( U1STAbits.UTXBF == 0 )
	{
	    if( wifiCommSendChar( bufSendWiFi.buf[bufSendWiFi.posRead] ) == true )
	    {
		bufSendWiFi.posRead++;
		if( bufSendWiFi.posRead >= BUF_LEN )
		{
		    bufSendWiFi.posRead = 0;
		}
	    }
	}
    }
    else
    {
	bufSendWiFi.posRead = 0;
	bufSendWiFi.posWrite = 0;
    }
    return;
}

bool wifiCommSendChar( char data )
{
    bool dataSent = false;
    static bool pause = false;
    static unsigned long pauseTimer;
    static bool pauseTimerOverflowWait;
#define PAUSE_TIME_MS 100

    if( pause == false )
    {
	if( data == CHAR_NULL )
	{
	    pause = true;
	    if( (ULONG_MAX - timer_ms) > PAUSE_TIME_MS )
	    {
		pauseTimer = timer_ms + PAUSE_TIME_MS;
	    }
	    else
	    {
		pauseTimer = PAUSE_TIME_MS - (ULONG_MAX - timer_ms);
		pauseTimerOverflowWait = true;
	    }

	    dataSent = true;
	}
	else
	{
	    if( U1STAbits.UTXBF == 0 )
	    {
		U1TXREG = data;
		dataSent = true;
		U2TXREG = data;

		// Flashes at UART Communication
		if( debugMode == 1 )
		{
		    if( LED3READ == 0 )
		    {
			LED3SET = 1;
		    }
		    else
		    {
			LED3SET = 0;
		    }
		}
	    }
	}
    }
    else
    {
	if( pauseTimerOverflowWait == true )
	{
	    if( timer_ms < pauseTimer )
	    {
		pauseTimerOverflowWait = false;
	    }
	}

	if( pauseTimerOverflowWait == false )
	{
	    if( timer_ms >= pauseTimer )
	    {
		pause = false;
	    }
	}
    }
    return dataSent;
}

bool wifiCommRecv( void )
{
    bool dataReceived = false;
    char data;

    if( wifiCommRecvChar( &data ) == true )
    {
	bufRecvWiFi.buf[bufRecvWiFi.posWrite] = data;
	dataReceived = true;
	bufRecvWiFi.posWrite++;
	if( bufRecvWiFi.posWrite >= BUF_LEN )
	{
	    bufRecvWiFi.posWrite = 0;
	}
	bufRecvWiFi.buf[bufRecvWiFi.posWrite] = CHAR_NULL;
    }

    return dataReceived;
}

bool wifiCommRecvChar( char *data )
{
    bool dataReceived = false;
    if( U1STAbits.URXDA == 1 )
    {
	*data = U1RXREG;
	dataReceived = true;
	U2TXREG = *data;
	if( getBuffer == true )
	{
	    networkInfo[counter] = *data;
	    counter++;
	}
	if( counter == 101 )
	{
	    counter = 0;
	}

	// Flashes at UART Communication
	if( debugMode == 1 )
	{
	    if( LED3READ == 0 )
	    {
		LED3SET = 1;
	    }
	    else
	    {
		LED3SET = 0;
	    }
	}
    }
    return dataReceived;
}

void spi( bool init )
{
    if( init == true )
    {
	bufSendSPI.posRead = 0;
	bufSendSPI.posWrite = 0;
	bufRecvSPI.posRead = 0;
	bufRecvSPI.posWrite = 0;

	bufSendSPIHolding.posRead = 0;
	bufSendSPIHolding.posWrite = 0;

	spiInit( );
	UART2SendString( "SPI Enabled" );

	//    FLASHES FAST TO LET USER KNOW ITS INITIALIZED
	//    This should be first blink after Boot Sequence
	if( debugMode == 1 )
	{
	    for( int inx = 0; inx < 30; inx++ )
	    {
		if( LED2READ == 0 )
		{
		    LED2SET = 1;
		}
		else
		{
		    LED2SET = 0;
		}
		__delay_ms( 30 );
	    }
	}

    } // END OF IF SPI(TRUE) condition

//    // DEBUG TOM
//    static unsigned long debugSPITimerOld = 0;
//    // send something every 3 seconds so we can see if it comes out the other side
//    if( (timer_ms - debugSPITimerOld) >= 3000 )
//    {
//	spiCommandSet( "!Read;PwrData*" );
//	UART2SendString( "!Read;PwrData*" );
//	debugSPITimerOld = timer_ms;
//    }
//    // DEBUG TOM

    //    SPI initialization error control (?)
    static bool spiInReset = false;
//    LED1SET = PORTBbits.RB15;// blinks chip select
    if( PORTBbits.RB15 == 1 )
    {
	if( spiInReset == false )
	{
	    spiInReset = true;
	    SPI1STATbits.SPIEN = 0; //disable SPI
	    spiCommandCopyBufferHolding( );
	}
    }
    else
    {
	if( spiInReset == true ) // One shot
	{
	    spiInReset = false;
	    bufRecvSPI.posRead = 0;
	    bufRecvSPI.posWrite = 0;
	    // spiCommandSet( "!Set;Watts;55*" );
	    SPI1STATbits.SPIEN = 1; //enable SPI
	}
	// This is running every clock cycle
	spiComm( );
	spiServer( );

	// here we need to check if we pass on a command

	if( spiResponseEnd( ) == true )
	{
	    bufRecvSPI.posRead = 0;
	    bufRecvSPI.posWrite = 0;
	}
    }
}

void spiServer( )
{

    // check the buffers and see if we have a command
    if( spiResponseEnd( ) == true )
    {
	//        LED3SET = 1;

	if( bufRecvSPI.buf[0] == '!' )
	{
	    //	    LED3SET = 1;
	    wifiCommandAdd( bufRecvSPI.buf );
	}
    }
}

void spiCommandSet( char *command )
{
    int inx;
    inx = 0;
    while( command[inx] != '*' )
    {

	//LED1SET = 1;
	bufSendSPIHolding.buf[bufSendSPIHolding.posWrite] = command[inx];
	bufSendSPIHolding.posWrite++;

	if( bufSendSPIHolding.posWrite >= BUF_LEN )
	{
	    bufSendSPIHolding.posWrite = 0;
	}
	inx++;
    }
    bufSendSPIHolding.buf[bufSendSPIHolding.posWrite] = command[inx];
    bufSendSPIHolding.posWrite++;
    if( bufSendSPIHolding.posWrite >= BUF_LEN )
    {
	bufSendSPIHolding.posWrite = 0;
    }

    return;
}

void spiCommandCopyBufferHolding( )
{
    while( bufSendSPIHolding.posRead != bufSendSPIHolding.posWrite )
    {
	bufSendSPI.buf[bufSendSPI.posWrite] = bufSendSPIHolding.buf[bufSendSPIHolding.posRead];
	bufSendSPI.posWrite++;
	if( bufSendSPI.posWrite >= BUF_LEN )
	{
	    bufSendSPI.posWrite = 0;
	}
	bufSendSPIHolding.posRead++;
	if( bufSendSPIHolding.posRead >= BUF_LEN )
	{
	    bufSendSPIHolding.posRead = 0;
	}
    }

    bufSendSPIHolding.posRead = 0;
    bufSendSPIHolding.posWrite = 0;


    return;

}

bool spiResponseEnd( void )
{
    bool responseEnd = false;

    if( bufRecvSPI.buf[ bufRecvSPI.posWrite - 1 ] == '*' )
    {
	//		if( LED3READ == 0 )
	//		{
	//		    LED3SET = 1;
	//		}
	//		else
	//		{
	//		    LED3SET = 1;
	//		}

	responseEnd = true;
    }

    return responseEnd;
}

void spiComm( void )
{
    spiCommSend( );
    spiCommRecv( );

    return;
}

void spiCommSend( void )
{
    if( bufSendSPI.posRead != bufSendSPI.posWrite )
    {
	if( spiCommSendChar( bufSendSPI.buf[bufSendSPI.posRead] ) == true )
	{
	    bufSendSPI.posRead++;

	    if( bufSendSPI.posRead >= BUF_LEN )
	    {
		bufSendSPI.posRead = 0;
	    }
	}
    }
    else
    {
	//  Confirmed, does send
	bufSendSPI.posRead = 0;
	bufSendSPI.posWrite = 0;
    }

    return;
}

//bool spiCommSendCharUART( char data )
//{
//    bool dataSent = false;
//
//    if( U2STAbits.UTXBF == 0 )
//    {
//	U2TXREG = data;
//	dataSent = true;
//    }
//
//    return dataSent;
//}

bool spiCommSendChar( char data )
{
    bool sendGood = false;


    if( SPI1STATbits.SPITBF == 0b0 ) //if in enhance mode use SPI1STATbits.SR1MPT
    {
	SPI1BUF = data;
	sendGood = true;
//	__delay_ms( 1 ); /// ZACH MAYBE COME BACK AND FIX THIS UNCOMMENT THIS OUT IF IT CRASHED
     if(debugMode == 1 ) {
            if(LED2READ == 0) {
                LED2SET = 1;
            } else {
                LED2SET = 0;
            }
            }
	//	U2TXREG = data;
    }

    return sendGood;
}

bool spiCommRecv( void )
{
    bool dataReceived = false;
    char data;
    if( spiCommRecvChar( &data ) == true )
    {

	bufRecvSPI.buf[bufRecvSPI.posWrite] = data;
	dataReceived = true;
	bufRecvSPI.posWrite++;
	if( bufRecvSPI.posWrite >= BUF_LEN )
	{
	    bufRecvSPI.posWrite = 0;
	}
	bufRecvSPI.buf[bufRecvSPI.posWrite] = CHAR_NULL;
    }

    return dataReceived;

}

//bool spiCommRecvCharUART( char *data )
//{
//    bool dataReceived = false;
//
//    if( U2STAbits.URXDA == 1 )
//    {
//	*data = U2RXREG;
//	dataReceived = true;
//    }
//
//    return dataReceived;
//}

bool spiCommRecvChar( char *data )
{
    static int counter = 0;
    static int inCommand = 0;
    bool recvGood = false;
    if( SPI1STATbits.SPIRBF == 0b1 )
    {
	*data = SPI1BUF;
	recvGood = true;
//     U2TXREG = *data;
//		if( *data != '#' )
//		{
//        U2TXREG = *data;
//		}
//    static unsigned long debugSPITimerOld = 0;
//    // send something every 3 seconds so we can see if it comes out the other side
//    if( (timer_ms - debugSPITimerOld) >= 3000 && sendingCommand != 0)
//    {
//        U2TXREG = sendingCommand;
//        U2TXREG = '#'
//        sendingCommand = 0;
//        debugSPITimerOld = timer_ms;
//    }
    if( *data == '!' ) {
        inCommand = 1;
        counter = 0;
    }
    if( *data == '*' && inCommand == 1) {
        sendingCommand = 1;
        inCommand = 0;
    }

    if( inCommand == 1 ) {
        if( *data != '#' && sendingCommand == 0 )
        {
            U2TXREG = *data;
//            U2TXREG = '7';
            if(debugMode == 1 ) {
            if(LED2READ == 0) {
                LED2SET = 1;
            } else {
                LED2SET = 0;
            }
            }
            spiResponse[counter] = *data;
            if( counter > 50 ) {
                counter = 0;
            } else {
                counter++;
            }
        }
    }
    
    if( sendingCommand == 3 ) {
        memset( &spiResponse[0], 0, sizeof (spiResponse) );
        counter = 0;
        sendingCommand = 0;
    } 
//    if ( sendingCommand == 2 && spiSendYes == true) {
//        sendingCommand = 3;
//        spiSendYes = false;
//    }
    if( sendingCommand == 1 ) {
        char prefix[] = "CIPSEND=0,";
        char dataLenChar[3];
        counter++;
        counter++;
        itoa( dataLenChar, counter, 10 );
        strcat(prefix,dataLenChar);
        wifiCommandSet(prefix, true);
        sendingCommand = 2;
    }
    }
    
    return recvGood;
}

void spiInit( void )
{
    // make sure analog is turned off - it messes with the pins
    ANSA = 0;
    ANSB = 0;
    //    ANSBbits.ANSB12 = 0;

    TRISBbits.TRISB5 = 1;
    TRISBbits.TRISB6 = 0;

    TRISBbits.TRISB12 = 1;
    //    TRISAbits.TRISA7 = 1;


    TRISBbits.TRISB15 = 1;
    TRISBbits.TRISB11 = 1;
    TRISBbits.TRISB10 = 1;
    TRISBbits.TRISB13 = 0;

    //SPI1 Initialize as Slave
    SPI1CON1bits.MSTEN = 0;

    SPI1CON1bits.DISSCK = 0b1; // SPI clock disabled
    SPI1CON1bits.DISSDO = 0b0; // SDO used
    SPI1CON1bits.MODE16 = 0b0; // 8 bit mode
    SPI1CON1bits.SMP = 0b0; // sample phase mode middle
    SPI1CON1bits.CKE = 0b1; // serial data changes on active to idle clock state
    SPI1CON1bits.SSEN = 0b1; // yes a slave
    SPI1CON1bits.CKP = 0b1; // clock idle is high
    SPI1CON1bits.SPRE = 0b000; // secondary prescale 8:1, not used - no clock is run

    SPI1CON2bits.FRMEN = 0b0; // frame mode, unused
    SPI1CON2bits.SPIFSD = 0b0; // frame mode, unused
    SPI1CON2bits.SPIFPOL = 0b0; // frame mode, unused
    SPI1CON2bits.SPIFE = 0b0; // frame mode, unused

    SPI1CON2bits.SPIBEN = 0b0; // 1=enhanced buffer mode

    SPI1STATbits.SPIROV = 0; //clear flag for overflow data

    SPI1BUF = SPI1BUF; //clear the buffer
    SPI1STATbits.SPIEN = 1; //enable SPI

    return;
}

void remove_all_chars( char* str, char c )
{
    char *pr = str, *pw = str;
    while( *pr )
    {
	*pw = *pr++;
	pw += (*pw != c);
    }
    *pw = '\0';
}