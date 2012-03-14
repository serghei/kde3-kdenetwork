#!/usr/bin/perl
# convert which file to which file?
# I did nto bother to build in any checks. so that file better exists and it is readable and the data will
# be written to a writeable directory. Since this is an old config file, it is safe to assume, that is the 
# case.

#$file = $ARGV[0];

# Take your hands off the rest unless you darn well knwo what you are doing
##################################################################################
##################################################################################

open(INFO, $file);  
 my ($section, %data);

#read in all the data, split it up into hashes. Thanks again to malte for much input
while (<>) {
	if (/\[(.*)\]/) {
		$sections{$section} = {%data} if $section;
		$section = $1;
		undef %data;
		next;
 	}
$data{$1} = $2 if /^([^=]*)=(.*)$/;
}

$sections{$section} = {%data} if $section;

# do the data writing magic
#first of all be check how many old news souerces we have

$sources = $sections{'General'}->{'News sources'}; 
#gather all news sources into a very pretty string before we write the global section
#also give some feedback to the user
for my $i (0..($sources-1)) { 
	$all .= "," .$sections{"News source #$i"}->{'Name'}; 
	$all =~s/^,//;
}

# write the main section
print "[KNewsTicker]\n";

while (($key,$dat) = each(%{$sections{'General'}})) {
	if ($key ne "News sources") { 
		if ($key eq "Interval") {
			$key="Update interval";
				}
		if ($key eq "Scroll most recent only") {
			$key="Scroll most recent headlines only";
				}
		print "$key=$dat\n";
		} else {
		print "News sources=".$all."\n";
		}	

}

# next merge the old scrolling section into the KNewticker Section
while (($key,$dat) = each(%{$sections{'Scrolling'}})) {
	 if ($key eq "Background") {
                $key="Background color";
		
                } 
	 if ($key eq "Foreground") {
                $key="Foreground color";
                }
	 if ($key eq "Highlighted") {
                $key="Highlighted color";
                }
	 if ($key eq "Underline highlighted") {
                $key="Underline highlighted headlines";
                }
	 if (($key eq "Direction") && ($dat eq "Left")) {
		$key ="Scrolling direction";
		$dat = "0";	

} 
if (($key eq "Direction") && ($dat ne "Left")) {

		$key ="Scrolling direction";

}
if ($key eq "Speed") {
	$key="Scrolling speed";
}



 print "$key=$dat\n"; 
} 

# next we write the news sources, making sure we have the correct headers
for my $i (0..($sources-1)) { 

print "\n[" .$sections{"News source #$i"}->{'Name'} ."]\n";
	while (($key,$dat) = each(%{$sections{"News source #$i"}})) {
	if ($key ne "Address") {
		print "$key=$dat\n";
		} else {
		print "Source file=".$dat."\n";
	}
	} 
}

