
/**************************************************

file: demo_rx.c
purpose: simple demo that receives characters from
the serial port and print them on the screen,
exit the program by pressing Ctrl-C

compile with the command: gcc demo_rx.c rs232.c -Wall -Wextra -o2 -o test_rx

**************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// #include "c:\\i\Python\My.python\RS-232\rs232.h"
#include "rs232.h"

#define VERBOSE_CHARS 0
#define VERBOSE_SUMMARY 0
#define VERBOSE_EVERY 0
#define VERBOSE_BYTES 0
#define LIMITED_ITERATIONS 0

#define TEXT_SIZE 256

int RS232_PollComport_full(int cport_nr, unsigned char * buf, int nbytes)
{
    int i;
    int part_size;
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
        part_size = RS232_PollComport(cport_nr, dst_pointer, bytes_to_read);
        if(part_size > 0)
        {
            #if VERBOSE_CHARS
                printf("received %i bytes: ", part_size);
                for(i = 0; i < part_size; i++)
                {
                    printf("%02X", dst_pointer[i]);
                    if(i == part_size - 1)
                    {
                        printf("\n");
                    }
                    else
                    {
                        printf(" ");
                    }
                }
            #endif // VERBOSE_CHARS
            already_read += part_size;
            #if VERBOSE_SUMMARY
                printf("Total read: %i bytes, waiting for %i more bytes\n", already_read, nbytes - already_read);
            #endif // VERBOSE_SUMMARY
        }
        #if VERBOSE_EVERY
            if(part_size != 0)
            {
                printf("part_size: %i\n", part_size);
            }
        #endif // VERBOSE_EVERY
    }
    return already_read;
}

float calc_dust(unsigned char * dust_data)
{
    return (dust_data[1] * 256 + dust_data[0]) / 10.0;
}

int main(void)
{
    int
    cport_nr = 7,   /*    bylo cport_nr = 3     /dev/ttyS0 (COM1 on windows) */
    bdrate = 9600;  /* 9600 baud */
    float PM25ugpm3;
    float PM10ugpm3;
    unsigned char buf0;
    unsigned char buf1;
    unsigned char buf2[8];
    char mode[] = {'8', 'N', '1', 0};
    char out_text[TEXT_SIZE];
    time_t rawtime;
    struct tm * timeinfo;

    if(RS232_OpenComport(cport_nr, bdrate, mode))
    {
        printf("Can not open comport\n");
        return 0;
    }
    int nj = 10000;
    int char_cnt = 0;
    FILE * out_file;
    struct tm *tm_ptr;

    time( & rawtime);
    tm_ptr = localtime( & rawtime);
    sprintf(out_text, "out_c_%4d.%02d.%02d_%02d.%02d.%02d.csv",
        tm_ptr->tm_year,
        tm_ptr->tm_mon + 1,
        tm_ptr->tm_mday,
        tm_ptr->tm_hour,
        tm_ptr->tm_min,
        tm_ptr->tm_sec
        );
    printf("Nazwa: %s\n", out_text);
    out_file = fopen(out_text, "w");
    sprintf(out_text, "Time;PM 2.5 [ug/m^3];PM 10 [ug/m^3]\n");
    while(1)
    {
        if(char_cnt > 0)
        {
            buf0 = buf1; /* Copy previous byte */
        }
        RS232_PollComport_full(cport_nr, &buf1, 1);
        if(char_cnt < 2)
        {
            char_cnt++;
        }
        if(char_cnt == 2)
        {
            if ((buf0 == 0xAA) & (buf1 == 0xC0))
            {
                RS232_PollComport_full(cport_nr, buf2, 8);
                if(VERBOSE_BYTES)
                {
                    printf ("po (buf0==(0xAA))&(buf1==0xC0)");
                    printf ("prc.ld. buf0,buf1= %ld; %ld; \n", buf0, buf1);
                    printf ("prc.X.  buf0,buf1= %02X; %02X;   \n", buf0, buf1);
                    printf ("prc.i.  buf0,buf1= %i; %i;   \n", buf0, buf1);
                    printf ("prc.ld. buf2[0;1;2;3;4;]= %ld; %ld; %ld; %ld; %ld;\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
                    printf ("prc.X.  buf2[0;1;2;3;4;]= %02X; %02X; %02X; %02X; %02X;\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
                    printf ("prc.d.  buf2[0;1;2;3;4;]= %d; %d; %d; %d; %d;\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
                    printf ("prc.i.  buf2[0;1;2;3;4;]= %i; %i; %i; %i; %i;\n", buf2[0], buf2[1], buf2[2], buf2[3], buf2[4]);
                }
                PM25ugpm3 = calc_dust(buf2 + 0);
                PM10ugpm3 = calc_dust(buf2 + 2);
                printf("PM25ugpm3 = %4.2f  PM10ugpm3 = %4.2f\n", PM25ugpm3, PM10ugpm3);
                time( & rawtime);
                timeinfo = localtime ( & rawtime);
                fprintf(out_file, "%s;", asctime(timeinfo));
                sprintf(out_text, "%4.2f;%4.2f\n",  PM25ugpm3, PM10ugpm3);
                int i;
                int s_len = strlen(out_text);
                for(i = 0; i < s_len; i++)
                {
                    if(out_text[i] == '.')
                    {
                        out_text[i] = ',';
                    }
                }
                fprintf(out_file, "%s\n", out_text);
                fflush(out_file);
            }
        }
        #if LIMITED_ITERATIONS
        printf("nj %i\n", nj);
        if (--nj < 0)
        {
            break;
        }
        #endif // LIMITED_ITERATIONS
    }
    RS232_CloseComport(cport_nr);
    fclose(out_file);
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
