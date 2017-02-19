#!perl

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl filter_tweets.pl
#
# Program Description:
# Takes the original sentiment140 twitter data and for each message,
# finds out the referenced twitter handles and hashtags, and
# massages the content so as to remove all special characters.
# 
# Output:
# Two CSV file with NO header. The format for complete messages file is:
# 
# Complete messages file format:
#   1. When a user filter is applied, then:
#     $idx,$userid,$massaged,$handles,$hashtags
#
#   2. When to user filter is applied, then:
#     $userid,$massaged,$handles,$hashtags
#
# Social Network File format:
#   $userid,$handles,$hashtags
#
# NOTE: Make sure that the variables $inPath, $outPath and
# $userReferencesFile are correctly set.
#
#===============================================================

sub processFile($$$$);
sub load_userids($);
sub trim($);

my $isTest = 0;
my $isUserFiltered = 0;

my $inPath = "data/TwitterData/trainingandtestdata";
my $outPath = "data/TwitterData/parsed";

# Input Files
my $twitterFile = "$inPath/training.1600000.processed.noemoticon.csv";
my $userFile = "$outPath/training-filtered-users.csv";

# Output Files
my $outFile = "$outPath/training-msgs.csv";
my $userReferencesFile = "$outPath/training-user-references.csv";

if ($isTest == 1) {
	$twitterFile = "$inPath/testdata.manual.2009.06.14-mod.csv";
	$outFile = "$outPath/test-msgs.csv";
	$userReferencesFile = "$outPath/test-user-references.csv";
}

my $userids = {};
if ($isUserFiltered) {
	$userids = load_userids($userFile);
	print ("Loaded User ids...\n");
	for my $userid (keys %$userids) {
		my $idx = $userids->{$userid};
		#print ("$idx => $userid\n");
	}
}

processFile ($twitterFile, $outFile, $userReferencesFile, $userids);

sub processFile($$$$) {

	my $filePath = @_[0];
	my $outFile = @_[1];
	my $userReferencesFile = @_[2];
	my $userids = @_[3];
	
	my $isUserFiltered = 0;
	
	$file=open(INPT, "$filePath") || die("could not open file");
	unless (open(OUTF, ">$outFile")) {
		die("could not open file");
	}
	unless (open(NETOUTF, ">$userReferencesFile")) {
		die("could not open output network file");
	}

	my $line = ""; # <INPT>; # No Header line
	$line = <INPT>; # First row of data
	while ($line ne "") {
		
		my $userid = "";
		my $msg = "";
		$line = trim($line);
		if ($line =~ /^([^,]+,){4}([^,]+),(.*)/) {
			$userid = lc($2);
			$msg = $3;
			$msg =~ s/\"\"//gs;
			if ($userid =~ /^\"(.*)\"$/) {
				$userid = $1;
			}
			my $accept = 1;
			my $idx = -1;
			my $userFound = 0;
			if (exists $userids->{$userid}) {
				$userFound = 1;
				$idx = $userids->{$userid};
			}
			if ($isUserFiltered == 1 and $userFound == 0) {
				$accept = 0;
			}
			if ($accept == 1) {
				if ($msg =~ /^\"(.*)\"$/) {
					$msg = $1;
				}
				my $tmpmassaged = $msg;
				
				# now, we need to take out @xxx values since these are userids
				my $massaged = "";
				my $handles = "";
				my $hashtags = "";
				while ($tmpmassaged =~ /([^\@\#]*)(([\@\#])([A-Za-z0-9]+))?/gs) {
					$massaged = $massaged.$1;
					if ($3 eq "@" and $4 ne "") {
						if ($handles eq "") {
							$handles = $4;
						} else {
							$handles = $handles.";".$4;
						}
					} elsif ($3 eq "#" and $4 ne "") {
						if ($hashtags eq "") {
							$hashtags = $4;
						} else {
							$hashtags = $hashtags.";".$4;
						}
					}
				}
				$handles = lc($handles);
				$hashtags = lc($hashtags);
				
				# strip off '_'
				$massaged =~ s/_/ /gs;
				# strip off all non-word
				$massaged =~ s/\W/ /gs;
				# strip off all numeric
				$massaged =~ s/[0-9]+/ /gs;
				# replace multiple spaces with single space
				$massaged =~ s/  +/ /gs;
				# lower case
				$massaged = lc(trim($massaged));
				if ($massaged ne "") {
					if ($isUserFiltered == 1) {
						print OUTF ("$idx,$userid,$massaged,$handles,$hashtags\n");
					} else {
						print OUTF ("$userid,$massaged,$handles,$hashtags\n");
					}
				}
				if ($handles ne "" or $hashtags ne "") {
					print NETOUTF ("$userid,$handles,$hashtags\n");
				}
			}
		}
		
		$line = <INPT>;
		
	}
	close(INPT);
	close(OUTF);
	close(NETOUTF);
}

sub load_userids($) {

	my $file_path = @_[0];
	
	my $userids = {};
	
	my $file=open(INPT, "$file_path") || die("could not open file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),([^,]+)/igsm) {
			my $userid = $2;
			$userids->{$userid} = $1;
			#print ("$userid\n");
		}
		$line = <INPT>;
	}
	
	close(INPT);
	
	return $userids;
	
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
