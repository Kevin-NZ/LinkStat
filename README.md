                                                                          
 File          : linkstat.c  --  link status daemon                       
 Version       : 2.0.0                                                    
                                                                          
 Created On    : Sat Feb 21 11:12:32 NZDT 1998                            
 Author        : Kevin Clark                                              
                                                                          
 Last Modifier : Kevin Clark                                              
 Modified On   : Sat Aug  7 13:11:44 NZDT 2010                            
 Mod Count     : 17                                                       
 Status        : TESTED                                                   
                                                                          
                                                                          
 Purpose:                                                                 
                                                                          
     To provide a quick accurate method of monitoring system uptime.      
     Using this program it is possible to get an immediate update when    
     a system goes down.  It is also possible to calculate the uptime     
     of a system and track when systems (WinNT) crash and automatically   
     reboot.                                                              
                                                                          
                                                                          
 Method:                                                                  
                                                                          
     When started each host listed is processed one at a time and sent    
     an ICMP packet followed by a 10ms pause (configured through the      
     "interval" parameter).  If at the end of this process we are still   
     waiting for some ICMP packets to return then we increase the         
     pause interval by 2ms.  This will give each ICMP packet a little     
     longer to return.  If after ten consecutive cycles we are not        
     waiting for any packets then the pause interval starts to decrement  
     by one for each ten cycles (for example if after 30 cycles we are    
     still not waiting on any packets, then the pause interval will be    
     decremented by 3ms during each cycle).  This process stops when      
     either we hit the minimum interval (default 10ms) or we encounter    
     an outstanding packet.                                               
                                                                          
     At the end of each cycle we wait 1000ms (configured through the      
     "timeout" parameter) for any outstanding packets.  This also has     
     effect of pausing for at lease a second between cycles.              
                                                                          
     If a packet is not received from a host after 3 consecutive cycles   
     (configured through the "retry" parameter) then the host is          
     considered to be "down".  A host is considered to be "up" as soon    
     as any packet is received from it.                                   
                                                                          
                                                                          
 Command Line Options:                                                    
                                                                          
     -help            display information                                 
     -version         display version information                         
     -timeout #       delay between polls (default 1000 msecs)            
     -interval #      delay between packets (default 10 msecs)            
     -retry #         number of retries to a host (default 3)             
     -update #        frequency of statistical updates (default 300 secs) 
     -notify command  command to run during state changes                 
     -slarep #        delay (sec) before SLA Report is run (default 5pm)  
     -file file       file to read list of hosts                          
     -log file        file to log output when detached from terminal      
     -mac_check       check the MAC address of the returned packets       
                                                                          
                                                                          
 Notes:                                                                   
                                                                          
     This should cause minimal network load as we should only have one    
     ICMP datagram out on the network at any one time.  These datagrams   
     are at low priority and will only be to hosts that are directly      
     connected (or near) to the source.                                   
                                                                          
     If started with defaults, each host listed will be sent an ICMP      
     packet (one at a time), then there will be at least a 1 second       
     pause, at which time the whole process is started again.             
                                                                          
     A status message is produced after the first 5 seconds, and then     
     at 5 minute intervals.                                               
                                                                          
     It is possible to have a command executed when a host changes        
     state.  Using this it is possible to double check all state changes  
     by attempting to attach (or telnet) to that particular host.         
     This command is configured through the "notify" parameter and is     
     passed three command line arguments.  They are the hostname, the     
     current state and a message description (detailing time, host and    
     state change).                                                       
                                                                          
     Many of the parameters are configurable on the command line.         
     The minimum value for the interval parameter is 5ms, and 500ms for   
     the timeout parameter.                                               
                                                                          
     While other methods of listing the hosts to check are available,     
     such as command line and standard input, the only one that has full  
     support is the "hosts" file specification (-f option).               
     Lines in this file have the following format:                        
                                                                          
        <IP Address> <Hostname> # (int=<freq>,ret=<num>,mon=<HHMM:hhmm>)  
                                                                          
     While the items after the "#" are optional, the currently supported  
     options are as follows:                                              
        int=<frequency> - Specifies how often to check the host (secs)    
        ret=<number>    - Specifies the max number of packet retransmits  
        mon=<HHMM:hhmm> - Monitor between the hours of HHMM and hhmm      
                                                                          
     A description of the lines recorded in the logfile are as follows:   
     1/ <host> is unreachable, after <time>                               
          This reports that the host is no longer contactable. There may  
          also be displayed the time that the host was previously         
          available (ie uptime)                                           
     2/ <host> is alive, after <time>                                     
          This reports that the host is now responding.  There may also   
          be displayed the time that the host was previously unavailable  
          (ie downtime)                                                   
     3/ Waiting on <x> (<y> unreachable), I:<i> R:<r> C:<c> M:<m>         
          This is a status message showing that we are currently waiting  
          on X number of local hosts to respond with Y local hosts        
          currently unreachable.  It also reports the current Interval    
          value.  The R value is the optimal retry count (this will be at 
          its maximum when a host has gone down).  The C parameter shows  
          how many cycles the process has gone through since last         
          displaying this message.  The M parameter indicates how many    
          hardware (MAC) addresses are being checked.                     
                                                                          
                                                                          
 Possible Improvements:                                                   
                                                                          
     Currently there is no maximum value for the interval value.          
     Currently there is no formal structure for the input hosts file.     
     Configuration file reload support has been removed/replaced with     
     a SLA report and exit.  Should be re-enabled in the future, problems 
     with structure re-initialization while still keeping the current     
     statistics.                                                          
     The program has a predefined maximum number of hosts (MAX_HOSTS).    
     This could be changed to be a runtime malloc so as to be a little    
     more efficient on memory resources, as well as handling much larger  
     lists.                                                               
     Need to get a little smarter in the parsing of the input hosts       
     file as it is currently fixed format (eg int=x,ret=x,mon=x).         
                                                                          
                                                                          
 Caveats                                                                  
                                                                          
     You should be aware that a hardware NIC can respond to ICMP pings    
     even if the operating system is hung.  A semi-bad network connection 
     (eg 50% packet loss) may not be picked up by this method due to some 
     packets still being returned.                                        
                                                                          
                                                                          
 CPU Utilization:                                                         
                                                                          
     CPU Utilization should be < 1.20% for 110 hosts                      
     CPU Utilization should be < 1.50% for 195 hosts                      
     CPU Time Should be approx 1:10 (8am-5pm for 110 hosts)               
     CPU Time Should be approx 2:05 (8am-5pm for 195 hosts)               
                                                                          
                                                                          
 History:                                                                 
                                                                          
   1.0.0  21-FEB-98  Original version                                     
   1.1.0  24-FEB-98  Added notification command                           
   1.2.0  06-SEP-98  Added configuration file reload support (SIGHUP)     
   1.3.0  21-OCT-98  Added WinCenter services/downtime hack (see WC_MOD)  
   1.3.1  30-OCT-98  Check that the WinCenter status file was created     
   1.3.2  07-JAN-99  Fixed statistics reporting (for WinCenter hack)      
   1.3.3  16-MAR-99  Added DBUG3 to diagnose a problem                    
   1.3.4  30-MAR-99  Fixed above problem (offset var not being reset)     
   1.4.0  16-JUL-99  Ported code to RedHat Linux 6.0                      
   1.5.0  22-SEP-99  Added a "Year" component to the log files            
   1.6.0  03-NOV-99  Added variable packet frequency (int= option)        
   1.6.1  04-MAY-00  Altered interval adjustment algorithm                
   1.6.2  27-JUL-00  Made big changes to response detection               
   1.6.3  04-AUG-00  Added variable packet retries (ret= option)          
   1.7.0  13-Mar-02  Added option to check the MAC address of the source  
   1.7.1  07-Nov-02  Removed WinCenter hacks (by using #ifdef WC_MOD)     
   1.8.0  01-Jul-03  Altered code to query multiple arp caches (linux)    
   1.9.0  10-Jan-06  Added support for time filtering (mon= option)       
   1.9.1  27-Feb-09  Clean up some compiler warnings                      
   2.0.0  07-Aug-10  Support for System/390 (s390x)                       
