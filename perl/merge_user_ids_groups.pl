#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl merge_user_ids_groups.pl
#
# Program Description:
# Takes the user ids as they appear in sequence in feature vectors
# and the corresponding groups inferred.
#
# Output: unique user ids in the order of occurence in the the input
# 
#===============================================================

sub load_idx2userid($);
sub trim($);

my $inPath = "data/TwitterData/parsed/featurevectors";
my $outPath = "data/TwitterData/lda-runs/lda-2-1-inferGroups";

my $userIdsFile = "$inPath/userids-in-sequence.csv";
my $idx2useridFile = "$inPath/index-to-user.csv";
my $groupAssignmentsFile = "$outPath/groups-2000.csv";
my $groups2userFile = "$outPath/groups2users.csv";

my $idx2userid = load_idx2userid($idx2useridFile);
my @userids = keys %$idx2userid;
my $nusers = @userids;
print ("Loaded $nusers users...\n");

my $file=open(USRINPT, "$userIdsFile") || die("could not open input file");
my $file=open(GRPINPT, "$groupAssignmentsFile") || die("could not open input file");

unless (open(OUTF, ">$groups2userFile")) {
	die("could not open output file");
}

my $uline = <USRINPT>;
my $gline = <GRPINPT>;
while ($uline ne "" && $gline ne "") {
	$uline = trim($uline);
	$gline = trim($gline);
	my $useridx = $uline;
	my $userid = $idx2userid->{$useridx};
	if ($gline =~ /^([^,]+),([^,]+)/igsm) {
		my $groupid = $2;
		print OUTF ("$groupid,$userid\n");
	}
	$uline = <USRINPT>;
	$gline = <GRPINPT>;
}

close (USRINPT);
close (GRPINPT);
close (OUTF);

sub load_idx2userid($) {

	my $file_path = @_[0];
	
	my $idx2userid = {};
	
	my $file=open(INPT, "$file_path") || die("could not open groups file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+)/igsm) {
			my $useridx = $1;
			my $userid = $2;
			$idx2userid->{$useridx} = $userid;
		}
		$line = <INPT>;
	}
	
	close(INPT);
	
	return $idx2userid;
	
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
