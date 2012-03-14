#
#
&print("*** Loading auto dcc...");

%A_DCC = ();

sub cmd_anick {
    &getarg;
    $A_DCC{$newarg} = 1;    
    &print("*\cbI\cb* Added $newarg to auto dcc");
}

&addcmd("anick");

sub cmd_dnick {
    &getarg;
    delete $A_DCC{$newarg};
    &print("*\cbI\cb* Removed $newarg from auto dcc");
}

&addcmd("dnick");

sub cmd_lnick {
    my $key;
    my $str;
    foreach $key (keys %A_DCC){
        $str .= "$key ";
    }
    &print("*\cbI\cb* Nicks allowed to auto dcc: $str");
}

&addcmd("lnick");

sub set_autoresume {
 $set{'AUTORESUME'}="on", $a_autoresume=1 if $_[0] =~ /^on$/i;
 $set{'AUTORESUME'}="off", $a_autoresume=0 if $_[0] =~ /^off$/i;
}
&addset("autoresume");
$set{"AUTORESUME"}="on";
$n_autoresume=1;

my %A_AUTOSTART = ();

sub hook_adcc_do {
    my $cmd = shift;
    
    if($cmd eq "SEND"){
        if($A_DCC{$who} == 1){
            my $host = shift;
            my $dport = shift;
            my $file = shift;
            my $size = shift;
            if(-e $file){
                if(($n_autoresume == 1) && ((-s $file) < $size)){
                    &docommand("resume $who $file");
                    $A_AUTOSTART{$dport} = "dcc get $who $file";
                }
                else {
                    &tell("*E* Autodcc rejected.  File exists and no auto resume, or size mismatch");
                }                    
            }
            else {
                &docommand("dcc get $who $file");
            }
        }
    }
}

&addhook("dcc_request", "adcc_do");


