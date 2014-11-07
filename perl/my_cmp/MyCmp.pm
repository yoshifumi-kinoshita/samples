package MyCmp;

use strict;
use Data::Dumper;

sub split_name_ver_rel{
	my($rpm) = @_;
	$rpm =~ m/^(.*)-([^-]+)-([^-]+)$/;
	return {name=>$1, ver=>$2, rel=>$3};
}


sub my_cmp{
        my($a,$b) = @_;

	my @a_ver = split /\./,$a->{ver};
	my @b_ver = split /\./,$b->{ver};

	my $res = my_cmp_d( \@a_ver, \@b_ver );
	if( $res ){
		return $res;
	}
	else{
		my @a_rel = split /\./,$a->{rel};
		my @b_rel = split /\./,$b->{rel};
		return my_cmp_d( \@a_rel, \@b_rel );
	}
}

# ex: compare 3_4 and 3_33
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

# ex: compare 1.2.3_4 and 1.2.3_33
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

1;

