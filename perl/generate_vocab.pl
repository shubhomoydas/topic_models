#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl generate_vocab.pl
#
# Program Description:
# Takes the file with preprocessed twitter messages. Expects the
# file to have the CSV format with NO headers:
#   $groupidx,$user,$msg,$handles,$hashtags
# 
# Output:
# CSV file in the format:
#   $wordidx,$word,$cnt
# 
# NOTE: Make sure that the variables $inPath and $outPath and
# $twitterFile, $outFile are correctly set.
#
#===============================================================

sub load_vocab($);
sub trim($);

my $inPath = "data/TwitterData/parsed";
my $outPath = "data/TwitterData/parsed/featurevectors";
my $twitterFile = "$inPath/training-msgs-group-filtered.csv";
my $outFile = "$outPath/vocab.csv";

my $MIN_CNT = 60;
my $vocab = load_vocab($twitterFile);

my $len = scalar keys %$vocab;

print ("Loaded Vocab. Len=$len...\n");

unless (open(OUTF, ">$outFile")) {
	die("could not open file");
}
my $wordidx = 0;
foreach my $word (sort { ($vocab->{$b} <=> $vocab->{$a}) } keys %$vocab) {
	my $cnt = $vocab->{$word};
	if ($cnt >= $MIN_CNT) {
		print OUTF ("$wordidx,$word,$cnt\n");
		$wordidx = $wordidx + 1;
	}
}
close(OUTF);

sub load_vocab($) {

	my $file_path = @_[0];
	
	my $vocab = {};
	
	my $file=open(INPT, "$file_path") || die("could not open file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),([^,]+),(.*)/igsm) {
			my $message = $3;
			my @tokens = split(/ /,$message);
			foreach my $token (@tokens) {
				if ($token eq "") {
					next;
				}
				if (exists $vocab->{$token}) {
					$vocab->{$token} = $vocab->{$token} + 1;
				} else {
					$vocab->{$token} = 1;
				}
			}
		}
		$line = <INPT>;
	}
	
	close(INPT);
	
	return $vocab;
	
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
