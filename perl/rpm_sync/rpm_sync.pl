#!/usr/bin/perl

use strict;
#use warnings;
use Data::Dumper;

sub usage{
	print "usage: rpm_sync.pl installed-rpms","\n";
	exit;
}

usage unless $ARGV[0];

my $DESTDIR="rpm_sync";
my %rpm_qa; #"{%NAME}"=>"%{VERSION}-%{RELEASE}.%{ARCH}"

download_rpms($ARGV[0]);

rpm_qa();

#print Dumper %rpm_qa;

# compare and install or update RPMs.
compare_install_update();


sub download_rpms{
	my $installed_rpms = shift;
	my $fh;
	open($fh, $installed_rpms) || die "Can't open $installed_rpms";
	mkdir $DESTDIR;
	while(<$fh>){
		s/\s.*//g;
		if($_ && !-e "$DESTDIR/$_.rpm"){
			print "/usr/bin/yumdownloader --destdir=$DESTDIR $_", "\n";
			print `/usr/bin/yumdownloader --destdir=$DESTDIR $_`;
		}
	}
	close $fh;
}


sub rpm_qa{
	my $fh;
	open($fh, 'rpm -qa --queryformat="%{NAME} %{VERSION}-%{RELEASE}.%{ARCH}\n" | sort |');	
	while(<$fh>){
		m/(.*) (.*)/;
		$rpm_qa{$1}="$2";
	}
	close $fh;
	
}

sub compare_install_update{
	chdir $DESTDIR;
	my $YUMOPT='-y --disablerepo=* ';
	my $fh;
	open($fh, 'ls *.rpm |');
	while(<$fh>){
		m/^(.*)-([^-]+-[^-]+).rpm$/;
		# "%{VERSION}-%{RELEASE}.%{ARCH}"
		if($rpm_qa{$1} eq ''){
			print "yum $YUMOPT install $_"; 
			print `yum $YUMOPT install $_`;
		} elsif( 1 == ($2 cmp $rpm_qa{$1}) ){
			print "yum $YUMOPT update $_"; 
			print `yum $YUMOPT update $_`;
		} elsif( -1 == ($2 cmp $rpm_qa{$1}) ){
			print "yum $YUMOPT update --downgrade $_"; 
			print `yum $YUMOPT update --downgrade$_`;
		}
	}
	close $fh;
}

