#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl filter_tweets_for_groups.pl
#
# Program Description:
# Takes the file with all user groups and outputs only those
# tweets that are for the subset of users in the input groups file.
# 
# Output:
# CSV file with twitter messages for groups in input file.
# 
# Complete messages file format:
#     $groupidx,$user,$msg,$handles,$hashtags
#
# NOTE: Make sure that the variables $dirPath and $outPath and
# $twitterFile, $outFile, $outCountFile are correctly set.
#
#===============================================================

sub filterMessages($$$);
sub get_users($);
sub load_groups($);
sub trim($);

my $dirPath = "data/TwitterData/trainingandtestdata";
my $outPath = "data/TwitterData/parsed";
my $twitterFile = "$outPath/training-msgs-without-stopwords.csv";
my $groupsFile = "$outPath/training-filtered-groups.csv";
my $outMessagesFile = "$outPath/training-msgs-group-filtered.csv";
my $outCountFile = "$outPath/training-msgs-user-counts.csv";

my $groups = load_groups($groupsFile);
my $groupsz = (keys %$groups);
print ("total group records loaded: $groupsz\n");

my $users = get_users($groups);
my $usersz = (keys %$users);
print ("total user records loaded: $usersz\n");

my $usercounts = filterMessages($twitterFile, $outMessagesFile, $users);

unless (open(OUTF, ">$outCountFile")) {
	die("could not open file");
}
while (my ($user, $count) = each %$usercounts) {
	my $groupidx = $users->{$user};
	print OUTF ("$groupidx,$user,$count\n");
}
close(OUTF);

sub get_users($) {
	my $groups = @_[0];
	my $users = {};
	while (my ($groupidx, $group) = each %$groups) {
		while (my ($user, $dummy) = each %$group) {
			$users->{$user} = $groupidx;
		}
	}
	return $users;
}

sub filterMessages($$$) {

	my $filePath = @_[0];
	my $outFile = @_[1];
	my $users = @_[2];
	
	my $usercounts = {};
	
	my $file=open(INPT, "$filePath") || die("could not open file");
	unless (open(OUTF, ">$outFile")) {
		die("could not open file");
	}

	my $line = ""; # <INPT>; # No Header line
	$line = <INPT>; # First row of data
	while ($line ne "") {
		
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),(.*)/) {
			my $user = lc($1);
			my $msg = $2;
			my $refs = $3;
			if ($user =~ /^\"(.*)\"$/) {
				$user = $1;
			}
			if ($msg =~ /^\"(.*)\"$/) {
				$msg = $1;
			}
			
			if (exists $users->{$user}) {
				my $groupidx = $users->{$user};
				print OUTF ("$groupidx,$user,$msg,$refs\n");
				if (!exists $usercounts->{$user}) {
					$usercounts->{$user} = 1;
				} else {
					$usercounts->{$user} = $usercounts->{$user} + 1;
				}
			}
		}
		
		$line = <INPT>;
		
	}
	close(INPT);
	close(OUTF);
	return $usercounts;
}

sub load_groups($) {

	my $file_path = @_[0];
	
	my $groups = {};
	
	my $file=open(INPT, "$file_path") || die("could not open groups file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+)/igsm) {
			my $groupidx = $1;
			my $user = $2;
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
