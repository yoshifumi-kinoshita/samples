#!/usr/bin/perl

use strict;
use warnings;
use Sys::Syslog qw(:DEFAULT setlogsock);

setlogsock("inet");
$Sys::Syslog::host = "192.168.222.57";
openlog("test", "ndelay", "local0");
syslog("warning", "msg to syslog server.");


