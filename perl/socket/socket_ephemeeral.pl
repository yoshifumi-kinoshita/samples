#!/usr/bin/perl

use strict;
use warnings;
use Socket;

my $sock;
my $host=$ARGV[0];
my $port=$ARGV[1];
my $count=$ARGV[2];

my $sock_addr = sockaddr_in($port, inet_aton($host) ) or die "Cannot pack $host:$port: $!";

for(my $i=0; $i<$count; $i++){
	my $sock;
	socket($sock, PF_INET, SOCK_STREAM, getprotobyname('tcp')) or die "Cannot create socket: $!";
	connect($sock, $sock_addr) or die "Cannot connect $host:$port: $!";
	sleep(1);
}


