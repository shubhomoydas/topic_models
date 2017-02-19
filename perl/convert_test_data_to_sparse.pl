#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl convert_test_data_to_sparse.pl
#
# Program Description:
# Takes the synthetic test file in dense format and convertes it to sparse format.
#
# Input: CSV file
#   - $userid,csv_bag_of_words
# 
# Output:
#   - Docs in sparse format
#   - Vocab in CSV
#     $wordidx,$word,$wordcount
# 
#
#===============================================================

sub getIdx2ref($);
sub trim($);

my $inPath = "git/topic_models//sampledata";
my $outPath = "git/topic_models//sampledata";

# Input files...
my $dataFile = "$inPath/data_true_multinomial.csv";

# Output files...
my $sparseFile = "$outPath/data_true_multinomial-sparse.csv";
my $vocabFile = "$outPath/data_true_multinomial-vocab.csv";

my $groups = {};
for (my $i = 1; $i <= 30; $i = $i + 1) {
	$groups->{$i} = int(($i-1)/10) + 1;
}

my $vocab = {};
my $vocabcounts = {};
my $maxwidx = 0;

my $groupcounts = {};
my $featurevectors = {};

my $file=open(INPT, "$dataFile") || die("could not open input file");

my $line = ""; # <INPT>; # No Header line
$line = <INPT>; # First row of data
while ($line ne "") {
	$line = trim($line);
	if ($line =~ /^([^,]+),(.*)/) {
		my $user = $1;
		my $groupidx = $groups->{$user};
		my $vec = $2;
		my @tokens = split(/,/,$vec);
		my $wordcount = 0;
		my $doc = {};
		my $idx = 0;
		foreach my $wval (@tokens) {
			my $word = "w$idx";
			if (!exists $vocab->{$word}) {
				$vocab->{$word} = $maxwidx;
				$vocabcounts->{$maxwidx} = 0;
				$maxwidx = $maxwidx + 1;
			}
			my $widx = $vocab->{$word};
			if ($wval > 0) {
				$doc->{$widx} = $wval;
				$wordcount = $wordcount + $wval;
				$vocabcounts->{$widx} = $vocabcounts->{$widx} + $wval;
			}
			$idx = $idx + 1;
		}
		if ($wordcount > 0) {
			if (!exists $groupcounts->{$groupidx}) {
				$groupcounts->{$groupidx} = {};
			}
			my $group = $groupcounts->{$groupidx};
			if (!exists $group->{$user}) {
				$group->{$user} = 1;
			} else {
				$group->{$user} = $group->{$user} + 1;
			}
			if (!exists $featurevectors->{$user}) {
				$featurevectors->{$user} = [];
			}
			my $userdata = $featurevectors->{$user};
			push (@$userdata, $doc);
		}
	}
	$line = <INPT>;
}

close (INPT);

print ("Input data loaded...\n");

unless (open(SOUTF, ">$sparseFile")) {
	die("could not open output sparse file");
}

foreach my $groupidx (sort { $a <=> $b } keys %$groupcounts) {
	my $group = $groupcounts->{$groupidx};
	my $groupsize = scalar keys %$group;
	foreach my $user (sort { $a <=> $b } keys %$group) {
		my $userdata = $featurevectors->{$user};
		my $doccount = @$userdata;
		foreach my $doc (@$userdata) {
			print SOUTF ("$groupidx,$user");
			foreach my $wordidx (sort { $a <=> $b } keys %$doc) {
				my $wordcount = $doc->{$wordidx};
				print SOUTF (",$wordidx:$wordcount");
			}
			print SOUTF ("\n");
		}
	}
}

close (SOUTF);

unless (open(VOUTF, ">$vocabFile")) {
	die("could not open output vocab file");
}

my $idx2word = getIdx2ref($vocab);
foreach my $widx (sort { $a <=> $b } keys %$idx2word) {
	my $word = $idx2word->{$widx};
	my $wordcount = $vocabcounts->{$widx};
	print VOUTF ("$widx,$word,$wordcount\n");
}

close (VOUTF);

sub getIdx2ref($) {
	my $refs = @_[0];
	my $idxs = {};
	while (my ($ref, $idx) = each %$refs) {
		$idxs->{$idx} = $ref;
	}
	return $idxs;
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
