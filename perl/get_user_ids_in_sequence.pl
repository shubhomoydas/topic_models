#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl get_user_ids_in_sequence.pl
#
# Program Description:
# Takes the feature vector file as input.
#
# Output: unique user ids in the order of occurence in the the input
# 
#===============================================================

sub trim($);

my $inPath = "data/TwitterData/parsed/featurevectors";
my $outPath = "data/TwitterData/parsed/featurevectors";

my $featureVectorFile = "$inPath/feature-vectors.csv";
my $userIdsFile = "$outPath/userids-in-sequence.csv";

my $file=open(INPT, "$featureVectorFile") || die("could not open input file");

unless (open(OUTF, ">$userIdsFile")) {
	die("could not open output file");
}

my $userids = {};
my $line = <INPT>;
while ($line ne "") {
	$line = trim($line);
	if ($line =~ /^([^,]+),([^,]+),.*/igsm) {
		my $userid = $2;
		if (!exists $userids->{$userid}) {
			print OUTF ("$userid\n");
			$userids->{$userid} = 1;
		}
	}
	$line = <INPT>;
}

close (INPT);
close (OUTF);

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
