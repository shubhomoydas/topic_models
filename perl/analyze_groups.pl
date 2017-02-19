#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl analyze_groups.pl
#
# Program Description:
# Takes two input files:
#
# 1. The generated user groups file with NO headers with rows in 
#    the following format:
#      $groupid,$userid
#
# 2. The user-communications file in CSV format with NO headers:
#      $userid,$to_userid
# 
# Output:
# 
# 
# NOTE: Make sure that the variables $inPath and $outPath are
# correctly set.
#
#===============================================================
use List::Util qw( min max );

sub count_group_communications($$$);
sub load_groups($);
sub load_communications($$);
sub getIdx2reference($);
sub trim($);

#my $inPath = "data/TwitterData/parsed";
#my $outPath = "data/TwitterData/lda-runs/lda-2-1-inferGroups";

my $inPath = "data/TwitterData/parsed";
my $outPath = "data/TwitterData/parsed";
#my $inPath = "data/TwitterData/parsed";
#my $outPath = "data/TwitterData/parsed";

my $inGroupsPath = $outPath;

my $userCommunicationsFile = "$inPath/training-user-communications.csv";
my $groupsFile = "$inGroupsPath/training-filtered-groups.csv";
#my $groupsFile = "$inGroupsPath/groups2users.csv";

my $groupCommunicationCountsFile = "$outPath/group-communication-counts.csv";

my ($groups, $reference2idx) = load_groups($groupsFile);
my @groupids = keys %$groups;
my $ngroups = @groupids;
my @refids = keys %$reference2idx;
my $nrefs = @refids;
print ("Loaded $ngroups groups, $nrefs references...\n");

my $communications = load_communications($userCommunicationsFile, $reference2idx);
my @userids = keys %$communications;
my $ncomms = @userids;
print ("Loaded $ncomms user communications...\n");

unless (open(OUTF, ">$groupCommunicationCountsFile")) {
	die("could not open output file");
}
while (my ($groupidx, $group) = each %$groups) {
	my @memberids = keys %$group;
	my $groupsz = @memberids;
	print ("Processing groupidx $groupidx: $groupsz...");
	#print OUTF ("$groupidx,$groupsz\n");
	if ($groupsz > 10000) {
		print OUTF ("$groupidx,0,$groupsz,0\n");
		next;
	}
	my $commCounts = count_group_communications($group, $communications, $reference2idx);
	my $ratio = 1;
	if ($groupsz > 1) {
		$ratio = $commCounts/($groupsz * ($groupsz-1)/2);
	}
	print ("$commCounts,$ratio...\n");
	print OUTF ("$groupidx,$commCounts,$groupsz,$ratio\n");
}
close (OUTF);

sub count_group_communications($$$) {
	my $group = @_[0];
	my $communications = @_[1];
	my $reference2idx = @_[2];
	my $ncomms = 0;
	my @members = keys %$group;
	for my $useridx_i (@members) {
		if (!exists $communications->{$useridx_i}) {
			next;
		}
		my $userComms_i = $communications->{$useridx_i};
		for my $useridx_j (@members) {
			if ($useridx_j == $useridx_i) {
				next;
			}
			if (exists $userComms_i->{$useridx_j}) {
				$ncomms++;
			}
			#if (!exists $communications->{$useridx_j}) {
			#	next;
			#}
			#my $userComms_j = $communications->{$useridx_j};
			#if (exists $userComms_j->{$useridx_i}) {
			#	$ncomms++;
			#}
		}
	}
	return $ncomms;
}

sub load_groups($) {

	my $file_path = @_[0];
	
	my $groups = {};
	my $reference2idx = {};
	my $maxidx = 0;
	
	my $file=open(INPT, "$file_path") || die("could not open groups file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+)/igsm) {
			my $groupidx = $1;
			my $user = $2;
			if (!exists $groups->{$groupidx}) {
				$groups->{$groupidx} = {};
			}
			my $group = $groups->{$groupidx};
			if (!exists $reference2idx->{$user}) {
				$reference2idx->{$user} = $maxidx;
				$maxidx = $maxidx + 1;
			}
			my $useridx = $reference2idx->{$user};
			$group->{$useridx} = 1;
		}
		$line = <INPT>;
	}
	
	close(INPT);
	
	return ($groups, $reference2idx);
	
}

sub load_communications($$) {

	my $file_path = @_[0];
	my $reference2idx = @_[1];
	
	my $communications = {};
	
	my $maxidx = max (values %$reference2idx);
	$maxidx = $maxidx + 1;
	print ("Max group idx: $maxidx\n");

	my $file=open(INPT, "$file_path") || die("could not open file");
	
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+)/igsm) {
			my $user = $1;
			my $conn = $2;
			if (!exists $reference2idx->{$user}) {
				$reference2idx->{$user} = $maxidx;
				$maxidx = $maxidx + 1;
			}
			if (!exists $reference2idx->{$conn}) {
				$reference2idx->{$conn} = $maxidx;
				$maxidx = $maxidx + 1;
			}
			my $useridx = $reference2idx->{$user};
			my $connidx = $reference2idx->{$conn};
			if (!exists $communications->{$useridx}) {
				$communications->{$useridx} = {};
			}
			my $communicationMembers = $communications->{$useridx};
			$communicationMembers->{$connidx} = 1;
		}
		$line = <INPT>;
	}
	
	close(INPT);
	
	return $communications;
	
}

sub getIdx2reference($) {
	my $reference2idx = @_[0];
	my @idx2reference = sort { $reference2idx->{$a} <=> $reference2idx->{$b} } keys(%$reference2idx);
	my $gsize = @idx2reference;
	print ("Total number of communicationCounts: $gsize\n");
	return \@idx2reference;
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
