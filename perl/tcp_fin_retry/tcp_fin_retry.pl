use strict;
use warnings;

use Socket;

my $host = $ARGV[0];

my $sock;
socket( $sock, PF_INET, SOCK_STREAM, getprotobyname('tcp' ) )
    or die "Cannot create socket: $!";

my $remote_host = $host;
my $packed_remote_host = inet_aton( $remote_host )
    or die "Cannot pack $remote_host: $!";

my $remote_port = 80; 

my $sock_addr = sockaddr_in( $remote_port, $packed_remote_host )
    or die "Cannot pack $remote_host:$remote_port: $!";

connect( $sock, $sock_addr )
    or die "Cannot connect $remote_host:$remote_port: $!";

my $old_handle = select $sock;
$| = 1; 
select $old_handle;

print $sock "Hello";
print "Hello\n";

print "sleep 30\n";
sleep 30;

shutdown $sock, 1;

close $sock;

__END__

