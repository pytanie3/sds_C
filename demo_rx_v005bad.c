
/**************************************************

file: demo_rx.c
purpose: simple demo that receives characters from
the serial port and print them on the screen,
exit the program by pressing Ctrl-C

compile with the command: gcc demo_rx.c rs232.c -Wall -Wextra -o2 -o test_rx

**************************************************/

#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// #include "c:\\i\Python\My.python\RS-232\rs232.h"
#include "rs232.h"

#define VERBOSE_CHARS 1
#define VERBOSE_SUMMARY 1

int RS232_PollComport_full(int cport_nr, unsigned char * buf, int nbytes)
{
    int i;
    int n;
    int already_read = 0;
    int bytes_to_read;
    unsigned char * dst_pointer;
    while(1)
    {
        bytes_to_read = nbytes - already_read;
        if(bytes_to_read <= 0)
        {
            break;
        }
        dst_pointer = buf + already_read;
        n = RS232_PollComport(cport_nr, dst_pointer, bytes_to_read);
        if(n > 0)
        {
            #if VERBOSE_CHARS
                printf("received %i bytes: ", n);
                for(i = 0; i < n; i++)
                {
                    printf("%02X", dst_pointer[i]);
                    if(i == n - 1)
                    {
                        printf("\n");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
            #endif // VERBOSE_CHARS
            already_read += n;
            #if VERBOSE_SUMMARY
                printf("Total read: %i bytes, waiting for %i more bytes\n", already_read, nbytes - already_read);
            #endif // VERBOSE_SUMMARY
        }
    }
    return already_read;
}

int main(void)
{
    int byte = 0,
    lastbyte = 0,
    PM25_High_byte,
    PM25_Low_byte,
    PM10ugpm3 = 0,
    PM10_High_byte,
    PM10_Low_byte,
    i = 0,
    j0 = 0,
    j1 = 0,
    j2 = 0,
    n = 0,
    k_while = 0,
    cport_nr = 7,   /*    bylo cport_nr = 3     /dev/ttyS0 (COM1 on windows) */
    bdrate = 9600;  /* 9600 baud */
    float PM25ugpm3 = 0;
    unsigned char buf0;
    unsigned char buf1;
    unsigned char buf2[8];
    unsigned char buf3[8];
    unsigned char packet[8];
    unsigned char ind = 0;
    unsigned char state = 0;      // 0 - oczekujemy na znak 0xAA
    // 1 - oczekujemy na znak 0xC0
    // 2 - oczekujemy na 8 pozosta³ych bajtów
    char mode[] = {'8', 'N', '1', 0};

    j0 = RS232_PollComport_full(cport_nr, &buf0, 1);
    printf ("Start przed if floats: %4.2f %ld %+.0e  \n", 3.1416, k_while, 3.1416);
    if(RS232_OpenComport(cport_nr, bdrate, mode))
    {
        printf("Can not open comport\n");
        return 0;
    }
    int nj = 10000;
    int char_cnt = 0;
    while(1)
    {
        if(char_cnt > 0)
        {
            buf1 = buf0; /* Copy previous byte */
        }
        j0 = RS232_PollComport_full(cport_nr, &buf0, 1);
        if(char_cnt < 2)
        {
            char_cnt++;
        }
        if(char_cnt == 2)
        {
            if ((buf0 == 0xAA) & (buf1 == 0xC0))
            {                                       //if (buf2[0] == 0xAA)
                printf("odebrano pakiet\n");
                j2 = RS232_PollComport_full(cport_nr, buf2, 8);
                printf ("po (buf0==(0xAA))&(buf1==0xC0)");
                printf ("prc.ld. buf0,buf1= %ld; %ld; \n", buf0, buf1);
                printf ("prc.X.  buf0,buf1= %02X; %02X;   \n", buf0, buf1);
                printf ("prc.i.  buf0,buf1= %i; %i;   \n", buf0, buf1);
                printf ("prc.ld. buf2[0;1;2;3;4;]= %ld; %ld; %ld; %ld; %ld;\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
                printf ("prc.X.  buf2[0;1;2;3;4;]= %02X; %02X; %02X; %02X; %02X;\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
                printf ("prc.d.  buf2[0;1;2;3;4;]= %d; %d; %d; %d; %d;\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
                printf ("prc.i.  buf2[0;1;2;3;4;]= %i; %i; %i; %i; %i;\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
                state = 0;
                PM25_High_byte = buf3[1];
                PM25_Low_byte = buf3[0];
                PM25ugpm3 = (
                    (
                        PM25_High_byte * 256 + PM25_Low_byte
                    )
                    /
                    10.0);
                printf("PM25ugpm3= %4.2f\n", PM25ugpm3);
                printf("nj %i \n",nj) ;
            }
        }
        if (--nj < 0)
        {
            break;
        }
    }
    return 0;
} //main

/*
# 00 01 02 03 04 05 06 07 08 09
# AA C0 72 00 90 00 62 F0 54 AB
# 0 MessageHeader AA
# 1 CommanderNo. C0
# 2 DATA 1 PM2.5 Low byte
# 3 DATA 2 PM2.5 High byte
# 4 DATA 3 PM10 Low byte
# 5 DATA 4 PM10 High byte
#PM2.5 value: PM25ugpm3 = ((PM25_High_byte *256) + PM25_low_byte)/10
PM25_Low_byte = 72
PM25_High_byte = 00
struct.unpack(72)
bytes([00])
PM25ugpm3 = ((PM25_High_byte *256) + PM25_Low_byte)/10
*/
