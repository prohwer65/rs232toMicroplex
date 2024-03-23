#!/usr/bin/env perl
#===============================================================================
#
#         FILE: sendCommands.pl
#
#        USAGE: ./sendCommands.pl  
#
#  DESCRIPTION: 
#
#      OPTIONS: ---
# REQUIREMENTS: ---
#         BUGS: ---
#        NOTES: ---
#       AUTHOR: Paul Rohwer (PWR), prohwer@mindspring.com
# ORGANIZATION: PowerAudio
#      VERSION: 1.0
#      CREATED: 3/9/2024 2:25:14 PM
#     REVISION: ---
#===============================================================================

use strict;
use warnings;
use utf8;
use FindBin qw{$Script };
#use FindBin qw{$Bin $Script $RealBin $RealScript $Dir $RealDir};

my $VERSION        = '';

my $DOCUMENTATION = <<EOMESSAGE;
Please describe what this program does

Usage : $Script [-he [-w Z] [-d X] [-f N] 

    Options     : Descriptions
    --------      ------------------------------------------------------
    -h          : Help menu
    -e          : Enable something
    -d  drive   : Option with agrument
    -w  win     : Option with agrument
    --help      : Help Menu
    --version   : Version 

EOMESSAGE

use Getopt::Std;
use POSIX ":sys_wait_h";
use English '-no_match_vars';
    # see perlvar for variable names and features
    # no_match to reduce regx effiecency loss
our $OS_win;


BEGIN {
        $OS_win = ($^O eq "MSWin32") ? 1 : 0;

        print "Perl version: $]\n";
        print "OS   version: $^O\n";

            # This must be in a BEGIN in order for the 'use' to be conditional
        if ($OS_win) {
            print "Loading Windows module\n";
            eval "use Win32::SerialPort";
	    die "$@\n" if ($@);

        }
        else {
            print "Loading Unix module\n";
            eval "use Device::SerialPort";
	    die "$@\n" if ($@);
        }
} # End BEGIN

#use Win32::SerialPort qw( :STAT 0.19 );

#use File::stat;
#use File::Copy;
use Config;
use Data::Dumper;


#use Readonly;
#Readonly my $PI => 3.14;

# ------------------------------------------------------------------------------
# BEGIN
# ------------------------------------------------------------------------------
#BEGIN {
#}

# ------------------------------------------------------------------------------
# INIT
# ------------------------------------------------------------------------------
#INIT {
#}

# ------------------------------------------------------------------------------
# END
# ------------------------------------------------------------------------------
#END {
#}

# ------------------------------------------------------------------------------
# CHECK
# ------------------------------------------------------------------------------
#CHECK {
#}

# ------------------------------------------------------------------------------
# declare sub  <+SUB+>
# ------------------------------------------------------------------------------
#
sub changeLevels;
sub changeNumberOfChannels ;
sub changeNumberOfChannelsTest ;
sub blackout ;
sub fadeToBlack ;
sub fadeToLevel ;
sub bumpTest ;
sub passing_argu_3orless;
sub passing_argu_4ormore;
sub HELP_MESSAGE();
sub VERSION_MESSAGE();


# ------------------------------------------------------------------------------
# global variables
# ------------------------------------------------------------------------------

my $OS;
my %cmdLineOption;
getopts( "hp:f:", \%cmdLineOption );
    #	<+INPUTOPTIONS+>

 # examples of direct associating
my @ARRAY = qw(0  2 3 4 5 6 7 8 9   17 19 20 21 23 25);
my %HASH  = ( somevalue => 'as', );

my $ref_ARRAY = [ qw(0  2 3 4 5 6 7 8 9   17 19 20 21 23 25)] ;
my $ref_HASH  = { somevalue => 'as', another => "value", };
# ------------------------------------------------------------------------------
# Database of values;
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# parse command line and setup defaults
# ------------------------------------------------------------------------------
if ( $Config{'osname'} =~ /Win/ ) {
    $OS = "Windows";
}

local $Data::Dumper::Sortkeys = 1;
local $Data::Dumper::Purity   = 1;  ##new to verify this

#print Data::Dumper->Dump( [ \%Config ], [qw(Config  )] );

if ( defined $cmdLineOption{h} ) {
    HELP_MESSAGE();
    exit(15);
}

my $PortName = "COM1";
if ( defined $cmdLineOption{p} )  {
 $PortName =   $cmdLineOption{p} ;
 #<+INPUTOPTIONS+>
}

#if ( defined $cmdLineOption{f} )  {
# something =   $cmdLineOption{f} ;
#	<+INPUTOPTIONS+>
#}

# ------------------------------------------------------------------------------
#  MAIN part of program
# ------------------------------------------------------------------------------

#<+MAIN+>
my $time = localtime();
print "$time\n";

my $quiet = 0;

my $PortObj;
if ($OS_win) {
    $PortObj = new Win32::SerialPort ($PortName, $quiet) || die "Can't open $PortName: $^E\n";    # $quiet is optional
} else {
    $PortObj = Device::SerialPort->new ($PortName,1);
}


   # similar
$PortObj->baudrate(9600);
$PortObj->parity("none");
$PortObj->databits(8);
$PortObj->stopbits(1);
$PortObj->handshake("none");           # set parameter
$PortObj->binary(1);          
$PortObj->debug(1);

  $PortObj->error_msg(1);  # prints hardware messages like "Framing Error"
  $PortObj->user_msg(1);   # prints function messages like "Waiting for CTS"

  $PortObj->status();

my @handshake_opts = $PortObj->handshake; 
print "Handshakes " . join( ", ", @handshake_opts ) . "\n";
   # range parameters return (minimum, maximum) in list context
   #$PortObj->xon_limit(100);      # bytes left in buffer
   #$PortObj->xoff_limit(100);     # space left in buffer
   #$PortObj->xon_char(0x11);
   #$PortObj->xoff_char(0x13);
   #$PortObj->eof_char(0x0);
   #$PortObj->event_char(0x0);
   #$PortObj->error_char(0);       # for parity errors
 
   #$PortObj->buffers(4096, 4096);  # read, write
      # returns current in list context
 
      #$PortObj->read_interval(100);    # max time between read char (milliseconds)
      #$PortObj->read_char_time(5);     # avg time between read char
      #$PortObj->read_const_time(100);  # total = (avg * bytes) + const
      #$PortObj->write_char_time(5);
      #$PortObj->write_const_time(100);
 
   # true/false parameters (return scalar context only)
 
$PortObj->binary(1);          # just say Yes (Win 3.x option)
#$PortObj->parity_enable(F);   # faults during input



my $rampTime = 20;
my $level    = 159;
my $numOfChannels = 24;

while (1) {
    $PortObj->status();
    print "1.  Blackout Test\n";
    print "2.  Fade To Black Test\n";
    print "3.  Bump sequencial channels to ". $level . " Test\n";
    print "4.  Set All Channels to  ". $level . "  Test\n";
    print "5.  Fade All Channels to ". $level . " Test\n";
    print "6.  Change Number of channels from (". $numOfChannels. ") \n";
    print "7.  Change Ramp Time from (". $rampTime .")\n";
    print "8.  Change Level from (". $level .")\n";
    print "Q.  Quit \n";
    
    
    my $choice = readline ( STDIN); 
    chomp $choice;
    
    if ( $choice =~ "Q"  || $choice =~ "q" || $choice == "0"  ) {
        exit 0;
    } elsif ( $choice ==  "1" ) {
        blackout();
    
    } elsif ( $choice == "2" ) {
        fadeToBlack( $rampTime );
    
    } elsif ( $choice == "3" ) {
        bumpTest( $level );
    } elsif ( $choice == "4" ) {
    
        changeLevels( $level );
    } elsif ( $choice == "5" ) {
        fadeToLevel( $rampTime, $level ) ;
    } elsif ( $choice == "6" ) {
        $numOfChannels = changeNumberOfChannels();
        changeNumberOfChannelsTest( $numOfChannels );
    } elsif ( $choice == "7" ) {
        $rampTime = changeRampTime();
    } elsif ( $choice == "8" ) {
        $level = changeNewLevel();
    }
    
}







#
#CPAN shell
#
#perl -MCPAN -e shell
#install Win32::SerialPort
#
#











exit 0;
















#passing_argu_3orless( 1, 2, 3 );
#my $runFunction = \&passing_argu_3orless;
#
#&{ $runFunction }(4, 5, 6);
## or [\&passing_argu_3orless, 4, 5, 6 ]   # when passing a agru to another function like Tk's -command =>  
#
#
#passing_argu_4ormore( { text => "test", cols => 20, centered => 1, } );
#
#my $sc = returnScalar();
#my $ar = returnArray();
#my $ha = returnHash();
#
#my $b = 0;
#my $a = ref( \$b);
#
#print "Return Scalar ". $a ." \n";
#print "Return Array ". ref( $ar ) ." \n";
#print "Return Hash ". ref( $ha ) ." \n";
#print "Return Hash ". scalar( $ha ) ." \n";
#

#print Data::Dumper->Dump( [ \$time, \@ARRAY, \%cmdLineOption, \%HASH ], [qw(time   ARRAY    cmd_line_option    HASH )] );

exit 0;

# ------------------------------------------------------------------------------
# Define subroutines
#     <+SUB+>
# ------------------------------------------------------------------------------


sub changeLevels {
    my $newLevel = shift || 159;
    $PortObj->write( 0xAA);         # send Fade command;
    $PortObj->write( 0x00);         # starting at address 00h
    for( my $i       = 0; $i<$numOfChannels; $i++ ) {
        $PortObj->write( atoi($newLevel));
    }

}

sub changeNumberOfChannelsTest {
    my $newNumberOfChannels = shift || 24;
    $PortObj->write( 0xA7, atoi($newNumberOfChannels));     # send Black out command;
    $numOfChannels = $newNumberOfChannels;

}

sub blackout {

    my $i = shift;


    $PortObj->write( 0xBB);     # send Black out command;

    #sleep( 5);



}

sub fadeToBlack {
    my $rampSpeed= shift || 20;         # number of half seconds to ramp
    $PortObj->write( 0xFB);             # send Fade to Black;
    $PortObj->write( atoi($rampSpeed));       # half-seconds 

}


sub fadeToLevel {
    my $rampSpeed = shift || 20;
    my $newLevel = shift || 159;

    $PortObj->write( 0xFA);         # send Fade command;
    $PortObj->write( atoi($rampSpeed));   # half-seconds 
    $PortObj->write( 0x00);         # starting at address 00h
    for( my $i       = 0; $i<$numOfChannels; $i++ ) {
        $PortObj->write( atoi($newLevel));
    }

}

sub bumpTest {

    my $newLevel = shift || 159;

    for( my $i       = 0; $i<$numOfChannels; $i++ ) {
        $PortObj->write( 0xBB);     # send Black out command;


        print "Changing Channel " . ($i+1) . " to new level: ". $newLevel . "\n";
        $PortObj->write( 0xAA);         # send Fade command;
        $PortObj->write( atoi($i));           # starting at address 00h
        $PortObj->write( atoi($newLevel));
        sleep( 2);
    }

}


sub changeNumberOfChannels {
    
    my $newValue;
    my $attempts=0;
    do {
        print "Invalid value\n" if ( $attempts++);
        return $numOfChannels if ( $attempts == 4);
        print "New number of channels (6-48) ";
        $newValue = <STDIN>;
        chomp $newValue;

        
    } while ( $newValue < 6 || $newValue >48 );

    return ( $newValue );
}



sub changeRampTime {
    
    my $newValue;
    my $attempts=0;
    do {
        print "Invalid value\n" if ( $attempts++);
        return $rampTime if ( $attempts == 4);
        print "New ramp time in half-seconds (0-20) ";
        $newValue = <STDIN>;
        chomp $newValue;

        
    } while ( $newValue < 0 || $newValue >20 );

    return ( $newValue );
}
sub changeNewLevel {
    
    my $newValue;
    my $attempts=0;
    do {
        print "Invalid value\n" if ( $attempts++);
        return $level if ( $attempts == 4);
        print "New brightness level (0-159) ";
        $newValue = <STDIN>;
        chomp $newValue;
        print "\nvalue is $newValue\n";

        
    } while ( $newValue < 0 || $newValue >159 );

    return ( $newValue );
}



sub atoi {
    my $i = shift;
    return (int $i);
}

sub passing_argu_3orless {

    # unpack input arguments
    my ( $first, $second, $third ) = @_;

    print "passing_argu_3orless()\n";
    print "First arg is $first, then $second, and $third\n";
}

sub passing_argu_4ormore() {

    # when passing in several input arguments, use a hash
    my ($in_argu_ref) = @_;

    print "passing_argu_4ormore()\n";
    if ( !defined $in_argu_ref->{junk} ) {
        # set an agrument to default value if not defined. 
        $in_argu_ref->{junk} = "JUNK";
    }

    foreach my $key (keys %$in_argu_ref) {
        print "$key -> $in_argu_ref->{$key}, ";
    }
    print "\n";

    #print " $in_argu_ref->{cols};\n";
    #print " $in_argu_ref->{centered};\n";
} # end of passing_argu_4ormore


sub returnScalar {
    my $a = 0;
    return \$a;
}

sub returnArray {
    my @a = qw/ 1 2 3/; 
    return \@a;
}

sub returnHash {
    my %a = ( 'a' => 'asdf');

    return \%a;
}

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
sub HELP_MESSAGE() {

    print <<EOTEXT;
-----------------------------------------------------------------------------
$Script - TITLE
$DOCUMENTATION

$^X
EOTEXT
    exit(1);
}

# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
sub VERSION_MESSAGE() {
    $Getopt::Std::STANDARD_HELP_VERSION = 1;

    # The above prevents this function from running exit();
    # but it causes a false warning, therefore I print it.
    print "$Script :  $VERSION $Getopt::Std::STANDARD_HELP_VERSION \n";
}


##################################################################
#                                                                #
#                                                                #
#                                                                #
##################################################################

# }}}1
# vim:tabstop=4:si:expandtab:shiftwidth=4:shiftround:set foldenable foldmethod=marker:


