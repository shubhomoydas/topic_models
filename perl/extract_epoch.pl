#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl extract_epoch.pl
#
# Program Description:
# Takes the inferred parameter file(s) and outputs a subset of rows that
# belong to a particular epoch.
#   $epoch,...
# 
# Output: Same format of csv as input
# 
# 
# NOTE: Make sure that the variables $inPath and $outPath are
# correctly set.
#
#===============================================================

sub trim($);

my $epoch = 2000;
my $filenameprefix = "taus";
my $runid = "lda-0-1";

my $inPath = "data/TwitterData/lda-runs/$runid";
my $outPath = "data/TwitterData/lda-runs/topics";

my $infile = "$inPath/${filenameprefix}.csv";
my $outfile = "$outPath/${filenameprefix}-$runid-$epoch.csv";

my $file=open(INPT, "$infile") || die("could not open input file");
unless (open(OUTF, ">$outfile")) {
	die("could not open file");
}

my $line = ""; # <INPT>; # No Header line
$line = <INPT>; # First row of data
while ($line ne "") {
	#$line = trim($line);
	if ($line =~ /^$epoch,(.*)/) {
		print OUTF ("$epoch,$1\n");
	}
	$line = <INPT>;
}

close (INPT);
close (OUTF);

sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
