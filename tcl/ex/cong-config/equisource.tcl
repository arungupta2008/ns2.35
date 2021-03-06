# config file for remy simulations
# this one is where all the sources are identical
global opt

# source, sink, and app types
set opt(nsrc) 2;                # number of sources in experiment
set opt(tcp) TCP/Reno
set opt(sink) TCPSink
set opt(cycle_protocols) false
set protocols [list TCP/Newreno TCP/Vegas TCP/Vegas TCP/Newreno]; # don't put Linux TCPs first on list
set protosinks [list TCPSink TCPSink TCPSink TCPSink]
#set protocols [list TCP/Newreno TCP/Linux/cubic]
#set protosinks [list TCPSink TCPSink/Sack1/DelAck]
#set protocols [list TCP/Newreno TCP/Rational]
set protocols [list TCP/Linux/cubic TCP/Linux/compound TCP/Linux/cubic TCP/Linux/compound ]
set protosinks [list TCPSink/Sack1 TCPSink/Sack1 TCPSink/Sack1 TCPSink/Sack1 ]

set opt(app) FTP
set opt(pktsize) 1210
set opt(rcvwin) 16384

# topology parameters
set opt(gw) DropTail;           # queueing at bottleneck
set opt(bneck) 10Mb;             # bottleneck bandwidth (for some topos)
set opt(maxq) 200;             # max queue length at bottleneck
set opt(delay) 49ms;            # total one-way delay in topology
set opt(link) None

# random on-off times for sources
set opt(seed) 0
set opt(onrand) Exponential
set opt(offrand) Exponential
set opt(onavg) 5.0;              # mean on and off time
set opt(offavg) 5.0;              # mean on and off time
set opt(avgbytes) 16000;          # 16 KBytes flows on avg (too low?)
set opt(ontype) "time";           # valid options are "time" and "bytes"

# simulator parameters
set opt(simtime) 300.0;        # total simulated time
set opt(tr) remyout;            # output trace in opt(tr).out
set opt(partialresults) false;   # show partial throughput, delay, and utility scores?

# utility and scoring
set opt(alpha) 1.0
set opt(tracewhisk) "none";     # give a connection ID to print for that flow, or give "all"

proc set_access_params { nsrc } {
    global accessdelay
    for {set i 0} {$i < $nsrc} {incr i} {
        set accessdelay($i) 1ms;       # latency of access link
    }
    global accessrate
    for {set i 0} {$i < $nsrc} {incr i} {
        set accessrate($i) 1000Mb;       # speed of access link
    }
}
