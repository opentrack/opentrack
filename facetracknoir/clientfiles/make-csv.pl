#!/usr/bin/env perl

use strict;
use List::Util qw'reduce';

sub get_games_1 {
	my @games;

	open my $fd, "<", $ARGV[1] or die "open: $!";
	<$fd>;

	while (defined(my $line = <$fd>)) {
		chomp $line;
		if ($line !~ /^(\d+)\s+"([^"]+)"(?:\s+\(([0-9A-F]{16})\))?$/) {
			warn "Broken line";
			next;
		}
		push @games, +{ id => $1, name => $2, key => defined $3 ? (sprintf "%04X", $1) . $3 . '00' : undef};
	}

	[@games];
}

sub get_games_2 {
	open my $fd, "<", $ARGV[0] or die "open: $!";
	<$fd>;
	my @games;
	while (defined(my $line = <$fd>)) {
		chomp $line;
		my @line = split/;/, $line;
		if (@line != 8) {
			warn "Broken line";
			next;
		}
		my @cols = qw'no name proto since verified by id key';
		push @games, +{ map { $cols[$_] => $line[$_] } 0..$#cols };
	}
	[@games];
}

sub merge {
	my ($new_games, $old_games) = @_;
	my $no = (reduce { $a->{no} > $b->{no} ? $a : $b } +{id=>0}, @$old_games)->{no} + 1;
	my %game_hash = map { $_->{name} => $_ } @$old_games;
	my %ids = map { $_->{id} => 1 } @$old_games;
	for my $g (@$new_games) {
		if (!exists $game_hash{$g->{name}} && !exists $ids{$g->{id}}) {
			$game_hash{$g->{name}} = +{
				no => $no++,
				name => $g->{name},
				proto => 'FreeTrack20',
				since => (defined $g->{key} ? 'V170' : 'V160'),
				verified => '',
				by => '',
				id => $g->{id},
				key => $g->{key}
			};
		}
	}
	print "No;Game Name;Game protocol;Supported since;Verified;By;INTERNATIONAL_ID;FTN_ID\n";
	for (sort { lc($a->{name}) cmp lc($b->{name}) } values %game_hash) {
		my $g = {%$_};
		if (!defined $g->{key}) {
			$g->{key} = (sprintf "%04X", $g->{no}) . (join"", map { sprintf "%02X", int rand 256 } 0 .. 7) . '00';
		}
		my @cols = qw'no name proto since verified by id key';
		print join";", map { $g->{$_} } @cols;
		print "\n";
	}
}

merge(get_games_1(), get_games_2());
