#!/usr/bin/perl

use strict;
#use warnings;
use Data::Dumper;

sub usage{
	print "usage: rpm_sync.pl installed-rpms","\n";
	exit;
}

usage unless $ARGV[0];

my $DESTDIR="/var/tmp/rpm_sync";
my %rpm_qa; #"{%NAME}"=>"%{VERSION}-%{RELEASE}.%{ARCH}"

download_rpms($ARGV[0]);

# create local repo
install_createrepo();

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
	my $YUMOPT='-y --disablerepo=* --enablerepo=rpm_sync';
	my @installs;
	my @updates;
	my @downgrades;
	my $fh;
	open($fh, 'ls *.rpm |');
	while(<$fh>){
		chomp;
		m/^(.*)-([^-]+-[^-]+).rpm$/;
		# "%{VERSION}-%{RELEASE}.%{ARCH}"
		if($rpm_qa{$1} eq ''){
			push @installs, $_;
		#} elsif( 1 == ($2 cmp $rpm_qa{$1}) ){
		} elsif( 1 == my_cmp($2, $rpm_qa{$1}) ){
			push @updates, $_;
		#} elsif( -1 == ($2 cmp $rpm_qa{$1}) ){
		} elsif( -1 == my_cmp($2, $rpm_qa{$1}) ){
			push @downgrades, $_;
		}
	}
	if(@downgrades){
		my $yum_downgrade = "yum downgrade $YUMOPT ". join " ",@downgrades;
		print $yum_downgrade, "\n";
		print `$yum_downgrade`;
	}
	if(@installs){
		my $yum_install = "yum install $YUMOPT " . join " ", @installs;
		print $yum_install, "\n";
		print `$yum_install`;
	}
	if(@updates){
		my $yum_update = "yum update $YUMOPT " . join " ", @updates;
		print $yum_update, "\n";
		print `$yum_update`;
	}
	close $fh;
}

sub my_cmp{
	my $a = shift;
	my $b = shift;
	my @a;
	my @b;
	@a = split /[\-\._]/, $a;
	@b = split /[\-\._]/, $b;
	for(my $i=0; $i<@a; $i++){
		my $result = ($a[$i] <=> $b[$i]);
		return $result if( $result );
	}
	return 1;
}

sub install_createrepo{
	`yum install -y createrepo`;
	my $fh;
	my $RPM_SYNC_REPO='/etc/yum.repos.d/rpm_sync.repo';
	open($fh, ">$RPM_SYNC_REPO") || die "can't open $RPM_SYNC_REPO";
	my $repo=<<EOF
[rpm_sync]
name=rpm_sync
baseurl=file://$DESTDIR
enabled=1
gpgcheck=0

EOF
	;
	print $fh $repo;
	close $fh;
	`createrepo $DESTDIR`;
}

