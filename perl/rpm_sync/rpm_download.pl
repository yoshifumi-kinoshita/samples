#!/usr/bin/perl

# If you use this on RHEL5, you need to install yum-utils for yumdownloader in advance.

use strict;
#use warnings;
use Data::Dumper;

my $wget_opt=' -nc ';

sub usage{
	print "usage: rpm_download.pl url_list.txt installed-rpms","\n";
	exit;
}

usage unless $ARGV[0];
usage unless $ARGV[1];

my $DESTDIR="/var/tmp/rpm_sync";

my %url_list;

parse_url_list();
download();


sub parse_url_list{
	my $fh;
	open($fh, $ARGV[0]) or die "Can't open $ARGV[0]","\n";
	while(<$fh>){
		chomp;
		my($rpm,$url)=split;
		$url_list{ $rpm } = $url;
	}
}

sub download{
	my $fh;
	open($fh, $ARGV[1]) or die "Can't open $ARGV[1]","\n";
	while(<$fh>){
		my($rpm, undef)=split;
		continue if( $rpm=~m/\(/ );
		$rpm .= ".rpm";
		print "wget $wget_opt $url_list{$rpm}","\n";
		`wget $wget_opt $url_list{$rpm}`;
	}
}


