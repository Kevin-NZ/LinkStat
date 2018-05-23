
/****************************************************************************\
|*                                                                          *|
|* File          : linkstat.c  --  link status daemon                       *|
|* Version       : 2.0.0                                                    *|
|*                                                                          *|
|* Created On    : Sat Feb 21 11:12:32 NZDT 1998                            *|
|* Author        : Kevin Clark                                              *|
|*                                                                          *|
|* Last Modifier : Kevin Clark                                              *|
|* Modified On   : Sat Aug  7 13:11:44 NZDT 2010                            *|
|* Mod Count     : 17                                                       *|
|* Status        : TESTED                                                   *|
|*                                                                          *|
|*                                                                          *|
|* Purpose:                                                                 *|
|*                                                                          *|
|*     To provide a quick accurate method of monitoring system uptime.      *|
|*     Using this program it is possible to get an immediate update when    *|
|*     a system goes down.  It is also possible to calculate the uptime     *|
|*     of a system and track when systems (WinNT) crash and automatically   *|
|*     reboot.                                                              *|
|*                                                                          *|
|*                                                                          *|
|* Method:                                                                  *|
|*                                                                          *|
|*     When started each host listed is processed one at a time and sent    *|
|*     an ICMP packet followed by a 10ms pause (configured through the      *|
|*     "interval" parameter).  If at the end of this process we are still   *|
|*     waiting for some ICMP packets to return then we increase the         *|
|*     pause interval by 2ms.  This will give each ICMP packet a little     *|
|*     longer to return.  If after ten consecutive cycles we are not        *|
|*     waiting for any packets then the pause interval starts to decrement  *|
|*     by one for each ten cycles (for example if after 30 cycles we are    *|
|*     still not waiting on any packets, then the pause interval will be    *|
|*     decremented by 3ms during each cycle).  This process stops when      *|
|*     either we hit the minimum interval (default 10ms) or we encounter    *|
|*     an outstanding packet.                                               *|
|*                                                                          *|
|*     At the end of each cycle we wait 1000ms (configured through the      *|
|*     "timeout" parameter) for any outstanding packets.  This also has     *|
|*     effect of pausing for at lease a second between cycles.              *|
|*                                                                          *|
|*     If a packet is not received from a host after 3 consecutive cycles   *|
|*     (configured through the "retry" parameter) then the host is          *|
|*     considered to be "down".  A host is considered to be "up" as soon    *|
|*     as any packet is received from it.                                   *|
|*                                                                          *|
|*                                                                          *|
|* Command Line Options:                                                    *|
|*                                                                          *|
|*     -help            display information                                 *|
|*     -version         display version information                         *|
|*     -timeout #       delay between polls (default 1000 msecs)            *|
|*     -interval #      delay between packets (default 10 msecs)            *|
|*     -retry #         number of retries to a host (default 3)             *|
|*     -update #        frequency of statistical updates (default 300 secs) *|
|*     -notify command  command to run during state changes                 *|
|*     -slarep #        delay (sec) before SLA Report is run (default 5pm)  *|
|*     -file file       file to read list of hosts                          *|
|*     -log file        file to log output when detached from terminal      *|
|*     -mac_check       check the MAC address of the returned packets       *|
|*                                                                          *|
|*                                                                          *|
|* Notes:                                                                   *|
|*                                                                          *|
|*     This should cause minimal network load as we should only have one    *|
|*     ICMP datagram out on the network at any one time.  These datagrams   *|
|*     are at low priority and will only be to hosts that are directly      *|
|*     connected (or near) to the source.                                   *|
|*                                                                          *|
|*     If started with defaults, each host listed will be sent an ICMP      *|
|*     packet (one at a time), then there will be at least a 1 second       *|
|*     pause, at which time the whole process is started again.             *|
|*                                                                          *|
|*     A status message is produced after the first 5 seconds, and then     *|
|*     at 5 minute intervals.                                               *|
|*                                                                          *|
|*     It is possible to have a command executed when a host changes        *|
|*     state.  Using this it is possible to double check all state changes  *|
|*     by attempting to attach (or telnet) to that particular host.         *|
|*     This command is configured through the "notify" parameter and is     *|
|*     passed three command line arguments.  They are the hostname, the     *|
|*     current state and a message description (detailing time, host and    *|
|*     state change).                                                       *|
|*                                                                          *|
|*     Many of the parameters are configurable on the command line.         *|
|*     The minimum value for the interval parameter is 5ms, and 500ms for   *|
|*     the timeout parameter.                                               *|
|*                                                                          *|
|*     While other methods of listing the hosts to check are available,     *|
|*     such as command line and standard input, the only one that has full  *|
|*     support is the "hosts" file specification (-f option).               *|
|*     Lines in this file have the following format:                        *|
|*                                                                          *|
|*        <IP Address> <Hostname> # (int=<freq>,ret=<num>,mon=<HHMM:hhmm>)  *|
|*                                                                          *|
|*     While the items after the "#" are optional, the currently supported  *|
|*     options are as follows:                                              *|
|*        int=<frequency> - Specifies how often to check the host (secs)    *|
|*        ret=<number>    - Specifies the max number of packet retransmits  *|
|*        mon=<HHMM:hhmm> - Monitor between the hours of HHMM and hhmm      *|
|*                                                                          *|
|*     A description of the lines recorded in the logfile are as follows:   *|
|*     1/ <host> is unreachable, after <time>                               *|
|*          This reports that the host is no longer contactable. There may  *|
|*          also be displayed the time that the host was previously         *|
|*          available (ie uptime)                                           *|
|*     2/ <host> is alive, after <time>                                     *|
|*          This reports that the host is now responding.  There may also   *|
|*          be displayed the time that the host was previously unavailable  *|
|*          (ie downtime)                                                   *|
|*     3/ Waiting on <x> (<y> unreachable), I:<i> R:<r> C:<c> M:<m>         *|
|*          This is a status message showing that we are currently waiting  *|
|*          on X number of local hosts to respond with Y local hosts        *|
|*          currently unreachable.  It also reports the current Interval    *|
|*          value.  The R value is the optimal retry count (this will be at *|
|*          its maximum when a host has gone down).  The C parameter shows  *|
|*          how many cycles the process has gone through since last         *|
|*          displaying this message.  The M parameter indicates how many    *|
|*          hardware (MAC) addresses are being checked.                     *|
|*                                                                          *|
|*                                                                          *|
|* Possible Improvements:                                                   *|
|*                                                                          *|
|*     Currently there is no maximum value for the interval value.          *|
|*     Currently there is no formal structure for the input hosts file.     *|
|*     Configuration file reload support has been removed/replaced with     *|
|*     a SLA report and exit.  Should be re-enabled in the future, problems *|
|*     with structure re-initialization while still keeping the current     *|
|*     statistics.                                                          *|
|*     The program has a predefined maximum number of hosts (MAX_HOSTS).    *|
|*     This could be changed to be a runtime malloc so as to be a little    *|
|*     more efficient on memory resources, as well as handling much larger  *|
|*     lists.                                                               *|
|*     Need to get a little smarter in the parsing of the input hosts       *|
|*     file as it is currently fixed format (eg int=x,ret=x,mon=x).         *|
|*                                                                          *|
|*                                                                          *|
|* Caveats                                                                  *|
|*                                                                          *|
|*     You should be aware that a hardware NIC can respond to ICMP pings    *|
|*     even if the operating system is hung.  A semi-bad network connection *|
|*     (eg 50% packet loss) may not be picked up by this method due to some *|
|*     packets still being returned.                                        *|
|*                                                                          *|
|*                                                                          *|
|* CPU Utilization:                                                         *|
|*                                                                          *|
|*     CPU Utilization should be < 1.20% for 110 hosts                      *|
|*     CPU Utilization should be < 1.50% for 195 hosts                      *|
|*     CPU Time Should be approx 1:10 (8am-5pm for 110 hosts)               *|
|*     CPU Time Should be approx 2:05 (8am-5pm for 195 hosts)               *|
|*                                                                          *|
|*                                                                          *|
|* History:                                                                 *|
|*                                                                          *|
|*   1.0.0  21-FEB-98  Original version                                     *|
|*   1.1.0  24-FEB-98  Added notification command                           *|
|*   1.2.0  06-SEP-98  Added configuration file reload support (SIGHUP)     *|
|*   1.3.0  21-OCT-98  Added WinCenter services/downtime hack (see WC_MOD)  *|
|*   1.3.1  30-OCT-98  Check that the WinCenter status file was created     *|
|*   1.3.2  07-JAN-99  Fixed statistics reporting (for WinCenter hack)      *|
|*   1.3.3  16-MAR-99  Added DBUG3 to diagnose a problem                    *|
|*   1.3.4  30-MAR-99  Fixed above problem (offset var not being reset)     *|
|*   1.4.0  16-JUL-99  Ported code to RedHat Linux 6.0                      *|
|*   1.5.0  22-SEP-99  Added a "Year" component to the log files            *|
|*   1.6.0  03-NOV-99  Added variable packet frequency (int= option)        *|
|*   1.6.1  04-MAY-00  Altered interval adjustment algorithm                *|
|*   1.6.2  27-JUL-00  Made big changes to response detection               *|
|*   1.6.3  04-AUG-00  Added variable packet retries (ret= option)          *|
|*   1.7.0  13-Mar-02  Added option to check the MAC address of the source  *|
|*   1.7.1  07-Nov-02  Removed WinCenter hacks (by using #ifdef WC_MOD)     *|
|*   1.8.0  01-Jul-03  Altered code to query multiple arp caches (linux)    *|
|*   1.9.0  10-Jan-06  Added support for time filtering (mon= option)       *|
|*   1.9.1  27-Feb-09  Clean up some compiler warnings                      *|
|*   2.0.0  07-Aug-10  Support for System/390 (s390x)                       *|
|*                                                                          *|
\****************************************************************************/

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

#include <sys/stat.h>
#include <netdb.h>
#include <time.h>

#ifdef CHECK_MAC_ADDR
/* Stuff for ARP Requests */
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#endif

#include <getopt.h>
#include "version.h"

/* externals */

extern char *optarg;
extern int optind,opterr;
#ifdef hpux
extern char *sys_errlist[];
extern int sys_nerr;
#endif

int ident; 
int sock;

int adjusting=0;
int queue_len=0;

time_t start_time;
time_t baseline;
time_t report_time;

/* constants */

#define DEFAULT_INTERVAL  10  /* default time between packets (msec) */
#define DEFAULT_TIMEOUT 1000  /* individual host timeouts (msec) */
#define DEFAULT_RETRY      3  /* number of times to retry a host */
#define DEFAULT_UPDATE   300  /* update stats every 5 minutes */

#define MAX_HOSTS       1000  /* maximum number of hosts */

#define NOTIFY_LIMIT      10  /* limit notifications within 30s */

#define MIN_INTERVAL       5
#define MIN_TIMEOUT      500

/* global variables/flags */

int  retry         = DEFAULT_RETRY;
int  timeout       = DEFAULT_TIMEOUT;
int  interval      = DEFAULT_INTERVAL;
int  min_interval  = DEFAULT_INTERVAL;
int  update        = DEFAULT_UPDATE;
int  check_hw      = 0;
long slarep        = 0;
int  debug         = 0; 
int  optimal_retry = 0;
int  macs_checked  = 0;

char  *command;               /* command to run during state changes */
struct timeval current_time;  /* current time (pseudo) */
struct timezone tz;

/* entry used to keep track of each host we are pinging */
typedef struct host_entry {
  char               *host;             /* text description of host */
  struct sockaddr_in  saddr;            /* internet address */
  unsigned char      *mac_addr;         /* hardware address */
  int                 retry;            /* maximum retries allowed */
  int                 response;         /* host has responded / retry count */
  short               alive;            /* host state, 1=up, 0=down */
  int                 i;                /* index into array */
  int                 packet_schedule;  /* secs between packets to this host */

  struct timeval      first_time;       /* time of first packet received */
  struct timeval      last_time;        /* time of last packet received */
  struct timeval      next_time;        /* time to send next packet */

  short               monitor_from;     /* monitor this host from this time */
  short               monitor_until;    /* monitor this host until this time */

  long int            downtime;         /* seconds spent unavailable */
  int                 downtime_cnt;     /* number of times unavailable */
} HOST_ENTRY;

HOST_ENTRY *table[MAX_HOSTS];

int num_hosts=0;
int num_local_hosts=0;
int num_local_unreachable=0;

static void
detachFromTTY(log_file)
char *log_file;
{
  int fd;
  
  if ((fd = open(log_file, O_WRONLY | O_CREAT | O_APPEND)) == -1) {
    perror("detachFromTTY: Unable to open log_file");
    return;
  }
  (void) dup2(fd, STDOUT_FILENO);
  (void) dup2(fd, STDERR_FILENO);
  
  if ((fd = open("/dev/null", O_RDONLY)) == -1) {
    perror("detachFromTTY: Unable to open /dev/null");
    return;
  }
  (void) dup2(fd, STDIN_FILENO);
  
  for (fd = 0; fd < _SC_OPEN_MAX; fd++)
    if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO)
      (void) close(fd);
  
  switch (fork()) {
    case 0:   break;          /* child, continue */
    case -1:  perror("detachFromTTY: Unable to fork process");
              exit(1);        /* error */
    default:  exit(0);        /* parent, terminate */
  }
  (void) setsid();
}

static char *
curr_time()
{
    static char buf[25];

    static time_t sys_clock;
    struct tm *the_time;

    sys_clock = time(NULL);
    the_time = localtime(&sys_clock);

    if (the_time != NULL) {
        /* FORMAT DDD MMM DD HH:MM:SS YYYY */
        if (strftime(buf, 25, "%a %h %e %H:%M:%S %Y", the_time) == 0)
	    strcpy(buf, "n/a");
    } else
      strcpy(buf, "n/a");

    return (char *)buf;
}

char *
timeval_diff(stamp,current)
struct timeval stamp, current;
{
  static char buf[20],days[20],hours[20],mins[20],secs[20];
  struct timeval temp_current,temp_stamp,diff;
  unsigned long seconds;  
  
  temp_current= current;
  temp_stamp =  stamp;
  
  if (temp_current.tv_usec < temp_stamp.tv_usec ) {
    temp_current.tv_usec += 1000000;
    temp_current.tv_sec--;
  }
  
  diff.tv_usec = temp_current.tv_usec - temp_stamp.tv_usec;
  diff.tv_sec  = temp_current.tv_sec  - temp_stamp.tv_sec;

  days[0] =  '\0';
  hours[0] = '\0';
  mins[0] =  '\0';
  secs[0] =  '\0';

  seconds = diff.tv_sec;

  if (seconds >= 86400) {
    sprintf(days, "%lud", seconds / 86400);
    seconds %= 86400;
  }
  if (seconds >= 3600) {
    sprintf(hours, "%luh", seconds / 3600);
    seconds %= 3600;
  }
  if (seconds >= 60) {
    sprintf(mins, "%lum", seconds / 60);
    seconds %= 60;
  }
  sprintf(secs, "%lus", seconds);

  snprintf(buf, 20, "%s%s%s%s (%fs)", days,hours,mins,secs,(double)diff.tv_sec + ((double)diff.tv_usec / (double)1000000));
  return (char *)buf;
}

void errno_crash_and_burn(message)
char *message;
{
  fprintf(stderr,"%s Error : %s - %s\n",curr_time(),message,strerror(errno));
  
  /* Depreciated:
  if (errno < sys_nerr && errno >= 0)
    fprintf(stderr,"%s Error : %s - %s\n",curr_time(),message,sys_errlist[errno]);
  else
    fprintf(stderr,"%s Error : %s\n",curr_time(),message);
  */

  exit(4);
}

void crash_and_burn(message)
char *message;
{
  fprintf(stderr,"%s Error : %s\n",curr_time(),message);
  exit(4);
}

HOST_ENTRY *
create_host_entry(host,ip_addr,packet_schedule,uniq_retry,from,until)
char *host, *ip_addr;
int packet_schedule, uniq_retry;
short from, until;
{
  HOST_ENTRY *p;
  struct hostent *host_ent;
  struct in_addr *host_add;

  host_add = (struct in_addr *) malloc(sizeof(struct in_addr));
  if (!host_add) crash_and_burn("create_host_entry: can't allocate host_add");

  if (ip_addr != NULL) {
    if (inet_aton(ip_addr, host_add) == 0) {
      fprintf(stderr,"problem resolving %s\n",ip_addr);
      free(host_add);
      return NULL;
    }
  } else {
    /* need to modify this as gethostbyname returns static, so
       should copy info to ipaddress variable, and throw other
       stuff away - see shepperd-1.0
    */
    if (inet_aton(host, host_add) == 0) {
      free(host_add);
      if (((host_ent = gethostbyname(host)) == NULL) ||
          ((host_add = (struct in_addr *) *(host_ent->h_addr_list))==NULL)
        ) {
        fprintf(stderr,"%s address not found\n",host);
        /* can't free host_add as gethostbyname returns static, so
           any subsequent call to gethostbyname will produce a segfault

           free(host_add);
        */
        return NULL;
      }
    }
  }

  p = (HOST_ENTRY *) malloc(sizeof(HOST_ENTRY));
  if (!p) crash_and_burn("create_host_entry: can't allocate HOST_ENTRY");

  p->host=host;

  /* Interval between pkts to this host (in seconds), 0=every cycle */
  p->packet_schedule    = packet_schedule;

  /* Set the number of retries for this particular host */
  p->retry    = uniq_retry;

  p->first_time.tv_sec  =0;    /* Used to determine uptime */
  p->first_time.tv_usec =0;

  p->last_time.tv_sec   =0;    /* Used to determine downtime */
  p->last_time.tv_usec  =0;

  p->next_time.tv_sec   =0;    /* Used to alter packet frequency */
  p->next_time.tv_usec  =0;

  p->downtime =0;              /* Used for reporting SLA's */
  p->downtime_cnt =0;

  memset((char*) &p->saddr, 0, sizeof(p->saddr));
  p->saddr.sin_family      = AF_INET;

  p->saddr.sin_addr = *host_add; 

  p->mac_addr = NULL;

  p->monitor_from = from;
  p->monitor_until = until;

  return p;
}

u_short in_cksum(p,n)
u_short *p; int n;
{
  register u_short answer;
  register u_long sum = 0;
  u_short odd_byte = 0;

  while( n > 1 )  { sum += *p++; n -= 2; }

  /* mop up an odd byte, if necessary */
  if( n == 1 ) {
      *(u_char *)(&odd_byte) = *(u_char *)p;
      sum += odd_byte;
  }

  sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
  sum += (sum >> 16);			/* add carry */
  answer = (u_short) ~sum;		/* ones-complement, truncate */
  return (answer);
}

void notify_command(host, state, msg)
char *host,*state,*msg;
{
    static char   cmd[1024];

    static time_t last  = 0;  /* Check that we do not flood a whole */
    static int    count = 0;  /* heap of messages onto the network  */

    if ((time(NULL) - last) > 30) {
        if (count > NOTIFY_LIMIT) {
	    printf("%s Overload reset... Notifications enabled\n",curr_time());
            (void) fflush(stdout);
        }
	count = 0; /* reset */
    }

    count++;

    if (count <= NOTIFY_LIMIT) {
        /* create command string */
        snprintf(cmd,1024,"%s \"%s\" \"%s\" \"%s\">/dev/null 2>&1 &",command,host,state,msg);
        (void) system(cmd);
    } else if (count == (NOTIFY_LIMIT+1)) {
	/* We received more than "NOTIFY_LIMIT" notifications less than 30s apart */
        printf("%s Overload... Notifications disabled\n",curr_time()); (void) fflush(stdout);
        snprintf(cmd,1024,"%s \"OVERLOAD\" \"n/a\" \"Too many state changes being logged\">/dev/null 2>&1 &",command);
        (void) system(cmd);
    }

    last = time(NULL);
}

void send_ping(s,h)
int s; HOST_ENTRY *h;
{
  static char  buffer[32];
  static int   glitch = 0;
  struct icmp *icp = (struct icmp *) buffer;
  int n;

  icp->icmp_type = ICMP_ECHO;
  icp->icmp_code = 0;
  icp->icmp_cksum = 0;
  icp->icmp_seq = h->i;
  icp->icmp_id = ident;
  icp->icmp_cksum = in_cksum( (u_short *)icp, 32 );

  n = sendto( s, buffer, 32, 0, (struct sockaddr *)&h->saddr, sizeof(struct sockaddr_in) );

  if ( n < 0 || n != 32 ) {
    /* Might be a little nicer here an allow the occasional glitch
     * due to periods where firewall/iptable rules are being updated
     * or reloaded
     */
    if (glitch++) errno_crash_and_burn("send_ping: sendto");

    fprintf(stderr,"%s Glitch? : send_ping: sendto - %s\n",curr_time(),strerror(errno));
    sleep(1);
  } else
    glitch=0; 
}

int recvfrom_wto (s,buf,len, saddr, timo)
int s; char *buf; int len; struct sockaddr *saddr; int timo;
{
  int nfound,n;
  socklen_t slen;
  struct timeval to;
  fd_set readset,writeset;

  to.tv_sec  = timo/1000;
  to.tv_usec = (timo - (to.tv_sec*1000))*1000;

  FD_ZERO(&readset);
  FD_ZERO(&writeset);
  FD_SET(s,&readset);
  nfound = select(s+1,&readset,&writeset,NULL,&to);
  if (nfound<0) errno_crash_and_burn("send_ping: select");
  if (nfound==0) return -1;  /* timeout */
  slen=sizeof(struct sockaddr);
  n=recvfrom(s,buf,len,0,saddr,&slen);
  if (n<0) errno_crash_and_burn("send_ping: recvfrom");
  return n;
}

#ifdef X_WIP_ALLOW_STATUS_FILE

int provide_status ()
{
  int fd;
  char buf[30];

  snprintf(buf, 30, "testCCCCCC\n");

  if ((fd = open("/tmp/pipe", O_NONBLOCK | O_WRONLY)) == -1)
    return -1;

  /*
     If here then the fifo exists and somebody is wanting
     to read from it...  or it is a regular file, which we
     should check for before running this, or use mkfifo
     to create the requested status pipe.
  */
  if (write(fd, buf, strlen(buf)) < 1)
    return -1;
  close(fd);
  return 0;
}
#else
#define provide_status()        ((void)0)
#endif

#ifdef CHECK_MAC_ADDR

char *get_ip_from_long(ipaddress)
u_long ipaddress;
{
    static char buf[20];
    int a,b,c,d;
   
    #ifdef linux
    /* Little-Endian */
    d = ipaddress / 16777216; ipaddress -= d * 16777216;
    c = ipaddress / 65536;    ipaddress -= c * 65536;
    b = ipaddress / 256;      ipaddress -= b * 256;
    a = ipaddress;
    #else
    /* Big-Endian */
    a = ipaddress / 16777216; ipaddress -= a * 16777216;
    b = ipaddress / 65536;    ipaddress -= b * 65536;
    c = ipaddress / 256;      ipaddress -= c * 256;
    d = ipaddress;
    #endif

    if ((a<257)&(b<257)&(c<257)&(d<257))
      snprintf(buf, 20, "%d.%d.%d.%d",a,b,c,d);
    else
      snprintf(buf, 20, "Invalid Address");

    return buf;
}

char *get_mac(s, ipaddress)
int s; u_long ipaddress;
{
  static struct arpreq ar;
  struct sockaddr_in *sin;

#ifdef linux
  static char ifbuffer[sizeof(struct ifreq) * 64];
  static struct ifconf ifc;
  struct ifreq *ifr;
  int offset;
#endif

  bzero((caddr_t)&ar, sizeof(ar));  /* ALT: memset(&ar, '\0', sizeof(ar)) */
  ar.arp_pa.sa_family = AF_INET;
  sin = (struct sockaddr_in *) &ar.arp_pa;
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = ipaddress;
  sin->sin_port = 0;

#ifdef linux
  /* Linux holds a different arp cache for each interface (must use ar.arp_dev) */

  /*
   * Get a list of interface names
   */
  ifc.ifc_len = (int) sizeof(ifbuffer);
  ifc.ifc_buf = ifbuffer;
  if (ioctl(s, SIOCGIFCONF, &ifc) < 0) {return NULL;}
  if (ifc.ifc_len > (int) sizeof(ifbuffer)) {
    printf("%s ERROR: Interface list too long - %d\n", curr_time(), ifc.ifc_len); (void) fflush(stdout);
    return NULL;
  }

  /*
   * Attempt an ARP lookup on each interface
   */
  offset = 0;
  while (offset < ifc.ifc_len) {
    ifr = (struct ifreq *) (ifbuffer + offset);
    offset += sizeof(*ifr);

    /* Skip loopback and aliased interfaces */
    if (strncmp(ifr->ifr_name, "lo", 2) == 0) continue;
    if (strchr(ifr->ifr_name, ':') != NULL)   continue;

    /* Set up the device entry for the ARP lookup */
    strncpy(ar.arp_dev, ifr->ifr_name, sizeof(ar.arp_dev) - 1);
    ar.arp_dev[sizeof(ar.arp_dev) - 1] = '\0';

    /* Query ARP table */
    if (ioctl(s, SIOCGARP, &ar) >= 0) {
      /* Skip non-ethernet interfaces? */
      if (ar.arp_ha.sa_family != ARPHRD_ETHER) {
        printf("%s ERROR: MAC found on non-ethernet interface, IP address=%s, Int=%s\n", curr_time(),get_ip_from_long(ipaddress), ar.arp_dev); (void) fflush(stdout);
        continue;
      }
      break;
    } else {
      if (offset >= ifc.ifc_len) {
        /* This was the last interface to try */
        return NULL;
      }
    }
  }
#else
  /* many systems keep a common arp cache */
  if (ioctl(s, SIOCGARP, (caddr_t)&ar) < 0) {
    /* no arp address was found... most likely this
       is a system that is not on our subnet */
    return NULL;
  }
#endif

  /* Check the address is complete */
  if (!(ar.arp_flags & ATF_COM)) {
    printf("%s ERROR: Incomplete MAC address, IP address=%s\n", curr_time(),get_ip_from_long(ipaddress)); (void) fflush(stdout);
    return NULL;
  }

  return ar.arp_ha.sa_data;
}

int check_arp (s, ipaddress, n)
int s; u_long ipaddress; int n;
{
  char *mac;

  if ((mac=get_mac(s, ipaddress)) == NULL) {
    return 0;
  }
 
  if (table[n]->mac_addr == NULL) {
    table[n]->mac_addr = (unsigned char*) malloc(14);
    if (!table[n]->mac_addr) crash_and_burn("check_arp: can't malloc MAC address");
    memcpy(table[n]->mac_addr,mac,14);
    macs_checked++;
  }

  if (memcmp(mac, table[n]->mac_addr, 14) != 0) {
    /* ODD... Big/Little Endian does not seem to be a problem here... */
    printf("%s NIDS WARNING received a packet from %02x:%02x:%02x:%02x:%02x:%02x",curr_time(),mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    printf(" rather than the expected %02x:%02x:%02x:%02x:%02x:%02x (%s)\n",table[n]->mac_addr[0],table[n]->mac_addr[1],table[n]->mac_addr[2],table[n]->mac_addr[3],table[n]->mac_addr[4],table[n]->mac_addr[5],get_ip_from_long(ipaddress));
    memcpy(table[n]->mac_addr,mac,14);
    if (command) notify_command(table[n]->host, "nids", "MAC address changed");
    return 2;
  }

  return 0;
}

#else
#define check_arp(s, ipaddress, index)        ((void)0)
#endif

char *get_host_by_address(in)
struct in_addr in;
{
  struct hostent *h;

  h=gethostbyaddr((char *) &in,sizeof(struct in_addr),AF_INET);
  if (h==NULL || h->h_name==NULL) return inet_ntoa(in);
  else return h->h_name;
}

int wait_for_reply(s, wait_time)
int s, wait_time;
{
  int result;
  static char buffer[4096];
  struct sockaddr_in response_addr;
  struct ip *ip;
  int hlen;
  struct icmp *icp;
  int n;
  
  result=recvfrom_wto(s,buffer,4096,
		      (struct sockaddr *)&response_addr,wait_time);

  if (result<0) { return 0; } /* timeout */
  
  ip = (struct ip *) buffer;
  hlen = ip->ip_hl << 2;
  if (result < hlen+ICMP_MINLEN) { return(1); /* too short */ }

  icp = (struct icmp *)(buffer + hlen);
  
  if (
      ( icp->icmp_type != ICMP_ECHOREPLY ) ||
      ( icp->icmp_id   != ident          )
      ) {
    /*
     * This will happen if we use the host that is running
     * linkstat to ping other hosts on the network.
     */
    /*printf("%s Hmm... Not one of our packets (src=%s)\n", curr_time(), get_host_by_address(response_addr.sin_addr)); (void) fflush(stdout);*/
    return 1; /* packet received, but not the one we are looking for! */
  }

  /*
   * Get the index into our table
   */
  n=icp->icmp_seq;
  
  /*
   * Better check that the index is within the boundaries
   */
  if ((n < 0) || (n >= num_hosts)) {
    printf("%s ERROR: Invalid packet, index=%d (src=%s)\n", curr_time(),n,get_host_by_address(response_addr.sin_addr)); (void) fflush(stdout);
    return 1; /* Corruption */
  }

  /*
   * Check that the Source IP Address is as we expect it, this
   * pretty much ensures that this is a packet sent by this
   * process... and not by something else running on this box.
   */
  if (table[n]->saddr.sin_addr.s_addr != response_addr.sin_addr.s_addr) {
    printf("%s ERROR: Invalid packet, index=%d, src=%s (exp=%s)\n", curr_time(),n,get_host_by_address(response_addr.sin_addr),get_host_by_address(table[n]->saddr.sin_addr)); (void) fflush(stdout);
    return 1; /* Corruption */
  }

  if (check_hw) {
    /*
     * Check that the Hardware address of the source is as expected.
     * This is a simple attempt at finding duplicate addresses, as
     * well as checking for arp address poisoning (used with man-in-
     * the-middle attacks).
     */
    check_arp(s, response_addr.sin_addr.s_addr, n);
  }

  /*
   * For the hosts that are using the default number of retries:
   * Try work out an optimal retry count, then reset the value.
   *
   * NOTE: While it is not a biggie... The retry count will be
   *       one greater than it needs to be, as we are using this
   *       field for more than one purpose to keep things simple.
   *       We would need to have another field for the retry
   *       count if we were to fix this up.
   */
   if ((table[n]->retry == retry) && (optimal_retry < retry - table[n]->response))
    optimal_retry = retry - table[n]->response;

  table[n]->response = table[n]->retry;

  if (!table[n]->alive) {
    /* At this point the host has only just come up
       after being down for a period of time
     */

    static char msg[255];

    if (table[n]->packet_schedule == 0)
      num_local_unreachable--;

    /* timestamp the last time the host responded */
    gettimeofday(&current_time,&tz);

/* Removed as MetaFrame application servers are better :) */
#ifdef WC_MOD
    /* WC_MOD: This is a mod to correct WinCenter downtime
     *         It is a little out of the scope of this utility
     *         but never the less there was a need for it  :)
     *
     *         This is required as the server may still be responding
     *         to pings, but it may have services that have hung.
     *         We have an external process that check the services
     *         and creates a file in .../log/status if a server has
     *         services that have hung.  This information is used
     *         to "adjust" the downtime of the server.
     *
    */
    if (((table[n]->host)[1] == 'i') || ((table[n]->host)[1] == 'M')) {
      struct stat buf;
      /* This might be a WinCenter server that had hung services */
      snprintf(msg, 255, "/opt/LinkStat/log/status/%s", table[n]->host);
      if (!stat(msg, &buf)) {
	/* Check to see that status file was created before system
	   network connectivity ceased. */
	if (buf.st_mtime < table[n]->last_time.tv_sec) {
	    table[n]->last_time.tv_sec  = buf.st_mtime;
	    table[n]->last_time.tv_usec = 0;
	}
	unlink(msg);
      }
    }
#endif

    if (table[n]->last_time.tv_sec) {
      snprintf(msg, 255, "%s %s is alive, after %s",curr_time(), table[n]->host, timeval_diff(table[n]->last_time, current_time));
      table[n]->downtime += current_time.tv_sec - table[n]->last_time.tv_sec;
    } else {
      snprintf(msg, 255, "%s %s is alive",curr_time(), table[n]->host);
      table[n]->downtime += current_time.tv_sec - start_time;
    }

    table[n]->alive = 1;
    printf("%s\n", msg);
    (void) fflush(stdout);

    /* timestamp the first time the host responded */
    table[n]->first_time = current_time;
    table[n]->last_time = current_time;

    /* Check and execute any Notification commands */
    if (command) notify_command(table[n]->host, "up", msg);

  } else {
    /* timestamp the last time the host responded */
    gettimeofday(&current_time,&tz);
    table[n]->last_time = current_time;
  }
  return n;
}

void
usage(val)
int val;
{
  printf("usage: linkstat [-help] [-mac_check] [-log <log_file>] [-timeout <delay>]\n");
  printf("                [-interval <delay>] [-retry <num>] [-update <delay>]\n");
  printf("                [-notify <command>] [-file file | hosts...]\n");
  exit (val);
}

/*
 * This was replaced with a different system for monitoring the time
 *
void
alarmtout()
{
  signal(SIGALRM,(void *)alarmtout);
  longjmp(Sjbuf,1);
}
*/

void
display_report()
{
  /* Report statistics */
  long int period, offset;
  int i, count_offset;

  report_time = 0;  /* do not report again */
  period = time(NULL) - start_time;

  printf("%s SLA_REP Reporting Output (period %lds)\n",curr_time(), period);
  for (i=0; i<num_hosts; i++) {
    offset = 0;
    count_offset = 0;

/* Removed as MetaFrame application servers are better :) */
#ifdef WC_MOD
    /* WC_MOD: This is a mod to correct Citrix server downtime
     *         It is a little out of the scope of this utility
     *         but never the less there was a need for it  :)
     */
    if (((table[i]->host)[1] == 'i') || ((table[i]->host)[1] == 'M')) {
      struct stat buf;
      char msg[255];
      /* This might be a Citrix server that had hung services */
      snprintf(msg, 255, "/opt/LinkStat/log/status/%s", table[i]->host);
      if (!stat(msg, &buf)) {
	/* Check to see that status file was created before system
	   network connectivity ceased. */
	if (buf.st_mtime < table[i]->last_time.tv_sec) {
	  table[i]->last_time.tv_sec  = buf.st_mtime;
	  table[i]->last_time.tv_usec = 0;
	}
	if (table[i]->alive) {
	  /* Host is still responding to pings, so must fudge the stats */
          /* The conditional following this one will handle the case
           * where the host is currently down */
	  offset = start_time + period - table[i]->last_time.tv_sec;
          count_offset = 1;
        }
      }
    }
#endif

    if (!table[i]->alive) {
      /* Host is currently down, so need to update stats */
      if (table[i]->last_time.tv_sec)
	offset = start_time + period - table[i]->last_time.tv_sec;
      else
	offset = period;
    }

    if (table[i]->downtime + offset > period) {
      /*
       * Something is wrong here... This is usually the result
       * of an old Citrix status file left lying around.
       */
      printf("%s DBUG3 %ld %ld\n", curr_time(), period, table[i]->downtime + offset);
      printf("  Host: %s\n", table[i]->host);
      printf("    response: %d\n", table[i]->response);
      printf("    alive:    %d\n", table[i]->alive);
      printf("    index:    %d\n", table[i]->i);
      printf("    first_tm: %s", ctime(&(table[i]->first_time.tv_sec)));
      printf("    last_tm : %s", ctime(&(table[i]->last_time.tv_sec)));
      printf("    downtime: %ld\n", table[i]->downtime);
      printf("    count   : %d\n", table[i]->downtime_cnt);
    }

    if (table[i]->downtime_cnt + count_offset > 0)
      printf("%s SLA_REP %s down(sec) %ld count %d percentage %01.4f\n",curr_time(), table[i]->host,table[i]->downtime + offset, table[i]->downtime_cnt + count_offset, (double)((table[i]->downtime + offset) * 100) / (double)period);

    (void) fflush(stdout);
  }
}

void
hangup()
{
  printf("%s SIGHUP received\n", curr_time());

  /* Display downtime report */
  display_report();

  close(sock);
  exit(0);
}

void
process_host_list (argc, argv, filename)
     int argc;
     char **argv;
     char *filename;
{
  num_hosts=0;
  num_local_hosts=0;

  if (argc > 1 && *argv) {
    printf("Create Table Entries for:");
    while (*argv) {
      if ((table[num_hosts]=create_host_entry(*argv,NULL,0,retry,0,0)) != NULL) {
        printf(" %s", *argv);
        num_hosts++;
        num_local_hosts++;
        if (num_hosts == MAX_HOSTS)
	  crash_and_burn("process_host_list: MAX_HOSTS reached !");
      }
      ++argv;
    }
    printf("\n");
  } else if (filename) {
    FILE *ping_file;
    char line[132];
    char host[132],ip_addr[132],*p;
    int count,schedule,uniq_retry;
    short from,until;

    if (strcmp(filename,"-")==0) {
      ping_file=fdopen(0,"r");
    } else {
      ping_file=fopen(filename,"r");
    }
    if (!ping_file) errno_crash_and_burn("process_host_list: fopen");
    printf("Create Table Entries for:");
    while(fgets(line,132,ping_file)) {
      count=sscanf(line,"%131s %131s # (int=%d,ret=%d,mon=%hd:%hd",ip_addr,host,&schedule,&uniq_retry,&from,&until);
      if (count > 0) {
	if (ip_addr[0] == '#') continue;
	if (count > 1) {
	  /* Assume that we read in a host file entry */
	  p=(char*)malloc(strlen(host)+1);
	  if (!p) crash_and_burn("process_host_list: can't malloc host");
	  strcpy(p,host);

	  if (count <3) schedule=0;
	  if (count <4) uniq_retry=retry;
	  if (count <6) {from=0;until=0;}

	  if ((table[num_hosts]=create_host_entry(p,ip_addr,schedule,uniq_retry,from,until)) != NULL) {
	    if (schedule) { 
	      printf(" %s(%d", p, schedule);
	      if (uniq_retry != retry)
		printf(",%d", uniq_retry);
	      if (until)
		printf(",%hd-%hd", from, until);
	      printf(")");
	    } else {
	      num_local_hosts++;
	      printf(" %s", p);
	    }
	    num_hosts++;
	  }
	} else {
	  /* Assume we only entered a host name */
	  p=(char*)malloc(strlen(ip_addr)+1);
	  if (!p) crash_and_burn("process_host_list: can't malloc host");
	  strcpy(p,ip_addr);
	  if ((table[num_hosts]=create_host_entry(p,NULL,0,retry,0,0)) != NULL) {
	    printf(" %s", p);
	    num_hosts++;
	    num_local_hosts++;
	  }
	}
        if (num_hosts == MAX_HOSTS)
	  crash_and_burn("process_host_list: MAX_HOSTS reached !");
      }
    }
    fclose(ping_file);
    printf("\n");
  } else usage(7);

  if (num_hosts == 0) {
    printf("No valid hosts!\n");
    exit(1);
  }
}  

void
process_command_line (argc, argv)
     int argc;
     char **argv;
{
  extern char *optarg;
  char *filename = NULL;
  char *progname;             /* The name of this program. */
  char  option;
  char *log_file = NULL;

  static struct option long_options[] =
  {
    {"timeout",     1,   0,  't'},
    {"interval",    1,   0,  'i'},
    {"retry",       1,   0,  'r'},
    {"update",      1,   0,  'u'},
    {"slarep",      1,   0,  's'},
    {"debug",       1,   0,  'd'},
    {"file",        1,   0,  'f'},
    {"log",         1,   0,  'l'},
    {"notify",      1,   0,  'n'},
    {"mac_check",   0,   0,  'm'},
    {"help",        0,   0,  'h'},
    {"version",     0,   0,  'v'},
    {0, 0, 0, 0}
  };
  
  /* Find the base of the program name. */
  progname = strrchr (argv[0], '/');
  if (progname)
    progname++;           /* Skip the directory separator. */
  else
    progname = argv[0];   /* We already have a basename. */

/**
  while ((option = _getopt_internal(argc, argv, "n:t:i:r:u:f:s:l:d:mhv", long_options, 0, 1)) != -1)
**/
  int option_index=0;
  while ((option = getopt_long(argc, argv, "n:t:i:r:u:f:s:l:d:mhv", long_options, &option_index)) != (char)-1)
    switch (option) {
      case 't': if ((timeout=atoi(optarg)) <0) usage(1);  break;
      case 'i': if ((interval=atoi(optarg)) <0) usage(2); break;
      case 'r': if ((retry=atoi(optarg)) <1) usage(3);    break;
      case 'u': if ((update=atoi(optarg)) <1) usage(4);   break;
      case 's': if ((slarep=atol(optarg)) <1) usage(5);   break;
      case 'd': if ((debug=atoi(optarg)) <0) usage(6);    break;
      case 'f': filename= optarg;                         break;
      case 'l': log_file= optarg;                         break;
      case 'n': command= optarg;                          break;
      case 'm': check_hw=1;                               break;
      case 'v': version_display_info(2); exit(0);
      case 'h':
            printf("usage: %s [-help] [-mac_check] [-log <log_file>] [-timeout <delay>]\n", progname);
            printf("                [-interval <delay>] [-retry <num>] [-update <delay>]\n");
            printf("                [-notify <command>] [-file file | hosts...]\n\n");
            printf("    -mac_check\t\tcheck hardware (MAC) addresses of responding hosts\n");
            printf("    -timeout #\t\tdelay between polls (default %d msecs)\n", DEFAULT_TIMEOUT);
            printf("    -interval #\t\tdelay between packets (default %d msecs)\n", DEFAULT_INTERVAL);
            printf("    -retry #\t\tnumber of retries to a host (default %d)\n", DEFAULT_RETRY);
            printf("    -update #\t\tfrequency of statistical updates (default %d secs)\n", DEFAULT_UPDATE);
            printf("    -notify command\tcommand to run during state changes\n");
            printf("    -slarep #\t\tdelay (sec) before SLA Report is run (default 5pm)\n");
            printf("    -file file\t\tfile to read list of hosts\n");
            printf("    -log file\t\tfile to log output when detached from terminal\n\n");
            printf("note: only the first letter of each argument is required.\n\n");
            printf("examples:\n");
            printf("    %s -l log -f /etc/hosts\n", progname);
            exit(-1);
        default:
            usage(-1);
            exit(-1);
    }

  argv = &argv[optind];
  if (*argv && filename)   { usage(8); }
  if (!*argv && !filename) { filename = "-"; }
  
  /*
   * Sanity Check on some parameters
   */
  if (interval < MIN_INTERVAL) interval = MIN_INTERVAL;
  min_interval = interval;
  if (timeout < MIN_TIMEOUT) timeout = MIN_TIMEOUT;
  if (retry < 1) retry = 1;

  process_host_list (argc, argv, filename);

  printf("Done!  %d hosts (%d local)\n",num_hosts,num_local_hosts);
  (void) fflush(stdout);

  if (log_file) detachFromTTY(log_file);
}

int
main(argc,argv)
     int argc;
     char ** argv;
{
  int i, cycles;
  struct protoent *proto;
  struct tm *timeptr;
  time_t sys_clock;
  short  sys_time;
  struct tm *the_time;
  ident = getpid() & 0xFFFF;

  process_command_line(argc, argv);

  if ((proto = getprotobyname("icmp")) == NULL) {
    printf("icmp: unknown protocol\n");
    exit(-1);
  }
  
  sock = socket(AF_INET, SOCK_RAW, proto->p_proto);
  if (sock<0) errno_crash_and_burn("main: socket");

  /* Initialize Index Entries */
  gettimeofday(&current_time, &tz);
  for( i=0; i < num_hosts; i++ ) {
    table[i]->alive = 1;            /* Assume all hosts initially live */
    table[i]->response = retry;     /* Set number of times to retry host */
    table[i]->i = i;                /* Quick index into table */

    /* Set the time to receive its first packet (now) */
    table[i]->next_time.tv_sec = current_time.tv_sec;
  }

  signal(SIGHUP,hangup);

  /*
   * Changed the following signal-based status updates to a time
   * base system.  The signals were interupting processing causing
   * errors in the displayed data.
   */

  /*
   * The following is a little like alarm() but will send the first signal
   * after 5 seconds and then every "update" seconds after that
   *
  interval_timer.it_value.tv_sec  = 5;
  interval_timer.it_value.tv_usec = 0;
  interval_timer.it_interval.tv_sec  = update;
  interval_timer.it_interval.tv_usec = 0;
  signal(SIGALRM,alarmtout);
  setitimer(ITIMER_REAL, &interval_timer, NULL);
   *
   */

  cycles = 0;
  baseline = time(NULL) - update + 5;  /* first display after 5 seconds */
  start_time = time(NULL);

  if (slarep)
    report_time = time(NULL) + slarep;
  else {
    /*
     * Default: If started before 5pm then a SLA report will be
     *          produced at, or near after, 5pm.  The exact time
     *          that the SLA report will be produced will depend
     *          on the time between updates (value of update).
     */
    timeptr = localtime(&start_time);
    if (timeptr->tm_hour < 17) {
      timeptr->tm_hour = 17;
      timeptr->tm_min = 0;
      timeptr->tm_sec = 0;
      report_time = mktime(timeptr);
    } else {
      report_time = 0;
    }
  }

  printf("%s LinkStat v%s (%s)\n", curr_time(),version_get_str(),version_get_rel_date());

  /*printf("%s Polling %d hosts with a %ds timeout, %d retries, %ds updates, %d ident\n", curr_time(),num_hosts,timeout/1000,retry,update,ident);*/
  printf("%s Loaded %d host%s, using %ds updates, %d ident\n", curr_time(),num_hosts,(num_hosts == 1 ? "" : "s"),update,ident);
  printf("%s Polling %d host%s with a %ds timeout, %d retries\n", curr_time(),num_local_hosts,(num_local_hosts == 1 ? "" : "s"),timeout/1000,retry);

  if (num_hosts - num_local_hosts > 0)
    printf("%s Polling %d remote hosts with various timeouts\n", curr_time(),num_hosts-num_local_hosts);
  if (report_time)
    printf("%s Service Level Report will be produced on %s", curr_time(), ctime(&report_time));
  (void) fflush(stdout);

  while (1) {
    /*
     * Start a new cycle - count the number of times we poll in each
     * interval.  Keep a track of how many local hosts we are waiting
     * on for a reponse.
     */

    cycles++;

    /*
     * Update the sys_time to contain the current system time in 24hr format
     */
    sys_clock = time(NULL);
    the_time = localtime(&sys_clock);
    sys_time = the_time->tm_hour * 100 + the_time->tm_min;

    /*
     * Start collecting results, one at a time with
     * a possible pause of "interval" milliseconds.
     * At the same time, initialize the results table
     * for each scheduled entry.
     */
    for (i=0; i<num_hosts; i++) {
      if (table[i]->monitor_until && (sys_time < table[i]->monitor_from || sys_time > table[i]->monitor_until))
	continue;

      gettimeofday(&current_time, &tz);
      if (table[i]->next_time.tv_sec <= current_time.tv_sec) {
        /*
         * Ready to send the next packet to this host
         */

	if ((table[i]->response != table[i]->retry) &&
	    (table[i]->alive) &&
	    (table[i]->packet_schedule == 0)) {
	  /*
	   * We have been around a full cycle with no response
           * from this (local) host - lets re-evalutate the interval.
	   */
	  queue_len++;
	}
	if (table[i]->response) table[i]->response--;

        /*
         * Ship off a packet and wait (a little while) for
         * one's return.  This gives the network interface
         * a possible break between probes.
         */
	send_ping(sock,table[i]);
	wait_for_reply(sock,interval);

        /*
         * Update schedule for the next packet to this host
	 */
        table[i]->next_time.tv_sec += table[i]->packet_schedule;

	/*
	 * For every ~10 packets sent, give any queued packets a
         * chance to be cleared.  This is required as the above
         * wait_for_reply(s,interval) will always return immediately
         * as soon as we get behind in processing traffic.
	 */
	if (i % 10 == 9 || i == (num_hosts-1)) while (wait_for_reply(sock,1));

      }
    }

    if (time(NULL) >= (baseline + update)) {
      if (!check_hw)
        printf("%s Waiting on %d (%d unreachable), I:%dms R:%d C:%d\n", curr_time(), queue_len, num_local_unreachable, interval, optimal_retry, cycles);
      else
        printf("%s Waiting on %d (%d unreachable), I:%dms R:%d C:%d M:%d\n", curr_time(), queue_len, num_local_unreachable, interval, optimal_retry, cycles, macs_checked);

      (void) fflush(stdout);
      cycles=0;
      optimal_retry=0;
      baseline = time(NULL);

      if (report_time && baseline >= report_time) {
        display_report();
      }
    }

    /*
     * Adjust the inter-packet interval if we are not allowing
     * enough time during the intervals for the packets to return.
     */

    if (queue_len > 0) {
      /*
       * If we are still waiting on some hosts then increase the interval
       * (to let the hosts catch up).  At this point also disable the
       * negative adjustment of the same interval.  (May like to look into
       * resetting this timer)
       */
      interval+=2;        /* add 2ms to the interval between packets */
      queue_len=0;        /* reset the counter */
      adjusting=0;        /* zero adjustment value */
    } else if ((adjusting++ > 9) &&
	       (interval > min_interval)) {
      /*
       * If we are not waiting for any hosts then decrease the interval
       * timer (to speed up the polling interval), but do not go below
       * the initially specified interval.
       */
      interval-=(adjusting/10);  /* decrease the interval */
      if (interval < min_interval) {
	/* if we are below the minimum then set the interval to the
	   minimum and hold off adjustment, until the next increment */
	interval = min_interval;
	adjusting= -32000;
      }
    }

    /*
     * The following will clear any waiting packets and
     * cause a pause of timeout/1000 seconds
     */
    while (wait_for_reply(sock,timeout));

    /*
     * Find all hosts that are no longer reachable
     */
    for( i=0; i < num_hosts; i++ ) {
      static char msg[255];
      if ((table[i]->response < 1) && table[i]->alive) {
	if (table[i]->packet_schedule == 0)
	  num_local_unreachable++;
	if (table[i]->first_time.tv_sec)
	  snprintf(msg, 255, "%s %s is unreachable, after %s",curr_time(), table[i]->host, timeval_diff(table[i]->first_time, table[i]->last_time));
	else
	  snprintf(msg, 255, "%s %s is unreachable",curr_time(), table[i]->host);
	printf("%s\n", msg);
	(void) fflush(stdout);
        table[i]->alive=0;
	table[i]->downtime_cnt++;
	if (command) notify_command(table[i]->host, "down", msg);
      }
    }
  }

  /* should not get here as the previous is a loop forever */
  close(sock);
  return 0;
}
