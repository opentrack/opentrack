#!/usr/bin/env perl

use strict;
use List::Util qw'reduce';

use POSIX qw(locale_h);
setlocale(LC_ALL, "C");

sub get_games_1
{
    my @games;

    open my $fd, "<", $ARGV[1] or die "open: $!";
    binmode $fd;
    <$fd>;

    while (defined(my $line = <$fd>))
    {
        $line =~ s/[\r\n]+$//s;
        if ($line !~ /^(\d+)\s+"([^;"]+)"(?:\s+\(([0-9A-F]{16})\))?$/)
        {
            warn "Broken line";
            next;
        }
        next if $1 <= 0;
        push @games, +{ id => $1, name => $2, key => $3 }
    }
    [sort { lc($a->{name}) cmp lc($b->{name}) } @games]
}

sub get_games_2
{
    open my $fd, "<", $ARGV[0] or die "open: $!";
    binmode $fd;
    <$fd>;
    my @games;
    my %ids;
    while (defined(my $line = <$fd>))
    {
        $line =~ s/[\r\n]+$//s;
        my @line = split/;/, $line;
        if (@line != 8)
        {
            warn "Broken line";
            next;
        }
        my @cols = qw'no name proto since verified by id key';
        my $h = +{ map { $cols[$_] => $line[$_] } 0..$#cols };
        next if exists $ids{$h->{id}};
        $ids{$h->{id}} = undef;
        next if $h->{id} <= 0;
        push @games, $h;
    }
    [@games];
}

sub merge
{
    my ($new_games, $old_games) = @_;
    my $no = (reduce { $a->{no} > $b->{no} ? $a : $b } +{id=>0}, @$old_games)->{no} + 1;
    my %ids = map { $_->{id} => $_ } @$old_games;
    binmode \*STDOUT;
    for my $g (@$new_games)
    {
        my $id = $g->{id};
        my $no_ = $ids{$id} ? $ids{$id}->{no} : $no;
        next if (exists($ids{$id}) && $ids{$id}->{verified} ne '');
        my $old = $ids{$id} || do { $no++; +{} };
        $ids{$id} =
            +{
                no => $no_,
                name => $g->{name},
                proto => 'FreeTrack20',
                verified => '',
                by => '',
                id => $g->{id},
                %$old,
                since => $g->{key} ? 'V170' : 'V160',
                key => $g->{key} ? (sprintf "%04X", $no_) . $g->{key} . '00' : $old->{key}
            };
    }
    print "No;Game Name;Game protocol;Supported since;Verified;By;INTERNATIONAL_ID;FTN_ID\n";
    for (sort { $a->{no} <=> $b->{no} } values %ids)
    {
        my $g = {%$_};
        if (!defined $g->{key})
        {
            $g->{key} = (sprintf "%04X", $g->{no}) . (join"", map { sprintf "%02X", int rand 256 } 0 .. 7) . '00';
        }
        my @cols = qw'no name proto since verified by id key';
        print join";", map { $g->{$_} } @cols;
        print "\n";
    }
}

if (@ARGV != 2)
{
    warn "usage: $0 orig.csv dump.txt\n";
    exit 1;
}
else
{
    merge(get_games_1(), get_games_2());
    exit 0;
}
