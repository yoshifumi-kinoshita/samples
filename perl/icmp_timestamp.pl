#!/usr/bin/perl

use strict;
use Socket;

use constant ICMP_TS => 13;
use constant ICMP_STRUCT => "C2 n3 N3 A"; # Structure of a minimal ICMP packet
use constant SUBCODE => 0; # No ICMP subcodefor ECHO and ECHOREPLY
use constant ICMP_FLAGS => 0; # No special flags for send or recv
use constant ICMP_PORT => 0; # No port with ICMP
use constant IOT => 0; #ICMP originate timestamp
use constant IRT => 0; #ICMP receive timestamp
use constant ITT => 0; #ICMP transmit timestamp

ping_icmp();
sub ping_icmp
{
  my ($ip) = inet_aton('10.64.221.223');
  my ($saddr, $checksum, $msg, $len_msg);

  my $seq = 1;
  my $pid = $$ & 0xffff;
  $checksum = 0; # No checksum for starters
  my $data= "";
  $msg = pack(ICMP_STRUCT .0,ICMP_TS,SUBCODE,$checksum,$pid,$seq,IOT,IRT,ITT,$data);
  $checksum =checksum($msg);
  $msg = pack(ICMP_STRUCT .0,ICMP_TS,SUBCODE,$checksum,$pid,$seq,IOT,IRT,ITT,$data);
  $len_msg = length($msg);
  $saddr = sockaddr_in(ICMP_PORT, $ip);

  socket(SOCK,PF_INET,SOCK_RAW,1);
  send(SOCK,$msg,ICMP_FLAGS,$saddr);
}

sub checksum{
  my ($msg) = @_;
  my ($len_msg,$num_short,$short,$chk);

  $len_msg = length($msg);
  $num_short = int($len_msg / 2);
  $chk = 0;
  foreach $short (unpack("n$num_short", $msg)) {
    $chk += $short;
  } 

  $chk += (unpack("C", substr($msg, $len_msg - 1, 1))<< 8) if $len_msg % 2;
  $chk = ($chk >> 16) + ($chk & 0xffff); # Foldhigh into low
  return(~(($chk >> 16) + $chk) & 0xffff); # Again and complement
}

