#!/usr/bin/perl

use strict;
use Data::Dumper;
use MyCmp;

#usage:
#    perl my_cmp.pl  name-devel-1.7.0e-1.10_3.el6_20 name-devel-1.7.0e-1.10_3.el6_1

sub split_name_ver_rel{
        my($rpm) = @_; 
        $rpm =~ m/^(.*)-([^-]+)-([^-]+)$/;
        return {name=>$1, ver=>$2, rel=>$3};
}

print MyCmp::my_cmp( split_name_ver_rel( shift ), split_name_ver_rel( shift ) );

