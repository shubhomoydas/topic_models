#!perl

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl parse_twitter.pl
#
# NOTE: Make sure that the variables $dirPath and $outPath are
# correctly set.
#
#===============================================================

sub processFile($$$);
sub trim($);

$dirPath = "data/TwitterData/trainingandtestdata";
$fileName = "training.1600000.processed.noemoticon.csv";
#$fileName = "testdata.manual.2009.06.14.csv";
$outPath = "data/TwitterData/parsed";
$outFile = "$outPath/users.txt";
$outCountFile = "$outPath/userCounts.csv";

$userCounts = {};

processFile ($dirPath, $fileName, $outFile);

unless (open(CNTF, ">$outCountFile")) {
	die("could not open file");
}
for $userid (keys %$userCounts) {
	my $cnt = $userCounts->{$userid};
	print CNTF ("$userid,$cnt\n");
}
close(CNTF);

sub processFile($$$) {

	my $dirPath = @_[0];
	my $fileName = @_[1];
	my $outFile = @_[2];
	
	#print ("$dirPath/$fileName\n");
	
	$file=open(INPT, "$dirPath/$fileName") || die("could not open file");
	unless (open(OUTF, ">$outFile")) {
		die("could not open file");
	}

	my $line = ""; # <INPT>; # No Header line
	
	#print SUMMARYF ("$dirPath/$fileName\n");
	#print SUMMARYF ("$line");
	
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
			if ($msg =~ /^\"(.*)\"$/) {
				$msg = $1;
			}
			if (exists $userCounts->{$userid}) {
				$userCounts->{$userid} = $userCounts->{$userid} + 1;
			} else {
				$userCounts->{$userid} = 1;
			}
			#print OUTF ("$userid - $msg\n");
		}
		
		#print SUMMARYF ("$reg ");
		
		$line = <INPT>;
		
	}
	#print SUMMARYF ("\n");
	close(INPT);
	close(OUTF);
	#close(SUMMARYF);
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
