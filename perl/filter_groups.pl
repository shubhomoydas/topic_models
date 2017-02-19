#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl filter_groups.pl
#
# Program Description:
# Takes the complete set of user groups identified in the previous
# process and returns a subset based on the group size.
# 
# Output:
# CSV file with filtered groups
# 
# Complete messages file format:
#     $groupidx,$user
#
# NOTE: Make sure that the variables $dirPath and $outPath and
# $groupsFile are correctly set.
#
#===============================================================

sub load_groups($);
sub trim($);

my $inPath = "data/TwitterData/parsed";
my $outPath = "data/TwitterData/parsed";
my $groupsFile = "$inPath/training-user-groups.csv";
my $outFile = "$outPath/training-filtered-groups.csv";

my $minGroupSize = 150;

my $groups = load_groups($groupsFile);
my $groupsz = (keys %$groups);
print ("total group records loaded: $groupsz\n");

unless (open(OUTF, ">$outFile")) {
	die("could not open file");
}

my $grpNum = 1;
while (my ($groupidx, $group) = each %$groups) {
	my $size = (keys %$group);
	if ($size >= $minGroupSize) {
		while (my ($user, $dummy) = each %$group) {
			print OUTF ("$grpNum,$user\n");
		}
		$grpNum = $grpNum + 1;
	}
}

close(OUTF);

sub load_groups($) {

	my $file_path = @_[0];
	
	my $groups = {};
	
	my $file=open(INPT, "$file_path") || die("could not open groups file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),([^,]+)/igsm) {
			my $groupidx = $1;
			my $user = $2;
			my $size = $3;
			if (!exists $groups->{$groupidx}) {
				$groups->{$groupidx} = {};
			}
			my $group = $groups->{$groupidx};
			$group->{$user} = 1;
		}
		$line = <INPT>;
	}
	
	close(INPT);
	
	return $groups;
	
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
