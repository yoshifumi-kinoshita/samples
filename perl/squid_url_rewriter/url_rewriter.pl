#!/usr/bin/perl

use strict;

$|=1;
while (<>) {
  chomp;
  # "http://www.redhat.com/ 127.0.0.1/localhost.localdomain - GET -\n"
  my ($url, $client, undef, $method undef) = split;
  $url =~ s/www\.//;
  print $url, "\n";
}


