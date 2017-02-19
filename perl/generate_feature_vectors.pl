#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl generate_feature_vectors.pl
#
# Program Description:
# Takes the file with all user groups and outputs only those
# tweets that are for the subset of users in the input groups file.
# Input file should be a CSV file with NO headers in the format:
#   $groupidx,$user,$msg,$handles,$hashtags
# 
# Output:
# Three CSV files will be generated:
# 1. feature vector file
# 2. Count of users in each group
#    $groupidx,$usercount
# 3. Count of messages for each user
#    $useridx,$user,$messagecount
#
# NOTE: Make sure that the variables $dirPath and $outPath and
# $twitterFile, $vocabFile, $featuresFile, $outCountFile are correctly set.
#
#===============================================================

sub featurize($$);
sub getIdx2ref($);
sub load_vocab($);
sub trim($);

my $inPath = "data/TwitterData/parsed";
my $outPath = "data/TwitterData/parsed/featurevectors";

my $twitterFile = "$inPath/training-msgs-group-filtered.csv";
my $vocabFile = "$outPath/vocab.csv";

my $featuresFile = "$outPath/feature-vectors.csv";
my $groupCountsFile = "$outPath/group-vector-counts.csv";
my $userCountsFile = "$outPath/user-vector-counts.csv";

my $idx2useridFile = "$outPath/index-to-user.csv";

my $verify = 1;
my $verifyFeaturesFile = "$outPath/verify-feature-vectors.csv";

my $vocab = load_vocab($vocabFile);
my $vocabsz = (keys %$vocab);
print ("total vocab words loaded: $vocabsz\n");

my ($groupcounts, $useridxs, $featurevectors) = featurize($twitterFile, $vocab);

my $idx2user = getIdx2ref($useridxs);
my $idx2word = getIdx2ref($vocab);

unless (open(USRIDXF, ">$idx2useridFile")) {
	die("could not open features file");
}
while (my ($user, $idx) = each %$useridxs) {
	print USRIDXF ("$idx,$user\n");
}
close (USRIDXF);

unless (open(OUTF, ">$featuresFile")) {
	die("could not open features file");
}

if ($verify) {
	unless (open(VOUTF, ">$verifyFeaturesFile")) {
		die("could not open features file");
	}
}

unless (open(GROUTF, ">$groupCountsFile")) {
	die("could not open group counts file");
}

unless (open(UOUTF, ">$userCountsFile")) {
	die("could not open group counts file");
}

foreach my $groupidx (sort { $a <=> $b } keys %$groupcounts) {
	my $group = $groupcounts->{$groupidx};
	my $groupsize = scalar keys %$group;
	print GROUTF ("$groupidx,$groupsize\n");
	foreach my $useridx (sort { $a <=> $b } keys %$group) {
		my $userdata = $featurevectors->{$useridx};
		my $doccount = @$userdata;
		my $user = $idx2user->{$useridx};
		print UOUTF ("$groupidx,$user,$useridx,$doccount\n");
		foreach my $doc (@$userdata) {
			print OUTF ("$groupidx,$useridx");
			if ($verify) {
				print VOUTF ("$groupidx,$user");
			}
			foreach my $wordidx (sort { $a <=> $b } keys %$doc) {
				my $wordcount = $doc->{$wordidx};
				print OUTF (",$wordidx:$wordcount");
				if ($verify) {
					my $word = $idx2word->{$wordidx};
					print VOUTF (",$word:$wordcount");
				}
			}
			print OUTF ("\n");
			if ($verify) {
				print VOUTF ("\n");
			}
		}
	}
}

close(GRUTF);
close(UOUTF);
close(OUTF);

if ($verify) {
	close(VOUTF);
}

sub getIdx2ref($) {
	my $refs = @_[0];
	my $idxs = {};
	while (my ($ref, $idx) = each %$refs) {
		$idxs->{$idx} = $ref;
	}
	return $idxs;
}

sub featurize($$) {

	my $filePath = @_[0];
	my $vocab = @_[1];
	
	my $groupcounts = {};
	my $useridxs = {};
	my $maxuseridx = 0;
	
	my $doccount = 0;
	my $totalwordcount = 0;
	
	my $file=open(INPT, "$filePath") || die("could not open file");
	
	my $i = 0;
	my $line = ""; # <INPT>; # No Header line
	$line = <INPT>; # First row of data
	while ($line ne "") {
		
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),([^,]+),(.*)/) {
			my $groupidx = $1;
			my $user = lc($2);
			my $msg = $3;
			my $refs = $4;
			if ($msg =~ /^\"(.*)\"$/) {
				$msg = $1;
			}
			
			my $doc = {};
			my $wordcnt = 0;
			my @msgwords = split(/ /, $msg);
			for my $word (@msgwords) {
				if (exists $vocab->{$word}) {
					my $wordidx = $vocab->{$word};
					$wordcnt = $wordcnt + 1;
					$totalwordcount = $totalwordcount + 1;
					my $wordidx = $vocab->{$word};
					if (exists $doc->{$wordidx}) {
						$doc->{$wordidx} = $doc->{$wordidx} + 1;
					} else {
						$doc->{$wordidx} = 1;
					}
				}
			}
			
			if ($wordcnt > 0) {
				if (!exists $useridxs->{$user}) {
					$useridxs->{$user} = $maxuseridx;
					$maxuseridx = $maxuseridx + 1;
				}
				my $useridx = $useridxs->{$user};
				if (!exists $groupcounts->{$groupidx}) {
					$groupcounts->{$groupidx} = {};
				}
				my $group = $groupcounts->{$groupidx};
				if (!exists $group->{$useridx}) {
					$group->{$useridx} = 1;
				} else {
					$group->{$useridx} = $group->{$useridx} + 1;
				}
				if (!exists $featurevectors->{$useridx}) {
					$featurevectors->{$useridx} = [];
				}
				my $userdata = $featurevectors->{$useridx};
				push (@$userdata, $doc);
				$doccount = $doccount + 1;
			}
			
		}
		
		$line = <INPT>;
		
		$i = $i + 1;
		if ($i % 10000 == 0) {
			print ("Processed $i...\n");
		}
		
	}
	
	my $avgwordcount = $totalwordcount / $doccount;
	print ("Total Docs: $doccount, Total word count: $totalwordcount, Avg words/doc: $avgwordcount\n");
	
	close(INPT);
	
	return ($groupcounts, $useridxs, $featurevectors);
	
}

sub load_vocab($) {

	my $file_path = @_[0];
	
	my $vocab = {};
	
	my $file=open(INPT, "$file_path") || die("could not open groups file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),(.*)/igsm) {
			my $wordidx = $1;
			my $word = $2;
			$vocab->{$word} = $wordidx;
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
