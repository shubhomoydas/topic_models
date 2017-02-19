#!perl

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl create_feature_vectors.pl
#
# NOTE: Make sure that the variables $dirPath and $outPath are
# correctly set.
#
#===============================================================

sub processFile($$$$);
sub load_vocab($$);
sub write_output($$);
sub trim($);

my $codePath = ".";
my $dirPath = "data/TwitterData/parsed";
my $fileName = "msgs.csv";
my $outPath = "data/TwitterData/parsed";
my $vocabFile = "$dirPath/vocab.csv";
my $outFile = "$outPath/twitter_feature_vectors.csv";

my $vocab = {};
load_vocab("$vocabFile",$vocab);

my $len = scalar keys %$vocab;
print ("Loaded Vocab. Len=$len...\n");

my $fvs = {};
processFile("$dirPath/$fileName",$outFile,$fvs,$vocab);

write_output($fvs,$outFile);

sub write_output($$) {
	my $fvs = @_[0];
	my $outFile = @_[1];
	unless (open(OUTF, ">$outFile")) {
		die("could not open file");
	}
	my $i = 1;
	foreach my $id (sort { ($a <=> $b) } keys %$fvs) {
		my $fv_user = $fvs->{$id};
		my $n = scalar @$fv_user;
		#if ($n == 0) {
		#	print ("$id,$n\n");
		#}
		foreach $fv (@$fv_user) {
			print OUTF ("$id");
			foreach my $word (sort { ($a <=> $b) } keys %$fv) {
				my $cnt = $fv->{$word};
				print OUTF (",$word:$cnt");
			}
			print OUTF ("\n");
		}
	}
	close(OUTF);
}

sub processFile($$$$) {

	my $filePath = @_[0];
	my $outFile = @_[1];
	my $fvs = @_[2];
	my $vocab = @_[3];
	
	$file=open(INPT, "$filePath") || die("could not open file");

	my $line = ""; # <INPT>; # No Header line
	$line = <INPT>; # First row of data
	while ($line ne "") {
		
		my $userid = "";
		my $msg = "";
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),([^,]+)/) {
			$id = $1;
			$message = $3;
			my $fv_user;
			if (exists $fvs->{$id}) {
				$fv_user = $fvs->{$id};
			} else {
				$fv_user = [];
				$fvs->{$id} = $fv_user;
			}
			my @words = split(/ /,$message);
			my $fv = {};
			foreach my $word (@words) {
				if (exists $vocab->{$word}) {
					my $wid = $vocab->{$word};
					if (exists $fv->{$wid}) {
						$fv->{$wid} = $fv->{$wid} + 1;
					} else {
						$fv->{$wid} = 1;
					}
				}
			}
			my $l = scalar keys %$fv;
			if ($l > 0) {
				push(@$fv_user,$fv);
			}
		}
		
		$line = <INPT>;
		
	}
	close(INPT);
}

sub load_vocab($$) {

	my $file_path = @_[0];
	my $vocab = @_[1];
	
	my $file=open(INPT, "$file_path") || die("could not open file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),([^,]+)/igsm) {
			my $word = $2;
			my $id = $1;
			$vocab->{$word} = $id;
		}
		$line = <INPT>;
	}
	
	close(INPT);
	
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
