#!/usr/bin/perl -wT

use strict;
use Socket;
use Time::HiRes qw( gettimeofday );
use Time::Local;

my ( $rb, $sb, $vn, $client, $sec, $msec, $proto, $paddr );
my ( $NTP_LI, $NTP_LEAP, $MODE, $FLAG, $STR, $POLL, $PREC );
my ( $USO_LEAP, $USO_OFFSET );

$NTP_LI = timegm( 0, 0, 0, 31, 12-1, 05 );      # 05/12/31 00:00:00 UTC
$NTP_LEAP = timegm( 0, 0, 0, 1, 1-1, 06 );      # 06/01/01 00:00:00 UTC

$USO_LEAP = timegm( 0, 0, 0, 11, 12-1, 05 );    # 05/12/11 00:00:00 UTC
$USO_OFFSET = $NTP_LEAP - $USO_LEAP;

$MODE = 4;              # mode = 4 (server)
$STR = 1;               # stratum 1
$POLL = 6;              # poll interval = 2^6 sec
$PREC = -16;            # precision = 2^-16 sec

$proto = getprotobyname('udp');
socket(NTP, PF_INET, SOCK_DGRAM, $proto)        or die "socket: $!";
$paddr = sockaddr_in( 123, INADDR_ANY );
bind(NTP, $paddr)                               or die "bind: $!";

$sb = "\0" x 48;

while ("You hate leapseconds") {
        ($client = recv(NTP, $rb, 256, 0)) or next;
        ( length($rb) >= 48 ) or next;
        ($sec, $msec) = gettimeofday;
        $sec += $USO_OFFSET;
        $FLAG = $MODE | ( unpack("C",substr($rb,0,1)) & 0x38 ); # version
        #if(($sec >= $NTP_LI) && ($sec <= $NTP_LEAP)) { $FLAG |= 0x40; }
        if(1) { $FLAG |= 0x40; }
        if(($sec > $NTP_LEAP)) { $sec -= 1; }           # offset
        substr($sb, 0, 4) = pack("CCCc", $FLAG, $STR, $POLL, $PREC);
        substr($sb, 4, 4) = pack("N", 0);               # delay
        substr($sb, 8, 4) = pack("N", 0);               # dispersion
        substr($sb,12, 4) = "LOCL";                     # Ref ID
        substr($sb,16, 4) = pack("N", $sec + 0x83AA7E80);
        substr($sb,20, 4) = pack("N", 0 );
        substr($sb,24, 8) = substr($rb,40, 8);
        substr($sb,32, 4) = pack("N", $sec + 0x83AA7E80);
        substr($sb,36, 4) = pack("N", ($msec/500000) * 0x80000000 );
        ($sec, $msec) = gettimeofday;
        $sec += $USO_OFFSET;
        if(($sec > $NTP_LEAP)) { $sec -= 1; }
        substr($sb,40, 4) = pack("N", $sec + 0x83AA7E80);
        substr($sb,44, 4) = pack("N", ($msec/500000) * 0x80000000 );

        send(NTP, $sb, 0, $client);
}

