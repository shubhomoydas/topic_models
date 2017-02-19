#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl generate_random_groups.pl
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
my $outPath = "data/TwitterData/lda-runs/random-groups";

my $idx2useridFile = "$inPath/index-to-user.csv";

my $idx2userid = load_idx2userid($idx2useridFile);
my @userids = keys %$idx2userid;
my $nusers = @userids;
print ("Loaded $nusers users...\n");

my $ngroups = 100;

srand(42);

for (my $i = 0; $i < 500; $i++) {
	unless (open(GRPF, ">$outPath/random-groups2users-$i.csv")) {
		die("could not open output file");
	}
	while (my ($useridx, $userid) = each %$idx2userid) {
		my $group = int(rand($ngroups));
		print GRPF ("$group,$userid\n");
	}
	close (GRPF);
}

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
