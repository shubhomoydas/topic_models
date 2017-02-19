#!perl

#===============================================================
#
# IMPORTANT!!!
# ============
# 
# THIS CODE IS NOW OBSOLETE!!
#
# To run this use:
# cd git/topic_models/perl
# perl filter_users.pl
#
# NOTE: Make sure that the variables $dirPath and $outPath are
# correctly set.
#
#===============================================================

sub processFile($$$);
sub trim($);

$MIN_CNT = 20;

$dirPath = "data/TwitterData/parsed";
$fileName = "userCounts.csv";
$outPath = "data/TwitterData/parsed";
$outFile = "$outPath/filteredUsers.csv";

$file=open(INPT, "$dirPath/$fileName") || die("could not open file");
unless (open(OUTF, ">$outFile")) {
	die("could not open file");
}

my $line = ""; # <INPT>; # No Header line

$line = <INPT>; # First row of data
$idx = 1;
while ($line ne "") {

	my $userid = "";
	my $cnt = 0;
	$line = trim($line);
	if ($line =~ /^(.+),(.+)/) {
		$userid = $1;
		$cnt = $2;
		if ($cnt >= $MIN_CNT) {
			print OUTF ("$idx,$line\n");
			$idx = $idx + 1;
		}
	}

	$line = <INPT>;

}
close(INPT);
close(OUTF);


# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
