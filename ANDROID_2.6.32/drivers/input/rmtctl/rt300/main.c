#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ir_api.h"


#define  DEBUG
#ifdef   DEBUG
#define  ir_dbg(fmt, args...) printf("[%s]_%d: " fmt, __func__ , __LINE__, ## args)
#define  ir_trace()           printf("trace in %s %d\n", __func__, __LINE__);
#else
#define  ir_dbg(fmt, args...)
#define  ir_trace()
#endif


static int version = 0;
static int clear = 0;
static int stop = 0;
static char *device = "/dev/tts/USB0";
static unsigned short code_num = 0x0000;
static unsigned char key_id  = 0x00;
static unsigned char dev_id = 0x01;
static unsigned char code_src = 0x01; 
static unsigned char location = 0;
static unsigned char type = 0x80;
static unsigned char prog = 0x00;
static unsigned char learning = 0x00;
static unsigned char learned = 0x00;

static void pabort(const char *s)
{
    perror(s);
    abort();
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-dvcspeliktrh] <SparkSun, SparkSun@viatech.com.cn>\n", prog);
	
    puts(" -d --device       device to use (default /dev/tts/USB0)\n"
         " -v --version      get firmware version\n"
	     " -c --clear        master clear test\n"
         " -s --stop         stop IR transmission\n"
         " -p --prog         transmit preprogrammed IR code\n"
	     " -e --learned      transmit learned IR code\n"
	     " -l --learn        learning a IR code\n"
         " -i --id           device id\n"
         " -k --key          key id\n"
         " -t --type         transmission type(1 single, 2 continuous)\n"
         " -r --src          code source\n"
         " -h --help         help\n");

	exit(1);
}

static void parse_opts(int argc, char **argv)
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",         1, 0, 'd' },
            { "version",        0, 0, 'v' },
			{ "clear",          0, 0, 'c' },
            { "stop",           0, 0, 's' },
            { "prog",           1, 0, 'p' },
			{ "learned",        1, 0, 'e' },
			{ "learn",          1, 0, 'l' },
			{ "id",             1, 0, 'i' },
			{ "key",            1, 0, 'k' },
			{ "type",           1, 0, 't' },
            { "src",            1, 0, 'r' },
            { "help",           0, 0, 'h' },
			{ NULL,             0, 0, 0   },
		};
        int t = 0;
		int c = getopt_long(argc, argv, "d:vcsp:e:l:i:k:t:r:h", lopts, NULL);

        if (c == -1)
            break;

        switch (c) {
        case 'd':
            device  = optarg;
            break;
        case 'v':
            version = 1;
            break;
        case 'c':
            clear = 1;
            break;
        case 's':
            stop = 1;
            break;
        case 'p':
            prog = 1;
            code_num = atoi(optarg);
            break;
        case 'e':
            learned = 1;
            location = atoi(optarg);
            break;
        case 'l':
            learning = 1;
            location = atoi(optarg);
            break;
        case 'i':
            dev_id = atoi(optarg);
            ir_dbg("device id is %d\n", dev_id);
            break;
        case 'k':
            key_id = atoi(optarg);
            ir_dbg("key id is 0x%0x\n", key_id);
            break;
        case 't':
            t = atoi(optarg);
            if (t == 1)
                type = 0x00;
            else
                type = 0x08;
            break;
        case 'r':
            code_src = atoi(optarg);
            break;
        case 'h':
        default:
            print_usage(argv[0]);
            break;
        }
    }
}

static int get_version_test(int fd)
{
    int ret = 0;
    unsigned int v   = 0;
    BYTE len = 0;
    
    ret = IrGetVersion(fd, (BYTE *)&v, &len);
    if (ret)
        pabort("get version test failed");

    return v;
}

static int send_stop_command(int fd)
{
    return IrTransmitStop(fd);
}

static int learned_code_test(int fd, unsigned char addr, int type)
{   
    /* codenum is learned location, codesrc is 0x04,
     *  keyid is 0x00, no need 
     *  devicetype is 0x00, no need */
   return IrTransmitCode(fd, addr, 0x00, 0x04, type * 0x10 + 1, 0x00);
}


int main(int argc, char **argv)
{
    int fd  = 0;
    int ret = 0;
    
	parse_opts(argc, argv);
    /* open usb serial */
    fd = serial_open(device, 9600);
    if (fd < 0)
        pabort("can not open device");
    
    /* get firmware version information */
    if (version)
        printf("firmware version is 0x%08x\n", get_version_test(fd));

    /* send stop command */
    if (stop) {
        ret = send_stop_command(fd);
        if (ret)
            pabort("send stop command failed");
    }

    /* transmit preprogrammed command */
    if (prog) {
        ret = IrTransmitCode(fd, code_num, key_id, code_src, type, dev_id);
        if (ret)
            pabort("transmit preprogrammed code failed");
    }

    /* master clear */
    if (clear) {
        ret = IrMasterClear(fd);
        if (ret)
            pabort("master clear failed");
    }

    /* learning function test */
    if (learning) {
        ret = IrLearnCode(fd, location);
        if (ret)
            pabort("learning function test failed");
    }

    /* learned code test */
    if (learned) {
        ret = learned_code_test(fd, location, type);
        if (ret)
            pabort("learned code test failed");
    }


    /* test finish, transmit stop cmd and close */
    IrTransmitStop(fd);
    serial_close(fd);
    return 0;
}
