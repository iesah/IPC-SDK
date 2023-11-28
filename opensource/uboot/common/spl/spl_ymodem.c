/*
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2011
 * Texas Instruments, <www.ti.com>
 *
 * Matt Porter <mporter@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <spl.h>
#include <xyzModem.h>
#include <asm/u-boot.h>
//#include <asm/utils.h>
#include <stdarg.h>
#include <crc.h>

#define UART_YMODEM_DEBUG
#define BUF_SIZE 1024

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#define SOH 0x01
#define STX 0x02
#define EOT 0x04
#define ACK 0x06
#define BSP 0x08
#define NAK 0x15
#define CAN 0x18
#define EOF 0x1A		/* ^Z for DOS officionados */

#define USE_YMODEM_LENGTH

/* Data & state local to the protocol */
static struct
{
#ifdef REDBOOT
    hal_virtual_comm_table_t *__chan;
#else
    int *__chan;
#endif
    unsigned char pkt[1024], *bufp;
    unsigned char blk, cblk, crc1, crc2;
    unsigned char next_blk;	/* Expected block */
    int len, mode, total_retries;
    int total_SOH, total_STX, total_CAN;
    bool crc_mode, at_eof, tx_ack;
#ifdef USE_YMODEM_LENGTH
    unsigned long file_length, read_length;
#endif
} xyz;

#define xyzModem_CHAR_TIMEOUT            1000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3	/* Wait for 3 CAN before quitting */
static int test_count = 0;

#ifndef REDBOOT			/*SB */
typedef int cyg_int32;
    static int
CYGACC_COMM_IF_GETC_TIMEOUT (char chan, char *c)
{
#define DELAY 20
    unsigned long counter = 0;
    while (!tstc () && (counter < xyzModem_CHAR_TIMEOUT * 1000 / DELAY))
    {
        udelay (DELAY);
        counter++;
    }
    if (tstc ())
    {
        *c = getc ();
        return 1;
    }
    return 0;
}

static void CYGACC_COMM_IF_PUTC (char x, char y)
{
    putc (y);
}

/* Validate a hex character */
__inline__ static bool _is_hex (char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ static int _from_hex (char c)
{
    int ret = 0;

    if ((c >= '0') && (c <= '9'))
    {
        ret = (c - '0');
    }
    else if ((c >= 'a') && (c <= 'f'))
    {
        ret = (c - 'a' + 0x0a);
    }
    else if ((c >= 'A') && (c <= 'F'))
    {
        ret = (c - 'A' + 0x0A);
    }
    return ret;
}

/* Convert a character to lower case */
__inline__ static char _tolower (char c)
{
    if ((c >= 'A') && (c <= 'Z'))
    {
        c = (c - 'A') + 'a';
    }
    return c;
}

/* Parse (scan) a number */
static bool parse_num (char *s, unsigned long *val, char **es, char *delim)
{
    bool first = true;
    int radix = 10;
    char c;
    unsigned long result = 0;
    int digit;

    while (*s == ' ')
        s++;
    while (*s)
    {
        if (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
        {
            radix = 16;
            s += 2;
        }
        first = false;
        c = *s++;
        if (_is_hex (c) && ((digit = _from_hex (c)) < radix))
        {
            /* Valid digit */
#ifdef CYGPKG_HAL_MIPS
            /* FIXME: tx49 compiler generates 0x2539018 for MUL which */
            /* isn't any good. */
            if (16 == radix)
                result = result << 4;
            else
                result = 10 * result;
            result += digit;
#else
            result = (result * radix) + digit;
#endif
        }
        else
        {
            if (delim != (char *) 0)
            {
                /* See if this character is one of the delimiters */
                char *dp = delim;
                while (*dp && (c != *dp))
                    dp++;
                if (*dp)
                    break;		/* Found a good delimiter */
            }
            return false;		/* Malformatted number */
        }
    }
    *val = result;
    if (es != (char **) 0)
    {
        *es = s;
    }
    return true;
}

#endif

#define USE_SPRINTF
#ifdef DEBUG
#ifndef USE_SPRINTF
/*
 * Note: this debug setup only works if the target platform has two serial ports
 * available so that the other one (currently only port 1) can be used for debug
 * messages.
 */
static int zm_dprintf (char *fmt, ...)
{
    int cur_console;
    va_list args;

    va_start (args, fmt);
#ifdef REDBOOT
    cur_console =
        CYGACC_CALL_IF_SET_CONSOLE_COMM
        (CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
    CYGACC_CALL_IF_SET_CONSOLE_COMM (1);
#endif
    diag_vprintf (fmt, args);
#ifdef REDBOOT
    CYGACC_CALL_IF_SET_CONSOLE_COMM (cur_console);
#endif
}

static void zm_flush (void)
{
}

#else
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
#define FINAL
#ifdef FINAL
static char *zm_out = (char *) 0x00380000;
static char *zm_out_start = (char *) 0x00380000;
#else
static char zm_buf[8192];
static char *zm_out = zm_buf;
static char *zm_out_start = zm_buf;

#endif
    static int
zm_dprintf (char *fmt, ...)
{
    int len;
    va_list args;

    va_start (args, fmt);
    len = diag_vsprintf (zm_out, fmt, args);
    zm_out += len;
    return len;
}

    static void
zm_flush (void)
{
#ifdef REDBOOT
    char *p = zm_out_start;
    while (*p)
        mon_write_char (*p++);
#endif
    zm_out = zm_out_start;
}
#endif

    static void
zm_dump_buf (void *buf, int len)
{
#ifdef REDBOOT
    diag_vdump_buf_with_offset (zm_dprintf, buf, len, 0);
#else

#endif
}

static unsigned char zm_buf[2048];
static unsigned char *zm_bp;

    static void
zm_new (void)
{
    zm_bp = zm_buf;
}

    static void
zm_save (unsigned char c)
{
    *zm_bp++ = c;
}

    static void
zm_dump (int line)
{
    zm_dprintf ("Packet at line: %d\n", line);
    zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#else
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
static void xyzModem_flush (void)
{
    int res;
    char c;
    while (true)
    {
        res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
        if (!res)
            return;
    }
}

static int xyzModem_get_hdr (void)
{
    char c;
    int res;
    bool hdr_found = false;
    int i, can_total, hdr_chars;
    unsigned short cksum;

  //  ZM_DEBUG (zm_new ());
    /* Find the start of a header */
    can_total = 0;
    hdr_chars = 0;

    if(xyz.tx_ack){
        CYGACC_COMM_IF_PUTC(*xyz.__chan,ACK);
        xyz.tx_ack =false;
    }

    while (!hdr_found)
    {
        res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
        ZM_DEBUG (zm_save (c));
        if (res)
        {
            hdr_chars++;
            switch (c)
            {
            case SOH:
                xyz.total_SOH++;
            case STX:
                if (c == STX)
                    xyz.total_STX++;
                hdr_found = true;
                break;
            case CAN:
                xyz.total_CAN++;
                ZM_DEBUG (zm_dump (__LINE__));
                if (++can_total == xyzModem_CAN_COUNT)
                {
                    return xyzModem_cancel;
                }
                else
                {
                    /* Wait for multiple CAN to avoid early quits */
                    break;
                }
            case EOT:
                /* EOT only supported if no noise */
                if (hdr_chars == 1)
                {
                    CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
                    ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
                    ZM_DEBUG (zm_dump (__LINE__));
                    return xyzModem_eof;
                }
            default:
                /* Ignore, waiting for start of header */
                ;
            }
        }
        else
        {
            /* Data stream timed out */
            xyzModem_flush ();	/* Toss any current input */
            ZM_DEBUG (zm_dump (__LINE__));
            CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
            return xyzModem_timeout;
        }
    }

    /* Header found, now read the data */
    res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (char *) &xyz.blk);
    ZM_DEBUG (zm_save (xyz.blk));
    if (!res)
    {
        ZM_DEBUG (zm_dump (__LINE__));
        return xyzModem_timeout;
    }
    res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (char *) &xyz.cblk);
    ZM_DEBUG (zm_save (xyz.cblk));
    if (!res)
    {
        ZM_DEBUG (zm_dump (__LINE__));
        return xyzModem_timeout;
    }
    xyz.len = (c == SOH) ? 128 : 1024;
    xyz.bufp = xyz.pkt;
    for (i = 0; i < xyz.len; i++)
    {
        res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
        ZM_DEBUG (zm_save (c));
        if (res)
        {
            xyz.pkt[i] = c;
        }
        else
        {
            ZM_DEBUG (zm_dump (__LINE__));
            return xyzModem_timeout;
        }
    }
    res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (char *) &xyz.crc1);
    ZM_DEBUG (zm_save (xyz.crc1));
    if (!res)
    {
        ZM_DEBUG (zm_dump (__LINE__));
        return xyzModem_timeout;
    }
    if (xyz.crc_mode)
    {
        res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (char *) &xyz.crc2);
        ZM_DEBUG (zm_save (xyz.crc2));
        if (!res)
        {
            ZM_DEBUG (zm_dump (__LINE__));
            return xyzModem_timeout;
        }
    }
    ZM_DEBUG (zm_dump (__LINE__));
    /* Validate the message */
    if(xyz.next_blk == 0){
        xyz.next_blk = xyz.blk;
    }
    if ((xyz.blk ^ xyz.cblk) != (unsigned char) 0xFF)
    {
        ZM_DEBUG (zm_dprintf
                  ("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
                   (xyz.blk ^ xyz.cblk)));
        ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
        xyzModem_flush ();
        return xyzModem_frame;
    }
    /* Verify checksum/CRC */
    if (xyz.crc_mode)
    {
        cksum = cyg_crc16 (xyz.pkt, xyz.len);
        if (cksum != ((xyz.crc1 << 8) | xyz.crc2))
        {
            ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
                                  xyz.crc1, xyz.crc2, cksum & 0xFFFF));
            return xyzModem_cksum;
        }
    }
    else
    {
        cksum = 0;
        for (i = 0; i < xyz.len; i++)
        {
            cksum += xyz.pkt[i];
        }
        if (xyz.crc1 != (cksum & 0xFF))
        {
            ZM_DEBUG (zm_dprintf
                      ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
                       cksum & 0xFF));
            return xyzModem_cksum;
        }
    }
    /* If we get here, the message passes [structural] muster */
    //printf("----3---\n");
    return 0;
}

int xyzModem_stream_open(connection_info_t * info, int *err)
{
#ifdef REDBOOT
    int console_chan;
#endif
    int stat = 0;
    int retries = xyzModem_MAX_RETRIES;
    int crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

    /*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
    if (info->mode == xyzModem_zmodem)
    {
        *err = xyzModem_noZmodem;
        return -1;
    }
#endif

#ifdef REDBOOT
    /* Set up the I/O channel.  Note: this allows for using a different port in the future */
    console_chan =
        CYGACC_CALL_IF_SET_CONSOLE_COMM
        (CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
    if (info->chan >= 0)
    {
        CYGACC_CALL_IF_SET_CONSOLE_COMM (info->chan);
    }
    else
    {
        CYGACC_CALL_IF_SET_CONSOLE_COMM (console_chan);
    }
    xyz.__chan = CYGACC_CALL_IF_CONSOLE_PROCS ();

    CYGACC_CALL_IF_SET_CONSOLE_COMM (console_chan);
    CYGACC_COMM_IF_CONTROL (*xyz.__chan, __COMMCTL_SET_TIMEOUT,
                            xyzModem_CHAR_TIMEOUT);
#else
    /* TODO: CHECK ! */
    int dummy = 0;
    xyz.__chan = &dummy;
#endif
    xyz.len = 0;
    xyz.crc_mode = true;
    xyz.at_eof = false;
    xyz.tx_ack = false;
    xyz.mode = info->mode;
    xyz.total_retries = 0;
    xyz.total_SOH = 0;
    xyz.total_STX = 0;
    xyz.total_CAN = 0;
#ifdef USE_YMODEM_LENGTH
    xyz.read_length = 0;
    xyz.file_length = 230*1024;
#endif
    xyz.blk = 0;

    /* Y-modem file information header */
    /* The rest of the file name data block quietly discarded */
    xyz.tx_ack = true;
    xyz.next_blk = 0;
    xyz.len = 0;
    *err = stat;
    ZM_DEBUG (zm_flush ());
    return 0;
}


int xyzModem_stream_read (char *buf, int size, int *err)
{
    int stat, total, len;
    int retries;

    total = 0;
    stat = xyzModem_cancel;
    /* Try and get 'size' bytes into the buffer */

    while (!xyz.at_eof && (size > 0))
    {
        //printf("-----1----\n");
        if (xyz.len == 0)
        {
           // printf("-----2----\n");
            retries = xyzModem_MAX_RETRIES;
            while (retries-- > 0)
            {
             //   printf("-----4----:%d\n",retries);
                stat = xyzModem_get_hdr();
                if (stat == 0)
                {
                    if (xyz.blk == xyz.next_blk)
                    {
                        xyz.tx_ack = true;
                        ZM_DEBUG (zm_dprintf
                                  ("ACK block %d (%d)\n", xyz.blk, __LINE__));
                        xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

#if defined(xyzModem_zmodem) || defined(USE_YMODEM_LENGTH)
                        if (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
#else
                            if (1)
#endif
                            {
                                /* Data blocks can be padded with ^Z (EOF) characters */
                                /* This code tries to detect and remove them */
                                if ((xyz.bufp[xyz.len - 1] == EOF) &&
                                    (xyz.bufp[xyz.len - 2] == EOF) &&
                                    (xyz.bufp[xyz.len - 3] == EOF))
                                {
                                    while (xyz.len
                                           && (xyz.bufp[xyz.len - 1] == EOF))
                                    {
                                        xyz.len--;
                                    }
                                }
                            }

#ifdef USE_YMODEM_LENGTH
                        /*
                         * See if accumulated length exceeds that of the file.
                         * If so, reduce size (i.e., cut out pad bytes)
                         * Only do this for Y-modem (and Z-modem should it ever
                         * be supported since it can fall back to Y-modem mode).
                         */
                        if (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
                        {
                            xyz.read_length += xyz.len;
                            if (xyz.read_length > xyz.file_length)
                            {
                                xyz.len -= (xyz.read_length - xyz.file_length);
                            }
                        }
#endif
                        break;
                    }
                    else if (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
                    {
                        /* Just re-ACK this so sender will get on with it */
                        CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
                        test_count ++;
                        continue;	/* Need new header */
                    }
                    else
                    {
                        stat = xyzModem_sequence;
                    }
                }
                if (stat == xyzModem_cancel)
                {
                    break;
                }
                if (stat == xyzModem_eof)
                {
                    CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
                    ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
                    if (xyz.mode == xyzModem_ymodem)
                    {
                        CYGACC_COMM_IF_PUTC (*xyz.__chan,
                                             (xyz.crc_mode ? 'C' : NAK));
                        xyz.total_retries++;
                        ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
                        stat = xyzModem_get_hdr ();
                        CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
                        ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
                    }
                    xyz.at_eof = true;
                    break;
                }
                CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
                printf("----------------------blk:%d,nexb:%d\n",xyz.blk,xyz.next_blk);
                xyz.total_retries++;
                ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
            }
            if (stat < 0)
            {
                *err = stat;
                xyz.len = -1;
                return total;
            }
        }
        /* Don't "read" data from the EOF protocol package */
        if (!xyz.at_eof)
        {
            len = xyz.len;
            if (size < len)
                len = size;
            memcpy (buf, xyz.bufp, len);
            size -= len;
            buf += len;
            total += len;
            xyz.len -= len;
            xyz.bufp += len;
        }
    }
    return total;
}

void xyzModem_stream_close (int *err)
{
    diag_printf
        ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries;test_count :%d\n",
            xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
            xyz.total_CAN, xyz.total_retries,test_count);
    ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
void xyzModem_stream_terminate (bool abort, int (*getc) (void))
{
    int c;

    if (abort)
    {
        ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
        switch (xyz.mode)
        {
        case xyzModem_xmodem:
        case xyzModem_ymodem:
            /* The X/YMODEM Spec seems to suggest that multiple CAN followed by an equal */
            /* number of Backspaces is a friendly way to get the other end to abort. */
            CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
            CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
            CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
            CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
            CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
            CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
            CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
            CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
            /* Now consume the rest of what's waiting on the line. */
            ZM_DEBUG (zm_dprintf ("Flushing serial line.\n"));
            xyzModem_flush ();
            xyz.at_eof = true;
            break;
#ifdef xyzModem_zmodem
        case xyzModem_zmodem:
            /* Might support it some day I suppose. */
#endif
            break;
        }
    }
    else
    {
        ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
        /*
            * Consume any trailing crap left in the inbuffer from
            * previous received blocks. Since very few files are an exact multiple
            * of the transfer block size, there will almost always be some gunk here.
            * If we don't eat it now, RedBoot will think the user typed it.
            */
        ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
        while ((c = (*getc) ()) > -1);
        ZM_DEBUG (zm_dprintf ("\n"));
        /*
            * Make a small delay to give terminal programs like minicom
            * time to get control again after their file transfer program
            * exits.
            */
        CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
    }
}

char *xyzModem_error (int err)
{
    switch (err)
    {
    case xyzModem_access:
        return "Can't access file";
        break;
    case xyzModem_noZmodem:
        return "Sorry, zModem not available yet";
        break;
    case xyzModem_timeout:
        return "Timed out";
        break;
    case xyzModem_eof:
        return "End of file";
        break;
    case xyzModem_cancel:
        return "Cancelled";
        break;
    case xyzModem_frame:
        return "Invalid framing";
        break;
    case xyzModem_cksum:
        return "CRC/checksum error";
        break;
    case xyzModem_sequence:
        return "Block sequence error";
        break;
    default:
        return "Unknown error";
        break;
    }
}

    /*
     * RedBoot interface
     */
#if 0				/* SB */
    GETC_IO_FUNCS (xyzModem_io, xyzModem_stream_open, xyzModem_stream_close,
                   xyzModem_stream_terminate, xyzModem_stream_read,
                   xyzModem_error);
    RedBoot_load (xmodem, xyzModem_io, false, false, xyzModem_xmodem);
    RedBoot_load (ymodem, xyzModem_io, false, false, xyzModem_ymodem);
#endif

static int getcymodem(void) {
    if (tstc())
        return (getc());
    return -1;
}

typedef struct
{
    int head;
//    unsigned char pkt[128];
    unsigned char *pkt;
    unsigned char *bufp;
    unsigned char blk,cblk,crc1,crc2;
    unsigned char alloc_l2; //0 no,1 yes
    int len, total_retries;
    int total_SOH, total_STX, total_CAN;
    unsigned long file_length,total_length;
    unsigned long store_addr;
    int start_retries;
    int data_retries;
}ym_frame;

#define START_ADDR          (0x80001000)
#define UBOOT_SAVE          (0x80600000)
#define PKT_OFFSET          (100*1024)
#define MODEM_OFFSET        (100*1024 + 128)
#define PKT_ADDR            (START_ADDR + PKT_OFFSET)
#define MODEM_ADDR          (START_ADDR + MODEM_OFFSET)
void spl_ymodem_load_image(void)
{
    int size = 0;
    int i = 0;
    int err;
    int res;
    int ret;
    char *start_headr = NULL;
    char file_name[128];
    char *p_file;
    ym_frame spl;
    connection_info_t info;
    char buf[BUF_SIZE];
    ulong store_addr = ~0;
    ulong addr = 0;
    info.mode = xyzModem_ymodem;
    ulong offset = UBOOT_SAVE;

    memcpy((char *)&spl,(char *)MODEM_ADDR,sizeof(ym_frame));//memcpy strcut ym_frame information
    memcpy(file_name,PKT_ADDR,128);//copy file name debug

    p_file = (char *)(PKT_ADDR);//get name
    while(p_file[i++]);//skip name
	parse_num ((char *)&p_file[i], &spl.file_length, NULL, " ");//get file_length

    ret = xyzModem_stream_open(&info, &err);
    if (!ret) {
        while ((res = xyzModem_stream_read(buf, BUF_SIZE, &err)) > 0) {
            store_addr = addr + offset;
            size += res;
            addr += res;
            memcpy((char *)(store_addr), buf, res);
        }
    } else {
        printf("spl: ymodem err - %s\n", xyzModem_error(err));
        hang();
    }
    if(CONFIG_UBOOT_OFFSET > spl.total_length ){
        start_headr = offset + (CONFIG_UBOOT_OFFSET - spl.total_length);
        memcpy((char*)(CONFIG_SYS_TEXT_BASE),(char*)(start_headr),size);
    } else{
        memcpy((char*)(CONFIG_SYS_TEXT_BASE),(char *)(START_ADDR+CONFIG_UBOOT_OFFSET),spl.total_length-CONFIG_UBOOT_OFFSET);
        memcpy((char*)(CONFIG_SYS_TEXT_BASE+ spl.total_length - CONFIG_UBOOT_OFFSET),UBOOT_SAVE,size);
        start_headr = CONFIG_SYS_TEXT_BASE;
    }
    if(start_headr != NULL)
        spl_parse_image_header((struct image_header *)start_headr);

    xyzModem_stream_close(&err);
    xyzModem_stream_terminate(false, &getcymodem);

    printf("[%s] :Loaded %d bytes\n",file_name,spl.total_length+size);
#ifdef UART_YMODEM_DEBUG
    printf("----------------dump--------------\n");
    printf("start_headr=%p\n",start_headr);
    printf("spl_image.size=%d\n",spl_image.size);
    printf("[Bootrom]total_retries: %d\n",spl.total_retries);
    printf("[Bootrom]total_SOH:     %d\n",spl.total_SOH);
    printf("[Bootrom]total_STX:     %d\n",spl.total_STX);
    printf("[Bootrom]total_CAN:     %d\n",spl.total_CAN);
    printf("[Bootrom]file_length:   %d\n",spl.file_length);
    printf("[Bootrom]total_length:  %d\n",spl.total_length);
    printf("[Bootrom]store_addr:    0x%x\n",spl.store_addr);
    printf("[Bootrom]start_retries: %d\n",spl.start_retries);
    printf("[Bootrom]data_retries:  %d\n",spl.data_retries);
    printf("----------------dump--------------\n");
#endif
    flush_cache_all();
}
