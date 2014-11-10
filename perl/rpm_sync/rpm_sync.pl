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

print qq(\n === Please delete "/etc/yum.repos.d/rpm_sync.repo" and "/var/tmp/rpm_sync". === \n);

sub download_rpms{
	my $installed_rpms = shift;
	my $fh;
	open($fh, $installed_rpms) || die "Can't open $installed_rpms";
	mkdir $DESTDIR;
	while(<$fh>){
		s/\s.*//g;
		if($_ && !-e "$DESTDIR/$_.rpm"){
			print "/usr/bin/yumdownloader --destdir=$DESTDIR $_", "\n";
			`/usr/bin/yumdownloader --destdir=$DESTDIR $_`;
		}
	}
	close $fh;
}


sub rpm_qa{
	my $fh;
	open($fh, 'rpm -qa --queryformat="%{NAME}.%{ARCH} %{VERSION}-%{RELEASE}\n" | sort |');	
	while(<$fh>){
		m/(.*) (.*)/;
		$rpm_qa{$1}="$2";
	}
	close $fh;
	
}


sub compare_install_update{
	chdir $DESTDIR;
	my $YUMOPT='-y --disablerepo=* --enablerepo=rpm_sync';
	#my $YUMOPT='-y --disablerepo=* ';
	my @installs;
	my @updates;
	my @downgrades;
	my $fh;
	open($fh, 'ls *.rpm |');
	while(<$fh>){
		chomp;
		m/^(.*)-([^-]+-[^-]+)\.([0-9A-z_]+)\.rpm$/;
		# "%{VERSION}-%{RELEASE}.%{ARCH}"
		if($rpm_qa{"$1.$3"} eq ''){
			push @installs, $_;
                        print qq(yum install $YUMOPT $_ \n);
                        `yum install $YUMOPT $_`;
		} elsif( 1 == my_cmp($2, $rpm_qa{"$1.$3"}) ){
			push @updates, $_;
                        print qq(yum update $YUMOPT $_ \n);
                        `yum update $YUMOPT $_`;
		} elsif( -1 == my_cmp($2, $rpm_qa{"$1.$3"}) ){
			push @downgrades, $_;
                        print qq(yum downgrade $YUMOPT $_ \n);
                        `yum downgrade $YUMOPT $_`;
		}
	}

	if(@downgrades){
		my $yum_downgrade = "yum downgrade $YUMOPT ". join " ", @downgrades;
		#print $yum_downgrade, "\n";
		#`$yum_downgrade`;
	}
	if(@installs){
		my $yum_install = "yum install $YUMOPT " . join " ", @installs;
		#print $yum_install, "\n";
		#`$yum_install`;
	}
	if(@updates){
		my $yum_update = "yum update $YUMOPT " . join " ", @updates;
		#print $yum_update, "\n";
		#`$yum_update`;
	}
	close $fh;
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
enabled=0
gpgcheck=0

EOF
	;
	print $fh $repo;
	close $fh;
	`createrepo $DESTDIR`;
}

sub split_ver_rel{
        my($rpm) = @_;
        $rpm =~ m/^([^-]+)-([^-]+)$/;
        return {ver=>$1, rel=>$2};
}

sub my_cmp{
        my $a_ver_rel = split_ver_rel( shift );
        my $b_ver_rel = split_ver_rel( shift );

        my @a_ver = split /\./,$a_ver_rel->{ver};
        my @b_ver = split /\./,$b_ver_rel->{ver};

        my $res = my_cmp_d( \@a_ver, \@b_ver );
        if( $res ){
                return $res;
        }
        else{
                my @a_rel = split /\./,$a_ver_rel->{rel};
                my @b_rel = split /\./,$b_ver_rel->{rel};
                return my_cmp_d( \@a_rel, \@b_rel );
        }
}

sub my_cmp_u{
        my($arga, $argb) = @_; 
        my $a = shift @$arga;
        my $b = shift @$argb;

        if(defined $a && !defined $b){
                return 1;
        }   
        elsif(!defined $a && defined $b){
                return -1; 
        }
        elsif(!defined $a && !defined $b){
                return 0;
        }   
        
        if( $a <=> $b ){
                return $a <=> $b; 
        }   
        elsif( $a cmp $b ){
                return $a cmp $b; 
        }   
        else {
                return my_cmp_u($arga, $argb);
        }
}

sub my_cmp_d{
        my($arga, $argb) = @_;
        my $a = shift @$arga;
        my $b = shift @$argb;

        if(defined $a && !defined $b){
                return 1;
        }
        elsif(!defined $a && defined $b){
                return -1;
        }
        elsif(!defined $a && !defined $b){
                return 0;
        }

        my @a = split /_/, $a;
        my @b = split /_/, $b;

        my $res = my_cmp_u(\@a,\@b);
        if( $res ){
                return $res;
        }
        elsif( $a cmp $b ){
                return $a cmp $b;
        }
        else {
                return my_cmp_d($arga, $argb);
        }
}

