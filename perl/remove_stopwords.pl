#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl remove_stopwords.pl
#
# Program Description:
# Takes the twitter dataset which has already been cleaned of hashtags
# and handles and also other special characters.
# 
# Output:
# CSV file with stopwords removed from the twitter messages.
# 
# Complete messages file format:
#     $userid,$massaged,$handles,$hashtags
#
# NOTE: Make sure that the variables $dirPath and $outPath and
# $twitterFile, $stopwordsFile are correctly set.
#
#===============================================================

sub processFile($$$);
sub load_stopwords($);
sub trim($);

my $isTest = 0;
my $dirPath = "data/TwitterData/trainingandtestdata";
my $outPath = "data/TwitterData/parsed";
my $twitterFile = "$outPath/training-msgs.csv";
my $outFile = "$outPath/training-msgs-without-stopwords.csv";
my $stopwordsFile = "./stopwords.txt";

my $stopwords = load_stopwords($stopwordsFile);
my $stopsz = (keys %$stopwords);
print ("total stopwords loaded: $stopsz\n");

if ($isTest == 1) {
	$twitterFile = "$outPath/test-msgs.csv";
	$outFile = "$outPath/test-msgs-without-stopwords.csv";
}

processFile($twitterFile, $outFile, $stopwords);

sub processFile($$$) {

	my $filePath = @_[0];
	my $outFile = @_[1];
	my $stopwords = @_[2];
	
	my $file=open(INPT, "$filePath") || die("could not open file");
	unless (open(OUTF, ">$outFile")) {
		die("could not open file");
	}

	my $line = ""; # <INPT>; # No Header line
	$line = <INPT>; # First row of data
	while ($line ne "") {
		
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),(.*)/) {
			my $userid = lc($1);
			my $msg = $2;
			my $refs = $3;
			$msg =~ s/\"\"//gs;
			$msg =~ s/((.{1})\2+)/$2$2/g;
			if ($userid =~ /^\"(.*)\"$/) {
				$userid = $1;
			}
			if ($msg =~ /^\"(.*)\"$/) {
				$msg = $1;
			}
			my $tmpmassaged = $msg;

			my $massaged = "";
			my @msgwords = split(/ /, $msg);
			for my $word (@msgwords) {
				$word = trim($word);
				if (!exists $stopwords->{$word}) {
					$massaged = $massaged." ".$word;
				}
			}

			$massaged = lc(trim($massaged));
			if ($massaged ne "") {
				print OUTF ("$userid,$massaged,$refs\n");
			}
		}
		
		$line = <INPT>;
		
	}
	close(INPT);
	close(OUTF);
}

sub load_stopwords($) {

	my $file_path = @_[0];
	
	my $stopwords = {};
	
	my $file=open(INPT, "$file_path") || die("could not open file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^(.+)/igsm) {
			my $stopword = $1;
			$stopwords->{$stopword} = $stopword;
		}
		$line = <INPT>;
	}
	
	close(INPT);
	
	return $stopwords;
	
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
