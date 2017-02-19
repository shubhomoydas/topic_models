#!perl
use strict;

#===============================================================
#
# To run this use:
# cd git/topic_models/perl
# perl explore_networks.pl
#
# Program Description:
# Takes the generated user groups file as input and outputs certain
# statistics. The input file is a CSV file with NO headers with
# rows in the following format:
#   $userid,$handles,$hashtags
# 
# Output:
# 
# 
# NOTE: Make sure that the variables $inPath and $outPath are
# correctly set.
#
#===============================================================
use List::Util qw( min max );

sub loadGroups($);
sub loadUserMemberships($$$$);
sub processFile($$$);
sub getIdx2reference($);
sub computeGroups($$$);
sub trim($);

my $inPath = "data/TwitterData/parsed";
my $outPath = "data/TwitterData/parsed";

# Input Files
my $userReferencesFile = "$inPath/training-user-references.csv";

# Either Input or Output depending on the $loadCommunicationData flag
my $userCommunicationsFile = "$inPath/training-user-communications.csv";
my $userCommunicationCountsFile = "$inPath/training-user-communication-counts.csv";

# Output Files
my $userGroupsFile = "$outPath/training-user-groups.csv";

my $loadCommunicationData = 1;
my $computeGroups = 1;

my $communications = {};
my $communicationCounts = {};
my $reference2idx = {};

if ($loadCommunicationData == 0) {
	($communicationCounts, $reference2idx, $communications) = processFile($userReferencesFile, $userCommunicationCountsFile, $userCommunicationsFile);
	print ("Computed Membership data...\n");
} else {
	($communicationCounts, $reference2idx) = loadGroups($userCommunicationCountsFile);
	my @tkeys = keys %$communicationCounts;
	my $cnt = @tkeys;
	print ("Number of communication counts loaded: $cnt...\n");

	$communications = loadUserMemberships($userCommunicationsFile, $communicationCounts, $reference2idx, 50);
	@tkeys = keys %$communications;
	my $cnt = @tkeys;
	print ("Number of user communications loaded: $cnt...\n");
	
	# DEBUG code
	my $debug = 0;
	if ($debug) {
		my $idx2reference = getIdx2reference($reference2idx);
		my @useridxs = keys %$communications;
		for (my $idx = 0; $idx <= 10; $idx++) {
			my $useridx = $useridxs[$idx];
			my $user = $$idx2reference[$useridx];
			print ("$user => $useridx\n");
			my $communicationMembers = $communications->{$useridx};
			while (my ($connidx, $connval) = each %$communicationMembers) {
				my $conn = $$idx2reference[$connidx];
				print ("  $conn => $connidx\n");
			}
		}
	}
}

if ($computeGroups == 1) {
	my $idx2reference = getIdx2reference($reference2idx);
	my $groups = computeGroups($communicationCounts, $reference2idx, $communications);
	unless (open(MOUTF, ">$userGroupsFile")) {
		die("could not open membership file");
	}
	my $groupid = 1;
	while (my ($useridx, $members) = each %$groups) {
		my $user = $$idx2reference[$useridx];
		my @k = keys %$members;
		my $groupsize = @k;
		#print MOUTF ("$groupid,$groupsize\n");
		while (my ($memberidx, $memberval) = each %$members) {
			my $member = $$idx2reference[$memberidx];
			print MOUTF ("$groupid,$member,$groupsize\n");
		}
		$groupid = $groupid + 1;
	}
	close(MOUTF);
}

sub computeGroups($$$) {
	my $communicationCounts = @_[0];
	my $reference2idx = @_[1];
	my $communications = @_[2];

	my $idx2reference = getIdx2reference($reference2idx);
	my $child2parent = {};
	my $groups = {};
	my $useActiveOnly = 1;
	# initialize each user as a individual parent
	while (my ($useridx, $communicationMembers) = each %$communications) {
		$groups->{$useridx} = {};
		$groups->{$useridx}->{$useridx} = $useridx;
		# do not ignore those handles that have no direct tweets
		while (my ($linkidx, $linkval) = each %$communicationMembers) {
			# initially, each entity is it's own parent
			if (!$useActiveOnly or exists $communications->{$linkidx}) {
				$child2parent->{$linkidx} = $linkidx;
			}
		}
	}
	my @tkeys = keys %$groups;
	my $nsize = @tkeys;
	my @useridxs = keys %$communications;
	my $usercnt = @useridxs;
	print ("Initialized groups, size: $nsize; communications size: $usercnt...\n");
	my $i = 0;
	my $maxgroupsize = 200;
	my $merged = 1000;
	my $epoch = 0;
	while ($merged > 50) {
		$merged = 0;
		while (my ($useridx, $communicationMembers) = each %$communications) {
			#my $starttime = time();
			#my $endtime = 0;
			my $user = $$idx2reference[$useridx];
			my $parentidx = $child2parent->{$useridx};
			my $parent = $$idx2reference[$parentidx];
			my $parentGroup = $groups->{$parentidx};
			#print ("Processing $parent...\n");
			my @nkeys = keys %$parentGroup;
			my $nsize = @nkeys;
			if ($nsize > $maxgroupsize) {
				# do not let the communicationCounts grow too large...
				next;
			}
			while (my ($linkidx, $linkval) = each %$communicationMembers) {
				my $link = $$idx2reference[$linkidx];

				# We will not process those handles which
				# never have replied directly. Else the
				# communicationCounts were getting unwieldy...
				if ($useActiveOnly and !exists $communications->{$linkidx}) {
					next;
				}
		
				# First, check if current parent is
				# already the parent for this link and
				# if so, then skip.
				my $linkParentidx = $child2parent->{$linkidx};
				my $linkParent = $$idx2reference[$linkParentidx];
				if ($linkParentidx eq $parentidx) {
					next;
				}
			
				# If the current link does not have a
				# parent node in the groups, then add
				# this to the current parent groups.
				# NOTE: This step is NOT required if we
				# are only looking for direct replies.
				#if (!exists $groups->{$linkParentidx}) {
				#	#print ("Adding $link to $parent\n");
				#	$parentGroup->{$linkidx} = $linkidx;
				#	$child2parent->{$linkidx} = $parentidx;
				#	next;
				#}
			
				# Now, take all children of the link's parent
				# and merge them into the current parent, and
				# delete the $linkidx groups from top level.
				# NOTE: Do this only is the group size is not
				# too large...
				my $linkGroup = $groups->{$linkParentidx};
				my @lkeys = keys %$linkGroup;
				my $lsize = @lkeys;
				if ($lsize > $maxgroupsize) {
					# do not let the communicationCounts grow too large...
					next;
				}
				while (my ($childidx, $childval) = each %$linkGroup) {
					$parentGroup->{$childidx} = $childidx;
					$child2parent->{$childidx} = $parentidx;
				}
				delete $groups->{$linkParentidx};
				#print ("merged $linkParent into $parent...\n");
				# Merge only one child per iteration to give all
				# communities equal chance to grow
				$merged = $merged + 1;
				last;
			}
			$i = $i + 1;
			if ($i % 2000 == 0 or $i == $usercnt) {
				my @k = keys %$groups;
				my $ngroups = @k;
				print ("No. groups: $ngroups\n");
			}
			#print ("Time (sec): ".($starttime-time())."\n");
		}
		$epoch = $epoch + 1;
		print ("Finished epoch $epoch, merged $merged...\n");
	} # while (merged)
	return $groups;
}

sub loadGroups($) {

	my $file_path = @_[0];
	
	my $communicationCounts = {};
	my $reference2idx = {};
	
	my $file=open(INPT, "$file_path") || die("could not open file");
	
	my $idx = 0;
	my $line = <INPT>;
	while ($line ne "") {
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]+),([^,]+)/igsm) {
			my $group = $1;
			if (!exists $reference2idx->{$group}) {
				$reference2idx->{$group} = $idx;
				$idx = $idx + 1;
			}
			my $groupidx = $reference2idx->{$group};
			$communicationCounts->{$groupidx} = $2;
		}
		$line = <INPT>;
	}
	
	close(INPT);
	
	return ($communicationCounts, $reference2idx);
	
}

sub loadUserMemberships($$$$) {

	my $file_path = @_[0];
	my $communicationCounts = @_[1];
	my $reference2idx = @_[2];
	my $max = @_[3];
	
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
			my $cnt = $communicationCounts->{$useridx};
			if ($cnt <= $max) {
				if (!exists $communications->{$useridx}) {
					$communications->{$useridx} = {};
				}
				my $communicationMembers = $communications->{$useridx};
				$communicationMembers->{$connidx} = 1;
			}
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

sub processFile($$$) {

	my $referencesFile = @_[0];
	my $communicationCountsFile = @_[1];
	my $communicatesFile = @_[2];
	
	my $useHashtags = 0;
	
	my $file=open(INPT, "$referencesFile") || die("could not open file");

	my $communicationCounts = {};
	my $reference2idx = {};
	my $communications = {};
	my $idx = 0;
	my $i = 0;
	my $line = ""; # <INPT>; # No Header line
	$line = <INPT>; # First row of data
	while ($line ne "") {
		
		$line = trim($line);
		if ($line =~ /^([^,]+),([^,]*),([^,]*)/) {
			my $user = trim($1);
			my @handles = split(/;/,$2);
			my @hashtags = split(/;/,$3);
			my $userlinks = {};
			if (!exists $reference2idx->{$user}) {
				$reference2idx->{$user} = $idx;
				$idx = $idx + 1;
			}
			my $useridx = $reference2idx->{$user};
			if (!exists $communications->{$useridx}) {
				$communications->{$useridx} = {};
				$communications->{$useridx}->{$useridx} = 1;
			}
			my $userlinks = $communications->{$useridx};
			foreach my $handle (@handles) {
				$handle = trim($handle);
				if ($handle eq "") {
					next;
				}
				if (!exists $reference2idx->{$handle}) {
					$reference2idx->{$handle} = $idx;
					$idx = $idx + 1;
				}
				my $handleidx = $reference2idx->{$handle};
				$userlinks->{$handleidx} = 1;
				if (exists $communicationCounts->{$handleidx}) {
					my $cnt = $communicationCounts->{$handleidx} + 1;
					$communicationCounts->{$handleidx} = $cnt;
				} else {
					$communicationCounts->{$handleidx} = 1;
				}
			}
			if ($useHashtags == 1) {
				foreach my $hashtag (@hashtags) {
					$hashtag = trim($hashtag);
					if ($hashtag eq "") {
						next;
					}
					if (!exists $reference2idx->{$hashtag}) {
						$reference2idx->{$hashtag} = $idx;
						$idx = $idx + 1;
					}
					my $hashtagidx = $reference2idx->{$hashtag};
					$userlinks->{$hashtagidx} = 1;
					if (exists $communicationCounts->{$hashtagidx}) {
						my $cnt = $communicationCounts->{$hashtagidx} + 1;
						$communicationCounts->{$hashtagidx} = $cnt;
					} else {
						$communicationCounts->{$hashtagidx} = 1;
					}
				}
			}
		}
		
		$i = $i + 1;
		if ($i % 20000 == 0) {
			print ("processed $i...\n");
		}
		
		$line = <INPT>;
		
	}
	close(INPT);
	
	unless (open(OUTF, ">$communicationCountsFile")) {
		die("could not open file");
	}
	
	my $idx2reference = getIdx2reference($reference2idx);
	my @sortedgroups = sort { -$communicationCounts->{$a} <=> -$communicationCounts->{$b} } keys(%$communicationCounts);
	for my $groupidx (@sortedgroups) {
		my $group = $$idx2reference[$groupidx];
		my $cnt = $communicationCounts->{$groupidx};
		my $isActive = 0;
		if (exists $communications->{$groupidx}) {
			$isActive = 1;
		}
		print OUTF ("$group,$cnt,$isActive\n");
	}
	close(OUTF);
	
	unless (open(MEMOUTF, ">$communicatesFile")) {
		die("could not open file");
	}
	for my $useridx (keys %$communications) {
		my $user = $$idx2reference[$useridx];
		my $communicationMembers = $communications->{$useridx};
		for my $memberidx (keys %$communicationMembers) {
			my $member = $$idx2reference[$memberidx];
			print MEMOUTF ("$user,$member\n");
		}
	}
	close(MEMOUTF);
	
	return ($communicationCounts, $reference2idx, $communications);
}

# Perl trim function to remove whitespace from the start and end of the string
sub trim($)
{
	my $string = shift;
	$string =~ s/^\s+//;
	$string =~ s/\s+$//;
	return $string;
}
