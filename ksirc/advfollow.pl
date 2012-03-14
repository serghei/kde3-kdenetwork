#
#
&print("*** Loading advance follow...");

@K_VAL_COL = (0,2,3,4,5,6,7,8,9,10,11,12,13,14,15);

print "*** Using colours:";
foreach (@K_VAL_COL) {
    print "~$_ $_";
}
print "\n";

%K_FOLLOW = ();

$KADV_ON = 1;

sub sum_nick {
    my $n = shift;
    my $sum = 0;
    
    $n =~ s/^(.*)[-_|].*$/$1/;
    
    foreach (split(//, $n)) {
        $sum += ord;
    }
    return $sum;
        
}

sub hook_ksircadvfollow {
    if($_[0] =~ /<~n(\w+)~c>/){
        my $n = $1;
        return if $n eq $nick;
        if(!defined($K_FOLLOW{$n})){
            $K_FOLLOW{$n} = $K_VAL_COL [ &sum_nick($n) %  $#K_VAL_COL];
        }
        my $c = $K_FOLLOW{$n};
        $_[0] =~ s/<~n\w+~c>/<~$c$n~c>/;
    }
    elsif ($_[0] =~ /~\* (\w+) \w+/) {
        my $n = $1;
        return if $n eq $nick;
        if(!defined($K_FOLLOW{$n})){
            $K_FOLLOW{$n} = $K_VAL_COL [ &sum_nick($n) % $#K_VAL_COL];
        }
        my $c = $K_FOLLOW{$n};
        $_[0] =~ s/\* \w+ (\w+)/\* ~$c$n~c $1/;
    }
}
addhook("print", "ksircadvfollow");

$K_ADV_LENGTH = 0;

sub kadv_save {
    &kadv_load();
    return if($K_ADV_LENGTH == (scalar %K_FOLLOW));
        
    open(SAVE, ">$ENV{HOME}/.adv_follow") || return;
    my $n, $v;
    while(($n, $v) = each %K_FOLLOW){
        print SAVE "$n\t$v\n";
    }
    close SAVE;
    $K_ADV_LENGTH = scalar %K_FOLLOW;
}

sub kadv_load {
    open(SAVE, "<$ENV{HOME}/.adv_follow") || return;
    while(<SAVE>){
        chomp;
        my ($n, $v) = split(/\t/, $_);
        if(!defined($K_FOLLOW{$n})){
            $K_FOLLOW{$n} = $v;
        }    
        
    }
    close SAVE;
}

&kadv_load();
sub kadv_timer_save {
    &timer(300, "&kadv_timer_save()", 324325);
    &kadv_save();
}
&timer(300, "&kadv_timer_save()", 324325);


sub cmd_afflush {
    %K_FOLLOW = ();
}

addhelp("afflush", "Usaage: afflush\nDelete all coloured nicks");
addcmd("afflush");


sub cmd_afnick {
    &getarg;
    my $n = $newarg;
    &getarg;
    if($newarg){
        $K_FOLLOW{$n} = $newarg;
    }
    else {
        $K_FOLLOW{$n} = $K_VAL_COL [int (rand scalar (@K_VAL_COL))];
    }
}

addhelp("afnick", "Usage: afnick nick <col>\nResets the colour for nick.  If col is specefied it is set to col.  Random otherwise.");
addcmd("afnick");

sub cmd_afoff {
    remhook("print", "ksircadvfollow");
}
addhelp("afoff", "Usage: afoff\nTurn off advance follow");
addcmd("afoff");

sub cmd_afon {
    addhook("print", "ksircadvfollow");
}
addhelp("afon", "Usage: afon\nTurn on advange follow");
addcmd("afon");

print "*** Advance follow Loaded\n";
print "*** New commands: /afflush /afnick /afoff /afon\n";

