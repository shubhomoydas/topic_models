#!/usr/bin/perl

# To run:
# Ensure that WNHOME env var is set
#### setenv WNHOME /scratch/wordnet/local/WordNet-3.0
# Ensure the perl module install locations are in perl lib search path
#### setenv PERL5LIB /scratch/wordnet/local/perl-lib
# cd git/topic_models/perl
# perl ./wordnet_test.pl

use WordNet::QueryData;

use WordNet::Similarity::path;

my $wn = WordNet::QueryData->new;

my $measure = WordNet::Similarity::path->new ($wn);

my $value = $measure->getRelatedness("car#n#1", "bus#n#2");

my ($error, $errorString) = $measure->getError();

die $errorString if $error;

print "car (sense 1) <-> bus (sense 2) = $value\n";

# repeat in reverse
#$measure = WordNet::Similarity::path->new ($wn);

$value = $measure->getRelatedness("bus#n#1", "car#n#2");

($error, $errorString) = $measure->getError();

die $errorString if $error;

print "car (sense 1) <-> bus (sense 2) = $value\n";

# All senses?
$value = $measure->getRelatedness("bus#n", "car#n");

($error, $errorString) = $measure->getError();

die $errorString if $error;

print "car (sense 1) <-> bus (sense 2) = $value\n";


