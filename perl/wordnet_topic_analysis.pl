#!/usr/bin/perl
use strict;

#===============================================================
#
# To run:
# Ensure that WNHOME env var is set
#  setenv WNHOME /scratch/wordnet/local/WordNet-3.0
# Ensure the perl module install locations are in perl lib search path
#  setenv PERL5LIB /scratch/wordnet/local/perl-lib
#
# cd git/topic_models/perl
# perl ./wordnet_topic_analysis.pl
#
#===============================================================

use feature "switch";

use WordNet::QueryData;
use WordNet::Similarity::path;
use WordNet::Similarity::lin;
use WordNet::Similarity::lesk;
use WordNet::Similarity::res;
use WordNet::Similarity::hso;
use WordNet::Similarity::jcn;
use WordNet::Similarity::vector;

sub loadTopics($);
sub trim($);

my $epoch = 4000;

my @runids = ("lda-0-1","lda-0-2","lda-1-1","lda-1-2","lda-2-1","lda-2-2");
my @similarityMeasures = ("res","lesk","lin","path","vector","jcn");

# Syntactic categories:
#   n - noun, v - verb, a - adjective, r - adverb
my @synsets = ("n","v","a","r");

my $maxSimNum = 6;

for my $runid (@runids) {

	my $filepath = "data/TwitterData/lda-runs/topics/taus-$runid-$epoch-topics.csv";
	my $topics = loadTopics($filepath);
	#my $topic = $topics->{0};
	#my $size = @$topic;
	#print ("size: $size\n");
	#for (my  $i = 0; $i < 10; $i++) {
	#	my $word = @$topic[$i];
	#	print ("$word ");
	#}
	#print ("\n");

	print ("Loaded topics for $runid...\n");

	for my $similarityMeasure (@similarityMeasures) {

		my $relatednessfilepath = "data/TwitterData/lda-runs/relatedness/taus-$runid-$epoch-$similarityMeasure-relatedness.csv";
		my $relatednessSummaryfilepath = "data/TwitterData/lda-runs/relatedness/taus-$runid-$epoch-$similarityMeasure-relatedness-summary.csv";
		my $relatednessErrfilepath = "data/TwitterData/lda-runs/relatedness/taus-$runid-$epoch-$similarityMeasure-relatedness-err.csv";

		unless (open(OUTF, ">$relatednessfilepath")) {
			die("could not open output relatedness file");
		}

		unless (open(SUMF, ">$relatednessSummaryfilepath")) {
			die("could not open output relatedness summary file");
		}

		unless (open(ERRF, ">$relatednessErrfilepath")) {
			die("could not open output relatedness error file");
		}

		my $wn = WordNet::QueryData->new;
		
		my $measure; # = WordNet::Similarity::path->new ($wn);
		given ($similarityMeasure) {
			when ("lin") { $measure = WordNet::Similarity::lin->new ($wn); }
			when ("lesk") { $measure = WordNet::Similarity::lesk->new ($wn); }
			when ("res") { $measure = WordNet::Similarity::res->new ($wn); }
			when ("hso") { $measure = WordNet::Similarity::hso->new ($wn); }
			when ("jcn") { $measure = WordNet::Similarity::jcn->new ($wn); }
			when ("vector") { $measure = WordNet::Similarity::vector->new ($wn); }
			default { $measure = WordNet::Similarity::path->new ($wn); }
		}

		foreach my $topicid (sort { $a <=> $b } keys %$topics) {
			my $topic = $topics->{$topicid};
			my $topicRelatednessSum = 0;
			my $topicRelatednessCount = 0;
			my $semanticHits = 0;
			for (my $i = 0; $i < 9; $i++) {
				my $wordi = @$topic[$i];
				for (my $j = $i+1; $j < 10; $j++) {
					my $wordj = @$topic[$j];
					my $relatednessSum = 0;
					my $relatednessCount = 0;
					#print ("Checking relatedness between $wordi,$wordj\n");
					for my $synseti (@synsets) {
						for my $synsetj (@synsets) {
							for (my $sensnumi = 1; $sensnumi <= $maxSimNum; $sensnumi++) {
								for (my $sensnumj = 1; $sensnumj <= $maxSimNum; $sensnumj++) {
									my $relatedness = $measure->getRelatedness("$wordi#$synseti#$sensnumi", "$wordj#$synsetj#$sensnumj");
									my ($error, $errorString) = $measure->getError();
									if (!$error) {
										#print ("relatedness ($wordi,$wordj) = $relatedness\n");
										$relatednessSum =  $relatednessSum + $relatedness;
										$relatednessCount++;
										$semanticHits++;
									} else {
										#print ERRF ("$wordi,$synseti,$sensnumi,$wordj,$synsetj,$sensnumj,$error,$errorString\n");
										last;
									}
								}
							}
						}
					}
					my $meanRelatedness = 0;
					if ($relatednessCount > 0) {
						$meanRelatedness = $relatednessSum/$relatednessCount;
						#print ("avg relatedness ($wordi,$wordj) = $meanRelatedness ($relatednessCount)\n");
					} else {
						#print ("avg relatedness ($wordi,$wordj) = NA\n");
					}
					$topicRelatednessSum = $topicRelatednessSum + $meanRelatedness;
					$topicRelatednessCount = $topicRelatednessCount + 1;
					print OUTF ("$topicid,$wordi,$wordj,$meanRelatedness,$relatednessCount\n");
				}
			}
			my $topicMeanRelatedness = $topicRelatednessSum/$topicRelatednessCount;
			print SUMF ("$topicid,$topicMeanRelatedness,$semanticHits,$topicRelatednessCount\n");
			if (($topicid+1) % 5 == 0) {
				print ("Processed till runid: $runid, sim: $similarityMeasure, topic: $topicid\n");
			}
		}

		close (OUTF);
		close (SUMF);
		close (ERRF);

	} # for $similarityMeasure

} # for $runid

sub loadTopics($) {
	my $file_path = @_[0];
	my $file=open(INPT, "$file_path") || die("could not open file");
	my $topics = {};
	
	# Initialize all topic arrays...
	my $line = <INPT>; # header line
	$line = trim($line);
	my @topicids = split(/,/,$line);
	my $topic = 0;
	for my $topicid (@topicids) {
		$topics->{$topic} = [];
		$topic = $topic+1;
	}

	my $line = <INPT>; # first line
	while ($line ne "") {
		$line = trim($line);
		my @words = split(/,/,$line);
		my $topic = 0;
		for my $word (@words) {
			my $topicwords = $topics->{$topic};
			push(@$topicwords, $word);
			$topic = $topic+1;
		}
		$line = <INPT>;
	}

	close (INPT);

	return $topics;
}

sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}

